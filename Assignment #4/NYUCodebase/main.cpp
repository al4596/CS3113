#ifdef _WINDOWS
    #include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ShaderProgram.h"
#include "Matrix.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6


using namespace std;

SDL_Window* displayWindow;

GLint LoadTexture(const char *filePath) {
    int w, h, comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image." << std::endl;
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return retTexture;
}

class Entity {
public:
    Entity(float height, float width, float posX, float posY, float velX = 0.0, float velY = 0.0, float accX = 0.0);
    bool collision(Entity* entity);
    float top;
    float bottom;
    float left;
    float right;
    float height;
    float width;
    float posX;
    float posY;
    float velX;
    float velY;
    float accX;
    bool toLeft;
    bool toRight;
    
    bool jump;
    bool fall;
    bool down;
    bool collide;
};

Entity::Entity(float hgt, float wdt, float pos_X, float pos_Y, float vel_X, float vel_Y, float acc_X) {
    height = hgt;
    width = wdt;
    posX = pos_X;
    posY = pos_Y;
    top = posY + hgt/2;
    bottom = posY - hgt/2;
    left = posX - wdt/2;
    right = posX + wdt/2;
    velX = vel_X;
    velY = vel_Y;
    accX = acc_X;
    toLeft = false;
    toRight = false;
    
    collide = false;
    jump = false;
    fall = false;
    down = false;
}

bool Entity::collision(Entity* entity) {
    top = posY + height/2;
    bottom = posY - height/2;
    left = posX - width/2;
    right = posX + width/2;
    
    collide = !(bottom > entity->top ||  top < entity->bottom || left > entity->right || right < entity->left);
    
    return (collide);
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Ashley Lee - Homework 4", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 360);
    
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    GLuint platformTexture = LoadTexture(RESOURCE_FOLDER"yellow-block.png");
    GLuint playerTexture = LoadTexture(RESOURCE_FOLDER"luma.png");
    
    Matrix projectionMatrix;
    Matrix playerMatrix;
    Matrix platform1Matrix;
    Matrix platform2Matrix;
    Matrix viewMatrix;
    Matrix modelviewMatrix;
    
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    SDL_Event event;
    bool done = false;
    float lastFrameTicks = 0;
    float gravity = -3.5f;
    
    Entity* player = new Entity(0.5, 0.5, 0.0, 0.9);
    Entity* platform1 = new Entity(0.4, 40.0, 0.0, -1.00);
    Entity* platform2 = new Entity(0.3, 0.5, 0.0, -0.27);

    platform1->fall = false;
    platform2->fall = false;
    player->fall = true;
    
    while (!done) {
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        viewMatrix.Identity();
        viewMatrix.Translate(-player->posX, -player->posY, 0.0f);
        program.SetModelviewMatrix(viewMatrix);
        
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            if(event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
                    player->jump = true;
                    player->velY = 2.5f;
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                    player->down = true;
                    player->velY = -3.5f;
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                    player->toLeft = true;
                    player->velX = -3.5f;
                    player->accX = -1.0f;
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                    player->toRight = true;
                    player->velX = 3.5f;
                    player->accX = 1.0f;
                }
            }
            
            if (event.type == SDL_KEYUP) {
                player->toLeft = false;
                player->toRight = false;
                player->down = false;
                player->jump = false;
                player->fall = true;
            }
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        program.SetModelviewMatrix(playerMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetModelviewMatrix(viewMatrix);
        
        platform1Matrix.Identity();
        platform1Matrix.Translate(platform1->posX, platform1->posY, 0.0f);
        program.SetModelviewMatrix(platform1Matrix);
        glBindTexture(GL_TEXTURE_2D, platformTexture);
        
        float platform1Vertices[] = {-8, .2, -8, -5, 8, -5, -8, .2, 8, -5,8, .2};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, platform1Vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        float platform1TexCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, platform1TexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        platform2Matrix.Identity();
        platform2Matrix.Translate(platform2->posX, platform2->posY, 0.0f);
        
        program.SetModelviewMatrix(platform2Matrix);
        
        glBindTexture(GL_TEXTURE_2D, platformTexture);
        
        float platform2Vertices[] ={-1, .1, -1, -.08, 1, -.08, -1, .1, 1, -.08, 1, .1};
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, platform2Vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float platform2TexCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, platform2TexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        if (player->fall || player->down) {
            player->velY += gravity * elapsed;
            player->posY += player->velY * elapsed;
        }
        
        if (player->collision(platform1) || player->collision(platform2)) {
            if (player->top > platform1->bottom) {
                float penetration = fabsf((player->posY - platform1->posY) - (platform1->height/2) - (player->height/2));
                player->posY += penetration + .0003;
                player->velY *= -1;
            }
            else if (player->top > platform1->bottom) {
                float penetration = fabsf((player->posY - platform2->posY) - (platform1->height/2) - (player->height/2));
                player->posY += penetration + .0003;
                player->velY *= -1;
            }
            player->fall = false;
        }
        
        if (player->toLeft || player->toRight) {
            player->velX += player->accX * elapsed;
            player->posX += player->velX * elapsed;
        }
        
        if (player->jump) {
            player->velY += elapsed * gravity;
            player->posY += player->velY * elapsed;
        }
        
        playerMatrix.Identity();
        playerMatrix.Translate(player->posX,player->posY, 0.0f);
        
        program.SetModelviewMatrix(playerMatrix);
        
        glBindTexture(GL_TEXTURE_2D, playerTexture);
        
        float playerVertices[] = {-0.25, 0.25, -0.25, -0.25, 0.25, -0.25, -0.25, 0.25, 0.25, -0.25, 0.25, 0.25};
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, playerVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float playerTexCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, playerTexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        glBindTexture(GL_TEXTURE_2D, playerTexture);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        
        SDL_GL_SwapWindow(displayWindow);
        
    }
    
    SDL_Quit();
    return 0;
}
