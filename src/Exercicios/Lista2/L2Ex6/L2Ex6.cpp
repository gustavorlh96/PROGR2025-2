/* 
* Processamento Gráfico 2025/2
* Lista 2 - Exercicio 6 - Criador de triângulos
* Aluno: Gustavo Haag
* Código original: Professora Rossana Baptista Queiroz
*/

#include <iostream>
#include <string>
#include <assert.h>
#include <vector>
#include <cmath>

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

int setupShader();
int setupGeometry();
void updateProjection(int width, int height);
void uploadBufferIfNeeded();
vec3 colorForTriangle(int index);

const GLuint WIDTH = 800, HEIGHT = 600;

// Variáveis globais para gerenciar os triângulos criados
static GLuint gShaderID = 0;
static GLuint gVAO = 0;
static GLuint gVBO = 0;
static GLint gProjectionLoc = -1;
static int gFBWidth = WIDTH;
static int gFBHeight = HEIGHT;
static std::vector<GLfloat> gVertices; // intervalados: x,y,z, r,g,b
static vec3 gCurrentColor = vec3(1.0, 1.0, 1.0);
static int gTriangleCount = 0;
static bool gNeedBufferUpdate = false;

const GLchar *vertexShaderSource = R"(
 #version 400
 layout (location = 0) in vec3 position;
 layout (location = 1) in vec3 color;
 out vec3 vColor; 
 uniform mat4 projection;
 void main()
 {
	 gl_Position = projection * vec4(position.x, position.y, position.z, 1.0);
	 vColor = color;
 }
 )";

const GLchar *fragmentShaderSource = R"(
 #version 400
 in vec3 vColor;
 out vec4 color;
 void main()
 {
	 color = vec4(vColor,1.0);
 }
 )";

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Exercicio 6 - Criador de triângulos", nullptr, nullptr);
	if (!window) {
		std::cerr << "Falha ao criar a janela GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Falha ao inicializar GLAD" << std::endl;
		return -1;
	}

	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	GLuint shaderID = setupShader();
	GLuint VAO = setupGeometry();

	glUseProgram(shaderID);

	// Guarda as variáveis globais
	gShaderID = shaderID;

	double prev_s = glfwGetTime();
	double title_countdown_s = 0.1;

	// Query inicial do tamanho do framebuffer e configuração da projeção
	glfwGetFramebufferSize(window, &gFBWidth, &gFBHeight);
	updateProjection(gFBWidth, gFBHeight);

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window)) {
		{
			double curr_s = glfwGetTime();
			double elapsed_s = curr_s - prev_s;
			prev_s = curr_s;

			title_countdown_s -= elapsed_s;
			if (title_countdown_s <= 0.0 && elapsed_s > 0.0) {
				double fps = 1.0 / elapsed_s;
				char tmp[256];
				sprintf(tmp, "Exercicio 6 - Criador de triângulos\tFPS %.2lf", fps);
				glfwSetWindowTitle(window, tmp);

				title_countdown_s = 0.1;
			}
		}
		glfwPollEvents();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);


		// Isso mantém o viewport e a projeção atualizados se a janela for redimensionada
		glfwGetFramebufferSize(window, &gFBWidth, &gFBHeight);
		glViewport(0, 0, gFBWidth, gFBHeight);
		updateProjection(gFBWidth, gFBHeight);

		// Desenha todos os triângulos completos
		glBindVertexArray(VAO);
		uploadBufferIfNeeded();
		GLsizei vertexCount = static_cast<GLsizei>(gVertices.size() / 6);
		GLsizei completeVertices = (vertexCount / 3) * 3;
		if (completeVertices > 0) {
			glDrawArrays(GL_TRIANGLES, 0, completeVertices);
		}
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &gVBO);
	glDeleteProgram(gShaderID);
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

int setupShader() {
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(shaderProgram);
	gProjectionLoc = glGetUniformLocation(shaderProgram, "projection");

	return shaderProgram;
}

int setupGeometry() {
	// Começando com um buffer dinâmico vazio; os vértices serão adicionados pelos cliques do mouse
	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);

	glGenBuffers(1, &gVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return gVAO;
}

void updateProjection(int width, int height) {
	if (width <= 0 || height <= 0) return;
	mat4 projection = ortho(0.0, static_cast<double>(width), static_cast<double>(height), 0.0, -1.0, 1.0);
	glUseProgram(gShaderID);
	glUniformMatrix4fv(gProjectionLoc, 1, GL_FALSE, value_ptr(projection));
}

void uploadBufferIfNeeded() {
	if (!gNeedBufferUpdate) return;
	glBindBuffer(GL_ARRAY_BUFFER, gVBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(gVertices.size() * sizeof(GLfloat)), gVertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	gNeedBufferUpdate = false;
}

// HSV simples para RGB para gerar cores distintas por triângulo (S=0.7, V=1.0)
static vec3 hsv2rgb(double h, double s, double v) {
	double c = v * s;
	double hp = fmod(h * 6.0, 6.0);
	double x = c * (1.0 - fabs(fmod(hp, 2.0) - 1.0));
	double r = 0.0, g = 0.0, b = 0.0;
	if (0.0 <= hp && hp < 1.0) { r = c; g = x; b = 0.0; }
	else if (1.0 <= hp && hp < 2.0) { r = x; g = c; b = 0.0; }
	else if (2.0 <= hp && hp < 3.0) { r = 0.0; g = c; b = x; }
	else if (3.0 <= hp && hp < 4.0) { r = 0.0; g = x; b = c; }
	else if (4.0 <= hp && hp < 5.0) { r = x; g = 0.0; b = c; }
	else { r = c; g = 0.0; b = x; }
	double m = v - c;
	return vec3(r + m, g + m, b + m);
}

vec3 colorForTriangle(int index) {
	// Usa a razão áurea para variar o matiz e obter cores mais distribuídas
	const double golden = 0.6180339887498949;
	double hue = fmod(index * golden, 1.0);
	return hsv2rgb(hue, 0.7, 1.0);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		int currentVertexCount = static_cast<int>(gVertices.size() / 6);
		if (currentVertexCount % 3 == 0) {
			gCurrentColor = colorForTriangle(gTriangleCount);
		}

		gVertices.push_back(static_cast<GLfloat>(xpos));
		gVertices.push_back(static_cast<GLfloat>(ypos));
		gVertices.push_back(0.0);
		gVertices.push_back(static_cast<GLfloat>(gCurrentColor.r));
		gVertices.push_back(static_cast<GLfloat>(gCurrentColor.g));
		gVertices.push_back(static_cast<GLfloat>(gCurrentColor.b));

		currentVertexCount += 1;
		if (currentVertexCount % 3 == 0) {
			gTriangleCount += 1;
		}

		gNeedBufferUpdate = true;
	}
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	gFBWidth = width;
	gFBHeight = height;
	glViewport(0, 0, width, height);
	updateProjection(width, height);
}