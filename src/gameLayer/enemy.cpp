#include <enemy.h>
#include <tiledRenderer.h>

void Enemy::render(gl2d::Renderer2D& renderer, gl2d::Texture& sprites, gl2d::TextureAtlasPadding& atlas) {
	renderSpaceShip(renderer, position, ENEMY_SHIP_SIZE, sprites, atlas.get(type.x, type.y), viewDirection);
}

bool Enemy::update(float deltaTime, glm::vec2 playerPos) {
#pragma region enemy movement

	glm::vec2 directionToPlayer = playerPos - position;

	if (glm::length(directionToPlayer) == 0) {
		directionToPlayer = { 1, 0 };
	} else {
		directionToPlayer = glm::normalize(directionToPlayer);
	}

	// make the enemy slowly turns its direction to the player
	glm::vec2 newDirection = {};

	// if the vector is close to 0, that means that the two directions are opposite so we will just turn the enemy 90deg
	if (glm::length(directionToPlayer + viewDirection) <= 0.2) {

		// make the enemy ship randomly turn up or down
		if (rand() % 2) {
			newDirection = glm::vec2(directionToPlayer.y, -directionToPlayer.x);
		} else {
			newDirection = glm::vec2(-directionToPlayer.y, directionToPlayer.x);
		}
		
	} else {
		newDirection = deltaTime * turnSpeed * directionToPlayer + viewDirection;
	}

	// this length will make the enemies move "more elliptical"
	float length = glm::length(newDirection);
	viewDirection = glm::normalize(newDirection);

	length = glm::clamp(length, 0.1f, 3.f);

	position += viewDirection * deltaTime * speed * length;

#pragma endregion

#pragma region enemy firing

	bool shoot = (glm::length(directionToPlayer + viewDirection) >= fireRange);

	if (shoot) {
		if (firedTime <= 0.f) {
			// enemy can shoot
			firedTime = fireTimeReset;
		} else {
			shoot = 0;
		}
	}

	firedTime -= deltaTime;
	if (firedTime < 0) {
		firedTime = 0.f;
	}

#pragma endregion

	return shoot;
}
