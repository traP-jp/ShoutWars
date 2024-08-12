# pragma once
# include "common.hpp"

class Game : public App::Scene
{
public:

	Game(const InitData& init);

	void update() override;

	void drawFadeIn(double t) const override;

	void draw() const override;
};
