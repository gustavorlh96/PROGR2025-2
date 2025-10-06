#ifndef SPRITE_H
#define SPRITE_H

#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW - porque inclui a OpenGL
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

class Sprite
{
    public:
        Sprite();
        ~Sprite();
        void initialize(GLuint shaderID, GLuint texID, int nAnimations, int nFrames, vec3 pos, vec3 dimensions, float angle = 0.0);
        void update();
        void draw();
        void moveRight();
        void moveLeft();

        inline void setFrame(int f) { if (f < 0) f = 0; if (f >= nFrames) f = nFrames - 1; iFrames = f; }
        inline void setAnimationRow(int a) { if (a < 0) a = 0; if (a >= nAnimations) a = nAnimations - 1; iAnimations = a; }
        inline void setPosition(const vec3 &p) { pos = p; }
        inline void setDimensions(const vec3 &d3) { dimensions = d3; }
        inline void setAngle(float a) { angle = a; }
        inline void setAnchor(const vec2 &a) { anchor = a; }

    private:
        GLuint VAO; //id do VAO da geometria
        GLuint texID; //id da textura
        vec3 pos; //posicao
        vec3 dimensions; //fatores de escala
        float angle; //angulo para rotação no eixo z
        GLuint shaderID; //para acessar o shader
        int nAnimations, nFrames, iAnimations, iFrames;
        vec2 d;
        vec2 anchor;

        float lastTime, FPS;
        float vel;

        GLuint setupGeometry();

};

#endif