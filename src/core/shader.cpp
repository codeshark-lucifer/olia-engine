#include <engine/shader.hpp>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

// ---------------- Constructors ----------------

Shader::Shader(const std::string &filepath)
{
    std::string vertex, fragment;

    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("Failed to read file: " + filepath);

    std::string line;
    enum class ShaderType
    {
        NONE,
        VERTEX,
        FRAGMENT
    };
    ShaderType type = ShaderType::NONE;

    while (std::getline(file, line))
    {
        if (line == "#shader vertex")
            type = ShaderType::VERTEX;
        else if (line == "#shader fragment")
            type = ShaderType::FRAGMENT;
        else
        {
            if (type == ShaderType::VERTEX)
                vertex += line + "\n";
            else if (type == ShaderType::FRAGMENT)
                fragment += line + "\n";
        }
    }

    vertexShader = Compile(GL_VERTEX_SHADER, vertex);
    fragmentShader = Compile(GL_FRAGMENT_SHADER, fragment);

    LinkProgram();
}

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath)
{
    vertexShader = Compile(GL_VERTEX_SHADER, ReadFile(vertexPath));
    fragmentShader = Compile(GL_FRAGMENT_SHADER, ReadFile(fragmentPath));
    LinkProgram();
}

// ---------------- Destructor ----------------

Shader::~Shader()
{
    glDeleteProgram(ID);
}

// ---------------- Public ----------------

void Shader::Use() const
{
    glUseProgram(ID);
}

// ---------------- Private ----------------

GLuint Shader::Compile(GLenum type, const std::string &source)
{
    GLuint shader = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    CheckShaderCompile(shader, type == GL_VERTEX_SHADER ? "Vertex" : "Fragment");
    return shader;
}

void Shader::LinkProgram()
{
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);
    CheckProgramLink(ID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::CheckShaderCompile(GLuint shader, const std::string &type)
{
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        throw std::runtime_error(type + " shader compile error:\n" + infoLog);
    }
}

void Shader::CheckProgramLink(GLuint program)
{
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        throw std::runtime_error("Program link error:\n" + std::string(infoLog));
    }
}

std::string Shader::ReadFile(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("Failed to read file: " + filepath);

    std::string content, line;
    while (std::getline(file, line))
        content += line + "\n";

    return content;
}

unsigned int Shader::GetUniformLocation(const std::string &name) const
{
    auto it = m_UniformCache.find(name);
    if (it != m_UniformCache.end())
        return it->second;

    unsigned int location = glGetUniformLocation(ID, name.c_str());
    m_UniformCache[name] = location;
    return location;
}
