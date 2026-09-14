# include "CommandRecognizer.hpp"

using namespace std;

namespace {
	// 音素: [0:無, 1:息, 2:ア, 3:あ, 4:イ, 5:い, 6:ウ, 7:う, 8:エ, 9:え, 10:オ, 11:お] → [あ, い, う, え, お, 無音]
	constexpr array<size_t, 12> PhonemeLabels = { 5, 5, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4 };
	constexpr size_t SilenceLabel = 5;
	constexpr double Smoothing = 0.02;
	constexpr double Infinity = 1e18;

	const Array<VoiceCommand> CommonCommands = {
		{ U"ガード", U"AO", 4 },
		{ U"守れ", U"AOE", 4 },
		{ U"壊せ", U"OA_E", 5 },
		{ U"刺せ", U"A_E", 5 },
	};

	enum class SegmentType {
		Vowel,
		Gap,
		/// @brief 発話の前後の、無音ならほぼ無償で当てはめられる区間
		Edge,
		/// @brief 母音と母音の間の子音などを当てはめる区間
		Join,
	};

	struct Segment {
		SegmentType type;
		size_t label = 0;
		size_t minFrames = 0;
		bool skippable = true;
	};
}

Array<VoiceCommand> VoiceCommandsOf(int32 character) {
	Array<VoiceCommand> commands;
	switch (character) {
	case 0:
		commands = {
			{ U"殴れ", U"AUE", 1 },
			{ U"キック", U"I_ue", 2 },
			{ U"龍虎水雷撃", U"U_O_uIAIE_i", 3 },
			{ U"魚雷", U"OAI", 6 },
		};
		break;
	case 2:
		commands = {
			{ U"撃て", U"U_E", 1 },
			{ U"斬れ", U"IE", 2 },
			{ U"デッドリーアサルト", U"E_OIA_AU_O", 3 },
			{ U"連射", U"Ei_A", 6 },
		};
		break;
	default:
		commands = {
			{ U"殴れ", U"AUE", 1 },
			{ U"キック", U"I_ue", 2 },
			{ U"龍虎水雷撃", U"U_O_uIAIE_i", 3 },
		};
		break;
	}
	return commands.append(CommonCommands);
}

CommandRecognizer::CommandRecognizer(const CommandRecognizerOptions& options) : options(options) {}

int32 CommandRecognizer::update(const Array<double>& phonemeScores, int32 character, uint64 timeUs) {
	if (timeUs + FrameIntervalUs / 2 < nextFrameUs) return 0;
	nextFrameUs = (nextFrameUs + FrameIntervalUs * 4 < timeUs) ? timeUs + FrameIntervalUs : nextFrameUs + FrameIntervalUs;
	if (cooldown) {
		--cooldown;
		return 0;
	}

	Frame frame{};
	for (size_t id : step(Min(phonemeScores.size(), PhonemeLabels.size()))) frame[PhonemeLabels[id]] += Max(phonemeScores[id], 0.0);
	const double sum = accumulate(frame.begin(), frame.end(), 0.0);
	if (sum <= 0.0) frame[SilenceLabel] = 1.0;
	for (auto& p : frame) p = ((sum > 0.0 ? p / sum : p) + Smoothing) / (1.0 + Smoothing * frame.size());

	const bool voiced = frame[SilenceLabel] < 0.5;
	if (utterance.empty() && !voiced) return 0;
	utterance << frame;
	if (voiced) {
		++voicedFrames;
		silentFrames = 0;
	}
	else {
		++silentFrames;
	}
	if (utterance.size() > options.maxUtteranceFrames) {
		if (utterance.front()[SilenceLabel] < 0.5) --voicedFrames;
		utterance.pop_front();
	}
	if (silentFrames == options.endSilenceFrames && voicedFrames >= options.minVoicedFrames) {
		const int32 action = decide(Array<Frame>(utterance.begin(), utterance.end() - silentFrames), character);
		if (action) {
			utterance.clear();
			voicedFrames = 0;
			silentFrames = 0;
			cooldown = options.cooldownFrames;
			return action;
		}
	}
	if (silentFrames >= Max(options.endSilenceFrames, options.mergeSilenceFrames)) {
		utterance.clear();
		voicedFrames = 0;
		silentFrames = 0;
	}
	return 0;
}

int32 CommandRecognizer::decide(const Array<Frame>& frames, int32 character) const {
	double freeCost = 0.0;
	for (const auto& frame : frames) freeCost += -log(*max_element(frame.begin(), frame.end()));

	struct Candidate {
		const VoiceCommand* command;
		double cost;
		size_t length;
	};
	const auto commands = VoiceCommandsOf(character);
	Array<Candidate> candidates;
	for (const auto& command : commands) {
		const double cost = (alignmentCost(frames, command.pronunciation) - freeCost) / frames.size();
		const size_t length = count_if(command.pronunciation.begin(), command.pronunciation.end(), [](char32 c) { return IsUpper(c); });
		candidates << Candidate{ &command, cost, length };
	}
	const auto best = ranges::min_element(candidates, {}, &Candidate::cost);
	const Candidate* chosen = &*best;
	for (const auto& candidate : candidates) {
		if (candidate.length > chosen->length && candidate.cost <= best->cost + options.longerPreference) chosen = &candidate;
	}
	if (chosen->cost > options.threshold) return 0;
	for (const auto& candidate : candidates) {
		if (candidate.command->action != chosen->command->action && candidate.cost < chosen->cost + options.ambiguityMargin) return 0;
	}
	return chosen->command->action;
}

double CommandRecognizer::alignmentCost(const Array<Frame>& frames, StringView pronunciation) const {
	// 発話の前後 → 母音 → (母音間の子音 or 無声子音の無音) → 母音 ... → 発話の前後 と、区間を一列に並べる
	Array<Segment> segments = { { SegmentType::Edge } };
	bool gap = false;
	for (const char32 c : pronunciation) {
		if (c == U'_') {
			gap = true;
			continue;
		}
		const auto label = StringView{ U"AIUEO" }.indexOf(ToUpper(c));
		if (label == StringView::npos) continue;
		if (segments.size() > 1) {
			segments << Segment{ SegmentType::Join };
			if (gap) {
				segments << Segment{ SegmentType::Gap, SilenceLabel, options.minGapFrames, false };
				segments << Segment{ SegmentType::Join };
			}
		}
		segments << Segment{ SegmentType::Vowel, label, options.minVowelFrames, IsLower(c) };
		gap = false;
	}
	segments << Segment{ SegmentType::Edge };

	Array<size_t> first(segments.size());
	size_t stateCount = 0;
	for (size_t s : step(segments.size())) {
		first[s] = stateCount;
		stateCount += Max<size_t>(segments[s].minFrames, 1);
	}
	const auto last = [&](size_t s) { return first[s] + Max<size_t>(segments[s].minFrames, 1) - 1; };

	Array<double> previous(stateCount, Infinity), current(stateCount);
	Array<double> entry(segments.size() + 1);
	bool started = false;
	const auto computeEntry = [&](const Array<double>& costs) {
		entry[0] = started ? Infinity : 0.0;
		for (size_t s : step(segments.size())) {
			entry[s + 1] = costs[last(s)];
			if (segments[s].skippable) entry[s + 1] = Min(entry[s + 1], entry[s]);
		}
	};

	for (const auto& frame : frames) {
		computeEntry(previous);
		started = true;
		for (size_t s : step(segments.size())) {
			const auto& segment = segments[s];
			double cost = 0.0;
			switch (segment.type) {
			case SegmentType::Vowel: cost = -log(frame[segment.label]); break;
			case SegmentType::Gap: cost = -log(frame[SilenceLabel]); break;
			case SegmentType::Edge: cost = Min(-log(frame[SilenceLabel]), options.fillerCost); break;
			case SegmentType::Join: cost = options.fillerCost; break;
			}
			const size_t length = last(s) - first[s] + 1;
			for (size_t j : step(length)) {
				double from = (j == 0) ? entry[s] : previous[first[s] + j - 1];
				if (j == length - 1) from = Min(from, previous[last(s)]);
				current[first[s] + j] = from + cost;
			}
		}
		swap(previous, current);
	}
	computeEntry(previous);
	return entry[segments.size()];
}
