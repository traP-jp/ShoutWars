# pragma once
# include "common.hpp"

class Title : public App::Scene
{
public:
	Title(const InitData& init);

	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override;
	void drawFadeOut(double t) const override;
};
