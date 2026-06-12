#include <levels/level1.h>
#include <tiledRenderer.h>

void Level1::loadBackgroundTextures() {
	backgroundTextures[0].loadFromFile(RESOURCES_PATH "background1.png", true);
	backgroundTextures[1].loadFromFile(RESOURCES_PATH "background2.png", true);
	backgroundTextures[2].loadFromFile(RESOURCES_PATH "background3.png", true);

	backgroundsParallaxValues[0] = 0;
	backgroundsParallaxValues[1] = 0.5;
	backgroundsParallaxValues[2] = 0.9;
};

void Level1::renderLevel(gl2d::Renderer2D& renderer) {
	for (int i = 0; i < 3; i++) {
		tiledRenderer[i].texture = backgroundTextures[i];
		tiledRenderer[i].backgroundSize = 10000;
		tiledRenderer[i].parallaxStrength = backgroundsParallaxValues[i];
		tiledRenderer[i].render(renderer);
	}
};
