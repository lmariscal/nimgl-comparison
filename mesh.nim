import nimgl/opengl

type
  Mesh* = object
    vao*: uint32
    vbo*: uint32
    idx*: uint32
    len: int32

proc newMesh*(vertices: var seq[float32], indices: var seq[uint32]): Mesh =
  glGenVertexArrays(1, result.vao.addr)
  glGenBuffers(1, result.vbo.addr)
  glGenBuffers(1, result.idx.addr)
  glBindVertexArray(result.vao)

  if indices.len == 0:
    for i in 0 ..< (vertices.len / 3).int32:
      indices.add(i.uint32)

  glBindBuffer(GL_ARRAY_BUFFER, result.vbo)
  glBufferData(GL_ARRAY_BUFFER, float32.sizeof * vertices.len, vertices[0].addr, GL_STATIC_DRAW)

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result.idx)
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, int32.sizeof * indices.len, indices[0].addr, GL_STATIC_DRAW)
  result.len = indices.len.int32

  glVertexAttribPointer(0, 3, EGL_FLOAT, false, 5 * float32.sizeof(), nil)
  glEnableVertexAttribArray(0)
  glVertexAttribPointer(1, 2, EGL_FLOAT, false, 5 * float32.sizeof(), cast[pointer](float32.sizeof * 3))
  glEnableVertexAttribArray(1)

proc destroy*(mesh: var Mesh) =
  glDeleteVertexArrays(1, mesh.vao.addr)
  glDeleteBuffers(1, mesh.vbo.addr)
  glDeleteBuffers(1, mesh.idx.addr)

proc draw*(mesh: Mesh) =
  glBindVertexArray(mesh.vao)
  glDrawElements(GL_TRIANGLES, mesh.len, GL_UNSIGNED_INT, nil)