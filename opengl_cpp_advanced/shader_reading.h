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

GLuint compileShaderFromFile(GLenum type, const std::filesystem::path& path)
{
    const std::string source = readFile(path);
    const char* src = source.c_str();

    GLuint shader = compileShader(type, src);
    if (!shader) {
        throw std::runtime_error("Invalid Shader: " + path.string());
    }
    return shader;
}