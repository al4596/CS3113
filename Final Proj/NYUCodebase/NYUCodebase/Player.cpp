#include "Player.h"
#include "Settings.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#define HORIZONTAL_SPEED_LIMIT 0.0018f
#define JUMP_SPEED 0.0022f
float lerp(float v0, float v1, float t) {
	return (1.0f - t) * v0 + t * v1;
}
Player::Player(unsigned int row, unsigned int column, bool player, Mix_Chunk *sound)
	: Rectangle(row, column), ContainsCenterOf(this), playerNum(player), sound(sound) {
	posX = 0.5f * column - ORTHO_X_BOUND;
	posY = 0.5f * row - ORTHO_Y_BOUND;
	hurt = false;
	rage = false;
	Stand();
}	
const float* Player::GetVertices() const {
	return STATE_VERTICES[state];
}
const float* Player::GetTexture() const {
	return STATE_TEXTURE[state];
}
float Player::GetLeftBoxBound() const {
	return GetCenterX() + GetVertices()[T1_TOP_LEFT_X];
}
float Player::GetRightBoxBound() const {
	return GetCenterX() + GetVertices()[T1_TOP_RIGHT_X];
}
float Player::GetTopBoxBound() const {
	return GetCenterY() + GetVertices()[T1_TOP_LEFT_Y];
}
float Player::GetBottomBoxBound() const {
	return GetCenterY() + GetVertices()[T1_BOTTOM_LEFT_Y];
}
bool Player::_ContainsCenterOf::operator()(const Rectangle& other) const {
	// Return whether the center of the other rectangle is within the bounds of this Player.
	return parent->GetLeftBoxBound() <= other.GetCenterX() && other.GetCenterX() <= parent->GetRightBoxBound() &&
		parent->GetBottomBoxBound() <= other.GetCenterY() && other.GetCenterY() <= parent->GetTopBoxBound();
}
Player::_ContainsCenterOf::_ContainsCenterOf(Player* parent)
	: parent(parent) {}
void Player::StayAbove(float y) {
	model.Translate(0.0f, y - GetBottomBoxBound(), 0.0f);
	velocityY = 0.0f;
}
void Player::StayToRightOf(float x) {
	model.Translate(x - GetLeftBoxBound(), 0.0f, 0.0f);
	velocityX = 0.0f;
}
void Player::StayBelow(float y) {
	model.Translate(0.0f, y - GetTopBoxBound(), 0.0f);
	velocityY = 0.0f;
}
void Player::StayToLeftOf(float x) {
	model.Translate(x - GetRightBoxBound(), 0.0f, 0.0f);
	velocityX = 0.0f;
}
float Player::GetVelocityY() const {
	return velocityY;
}
void Player::Jump() {
	state = JUMP;
}
void Player::Stand() {
	state = STAND;
}
void Player::Walk() {
	state = static_cast<States>(WALK01 + SDL_GetTicks() / 100 % (NUM_STATES - WALK01));
}
void Player::activateRage() {
	if (ragecooldown <= 0) {
		rage = true;
		rageTimer = 50;
		ragecooldown = 60;
	}
}
bool Player :: decreaseHealth() {
	if (!hurt)
		health -= 1;
	hurt = true;
	invccooldown = 75;
	return health > 0;
}
bool Player::isHurt() {
	return hurt;
}
bool Player::isRage() {
	return rage;
}

void Player::bouncePlayer(float mse) {
	velocityY += JUMP_SPEED * 1;
	model.Translate(0.0f, velocityY * mse, 0.0f);

}

void Player::ProcessInput(Uint32 mse) {
	if (hurt) {
		if (invccooldown-- <= 0) {
			hurt = false;
			invccooldown = 75;
		} 
	}
	if (rage) {
		rageTimer--;
		if (rageTimer <= 0)
			rage = false;
	}

	if (!rage) {
		ragecooldown--;
		rageTimer = 50;
	}

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	// Moving left and right
	if ((keys[SDL_SCANCODE_RIGHT] && playerNum) || (keys[SDL_SCANCODE_D] && !playerNum)) {
		Mix_VolumeChunk(sound, 4);
		Mix_PlayChannel(-1, sound, 0);

		if (velocityX < HORIZONTAL_SPEED_LIMIT) {
			accelerationX = 0.00001f;
		}
		else {
			accelerationX = 0.0f;
		}
	}
	
	else if ((keys[SDL_SCANCODE_LEFT] && playerNum) || (keys[SDL_SCANCODE_A] && !playerNum)) {
		Mix_VolumeChunk(sound, 4);
		Mix_PlayChannel(-1, sound, 0);
		if (velocityX > -HORIZONTAL_SPEED_LIMIT) {
			accelerationX = -0.00001f;
		}
		else {
			accelerationX = 0.0f;
		}
	}
	else {
		accelerationX = 0.0f;
		velocityX = lerp(velocityX, 0.0f, 0.01f * mse);
	}
	velocityX += accelerationX * mse;
	// Jumping
	if (velocityY == 0.0f) {
		allowedJumps = 2;
	}
	if ((!lastStateOfUpKey && keys[SDL_SCANCODE_UP] && allowedJumps > 0 && playerNum) || 
		(!lastStateOfUpKey && keys[SDL_SCANCODE_W] && allowedJumps > 0 && !playerNum)) {
		velocityY += JUMP_SPEED * allowedJumps;
		--allowedJumps;
	}
	if (playerNum) {
		lastStateOfUpKey = keys[SDL_SCANCODE_UP];
	}
	else {
		lastStateOfUpKey = keys[SDL_SCANCODE_W];
	};
	velocityY += GRAVITY * mse;
	// Appearance
	posX += velocityX * mse;
	posY = velocityY * mse;
	model.Translate(velocityX * mse, velocityY * mse, 0.0f);
	if (velocityY > 0.0001f) {
		Jump();
	}
	else if (fabs(velocityX) > 0.0001f) {
		Walk();
	}
	else {
		Stand();
	}
}
#include "pStateData.txt"
