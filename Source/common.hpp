# pragma once
# include <Siv3D.hpp>
# include "Voice/Phoneme.hpp"
#define elif else if

// シーンの名前
enum class State
{
	logo,
	Title,
	Game,
	Matching,
	Result,
	Calibration
};

// 共有するデータ
struct GameData
{
	int clear = 0;
	//0:ルーム作成,1:ルーム参加
	int room_mode = 0;
	//ID
	int room_ID = 0;
	//TODO:Debug用
	//INFO:playerが増えたらここを変更する
	//0:玲（レイ）,1:ユウカ,2:アイリ,3:No.0 (レイ）
	int player[2] = { 1,0 };

	// 音素認識用 (音素: [0:無, 1:息, 2:ア, 3:あ, 4:イ, 5:い, 6:ウ, 7:う, 8:エ, 9:え, 10:オ, 11:お])
	Phoneme phoneme{ 0.01, 12 };
};

using App = SceneManager<State, GameData>;
