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
		{ U"壊せ", U"OAE", 5 },
		{ U"刺せ", U"AE", 5 },
	};

	Optional<size_t> VowelLabel(char32 vowel) {
		const auto index = StringView{ U"AIUEO" }.indexOf(ToUpper(vowel));
		return index == StringView::npos ? none : Optional<size_t>{ index };
	}
}

Array<VoiceCommand> VoiceCommandsOf(int32 character) {
	Array<VoiceCommand> commands;
	switch (character) {
	case 0:
		commands = {
			{ U"殴れ", U"AUE", 1 },
			{ U"キック", U"Iu", 2 },
			{ U"龍虎水雷撃", U"UOuIAIEi", 3 },
			{ U"魚雷", U"OAI", 6 },
		};
		break;
	case 2:
		commands = {
			{ U"撃て", U"UE", 1 },
			{ U"斬れ", U"IE", 2 },
			{ U"デッドリーアサルト", U"EOIAuO", 3 },
			{ U"連射", U"EiA", 6 },
		};
		break;
	default:
		commands = {
			{ U"殴れ", U"AUE", 1 },
			{ U"キック", U"Iu", 2 },
			{ U"龍虎水雷撃", U"UOuIAIEi", 3 },
		};
		break;
	}
	return commands.append(CommonCommands);
}

CommandRecognizer::CommandRecognizer(const CommandRecognizerOptions& options) : options(options) {}

int32 CommandRecognizer::update(const Array<double>& phonemeScores, int32 character, uint64 timeUs) {
	if (timeUs + FrameIntervalUs / 2 < nextFrameUs) return 0;
	nextFrameUs = (nextFrameUs + FrameIntervalUs * 4 < timeUs) ? timeUs + FrameIntervalUs : nextFrameUs + FrameIntervalUs;

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
	if (silentFrames < options.endSilenceFrames) return 0;

	utterance.resize(utterance.size() - silentFrames);
	const int32 action = (voicedFrames >= options.minVoicedFrames) ? decide(character) : 0;
	utterance.clear();
	voicedFrames = 0;
	silentFrames = 0;
	return action;
}

int32 CommandRecognizer::decide(int32 character) const {
	double freeCost = 0.0;
	for (const auto& frame : utterance) freeCost += -log(*max_element(frame.begin(), frame.end()));

	struct Candidate {
		const VoiceCommand* command;
		double cost;
		size_t length;
	};
	const auto commands = VoiceCommandsOf(character);
	Array<Candidate> candidates;
	for (const auto& command : commands) {
		const double cost = (alignmentCost(command.vowels) - freeCost) / utterance.size();
		const size_t length = count_if(command.vowels.begin(), command.vowels.end(), [](char32 c) { return IsUpper(c); });
		candidates << Candidate{ &command, cost, length };
	}
	const auto best = ranges::min_element(candidates, {}, &Candidate::cost);
	const Candidate* chosen = &*best;
	for (const auto& candidate : candidates) {
		if (candidate.length > chosen->length && candidate.cost <= best->cost + options.longerPreference) chosen = &candidate;
	}
	const double threshold = (chosen->command->action == 3) ? options.specialThreshold : options.threshold;
	return (chosen->cost <= threshold) ? chosen->command->action : 0;
}

double CommandRecognizer::alignmentCost(StringView vowels) const {
	Array<size_t> labels;
	Array<bool> optional;
	for (const char32 vowel : vowels) {
		if (const auto label = VowelLabel(vowel)) {
			labels << *label;
			optional << IsLower(vowel);
		}
	}
	const size_t n = labels.size();
	const size_t d = Max<size_t>(options.minVowelFrames, 1);
	// 状態: 母音 i の前の「どれにも当てはめない」状態 F(i) と、母音 i の d 段の継続 V(i, j)
	const auto F = [d](size_t i) { return i * (d + 1); };
	const auto V = [d](size_t i, size_t j) { return i * (d + 1) + 1 + j; };

	Array<double> previous(n * (d + 1) + 1, Infinity), current(previous.size());
	Array<double> entry(n + 1);
	previous[F(0)] = 0.0;
	const auto computeEntry = [&](const Array<double>& costs) {
		for (size_t i : step(n + 1)) {
			entry[i] = costs[F(i)];
			if (i > 0) entry[i] = Min(entry[i], costs[V(i - 1, d - 1)]);
			if (i > 0 && optional[i - 1]) entry[i] = Min(entry[i], entry[i - 1]);
		}
	};

	for (const auto& frame : utterance) {
		computeEntry(previous);
		const double filler = Min(-log(frame[SilenceLabel]), options.fillerCost);
		for (size_t i : step(n + 1)) current[F(i)] = entry[i] + filler;
		for (size_t i : step(n)) {
			const double cost = -log(frame[labels[i]]);
			for (size_t j : step(d)) {
				double from = (j == 0) ? entry[i] : previous[V(i, j - 1)];
				if (j == d - 1) from = Min(from, previous[V(i, j)]);
				current[V(i, j)] = from + cost;
			}
		}
		swap(previous, current);
	}
	computeEntry(previous);
	return entry[n];
}
