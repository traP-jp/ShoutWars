# pragma once
# include <Siv3D.hpp>
# include "Voice/Phoneme.hpp"
# include "Multiplay/Room.hpp"
# include "Multiplay/LocalRoom.hpp"
# include "ServerConfig.hpp"
#define elif else if
#define M_PI 3.14159265358979323846

inline constexpr StringView GameVersion = U"0.3";

// シーンの名前
enum class State
{
	logo,
	Title,
	Game,
	Matching,
	Result,
	Config,
	Calibration
};

// 共有するデータ
struct GameData
{
	//キャラが確定したかどうか
	bool decided_character = false;
	//Config画面に遷移する前のシーン
	State before_scene = State::Title;
	//0:ルーム作成,1:ルーム参加,2:CPU と対戦
	int room_mode = 0;
	//ID
	std::string room_ID;
	//TODO:Debug用
	//INFO:playerが増えたらここを変更する
	//0:玲（レイ）,1:ユウカ,2:アイリ,3:No.0 (レイ）
	int player[2] = { 0,3 };

	// 音素認識用 (音素: [0:無, 1:息, 2:ア, 3:あ, 4:イ, 5:い, 6:ウ, 7:う, 8:エ, 9:え, 10:オ, 11:お])
	Phoneme phoneme{ U"config.json", 0.01, 12, 2'200'000uLL, PhonemeOptions{ .noiseSuppressionWeights = Resource(U"models/rnnoise.bin") } };
	
	//通信用
	ServerConfig server = LoadServerConfig(U"config.json", GameVersion);
	std::unique_ptr<Multiplay::IRoom> room;
	//ゲームを開始する tick
	uint64 start_tick = 0;
};

using App = SceneManager<State, GameData>;
