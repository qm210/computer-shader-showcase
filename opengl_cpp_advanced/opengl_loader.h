#ifndef GOL_SHOWCASE_OPENGL_STUFF_H
#define GOL_SHOWCASE_OPENGL_STUFF_H

// Wie in VL6 angesprochen -- Die OpenGL-DLL stellt nur Grundlegendes bereit,
// und aus cross-platform-Gründen laden wir deswegen alles (es gäbe Helfer wie GLAD),
// aber hier nehmen wir nur das Minimum, das wir brauchen.
#include <GL/glcorearb.h>

#include <vector>
#include <string>
#include <random>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>

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
extern PFNGLUNIFORM1FPROC           glUniform1f_;
extern PFNGLDISPATCHCOMPUTEPROC     glDispatchCompute_;
extern PFNGLMEMORYBARRIERPROC       glMemoryBarrier_;
extern PFNGLBINDIMAGETEXTUREPROC    glBindImageTexture_;
extern PFNGLACTIVETEXTUREPROC       glActiveTexture_;
extern PFNGLGENVERTEXARRAYSPROC     glGenVertexArrays_;
extern PFNGLBINDVERTEXARRAYPROC     glBindVertexArray_;
extern PFNGLDELETEVERTEXARRAYSPROC  glDeleteVertexArrays_;
extern PFNGLGENBUFFERSPROC          glGenBuffers_;
extern PFNGLBINDBUFFERPROC          glBindBuffer_;
extern PFNGLBUFFERDATAPROC          glBufferData_;
extern PFNGLDELETEBUFFERSPROC       glDeleteBuffers_;
extern PFNGLBINDBUFFERBASEPROC      glBindBufferBase_;
extern PFNGLGETBUFFERSUBDATAPROC    glGetBufferSubData_;
extern PFNGLBUFFERSUBDATAPROC       glBufferSubData_;
// Die folgenden sind (ohne Unterstrich) unter Windows aus OpenGL 1.0 vorhanden, aber für Linux:
extern PFNGLGENTEXTURESPROC         glGenTextures_;
extern PFNGLBINDTEXTUREPROC         glBindTexture_;
extern PFNGLTEXPARAMETERIPROC       glTexParameteri_;
extern PFNGLPIXELSTOREIPROC         glPixelStorei_;
extern PFNGLTEXIMAGE2DPROC          glTexImage2D_;
extern PFNGLVIEWPORTPROC            glViewport_;
extern PFNGLDRAWARRAYSPROC          glDrawArrays_;
extern PFNGLDELETETEXTURESPROC      glDeleteTextures_;
extern PFNGLGETSTRINGPROC           glGetString_;

template<typename T>
static bool loadGL(T& funcPtr, const char* name) {
    funcPtr = reinterpret_cast<T>(glfwGetProcAddress(name));
    if (!funcPtr) {
        std::cerr << "Missing GL symbol: " << name << "\n";
        return false;
    }
    return true;
}

static bool loadGLExtensions() {
    bool ok = true;
    ok &= loadGL(glCreateShader_, "glCreateShader");
    ok &= loadGL(glShaderSource_, "glShaderSource");
    ok &= loadGL(glCompileShader_, "glCompileShader");
    ok &= loadGL(glGetShaderiv_, "glGetShaderiv");
    ok &= loadGL(glGetShaderInfoLog_, "glGetShaderInfoLog");
    ok &= loadGL(glDeleteShader_, "glDeleteShader");
    ok &= loadGL(glCreateProgram_, "glCreateProgram");
    ok &= loadGL(glAttachShader_, "glAttachShader");
    ok &= loadGL(glLinkProgram_, "glLinkProgram");
    ok &= loadGL(glGetProgramiv_, "glGetProgramiv");
    ok &= loadGL(glGetProgramInfoLog_, "glGetProgramInfoLog");
    ok &= loadGL(glUseProgram_, "glUseProgram");
    ok &= loadGL(glDeleteProgram_, "glDeleteProgram");
    ok &= loadGL(glGetUniformLocation_, "glGetUniformLocation");
    ok &= loadGL(glUniform2i_, "glUniform2i");
    ok &= loadGL(glUniform1i_, "glUniform1i");
    ok &= loadGL(glUniform1f_, "glUniform1f");
    ok &= loadGL(glDispatchCompute_, "glDispatchCompute");
    ok &= loadGL(glMemoryBarrier_, "glMemoryBarrier");
    ok &= loadGL(glBindImageTexture_, "glBindImageTexture");
    ok &= loadGL(glActiveTexture_, "glActiveTexture");
    ok &= loadGL(glGenVertexArrays_, "glGenVertexArrays");
    ok &= loadGL(glBindVertexArray_, "glBindVertexArray");
    ok &= loadGL(glDeleteVertexArrays_, "glDeleteVertexArrays");
    ok &= loadGL(glGenBuffers_, "glGenBuffers");
    ok &= loadGL(glBindBuffer_, "glBindBuffer");
    ok &= loadGL(glBufferData_, "glBufferData");
    ok &= loadGL(glDeleteBuffers_, "glDeleteBuffers");
    ok &= loadGL(glBindBufferBase_, "glBindBufferBase");
    ok &= loadGL(glGetBufferSubData_, "glGetBufferSubData");
    ok &= loadGL(glBufferSubData_, "glBufferSubData");
    ok &= loadGL(glGenTextures_, "glGenTextures");
    ok &= loadGL(glBindTexture_, "glBindTexture");
    ok &= loadGL(glTexParameteri_, "glTexParameteri");
    ok &= loadGL(glPixelStorei_, "glPixelStorei");
    ok &= loadGL(glTexImage2D_, "glTexImage2D");
    ok &= loadGL(glViewport_, "glViewport");
    ok &= loadGL(glDrawArrays_, "glDrawArrays");
    ok &= loadGL(glDeleteTextures_, "glDeleteTextures");
    ok &= loadGL(glGetString_, "glGetString");
    return ok;
}

static GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader_(type);
    glShaderSource_(shader, 1, &src, nullptr);
    glCompileShader_(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv_(shader, GL_COMPILE_STATUS, &ok);
    if (ok) {
        return shader;
    }

    GLint len;
    glGetShaderiv_(shader, GL_INFO_LOG_LENGTH, &len);
    std::string log(len, '\0');
    glGetShaderInfoLog_(shader, len, nullptr, log.data());
    std::cerr << "compileShader() failed:\n" << log << "\n";
    glDeleteShader_(shader);
    return 0;
}

static GLuint linkProgram(std::initializer_list<GLuint> shaders) {
    GLuint program = glCreateProgram_();
    for (GLuint s : shaders) glAttachShader_(program, s);
    glLinkProgram_(program);

    GLint ok = GL_FALSE;
    glGetProgramiv_(program, GL_LINK_STATUS, &ok);
    if (ok) {
        return program;
    }

    GLint len;
    glGetProgramiv_(program, GL_INFO_LOG_LENGTH, &len);
    std::string log(len, '\0');
    glGetProgramInfoLog_(program, len, nullptr, log.data());
    std::cerr << "linkProgram() failed:\n" << log << "\n";
    exit(EXIT_FAILURE);
}

#endif //GOL_SHOWCASE_OPENGL_STUFF_H
