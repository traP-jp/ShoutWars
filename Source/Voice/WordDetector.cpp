# include "WordDetector.hpp"

using namespace std;

WordDetector::WordDetector(uint64 vowelHistoryLife) : vowelHistoryLife(vowelHistoryLife) {}

char32 WordDetector::addVowel(char32 vowel) {
	vowelHistory << pair{ Time::GetMicrosec(), vowel };
	const auto peakVowel = peak();
	if (vowelBuffer.empty() || vowelBuffer.back() != peakVowel) {
		if (peakVowel != U' ') vowelBuffer << peakVowel;
	}
	return peakVowel;
}

bool WordDetector::detect(String word) {
	if (vowelBuffer.ends_with(word)) {
		vowelBuffer.clear();
		return true;
	}
	return false;
}

char32 WordDetector::peak() {
	const auto now = Time::GetMicrosec();
	vowelHistory.remove_if([this, now](const auto& v) { return now - v.first > vowelHistoryLife; });

	HashTable<char32, uint64> vowelTimes;
	for (size_t i : step(vowelHistory.size())) {
		vowelTimes[vowelHistory[i].second] +=
			(i + 1 < vowelHistory.size() ? vowelHistory[i + 1].first : now) - vowelHistory[i].first;
	}

	char32 peakVowel = U' ';
	vowelTimes[peakVowel] = 0;
	for (const auto& [vowel, time] : vowelTimes) {
		if (time >= vowelTimes[peakVowel]) peakVowel = vowel;
	}
	return peakVowel;
}

StringView WordDetector::getVowelBuffer(size_t limit) const {
	return vowelBuffer.substrView(max(vowelBuffer.size(), limit) - limit);
}
