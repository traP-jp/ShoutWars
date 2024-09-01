# pragma once
# include <Siv3D.hpp>
# include "Multiplay/SyncClient.hpp"
#define elif else if

// シーンの名前
enum class State
{
	logo,
	Title,
	Game,
	Matching,
	Result,
	Config
};

// 共有するデータ
struct GameData
{
	//キャラが確定したかどうか
	bool decided_character = false;
	//接続開始時間
	int timer = 0;
	//Config画面に遷移する前のシーン
	State before_scene = State::Title;
	//0:ルーム作成,1:ルーム参加
	int room_mode = 0;
	//ID
	std::string room_ID;
	//TODO:Debug用
	//INFO:playerが増えたらここを変更する
	//0:玲（レイ）,1:ユウカ,2:アイリ,3:No.0 (レイ）
	int player[2] = { 1,0 };
	//通信用
	std::unique_ptr<SyncClient> client;
};

using App = SceneManager<State, GameData>;
