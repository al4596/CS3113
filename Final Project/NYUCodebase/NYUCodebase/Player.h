#pragma once
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "Rectangle.h"
struct Player : public Rectangle {
	Player(unsigned int row, unsigned int column, bool player, Mix_Chunk *sound = NULL);
	const float* GetVertices() const;
	const float* GetTexture() const;
	float GetLeftBoxBound() const;
	float GetRightBoxBound() const;
	float GetTopBoxBound() const;
	float GetBottomBoxBound() const;
	void StayAbove(float y);
	void StayToRightOf(float x);
	void StayBelow(float y);
	void StayToLeftOf(float x);
	void activateRage();
	bool decreaseHealth();
	bool isHurt();
	bool isRage();
	float GetVelocityY() const;
	Mix_Chunk *sound;
	void Jump();
	void Stand();
	void Walk();
	void ProcessInput(Uint32 MillisecondsElapsed);
	class _ContainsCenterOf {
	public:
		bool operator()(const Rectangle&) const;
		friend Player;
	private:
		_ContainsCenterOf(Player*);
		Player* parent;
	} ContainsCenterOf;
#include "pEnums.txt"
	static const float STATE_VERTICES[][NUM_VERTICES];
	static const float STATE_TEXTURE[][NUM_VERTICES];
	float posX = 0.0f;
	float posY = 0.0f;
	void bouncePlayer(float mse);

private:
	int health = 3;
	bool playerNum;
	static const float VERTICES[];
	int invccooldown = 75;
	bool hurt;
	int ragecooldown = 0;
	int rageTimer = 50;
	bool rage;
	States state;
	float velocityX = 0.0f;
	float velocityY = 0.0f;
	
	float accelerationX = 0.0f;
	int allowedJumps = 0;
	uint8_t lastStateOfUpKey = 0;
};
