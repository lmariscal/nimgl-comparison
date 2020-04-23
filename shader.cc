#include "shader.h"

#include <sstream>
#include <fstream>
#include <iostream>

#include "opengl.h"

namespace vx {

bool canWrite(ShaderType type, ShaderWrite shader_write) {
  if (type != ShaderType::None)
    if (shader_write == ShaderWrite::Indifferent || shader_write == ShaderWrite::Native)
      return true;
  return false;
}

std::vector<std::string> ShaderSource::readShader(std::string file_path, ShaderType initial_type) {
  std::stringstream source_stream[2];
  std::string directory = file_path.substr(0, file_path.find_last_of("/") + 1);
  std::string line;
  std::ifstream file(file_path);

  if (!file.is_open()) {
    printf("[opengl] failed to open shader \"%s\"\n", file_path.c_str());
    return std::vector<std::string>{""};
  }

  ShaderWrite shader_write = ShaderWrite::Indifferent;
  ShaderType type = initial_type;
  i32 line_count = 0;

  while (getline(file, line)) {
    line_count++;

    if (line.find("//") == 0)
      continue;
    if (line == "")
      continue;

    if (line[0] == '$') {
      if (line.find("$include ") != std::string::npos) {

        std::string include_path = line.substr(9, line.length());
        std::vector<std::string> include_source = readShader(directory + include_path, ShaderType::Vertex);
        if (include_source.size() < 2) {
          printf("[opengl] failed to include \"%s\"\n\tshader \"%s\":%u\n",
                 (directory + include_path).c_str(), file_path.c_str(), line_count);
          continue;
        }
        if (canWrite(type, shader_write))
          source_stream[(i32)type] << include_source[0] << '\n';

      } else if (line.find("$vertex") != std::string::npos) {
        type = ShaderType::Vertex;
      } else if (line.find("$fragment") != std::string::npos) {
        type = ShaderType::Fragment;
      } else if (line.find("$ignore") != std::string::npos) {
        type = ShaderType::None;
      } else if (line.find("$native") != std::string::npos) {
        shader_write = ShaderWrite::Native;
      } else if (line.find("$emscripten") != std::string::npos) {
        shader_write = ShaderWrite::Emscripten;
      } else if (line.find("$endif") != std::string::npos) {
        shader_write = ShaderWrite::Indifferent;
      }
      continue;
    }

    if (canWrite(type, shader_write))
      source_stream[(i32)type] << line << '\n';
  }

  file.close();

#ifdef BRAVOXEL_DEBUG
  printf("[opengl] shader \"%s\" loaded\n", file_path.c_str());
#endif

  std::vector<std::string> source(2);
  source[0] = source_stream[0].str();
  source[1] = source_stream[1].str();
  return source;
}

ShaderSource::ShaderSource(std::string file_path):
vertex_path(file_path), fragment_path(file_path) {
  std::vector<std::string> source = readShader(file_path);

  if (source.size() < 2)
    return;

  vertex_shader = source[0];
  fragment_shader = source[1];
}

ShaderSource::ShaderSource(std::string vertex_path, std::string fragment_path):
vertex_path(vertex_path), fragment_path(fragment_path) {
  vertex_shader = readShader(vertex_path, ShaderType::Vertex)[0];
  fragment_shader = readShader(fragment_path, ShaderType::Fragment)[1];
}

ShaderSource::ShaderSource(std::string vertex_source, std::string fragment_source,
std::string file_path): vertex_shader(vertex_source), fragment_shader(fragment_source),
vertex_path(file_path), fragment_path(file_path) {
}

ShaderSource::ShaderSource(std::string vertex_source, std::string fragment_source,
std::string vertex_path, std::string fragment_path): vertex_shader(vertex_source),
fragment_shader(fragment_source), vertex_path(vertex_path),
fragment_path(fragment_path) {
}

ShaderSource::~ShaderSource() {
}


Shader::Shader(const ShaderSource &source) {
  createShader(source);
}

Shader::Shader(std::string file_path) {
  ShaderSource source(file_path);
  createShader(source);
}

Shader::~Shader() {
  glDeleteProgram(program_id_);
}

void Shader::createShader(const ShaderSource &source) {
  vertex_id_ = glCreateShader(GL_VERTEX_SHADER);
  const char *vertex_source = source.vertex_shader.c_str();
  glShaderSource(vertex_id_, 1, &vertex_source, nullptr);
  glCompileShader(vertex_id_);
  statusShader(vertex_id_, "Vertex", source.vertex_path);

  fragment_id_ = glCreateShader(GL_FRAGMENT_SHADER);
  const char *fragment_source = source.fragment_shader.c_str();
  glShaderSource(fragment_id_, 1, &fragment_source, nullptr);
  glCompileShader(fragment_id_);
  statusShader(fragment_id_, "Fragment", source.fragment_path);

  program_id_ = glCreateProgram();
  glAttachShader(program_id_, vertex_id_);
  glAttachShader(program_id_, fragment_id_);
  glLinkProgram(program_id_);
  statusProgram(source);

  glDeleteShader(vertex_id_);
  glDeleteShader(fragment_id_);
}

void Shader::statusProgram(const ShaderSource &source) {
  i32 status;
  glGetProgramiv(program_id_, GL_LINK_STATUS, &status);
  if (status == GL_TRUE)
    return;

  i32 log_length;
  glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &log_length);
  std::vector<char> message_array(log_length);
  glGetProgramInfoLog(program_id_, log_length, nullptr, &message_array[0]);
  std::string message(begin(message_array), end(message_array));

  printf("[opengl] failed to link shader program %d:\n\tVertex Shader: \"%s\"\n\tFragment Shader: \"%s\"\n\n%s\n",
          program_id_, source.vertex_path.c_str(), source.fragment_path.c_str(),
          message.c_str());
}

void Shader::statusShader(u32 shader_id, std::string shader_type, std::string file_path) {
  i32 status;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
  if (status == GL_TRUE)
    return;

  i32 log_length;
  glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
  std::vector<char> message_array(log_length);
  glGetShaderInfoLog(shader_id, log_length, nullptr, &message_array[0]);
  std::string message(begin(message_array), end(message_array));

  printf("[opengl] failed to compile shader %s shader \"%s\":\n\n%s\n",
          shader_type.c_str(), file_path.c_str(), message.c_str());
}

void Shader::draw() {
  glUseProgram(program_id_);
}

i32 Shader::getLocation(const std::string &name) {
  return 0;
}

}
