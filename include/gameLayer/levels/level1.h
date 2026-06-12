#pragma once
#include <levels/baseLevel.h>

class Level1 : public BaseLevel {
public:
	// Level1();
	// ~Level1();

	void loadBackgroundTextures() override;
	void renderLevel(gl2d::Renderer2D& renderer) override;
	// Implement other pure virtual functions from BaseLevel here
};

// Level1::Level1() {
//     // Constructor implementation
// }

// Level1::~Level1() {
//     // Destructor implementation
// }

// void Level1::renderLevel(gl2d::Renderer2D& renderer) {
// 	// Implementation of renderLevel
// }