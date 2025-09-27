/* 
* Processamento Gráfico 2025/2
* Lista 1 - Exercicio 9 - Desenho com primitivas
* Aluno: Gustavo Haag
* Código original: Professora Rossana Baptista Queiroz
*/

#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

int setupShader();
int setupGeometry();

const GLuint WIDTH = 800, HEIGHT = 800;

const GLchar *vertexShaderSource = R"(
 #version 400
 layout (location = 0) in vec3 position;
 void main()
 {
	 gl_Position = vec4(position.x, position.y, position.z, 1.0);
 }
 )";

const GLchar *fragmentShaderSource = R"(
 #version 400
 uniform vec4 inputColor;
 out vec4 color;
 void main()
 {
	 color = inputColor;
 }
 )";

int main() {
	glfwInit();

	 glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	 glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	 glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	 glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Exercicio 9 - Desenho com primitivas", nullptr, nullptr);
	if (!window) {
		std::cerr << "Falha ao criar a janela GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Falha ao inicializar GLAD" << std::endl;
		return -1;
	}

	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	GLuint shaderID = setupShader();

	GLuint VAO = setupGeometry();

	GLint colorLoc = glGetUniformLocation(shaderID, "inputColor");

	glUseProgram(shaderID);

	double prev_s = glfwGetTime();
	double title_countdown_s = 0.1;

	while (!glfwWindowShouldClose(window)) {
		{
			double curr_s = glfwGetTime();
			double elapsed_s = curr_s - prev_s;
			prev_s = curr_s;

			title_countdown_s -= elapsed_s;
			if (title_countdown_s <= 0.0 && elapsed_s > 0.0)
			{
				double fps = 1.0 / elapsed_s;

				char tmp[256];
				sprintf(tmp, "Exercicio 9 - Desenho com primitivas\tFPS %.2lf", fps);
				glfwSetWindowTitle(window, tmp);

				title_countdown_s = 0.1;
			}
		}
		glfwPollEvents();

		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		{
			// Telhado vermelho, triângulo
			GLfloat roofVertices[] = {
				0.0, 0.8, 0.0,
				0.7, 0.1, 0.0,
				-0.7, 0.1, 0.0
			};
			GLuint roofVAO, roofVBO;
			glGenVertexArrays(1, &roofVAO);
			glGenBuffers(1, &roofVBO);
			glBindVertexArray(roofVAO);
			glBindBuffer(GL_ARRAY_BUFFER, roofVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(roofVertices), roofVertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);
			glUniform4f(colorLoc, 0.5, 0.0, 0.0, 1.0); // vermelho escuro
			glDrawArrays(GL_TRIANGLES, 0, 3);

			// Janela amarela - retângulo usando dois triângulos
			GLfloat windowFillVertices[] = {
				-0.4, -0.2, 0.0,
				-0.1, -0.2, 0.0,
				-0.4,  0.0, 0.0,
				-0.4,  0.0, 0.0,
				-0.1, -0.2, 0.0,
				-0.1,  0.0, 0.0
			};
			GLuint windowFillVAO, windowFillVBO;
			glGenVertexArrays(1, &windowFillVAO);
			glGenBuffers(1, &windowFillVBO);
			glBindVertexArray(windowFillVAO);
			glBindBuffer(GL_ARRAY_BUFFER, windowFillVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(windowFillVertices), windowFillVertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);
			glUniform4f(colorLoc, 1.0, 1.0, 0.0, 1.0); // amarelo
			glDrawArrays(GL_TRIANGLES, 0, 6);

			// Porta marrom - retângulo usando dois triângulos
			GLfloat doorFillVertices[] = {
				0.0, -0.4, 0.0,
				0.2, -0.4, 0.0,
				0.0,  0.0, 0.0,
				0.0,  0.0, 0.0,
				0.2, -0.4, 0.0,
				0.2,  0.0, 0.0
			};
			GLuint doorFillVAO, doorFillVBO;
			glGenVertexArrays(1, &doorFillVAO);
			glGenBuffers(1, &doorFillVBO);
			glBindVertexArray(doorFillVAO);
			glBindBuffer(GL_ARRAY_BUFFER, doorFillVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(doorFillVertices), doorFillVertices, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			glEnableVertexAttribArray(0);
			glUniform4f(colorLoc, 0.4, 0.2, 0.0, 1.0); // marrom
			glDrawArrays(GL_TRIANGLES, 0, 6);

			// Limpa VAOs/VBOs temporários a cada frame
			glDeleteVertexArrays(1, &roofVAO);
			glDeleteBuffers(1, &roofVBO);
			glDeleteVertexArrays(1, &windowFillVAO);
			glDeleteBuffers(1, &windowFillVBO);
			glDeleteVertexArrays(1, &doorFillVAO);
			glDeleteBuffers(1, &doorFillVBO);
			glBindVertexArray(0);
		}
	
		glEnable(GL_LINE_SMOOTH); // Não consegui engrossar a linha, mas pelo menos assim parece um pouco melhor

		// Contorno preto da casa
		glBindVertexArray(VAO);
		glUniform4f(colorLoc, 0.0, 0.0, 0.0, 1.0);
		glDrawArrays(GL_LINE_LOOP, 0, 8);
		
		// Janela - contorno preto
		GLfloat windowVertices[] = {
			-0.1, -0.2, 0.0,
			-0.4, -0.2, 0.0,
			-0.4,  0.0, 0.0,
			-0.1,  0.0, 0.0,
			-0.1, -0.1, 0.0,
			-0.4, -0.1, 0.0,
			-0.4,  0.0, 0.0,
			-0.3,  0.0, 0.0,
			-0.3, -0.2, 0.0,
			-0.2, -0.2, 0.0,
			-0.2,  0.0, 0.0,
			-0.1,  0.0, 0.0
		};
		GLuint windowVBO, windowVAO;
		glGenVertexArrays(1, &windowVAO);
		glGenBuffers(1, &windowVBO);
		glBindVertexArray(windowVAO);
		glBindBuffer(GL_ARRAY_BUFFER, windowVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(windowVertices), windowVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
		glEnableVertexAttribArray(0);
		glUniform4f(colorLoc, 0.0, 0.0, 0.0, 1.0);
		glDrawArrays(GL_LINE_LOOP, 0, 12);
		glBindVertexArray(VAO);

		// Porta - contorno preto
		GLfloat doorVertices[] = {
			0.0, -0.4, 0.0,
			0.0,  0.0, 0.0,
			0.2,  0.0, 0.0,
			0.2, -0.4, 0.0
		};
		GLuint doorVBO, doorVAO;
		glGenVertexArrays(1, &doorVAO);
		glGenBuffers(1, &doorVBO);
		glBindVertexArray(doorVAO);
		glBindBuffer(GL_ARRAY_BUFFER, doorVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(doorVertices), doorVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
		glEnableVertexAttribArray(0);
		glUniform4f(colorLoc, 0.0, 0.0, 0.0, 1.0);
		glDrawArrays(GL_LINE_LOOP, 0, 4);
		glBindVertexArray(VAO);

		glfwSwapBuffers(window);
	}
	glDeleteVertexArrays(1, &VAO);
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

int setupShader(){
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

	return shaderProgram;
}

int setupGeometry() {
	GLfloat vertices[] = {
		-0.5, -0.4, 0.0,
		 0.5, -0.4, 0.0,
		 0.5,  0.1, 0.0,
		-0.7,  0.1, 0.0,
		 0.0,  0.8, 0.0,
		 0.7,  0.1, 0.0,
		-0.5,  0.1, 0.0,
		-0.5, -0.4, 0.0
	};

	GLuint VBO, VAO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return VAO;
}
