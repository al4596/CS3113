#include <algorithm>
#include <chrono>
#include <forward_list>
#include <fstream>
#include <iostream>
#include <list>
#include <thread>
#include <vector>
#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Settings.h"
#include "Matrix.h"
#include "Player.h"
#include "ShaderProgram.h"
#include "Tile.h"
#include "TileFile.h"
#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
using namespace std;

string TILE_FILE = "demo.txt";

const Matrix IDENTITY;
SDL_Window* displayWindow;
ShaderProgram* program;

//Classes
enum GameState { MAIN_MENU, MAP_SELECT, GAME_LEVEL, GAME_OVER };
	
class Vector3 {
public:
	Vector3() {}

	Vector3(float x, float y, float z) : x(x), y(y), z(z) {};
	float x;
	float y;
	float z;
};

//Loads Texture
GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
	if (image == NULL) {
		cerr << "Unable to load image: " << filePath << '\n';
		exit(7);
	}
	GLuint result;
	glGenTextures(1, &result);
	glBindTexture(GL_TEXTURE_2D, result);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return result;
}

//Sheet Class
class SheetSprite {
public:
	SheetSprite() {};
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
		size) :textureID(textureID), u(u), v(v), width(width), height(height), size(size) {}

	void Draw(ShaderProgram *program);
	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

void SheetSprite::Draw(ShaderProgram *program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};
	float aspect = width / height;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size ,
		0.5f * size * aspect, -0.5f * size };

	// draw our arrays
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);

}

//Draw Texts
void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
		int spriteIndex = (int)text[i];
		float texture_x = (float)(spriteIndex % 16) / 16.0f;
		float texture_y = (float)(spriteIndex / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}

	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6 * text.size());
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

//Entity Class
class Entity {
public:
	Entity() {}

	void Draw(ShaderProgram *program) {
		sprite.Draw(program);
	}

	Vector3 position;
	float velocity;
	Vector3 size;
	Matrix model;
	float rotation;
	SheetSprite sprite;
	float health;
	float somethingElse;
};

void DrawTrianglesWithTexture(const Matrix& ModelviewMatrix, GLsizei numTriangles, const float* vertices, const float* texCoords, GLuint textureID) {
	program->SetModelviewMatrix(ModelviewMatrix);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->positionAttribute);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 3 * numTriangles);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

Uint32 MillisecondsElapsed() {
	static Uint32 lastFrameTick = SDL_GetTicks();
	Uint32 thisFrameTick, delta;
	while (true) {
		// Calculate the number of milliseconds that have elapsed since the last call to this function.
		thisFrameTick = SDL_GetTicks();
		delta = thisFrameTick - lastFrameTick;
		// At 60 FPS, there should be 16.67 milliseconds between frames.
		// If it has been shorter than 16 milliseconds, sleep.
		if (delta < 16) {
			std::this_thread::sleep_for(std::chrono::milliseconds(16 - delta));
		}
		else {
			lastFrameTick = thisFrameTick;
			return delta;
		}
	}
}

//Double checks if Level has all the platforms

bool checkLevel(TileFile& tileFile){
	auto platform = tileFile.GetLayers().find("Platform");
	if (platform == tileFile.GetLayers().end()) {
		return false;
	}
	auto start = tileFile.GetEntities().find("Start");
	if (start == tileFile.GetEntities().end()) {
		return false;
	}
	auto start2 = tileFile.GetEntities().find("Start2");
	if (start2 == tileFile.GetEntities().end()) {
		return false;
	}

	return true;
}

GameState state = MAIN_MENU;


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Init(SDL_INIT_AUDIO);

	displayWindow = SDL_CreateWindow("Ashley & Simon's Final", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	Matrix projectionMatrix;
	projectionMatrix.SetOrthoProjection(-ORTHO_X_BOUND, ORTHO_X_BOUND, -ORTHO_Y_BOUND, ORTHO_Y_BOUND, -1.0f, 1.0f);
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	program->SetProjectionMatrix(projectionMatrix);
	glClearColor(0.0f, 0.3f, 0.6f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float fullscreenVertices[12];
	float fullscreenTextureCoordinates[12];
	Rectangle::SetBox(fullscreenVertices, ORTHO_Y_BOUND, ORTHO_X_BOUND, -ORTHO_Y_BOUND, -ORTHO_X_BOUND);
	Rectangle::SetBox(fullscreenTextureCoordinates, 0.0f, 1.0f, 1.0f, 0.0f);
	
	//Music Setup
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);


	Mix_Music *music;
	music = Mix_LoadMUS(RESOURCE_FOLDER"music.wav");

	Mix_Chunk *walk_music;
	walk_music = Mix_LoadWAV(RESOURCE_FOLDER"walk.wav");

	Mix_Chunk *swish;
	swish = Mix_LoadWAV(RESOURCE_FOLDER"swish.wav");

	//when player gets hit
	Mix_Chunk *thump;
	thump = Mix_LoadWAV(RESOURCE_FOLDER"thump.wav");

	Mix_PlayMusic(music, -1);
	Mix_VolumeMusic(MIX_MAX_VOLUME / 10);
	Mix_VolumeChunk(walk_music, MIX_MAX_VOLUME / 10);

	// Load textures
	GLuint Ticeland = LoadTexture(RESOURCE_FOLDER"spritesheet.png");
	GLuint Tplayer = LoadTexture(RESOURCE_FOLDER"Player.png");
	GLuint TRageplayer = LoadTexture(RESOURCE_FOLDER"RagePlayer.png");
	GLuint THurtplayer = LoadTexture(RESOURCE_FOLDER"HurtPlayer.png");
	GLuint Ttext = LoadTexture(RESOURCE_FOLDER"text.png");
	GLuint Tmenu = LoadTexture(RESOURCE_FOLDER"MainMenu.png");
	GLuint TmapSelect= LoadTexture(RESOURCE_FOLDER"MapSelect.png");


	bool done = false;

	Matrix titleModelViewMatrix;
	Matrix MapSelectMatrix;
	Matrix EndLevelMatrix;

	SDL_Event event;
	Matrix view;

	string Winner;
	float wallVertices1[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
	float wallTexCoords1[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

	while (!done) {

		switch (state) {

		case MAIN_MENU: {
			glClear(GL_COLOR_BUFFER_BIT);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			titleModelViewMatrix.Identity();

			glBindTexture(GL_TEXTURE_2D, Tmenu);
			titleModelViewMatrix.Translate(0.0f, 0.0f, 0.0f);
			titleModelViewMatrix.Scale(34.0f, 20.0f, 1.0);
			program->SetModelviewMatrix(titleModelViewMatrix);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, wallVertices1);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, wallTexCoords1);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_QUIT:
				case SDL_WINDOWEVENT_CLOSE:
					done = true;
					break;

				case SDL_KEYDOWN:
					switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_SPACE:
						state = MAP_SELECT;
						break;

					case SDL_SCANCODE_ESCAPE:
						done = true;
						break;
					}

					break;

				}
			}
			break;
		}
		case MAP_SELECT: {
			glClear(GL_COLOR_BUFFER_BIT);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			MapSelectMatrix.Identity();

			glBindTexture(GL_TEXTURE_2D, TmapSelect);
			MapSelectMatrix.Translate(0.0f, -0.5f, 0.0f);
			MapSelectMatrix.Scale(20.0f, 20.0f, 1.0);
			program->SetModelviewMatrix(MapSelectMatrix);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, wallVertices1);
			glEnableVertexAttribArray(program->positionAttribute);

			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, wallTexCoords1);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_QUIT:
				case SDL_WINDOWEVENT_CLOSE:
					done = true;
					break;

				case SDL_KEYDOWN:
					switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_1:
						state = GAME_LEVEL;
						TILE_FILE = "1.txt";
						break;

					case SDL_SCANCODE_2:
						state = GAME_LEVEL;
						TILE_FILE = "2.txt";
						break;

					case SDL_SCANCODE_3:
						state = GAME_LEVEL;
						TILE_FILE = "3.txt";
						break;

					case SDL_SCANCODE_ESCAPE:
						done = true;
						break;
					}

					break;

				}
			}
			break;
		}
		case GAME_LEVEL:{
			// Level Data
			TileFile tileFile;
			try {
				tileFile = TileFile(std::ifstream(TILE_FILE));
			}
			catch (const TileFile::ParseError& e) {
				cerr << "Unable to parse level data!\n" << e.message << '\n' << e.line << '\n';
				return 1;
			}

			if (!checkLevel(tileFile)) {
				return -1;
			}

			auto platform = tileFile.GetLayers().find("Platform");
			auto start = tileFile.GetEntities().find("Start");
			auto start2 = tileFile.GetEntities().find("Start2");

			vector<Tile> tiles;
			for (unsigned int i = 0; i < tileFile.GetMapHeight(); ++i) {
				for (unsigned int j = 0; j < tileFile.GetMapWidth(); ++j) {
					if (platform->second[i][j] >= 0) {
						tiles.emplace_back(platform->second[i][j], tileFile.RowFromTopToRowFromBottom(i), j);
					}
				}
			}

			float limitX = tileFile.GetMapWidth() * 0.5f - 2 * ORTHO_X_BOUND,
				limitYBottom = tileFile.GetMapHeight() * -0.5f - ORTHO_Y_BOUND;

			glClear(GL_COLOR_BUFFER_BIT);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			Matrix view;
			Player player(tileFile.RowFromTopToRowFromBottom(start->second.begin()->row), start->second.begin()->column + 1, true, walk_music);

			float oldPlayerVelocityY = 0.0f;
			float snowballVelocityY = 0.0f;

			Player player2(tileFile.RowFromTopToRowFromBottom(start2->second.begin()->row), start2->second.begin()->column + 1, false, walk_music);
			float oldPlayer2VelocityY = 0.0f;

			bool gameDone = false;
			// Main game loop
			while (!gameDone) {
				Uint32 mse = MillisecondsElapsed();
				while (SDL_PollEvent(&event)) {
					switch (event.type) {
					case SDL_QUIT:
					case SDL_WINDOWEVENT_CLOSE:
						done = true;
						break;
					case SDL_KEYDOWN:
						switch (event.key.keysym.scancode) {
						case SDL_SCANCODE_SPACE: {
								Mix_PlayChannel(-1, swish, 0);
								player2.activateRage();
								break;
						}
						case SDL_SCANCODE_J: {
							Mix_PlayChannel(-1, swish, 0);
							player.activateRage();
							break;
						}
						case SDL_SCANCODE_ESCAPE:
							state = MAIN_MENU;
							gameDone = true;
							break;
						}
						break;
					}
				}
				if (player.GetCenterY() < limitYBottom) {
					break;
				}
				player.ProcessInput(mse);
				player2.ProcessInput(mse);

				//jump-collision with players

				if (abs(player.GetCenterX() - player2.GetCenterX()) < 0.5 && abs(player.GetCenterY() - player2.GetCenterY()) < 0.5 && abs(player.GetCenterY() - player2.GetCenterY()) > 0.1){
					if (player.GetCenterY() > player2.GetCenterY() && !player.isHurt()) {
						player.bouncePlayer(mse);
						bool continueGame = player2.decreaseHealth();
						if (!continueGame) {
							state = GAME_OVER;
							Winner = "Player 1";
							gameDone = true;
						}
					}
					else if (player.GetCenterY() < player2.GetCenterY() && !player2.isHurt()) {
						player2.bouncePlayer(mse);
						bool continueGame = player.decreaseHealth();
						if (!continueGame) {
							state = GAME_OVER;
							Winner = "Player 2";
							gameDone = true;
						}
					}
				}

				// rage mode
				if (abs(player.GetCenterX() - player2.GetCenterX()) < 0.5 && (player.isRage() || player2.isRage()) && abs(player.GetCenterY() - player2.GetCenterY()) < 0.1){
					if (player.isRage() && !player2.isHurt()) {
						bool continueGame = player2.decreaseHealth();
						if (!continueGame) {
							state = GAME_OVER;
							Winner = "Player 1";
							gameDone = true;
						}
					}
					else if (player2.isRage() && !player.isHurt()) {
						bool continueGame = player.decreaseHealth();
						if (!continueGame) {
							state = GAME_OVER;
							Winner = "Player 2";
							gameDone = true;
						}
					}
				}

				glClear(GL_COLOR_BUFFER_BIT);

				// Draw tiles
				for (const auto& tile : tiles) {
					if (tile.GetLeftBoxBound() < player.GetCenterX() && player.GetCenterX() <= tile.GetRightBoxBound()) {
						// collision with player 2's bottom.
						if (tile.GetBottomBoxBound() < player.GetBottomBoxBound()) {
							register float topBound = tile.GetTopBoxBound();
							if (player.GetBottomBoxBound() <= topBound) {
								player.StayAbove(topBound);
							}
						}
						else if (tile.GetBottomBoxBound() < player.GetTopBoxBound() && player.GetTopBoxBound() <= tile.GetTopBoxBound()) {
							player.StayBelow(tile.GetBottomBoxBound());
						}
					}
					if (tile.GetBottomBoxBound() < player.GetCenterY() && player.GetCenterY() <= tile.GetTopBoxBound()) {
						// Right
						if (tile.GetLeftBoxBound() < player.GetRightBoxBound() && player.GetRightBoxBound() <= tile.GetRightBoxBound()) {
							player.StayToLeftOf(tile.GetLeftBoxBound());
						}
						// Left
						else if (tile.GetLeftBoxBound() < player.GetLeftBoxBound() && player.GetLeftBoxBound() <= tile.GetRightBoxBound()) {
							player.StayToRightOf(tile.GetRightBoxBound());
						}
					}

					if (tile.GetLeftBoxBound() < player2.GetCenterX() && player2.GetCenterX() <= tile.GetRightBoxBound()) {
						// collision with player 2's bottom.
						if (tile.GetBottomBoxBound() < player2.GetBottomBoxBound()) {
							register float topBound = tile.GetTopBoxBound();
							if (player2.GetBottomBoxBound() <= topBound) {
								player2.StayAbove(topBound);
							}
						}
						else if (tile.GetBottomBoxBound() < player2.GetTopBoxBound() && player2.GetTopBoxBound() <= tile.GetTopBoxBound()) {
							player2.StayBelow(tile.GetBottomBoxBound());
						}
					}
					if (tile.GetBottomBoxBound() < player2.GetCenterY() && player2.GetCenterY() <= tile.GetTopBoxBound()) {
						// Right
						if (tile.GetLeftBoxBound() < player2.GetRightBoxBound() && player2.GetRightBoxBound() <= tile.GetRightBoxBound()) {
							player2.StayToLeftOf(tile.GetLeftBoxBound());
						}
						// Left
						else if (tile.GetLeftBoxBound() < player2.GetLeftBoxBound() && player2.GetLeftBoxBound() <= tile.GetRightBoxBound()) {
							player2.StayToRightOf(tile.GetRightBoxBound());
						}
					}

					DrawTrianglesWithTexture(tile.model * view, 2, tile.VERTICES, tile.texture, Ticeland);
				}

				// player based on state
				oldPlayerVelocityY = player.GetVelocityY();
				if (player.isHurt()) {
					DrawTrianglesWithTexture(player.model * view, 2, player.GetVertices(), player.GetTexture(), THurtplayer);
				}
				else if (player.isRage()) {
					DrawTrianglesWithTexture(player.model * view, 2, player.GetVertices(), player.GetTexture(), TRageplayer);
				}
				else
					DrawTrianglesWithTexture(player.model * view, 2, player.GetVertices(), player.GetTexture(), Tplayer);

				oldPlayer2VelocityY = player2.GetVelocityY();
				if (player2.isHurt()) {
					DrawTrianglesWithTexture(player2.model * view, 2, player2.GetVertices(), player2.GetTexture(), THurtplayer);
				}
				else if (player2.isRage()) {
					DrawTrianglesWithTexture(player2.model * view, 2, player2.GetVertices(), player2.GetTexture(), TRageplayer);
				}
				else
					DrawTrianglesWithTexture(player2.model * view, 2, player2.GetVertices(), player2.GetTexture(), Tplayer);

				SDL_GL_SwapWindow(displayWindow);
			}
			break;
		}
		case GAME_OVER: {
			glClear(GL_COLOR_BUFFER_BIT);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			EndLevelMatrix.SetPosition(-7.25, 1, 0);
			program->SetModelviewMatrix(EndLevelMatrix);
			DrawText(program, Ttext, Winner + " has Won the Game. Press Space", 0.60f, -0.10f);

			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_QUIT:
				case SDL_WINDOWEVENT_CLOSE:
					done = true;
					break;

				case SDL_KEYDOWN:
					switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_SPACE:
						glClear(GL_COLOR_BUFFER_BIT);

						state = MAIN_MENU;
						break;

					case SDL_SCANCODE_ESCAPE:
						done = true;
						break;
					}

					break;

				}
			}
			break;
			}
		}
		

		SDL_GL_SwapWindow(displayWindow);

	}
	
	delete program;
	
	Mix_FreeMusic(music);
	Mix_FreeChunk(walk_music);
	Mix_FreeChunk(thump); 
	SDL_Quit();
	return 0;
}
