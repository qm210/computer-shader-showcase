#ifndef GOL_SHOWCASE_OPENGL_STUFF_H
#define GOL_SHOWCASE_OPENGL_STUFF_H

#include <windows.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <random>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>

#ifndef GL_COMPUTE_SHADER
#define GL_COMPUTE_SHADER 0x91B9
#endif
#ifndef GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#endif
#ifndef GL_TEXTURE_FETCH_BARRIER_BIT
#define GL_TEXTURE_FETCH_BARRIER_BIT 0x00000008
#endif
#ifndef GL_READ_ONLY
#define GL_READ_ONLY 0x88B8
#endif
#ifndef GL_WRITE_ONLY
#define GL_WRITE_ONLY 0x88B9
#endif
#ifndef GL_R8UI
#define GL_R8UI 0x8232
#endif
#ifndef GL_RED_INTEGER
#define GL_RED_INTEGER 0x8D94
#endif
#ifndef GL_UNSIGNED_BYTE
#define GL_UNSIGNED_BYTE 0x1401
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif
#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_TEXTURE_WRAP_S
#define GL_TEXTURE_WRAP_S 0x2802
#endif
#ifndef GL_TEXTURE_WRAP_T
#define GL_TEXTURE_WRAP_T 0x2803
#endif
#ifndef GL_TEXTURE_MIN_FILTER
#define GL_TEXTURE_MIN_FILTER 0x2801
#endif
#ifndef GL_TEXTURE_MAG_FILTER
#define GL_TEXTURE_MAG_FILTER 0x2800
#endif
#ifndef GL_NEAREST
#define GL_NEAREST 0x2600
#endif
#ifndef GL_TRIANGLES
#define GL_TRIANGLES 0x0004
#endif
#ifndef GL_COLOR_BUFFER_BIT
#define GL_COLOR_BUFFER_BIT 0x00004000
#endif
#ifndef GL_TEXTURE_2D
#define GL_TEXTURE_2D 0x0DE1
#endif
#ifndef GL_FALSE
#define GL_FALSE 0
#endif
#ifndef GL_TRUE
#define GL_TRUE 1
#endif

using GLchar = char;
using GLintptr = ptrdiff_t;
using GLsizeiptr = ptrdiff_t;

using PFNGLCREATESHADERPROC = GLuint (APIENTRY*)(GLenum);
using PFNGLSHADERSOURCEPROC = void (APIENTRY*)(GLuint, GLsizei, const GLchar* const*, const GLint*);
using PFNGLCOMPILESHADERPROC = void (APIENTRY*)(GLuint);
using PFNGLGETSHADERIVPROC = void (APIENTRY*)(GLuint, GLenum, GLint*);
using PFNGLGETSHADERINFOLOGPROC = void (APIENTRY*)(GLuint, GLsizei, GLsizei*, GLchar*);
using PFNGLDELETESHADERPROC = void (APIENTRY*)(GLuint);
using PFNGLCREATEPROGRAMPROC = GLuint (APIENTRY*)();
using PFNGLATTACHSHADERPROC = void (APIENTRY*)(GLuint, GLuint);
using PFNGLLINKPROGRAMPROC = void (APIENTRY*)(GLuint);
using PFNGLGETPROGRAMIVPROC = void (APIENTRY*)(GLuint, GLenum, GLint*);
using PFNGLGETPROGRAMINFOLOGPROC = void (APIENTRY*)(GLuint, GLsizei, GLsizei*, GLchar*);
using PFNGLUSEPROGRAMPROC = void (APIENTRY*)(GLuint);
using PFNGLDELETEPROGRAMPROC = void (APIENTRY*)(GLuint);
using PFNGLGETUNIFORMLOCATIONPROC = GLint (APIENTRY*)(GLuint, const GLchar*);
using PFNGLUNIFORM2IPROC = void (APIENTRY*)(GLint, GLint, GLint);
using PFNGLUNIFORM1IPROC = void (APIENTRY*)(GLint, GLint);
using PFNGLDISPATCHCOMPUTEPROC = void (APIENTRY*)(GLuint, GLuint, GLuint);
using PFNGLMEMORYBARRIERPROC = void (APIENTRY*)(GLbitfield);
using PFNGLBINDIMAGETEXTUREPROC = void (APIENTRY*)(GLuint, GLuint, GLint, GLboolean, GLint, GLenum, GLenum);
using PFNGLACTIVETEXTUREPROC = void (APIENTRY*)(GLenum);
using PFNGLGENVERTEXARRAYSPROC = void (APIENTRY*)(GLsizei, GLuint*);
using PFNGLBINDVERTEXARRAYPROC = void (APIENTRY*)(GLuint);
using PFNGLDELETEVERTEXARRAYSPROC = void (APIENTRY*)(GLsizei, const GLuint*);
using PFNGLGETSTRINGIPROC = const GLubyte* (APIENTRY*)(GLenum, GLuint);

extern PFNGLCREATESHADERPROC        glCreateShader_;
extern PFNGLSHADERSOURCEPROC        glShaderSource_;
extern PFNGLCOMPILESHADERPROC       glCompileShader_;
extern PFNGLGETSHADERIVPROC         glGetShaderiv_;
extern PFNGLGETSHADERINFOLOGPROC    glGetShaderInfoLog_;
extern PFNGLDELETESHADERPROC        glDeleteShader_;
extern PFNGLCREATEPROGRAMPROC       glCreateProgram_;
extern PFNGLATTACHSHADERPROC        glAttachShader_;
extern PFNGLLINKPROGRAMPROC         glLinkProgram_;
extern PFNGLGETPROGRAMIVPROC        glGetProgramiv_;
extern PFNGLGETPROGRAMINFOLOGPROC   glGetProgramInfoLog_;
extern PFNGLUSEPROGRAMPROC          glUseProgram_;
extern PFNGLDELETEPROGRAMPROC       glDeleteProgram_;
extern PFNGLGETUNIFORMLOCATIONPROC  glGetUniformLocation_;
extern PFNGLUNIFORM2IPROC           glUniform2i_;
extern PFNGLUNIFORM1IPROC           glUniform1i_;
extern PFNGLDISPATCHCOMPUTEPROC     glDispatchCompute_;
extern PFNGLMEMORYBARRIERPROC       glMemoryBarrier_;
extern PFNGLBINDIMAGETEXTUREPROC    glBindImageTexture_;
extern PFNGLACTIVETEXTUREPROC       glActiveTexture_;
extern PFNGLGENVERTEXARRAYSPROC     glGenVertexArrays_;
extern PFNGLBINDVERTEXARRAYPROC     glBindVertexArray_;
extern PFNGLDELETEVERTEXARRAYSPROC  glDeleteVertexArrays_;

static void* LoadGL(const char* name) {
    void* p = reinterpret_cast<void*>(glfwGetProcAddress(name));
    if (!p) {
        std::cerr << "Missing GL symbol: " << name << "\n";
    }
    return p;
}

static bool LoadGLFunctions() {
    glCreateShader_ = reinterpret_cast<PFNGLCREATESHADERPROC>(LoadGL("glCreateShader"));
    glShaderSource_ = reinterpret_cast<PFNGLSHADERSOURCEPROC>(LoadGL("glShaderSource"));
    glCompileShader_ = reinterpret_cast<PFNGLCOMPILESHADERPROC>(LoadGL("glCompileShader"));
    glGetShaderiv_ = reinterpret_cast<PFNGLGETSHADERIVPROC>(LoadGL("glGetShaderiv"));
    glGetShaderInfoLog_ = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(LoadGL("glGetShaderInfoLog"));
    glDeleteShader_ = reinterpret_cast<PFNGLDELETESHADERPROC>(LoadGL("glDeleteShader"));
    glCreateProgram_ = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(LoadGL("glCreateProgram"));
    glAttachShader_ = reinterpret_cast<PFNGLATTACHSHADERPROC>(LoadGL("glAttachShader"));
    glLinkProgram_ = reinterpret_cast<PFNGLLINKPROGRAMPROC>(LoadGL("glLinkProgram"));
    glGetProgramiv_ = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(LoadGL("glGetProgramiv"));
    glGetProgramInfoLog_ = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(LoadGL("glGetProgramInfoLog"));
    glUseProgram_ = reinterpret_cast<PFNGLUSEPROGRAMPROC>(LoadGL("glUseProgram"));
    glDeleteProgram_ = reinterpret_cast<PFNGLDELETEPROGRAMPROC>(LoadGL("glDeleteProgram"));
    glGetUniformLocation_ = reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(LoadGL("glGetUniformLocation"));
    glUniform2i_ = reinterpret_cast<PFNGLUNIFORM2IPROC>(LoadGL("glUniform2i"));
    glUniform1i_ = reinterpret_cast<PFNGLUNIFORM1IPROC>(LoadGL("glUniform1i"));
    glDispatchCompute_ = reinterpret_cast<PFNGLDISPATCHCOMPUTEPROC>(LoadGL("glDispatchCompute"));
    glMemoryBarrier_ = reinterpret_cast<PFNGLMEMORYBARRIERPROC>(LoadGL("glMemoryBarrier"));
    glBindImageTexture_ = reinterpret_cast<PFNGLBINDIMAGETEXTUREPROC>(LoadGL("glBindImageTexture"));
    glActiveTexture_ = reinterpret_cast<PFNGLACTIVETEXTUREPROC>(LoadGL("glActiveTexture"));
    glGenVertexArrays_ = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(LoadGL("glGenVertexArrays"));
    glBindVertexArray_ = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(LoadGL("glBindVertexArray"));
    glDeleteVertexArrays_ = reinterpret_cast<PFNGLDELETEVERTEXARRAYSPROC>(LoadGL("glDeleteVertexArrays"));

    return glCreateShader_ && glShaderSource_ && glCompileShader_ && glGetShaderiv_ &&
           glGetShaderInfoLog_ && glDeleteShader_ && glCreateProgram_ && glAttachShader_ &&
           glLinkProgram_ && glGetProgramiv_ && glGetProgramInfoLog_ && glUseProgram_ &&
           glDeleteProgram_ && glGetUniformLocation_ && glUniform2i_ && glUniform1i_ &&
           glDispatchCompute_ && glMemoryBarrier_ && glBindImageTexture_ && glActiveTexture_ &&
           glGenVertexArrays_ && glBindVertexArray_ && glDeleteVertexArrays_;
}

static GLuint CompileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader_(type);
    glShaderSource_(shader, 1, &src, nullptr);
    glCompileShader_(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv_(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv_(shader, GL_INFO_LOG_LENGTH, &len);
        std::string log(static_cast<size_t>(len), '\0');
        glGetShaderInfoLog_(shader, len, nullptr, log.data());
        std::cerr << "Shader compile failed:\n" << log << "\n";
        glDeleteShader_(shader);
        return 0;
    }
    return shader;
}

static GLuint LinkProgram(std::initializer_list<GLuint> shaders) {
    GLuint prog = glCreateProgram_();
    for (GLuint s : shaders) glAttachShader_(prog, s);
    glLinkProgram_(prog);

    GLint ok = GL_FALSE;
    glGetProgramiv_(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv_(prog, GL_INFO_LOG_LENGTH, &len);
        std::string log(static_cast<size_t>(len), '\0');
        glGetProgramInfoLog_(prog, len, nullptr, log.data());
        std::cerr << "Program link failed:\n" << log << "\n";
        exit(EXIT_FAILURE);
    }
    return prog;
}

#endif //GOL_SHOWCASE_OPENGL_STUFF_H
