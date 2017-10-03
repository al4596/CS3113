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

SDL_Window* displayWindow;

class Ball {
public:
    Ball(float pos_x, float pos_y, float speed, float acc, float d_x, float d_y): pos_x(pos_x), pos_y(pos_y), speed(speed), acc(acc), d_x(d_x), d_y(d_y) {}
    float pos_x = 0.0f;
    float pos_y = 0.0f;
    float speed = 0.02f;
    float acc = 0.001f;
    float d_x = (float)(rand()% 5 +1);
    float d_y = (float)(rand()%10 - 4);
    
    void reset() {
        pos_x = 0.0f;
        pos_y = 0.0f;
        speed = 0.05f;
        d_x = (float)rand();
        d_y = (float)rand();
    }
    
    void move(float elapsed) {
        pos_x += (elapsed*speed*d_x);
        pos_y += (elapsed*speed*d_y);
    }
};

class Paddle {
public:
    Paddle(float left, float right, float top, float bottom) : left(left), right(right), top(top), bottom(bottom) {}
    float left;
    float right;
    float bottom;
    float top;
};

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

ShaderProgram SetUp() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Ashley Lee - Assignment 2" , SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    #ifdef _WINDOWS
        glewInit();
    #endif
    
    glViewport(0, 0, 640, 360);
    
    return ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
}

int main(int argc, char *argv[])
{
    
    ShaderProgram program = SetUp();
    
    Matrix projectionMatrix;
    Matrix paddleRightMatrix;
    Matrix paddleLeftMatrix;
    Matrix ballMatrix;
    Matrix viewMatrix;
    Matrix modelviewMatrix;
    
    GLuint puppy_texture = LoadTexture(RESOURCE_FOLDER"puppy.png");
    GLuint white_texture = LoadTexture(RESOURCE_FOLDER"white.png");

    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    
    float lastFrameTicks = 0.0f;
    
    Paddle paddleLeft(-1.7f, -1.6f, 0.5f, -0.5f);
    Paddle paddleRight(1.6f, 1.7f, 0.5f, -0.5f);
    Ball ball = Ball(0.0f, 0.0f, 0.01f, 0.001f, (float)rand(), (float)rand());
    
    SDL_Event event;
    bool done = false;
    bool running = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClear(GL_COLOR_BUFFER_BIT);
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            if (event.type == SDL_KEYDOWN) {
                if (!running && event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    running = true;
                }
                
                //left paddle
                if (event.key.keysym.scancode == SDL_SCANCODE_W && paddleLeft.top < 3.0f){
                    paddleLeft.top += 0.3f;
                    paddleLeft.bottom += 0.3f;
                    paddleLeftMatrix.Translate(0.0f, 0.3f, 0.0f);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_S && paddleLeft.bottom > -3.0f){
                    paddleLeft.top -= 0.3f;
                    paddleLeft.bottom -= 0.3f;
                    paddleLeftMatrix.Translate(0.0f, 0.3f, 0.0f);
                }
                
                //right paddle
                if (event.key.keysym.scancode == SDL_SCANCODE_UP && paddleRight.top < 3.0f) {
                    paddleRight.top += 0.3f;
                    paddleRight.bottom += 0.3f;
                    paddleRightMatrix.Translate(0.0f, 0.3f, 0.0f);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN && paddleRight.bottom > -3.0f) {
                    paddleRight.top -= 0.3f;
                    paddleRight.bottom -= 0.3f;
                    paddleRightMatrix.Translate(0.0f, -0.3f, 0.0f);
                }
            }
        }
        
        //Draw
        
        glUseProgram(program.programID);
        
        program.SetProjectionMatrix(projectionMatrix);
        program.SetModelviewMatrix(viewMatrix);
        program.SetModelviewMatrix(paddleLeftMatrix);
        
        float texCoords1[] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
        
        //Puppy Ball
        program.SetModelviewMatrix(ballMatrix);
        modelviewMatrix.Identity();
        glBindTexture(GL_TEXTURE_2D, puppy_texture);
        program.SetModelviewMatrix(modelviewMatrix);
        
        float puppyVertices[] = {-0.2f, 0.2f, -0.2f, -0.2f, 0.2f, -0.2f, 0.2f, -0.2f, 0.2f, 0.2f, -0.2f, 0.2f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, puppyVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords1);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glBindTexture(GL_TEXTURE_2D, puppy_texture);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        //Paddle 1
        
        program.SetModelviewMatrix(paddleLeftMatrix);
        
        float paddleVertices1[] = {-3.0f, 1.0f, -3.0f, -1.0f, -2.0f, -1.0f, -2.0f, -1.0f, -2.0f, 1.0f, -3.0f, 1.0f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, paddleVertices1);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords1);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glBindTexture(GL_TEXTURE_2D, white_texture);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        //Paddle 2
        
        program.SetModelviewMatrix(paddleRightMatrix);
        
        float paddleVertices2[] = {1.2, -0.2, 1.2, -0.2, 1.2, 0.2, 1.2, 0.2, 1.2, 0.2, 1.2, -0.2};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, paddleVertices2);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords1);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glBindTexture(GL_TEXTURE_2D, white_texture);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);

        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        if (running) {
            if ((ball.pos_x <=paddleLeft.right && ball.pos_y <= paddleLeft.top && ball.pos_y >= paddleLeft.bottom) || (ball.pos_x >= paddleRight.left && ball.pos_y <= paddleRight.top && ball.pos_y >= paddleRight.bottom)) {
                ball.d_x *= -1;
                ball.speed += ball.acc * elapsed;
                ball.move(elapsed);
                ballMatrix.Translate((ball.d_x*elapsed*ball.speed), (ball.d_y*elapsed*ball.speed), 0.0f);
            } else if (ball.pos_x >= paddleRight.right) {
                running = false;
                ballMatrix.Translate(-ball.pos_x, -ball.pos_y, 0.0f);
                ball.reset();
            } else if (ball.pos_x <= paddleLeft.left) {
                running = false;
                ballMatrix.Translate(-ball.pos_x, -ball.pos_y, 0.0f);
                ball.reset();
            } else if (ball.pos_y + 0.1f >= 2.0f || ball.pos_y - 0.1f <= -2.0f) {
                ball.d_y *= -1;
                ball.speed += ball.acc * elapsed;
                ball.move(elapsed);
                ballMatrix.Translate((ball.speed * ball.d_x * elapsed), (ball.speed * ball.d_y * elapsed), 0.0f);
            } else {
                ball.move(elapsed);
                ballMatrix.Translate(((ball.speed)*ball.d_x*elapsed), (ball.speed * ball.d_y*elapsed), 0.0f);
            }
        }
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
