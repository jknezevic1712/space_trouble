#define GLM_ENABLE_EXPERIMENTAL
#include "gameLayer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "platformInput.h"
// #include "imgui.h"
#include <iostream>
#include <sstream>
// #include "imfilebrowser.h"
#include <gl2d/gl2d.h>
#include <platformTools.h>
// #include <tiledRenderer.h>
#include <bullet.h>
#include <vector>
#include <enemy.h>
#include <glui/glui.h>
#include <raudio.h>
#include <memory>
// Levels
#include <levels/baseLevel.h>
#include <levels/level1.h>
#include <levels/level2.h>

struct GameplayData
{
    glm::vec2 playerPos = { 100, 100 };
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;

    float health = 1.f; // player's health goes from 1.0 -> 0.0
    float spawnEnemyTimerSeconds = 3;

    int currentLevel = 0;
};

GameplayData data;

gl2d::Renderer2D renderer;

constexpr int BACKGROUNDS = 3;
constexpr float SHIP_SIZE = 250.f;

// Levels
// BaseLevel levels[1];
// Level1 level1;
// Only one level lives in memory at a time. unique_ptr<BaseLevel> owns whichever
// concrete level is active; reassigning it destroys the previous one automatically.
std::unique_ptr<BaseLevel> currentLevel;


gl2d::Texture spaceShipsTexture;
gl2d::TextureAtlasPadding spaceShipsAtlas;

gl2d::Texture bulletsTexture;
gl2d::TextureAtlasPadding bulletsAtlas;

// gl2d::Texture backgroundTexture[BACKGROUNDS];
// TiledRenderer tiledRenderer[BACKGROUNDS];

gl2d::Texture healthBarTexture;
gl2d::Texture healthTexture;

Sound shootSound;

void static spawnEnemy() {
    glm::uvec2 shipTypes[] = { {0, 0}, {0, 1}, {2, 0}, {3, 1} };

    Enemy e;
    e.position = data.playerPos;

    // random spawn location
    glm::vec2 offset(2000, 0);
    offset = glm::vec2(
        glm::vec4(offset, 0, 1) * glm::rotate(
            glm::mat4(1.f), glm::radians((float)(rand() % 360)), glm::vec3(0, 0, 1)
        )
    );
    e.position += offset;

    // randomize enemy stats
    e.speed = 800 + rand() % 1000;
    e.turnSpeed = 2.2f + (rand() % 1000) / 500.f;
    e.type = shipTypes[rand() % 4];
    e.fireRange = 1.5 + (rand() % 1000) / 2000.f;
    e.fireTimeReset = 0.1 + (rand() % 1000) / 500;
    e.bulletSpeed = rand() % 3000 + 1000;

    data.enemies.push_back(e);
}

bool static intersectBullet(glm::vec2 bulletPos, glm::vec2 shipPos, float shipSize) {
    // has the bullet hit the ship
    return glm::distance(bulletPos, shipPos) <= shipSize;
}

// switches the active level: frees the old level's GPU textures, instantiates the
// requested one, and loads its backgrounds. An unknown index falls back to Level1
// so currentLevel is never left null (the render path dereferences it).
void static loadLevel(int index) {
    if (currentLevel) {
        currentLevel->unload();
    }

    switch (index) {
        case 1:  currentLevel = std::make_unique<Level2>(); break;
        default: currentLevel = std::make_unique<Level1>(); break;
    }

    data.currentLevel = index;
    currentLevel->loadBackgroundTextures();
}

// static in this case means that if you were to import this file somewhere,
// you wouldn't be able to call restartGame() function since it's only "visible" in this file
void static restartGame() {
    // for (int i = 0; i < sizeof(levels) / sizeof(levels[0]); ++i) {
    // 	delete levels[i];
    // }

    data = {};
    // reset player camera
    renderer.currentCamera.follow(data.playerPos, 550, 0, 0, renderer.windowW, renderer.windowH);
}

bool initGame()
{
    //initializing stuff for the renderer
    gl2d::init();
    renderer.create();

    // load sounds
    shootSound = LoadSound(RESOURCES_PATH "shoot.flac");
    SetSoundVolume(shootSound, 0.5);

    // load UI textures
    healthBarTexture.loadFromFile(RESOURCES_PATH "healthBar.png", true);
    healthTexture.loadFromFile(RESOURCES_PATH "health.png", true);

    // load spaceships and bullet textures
    spaceShipsTexture.loadFromFileWithPixelPadding(RESOURCES_PATH "spaceShip/stitchedFiles/spaceships.png", 128, true);
    spaceShipsAtlas = gl2d::TextureAtlasPadding(5, 2, spaceShipsTexture.GetSize().x, spaceShipsTexture.GetSize().y);
    bulletsTexture.loadFromFileWithPixelPadding(RESOURCES_PATH "spaceShip/stitchedFiles/projectiles.png", 500, true);
    bulletsAtlas = gl2d::TextureAtlasPadding(5, 2, bulletsTexture.GetSize().x, bulletsTexture.GetSize().y);

    // true parameter tells the GPU that we want the texture pixelated
    // spaceShipTexture.loadFromFile(RESOURCES_PATH "spaceShip/ships/green.png", true);

    // You can render using texture atlas with both ways, but the default render method has a problem;
    // neighbour tiles can leak which leads to visual artifacts like "extra piece of texture" you don't want to have
    // This custom method adds one pixel padding around each tile
    // Difference is that you need to specify the size of one tile in pixels, 128 in this case (currently works only for square sized tiles)
    // spaceShipsTexture.loadFromFileWithPixelPadding(RESOURCES_PATH "spaceShip/stitchedFiles/spaceships.png", 128, true);

    // DONE: level textures now live in Level1/Level2 classes inheriting from BaseLevel; loaded via loadLevel() below
    // * think about passing or not passing renderer to the level class
    // backgroundTexture[0].loadFromFile(RESOURCES_PATH "background1.png", true);
    // backgroundTexture[1].loadFromFile(RESOURCES_PATH "background2.png", true);
    // backgroundTexture[2].loadFromFile(RESOURCES_PATH "background3.png", true);

    // tiledRenderer[0].texture = backgroundTexture[0];
    // tiledRenderer[1].texture = backgroundTexture[1];
    // tiledRenderer[2].texture = backgroundTexture[2];

    // tiledRenderer[0].parallaxStrength = 0;
    // tiledRenderer[1].parallaxStrength = 0.5;
    // tiledRenderer[2].parallaxStrength = 0.7;

    // levels[0] = new Level1();
    // if (levels[data.currentLevel] != nullptr) {
    // 	levels[data.currentLevel]->loadBackgroundTextures();
    // }

    loadLevel(0);

    restartGame(); // called here just to make sure that everything is configured the same

    return true;
}

bool gameLogic(float deltaTime)
{

#pragma region init stuff
    int w = 0; int h = 0;
    w = platform::getFrameBufferSizeX(); //window w
    h = platform::getFrameBufferSizeY(); //window h

    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT); //clear screen

    renderer.updateWindowMetrics(w, h);
#pragma endregion

#pragma region movement

    glm::vec2 move = {};

    if (platform::isButtonHeld(platform::Button::W) || platform::isButtonHeld(platform::Button::Up)) {
        move.y = -1;
    }

    if (platform::isButtonHeld(platform::Button::S) || platform::isButtonHeld(platform::Button::Down)) {
        move.y = 1;
    }

    if (platform::isButtonHeld(platform::Button::A) || platform::isButtonHeld(platform::Button::Left)) {
        move.x = -1;
    }

    if (platform::isButtonHeld(platform::Button::D) || platform::isButtonHeld(platform::Button::Right)) {
        move.x = 1;
    }

    if (move.x != 0 || move.y != 0) {
        move = glm::normalize(move);
        move *= deltaTime * 2000; // 2000 pixels per second
        data.playerPos += move;
    }

#pragma endregion

#pragma region camera follow

    renderer.currentCamera.follow(data.playerPos, deltaTime * 550, 1, 150, w, h);

#pragma endregion

#pragma region render background

    renderer.currentCamera.zoom = 0.2;

    // if (levels[data.currentLevel] != nullptr) {
    // 	levels[data.currentLevel]->renderLevel(renderer);
    // }
    if (currentLevel) {
        currentLevel->renderLevel(renderer);
    }

    // for (int i = 0; i < BACKGROUNDS; i++) {
    // 	tiledRenderer[i].render(renderer);
    // }


#pragma endregion

#pragma region mouse position

    glm::vec2 mousePos = platform::getRelMousePosition();
    glm::vec2 screenCenter(w / 2.f, h / 2.f);

    glm::vec2 mouseDirection = mousePos - screenCenter;

    if (glm::length(mouseDirection) == 0.f) {
        mouseDirection = { 1, 0 };
    } else {
        mouseDirection = normalize(mouseDirection);
    }

    float spaceShipAngle = atan2(mouseDirection.y, -mouseDirection.x);

#pragma endregion

#pragma region handle bullets 

    if (platform::isLMousePressed() || platform::isSpaceBarPressed(platform::Button::Space)) {
        Bullet bullet;

        bullet.position = data.playerPos;
        bullet.fireDirection = mouseDirection;

        data.bullets.push_back(bullet);

        PlaySound(shootSound);
    }

    for (int i = 0; i < data.bullets.size(); i++) {

        if (glm::distance(data.bullets[i].position, data.playerPos) > 7'000) {
            data.bullets.erase(data.bullets.begin() + i);
            i--;
            continue;
        }

        // check if the bullet came from the player
        if (!data.bullets[i].isEnemy) {
            bool breakBothLoops = false;

            // go through all the enemies to check if one of them was hit
            for (int e = 0; e < data.enemies.size(); e++) {
                if (intersectBullet(data.bullets[i].position, data.enemies[e].position, ENEMY_SHIP_SIZE)) {

                    data.enemies[e].health -= 0.1;

                    if (data.enemies[e].health <= 0) {
                        // kill enemy
                        data.enemies.erase(data.enemies.begin() + e);
                    }

                    // TODO: move bullet erasing to class method
                    data.bullets.erase(data.bullets.begin() + i);
                    i--;
                    breakBothLoops = true;
                    break;
                }
            }

            if (breakBothLoops) {
                continue;
            }

        } else {
            if (intersectBullet(data.bullets[i].position, data.playerPos, SHIP_SIZE)) {
                data.health -= 0.1;

                // erase bullet when if it hit player
                data.bullets.erase(data.bullets.begin() + i);
                i--;
                continue;
            }
        }

        data.bullets[i].update(deltaTime);
    }

    if (data.health <= 0) {
        // kill player
        restartGame();
    } else {
        data.health += deltaTime * 0.01;
        data.health = glm::clamp(data.health, 0.f, 1.f);
    }

#pragma endregion

#pragma region handle enemies

    // spawning enemies randomly
    if (data.enemies.size() < 15) {
        data.spawnEnemyTimerSeconds -= deltaTime;

        if (data.spawnEnemyTimerSeconds < 0) {
            data.spawnEnemyTimerSeconds = rand() % 5 + 1;

            spawnEnemy();

            if (rand() % 3 == 0) {
                spawnEnemy();
            }
        }
    }

    for (int i = 0; i < data.enemies.size(); i++) {

        if (glm::distance(data.playerPos, data.enemies[i].position) > 7'000.f) {
            // de-spawn enemies
            data.enemies.erase(data.enemies.begin() + i);
            i--;
            continue;
        }

        if (data.enemies[i].update(deltaTime, data.playerPos)) {
            Bullet bullet;
            bullet.position = data.enemies[i].position;
            bullet.fireDirection = data.enemies[i].viewDirection;
            bullet.speed = data.enemies[i].bulletSpeed;
            bullet.isEnemy = true;

            data.bullets.push_back(bullet);

            // if (!IsSoundPlaying(shootSound)) PlaySound(shootSound); // enable enemy firing sounds
        }
    }


#pragma endregion

#pragma region render enemies

    for (Enemy& enemy : data.enemies) {
        enemy.render(renderer, spaceShipsTexture, spaceShipsAtlas);
    }

#pragma endregion

#pragma region render ship 

    renderSpaceShip(renderer, data.playerPos, SHIP_SIZE, spaceShipsTexture, spaceShipsAtlas.get(3, 0), mouseDirection);

#pragma endregion

#pragma region render bullets

    for (Bullet& bullet : data.bullets) {
        bullet.render(renderer, bulletsTexture, bulletsAtlas);
    }

#pragma endregion

#pragma region ui

    // glui is great for creating in game menus

    // we reset camera here so that our UI elements are 
    // always drawn in the same place no matter where we move
    renderer.pushCamera();
    {
        // frame is used to specify where we want the UI to be
        // * you can also have frames inside frames (using a different scope for each frame)
        glui::Frame f({ 0, 0, w, h });

        // Box is UI element and it's responsive as well meaning that it takes into account screen dimension (actually current Frames' dimension)
        // Box starts 65% from the left of the screen, 10% from the top of the screen
        // Width will be 30% the screens' width and be 8 times smaller on Y
        // With this we get a Box that we can convert to vec4 to specify the position and size of the UI element
        glui::Box healthBox = glui::Box().xLeftPerc(0.65).yTopPerc(0.1).xDimensionPercentage(0.3).yAspectRatio(1.f / 8.f);
        renderer.renderRectangle(healthBox, healthBarTexture);

        glm::vec4 newRect = healthBox();
        newRect.z *= data.health;

        // health bar wasn't appearing right because we were just squishing the texture
        // we need to also change from where we sample it as well
        // first two values top left corner and the other two are bottom right corner of the image
        glm::vec4 textCoords = { 0, 1, 1, 0 };
        // we just need to bring the bottom right corner to the left
        textCoords.z *= data.health;

        renderer.renderRectangle(newRect, healthTexture, Colors_White, {}, {}, textCoords);
    }
    renderer.popCamera();

#pragma endregion

    renderer.flush(); // tell GPU to start computing the stuff we gave it to render

#pragma region ImGui

    //ImGui::ShowDemoWindow();

    //ImGui::Begin("debug");

    //ImGui::Text("Bullets count: %d", (int)data.bullets.size());
    //ImGui::Text("Enemies count: %d", (int)data.enemies.size());

    //if (ImGui::Button("Spawn enemy")) {
    //	spawnEnemy();
    //}

    //if (ImGui::Button("Restart game")) {
    //	restartGame();
    //}

    //ImGui::SliderFloat("Player Health", &data.health, 0, 1);

    //ImGui::End();

#pragma endregion

    return true;
}

//This function might not be be called if the program is forced closed
void closeGame()
{
    // for (int i = 0; i < sizeof(levels) / sizeof(levels[0]); ++i) {
    // 	delete levels[i];
    // }

    // explicitly free the active level's GPU textures before the renderer is destroyed,
    // then release the level itself (unique_ptr would free it anyway, but order matters here).
    if (currentLevel) {
        currentLevel->unload();
        currentLevel.reset();
    }
}
