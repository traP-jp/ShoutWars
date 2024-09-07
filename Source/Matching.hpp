# pragma once
# include "common.hpp"

class Matching : public App::Scene
{
	//font////////////////////////////////////////////////////////////
	Font font{ 80,Typeface::Heavy };
	Font font2{ 50,Typeface::Heavy };
	//画像////////////////////////////////////////////////////////////
	const Texture background_img{ U"../images/matching/background.png" };
	const Texture select_char_img1{ U"../images/matching/select_char1.png" };
	const Texture select_char_img2{ U"../images/matching/select_char2.png" };
	const Texture select_char_img3{ U"../images/matching/select_char3.png" };
	const Texture select_char_img4{ U"../images/matching/select_char4.png" };
	const Texture stand_char_img1{ U"../images/matching/character1.png" };
	const Texture stand_char_img2{ U"../images/matching/character2.png" };
	const Texture stand_char_img3{ U"../images/matching/character3.png" };
	const Texture stand_char_img4{ U"../images/matching/character4.png" };
	const Texture random_select_img{ U"../images/matching/random_select.png" };
	const Texture return_img{ U"../images/common/return.png" };
	const Texture setting_img{ U"../images/common/setting.png" };
	const Texture disabled_setting_img{ U"../images/common/disabled_setting.png" };
	const Texture copied_img{ U"../images/matching/copied.png" };
	const Texture connecting_img{ U"../images/common/connecting.png" };
	const Texture not_found_img{ U"../images/matching/404.png" };
	const Texture not_found_img2{ U"../images/matching/810.png" };
	const Texture error400_img{ U"../images/matching/400.png" };
	const Texture error500_img{ U"../images/matching/500.png" };
	const Texture error_img{ U"../images/matching/error.png" };
	const Texture suneo_img{ U"../images/matching/suneo.png" };
	const Texture timeout_img{ U"../images/matching/timeout.png" };
	const Texture you_img{ U"../images/matching/you.png" };
	const Texture decide_img{ U"../images/matching/decide.png" };
	const Texture decided_img{ U"../images/matching/decided.png" };
	const Texture fixed_img{ U"../images/matching/fixed.png" };
	const Texture calibration_img{ U"../images/matching/calibration.png" };
	Texture stand_char_img[4] = { stand_char_img1,stand_char_img2,stand_char_img3,stand_char_img4 };
	//shape////////////////////////////////////////////////////////////
	const Quad select_char_shape1 { Vec2{280,720},Vec2{577,720},Vec2{527,957},Vec2{230,957} };
	const Quad select_char_shape2 { Vec2{590,720},Vec2{887,720},Vec2{837,957},Vec2{540,957} };
	const Quad select_char_shape3 { Vec2{1035,720},Vec2{1332,720},Vec2{1382,957},Vec2{1085,957} };
	const Quad select_char_shape4 { Vec2{1345,720},Vec2{1642,720},Vec2{1692,957},Vec2{1395,957} };
	const Quad random_select_shape{ Vec2{905,720},Vec2{1021,720},Vec2{1076,957},Vec2{850,957} };
	const Rect return_shape{ 20,20,80,80 };
	const Circle setting_shape{ 1852,68,48 };
	const RectF RoomID_shape = font(U"888888").regionAt(Vec2{960,70});
	const RectF timer_shape = font2(U"10:00").regionAt(Vec2{ 960,1000 });
	const Rect OK_shape {  680,464,240,105 };
	const Rect Yes_shape{ 1010,464,240,105 };
	const Rect decide_button_shape{ 960-300, 540-75, 600, 150};
	//音声素材////////////////////////////////////////////////////////
	const Audio bgm{ U"../audioes/th3_05.mp3" , Loop::Yes };
	const Audio cancel_sound{ U"../audioes/cancel.wav" };
	const Audio copied_se{ U"../audioes/copied.mp3" };
	const Audio click_sound{ U"../audioes/click.wav" };
	const Audio decision_sound{ U"../audioes/decided_char.mp3" };

	//キャラ選択関連
	int character_number = 0;
	int opponent_character_number = 0;
	bool character_changed = false;
	double decide_button_size = 1.0;
	bool opponent_decided = false;
	int confirm_num;
	//部屋に関して
	std::string room_ID;
	bool is_owner = false;
	int member_sum = 1;
	bool recieved_time = false;
	bool confirm_accuracy = false;

	//制限時間
	String remaining_time = U"10:00";
	String old_remaining_time = U"10:00";

	//エラーダイアログ関連
	int error_mode = 0;
	//0:バグ,1:404,2:810,3:400,4:500,5:スネ夫,6:時間切れ,7:音声認識のセットアップやれ
	int error_ID = 0;
	int error_pos_y = 1400;
	int error_timer = 0;
	double back_alpha = 0.0;
	//コピー関連
	int copy_mode = 0;
	int copy_pos_y = -30;
	int copy_timer = 0;

	bool gotoGame = false;
	//内部関数/////////////////////////////////////////////////////////
	void drawErrorDialog() const;
	void syncRoomInfo();
	void setErrorMessage(int error_code, String message);
	String CalcRemainingTime();
public:
	Matching(const InitData& init);
	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override;
	void drawFadeOut(double t) const override;
};
