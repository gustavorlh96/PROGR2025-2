#include "TestClass.h"

Testclass::Testclass():
p(vec3(0.0,0.0,0.0))
{
    cout << "Atributo p inicializado com os valores (" << p.x << "," << p.y << "," << p.z << ")" << endl; 
}

Testclass::~Testclass()
{
    cout << "Objeto eliminado" << endl;
}

void Testclass::msg(string m)
{
    cout << m << endl;
}
