# pragma once
# include "common.hpp"

class Matching : public App::Scene
{
	//font////////////////////////////////////////////////////////////
	Font font{ 80,Typeface::Heavy };
	//画像////////////////////////////////////////////////////////////
	const Texture background_img{ U"../images/matching/background.png" };
	const Texture select_char_img1{ U"../images/matching/select_char1.png" };
	const Texture select_char_img2{ U"../images/matching/select_char2.png" };
	const Texture select_char_img3{ U"../images/matching/select_char3.png" };
	const Texture select_char_img4{ U"../images/matching/select_char4.png" };
	const Texture random_select_img{ U"../images/matching/random_select.png" };
	const Texture return_img{ U"../images/matching/return.png" };
	const Texture copied_img{ U"../images/matching/copied.png" };
	const Texture connecting_img{ U"../images/common/connecting.png" };
	const Texture not_found_img{ U"../images/matching/404.png" };
	const Texture not_found_img2{ U"../images/matching/810.png" };
	//shape////////////////////////////////////////////////////////////
	const Quad select_char_shape1 { Vec2{280,720},Vec2{577,720},Vec2{527,957},Vec2{230,957} };
	const Quad select_char_shape2 { Vec2{590,720},Vec2{887,720},Vec2{837,957},Vec2{540,957} };
	const Quad select_char_shape3 { Vec2{1035,720},Vec2{1332,720},Vec2{1382,957},Vec2{1085,957} };
	const Quad select_char_shape4 { Vec2{1345,720},Vec2{1642,720},Vec2{1692,957},Vec2{1395,957} };
	const Quad random_select_shape{ Vec2{905,720},Vec2{1021,720},Vec2{1076,957},Vec2{850,957} };
	const Rect return_shape{ 20,20,80,80 };
	const RectF RoomID_shape = font(U"888888").region(Vec2{800,30});
	const Rect OK_shape {  680,464,240,105 };
	const Rect Yes_shape{ 1010,464,240,105 };
	//音声素材////////////////////////////////////////////////////////
	const Audio bgm{ U"../audioes/th3_05.mp3" , Loop::Yes };
	const Audio cancel_sound{ U"../audioes/cancel.wav" };
	const Audio copied_se{ U"../audioes/copied.mp3" };

	int character_number = 0;
	std::string room_ID;

	//エラーダイアログ関連
	int error_mode = 0;
	int error_pos_y = 1400;
	int error_timer = 0;
	double back_alpha = 0.0;
	//コピー関連
	int copy_mode = 0;
	int copy_pos_y = -30;
	int copy_timer = 0;

	void drawErrorDialog() const;
public:
	Matching(const InitData& init);
	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override;
	void drawFadeOut(double t) const override;
};
