# include "CommandRecognizer.hpp"

# include <ranges>

using namespace std;

namespace {
	// 音素: [0:無, 1:息, 2:ア, 3:あ, 4:イ, 5:い, 6:ウ, 7:う, 8:エ, 9:え, 10:オ, 11:お]
	constexpr array<char32, 12> PhonemeVowels = { U' ', U' ', U'A', U'A', U'I', U'I', U'U', U'U', U'E', U'E', U'O', U'O' };

	const Array<VoiceCommand> CommonCommands = {
		{ U"ガード", U"AO", 4 },
		{ U"守れ", U"AOE", 4 }, // ガードに吸われる
		{ U"壊せ", U"OAE", 5 },
	};
}

Array<VoiceCommand> VoiceCommandsOf(int32 character) {
	Array<VoiceCommand> commands;
	switch (character) {
	case 0:
		commands = {
			{ U"殴れ", U"AUE", 1 },
			{ U"キック", U"IU", 2 },
			{ U"龍虎水雷撃", U"AIEI", 3 }, // UOUIAIEI
			{ U"刺せ", U"AE", 5 },
			{ U"魚雷", U"OAI", 6 },
		};
		break;
	case 2:
		commands = {
			{ U"撃て", U"UE", 1 },
			{ U"斬れ", U"IE", 2 },
			{ U"デッドリーアサルト", U"IAUO", 3 }, // EOIAUO
			{ U"刺せ", U"AE", 5 },
			{ U"連射", U"EIA", 6 },
		};
		break;
	default:
		commands = {
			{ U"殴れ", U"AUE", 1 },
			{ U"キック", U"IU", 2 },
			{ U"龍虎水雷撃", U"AIEI", 3 }, // UOUIAIEI
			{ U"刺せ", U"AE", 5 },
		};
		break;
	}
	return commands.append(CommonCommands);
}

int32 CommandRecognizer::update(const Array<double>& phonemeScores, int32 character, uint64 timeUs) {
	HashTable<char32, double> scores;
	for (const auto& [id, score] : phonemeScores | views::enumerate) {
		const auto vowel = PhonemeVowels[id];
		if (vowel != U' ') scores[vowel] = max(scores[vowel], pow(score, 2) * (score >= 0.0 ? 1.0 : -1.0));
	}
	wordDetector.addScores(scores, timeUs);
	for (const auto& command : VoiceCommandsOf(character)) {
		if (wordDetector.detect(String{ command.vowels }, timeUs)) return command.action;
	}
	return 0;
}
