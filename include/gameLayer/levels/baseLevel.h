#pragma once
// #define GLM_ENABLE_EXPERIMENTAL
// #include <gl2d/gl2d.h>

// #include <baseLevel.h>
#include <tiledRenderer.h>
#include <iostream>
// #include <glad/glad.h>
// #include <glm/glm.hpp>
// #include <glm/gtx/transform.hpp>
// #include "platformInput.h"
// #include <gl2d/gl2d.h>
// #include <platformTools.h>

class BaseLevel {
protected:
	// constexpr static int NUM_OF_BACKGROUNDS = 3;

	TiledRenderer tiledRenderer[3];
	gl2d::Texture backgroundTextures[3] = {};
	float backgroundsParallaxValues[3] = {};

public:
	virtual void loadBackgroundTextures() = 0;
	virtual void renderLevel(gl2d::Renderer2D& renderer) = 0;
};