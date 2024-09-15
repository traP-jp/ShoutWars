# include "MFCC.hpp"

using namespace std;

bool MFCC::isUnset() const {
	return ranges::all_of(feature, [](double x) { return x == 0.0; });
}

double MFCC::norm() const {
	return sqrt(accumulate(feature.begin(), feature.end(), 0.0, [](const auto& norm, const auto& x) { return norm + x * x; }));
}

double MFCC::cosineSimilarity(const MFCC& other) const {
	if (feature.size() != other.feature.size()) throw Error{ U"MFCC order mismatch" };
	const double thisNorm = norm(), otherNorm = other.norm();
	if (thisNorm < 1e-8 || otherNorm < 1e-8) return 0.0;
	double innerProduct = 0.0;
	for (size_t i : step(feature.size())) innerProduct += feature[i] * other.feature[i];
	return innerProduct / thisNorm / otherNorm;
}
