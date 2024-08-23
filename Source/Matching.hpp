# pragma once
# include "common.hpp"

class Matching : public App::Scene
{
	const Texture background_img{ U"../images/matching/background.png" };
	const Texture select_char_img1{ U"../images/matching/select_char1.png" };
	const Texture select_char_img2{ U"../images/matching/select_char2.png" };
	const Texture select_char_img3{ U"../images/matching/select_char3.png" };
	const Texture select_char_img4{ U"../images/matching/select_char4.png" };
	const Texture random_select_img{ U"../images/matching/random_select.png" };
public:
	Matching(const InitData& init);
	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override;
};
