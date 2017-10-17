#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include "Matrix.h"

#include "ShaderProgram.h"
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
GLuint sprite_sheet_tex;
GLuint font_tex;
Matrix viewMatrix;
Matrix modelMatrix;
Matrix projectionMatrix;

ShaderProgram* program;

class Entity;
enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL };
bool running = true;

enum Type { PLAYER, SLIME };

int state;
float lastFrameTicks = 0.0f;
class Entity;
float elapsed;
float last_bullet_time = 0.0f;
float last_ball_time = 0.0f; //slime ball shot
bool left_control = false;
bool right_control = false;
bool shoot_control = false;

GLuint LoadTexture(const char* filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}

void DrawText(ShaderProgram* program, int fontTexture, std::string text, float size, float spacing) {
    float texture_size = 1.0 / 16.0f;
    
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    
    for (size_t i = 0; i < text.size(); i++) {
        float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
        float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
        
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
    glUseProgram(program->programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, font_tex);
    int m = text.size() * 6;
    glDrawArrays(GL_TRIANGLES, 0, m);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}


class Entity {
public:
    Entity() {}
    Entity(float x, float y, float spriteCountX, float spriteCountY, float dx, float dy, float index) {
        spriteCountX = spriteCountX;
        spriteCountY = spriteCountY;
        
        index = index;
        
        pos_x = x;
        pos_y = y;
        speed_x = dx;
        speed_y = dy;
        
        sides_top = y + 0.05f * size;
        sides_bott = y - 0.05f * size;
        sides_left = x - 0.05f * size;
        sides_right = x + 0.05f *size;
        
    }
    
    void draw(){
        
        u = (float)((int)(index) % (int)spriteCountX) / (float) spriteCountX;
        v = (float)(((int)index) / spriteCountY) / (float) spriteCountY;
        w = 1.0/(float)spriteCountX;
        h = 1.0/(float)spriteCountX;
        GLfloat texCoords[] = {
            u, v+h,
            u+w, v,
            u, v,
            u+w, v,
            u, v+h,
            u+w, v+h
        };
        float vertices[] = {-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f,
            -0.5f, 0.5f, -0.5f};
        
        // draw this data
        glUseProgram(program->programID);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->texCoordAttribute);
        glBindTexture(GL_TEXTURE_2D, sprite_sheet_tex);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program->positionAttribute);
        glDisableVertexAttribArray(program->texCoordAttribute);
    }
    
    Matrix entityMatrix;
    float w = 1.0f;
    float h = 1.0f;
    float size = 1.0f;
    float speed_x;
    float speed_y;
    float spriteU;
    float spriteV;
    float spriteCountX = 1.0f;
    float spriteCountY = 1.0f;
    float u;
    float v;
    Type type;
    float pos_x;
    float pos_y;
    float sides_top;
    float sides_bott;
    float sides_left;
    float sides_right;
    float index;
};

//GAME STATES/MODES

Entity user;
std::vector<Entity> minions;
std::vector<Entity> bullets;
std::vector<Entity> slimeballs;

void RenderMain() {
    modelMatrix.Identity();
    modelMatrix.Translate(0.0f, 1.5f, 0.0f);
    program->SetModelviewMatrix(modelMatrix);
    DrawText(program, font_tex, "SPACE INVADERS", 0.2f, 0.0001f);
    modelMatrix.Identity();
    modelMatrix.Translate(-1.6f, 0.0f, 0.0f);
    program->SetModelviewMatrix(modelMatrix);
    DrawText(program, font_tex, "USE ARROW KEYS TO MOVE, SPACE TO FIRE", 0.1f, 0.0001f);
    modelMatrix.Identity();
    modelMatrix.Translate(-1.6f, -1.0f, 0.0f);
    program->SetModelviewMatrix(modelMatrix);
    DrawText(program, font_tex, "PRESS SPACE TO START", 0.1f, 0.0001f);
}

void RenderLevel() {
    user.draw();
    for (size_t i = 0; i < bullets.size(); i++) {
        bullets[i].draw();
    }
    for (size_t i = 0; i < slimeballs.size(); i++) {
        slimeballs[i].draw();
    }
    
    for (size_t i = 0; i < minions.size(); i++) {
        minions[i].draw();
    }
}

void UpdateGame(float elapsed) {
    if (right_control) {
        user.pos_x += elapsed * user.speed_x;
        user.sides_left += user.speed_x * elapsed;
        user.sides_right += elapsed * user.speed_x;
    }
    else if (left_control){
        user.pos_x -= elapsed * user.speed_x;
        user.sides_left -= user.speed_x * elapsed;
        user.sides_right -= elapsed * user.speed_x;
    }
    
    if (shoot_control && last_bullet_time > 0.3f) {
        last_bullet_time=0.0f;
        bullets.push_back(Entity(user.pos_x, user.pos_y, 16, 8, 0.0f, 4.0f,98));
    }
    
//    std::vector<int> removeBullets;
//    
//    for (int i = 0; i < bullets.size(); i++) {
//        bullets[i].pos_y += elapsed * bullets[i].speed_y;
//        bullets[i].sides_top += bullets[i].speed_y * elapsed;
//        bullets[i].sides_bott += bullets[i].speed_y * elapsed;
//        
//        for (int j = 0; j < minions.size(); j++) {
//            if(minions[j].sides_bott < bullets[i].sides_top && minions[j].sides_top > bullets[i].sides_bott && minions[j].sides_left < bullets[i].sides_right && minions[j].sides_right > bullets[i].sides_left){
//                removeBullets.push_back(i);
//                minions.erase(minions.begin()+j);
//            }
//        }
//    }
//    
//    for (int i = 0; i < removeBullets.size(); i++){
//        bullets.erase(bullets.begin() + removeBullets[i] - i);
//    }
//    
//    for (size_t i = 0; i < minions.size(); i++) {
//        minions[i].pos_y -= elapsed*minions[i].speed_y;
//        minions[i].pos_x += elapsed*minions[i].speed_x;
//        
//        minions[i].sides_bott -= minions[i].speed_y*elapsed;
//        minions[i].sides_top -= minions[i].speed_y*elapsed;
//        
//        minions[i].sides_left += minions[i].speed_x*elapsed;
//        minions[i].sides_right += minions[i].speed_x*elapsed;
//        
//        if(minions[i].sides_bott < user.sides_top && minions[i].sides_top > user.sides_bott && minions[i].sides_left < user.sides_right && minions[i].sides_right > user.sides_left){
//            running = false;
//        }
//        
//        if ((minions[i].sides_left < -3.3f && minions[i].speed_x < 0)
//            ||(minions[i].sides_right > 3.3f && minions[i].speed_x)) {
//            for(int i = 0; i < minions.size(); i++){
//                minions[i].speed_x = -minions[i].speed_x;
//            }
//        }
//    }
//    
//    if (last_ball_time > 0.5f) {
//        last_ball_time = 0.0f;
//        int slime_shooter = rand()%minions.size();
//        slimeballs.push_back(Entity(minions[slime_shooter].pos_x, minions[slime_shooter].pos_y, 16, 8, 0, -2.0f, 22));
//    }
//    
//    for (size_t i = 0; i < slimeballs.size(); i++) {
//        slimeballs[i].pos_y += slimeballs[i].speed_y*elapsed;
//        slimeballs[i].sides_top += slimeballs[i].speed_y*elapsed;
//        slimeballs[i].sides_bott += slimeballs[i].speed_y*elapsed;
//        if(slimeballs[i].sides_bott < user.sides_top && slimeballs[i].sides_top > user.sides_bott && slimeballs[i].sides_left < user.sides_right && slimeballs[i].sides_right > user.sides_left){
//            running = false;
//        }
//    }
//    
//    if (minions.size() == 0){
//        running = false;
//    }
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    switch(state) {
        case STATE_MAIN_MENU:
            RenderMain();
            break;
        case STATE_GAME_LEVEL:
            RenderLevel();
            break;
    }
    SDL_GL_SwapWindow(displayWindow);
}

void Update(float elapsed) {
    switch (state) {
        case STATE_MAIN_MENU:
            break;
        case STATE_GAME_LEVEL:
            UpdateGame(elapsed);
            break;
    }
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 650, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    SDL_Event event;
    program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    bool done = false;
    
    projectionMatrix.SetOrthoProjection(-4.0, 4.0, -2.25f, 2.25f, -1.0f, 1.0f);
    program->SetModelviewMatrix(modelMatrix);
    program->SetProjectionMatrix(projectionMatrix);
    program->SetModelviewMatrix(viewMatrix);
    
    font_tex=LoadTexture("/Users/ashleylee/Desktop/CS3113/Assignment #3/NYUCodebase/sprite/character/sprites.png");
    font_tex=LoadTexture("/Users/ashleylee/Desktop/CS3113/Assignment #3/NYUCodebase/sprite/fonts/font1.png");
    
    user = Entity(0.0f, -1.0f, 16, 8, 3.0f, 0, 98);
    
    for (int i =0; i < 55; i++){
        Entity minion(-1.5 + (i % 11) * 0.5, 1.0 - (i / 11 * 0.5), 16, 8, 1.0f, 0.03f, 102);
        minions.push_back(minion);
    }
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            switch(event.type){
                case SDL_KEYDOWN:
                    if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                        if (state == STATE_MAIN_MENU) {
                            state = STATE_GAME_LEVEL;
                        }
                        else {
                            shoot_control = true;
                        }
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_LEFT && user.sides_left > -3.0f) {
                        left_control = true;
                    }
                    else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT && user.sides_right < 3.0f) {
                        right_control = true;
                    }
                    break;
                case SDL_KEYUP:
                    if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                        left_control = false;
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                        right_control = false;
                    }
                    if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                        shoot_control = false;
                    }
                    break;
            }
            
        }
        
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        elapsed = ticks - lastFrameTicks;
        last_bullet_time += elapsed;
        last_ball_time += elapsed;
        lastFrameTicks = ticks;
        if (running) {
            Update(elapsed);
            Render();
        }
    }
    
    SDL_Quit();
    return 0;
}
