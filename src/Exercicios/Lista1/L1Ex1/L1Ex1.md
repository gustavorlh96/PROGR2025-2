# Lista 1 - Exercício 1

## Aluno
- Gustavo Rodrigues Lima Haag  

---

## Questão e Resposta

O que é a GLSL? Quais os dois tipos de shaders são obrigatórios no pipeline programável da versão atual que trabalhamos em aula e o que eles processam? 

> O GLSL (OpenGL Shading Language) é a linguagem de programação específica que se usa para escrever _shaders_ em OpenGL. É uma linguagem de alto nível, e um tanto parecido com C, feita para controlar partes do pipeline gráfico. Ela permite escrever código que roda na GPU para fazer transformações, luz, cor, etc.  
> &nbsp;&nbsp;&nbsp;&nbsp;No OpenGL mais moderno (3.3+ em frente, usamos o 4.0 em aula), dois tipos de shaders são obrigatórios: O _Vertex Shader_, que recebe cada vértice de entrada para realizar tarefas como transformação de coordenadas, calcular atributos, etc. O segundo é o _Fragment Shader_, que recebe fragmentos (tipo "candidatos a pixels" depois da rasterização das primitivas) e calcula cor, pode também usar texturas, luz, sombras, etc. Ele é quem decide o valor final de cor de cada fragmento que vai poder se tornar pixel.