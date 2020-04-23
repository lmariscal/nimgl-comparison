#include "mesh.h"

#include "opengl.h"

namespace vx {

Mesh::Mesh(const std::vector<f32> &vertices, const std::vector<u32> &indices):
vertices_count_(vertices.size()), index_count_(indices.size()) {
  glGenBuffers(2, vbo_);

#ifndef __EMSCRIPTEN__
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);
#endif

  glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_[1]);

  glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * vertices_count_, &vertices[0], GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * index_count_, &indices[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32) * 5, (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 5, (void *)(3 * sizeof(f32)));

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Mesh::~Mesh() {
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(2, vbo_);
}

void Mesh::draw() {
  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_[1]);

  glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, (void *)0);

  glBindVertexArray(0);
}

}
