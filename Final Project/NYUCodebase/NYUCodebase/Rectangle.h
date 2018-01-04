#pragma once
#include "Matrix.h"
struct Rectangle {
	Rectangle(unsigned int row, unsigned int column);
	float GetCenterX() const;
	float GetCenterY() const;
	float GetLeftBoxBound() const;
	float GetRightBoxBound() const;
	float GetTopBoxBound() const;
	float GetBottomBoxBound() const;
	float GetWidth() const;
	float GetHeight() const;
	static void SetBoxLeft(float*, float);
	static void SetBoxRight(float*, float);
	static void SetBoxTop(float*, float);
	static void SetBoxBottom(float*, float);
	static void SetBox(float*, float top, float right, float bottom, float left);
	enum VertexIndices {
		T1_TOP_LEFT_X,
		T1_TOP_LEFT_Y,
		T1_BOTTOM_LEFT_X,
		T1_BOTTOM_LEFT_Y,
		T1_TOP_RIGHT_X,
		T1_TOP_RIGHT_Y,
		T2_TOP_RIGHT_X,
		T2_TOP_RIGHT_Y,
		T2_BOTTOM_LEFT_X,
		T2_BOTTOM_LEFT_Y,
		T2_BOTTOM_RIGHT_X,
		T2_BOTTOM_RIGHT_Y,
		NUM_VERTICES
	};
	Matrix model;
	const static float VERTICES[];
	float texture[NUM_VERTICES];
};

