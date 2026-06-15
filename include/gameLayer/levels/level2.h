#pragma once
#include <levels/baseLevel.h>

class Level2 : public BaseLevel {
public:
    void loadBackgroundTextures() override;
    void renderLevel(gl2d::Renderer2D& renderer) override;
};
