#include <levels/level2.h>
#include <tiledRenderer.h>

void Level2::loadBackgroundTextures() {
    // TODO: Add new bgs
    backgroundTextures[0].loadFromFile(RESOURCES_PATH "background1.png", true);
    backgroundTextures[1].loadFromFile(RESOURCES_PATH "background2.png", true);
    backgroundTextures[2].loadFromFile(RESOURCES_PATH "background3.png", true);

    backgroundsParallaxValues[0] = 0;
    backgroundsParallaxValues[1] = 0.3;
    backgroundsParallaxValues[2] = 0.7;
};

void Level2::renderLevel(gl2d::Renderer2D& renderer) {
    for (int i = 0; i < NUM_OF_BACKGROUNDS; i++) {
        tiledRenderer[i].texture = backgroundTextures[i];
        tiledRenderer[i].backgroundSize = 10000;
        tiledRenderer[i].parallaxStrength = backgroundsParallaxValues[i];
        tiledRenderer[i].render(renderer);
    }
};
