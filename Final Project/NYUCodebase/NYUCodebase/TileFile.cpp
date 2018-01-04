#include "TileFile.h"
#include <algorithm>
#include <cstdlib>
#include <sstream>
using std::begin;
using std::copy;
using std::end;
using std::getline;
using std::istream;
using std::istringstream;
using std::move;
using std::string;
using std::strtol;
using std::unordered_map;
TileFile::ParseError::ParseError(const char* message, const std::string& line)
	: message(message), line(line) {}
TileFile::Location::Location(unsigned int row, unsigned int column)
	: row(row), column(column) {}
TileFile::TileFile() {}
TileFile::TileFile(istream& tileFile) {
	// Create a map of handlers for each line in each section in the file.
	unordered_map<std::string, TileFileSection*> handlers = {
		{"[header]", &TileFileHeader()},
		{"[tilesets]", &TileFileTilesets()},
		{"[layer]", &TileFileLayer()},
		{"[Entities]", &TileFileEntities()}
	};
	// Read the file.
	string line, key, value;
	while (getline(tileFile, line)) {
		// Find the handler for this section.
		auto section = handlers.find(line);
		if (section == handlers.end()) {
			// The handler could not be found.
			throw ParseError("Invalid section", line);
		}
		else {
			// The handler was found.
			section->second->SectionStart(this);
			while (getline(tileFile, line)) {
				// Check for a blank line, which marks the end of the section.
				if (line.empty()) {
					break;
				}
				// If the line starts with an octothorpe or semicolon, ignore it.
				if (line[0] == '#' || line[0] == ';') {
					continue;
				}
				// Split the line into the key and value.
				istringstream ss(line);
				if (!ss) {
					throw ParseError("Could not instantiate string stream for line explosion", line);
				}
				if (!getline(ss, key, '=')) {
					throw ParseError("Could not read key", line);
				}
				getline(ss, value);
				// If the value is empty, then read lines into the value until a blank line is encountered.
				bool jump = false;
				if (value.empty()) {
					while (getline(tileFile, line)) {
						if (line.empty()) {
							jump = true;
							break;
						}
						value.append(line);
					}
				}
				// Pass the key and value to the handler.
				section->second->Data(this, key, value);
				if (jump) {
					break;
				}
			}
			section->second->SectionEnd(this);
		}
	}
}
TileFile::TileFile(const TileFile& rhs)
	: mapWidth(rhs.mapWidth), mapHeight(rhs.mapHeight), entities(rhs.entities) {
	CopyLayers(rhs.layers);
}
TileFile::TileFile(TileFile&& rhs)
	: mapWidth(rhs.mapWidth), mapHeight(rhs.mapHeight), layers(move(rhs.layers)), entities(move(rhs.entities)) {}
TileFile& TileFile::operator=(const TileFile& rhs)
{
	mapWidth = rhs.mapWidth;
	mapHeight = rhs.mapHeight;
	DestroyLayers();
	CopyLayers(rhs.layers);
	entities = rhs.entities;
	return *this;
}
TileFile::~TileFile() {
	DestroyLayers();
}
unsigned int TileFile::GetMapWidth() const {
	return mapWidth;
}
unsigned int TileFile::GetMapHeight() const {
	return mapHeight;
}
unsigned int TileFile::RowFromTopToRowFromBottom(unsigned int row) const {
	// The tile map counts rows down from the top, while OpenGL counts up from the bottom.
	return GetMapHeight() - row - 1;
}
const TileFile::Layers& TileFile::GetLayers() const {
	return layers;
}
const TileFile::Entities& TileFile::GetEntities() const {
	return entities;
}
void TileFile::TileFileHeader::SectionStart(TileFile*) {}
void TileFile::TileFileHeader::Data(TileFile* parent, const string& key, const string& value) {
	// Set the map width or height.
	if (key == "width") {
		parent->mapWidth = strtol(value.c_str(), nullptr, 10);
	}
	else if (key == "height") {
		parent->mapHeight = strtol(value.c_str(), nullptr, 10);
	}
}
void TileFile::TileFileHeader::SectionEnd(TileFile* parent) {
	// Check that the width and height have been set.
	if (parent->mapWidth <= 0) {
		throw ParseError("Map width not set or not positive");
	}
	if (parent->mapHeight <= 0) {
		throw ParseError("Map height not set or not positive");
	}
}
void TileFile::TileFileTilesets::SectionStart(TileFile*) {}
void TileFile::TileFileTilesets::Data(TileFile*, const string& key, const string& value) {}
void TileFile::TileFileTilesets::SectionEnd(TileFile*) {}
void TileFile::TileFileLayer::SectionStart(TileFile* parent) {
	// Make sure that the map data array has been allocated.
	if (!(parent->mapWidth && parent->mapHeight)) {
		throw ParseError("The layer section must come after the header section.");
	}
}
void TileFile::TileFileLayer::Data(TileFile* parent, const string& key, const string& value) {
	if (key == "type") {
		// Check whether this type was already defined.
		if (parent->layers.find(value) != parent->layers.end()) {
			throw ParseError("A layer type was seen twice:", value);
		}
		// Allocate a two-dimensional array to represent the map.
		map = new int*[parent->mapHeight];
		for (unsigned int i = 0; i < parent->mapHeight; ++i) {
			map[i] = new int[parent->mapWidth];
		}
		// Add that array to the layers.
		parent->layers.emplace(value, map);
	}
	else if (key == "data") {
		// Make sure that a type has been set.
		if (!map) {
			throw ParseError("In the layer section, data came before a type.");
		}
		// Split the value up by commas.
		istringstream ss(value);
		if (!ss) {
			throw ParseError("Could not instantiate string stream for layer data");
		}
		string tile;
		unsigned int i = 0, j = 0;
		while (getline(ss, tile, ',')) {
			// Make sure that the current row is in range.
			if (i >= parent->mapHeight) {
				throw ParseError("There are too many tiles.");
			}
			// Parse the value into a tile ID.
			map[i][j] = strtol(tile.c_str(), nullptr, 10) - 1;
			// Move to the next tile.
			if (++j >= parent->mapWidth) {
				++i;
				j = 0;
			}
		}
		// If the number of tiles is correct, then the current row should equal the map height.
		if (i < parent->mapHeight) {
			throw ParseError("There are too few tiles.");
		}
		// Unset the type to ensure that the next data is preceded by a type.
		map = nullptr;
	}
	else {
		throw ParseError("Unknown key in layers:", key);
	}
}
void TileFile::TileFileLayer::SectionEnd(TileFile*) {
	// If there is a type at the end with no data, then map will not be null.
	if (map) {
		throw ParseError("There is a layer type with no data.");
	}
}
void TileFile::TileFileEntities::SectionStart(TileFile*) {
	type.clear();
	location.clear();
}
void TileFile::TileFileEntities::Data(TileFile*, const std::string& key, const std::string& value) {
	if (key == "type") {
		type = value;
	}
	else if (key == "location") {
		location = value;
	}
	else {
		throw ParseError("Unknown key in Entities:", key);
	}
}
void TileFile::TileFileEntities::SectionEnd(TileFile* parent) {
	if (type.empty()) {
		throw ParseError("An entity is missing a type.");
	}
	if(location.empty()) {
		throw ParseError("An entity is missing a location.");
	}
	// Read the first two comma-separated values from the location.
	// The other two values will be ignored.
	istringstream ss(location);
	if (!ss) {
		throw ParseError("Could not instantiate string stream for entity location");
	}
	string column, row;
	if (!getline(ss, column, ',') || column.empty()) {
		throw ParseError("Could not read column of entity");
	}
	if (!getline(ss, row, ',') || row.empty()) {
		throw ParseError("Could not read row of entity");
	}
	parent->entities[type].emplace_back(strtol(row.c_str(), nullptr, 10), strtol(column.c_str(), nullptr, 10));
}	
void TileFile::CopyLayers(const TileFile::Layers& other) {
	for (const auto& layer : other) {
		int** map = new int*[mapHeight];
		for (unsigned int i = 0; i < mapHeight; ++i) {
			map[i] = new int[mapWidth];
			for (unsigned int j = 0; j < mapWidth; ++j) {
				map[i][j] = layer.second[i][j];
			}
		}
		layers.emplace(layer.first, map);
	}
}
void TileFile::DestroyLayers() {
	for (const auto& layer : layers) {
		for (unsigned int i = 0; i < mapHeight; ++i) {
			delete layer.second[i];
		}
		delete layer.second;
	}
}
