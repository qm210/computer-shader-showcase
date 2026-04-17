#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
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
constexpr double FPS = 15.0;

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

    // Wähle OpenGL 4.3, weil es ab da Compute Shader gibt
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    const int pxWidth = WIDTH * PIXEL_SCALE;
    const int pxHeight = HEIGHT * PIXEL_SCALE;
    GLFWwindow* window = glfwCreateWindow(pxWidth, pxHeight,
                                          "<Space> zum Pausieren",
                                          nullptr, nullptr);
    if (!window) {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (!loadGLExtensions()) {
        std::cerr << "Failed to load required OpenGL functions\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cout << "GL_VERSION: " << (version ? version : "unknown") << "\n";

    // <-- bis hierhin Infrastruktur-Code für ein Fenster mit OpenGL-Support.
    //     Ab jetzt also unser eigentlicher Anwendungscode.
    //     Wir nutzen zwei Shaderprogramme,
    //     - das Compute-Shader-Programm berechnet den reinen GoL-Zustand,
    //       (das wäre in einem Multipass-Fragment-Shader der Feedback-Pass)
    //     - das Vertex+Fragment-Shader-Programm zeigt die Daten nur graphisch an
    //       (das wäre in einem Multipass-Fragment-Shader der Render-Pass)

    GLuint cs = compileShaderFromFile(GL_COMPUTE_SHADER, "shaders/compute.glsl");
    GLuint computeProgram = linkProgram({cs});

    GLuint vs = compileShaderFromFile(GL_VERTEX_SHADER, "shaders/vertex.glsl");
    GLuint fs = compileShaderFromFile(GL_FRAGMENT_SHADER, "shaders/fragment.glsl");
    GLuint renderProgram = linkProgram({vs, fs});

    // Aufräumen - vergisst man manchmal, merkt man irgendwann vielleicht mal schon.
    glDeleteShader_(cs);
    glDeleteShader_(vs);
    glDeleteShader_(fs);

    // wir werden (s.u.) wieder ein Zustands-Ping-Pong nutzen,
    // das man von Grafikanwendungen mit Framebuffern kennt,
    // aber für den Compute Shadern reichen zwei 8bit-Integer-Texturen:
    auto initialState = initState();
    auto emptyState = std::vector<std::uint8_t>(WIDTH * HEIGHT, 0u);
    std::array<GLuint, 2> stateTex = {
            createStateTexture(initialState),
            createStateTexture(emptyState)
    };

    // Setup-Code der beiden Shaderprogramme:
    GLuint vao = 0;
    glGenVertexArrays_(1, &vao);
    glBindVertexArray_(vao);
    const GLint locTexState = glGetUniformLocation_(renderProgram, "texState");
    const GLint locGridSize = glGetUniformLocation_(computeProgram, "gridSize");

    // Application State für unsere kleine "Engine" hier (für OpenGL uninteressant)
    bool paused = false;
    bool spaceAlreadyPressed = false;
    using clock = std::chrono::steady_clock;
    const auto frameDelta = std::chrono::duration_cast<clock::duration>(
            std::chrono::duration<double>(1.0 / FPS)
    );
    auto nextFrameTime = clock::now();
    int frameIndex = 0;

    // Main Loop für User Input und integrierte Compute- und Render-Aufrufen
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

        // Das hier ist nur für konsistentes Window-Resize da
        int fbw, fbh;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        glViewport(0, 0, fbw, fbh);

        // "Ping-Pong" wie man es von Framebuffern kennt,
        // hier nur auf den Texturen für den Compute-Shader.
        int ping = frameIndex % 2;
        int pong = 1 - ping;

        if (!paused) {
            glUseProgram_(computeProgram);
            glUniform2i_(locGridSize, WIDTH, HEIGHT);
            // Ähnlich zur Auswahl der richtigen Read-Texture und Write-Framebuffer,
            // der erste Index muss eben zum "binding" des uimage2D-Uniforms passen.
            glBindImageTexture_(0, stateTex[ping], 0, GL_FALSE, 0, GL_READ_ONLY,  GL_R8UI);
            glBindImageTexture_(1, stateTex[pong], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
            // Dispatch ist das Compute-Äquivalent zum Rendern (z.B. glDrawArrays).
            // Die Argumente legen die "local size" der "work group" fest,
            // wir landen hiermit auf 16x16 = 256 GPU-Threads, das sollte reichen.
            glDispatchCompute_((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1);
            // diese MemoryBarrier legt fest, dass der Code nicht weiterläuft,
            // bis alle neuen Daten bereit sind, danach als Textur gelesen zu werden.
            glMemoryBarrier_(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
                            |GL_TEXTURE_FETCH_BARRIER_BIT);
        }
        else {
            pong = ping;
        }

        // Das Rendering-Programm auf den Back-Buffer ist recht schmal,
        // es visualisiert einfach die "0u" oder "1u" aus den Texturen.
        glUseProgram_(renderProgram);
        glUniform1i_(locTexState, 0);
        glActiveTexture_(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, stateTex[pong]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // "Swap" macht den Back-Buffer dann im Fenster erst sichtbar
        glfwSwapBuffers(window);

        // und hier noch abschließender Engine-Code:
        if (!paused) {
            ++frameIndex;
        }
        std::this_thread::sleep_until(nextFrameTime);
    }

    // Aufräumen - ist einfach sinnvoll, sich keine Memory Leaks anzugewöhnen.
    glDeleteVertexArrays_(1, &vao);
    glDeleteTextures(2, stateTex.data());
    glDeleteProgram_(computeProgram);
    glDeleteProgram_(renderProgram);
    // ...damit ist OpenGL wieder sauber; auch das Fenster kann jetzt weg:
    glfwDestroyWindow(window);
    glfwTerminate();
    //
    return EXIT_SUCCESS;
}
