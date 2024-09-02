# pragma once

# include "common.hpp"
# include "Voice/FormantAnalyzer.hpp"

class Calibration : public App::Scene {
	Microphone mic;
	std::unique_ptr<FormantAnalyzer> formantAnalyzer;

public:
	Calibration(const InitData& init);

	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override;
	void drawFadeOut(double t) const override;
};
