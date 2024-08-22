# pragma once
# include <Siv3D.hpp>
#define elif else if

// シーンの名前
enum class State
{
	logo,
	Title,
	Game,
	Matching,
	Result
};

// 共有するデータ
struct GameData
{
	int32 clear = 0;
	//TODO:Debug用
	//INFO:playerが増えたらここを変更する
	//0:玲（レイ）,1:ユウカ,2:アイリ,3:No.0 (レイ）
	int32 player[2] = { 1,0 };
};

using App = SceneManager<State, GameData>;
