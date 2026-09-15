# pragma once

# include "common.hpp"
# include "GlowBorder.hpp"

class Calibration : public App::Scene {
public:
	Calibration(const InitData& init);
	~Calibration() override;

	void update() override;
	void draw() const override;

private:
	struct Bar {
		double volumeDb;
		MFCC mfcc;
	};

	/// @brief 録音中の段階。グラフの棒 1 本が、録音に数えた 1 フレームにあたる
	struct Take {
		Array<Bar> bars;
		/// @brief 区間ごとの、登録するフレームのメルスペクトル
		Array<Array<Array<double>>> spectra;
	};

	FFTResult fftResult;
	Array<double> phonemeScores = Array<double>(12, 0.0);

	Font font{ FontMethod::MSDF, 80 };
	const Texture returnImage{ Resource(U"images/common/return.png") };
	const Audio cancelSound{ Resource(U"audioes/cancel.wav") };
	GlowBorder returnGlow;
	bool isReturnHovered = false;

	Optional<size_t> selectedStep;
	bool isRecording = false;
	/// @brief 「キャリブレーションにすすむ」から、段階を順に自動で選んでいる途中か
	bool isGuided = false;
	/// @brief 順に進めている途中で、何段階目まで録り終えたか
	size_t guidedSteps = 0;
	/// @brief 段階ごとの、登録済みの録音のグラフ
	Array<Array<Bar>> graphs;
	Take recording;

	void recordFrame();
	void finishTake();
	void pressButton();
	void close();

	[[nodiscard]] bool isComplete() const;
	[[nodiscard]] StringView buttonLabel() const;
	[[nodiscard]] String guideMessage() const;
	[[nodiscard]] String recognitionLabel() const;
	void drawSensitivity() const;
	void drawStep(size_t step) const;

	[[nodiscard]] static Array<Array<Bar>> LoadGraphs(FilePathView configPath);
	static void SaveGraphs(FilePathView configPath, const Array<Array<Bar>>& graphs);
};
