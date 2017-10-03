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

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Ashley Lee - Assignment 1" , SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif
    
    glViewport(0, 0, 640, 360);
    
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    GLuint brick_texture = LoadTexture(RESOURCE_FOLDER"brick.png");
    GLuint roof_texture = LoadTexture(RESOURCE_FOLDER"roof.png");
    GLuint ground_texture = LoadTexture(RESOURCE_FOLDER"grass.png");
    
    Matrix projectionMatrix;
    Matrix modelviewMatrix;
    projectionMatrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
    Matrix viewMatrix;

    glEnable(GL_BLEND);
    
	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true; 
			}
		}
        
        glClearColor(0.4f, 0.2f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(program.programID);
        
        program.SetProjectionMatrix(projectionMatrix);
        program.SetModelviewMatrix(viewMatrix);
        
        
        //Brick Wall
        
        modelviewMatrix.Identity();
        
        glBindTexture(GL_TEXTURE_2D, brick_texture);
        modelviewMatrix.Translate(0.0f, -0.5f, 0.0f);
        modelviewMatrix.Scale(2.0f, 1.0f, 1.0);
        program.SetModelviewMatrix(modelviewMatrix);
        
        float wallVertices1[] = {0.5f, -0.5f, 0.5, 0.5, -0.5f, -0.5f, 0.5, 0.5, -0.5, 0.5, -0.5f, -0.5f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, wallVertices1);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float wallTexCoords1[] = {0.0, 0.0, 0.0,1.0,1.0,0.0,0.0,1.0,1.0,1.0,1.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, wallTexCoords1);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        //Roof
        
        glBindTexture(GL_TEXTURE_2D, roof_texture);
        modelviewMatrix.Translate(0.0f, -0.5f, 0.0f);
        //modelviewMatrix.Scale(1.0f, 1.0f, 1.0f);
        program.SetModelviewMatrix(modelviewMatrix);
        
        float roofVertices[] = {0.8f, 1.0f, -0.8f, 1.0f, 0.0f, 2.0f};
        float roofTexCoords[] = {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, roofVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, roofTexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        //Ground
        
        modelviewMatrix.Identity();
        
        glBindTexture(GL_TEXTURE_2D, ground_texture);
        modelviewMatrix.Translate(0.0f, -2.0f, 0.0f);
        modelviewMatrix.Scale(1.0f, 1.0f, 1.0f);
        program.SetModelviewMatrix(modelviewMatrix);
        
        float groundVertices[] = {-4.0f, 1.0f, 4.0, 1.0, 4.0f, -1.0f, 4.0f, -1.0f, -4.0f, -1.0f, -4.0f, 1.0f};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, groundVertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float groundTexCoords[] = {1.0, 1.0, 1.0, 0.0, 0.0, 0.0,1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, groundTexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
