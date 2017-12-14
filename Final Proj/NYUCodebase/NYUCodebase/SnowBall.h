#pragma once
#include <SDL.h>
#include "Rectangle.h"
#include "Matrix.h"
struct SnowBall {
	SnowBall(unsigned int x, unsigned int y, bool SnowBall);
	const float* GetVertices() const;
	const float* GetTexture() const;
	float GetLeftBoxBound() const;
	float GetRightBoxBound() const;
	float GetTopBoxBound() const;
	float GetBottomBoxBound() const;
	float GetVelocityY() const;
	void ProcessInput(Uint32 MillisecondsElapsed);
	
	Matrix model;
	static const float STATE_VERTICES[][12];
	static const float STATE_TEXTURE[][12];
private:
	// The SnowBall class will not use the default vertex array.
	bool SnowBallSide;
	static const float VERTICES[];
	float velocityX = 0.0f;
	float velocityY = 0.0f;
	float startX;
	float startY;
	float accelerationX = 0.0f;
	int allowedJumps = 0;
	uint8_t lastStateOfUpKey = 0;
};
