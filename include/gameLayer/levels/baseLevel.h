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

// per-level data: each concrete level populates this in its constructor.
// defaults below intentionally mirror the current hardcoded behavior in gameLayer.cpp
// (spawn loop ~316-327, spawnEnemy ~78-83) so leaving them untouched changes nothing at runtime.
struct LevelConfig {
    // enemy spawn
    int   maxEnemies          = 15;  // cap in the spawn loop
    float spawnIntervalMin    = 1;   // today: rand() % 5 + 1
    float spawnIntervalMax    = 5;
    // difficulty tuning (multipliers applied to the randomized enemy stats)
    float enemySpeedMul       = 1.f;
    float enemyBulletSpeedMul = 1.f;
};

class BaseLevel {
protected:
    constexpr static int NUM_OF_BACKGROUNDS = 3;

    TiledRenderer tiledRenderer[NUM_OF_BACKGROUNDS];
    gl2d::Texture backgroundTextures[NUM_OF_BACKGROUNDS] = {};
    float backgroundsParallaxValues[NUM_OF_BACKGROUNDS] = {};

    LevelConfig config;

public:
    // virtual destructor is required so deleting a derived level through a
    // BaseLevel pointer (unique_ptr<BaseLevel>) is well-defined.
    virtual ~BaseLevel() {}

    virtual void loadBackgroundTextures() = 0;
    virtual void renderLevel(gl2d::Renderer2D& renderer) = 0;

    // frees only this level's GPU textures; the renderer is shared and owned by
    // gameLayer, so we must leave it alone (do NOT call renderer.cleanup()).
    void unload() {
        for (int i = 0; i < NUM_OF_BACKGROUNDS; i++) {
            backgroundTextures[i].cleanup();   // glDeleteTextures
            backgroundTextures[i] = {};        // zero the handle so it can't be reused/double-freed
            backgroundsParallaxValues[i] = 0;
        }
    }

    const LevelConfig& getConfig() const { return config; }
};