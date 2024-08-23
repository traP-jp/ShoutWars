//TODO:ネット関連のものが完成したら実装しろ

#include "Matching.hpp"

Matching::Matching(const InitData& init) : IScene(init)
{
	//TODO:ネット関連のものが完成したら実装しろ
	//TODO:ネット関連のものが完成したら実装しろ
}

void Matching::update()
{
}

void Matching::draw() const
{
	//background_img.draw(0, 0);
	select_char_img1.draw(230, 720);
	select_char_img2.draw(540, 720);
	random_select_img.draw(850, 720);
	select_char_img3.draw(1035, 720);
	select_char_img4.draw(1345, 720);
}

void Matching::drawFadeIn(double t) const
{
}
