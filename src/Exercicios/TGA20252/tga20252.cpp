/* 
* Processamento Gráfico 2025/2
* Trabalho do GA - Space Invaders
* Aluno: Gustavo Haag
* Código original de referência: Professora Rossana Baptista Queiroz
*/

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cfloat>
#include <random>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Sprite.h"

using namespace std;
using namespace glm;

static const int kWidth = 800;
static const int kHeight = 600;

// Vertex + Fragment shaders (cores simples + transform)
static const char* kVS = R"(
#version 400
layout (location = 0) in vec2 aPos; // rectangle local coords [0..1]
uniform mat4 uProjection;
uniform mat4 uModel;
void main() {
	gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
}
)";

// Fragment shader (cor solida)
static const char* kFS = R"(
#version 400
out vec4 FragColor;
uniform vec4 uColor;
void main() {
	FragColor = uColor;
}
)";

// Shader de sprites
static const char* kSpriteVS = R"glsl(
#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texc;
out vec2 tex_coord;
uniform mat4 projection;
uniform mat4 model;
void main(){
	gl_Position = projection * model * vec4(position,1.0);
	tex_coord = vec2(texc.s, 1.0 - texc.t);
}
)glsl";

// Fragment shader de sprites
static const char* kSpriteFS = R"glsl(
#version 400
in vec2 tex_coord;
out vec4 color;
uniform sampler2D tex_buffer;
uniform vec2 offsetTex;
void main(){
	color = texture(tex_buffer, tex_coord + offsetTex);
}
)glsl";

// Callback do GLFW para imprimir mensagens de erro no console.
static void APIENTRY glfwErrorCallback(int error, const char* description) {
	cerr << "GLFW Error " << error << ": " << description << endl;
}

// Compila os shaders de vértice e fragmento e cria um programa OpenGL.
static GLuint buildProgram(const char* vs, const char* fs) {
	auto compile = [](GLenum type, const char* src) -> GLuint {
		GLuint s = glCreateShader(type);
		glShaderSource(s, 1, &src, nullptr);
		glCompileShader(s);
		GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
		if (!ok) {
			char log[1024]; glGetShaderInfoLog(s, 1024, nullptr, log);
			cerr << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << " shader compile error:\n" << log << endl;
		}
		return s;
	};
	GLuint vsId = compile(GL_VERTEX_SHADER, vs);
	GLuint fsId = compile(GL_FRAGMENT_SHADER, fs);
	GLuint prog = glCreateProgram();
	glAttachShader(prog, vsId);
	glAttachShader(prog, fsId);
	glLinkProgram(prog);
	GLint ok = 0; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok) {
		char log[1024]; glGetProgramInfoLog(prog, 1024, nullptr, log);
		cerr << "Program link error:\n" << log << endl;
	}
	glDeleteShader(vsId);
	glDeleteShader(fsId);
	return prog;
}

// Carrega as texturas 2D (PNG). Se falhar, cria uma textura 1x1 branca. Retorna o ID da textura.
static GLuint loadTexture2D(const char* path, int* outW = nullptr, int* outH = nullptr, int* outC = nullptr) {
	int w = 0, h = 0, c = 0;
	stbi_set_flip_vertically_on_load(false);
	unsigned char* data = stbi_load(path, &w, &h, &c, 0);
	GLuint tex = 0;
	if (data) {
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		GLenum fmt = (c == 4) ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);
		cerr << "Loaded texture: " << path << " (" << w << "x" << h << ", channels=" << c << ")\n";
	} else {
		cerr << "Failed to load texture: " << path << ". Using 1x1 white fallback." << endl;
		unsigned char white[4] = {255, 255, 255, 255};
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
		glBindTexture(GL_TEXTURE_2D, 0);
		w = h = 1; c = 4;
	}
	if (outW) {
		*outW = w;
	}
	if (outH) {
		*outH = h;
	}
	if (outC) {
		*outC = c;
	}
	return tex;
}

struct RectMesh {
	GLuint vao = 0;
	GLuint vbo = 0;
	// Cria um quad unitário (0..1) para desenhar retângulos simples.
	void create() {
		const GLfloat verts[] = {
			0.0, 0.0,
			1.0, 0.0,
			1.0, 1.0,
			0.0, 0.0,
			1.0, 1.0,
			0.0, 1.0,
		};
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// Libera os buffers/VAO do quad.
	void destroy() {
		if (vbo) {
			glDeleteBuffers(1, &vbo);
		}
		if (vao) {
			glDeleteVertexArrays(1, &vao);
		}
		vbo = vao = 0;
	}
	// Desenha o quad na tela.
	void draw() const {
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
};

struct Uniforms {
	GLint uProjection = -1;
	GLint uModel = -1;
	GLint uColor = -1;
};

Sprite::Sprite() {}
Sprite::~Sprite() {}

// Configura a geometria (VAO/VBO) do sprite com coordenadas.
static GLuint Sprite_setupGeometry(GLuint &outVAO, vec2 d) {
	GLfloat vertices[] = {
		-0.5,  0.5 , 0.0, 0.0, d.t,
		-0.5, -0.5 , 0.0, 0.0, 0.0,
		 0.5,  0.5 , 0.0, d.s, d.t,
		 0.5, -0.5 , 0.0, d.s, 0.0
	};

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &outVAO);
	glBindVertexArray(outVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return outVAO;
}

// Inicializa o sprite com shader, textura, grid (linhas/colunas), posição e tamanho.
void Sprite::initialize(GLuint shaderID, GLuint texID, int nAnimations, int nFrames, vec3 pos, vec3 dimensions, float angle)
{
	this->shaderID = shaderID;
	this->texID = texID;
	this->pos = pos;
	this->dimensions = dimensions;
	this->angle = angle;
	this->nAnimations = nAnimations;
	this->nFrames = nFrames;
	this->iAnimations = 0;
	this->iFrames = 0;
	this->d.s = 1.0 / (float)nFrames;
	this->d.t = 1.0 / (float)nAnimations;
	this->FPS = 12.0;
	this->lastTime = 0.0;
	this->vel = 0.0;
	this->anchor = vec2(0.5, 0.5);
	this->VAO = Sprite_setupGeometry(this->VAO, this->d);
}

// Atualiza a matriz de modelo e o offset conforme frame/linha e âncora.
void Sprite::update() {
	mat4 model = mat4(1);
	vec2 localAnchor = anchor - vec2(0.5);
	vec3 preTranslate = vec3(-localAnchor.x * dimensions.x, -localAnchor.y * dimensions.y, 0.0);
	model = translate(model, pos + preTranslate);
	if (angle != 0.0)
	{
		model = rotate(model, radians(angle), vec3(0.0f, 0.0f, 1.0f));
	}
	model = scale(model, dimensions);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
	vec2 offsetTex = vec2(iFrames * d.s, -float(iAnimations) * d.t);
	glUniform2f(glGetUniformLocation(shaderID, "offsetTex"), offsetTex.s, offsetTex.t);
}

// Envia o sprite para a GPU desenhar (triângulo strip com textura).
void Sprite::draw() {
	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, texID);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

// Grid dos invasores
struct InvaderGrid {
	vector<vec2> base;
	vector<unsigned char> alive;
	float offsetX = 0.f;
	float offsetY = 0.f;
	float speedX = 80.f;
	float baseSpeedX = 80.f;
	int dir = 1;
	float stepDown = 18.f;
	float baseStepDown = 18.f;
	int bounceCount = 0;
	float minBaseX = 0.f, maxBaseX = 0.f;
	float minBaseY = 0.f, maxBaseY = 0.f;
};

// Preenche a grade base de invasores com posições e estado vivo/morto.
static void fillInvaderGrid(InvaderGrid& grid, int columns, int rows, vec2 start, vec2 spacing) {
	grid.base.clear();
	grid.alive.clear();
	for (int r = 0; r < rows; ++r) {
		for (int c = 0; c < columns; ++c) {
			float x = start.x + c * spacing.x;
			float y = start.y - r * spacing.y;
			grid.base.emplace_back(x, y);
			grid.alive.push_back(1);
		}
	}
	if (!grid.base.empty()) {
		grid.minBaseX = grid.maxBaseX = grid.base[0].x;
		grid.minBaseY = grid.maxBaseY = grid.base[0].y;
		for (auto& p : grid.base) {
			grid.minBaseX = std::min(grid.minBaseX, p.x);
			grid.maxBaseX = std::max(grid.maxBaseX, p.x);
			grid.minBaseY = std::min(grid.minBaseY, p.y);
			grid.maxBaseY = std::max(grid.maxBaseY, p.y);
		}
	}
}

// Testa a sobreposição entre dois retângulos (AABB) com origem no canto inferior esquerdo.
static inline bool aabbOverlap(const vec2& aPos, const vec2& aSize, const vec2& bPos, const vec2& bSize) {
	return (aPos.x < bPos.x + bSize.x) && (aPos.x + aSize.x > bPos.x) &&
		   (aPos.y < bPos.y + bSize.y) && (aPos.y + aSize.y > bPos.y);
}

struct Bullet {
	vec2 pos;
	vec2 vel;
	bool alive = true;
	Sprite spr;
	float animTime = 0.f;
};


// É o main. Inicializa a janela/GL, carrega recursos e executa o gameloop.
int main() {
	glfwSetErrorCallback(glfwErrorCallback);
	if (!glfwInit()) {
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(kWidth, kHeight, "TGA20252 - Space Invaders", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cerr << "Failed to init GLAD" << endl;
		return -1;
	}

	GLuint prog = buildProgram(kVS, kFS);
	GLuint spriteProg = buildProgram(kSpriteVS, kSpriteFS);
	glUseProgram(prog);
	Uniforms u;
	u.uProjection = glGetUniformLocation(prog, "uProjection");
	u.uModel = glGetUniformLocation(prog, "uModel");
	u.uColor = glGetUniformLocation(prog, "uColor");

	mat4 projection = ortho<float>(0.0, (float)kWidth, 0.0, (float)kHeight, -1.0, 1.0);
	glUniformMatrix4fv(u.uProjection, 1, GL_FALSE, value_ptr(projection));

	// Começa a projeção dos sprites
	glUseProgram(spriteProg);
	glUniformMatrix4fv(glGetUniformLocation(spriteProg, "projection"), 1, GL_FALSE, value_ptr(projection));
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(spriteProg, "tex_buffer"), 0);
	glUseProgram(0);

	// Geometria
	RectMesh rect; rect.create();

	// Ativa blending alpha para sprites
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Jogador (atirador)
	const vec2 baseShipSize = vec2(20.0, 20.0);
	const float spriteScale = 5.0;
	const float colliderScale = 3.2;
	vec2 playerSize = baseShipSize * colliderScale;
	const float playerBaseY = 24.0;
	vec2 playerPos = vec2((kWidth - playerSize.x) * 0.5, playerBaseY);
	float playerSpeed = 300.0;
	// Tiros
	vector<Bullet> bullets;
	const vec2 bulletSize = vec2(4.0, 12.0);
	const vec2 bulletVisualSize = vec2(32.f, 32.f);
	const float bulletBottomMarginFrac = 0.1875f;
	const float bulletSpeed = 480.0;
	float fireCooldown = 0.0;
	// Tiros dos invasores
	vector<Bullet> invBullets;
	const vec2 invBulletSize = vec2(4.0, 12.0);
	const vec2 invBulletVisualSize = vec2(32.f, 32.f);
	const float invBulletBottomMarginFrac = 0.1875;
	const float invBulletSpeed = 280.0;
	float invaderFireTimer = 0.7;
	std::mt19937 rng{std::random_device{}()};

	// Invasores: 3 linhas, 10 colunas.
	const vec2 invaderSize = vec2(64.0, 64.0);
	InvaderGrid grid;
	const int columns = 10;
	const int rows = 3;
	// Começa alto perto do topo
	float invGapX = -10.0;
	float invGapY = -10.0;
	float groupWidth = columns * invaderSize.x + (columns - 1) * invGapX;
	float startX = std::max(0.0, (kWidth - groupWidth) * 0.5);
	fillInvaderGrid(grid, columns, rows, vec2(startX, 520.0), vec2(invaderSize.x + invGapX, invaderSize.y + invGapY));
	grid.baseSpeedX = grid.speedX;
	grid.baseStepDown = grid.stepDown;
	grid.bounceCount = 0;

	// Estado do jogo
	int lives = 3;
	bool gameOver = false;
	// (tentar auheua) Garantir que apenas uma perda de vida (e explosão) seja acionada por quadro
	bool loseLifeTriggeredThisFrame = false;
	float lifeLossCooldown = 0.0;

	// Carrega a textura do jogador (5 frames, 1 row, full image 256x64)
	int texW = 0, texH = 0, texC = 0;
	GLuint playerTex = loadTexture2D("../assets/sprites/player_spr.png", &texW, &texH, &texC);

	// Carrega a textura do invasor (6 frames, 1 row, full image 1152x192)
	int invTexW = 0, invTexH = 0, invTexC = 0;
	GLuint invaderTex = loadTexture2D("../assets/sprites/invader_spr.png", &invTexW, &invTexH, &invTexC);

	// Carrega as texturas dos tiros (3 frames cada, 1 row)
	int pShotW = 0, pShotH = 0, pShotC = 0;
	GLuint playerShotTex = loadTexture2D("../assets/sprites/player_shot_spr.png", &pShotW, &pShotH, &pShotC);
	int iShotW = 0, iShotH = 0, iShotC = 0;
	GLuint invaderShotTex = loadTexture2D("../assets/sprites/invader_shot_spr.png", &iShotW, &iShotH, &iShotC);

	// Carrega a textura da explosão (10 frames no total dispostos em 2 linhas x 5 colunas)
	int expTexW = 0, expTexH = 0, expTexC = 0;
	GLuint explosionTex = loadTexture2D("../assets/sprites/explosion_spr.png", &expTexW, &expTexH, &expTexC);

	// Carrega as texturas de fundo (5 imagens estáticas)
	int bgW[5] = {0,0,0,0,0};
	int bgH[5] = {0,0,0,0,0};
	int bgC[5] = {0,0,0,0,0};
	GLuint bgTex[5];
	bgTex[0] = loadTexture2D("../assets/sprites/bg_1.png", &bgW[0], &bgH[0], &bgC[0]);
	bgTex[1] = loadTexture2D("../assets/sprites/bg_2.png", &bgW[1], &bgH[1], &bgC[1]);
	bgTex[2] = loadTexture2D("../assets/sprites/bg_3.png", &bgW[2], &bgH[2], &bgC[2]);
	bgTex[3] = loadTexture2D("../assets/sprites/bg_4.png", &bgW[3], &bgH[3], &bgC[3]);
	bgTex[4] = loadTexture2D("../assets/sprites/bg_5.png", &bgW[4], &bgH[4], &bgC[4]);

	vector<Sprite> bgSprites(5);
	vector<float> bgWidths(5, 0.0);
	glUseProgram(spriteProg);
	for (int i = 0; i < 5; ++i) {
		float aspect = (bgH[i] > 0) ? (float)bgW[i] / (float)bgH[i] : 1.0;
		float h = (float)kHeight;
		float w = h * aspect;
		bgWidths[i] = w;
		bgSprites[i].initialize(spriteProg, bgTex[i], 1, 1, vec3(0.0, 0.0, 0.0), vec3(w, h, 1.0));
		bgSprites[i].setAnchor(vec2(0.0, 0.0));
		bgSprites[i].setAngle(0.0);
	}
	glUseProgram(0);

	// Inicializa o sprite do jogador (1 row, 5 frames de animação)
	Sprite playerSpr;
	glUseProgram(spriteProg);
	vec3 spriteDims = vec3(baseShipSize * spriteScale, 1.0f);
	vec3 spritePos = vec3(playerPos.x + playerSize.x * 0.5f, playerPos.y, 0.0f);
	playerSpr.initialize(spriteProg, playerTex, 1, 5, spritePos, spriteDims);
	playerSpr.setAnchor(vec2(0.5, 0.0));
	playerSpr.setAngle(90.0f);
	glUseProgram(0);

	// Inicializa os sprites dos invasores (1 row, 6 frames de animação)
	vector<Sprite> invSprites;
	invSprites.resize(grid.base.size());
	glUseProgram(spriteProg);
	for (size_t i = 0; i < grid.base.size(); ++i) {
		vec2 p = grid.base[i];
		invSprites[i].initialize(spriteProg, invaderTex, 1, 6, vec3(p, 0.0f), vec3(invaderSize, 1.0f));
		invSprites[i].setAnchor(vec2(0.0, 0.0));
		invSprites[i].setAngle(-90.0f);
	}
	glUseProgram(0);

	// Estado da animação dos sprites do jogador e invasores
	int currentFrame = 0;
	float animAcc = 0.0;
	const float animStep = 0.08;

	// Explosões: 2x5 (10 frames), anchor no centro
	struct Explosion {
		Sprite spr;
		float elapsed = 0.f;
		float frameDur = 0.05f;
		int totalFrames = 10;
		bool alive = true;
	};
	vector<Explosion> explosions;
	// Cria uma explosão na posição/tamanho indicados
	auto spawnExplosion = [&](const vec2 &center, const vec2 &size){
		Explosion e;
		glUseProgram(spriteProg);
		vec2 expSize = size * static_cast<float>(1.25);
		e.spr.initialize(spriteProg, explosionTex, 2, 5, vec3(center, 0.0f), vec3(expSize, 1.0f));
		e.spr.setAnchor(vec2(0.5f, 0.5f));
		e.spr.setAngle(0.0f);
		glUseProgram(0);
		e.elapsed = 0.f;
		e.frameDur = 0.05;
		e.totalFrames = 10;
		e.alive = true;
		explosions.push_back(std::move(e));
	};

	// Auxiliar para reiniciar o jogo
	auto resetGame = [&]() {
		playerPos = vec2((kWidth - playerSize.x) * 0.5f, playerBaseY);
		grid.offsetX = 0.f;
		grid.offsetY = 0.f;
		grid.dir = 1;
		std::fill(grid.alive.begin(), grid.alive.end(), (unsigned char)1);
		bullets.clear();
		invBullets.clear();
		grid.speedX = grid.baseSpeedX;
		grid.stepDown = grid.baseStepDown;
		grid.bounceCount = 0;
		lives = 3;
		gameOver = false;
	};

	auto loseLife = [&]() {
		if (lives > 0) lives--;
		lifeLossCooldown = 0.6;
		// Explosão na nave do jogador
		{
			vec2 playerCenter = vec2(playerPos.x + playerSize.x * 0.5, playerPos.y + playerSize.y * 0.5);
			vec2 playerVisual = baseShipSize * spriteScale;
			spawnExplosion(playerCenter, playerVisual);
		}
		if (lives <= 0) {
			// Game Over: congela o estado atual da tela
			gameOver = true;
		} else {
			// Respawn do jogador
			playerPos = vec2((kWidth - playerSize.x) * 0.5f, playerBaseY);
			bullets.clear();
			invBullets.clear();
		}
	};

	double prevTime = glfwGetTime();
	double titleCountdown = 0.25;

	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	while (!glfwWindowShouldClose(window)) {
		double now = glfwGetTime();
		float dt = float(now - prevTime);
		prevTime = now;
	fireCooldown = std::max(0.0f, fireCooldown - dt);
	invaderFireTimer -= dt;
	lifeLossCooldown = std::max(0.0f, lifeLossCooldown - dt);
		titleCountdown -= dt;
		if (titleCountdown <= 0.0) {
			double fps = 1.0 / std::max(1e-6, (double)dt);
			char title[256];
			snprintf(title, sizeof(title), "TGA20252 - Space Invaders | FPS: %.1f", fps);
			glfwSetWindowTitle(window, title);
			titleCountdown = 0.25;
		}

		glfwPollEvents();
		loseLifeTriggeredThisFrame = false;

		// Input (AD + setas) - Y travado: só o movimento horizontal conta
		auto held = [&](int key) { return glfwGetKey(window, key) == GLFW_PRESS; };
		vec2 move(0);
		if (held(GLFW_KEY_A) || held(GLFW_KEY_LEFT)) {
			move.x -= 1.0f;
		}
		if (held(GLFW_KEY_D) || held(GLFW_KEY_RIGHT)) {
			move.x += 1.0f;
		}
		// ESC pra fechar
		if (held(GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		// Enter para reiniciar se travar no game over
		if (gameOver && held(GLFW_KEY_ENTER)) {
			resetGame();
		}

		// Atira no SPACE (se não estiver em game over)
		if (!gameOver && held(GLFW_KEY_SPACE) && fireCooldown <= 0.0f) {
			Bullet b;
			// Spawna o tiro na ponta da nave do jogador
			b.pos = vec2(playerPos.x + playerSize.x * 0.5f - bulletSize.x * 0.5f,
						 playerPos.y + playerSize.y);
			b.vel = vec2(0.0f, bulletSpeed);
			// Inicializa o sprite do tiro do jogador
			glUseProgram(spriteProg);
			// Aqui usa o tamanho visual maior, mas a colisão usa o menor
			b.spr.initialize(spriteProg, playerShotTex, 1, 3,
						 vec3(b.pos.x + bulletSize.x * 0.5f, b.pos.y - bulletVisualSize.y * bulletBottomMarginFrac, 0.0f),
						 vec3(bulletVisualSize, 1.0f));
			b.spr.setAnchor(vec2(0.5f, 0.0f));
			b.spr.setAngle(90.0f);
			glUseProgram(0);
			bullets.push_back(b);
			fireCooldown = 0.2f;
		}

	if (length(move) > 0.0f) {
		move = normalize(move);
	}
	if (!gameOver) {
		playerPos += move * playerSpeed * dt;
	}
		// Prende o jogador dentro da tela e trava o eixo Y
		playerPos.x = glm::clamp(playerPos.x, 0.0f, (float)kWidth - playerSize.x);
		playerPos.y = playerBaseY;

		// Atualiza a animação do jogador (se mexeu, muda frame; se parou, volta a idle)
		int targetDir = 0;
		if (!gameOver) {
			if (move.x < -0.1f) targetDir = -1; else if (move.x > 0.1f) targetDir = +1; else targetDir = 0;
		}
		animAcc += dt;
		if (animAcc >= animStep) {
			if (targetDir < 0) {
				if (currentFrame == 0) currentFrame = 1;
				else if (currentFrame == 1) currentFrame = 2;
				else if (currentFrame == 2) {}
				else if (currentFrame == 3) currentFrame = 0;
				else if (currentFrame == 4) currentFrame = 3;
			} else if (targetDir > 0) {
				if (currentFrame == 0) currentFrame = 3;
				else if (currentFrame == 3) currentFrame = 4;
				else if (currentFrame == 4) {}
				else if (currentFrame == 1) currentFrame = 0;
				else if (currentFrame == 2) currentFrame = 1;
			} else { // Volta ao idle
				if (currentFrame == 2) currentFrame = 1;
				else if (currentFrame == 1) currentFrame = 0;
				else if (currentFrame == 4) currentFrame = 3;
				else if (currentFrame == 3) currentFrame = 0;
			}
			animAcc = 0.f;
		}
	// Mantém o transform do sprite sincronizado com a posição do jogador
	playerSpr.setFrame(currentFrame);
	playerSpr.setPosition(vec3(playerPos.x + playerSize.x * 0.5f, playerPos.y, 0.0f));
	playerSpr.setDimensions(vec3(baseShipSize * spriteScale, 1.0f));

	// Escalona a velocidade horizontal dos invasores conforme vão sendo destruídos
	int aliveCountForSpeed = 0;
	for (auto a : grid.alive) if (a) ++aliveCountForSpeed;
	int totalInvaders = (int)grid.base.size();
	float aliveFrac = totalInvaders > 0 ? (float)aliveCountForSpeed / (float)totalInvaders : 1.0f;
	// Aumenta a velocidade com menos invasores vivos
	if (!gameOver) {
		grid.speedX = grid.baseSpeedX * (1.0f + (1.0f - aliveFrac) * 2.0f);
	}

	// Atualiza a posição do grupo de invasores
	// Computa o próximo offset X e testa se bateu (bounce) na borda da tela
	// (se bateu, inverte direção, desce e aumenta a velocidade)
	// Se for game over, congela o movimento dos invasores
	float nextOffsetX = grid.offsetX + (gameOver ? 0.f : grid.dir * grid.speedX * dt);
		// Computa os limites esquerdo/direito do grupo de invasores com o próximo offset
	float groupLeft = grid.minBaseX + std::min(grid.offsetX, nextOffsetX);
	float groupRight = grid.maxBaseX + std::max(grid.offsetX, nextOffsetX) + invaderSize.x;
		bool bounce = false;
		if (groupLeft < 0.f) { bounce = true; nextOffsetX = -grid.minBaseX; }
		if (groupRight > kWidth) { bounce = true; nextOffsetX = (float)kWidth - invaderSize.x - grid.maxBaseX; }
		if (!gameOver && bounce) {
			grid.dir *= -1;
			// Aumenta a velocidade de descida
			grid.bounceCount++;
			float stepDownThisBounce = grid.baseStepDown * (1.0f + grid.bounceCount * 0.03f + (1.0f - aliveFrac) * 0.8f);
			grid.offsetY -= stepDownThisBounce; // Desce mais a cada bounce
		}
		grid.offsetX = nextOffsetX;

		// Lógica de tiro dos invasores: atira do invasor vivo mais embaixo de uma coluna
	if (!gameOver && invaderFireTimer <= 0.f) {
			// Tiros intensificam com menos invasores vivos
			float nextCooldown = (1.2f - (1.0f - aliveFrac) * 0.8f) - 0.15f; // ~[1.05 .. 0.25]
			nextCooldown = glm::clamp(nextCooldown, 0.25f, 1.05f);

			// Tenta algumas colunas aleatórias para encontrar um atirador
			std::uniform_int_distribution<int> colDist(0, columns - 1);
			bool fired = false;
			for (int attempt = 0; attempt < 20 && !fired; ++attempt) {
				int c = colDist(rng);
				for (int r = rows - 1; r >= 0; --r) {
					size_t idx = (size_t)r * (size_t)columns + (size_t)c;
					if (idx < grid.alive.size() && grid.alive[idx]) {
						vec2 invPos = vec2(grid.base[idx].x + grid.offsetX, grid.base[idx].y + grid.offsetY);
						Bullet b{};
						b.pos = vec2(invPos.x + (invaderSize.x - invBulletSize.x) * 0.5f,
									  invPos.y - invBulletSize.y);
						b.vel = vec2(0.f, -invBulletSpeed);
						// Inicializa o sprite do tiro do invasor
						glUseProgram(spriteProg);
						b.spr.initialize(spriteProg, invaderShotTex, 1, 3,
							vec3(b.pos.x + invBulletSize.x * 0.5f, b.pos.y - invBulletVisualSize.y * invBulletBottomMarginFrac, 0.0f),
							vec3(invBulletVisualSize, 1.0f));
						b.spr.setAnchor(vec2(0.5f, 0.0f));
						b.spr.setAngle(-90.0f);
						glUseProgram(0);
						invBullets.push_back(b);
						fired = true;
						break;
					}
				}
			}
			// Se não achou nas colunas aleatórias, escolhe o invasor mais embaixo vivo
			if (!fired) {
				float bestY = -FLT_MAX; size_t bestIdx = (size_t)-1;
				for (size_t i = 0; i < grid.base.size(); ++i) if (grid.alive[i]) {
					float y = grid.base[i].y;
					if (y < bestY || bestIdx == (size_t)-1) { bestY = y; bestIdx = i; }
				}
				if (bestIdx != (size_t)-1) {
					vec2 invPos = vec2(grid.base[bestIdx].x + grid.offsetX, grid.base[bestIdx].y + grid.offsetY);
					Bullet b{};
					b.pos = vec2(invPos.x + (invaderSize.x - invBulletSize.x) * 0.5f,
								  invPos.y - invBulletSize.y);
					b.vel = vec2(0.f, -invBulletSpeed);
					// Inicializa o sprite do tiro do invasor
					glUseProgram(spriteProg);
					b.spr.initialize(spriteProg, invaderShotTex, 1, 3,
						vec3(b.pos.x + invBulletSize.x * 0.5f, b.pos.y - invBulletVisualSize.y * invBulletBottomMarginFrac, 0.0f),
						vec3(invBulletVisualSize, 1.0f));
					b.spr.setAnchor(vec2(0.5f, 0.0f));
					b.spr.setAngle(-90.0f);
					glUseProgram(0);
					invBullets.push_back(b);
				}
			}
			invaderFireTimer = nextCooldown;
		}

		// Atualiza a posição dos tiros e animação
		for (auto& b : bullets) {
			if (!b.alive) {
				continue;
			}
			b.pos += b.vel * dt;
			if (b.pos.y > kHeight) {
				b.alive = false;
			}
			b.animTime += dt;
			int f = (int)floor(b.animTime / 0.08f) % 3;
			b.spr.setFrame(f);
			b.spr.setPosition(vec3(b.pos.x + bulletSize.x * 0.5f, b.pos.y - bulletVisualSize.y * bulletBottomMarginFrac, 0.0f));
			b.spr.setDimensions(vec3(bulletVisualSize, 1.0f));
		}
		for (auto& b : invBullets) {
			if (!b.alive) {
				continue;
			}
			if (!gameOver) {
				b.pos += b.vel * dt;
			}
			if (b.pos.y + invBulletSize.y < 0.0f) {
				b.alive = false;
			}
			// Atualiza o sprite do tiro do invasor
			b.animTime += dt;
			int f2 = (int)floor(b.animTime / 0.08f) % 3;
			b.spr.setFrame(f2);
			b.spr.setPosition(vec3(b.pos.x + invBulletSize.x * 0.5f, b.pos.y - invBulletVisualSize.y * invBulletBottomMarginFrac, 0.0f));
			b.spr.setDimensions(vec3(invBulletVisualSize, 1.0f));
		}
		// Tiros vs colisão com invasores (AABB)
		for (auto& b : bullets) {
			if (!b.alive) {
				continue;
			}
			for (size_t i = 0; i < grid.base.size(); ++i) {
				if (!grid.alive[i]) {
					continue;
				}
				vec2 invPos = vec2(grid.base[i].x + grid.offsetX, grid.base[i].y + grid.offsetY);
				if (aabbOverlap(b.pos, bulletSize, invPos, invaderSize)) {
					grid.alive[i] = 0;
					b.alive = false;
					// Spawna a explosão centralizada no invasor
					vec2 invCenter = invPos + invaderSize * 0.5f;
					spawnExplosion(invCenter, invaderSize);
					break;
				}
			}
		}
		// Atualiza o estado de animação das explosões (desenha mais tarde na fase de renderização)
		for (auto &e : explosions) {
			if (!e.alive) {
				continue;
			}
			e.elapsed += dt;
			int idx = (int)floor(e.elapsed / e.frameDur);
			if (idx >= e.totalFrames) { e.alive = false; continue; }
			int row = idx / 5; // 0 or 1
			int frame = idx % 5;
			e.spr.setAnimationRow(row);
			e.spr.setFrame(frame);
		}
		// Tiros do invasor vs colisão com o jogador
		if (!gameOver) {
			for (auto& b : invBullets) {
				if (!b.alive) {
					continue;
				}
				if (!loseLifeTriggeredThisFrame && lifeLossCooldown <= 0.0f && aabbOverlap(b.pos, invBulletSize, playerPos, playerSize)) {
					// Perde uma vida; se ainda houver vidas, mantém os invasores como estão
					b.alive = false; // impede reativação em quadros subsequentes
					loseLife();
					loseLifeTriggeredThisFrame = true;
					break;
				}
			}
		}
		// Remove tiros pendentes de ambos os lados
		bullets.erase(remove_if(bullets.begin(), bullets.end(), [](const Bullet& b){ return !b.alive; }), bullets.end());
		invBullets.erase(remove_if(invBullets.begin(), invBullets.end(), [](const Bullet& b){ return !b.alive; }), invBullets.end());
		// Remove explosões finalizadas
		explosions.erase(remove_if(explosions.begin(), explosions.end(), [](const Explosion &e){ return !e.alive; }), explosions.end());

		// Game Over: se o invasor VIVO mais baixo alcançar a linha do jogador, perde vida e reseta
		float lowestAliveY = FLT_MAX;
		for (size_t i = 0; i < grid.base.size(); ++i) {
			if (!grid.alive[i]) {
				continue;
			}
			lowestAliveY = std::min(lowestAliveY, grid.base[i].y);
		}
		bool anyAlive = false;
		for (auto a : grid.alive) {
			if (a) {
				anyAlive = true;
				break;
			}
		}
		if (!gameOver && anyAlive) {
			float lowestInvaderY = lowestAliveY + grid.offsetY;
			float playerLineY = playerPos.y + playerSize.y;
			if (!loseLifeTriggeredThisFrame && lifeLossCooldown <= 0.0f && lowestInvaderY <= playerLineY) {
				// Invaders alcançaram a linha do jogador = perde uma vida
				loseLife();
				loseLifeTriggeredThisFrame = true;
			}
		} else if (!gameOver) {
			// Todos os invasores destruídos -> próxima onda
			std::fill(grid.alive.begin(), grid.alive.end(), (unsigned char)1);
			grid.offsetX = 0.f; grid.offsetY = 0.f; grid.dir = 1;
			bullets.clear();
			invBullets.clear();
			grid.speedX = grid.baseSpeedX;
			grid.stepDown = grid.baseStepDown;
			grid.bounceCount = 0;
		}

		// Limpa a tela
		int fbw, fbh; glfwGetFramebufferSize(window, &fbw, &fbh);
		glViewport(0, 0, fbw, fbh);
		glClear(GL_COLOR_BUFFER_BIT);

		// Desenha o fundo com parallax conforme a posição do jogador
		glUseProgram(spriteProg);
		glActiveTexture(GL_TEXTURE0);
		float pxCenter = playerPos.x + playerSize.x * 0.5;
		float norm = ((float)pxCenter - (float)kWidth * 0.5) / ((float)kWidth * 0.5);
		float maxShifts[5] = {0.0, 10.0, 20.0, 30.0, 40.0};
		for (int i = 0; i < 5; ++i) {
			float shift = norm * maxShifts[i];
			float w = bgWidths[i];
			float x = ((float)kWidth - w) * 0.5 + shift;
			bgSprites[i].setPosition(vec3(x, 0.0, 0.0));
			bgSprites[i].setDimensions(vec3(w, (float)kHeight, 1.0));
			bgSprites[i].update();
			bgSprites[i].draw();
		}
		glUseProgram(prog);

		// Desenha o jogador com sprite (animação conforme o frame atual)
		mat4 model(1);
		if (!gameOver) {
			glUseProgram(spriteProg);
			glActiveTexture(GL_TEXTURE0);
			playerSpr.update();
			playerSpr.draw();
			glUseProgram(prog);
		}

		// Desenha os invasores com sprites
		glUseProgram(spriteProg);
		glActiveTexture(GL_TEXTURE0);
		int invFrame = int(floor(now * 8.0)) % 6;
		for (size_t i = 0; i < grid.base.size(); ++i) {
			if (!grid.alive[i]) {
				continue;
			}
			vec2 pos = vec2(grid.base[i].x + grid.offsetX, grid.base[i].y + grid.offsetY);
			invSprites[i].setFrame(invFrame);
			invSprites[i].setPosition(vec3(pos, 0.0f));
			invSprites[i].setDimensions(vec3(invaderSize, 1.0f));
			invSprites[i].update();
			invSprites[i].draw();
		}
		glUseProgram(prog);

		// Desenha os tiros por cima dos atores
		glUseProgram(spriteProg);
		glActiveTexture(GL_TEXTURE0);
		for (auto& b : bullets) {
			if (!b.alive) {
				continue;
			}
			b.spr.update();
			b.spr.draw();
		}
		for (auto& b : invBullets) {
			if (!b.alive) {
				continue;
			}
			b.spr.update();
			b.spr.draw();
		}
		glUseProgram(prog);

		// Desenha explosões por cima dos atores
		glUseProgram(spriteProg);
		glActiveTexture(GL_TEXTURE0);
		for (auto &e : explosions) {
			if (!e.alive) {
				continue;
			}
			e.spr.update();
			e.spr.draw();
		}
		glUseProgram(prog);
		glfwSwapBuffers(window);
	}

	rect.destroy();
	glDeleteProgram(prog);
	glDeleteProgram(spriteProg);
	glfwTerminate();
	return 0;
}
