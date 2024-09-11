# include "MFCC.hpp"

using namespace std;

bool MFCC::isUnset() const {
	return ranges::any_of(feature, [](float x) { return x != 0.0f; });
}

float MFCC::norm() const {
	return sqrt(accumulate(feature.begin(), feature.end(), 0.0f, [](const auto& norm, const auto& x) { return norm + x * x; }));
}

float MFCC::cosineSimilarity(const MFCC& other) const {
	if (feature.size() != other.feature.size()) throw Error{ U"MFCC order mismatch" };
	const float thisNorm = norm(), otherNorm = other.norm();
	if (thisNorm < 1e-8 || otherNorm < 1e-8) return 0.0f;
	float innerProduct = 0.0f;
	for (size_t i : step(feature.size())) innerProduct += feature[i] * other.feature[i];
	return innerProduct / thisNorm / otherNorm;
}
