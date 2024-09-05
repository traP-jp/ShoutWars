# pragma once

# include "common.hpp"
# include "Voice/MFCCAnalyzer.hpp"

class Calibration : public App::Scene {
	Microphone mic;
	FFTResult fftResult;
	std::unique_ptr<MFCCAnalyzer> mfccAnalyzer;
	Array<float> mfcc;

public:
	Calibration(const InitData& init);

	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override;
	void drawFadeOut(double t) const override;
};
