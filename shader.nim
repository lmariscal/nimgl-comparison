import nimgl/opengl, strutils, glm/mat, glm/vec, strformat

type
  Shader* = object
    id*: uint32
    source*: ShaderSource
    path*: string
  ShaderSource = object
    vertex*: cstring
    fragment*: cstring
    other*: cstring

converter toString(chars: seq[cchar]): string =
  result = ""
  for c in chars:
    if c == '\0': continue
    result.add(c)

proc readShader*(path: string): ShaderSource =
  var source = readFile(path)

  var
    vertex = ""
    fragment = ""
    other = ""
    index = 0

  for line in source.splitLines:
    if line.startsWith("//") or line == "":
      continue
    
    if line[0] == '#' and not line.startsWith("#version"):
      if line == "#vertex":
        index = 1
      elif line == "#fragment":
        index = 2
      elif line == "#other":
        index = 0
      elif line.startsWith("#include"):
        var name = line["#include ".len ..< line.len]
        name.add(".glsl")
        let otherShader = readShader(path[0 .. path.rfind('/')] & name)
        if index == 0:
          other.add(otherShader.other)
        elif index == 1:
          vertex.add(otherShader.other)
        elif index == 2:
          fragment.add(otherShader.other)
      continue
    
    if index == 0:
      other.add(line & "\n")
    elif index == 1:
      vertex.add(line & "\n")
    elif index == 2:
      fragment.add(line & "\n")
  
  result.other = other
  result.vertex = vertex
  result.fragment = fragment
  echo("shader {path} read".fmt)

proc statusProgram(id: uint32) =
  var status: int32
  glGetProgramiv(id, GL_LINK_STATUS, status.addr)
  if status != GL_TRUE.ord:
    var message = newSeq[cchar](1024)
    glGetProgramInfoLog(id, 1024, nil, message[0].addr)
    echo("failed to compile shader")
    echo(message.toString())

proc statusShader(id: uint32) =
  var status: int32
  glGetShaderiv(id, GL_COMPILE_STATUS, status.addr)
  if status != GL_TRUE.ord:
    var message = newSeq[cchar](1024)
    glGetShaderInfoLog(id, 1024, nil, message[0].addr)
    echo("failed to compile shader")
    echo(message.toString())

proc newShader*(path: string): Shader =
  result.path = path
  result.source = readShader(path)

  var vertexShader = glCreateShader(GL_VERTEX_SHADER)
  vertexShader.glShaderSource(1, result.source.vertex.addr, nil)
  vertexShader.glCompileShader()
  vertexShader.statusShader()

  var fragmentShader = glCreateShader(GL_FRAGMENT_SHADER)
  fragmentShader.glShaderSource(1, result.source.fragment.addr, nil)
  fragmentShader.glCompileShader()
  fragmentShader.statusShader()

  result.id = glCreateProgram()
  result.id.glAttachShader(vertexShader)
  result.id.glAttachShader(fragmentShader)
  result.id.glLinkProgram()
  result.id.statusProgram()

  vertexShader.glDeleteShader()
  fragmentShader.glDeleteShader()

proc destroy*(shader: var Shader) =
  glDeleteProgram(shader.id)

proc use*(shader: Shader) =
  shader.id.glUseProgram()

proc getLocation*(shader: Shader, name: string): int32 =
  shader.use()
  result = shader.id.glGetUniformLocation(name.cstring)
  if result == -1:
    echo("uniform `{name}` not found".fmt)

proc setVec*(shader: Shader, location: int32, vec: var Vec2f) =
  shader.use()
  glUniform2fv(location, 1, vec.caddr)

proc setVec*(shader: Shader, location: int32, vec: var Vec3f) =
  shader.use()
  glUniform3fv(location, 1, vec.caddr)

proc setVec*(shader: Shader, location: int32, vec: var Vec4f) =
  shader.use()
  glUniform4fv(location, 1, vec.caddr)

proc setMat*(shader: Shader, location: int32, mat: var Mat2f) =
  shader.use()
  glUniformMatrix2fv(location, 1, false, mat.caddr)

proc setMat*(shader: Shader, location: int32, mat: var Mat3f) =
  shader.use()
  glUniformMatrix3fv(location, 1, false, mat.caddr)

proc setMat*(shader: Shader, location: int32, mat: var Mat4f) =
  shader.use()
  glUniformMatrix4fv(location, 1, false, mat.caddr)