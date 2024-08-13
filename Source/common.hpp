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
};

using App = SceneManager<State, GameData>;
