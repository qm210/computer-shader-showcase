#include <windows.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include "shader_loading.h"
#include "opengl_stuff.h"
PFNGLCREATESHADERPROC        glCreateShader_ = nullptr;
PFNGLSHADERSOURCEPROC        glShaderSource_ = nullptr;
PFNGLCOMPILESHADERPROC       glCompileShader_ = nullptr;
PFNGLGETSHADERIVPROC         glGetShaderiv_ = nullptr;
PFNGLGETSHADERINFOLOGPROC    glGetShaderInfoLog_ = nullptr;
PFNGLDELETESHADERPROC        glDeleteShader_ = nullptr;
PFNGLCREATEPROGRAMPROC       glCreateProgram_ = nullptr;
PFNGLATTACHSHADERPROC        glAttachShader_ = nullptr;
PFNGLLINKPROGRAMPROC         glLinkProgram_ = nullptr;
PFNGLGETPROGRAMIVPROC        glGetProgramiv_ = nullptr;
PFNGLGETPROGRAMINFOLOGPROC   glGetProgramInfoLog_ = nullptr;
PFNGLUSEPROGRAMPROC          glUseProgram_ = nullptr;
PFNGLDELETEPROGRAMPROC       glDeleteProgram_ = nullptr;
PFNGLGETUNIFORMLOCATIONPROC  glGetUniformLocation_ = nullptr;
PFNGLUNIFORM2IPROC           glUniform2i_ = nullptr;
PFNGLUNIFORM1IPROC           glUniform1i_ = nullptr;
PFNGLDISPATCHCOMPUTEPROC     glDispatchCompute_ = nullptr;
PFNGLMEMORYBARRIERPROC       glMemoryBarrier_ = nullptr;
PFNGLBINDIMAGETEXTUREPROC    glBindImageTexture_ = nullptr;
PFNGLACTIVETEXTUREPROC       glActiveTexture_ = nullptr;
PFNGLGENVERTEXARRAYSPROC     glGenVertexArrays_ = nullptr;
PFNGLBINDVERTEXARRAYPROC     glBindVertexArray_ = nullptr;
PFNGLDELETEVERTEXARRAYSPROC  glDeleteVertexArrays_ = nullptr;

constexpr int WIDTH = 400;
constexpr int HEIGHT = 300;
constexpr float PIXEL_SCALE = 3.f;
constexpr double FPS = 5.0;

// Anfangszustand zufallsverteilt
constexpr float INIT_ALIVE_CHANCE = 0.2;
constexpr uint8_t INIT_RANDOM_SEED = 0u;

static GLuint createStateTexture(const std::vector<std::uint8_t>& data) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI,
                 WIDTH, HEIGHT,
                 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
                 data.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

static std::vector<std::uint8_t> initState() {
    std::bernoulli_distribution alive(INIT_ALIVE_CHANCE);
    std::mt19937 rng(INIT_RANDOM_SEED);
    std::vector<std::uint8_t> data(WIDTH * HEIGHT, 0);
    for (auto& v : data) {
        v = alive(rng) ? 1u : 0u;
    }
    return data;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "glfwInit failed\n";
        return EXIT_FAILURE;
    }

    const int pxWidth = WIDTH * PIXEL_SCALE;
    const int pxHeight = HEIGHT * PIXEL_SCALE;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(pxWidth, pxHeight,
                                          "<Space> zum Starten/Pausieren!",
                                          nullptr, nullptr);
    if (!window) {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (!LoadGLFunctions()) {
        std::cerr << "Failed to load required OpenGL functions\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "GL_VERSION: " << (version ? version : "unknown") << "\n";

    GLuint vs = CompileShaderFromFile(GL_VERTEX_SHADER, "shaders/vertex.glsl");
    GLuint fs = CompileShaderFromFile(GL_FRAGMENT_SHADER, "shaders/fragment.glsl");
    GLuint cs = CompileShaderFromFile(GL_COMPUTE_SHADER, "shaders/compute.glsl");
    GLuint renderProg = LinkProgram({vs, fs});
    GLuint computeProg = LinkProgram({cs});
    glDeleteShader_(cs);
    glDeleteShader_(vs);
    glDeleteShader_(fs);
    
    auto initialState = initState();
    auto zeroState = std::vector<std::uint8_t>(WIDTH * HEIGHT, 0u);

    std::array<GLuint, 2> stateTex = {
            createStateTexture(initialState),
            createStateTexture(zeroState)
    };

    GLuint vao = 0;
    glGenVertexArrays_(1, &vao);
    glBindVertexArray_(vao);
    const GLint uGridSizeLoc = glGetUniformLocation_(computeProg, "gridSize");
    const GLint uStateTexLoc = glGetUniformLocation_(renderProg, "texState");

    // Application State für unsere kleine Engine hier
    bool paused = false;
    bool spaceAlreadyPressed = false;
    using clock = std::chrono::steady_clock;
    const auto frameDelta = std::chrono::duration_cast<clock::duration>(
            std::chrono::duration<double>(1.0 / FPS)
    );
    auto nextFrameTime = clock::now();
    int frameIndex = 0;

    while (!glfwWindowShouldClose(window)) {
        nextFrameTime += frameDelta;
        glfwPollEvents();

        // Leertaste zum Anhalten & Weiterlaufen lassen:
        bool spacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (spacePressed && !spaceAlreadyPressed) {
            spaceAlreadyPressed = true;
            paused = !paused;
        }
        spaceAlreadyPressed = spacePressed;

        // Das hier macht, dass Window Resize auch tut
        int fbw, fbh;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        glViewport(0, 0, fbw, fbh);

        // Framebuffer-Ping-Pong wie wir es kennen und lieben
        int ping = frameIndex % 2;
        int pong = ping ^ 1;

        if (!paused) {
            glUseProgram_(computeProg);
            glUniform2i_(uGridSizeLoc, WIDTH, HEIGHT);
            glBindImageTexture_(0, stateTex[ping], 0, GL_FALSE, 0, GL_READ_ONLY,  GL_R8UI);
            glBindImageTexture_(1, stateTex[pong], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
            glDispatchCompute_((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1);
            glMemoryBarrier_(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
                            |GL_TEXTURE_FETCH_BARRIER_BIT);
        }
        else {
            pong = ping;
        }

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram_(renderProg);
        glUniform1i_(uStateTexLoc, 0);
        glActiveTexture_(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, stateTex[pong]);

        // Rendert auf Back Buffer
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // "Swap" macht den Back Buffer im Fenster sichtbar
        glfwSwapBuffers(window);

        if (!paused) {
            ++frameIndex;
        }
        std::this_thread::sleep_until(nextFrameTime);
    }

    glDeleteVertexArrays_(1, &vao);
    glDeleteTextures(2, stateTex.data());
    glDeleteProgram_(computeProg);
    glDeleteProgram_(renderProg);

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
