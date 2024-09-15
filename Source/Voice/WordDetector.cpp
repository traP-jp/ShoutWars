# include "WordDetector.hpp"

using namespace std;

WordDetector::WordDetector(
	uint64 coolTime, uint64 wordTimeout, uint64 wordTimeLimit, uint64 scoresHistoryLife, double scoreThreshold
)
	: coolTime(coolTime), wordTimeout(wordTimeout), wordTimeLimit(wordTimeLimit), scoresHistoryLife(scoresHistoryLife),
	scoreThreshold(scoreThreshold), lastDetected(0uLL) {}

HashTable<char32, bool> WordDetector::addScores(HashTable<char32, double> scores) {
	const auto now = Time::GetMicrosec();
	scoresHistory << pair{ now, scores };
	if (now < lastDetected + coolTime) return {};
	const auto chances = vowelChances();
	vowelsBuffer << pair{ now, chances };
	//String chancesStr = U"";
	//for (const auto& [vowel, chance] : chances) chancesStr += U"{}={}, "_fmt(vowel, chance);
	//Print << U"time: {}us, chances: {}"_fmt(now, chancesStr);
	return chances;
}

bool WordDetector::detect(String word) {
	const auto now = Time::GetMicrosec();
	if (now < lastDetected + coolTime) return false;
	vowelsBuffer.remove_if([this, now](const auto& v) { return now - v.first > wordTimeLimit; });

	// Dynamic Programming
	Array<uint64> lastWordTimes(word.size() + 1, 4e18);
	lastWordTimes.back() = now;
	for (const auto& [time, vowels] : vowelsBuffer | views::reverse) {
		Array<uint64> currentWordTimes = lastWordTimes;
		for (size_t i : step(word.size())) {
			if (!vowels.contains(word[i])) continue;
			if (!vowels.at(word[i])) continue;
			if (min(lastWordTimes[i + 1], lastWordTimes[i]) <= time + wordTimeout) {
				if (i == 0) {
					// goal
					vowelsBuffer.clear();
					lastDetected = now;
					return true;
				}
				currentWordTimes[i] = min(currentWordTimes[i], time);
			}
		}
		lastWordTimes.swap(currentWordTimes);
	}
	return false;
}

HashTable<char32, bool> WordDetector::vowelChances() {
	const auto now = Time::GetMicrosec();
	scoresHistory.remove_if([this, now](const auto& s) { return now - s.first > scoresHistoryLife; });

	HashTable<char32, double> areas;
	for (size_t i : step(scoresHistory.size())) {
		const auto dt = (i + 1 < scoresHistory.size() ? scoresHistory[i + 1].first : now) - scoresHistory[i].first;
		for (const auto& [vowel, score] : scoresHistory[i].second) areas[vowel] += score * dt;
	}

	HashTable<char32, bool> chances;
	for (const auto& [vowel, area] : areas) chances[vowel] = area >= scoreThreshold * scoresHistoryLife;
	return chances;
}

Array<std::pair<uint64, HashTable<char32, bool>>> WordDetector::getVowelsBuffer(uint64 limit) const {
	if (limit == 0) return vowelsBuffer;
	const auto now = Time::GetMicrosec();
	Array<std::pair<uint64, HashTable<char32, bool>>> result;
	for (const auto& [time, vowels] : vowelsBuffer) {
		if (now - time <= limit) result << pair{ time, vowels };
	}
	return result;
}
