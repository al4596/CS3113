// David Tsai helped with collision
#include "Rectangle.h"
#include "Settings.h"
Rectangle::Rectangle(unsigned int row, unsigned int column) {
	model.Translate(0.5f * column - ORTHO_X_BOUND, 0.5f * row - ORTHO_Y_BOUND, 0.0f);
}
float Rectangle::GetCenterX() const {
	return model.m[3][0];
}
float Rectangle::GetCenterY() const {
	return model.m[3][1];
}
float Rectangle::GetLeftBoxBound() const {
	return GetCenterX() + VERTICES[T1_TOP_LEFT_X];
}
float Rectangle::GetRightBoxBound() const {
	return GetCenterX() + VERTICES[T1_TOP_RIGHT_X];
}
float Rectangle::GetTopBoxBound() const {
	return GetCenterY() + VERTICES[T1_TOP_LEFT_Y];
}
float Rectangle::GetBottomBoxBound() const {
	return GetCenterY() + VERTICES[T1_BOTTOM_LEFT_Y];
}
float Rectangle::GetWidth() const {
	return VERTICES[T1_TOP_RIGHT_X] - VERTICES[T1_TOP_LEFT_X];
}
float Rectangle::GetHeight() const {
	return VERTICES[T1_TOP_LEFT_Y] - VERTICES[T1_BOTTOM_LEFT_Y];
}
void Rectangle::SetBoxLeft(float* a, float x) {
	a[VertexIndices::T1_TOP_LEFT_X] =
		a[VertexIndices::T1_BOTTOM_LEFT_X] =
		a[VertexIndices::T2_BOTTOM_LEFT_X] =
		x;
}
void Rectangle::SetBoxRight(float* a, float x) {
	a[VertexIndices::T1_TOP_RIGHT_X] =
		a[VertexIndices::T2_TOP_RIGHT_X] =
		a[VertexIndices::T2_BOTTOM_RIGHT_X] =
		x;
}
void Rectangle::SetBoxTop(float* a, float y) {
	a[VertexIndices::T1_TOP_LEFT_Y] =
		a[VertexIndices::T1_TOP_RIGHT_Y] =
		a[VertexIndices::T2_TOP_RIGHT_Y] =
		y;
}
void Rectangle::SetBoxBottom(float* a, float y) {
	a[VertexIndices::T1_BOTTOM_LEFT_Y] =
		a[VertexIndices::T2_BOTTOM_LEFT_Y] =
		a[VertexIndices::T2_BOTTOM_RIGHT_Y] =
		y;
}
void Rectangle::SetBox(float* a, float top, float right, float bottom, float left) {
	SetBoxTop(a, top);
	SetBoxLeft(a, left);
	SetBoxRight(a, right);
	SetBoxBottom(a, bottom);
}
