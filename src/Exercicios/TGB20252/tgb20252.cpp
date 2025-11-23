/* 
* Processamento Gráfico 2025/2
* Trabalho do GB - VIApp
* Aluno: Gustavo Haag
* Código original de referência: Professora Rossana Baptista Queiroz
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <ctime>

#include "TextureManager.h"
#include "VideoHandler.h"
#include "FilterManager.h"
#include "StickerManager.h"
#include "FaceDetector.h"
#include "OverlayManager.h"

constexpr int WINDOW_WIDTH = 540;
constexpr int WINDOW_HEIGHT = 960;

class VIApp {
public:
    VIApp();
    ~VIApp();

    bool initialize();
    void run();
    void cleanup();

private:
    enum class AppMode { PHOTO, VIDEO };

    GLFWwindow* window{};
    TextureManager textureManager;
    VideoHandler videoHandler;
    FilterManager filterManager;
    StickerManager stickerManager;
    FaceDetector faceDetector;
    OverlayManager overlayManager;

    cv::Mat liveFrame;
    cv::Mat frameBuffer;

    FilterType currentFilter{FilterType::NONE};
    OverlayType currentOverlay{OverlayType::NONE};
    AppMode appMode{AppMode::VIDEO};
    bool faceDetectionEnabled{false};
    bool webcamEnabled{true};
    bool enableR{true};
    bool enableG{true};
    bool enableB{true};
    int selectedSticker{-1};
    int draggedSticker{-1};

    GLuint shaderProgram{};
    GLuint VAO{};
    GLuint VBO{};
    GLuint EBO{};
    double lastFrameTime{0.0};
    double frameDelta{0.0};

    static VIApp* instance;

    void initOpenGL();
    void initShaders();
    void initGeometry();
    void initImGui();
    void renderImGui();
    void shutdownImGui();
    void processFrame();
    void renderFrame();
    void switchMode();
    void saveCurrentImage() const;
    void resetImage();
    void updateVideoFeed();
    void applyOfflineOverlay();
    void drawFiltersPanel();
    void drawOverlayPanel();
    void drawTopButtons();
    void drawPhotoHud();
    void drawVideoHud();
    void drawWebcamButton();
    bool centeredButton(const char* label, const ImVec2& size);
    void handleFaceProcessing();
    void applyFiltersAndOverlays();
    void applyStickersLayer();

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

VIApp* VIApp::instance = nullptr;

VIApp::VIApp() {
    instance = this;
}

VIApp::~VIApp() {
    cleanup();
}

bool VIApp::initialize() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "VIApp - Photo & Video Editor", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync for smooth 60 FPS
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetKeyCallback(window, keyCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    initOpenGL();
    initShaders();
    initGeometry();
    initImGui();

    textureManager.createTexture(WINDOW_WIDTH, WINDOW_HEIGHT);

    if (!videoHandler.loadVideo("../assets/videos/camera_video.mp4")) {
        std::cerr << "Warning: Could not load video file" << std::endl;
        liveFrame = cv::Mat(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3, cv::Scalar(60, 60, 60));
        cv::putText(liveFrame, "No Camera Feed", cv::Point(100, WINDOW_HEIGHT / 2), cv::FONT_HERSHEY_SIMPLEX, 1.0,
                    cv::Scalar(180, 180, 180), 2);
        videoHandler.setPlaying(false);
    } else {
        videoHandler.setPlaying(true);
        liveFrame = videoHandler.getCurrentFrame();
    }

    frameBuffer = liveFrame.clone();

    stickerManager.loadStickers();
    overlayManager.load(WINDOW_WIDTH, WINDOW_HEIGHT);
    faceDetector.initialize();

    return true;
}

void VIApp::initOpenGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void VIApp::initShaders() {
    const char* vertexShaderSource = R"(
        #version 400 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 400 core
        out vec4 FragColor;
        in vec2 TexCoord;
        uniform sampler2D texture1;
        void main() {
            FragColor = texture(texture1, TexCoord);
        }
    )";
    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void VIApp::initGeometry() {
    float vertices[] = {
        1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
        1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
       -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
       -1.0f,  1.0f, 0.0f,   0.0f, 1.0f
    };
    
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void VIApp::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 8.0f;
    style.WindowPadding = ImVec2(15, 15);
    style.FramePadding = ImVec2(10, 6);
    style.ItemSpacing = ImVec2(10, 8);
    
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.85f);
    colors[ImGuiCol_Button] = ImVec4(0.26f, 0.26f, 0.26f, 0.80f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 0.90f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.80f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.30f, 0.30f, 0.90f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 400");
}

void VIApp::switchMode() {
    appMode = (appMode == AppMode::VIDEO) ? AppMode::PHOTO : AppMode::VIDEO;
    if (appMode == AppMode::VIDEO) {
        stickerManager.clearStickers();
        selectedSticker = -1;
        if (webcamEnabled) {
            videoHandler.setPlaying(true);
        }
        std::cout << "Switched to VIDEO mode" << std::endl;
    } else {
        std::cout << "Switched to PHOTO mode - Click CAPTURE to take photo" << std::endl;
    }
}

void VIApp::saveCurrentImage() const {
    if (frameBuffer.empty()) {
        return;
    }
    std::time_t now = std::time(nullptr);
    std::string filename = "../viapp_photo_" + std::to_string(now) + ".png";
    if (cv::imwrite(filename, frameBuffer)) {
        std::cout << "Photo saved: " << filename << std::endl;
    } else {
        std::cerr << "Failed to save photo" << std::endl;
    }
}

void VIApp::resetImage() {
    frameBuffer = liveFrame.clone();
    currentFilter = FilterType::NONE;
    currentOverlay = OverlayType::NONE;
    stickerManager.clearStickers();
    selectedSticker = -1;
    enableR = enableG = enableB = true;
    filterManager.setRGBChannels(true, true, true);
}

void VIApp::updateVideoFeed() {
    if (!webcamEnabled || !videoHandler.isPlaying()) {
        return;
    }
    cv::Mat frame = videoHandler.getNextFrame(frameDelta);
    if (!frame.empty()) {
        liveFrame = frame;
    }
}

void VIApp::applyOfflineOverlay() {
    if (frameBuffer.empty()) {
        return;
    }

    cv::Mat overlay(frameBuffer.size(), frameBuffer.type(), cv::Scalar(0, 0, 0));
    cv::addWeighted(overlay, 0.85, frameBuffer, 0.15, 0.0, frameBuffer);

    const std::string text = "WEBCAM OFFLINE";
    int baseline = 0;
    cv::Size size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 1.0, 2, &baseline);
    cv::Point origin((frameBuffer.cols - size.width) / 2, (frameBuffer.rows + size.height) / 2);
    cv::putText(frameBuffer, text, origin, cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
}

void VIApp::handleFaceProcessing() {
    if (!faceDetector.isInitialized() || !webcamEnabled) {
        filterManager.setFaceMask(cv::Mat());
        return;
    }

    std::vector<FaceData> faces = faceDetector.detectFaces(frameBuffer);
    if (faces.empty()) {
        filterManager.setFaceMask(cv::Mat());
        return;
    }

    cv::Mat mask = faceDetector.createFaceMask(frameBuffer, faces);
    filterManager.setFaceMask(mask);
    if (faceDetectionEnabled) {
        faceDetector.drawFaces(frameBuffer, faces);
    }
}

void VIApp::applyFiltersAndOverlays() {
    if (currentFilter == FilterType::RGB_CHANNELS) {
        filterManager.setRGBChannels(enableR, enableG, enableB);
    }

    if (currentFilter != FilterType::NONE) {
        frameBuffer = filterManager.applyFilter(frameBuffer, currentFilter, ChannelMode::RGB);
    }

    if (currentOverlay != OverlayType::NONE) {
        frameBuffer = overlayManager.apply(frameBuffer, currentOverlay);
    }
}

void VIApp::applyStickersLayer() {
    frameBuffer = stickerManager.applyStickers(frameBuffer);
    if (selectedSticker >= 0) {
        double xpos = 0.0;
        double ypos = 0.0;
        glfwGetCursorPos(window, &xpos, &ypos);
        frameBuffer = stickerManager.renderPreview(frameBuffer, selectedSticker, cv::Point(xpos, ypos), 0.5f);
    }
}



void VIApp::processFrame() {
    updateVideoFeed();
    if (liveFrame.empty()) {
        return;
    }

    frameBuffer = liveFrame.clone();

    handleFaceProcessing();
    applyFiltersAndOverlays();

    if (appMode == AppMode::PHOTO) {
        applyStickersLayer();
    }

    if (!webcamEnabled) {
        applyOfflineOverlay();
    }
}

void VIApp::renderFrame() {
    if (frameBuffer.empty()) {
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    textureManager.updateTexture(frameBuffer);

    glUseProgram(shaderProgram);
    textureManager.bind();
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    textureManager.unbind();

    renderImGui();

    glfwSwapBuffers(window);
}

bool VIApp::centeredButton(const char* label, const ImVec2& size) {
    float offset = (ImGui::GetContentRegionAvail().x - size.x) * 0.5f;
    if (offset > 0.0f) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    }
    return ImGui::Button(label, size);
}

void VIApp::drawFiltersPanel() {
    ImGui::SetNextWindowPos(ImVec2(15, 15), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(180, 0), ImGuiCond_Always);
    ImGui::Begin("Filters", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    std::vector<FilterInfo> filters = filterManager.getAvailableFilters();
    std::string currentLabel = "Sem filtro";
    for (const auto& info : filters) {
        if (info.type == currentFilter) {
            currentLabel = info.name;
            break;
        }
    }

    if (ImGui::BeginCombo("##Filter", currentLabel.c_str())) {
        bool noneSelected = currentFilter == FilterType::NONE;
        if (ImGui::Selectable("Sem filtro", noneSelected)) {
            currentFilter = FilterType::NONE;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Sem efeito aplicado");
        }

        for (const auto& info : filters) {
            bool active = info.type == currentFilter;
            if (ImGui::Selectable(info.name.c_str(), active)) {
                currentFilter = info.type;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", info.description.c_str());
            }
        }
        ImGui::EndCombo();
    }

    if (currentFilter == FilterType::RGB_CHANNELS) {
        ImGui::Separator();
        if (ImGui::Checkbox("R", &enableR)) {
            filterManager.setRGBChannels(enableR, enableG, enableB);
        }
        ImGui::SameLine();
        if (ImGui::Checkbox("G", &enableG)) {
            filterManager.setRGBChannels(enableR, enableG, enableB);
        }
        ImGui::SameLine();
        if (ImGui::Checkbox("B", &enableB)) {
            filterManager.setRGBChannels(enableR, enableG, enableB);
        }
    }

    ImGui::End();
}

void VIApp::drawOverlayPanel() {
    ImGui::SetNextWindowPos(ImVec2(15, 120), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(180, 0), ImGuiCond_Always);
    ImGui::Begin("Overlays", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    const auto& entries = overlayManager.options();
    std::vector<const char*> labels;
    labels.push_back("OFF");
    for (const auto& entry : entries) {
        labels.push_back(entry.label.c_str());
    }

    int index = 0;
    if (currentOverlay != OverlayType::NONE) {
        for (size_t i = 0; i < entries.size(); ++i) {
            if (entries[i].type == currentOverlay) {
                index = static_cast<int>(i) + 1;
                break;
            }
        }
    }

    if (ImGui::Combo("##Overlay", &index, labels.data(), static_cast<int>(labels.size()))) {
        if (index == 0) {
            currentOverlay = OverlayType::NONE;
        } else {
            currentOverlay = entries[index - 1].type;
        }
    }

    ImGui::End();
}

void VIApp::drawTopButtons() {
    ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - 90, 15), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(75, 0), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Face", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    if (centeredButton("FACE", ImVec2(55, 30))) {
        faceDetectionEnabled = !faceDetectionEnabled;
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - 90, 70), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(75, 0), ImGuiCond_Always);
    ImGui::Begin("Reset", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    if (centeredButton("RESET", ImVec2(55, 30))) {
        resetImage();
        std::cout << "Reset to original" << std::endl;
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void VIApp::drawPhotoHud() {
    ImGui::SetNextWindowPos(ImVec2(15, 225), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(190, 0), ImGuiCond_Always);
    ImGui::Begin("Stickers", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    const auto& stickers = stickerManager.getAvailableStickers();
    int numToShow = std::min(static_cast<int>(stickers.size()), 9);
    if (ImGui::BeginTable("StickerGrid", 3, ImGuiTableFlags_SizingFixedFit)) {
        for (int i = 0; i < numToShow; ++i) {
            ImGui::TableNextColumn();
            char label[16];
            snprintf(label, sizeof(label), "S%d", i + 1);
            bool isSelected = selectedSticker == i;
            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.8f, 1.0f));
            }
            if (ImGui::Button(label, ImVec2(50, 50))) {
                selectedSticker = i;
            }
            if (isSelected) {
                ImGui::PopStyleColor();
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT - 100), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(100, 0), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Capture", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    if (centeredButton("CAPTURE", ImVec2(80, 70))) {
        saveCurrentImage();
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(15, WINDOW_HEIGHT - 85), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(70, 0), ImGuiCond_Always);
    ImGui::Begin("Mode", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    if (centeredButton("VIDEO", ImVec2(50, 50))) {
        switchMode();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void VIApp::drawVideoHud() {
    ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT - 100), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(100, 0), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Switch", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    if (centeredButton("PHOTO", ImVec2(80, 70))) {
        switchMode();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void VIApp::drawWebcamButton() {
    ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH - 85, WINDOW_HEIGHT - 85), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(70, 0), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Webcam", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

    const char* label = webcamEnabled ? "CAM ON" : "CAM OFF";
    ImVec4 color = webcamEnabled ? ImVec4(0.2f, 0.6f, 0.2f, 1.0f) : ImVec4(0.6f, 0.2f, 0.2f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    if (centeredButton(label, ImVec2(60, 60))) {
        webcamEnabled = !webcamEnabled;
        videoHandler.setPlaying(webcamEnabled);
        if (!webcamEnabled && !frameBuffer.empty()) {
            liveFrame = frameBuffer.clone();
        }
    }
    ImGui::PopStyleColor();

    ImGui::End();
    ImGui::PopStyleVar();
}

void VIApp::renderImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawFiltersPanel();
    drawOverlayPanel();
    drawTopButtons();
    if (appMode == AppMode::PHOTO) {
        drawPhotoHud();
    } else {
        drawVideoHud();
    }
    drawWebcamButton();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void VIApp::shutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void VIApp::run() {
    lastFrameTime = glfwGetTime();
    
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        frameDelta = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        
        glfwPollEvents();
        processFrame();
        renderFrame();
    }
}

void VIApp::cleanup() {
    shutdownImGui();
    
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
    
    textureManager.cleanup();
    
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void VIApp::mouseButtonCallback(GLFWwindow* windowPtr, int button, int action, int mods) {
    if (!instance) return;
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double xpos = 0.0;
        double ypos = 0.0;
        glfwGetCursorPos(windowPtr, &xpos, &ypos);
        cv::Point pos(xpos, ypos);

        if (action == GLFW_PRESS && instance->appMode == AppMode::PHOTO) {
            int stickerId = instance->stickerManager.findStickerAtPosition(pos);
            if (stickerId >= 0) {
                instance->draggedSticker = stickerId;
            } else if (instance->selectedSticker >= 0) {
                instance->stickerManager.addSticker(instance->selectedSticker, pos);
                instance->selectedSticker = -1;
                std::cout << "Sticker placed at (" << static_cast<int>(xpos) << ", " << static_cast<int>(ypos) << ")" << std::endl;
            }
        } else if (action == GLFW_RELEASE) {
            instance->draggedSticker = -1;
        }
    }
}

void VIApp::cursorPosCallback(GLFWwindow* windowPtr, double xpos, double ypos) {
    if (!instance) return;
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    if (instance->draggedSticker >= 0) {
        instance->stickerManager.updateStickerPosition(instance->draggedSticker, cv::Point(xpos, ypos));
    }
}

void VIApp::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (!instance || action != GLFW_PRESS) return;
    
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    } else if (key == GLFW_KEY_SPACE) {
        instance->resetImage();
        std::cout << "Reset to original" << std::endl;
    }
}

int main() {
    VIApp app;
    
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    std::cout << "=== VIApp - Photo & Video Editor ===" << std::endl;
    std::cout << "Instagram-style camera interface" << std::endl;
    std::cout << "\nKeyboard Controls:" << std::endl;
    std::cout << "  SPACE - Reset filters and stickers" << std::endl;
    std::cout << "  ESC   - Exit application" << std::endl;
    std::cout << "\nUI Controls:" << std::endl;
    std::cout << "  VIDEO MODE:" << std::endl;
    std::cout << "    - Use the dropdowns to select filters and overlays" << std::endl;
    std::cout << "    - Click PHOTO button to switch to photo mode" << std::endl;
    std::cout << "    - Click FACE button to toggle face detection" << std::endl;
    std::cout << "  PHOTO MODE:" << std::endl;
    std::cout << "    - Click S1-S6 buttons to select stickers" << std::endl;
    std::cout << "    - Click on image to place selected sticker" << std::endl;
    std::cout << "    - Click CAPTURE button to save photo" << std::endl;
    std::cout << "    - Click VIDEO button to return to video mode" << std::endl;
    std::cout << "\nStarting application..." << std::endl;
    
    app.run();
    app.cleanup();
    
    return 0;
}