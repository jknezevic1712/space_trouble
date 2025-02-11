#include <bullet.h>

void Bullet::render(gl2d::Renderer2D& renderer, gl2d::Texture& bulletsTexture, gl2d::TextureAtlasPadding& bulletsAtlas) {

	float bulletAngle = atan2(fireDirection.y, -fireDirection.x);
	bulletAngle = glm::degrees(bulletAngle) + 90.f;

	glm::vec4 bulletTexture = bulletsAtlas.get(1, 1);
	if (isEnemy) {
		bulletTexture = bulletsAtlas.get(0, 0);
	}

	// simple bullet rendering
	// renderer.renderRectangle(
	//	{ position + glm::vec2(25, 25), 150, 150 },
	//	bulletsTexture,
	//	Colors_White,
	//	{},
	//	bulletAngle,
	//	bulletTexture
	// );

	// draw bullet five times, starting from a darker to a brighter color for better visuals
	for (int i = 0; i < 5; i++) {
		glm::vec4 color(1 * (i + 4) / 5.f, 1 * (i + 4) / 5.f, 1 * (i + 4) / 5.f, (i + 1) / 5.f);

		renderer.renderRectangle(
			{ position - glm::vec2(50, 50) + (float)i * 25.f * fireDirection, 100, 100},
			bulletsTexture,
			color,
			{},
			bulletAngle,
			bulletTexture
		);
	}
}

void Bullet::update(float deltaTime) {
	position += fireDirection * deltaTime * speed;
}