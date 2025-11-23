#ifndef TESTCLASS_H
#define TESTCLASS_H

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

using namespace std;
using namespace glm;

class Testclass 
{
    public:
        Testclass();
        ~Testclass();
        void msg(string m);

    private:
    vec3 p;

};

#endif