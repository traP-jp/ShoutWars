# pragma once

# include "common.hpp"

class Calibration : public App::Scene {
	FFTResult fftResult;
	Array<double> phonemeScores = Array<double>(12, 0.0);
	bool isWaitingToSet = false;

	Font font{ FontMethod::MSDF, 80 };

	Array<RectF> phonemeRects;
	Array<String> phonemeNames = {
		U"無音", U"鼻息", U"あ (高)", U"あ (低)", U"い (高)", U"い (低)",
		U"う (高)", U"う (低)", U"え (高)", U"え (低)", U"お (高)", U"お (低)",
	};

	RectF okButton{ Arg::topRight(1880, 40), 200, 70 };

public:
	Calibration(const InitData& init);
	~Calibration();

	void update() override;
	void draw() const override;
};
