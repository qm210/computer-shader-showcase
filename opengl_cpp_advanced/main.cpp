#include <GLFW/glfw3.h>
#include <GL/glcorearb.h>
#include "shader_reading.h"
#include "opengl_loader.h"
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
PFNGLUNIFORM1FPROC           glUniform1f_ = nullptr;
PFNGLDISPATCHCOMPUTEPROC     glDispatchCompute_ = nullptr;
PFNGLMEMORYBARRIERPROC       glMemoryBarrier_ = nullptr;
PFNGLBINDIMAGETEXTUREPROC    glBindImageTexture_ = nullptr;
PFNGLACTIVETEXTUREPROC       glActiveTexture_ = nullptr;
PFNGLGENVERTEXARRAYSPROC     glGenVertexArrays_ = nullptr;
PFNGLBINDVERTEXARRAYPROC     glBindVertexArray_ = nullptr;
PFNGLDELETEVERTEXARRAYSPROC  glDeleteVertexArrays_ = nullptr;
PFNGLGENBUFFERSPROC          glGenBuffers_ = nullptr;
PFNGLBINDBUFFERPROC          glBindBuffer_ = nullptr;
PFNGLBUFFERDATAPROC          glBufferData_ = nullptr;
PFNGLDELETEBUFFERSPROC       glDeleteBuffers_ = nullptr;
PFNGLBINDBUFFERBASEPROC      glBindBufferBase_ = nullptr;
PFNGLGETBUFFERSUBDATAPROC    glGetBufferSubData_ = nullptr;
PFNGLBUFFERSUBDATAPROC       glBufferSubData_ = nullptr;
PFNGLGENTEXTURESPROC         glGenTextures_ = nullptr;
PFNGLBINDTEXTUREPROC         glBindTexture_ = nullptr;
PFNGLTEXPARAMETERIPROC       glTexParameteri_ = nullptr;
PFNGLPIXELSTOREIPROC         glPixelStorei_ = nullptr;
PFNGLTEXIMAGE2DPROC          glTexImage2D_ = nullptr;
PFNGLVIEWPORTPROC            glViewport_ = nullptr;
PFNGLDRAWARRAYSPROC          glDrawArrays_ = nullptr;
PFNGLDELETETEXTURESPROC      glDeleteTextures_ = nullptr;
PFNGLGETSTRINGPROC           glGetString_ = nullptr;

#include <array>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <thread>

constexpr int WIDTH = 400;
constexpr int HEIGHT = 300;
constexpr float PIXEL_SCALE = 3.f;
constexpr double FPS = 25.0;

// Anfangszustand zufallsverteilt
constexpr float INIT_SPAWN_CHANCE = 0.2;
constexpr uint8_t INIT_RANDOM_SEED = 0u;
// <Enter> um später nachzubevölkern...
constexpr float ENTER_SPAWN_CHANCE = 0.05;

constexpr float AGE_MAX_SECONDS = 5.f;

struct Cell {
    uint32_t alive;
    float hue;
    float age;
    float ageMax;
};
// Sanity Check, ob Alignment passt (für Entwicklung an Cell)
static_assert(sizeof(Cell) == 16, "struct Cell inkompatibel mit std430");

using Cells = std::vector<Cell>;
GLsizeiptr cellsSize = static_cast<GLsizeiptr>(WIDTH * HEIGHT * sizeof(Cell));

struct AppState {
    std::array<GLuint, 2> buffer{};
    int frame = 0;
    float time = 0.;
    bool paused = false;
};

std::mt19937 rng(INIT_RANDOM_SEED);

static Cells emptyState() {
    return Cells(WIDTH * HEIGHT, Cell{});
}

static void spawnRandom(Cells& cells, float aliveChance) {
    std::bernoulli_distribution alive(aliveChance);
    std::uniform_real_distribution<float> hue(0.f, 1.f);
    for (auto& cell : cells) {
        if (alive(rng)) {
            cell.alive = 1u;
            cell.hue = hue(rng);
            cell.age = 0.f;
            cell.ageMax = AGE_MAX_SECONDS;
        }
    }
}

static Cells initState() {
    Cells result = emptyState();
    spawnRandom(result, INIT_SPAWN_CHANCE);
    return result;
}

static GLuint createStateBuffer(const Cells& cells) {
    GLuint buf = 0;
    glGenBuffers_(1, &buf);
    glBindBuffer_(GL_SHADER_STORAGE_BUFFER, buf);
    glBufferData_(GL_SHADER_STORAGE_BUFFER, cellsSize, cells.data(), GL_DYNAMIC_COPY);
    glBindBuffer_(GL_SHADER_STORAGE_BUFFER, 0);
    return buf;
}

static Cells readStateBuffer(GLuint buf) {
    GLuint count = WIDTH * HEIGHT;
    Cells cells(count);
    glBindBuffer_(GL_SHADER_STORAGE_BUFFER, buf);
    glGetBufferSubData_(GL_SHADER_STORAGE_BUFFER, 0, cellsSize, cells.data());
    glBindBuffer_(GL_SHADER_STORAGE_BUFFER, 0);
    return cells;
}

static GLuint replaceStateBuffer(GLuint buf, const Cells& cells) {
    glBindBuffer_(GL_SHADER_STORAGE_BUFFER, buf);
    glBufferSubData_(GL_SHADER_STORAGE_BUFFER, 0, cellsSize, cells.data());
    glBindBuffer_(GL_SHADER_STORAGE_BUFFER, 0);
    return buf;
}

static void keyCallback(GLFWwindow* win, int key, int, int action, int) {
    auto *state = static_cast<AppState*>(glfwGetWindowUserPointer(win));
    if (action != GLFW_PRESS) {
        return;
    }
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(win, GLFW_TRUE);
    }
    if (key == GLFW_KEY_SPACE) {
        state->paused = !state->paused;
    }
    if (key == GLFW_KEY_ENTER) {
        int pingIndex = state->frame % 2;
        GLuint writeToBuffer = state->buffer[pingIndex];
        Cells cells = readStateBuffer(writeToBuffer);
        spawnRandom(cells, ENTER_SPAWN_CHANCE);
        replaceStateBuffer(writeToBuffer, cells);
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "glfwInit failed\n";
        return EXIT_FAILURE;
    }

    // Brauche mind. OpenGL 4.3, damit es Compute Shader gibt
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    const int pxWidth = WIDTH * PIXEL_SCALE;
    const int pxHeight = HEIGHT * PIXEL_SCALE;
    std::string title("<Space> pausiert, <Esc> beendet, <Enter> erzeugt zufällige Neue.");
    if (AGE_MAX_SECONDS > 0.) {
        title += " Sterben nach " + std::to_string(AGE_MAX_SECONDS) + " sec.";
    }
    GLFWwindow* window = glfwCreateWindow(pxWidth, pxHeight, title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (!loadGLExtensions()) {
        std::cerr << "Failed to load OpenGL functions\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }
    const char* version = reinterpret_cast<const char*>(glGetString_(GL_VERSION));
    std::cout << "GL_VERSION: " << (version ? version : "unknown") << "\n";

    AppState state{};
    glfwSetWindowUserPointer(window, &state);
    glfwSetKeyCallback(window, keyCallback);

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
    glDeleteShader_(cs);
    glDeleteShader_(vs);
    glDeleteShader_(fs);

    // wir werden (s.u.) wieder ein Zustands-Ping-Pong nutzen,
    // das man von Grafikanwendungen mit Framebuffern kennt,
    // aber für den Compute Shadern reichen zwei 8bit-Integer-Texturen:
    state.buffer = {
            createStateBuffer(initState()),
            createStateBuffer(emptyState())
    };

    // Setup Compute-Programm
    const GLint locGridSize = glGetUniformLocation_(computeProgram, "gridSize");
    const GLint locTime = glGetUniformLocation_(computeProgram, "time");
    const GLint locTimeDelta = glGetUniformLocation_(computeProgram, "timeDelta");
    const GLint locAgeMax = glGetUniformLocation_(computeProgram, "ageMaxSec");
    // Setup Render-Programm
    const GLint locGridSizeFrag = glGetUniformLocation_(renderProgram, "gridSize");
    GLuint vao = 0;
    glGenVertexArrays_(1, &vao);
    glBindVertexArray_(vao);

    // Kleine "Engine" für uns (für OpenGL per se uninteressant)
    using clock = std::chrono::steady_clock;
    using seconds = std::chrono::duration<float>;
    const auto frameDelta = std::chrono::duration_cast<clock::duration>(
            std::chrono::duration<double>(1.0 / FPS)
    );
    std::chrono::time_point frameTimePoint = clock::now();

    // Main Loop, integriert, Engine, User Input und OpenGL (Compute- und Render-)Aufrufe
    while (!glfwWindowShouldClose(window)) {
        auto now = clock::now();
        float deltaSeconds = seconds(now - frameTimePoint).count();
        frameTimePoint = now;
        state.time += deltaSeconds;

        glfwPollEvents();

        // für konsistentes Window-Resizing...
        int fbw, fbh;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        glViewport_(0, 0, fbw, fbh);

        // "Ping-Pong" wie man es von Framebuffern kennt und liebt,
        // hier eben auf den beiden SSBO für den Compute-Shader.
        int ping = state.frame % 2;
        int pong = 1 - ping;

        if (state.paused) {
            pong = ping;
        } else {
            glUseProgram_(computeProgram);
            glUniform2i_(locGridSize, WIDTH, HEIGHT);
            glUniform1f_(locTime, state.time);
            glUniform1f_(locTimeDelta, deltaSeconds);
            glUniform1f_(locAgeMax, AGE_MAX_SECONDS);

            // SSBOs binden via Ping-Pong; compute.glsl sagt welchen schreiben, welchen lesen:
            glBindBufferBase_(GL_SHADER_STORAGE_BUFFER, 0, state.buffer[ping]);
            glBindBufferBase_(GL_SHADER_STORAGE_BUFFER, 1, state.buffer[pong]);
            // Dispatch ist das Compute-Äquivalent zum Rendern (z.B. glDrawArrays).
            // Die Argumente legen die "local size" der "work group" fest,
            // wir landen hiermit auf 16x16 = 256 GPU-Threads, das sollte reichen.
            glDispatchCompute_((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1);
            // MemoryBarrier um unten die SSBO sicher ins Rendering zu geben...
            glMemoryBarrier_(GL_SHADER_STORAGE_BARRIER_BIT);
        }

        glUseProgram_(renderProgram);
        glUniform2i_(locGridSizeFrag, WIDTH, HEIGHT);
        glBindBufferBase_(GL_SHADER_STORAGE_BUFFER, 0, state.buffer[pong]);
        glDrawArrays_(GL_TRIANGLES, 0, 3);

        // "Swap" macht den Back-Buffer dann im Fenster erst sichtbar
        glfwSwapBuffers(window);

        // abschließender Engine-Code, für stabile FPS und so
        if (!state.paused) {
            state.frame++;
        }
        std::this_thread::sleep_until(frameTimePoint + frameDelta);
    }

    // Aufräumen - you never know :D
    glDeleteVertexArrays_(1, &vao);
    glDeleteBuffers_(2, state.buffer.data());
    glDeleteProgram_(computeProgram);
    glDeleteProgram_(renderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}

