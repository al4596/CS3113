#pragma once
#include "Rectangle.h"
struct Tile : public Rectangle {
	Tile(unsigned int id, unsigned int row, unsigned int column);
	unsigned int GetID() const;
private:
	unsigned int id;
};
