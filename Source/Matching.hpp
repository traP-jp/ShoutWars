# pragma once
# include "common.hpp"

class Matching : public App::Scene
{
public:
	Matching(const InitData& init);
	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override;
};
