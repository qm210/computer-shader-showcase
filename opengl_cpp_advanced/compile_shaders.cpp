#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include "opengl_loader.h"

std::string readFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + path.string());
    }

    file.seekg(0, std::ios::end);
    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string text;
    text.resize(static_cast<size_t>(size));
    if (size > 0) {
        file.read(text.data(), size);
    }
    return text;
}

GLuint compileShader(GLenum type, const char* src) {
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

GLuint linkProgram(std::initializer_list<GLuint> shaders) {
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

namespace embedded_shaders {
    // Automatisch generiert, siehe embed_shaders.cmake
    extern const unsigned char fragment_glsl[];
    extern const std::size_t fragment_glsl_size;
    extern const unsigned char vertex_glsl[];
    extern const std::size_t vertex_glsl_size;
    extern const unsigned char compute_glsl[];
    extern const std::size_t compute_glsl_size;

    static inline std::string_view tryRead(const std::filesystem::path& path) {
        std::string stem = path.stem().string();
        if (stem == "fragment") {
            return {reinterpret_cast<const char*>(fragment_glsl), fragment_glsl_size};
        }
        else if (stem == "vertex") {
            return {reinterpret_cast<const char*>(vertex_glsl), vertex_glsl_size};
        }
        else if (stem == "compute") {
            return {reinterpret_cast<const char*>(compute_glsl), compute_glsl_size};
        }
        else {
            return "";
        }
    }
}

GLuint compileShaderFromFile(GLenum type, const std::filesystem::path& path)
{
    std::string source = std::string(embedded_shaders::tryRead(path));
    if (source.empty()) {
        source = readFile(path);
    }
    const char *src = source.c_str();

    GLuint shader = compileShader(type, src);
    if (!shader) {
        throw std::runtime_error("Invalid Shader: " + path.string());
    }
    return shader;
}

