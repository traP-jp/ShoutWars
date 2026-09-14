# include <Siv3D.hpp>
# include <algorithm>
# include <random>
# include <ranges>
# include "Voice/CommandRecognizer.hpp"
# include "Voice/Phoneme.hpp"

SIV3D_SET(EngineOption::Renderer::Headless);

// 録音コーパスに環境音を混ぜ、ゲームと同じ 60 fps の呼び出しを再現して音声認識を評価する。
// 使い方: VoiceEval.exe <コーパスのフォルダ> <環境音の WAV> <出力フォルダ> [key=value ...]
// takes=1,2,3 (キャリブレーションに使う回), mode=all|vowels, k, standardize, hamming, fmin, fmax, mel, order, preemph,
// 単語判定: end, early, earlythr, merge, cooldown, minvoiced, minvowel, mingap, filler, thr, ambiguity, longer (CommandRecognizerOptions)
// conditions=clean,snr10 (評価する条件), mode=dump (フレームごとのスコアを frames.csv に書き出す)
// margin (入力感度の閾値より何 dB 大きければ無音に分類しないか)
// gate (入力感度を環境音の音量の何倍にするか), silence / vowel (無音 / 母音を何秒登録するか), distance=cosine|euclidean

namespace {
	constexpr uint32 SampleRate = 48000;
	constexpr size_t Hop = SampleRate / 60;
	constexpr size_t WindowLength = 2048;
	constexpr size_t RMSLength = SampleRate / 50;
	constexpr uint64 ClockOffsetUs = 10'000'000;
	constexpr double DetectionGraceSeconds = 0.8;

	// ゲームのキャリブレーション画面の並び: [0:無, 1:息, 2:あ(高), 3:あ(低), ...]
	constexpr std::array<char32, 12> PhonemeVowels = { U' ', U' ', U'A', U'A', U'I', U'I', U'U', U'U', U'E', U'E', U'O', U'O' };
	const Array<std::pair<String, char32>> CorpusVowels = { { U"a", U'A' }, { U"i", U'I' }, { U"u", U'U' }, { U"e", U'E' }, { U"o", U'O' } };
	const Array<char32> VowelLabels = { U'A', U'I', U'U', U'E', U'O', U' ' };

	struct Take {
		String file;
		String group;
		String line;
		String text;
		String style;
		int32 take = 0;
		Array<float> samples;
		size_t activeBegin = 0;
		size_t activeEnd = 0;
		double level = 0.0;
	};

	struct Condition {
		String name;
		Optional<double> snrDb;
	};

	struct Noise {
		Array<float> samples;
		size_t calibrationEnd = 0;
		double level = 0.0;
	};

	size_t Seconds(double seconds) {
		return static_cast<size_t>(seconds * SampleRate);
	}

	uint64 ClockUs(size_t pos) {
		return ClockOffsetUs + pos * 1'000'000uLL / SampleRate;
	}

	double RootMeanSquare(const float* samples, size_t length) {
		double sum = 0.0;
		for (size_t i : step(length)) sum += samples[i] * samples[i];
		return length ? Math::Sqrt(sum / length) : 0.0;
	}

	Array<float> LoadMono(FilePathView path) {
		const Wave wave{ path };
		if (wave.isEmpty()) throw Error{ U"WAV を読めません: {}"_fmt(path) };
		if (wave.sampleRate() != SampleRate) throw Error{ U"{} Hz 以外は未対応です: {}"_fmt(SampleRate, path) };
		return Array<float>(wave.size(), Arg::generator = [&, i = size_t{ 0 }]() mutable { return wave[i++].left; });
	}

	void MeasureActivity(Take& take) {
		const size_t frame = SampleRate / 100;
		Array<double> rms;
		for (size_t pos = 0; pos + frame <= take.samples.size(); pos += frame) rms << RootMeanSquare(&take.samples[pos], frame);
		const double peak = rms.empty() ? 0.0 : *std::ranges::max_element(rms);
		const double threshold = Max(peak * 0.03, 0.003);
		const auto first = std::ranges::find_if(rms, [&](double x) { return x >= threshold; });
		if (first == rms.end()) throw Error{ U"声が入っていません: {}"_fmt(take.file) };
		size_t last = rms.size();
		while (rms[last - 1] < threshold) --last;
		take.activeBegin = (first - rms.begin()) * frame;
		take.activeEnd = last * frame;
		double sum = 0.0;
		size_t count = 0;
		for (double x : rms) {
			if (x >= peak * 0.1) {
				sum += x * x;
				++count;
			}
		}
		take.level = Math::Sqrt(sum / count);
	}

	Array<Take> LoadCorpus(FilePathView directory) {
		const CSV manifest{ FileSystem::PathAppend(directory, U"manifest.csv") };
		if (!manifest) throw Error{ U"manifest.csv を読めません: {}"_fmt(directory) };
		const auto header = manifest.getRow(0);
		const auto column = [&](StringView name) {
			const auto it = std::ranges::find(header, name);
			if (it == header.end()) throw Error{ U"manifest.csv に {} 列がありません"_fmt(name) };
			return static_cast<size_t>(it - header.begin());
		};
		Array<Take> takes;
		for (size_t row : Range(1, manifest.rows() - 1)) {
			if (manifest.columns(row) < header.size()) continue;
			Take take{
				.file = manifest.get(row, column(U"file")),
				.group = manifest.get(row, column(U"group")),
				.line = manifest.get(row, column(U"line")),
				.text = manifest.get(row, column(U"text")),
				.style = manifest.get(row, column(U"style")),
				.take = Parse<int32>(manifest.get(row, column(U"take"))),
			};
			take.samples = LoadMono(FileSystem::PathAppend(directory, take.file));
			MeasureActivity(take);
			takes << std::move(take);
		}
		return takes;
	}

	Array<float> ExtractFloor(const Array<Take>& takes) {
		const size_t length = Seconds(0.3);
		const auto quietest = std::ranges::min_element(takes, {}, [&](const Take& take) {
			return take.activeBegin < length ? 1.0 : RootMeanSquare(take.samples.data(), length);
		});
		return Array<float>(quietest->samples.begin(), quietest->samples.begin() + length);
	}

	class StreamBuilder {
	public:
		StreamBuilder(const Array<float>& floor, size_t length) : samples(length) {
			for (size_t i : step(length)) samples[i] = floor[i % floor.size()];
		}

		void place(const Array<float>& clip, size_t pos) {
			for (size_t i : step(Min(clip.size(), samples.size() - pos))) samples[pos + i] += clip[i];
		}

		void addNoise(const Noise& noise, bool calibration, double gain, std::mt19937& rng) {
			const size_t begin = calibration ? 0 : noise.calibrationEnd;
			const size_t end = calibration ? noise.calibrationEnd : noise.samples.size();
			size_t offset = std::uniform_int_distribution<size_t>{ begin, end - 1 }(rng);
			for (auto& sample : samples) {
				sample += static_cast<float>(noise.samples[offset] * gain);
				if (++offset == end) offset = begin;
			}
		}

		Array<float> build() {
			for (auto& sample : samples) sample = Clamp(sample, -1.0f, 1.0f);
			return std::move(samples);
		}

	private:
		Array<float> samples;
	};

	template <class F>
	void ForEachFrame(const Array<float>& stream, size_t begin, size_t end, F&& onFrame) {
		for (size_t pos = Max(begin, WindowLength); pos <= Min(end, stream.size()); pos += Hop) {
			Array<float> window(stream.begin() + (pos - WindowLength), stream.begin() + pos);
			const double rms = RootMeanSquare(&stream[pos - RMSLength], RMSLength);
			onFrame(pos, std::move(window), rms);
		}
	}

	double NoiseGain(const Noise& noise, const Condition& condition, double speechLevel) {
		return condition.snrDb ? speechLevel / noise.level * Math::Pow(10.0, -*condition.snrDb / 20.0) : 0.0;
	}

	const Take& FindTake(const Array<Take>& takes, StringView group, StringView line, StringView style, int32 number) {
		const auto it = std::ranges::find_if(takes, [&](const Take& take) {
			return take.group == group && take.line == line && take.style == style && take.take == number;
		});
		if (it == takes.end()) throw Error{ U"録音がありません: {}-{}-{}-{}"_fmt(group, line, style, number) };
		return *it;
	}

	struct CalibrationOptions {
		double gateScale = 1.25;
		double silenceSeconds = 1.0;
		double vowelSeconds = 1.0;
	};

	/// @brief キャリブレーション画面での長押し登録を、同じ環境音の中で再現する
	Phoneme Calibrate(const Array<Take>& takes, int32 number, const Array<float>& floor, const Noise& noise, double gain, const PhonemeOptions& options, const CalibrationOptions& calibration, std::mt19937& rng) {
		Array<std::pair<const Array<float>*, size_t>> clips;
		Array<std::pair<size_t, uint64>> registrations;
		size_t length = Seconds(0.5);
		const Array<float> silence(Seconds(calibration.silenceSeconds + 0.7), 0.0f);
		for (size_t id : step(PhonemeVowels.size())) {
			size_t center = length + silence.size() / 2;
			if (id < 2) {
				clips.emplace_back(&silence, length);
				length += silence.size();
			}
			else {
				const auto& [line, vowel] = CorpusVowels[(id - 2) / 2];
				const Take& take = FindTake(takes, U"vowel", line, StringView{ id % 2 == 0 ? U"high" : U"low" }, number);
				center = length + (take.activeBegin + take.activeEnd) / 2;
				clips.emplace_back(&take.samples, length);
				length += take.samples.size();
			}
			registrations.emplace_back(id < 2 ? length - Seconds(0.3) : center + Seconds(calibration.vowelSeconds / 2), id);
			length += Seconds(0.3);
		}

		StreamBuilder builder{ floor, length };
		for (const auto& [clip, pos] : clips) builder.place(*clip, pos);
		if (gain > 0.0) builder.addNoise(noise, true, gain, rng);
		const auto stream = builder.build();

		Phoneme phoneme{ U"", 0.01, PhonemeVowels.size(), 2'200'000uLL, options };
		size_t next = 0;
		ForEachFrame(stream, 0, stream.size(), [&](size_t pos, Array<float> window, double rms) {
			(void)phoneme.estimate(std::move(window), SampleRate, rms, ClockUs(pos));
			while (next < registrations.size() && registrations[next].first <= pos) {
				const size_t id = registrations[next].second;
				phoneme.setMFCC(id, ClockUs(pos), static_cast<uint64>((id < 2 ? calibration.silenceSeconds : calibration.vowelSeconds) * 1'000'000));
				++next;
			}
		});

		// 入力感度は、環境音がほぼ閾値を下回るように合わせてもらう想定
		if (gain > 0.0) {
			Array<double> calibrationNoise;
			const auto noiseOnly = [&] {
				StreamBuilder b{ floor, Seconds(10.0) };
				b.addNoise(noise, true, gain, rng);
				return b.build();
			}();
			ForEachFrame(noiseOnly, 0, noiseOnly.size(), [&](size_t, Array<float>, double rms) { calibrationNoise << rms; });
			std::ranges::sort(calibrationNoise);
			phoneme.volumeThreshold = Max(0.01, calibrationNoise[calibrationNoise.size() * 95 / 100] * calibration.gateScale);
		}
		return phoneme;
	}

	struct VowelResult {
		std::array<std::array<size_t, 6>, 5> confusion{};
		size_t noiseFrames = 0;
		size_t noiseAsVowel = 0;

		[[nodiscard]] double accuracy() const {
			size_t correct = 0, total = 0;
			for (size_t i : step(5)) {
				correct += confusion[i][i];
				for (size_t count : confusion[i]) total += count;
			}
			return total ? static_cast<double>(correct) / total : 0.0;
		}
	};

	char32 PredictVowel(const Array<double>& scores) {
		return PhonemeVowels[std::ranges::max_element(scores) - scores.begin()];
	}

	VowelResult EvaluateVowels(const Array<Take>& takes, Phoneme& phoneme, int32 calibrationTake, const Array<float>& floor, const Noise& noise, double gain, std::mt19937& rng) {
		VowelResult result;
		const size_t trim = Seconds(0.06);
		for (const Take& take : takes) {
			if (take.group != U"vowel" || take.take == calibrationTake) continue;
			const size_t truth = std::ranges::find(CorpusVowels, take.line, &std::pair<String, char32>::first) - CorpusVowels.begin();
			const size_t lead = Seconds(0.5);
			StreamBuilder builder{ floor, lead + take.samples.size() };
			builder.place(take.samples, lead);
			if (gain > 0.0) builder.addNoise(noise, false, gain, rng);
			const auto stream = builder.build();
			ForEachFrame(stream, 0, stream.size(), [&](size_t pos, Array<float> window, double rms) {
				const auto scores = phoneme.estimate(std::move(window), SampleRate, rms, ClockUs(pos));
				const size_t center = pos - WindowLength / 2;
				if (center < lead + take.activeBegin + trim || lead + take.activeEnd < center + trim) return;
				const size_t predicted = std::ranges::find(VowelLabels, PredictVowel(scores)) - VowelLabels.begin();
				++result.confusion[truth][predicted];
			});
		}

		StreamBuilder builder{ floor, Seconds(30.0) };
		if (gain > 0.0) builder.addNoise(noise, false, gain, rng);
		const auto stream = builder.build();
		ForEachFrame(stream, 0, stream.size(), [&](size_t pos, Array<float> window, double rms) {
			const auto scores = phoneme.estimate(std::move(window), SampleRate, rms, ClockUs(pos));
			++result.noiseFrames;
			if (PredictVowel(scores) != U' ') ++result.noiseAsVowel;
		});
		return result;
	}

	/// @brief コマンドとコマンドでない声の各フレームの音素スコアを書き出す
	void DumpFrames(const Array<Take>& takes, Phoneme& phoneme, const Array<float>& floor, const Noise& noise, double gain, std::mt19937& rng, TextWriter& writer, StringView prefix) {
		for (const Take& take : takes) {
			if (take.group == U"vowel") continue;
			const size_t lead = Seconds(0.5);
			StreamBuilder builder{ floor, lead + take.samples.size() };
			builder.place(take.samples, lead);
			if (gain > 0.0) builder.addNoise(noise, false, gain, rng);
			const auto stream = builder.build();
			ForEachFrame(stream, 0, stream.size(), [&](size_t pos, Array<float> window, double rms) {
				const auto scores = phoneme.estimate(std::move(window), SampleRate, rms, ClockUs(pos));
				const double ms = (static_cast<double>(pos) - static_cast<double>(lead + take.activeBegin)) * 1000.0 / SampleRate;
				const bool active = lead + take.activeBegin <= pos && pos - WindowLength / 2 <= lead + take.activeEnd;
				writer << U"{},{},{:.0f},{},{}"_fmt(prefix, take.file, ms, active ? 1 : 0, scores.map([](double s) { return U"{:.3f}"_fmt(s); }).join(U",", U"", U""));
			});
		}
	}

	struct LineResult {
		size_t count = 0;
		size_t hit = 0;
		size_t wrong = 0;
	};

	struct CommandResult {
		size_t expected = 0;
		size_t hit = 0;
		size_t wrong = 0;
		size_t falseBySpeech = 0;
		size_t falseByNoise = 0;
		double minutes = 0.0;
		Array<double> latencyMs;
		std::map<std::pair<String, String>, LineResult> lines;
		Array<String> takeDetections;
	};

	int32 ExpectedAction(const Take& take, int32 character) {
		for (const auto& command : VoiceCommandsOf(character)) {
			if (command.text == take.text) return command.action;
		}
		return 0;
	}

	CommandResult EvaluateCommands(const Array<Take>& takes, Phoneme& phoneme, int32 character, const CommandRecognizerOptions& recognizerOptions, const Array<float>& floor, const Noise& noise, double gain, std::mt19937& rng) {
		Array<const Take*> order;
		for (const Take& take : takes) {
			if (take.group != U"vowel") order << &take;
		}
		std::ranges::shuffle(order, rng);

		Array<size_t> offsets;
		size_t length = Seconds(2.0);
		for (const Take* take : order) {
			offsets << length;
			length += take->samples.size() + Seconds(std::uniform_real_distribution<double>{ 0.8, 1.6 }(rng));
		}
		StreamBuilder builder{ floor, length };
		for (size_t i : step(order.size())) builder.place(order[i]->samples, offsets[i]);
		if (gain > 0.0) builder.addNoise(noise, false, gain, rng);
		const auto stream = builder.build();

		CommandRecognizer recognizer{ recognizerOptions };
		Array<std::pair<size_t, int32>> detections;
		ForEachFrame(stream, 0, stream.size(), [&](size_t pos, Array<float> window, double rms) {
			const auto scores = phoneme.estimate(std::move(window), SampleRate, rms, ClockUs(pos));
			if (const int32 action = recognizer.update(scores, character, ClockUs(pos))) detections.emplace_back(pos, action);
		});

		CommandResult result;
		result.minutes = static_cast<double>(stream.size()) / SampleRate / 60.0;
		Array<bool> used(detections.size(), false);
		for (size_t i : step(order.size())) {
			const Take& take = *order[i];
			const size_t begin = offsets[i] + take.activeBegin;
			size_t end = offsets[i] + take.activeEnd + Seconds(DetectionGraceSeconds);
			if (i + 1 < order.size()) end = Min(end, offsets[i + 1] + order[i + 1]->activeBegin);
			const int32 expected = ExpectedAction(take, character);
			auto& line = result.lines[{ take.text, take.style }];
			if (expected) {
				++result.expected;
				++line.count;
			}
			bool judged = false;
			Array<int32> detected;
			for (size_t d : step(detections.size())) {
				const auto [pos, action] = detections[d];
				if (pos < begin || end <= pos) continue;
				used[d] = true;
				detected << action;
				if (!expected || judged) {
					++result.falseBySpeech;
					continue;
				}
				judged = true;
				if (action == expected) {
					++result.hit;
					++line.hit;
					result.latencyMs << (static_cast<double>(pos) - static_cast<double>(offsets[i] + take.activeEnd)) * 1000.0 / SampleRate;
				}
				else {
					++result.wrong;
					++line.wrong;
				}
			}
			result.takeDetections << U"{},{},{}"_fmt(take.file, expected, detected.join(U" ", U"", U""));
		}
		result.falseByNoise = used.count(false);
		return result;
	}

	double Median(Array<double> values) {
		if (values.empty()) return Math::NaN;
		std::ranges::sort(values);
		return values[values.size() / 2];
	}

	void Run(const Array<String>& args) {
		if (args.size() < 4) throw Error{ U"使い方: VoiceEval.exe <コーパスのフォルダ> <環境音の WAV> <出力フォルダ> [key=value ...]" };
		const FilePath outDirectory = args[3];
		FileSystem::CreateDirectories(outDirectory);
		Array<int32> calibrationTakes = { 1, 2, 3 };
		bool evaluateCommands = true;
		bool dumpFrames = false;
		Array<String> conditionNames;
		PhonemeOptions options;
		CalibrationOptions calibration;
		CommandRecognizerOptions recognizerOptions;
		for (const auto& arg : args.slice(4)) {
			const auto pair = arg.split(U'=');
			if (pair.size() != 2) throw Error{ U"key=value の形で指定してください: {}"_fmt(arg) };
			const auto& [key, value] = std::pair{ pair[0], pair[1] };
			if (key == U"takes") calibrationTakes = value.split(U',').map([](const String& s) { return Parse<int32>(s); });
			else if (key == U"mode") {
				evaluateCommands = (value == U"all");
				dumpFrames = (value == U"dump");
			}
			else if (key == U"conditions") conditionNames = value.split(U',');
			else if (key == U"k") options.k = Parse<size_t>(value);
			else if (key == U"standardize") options.standardize = Parse<bool>(value);
			else if (key == U"hamming") options.mfcc.hammingWindow = Parse<bool>(value);
			else if (key == U"fmin") options.mfcc.minFrequency = Parse<double>(value);
			else if (key == U"fmax") options.mfcc.maxFrequency = Parse<double>(value);
			else if (key == U"mel") options.mfcc.melChannels = Parse<size_t>(value);
			else if (key == U"order") options.mfcc.order = Parse<size_t>(value);
			else if (key == U"preemph") options.mfcc.preEmphasisCoefficient = Parse<double>(value);
			else if (key == U"margin") options.silenceMarginDb = Parse<double>(value);
			else if (key == U"gate") calibration.gateScale = Parse<double>(value);
			else if (key == U"silence") calibration.silenceSeconds = Parse<double>(value);
			else if (key == U"vowel") calibration.vowelSeconds = Parse<double>(value);
			else if (key == U"end") recognizerOptions.endSilenceFrames = Parse<size_t>(value);
			else if (key == U"minvoiced") recognizerOptions.minVoicedFrames = Parse<size_t>(value);
			else if (key == U"minvowel") recognizerOptions.minVowelFrames = Parse<size_t>(value);
			else if (key == U"mingap") recognizerOptions.minGapFrames = Parse<size_t>(value);
			else if (key == U"cooldown") recognizerOptions.cooldownFrames = Parse<size_t>(value);
			else if (key == U"merge") recognizerOptions.mergeSilenceFrames = Parse<size_t>(value);
			else if (key == U"early") recognizerOptions.earlySilenceFrames = Parse<size_t>(value);
			else if (key == U"earlythr") recognizerOptions.earlyThreshold = Parse<double>(value);
			else if (key == U"filler") recognizerOptions.fillerCost = Parse<double>(value);
			else if (key == U"thr") recognizerOptions.threshold = Parse<double>(value);
			else if (key == U"ambiguity") recognizerOptions.ambiguityMargin = Parse<double>(value);
			else if (key == U"longer") recognizerOptions.longerPreference = Parse<double>(value);
			else if (key == U"distance") options.distance = (value == U"cosine" ? PhonemeDistance::Cosine : PhonemeDistance::Euclidean);
			else throw Error{ U"不明なオプションです: {}"_fmt(key) };
		}
		TextWriter{ FileSystem::PathAppend(outDirectory, U"options.txt") } << args.slice(4).join(U" ", U"", U"");

		const Stopwatch stopwatch{ StartImmediately::Yes };
		const auto takes = LoadCorpus(args[1]);
		const auto floor = ExtractFloor(takes);
		Noise noise{ .samples = LoadMono(args[2]) };
		noise.calibrationEnd = noise.samples.size() / 2;
		noise.level = RootMeanSquare(noise.samples.data(), noise.samples.size());
		Array<double> commandLevels;
		for (const Take& take : takes) {
			if (take.group == U"command") commandLevels << take.level;
		}
		const double speechLevel = Median(commandLevels);

		const Array<Condition> conditions = {
			{ U"clean", none }, { U"snr30", 30.0 }, { U"snr20", 20.0 }, { U"snr10", 10.0 }, { U"snr5", 5.0 }, { U"snr0", 0.0 },
		};
		const Array<std::pair<int32, String>> characters = { { 1, U"yuuka" }, { 2, U"airi" } };

		TextWriter results{ FileSystem::PathAppend(outDirectory, U"results.csv") };
		results << U"condition,calibration_take,character,vowel_accuracy,noise_as_vowel,expected,hit,wrong,miss,false_by_speech,false_by_noise,false_per_minute,latency_median_ms";
		TextWriter detectionsWriter{ FileSystem::PathAppend(outDirectory, U"detections.csv") };
		detectionsWriter << U"condition,calibration_take,character,file,expected,detected";
		TextWriter lines{ FileSystem::PathAppend(outDirectory, U"lines.csv") };
		lines << U"condition,calibration_take,character,text,style,count,hit,wrong";
		TextWriter confusion{ FileSystem::PathAppend(outDirectory, U"vowel_confusion.csv") };
		confusion << U"condition,calibration_take,truth,A,I,U,E,O,none";

		TextWriter frames;
		if (dumpFrames) {
			frames.open(FileSystem::PathAppend(outDirectory, U"frames.csv"));
			frames << U"condition,file,ms_from_speech,active,s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11";
		}

		for (const auto& condition : conditions) {
			if (conditionNames && !conditionNames.contains(condition.name)) continue;
			const double gain = NoiseGain(noise, condition, speechLevel);
			for (const int32 calibrationTake : calibrationTakes) {
				std::mt19937 rng{ static_cast<uint32>(calibrationTake * 1000 + (condition.snrDb ? static_cast<int32>(*condition.snrDb) : 99)) };
				auto phoneme = Calibrate(takes, calibrationTake, floor, noise, gain, options, calibration, rng);
				if (dumpFrames) {
					DumpFrames(takes, phoneme, floor, noise, gain, rng, frames, U"{}"_fmt(condition.name));
					continue;
				}
				const auto vowels = EvaluateVowels(takes, phoneme, calibrationTake, floor, noise, gain, rng);
				for (size_t i : step(5)) {
					confusion << U"{},{},{},{}"_fmt(condition.name, calibrationTake, VowelLabels[i], Array<size_t>(vowels.confusion[i].begin(), vowels.confusion[i].end()).join(U",", U"", U""));
				}
				if (!evaluateCommands) {
					results << U"{},{},-,{:.4f},{:.4f},0,0,0,0,0,0,0,nan"_fmt(condition.name, calibrationTake, vowels.accuracy(), static_cast<double>(vowels.noiseAsVowel) / vowels.noiseFrames);
				}
				for (const auto& [character, name] : evaluateCommands ? characters : Array<std::pair<int32, String>>{}) {
					const auto commands = EvaluateCommands(takes, phoneme, character, recognizerOptions, floor, noise, gain, rng);
					const size_t falseTotal = commands.falseBySpeech + commands.falseByNoise;
					results << U"{},{},{},{:.4f},{:.4f},{},{},{},{},{},{},{:.2f},{:.0f}"_fmt(
						condition.name, calibrationTake, name, vowels.accuracy(),
						static_cast<double>(vowels.noiseAsVowel) / vowels.noiseFrames,
						commands.expected, commands.hit, commands.wrong, commands.expected - commands.hit - commands.wrong,
						commands.falseBySpeech, commands.falseByNoise, falseTotal / commands.minutes, Median(commands.latencyMs));
					for (const auto& detection : commands.takeDetections) detectionsWriter << U"{},{},{},{}"_fmt(condition.name, calibrationTake, name, detection);
					for (const auto& [key, line] : commands.lines) {
						if (line.count) lines << U"{},{},{},{},{},{},{},{}"_fmt(condition.name, calibrationTake, name, key.first, key.second, line.count, line.hit, line.wrong);
					}
				}
				TextWriter{ FileSystem::PathAppend(outDirectory, U"progress.txt"), OpenMode::Append } << U"{} take{} {:.1f}s"_fmt(condition.name, calibrationTake, stopwatch.sF());
			}
		}
		TextWriter{ FileSystem::PathAppend(outDirectory, U"done.txt") } << U"elapsed_seconds={:.1f}"_fmt(stopwatch.sF());
	}
}

void Main() {
	const auto args = System::GetCommandLineArgs();
	try {
		Run(args);
	}
	catch (const std::exception& e) {
		if (args.size() >= 4) TextWriter{ FileSystem::PathAppend(args[3], U"error.txt") } << Unicode::FromUTF8(e.what());
	}
	catch (const Error& e) {
		if (args.size() >= 4) TextWriter{ FileSystem::PathAppend(args[3], U"error.txt") } << e.what();
	}
}
