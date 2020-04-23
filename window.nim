import glfw, nimgl/opengl, nimgl/imgui, glm/vec

type
  Window* = object
    height*: uint16
    width*: uint16
    raw*: GLFWWindow
    id*: uint8

var isGLFWInit = false
var isGLInit = false
var isIGInit = false
var activeWindowsCount = 0
var igContext: ptr ImGuiContext
var pastTime: float32 = 0
var deltaTime*: float32 = 0

proc glfwErrorEvent(error: int32, description: cstring): void {.cdecl.} =
  echo($description)

proc keyEvent(window: GLFWWindow, key: int32, scancode: int32, action: int32, mods: int32): void {.cdecl.} =
  iomanager.keyEvent(key, action != GLFWRelease)
  igGlfwKeyCallback(window, key, scancode, action, mods)

proc scrollEvent(window: GLFWWindow, xoff: float64, yoff: float64): void {.cdecl.} =
  igGlfwScrollCallback(window, xoff, yoff)

proc charEvent(window: GLFWWindow, code: uint32): void {.cdecl.} =
  igGlfwCharCallback(window, code)

proc cursorPos(window: GLFWWindow, xpos: float64, ypos: float64): void {.cdecl.} =
  iomanager.mouseEvent(vec2(xpos.float32, ypos.float32))

proc mouseEvent(window: GLFWWindow, button: int32, action: int32, mods: int32): void {.cdecl.} =
  iomanager.mouseButtonEvent(button, action != GLFWRelease)
  igGlfwMouseCallback(window, button, action, mods)

proc resizeEvent(window: GLFWWindow, width: int32, height: int32): void {.cdecl.} =
  glViewport(0, width, 0, height)

proc newWindow*(width: uint16, height: uint16, makeCurrent: bool = true): Window =
  if not isGLFWInit:
    discard glfwInit()

  discard glfwSetErrorCallback(glfwErrorEvent)

  glfwWindowHint(GLFWContextVersionMajor, 3)
  glfwWindowHint(GLFWContextVersionMinor, 3)
  when defined(macosx):
    glfwWindowHint(GLFWOpenglForwardCompat, GLFW_TRUE)
  glfwWindowHint(GLFWOpenglProfile, GLFW_OPENGL_CORE_PROFILE)
  glfwWindowHint(GLFWResizable, GLFW_FALSE)

  result.width = width
  result.height = height
  result.raw = glfwCreateWindow(width.int32, height.int32, "Mint")
  if result.raw == nil:
    echo("failed to create window({width}x{height})".fmt)
  if makeCurrent:
    result.raw.makeContextCurrent()
  result.id = activeWindowsCount.uint8
  activeWindowsCount.inc

  var
    xpos: float64
    ypos: float64
  result.raw.getCursorPos(xpos.addr, ypos.addr)
  iomanager.mouseEvent(vec2(xpos.float32, ypos.float32))

  discard result.raw.setKeyCallback(keyEvent)
  discard result.raw.setMouseButtonCallback(mouseEvent)
  discard result.raw.setScrollCallback(scrollEvent)
  discard result.raw.setCharCallback(charEvent)
  discard result.raw.setWindowSizeCallback(resizeEvent)
  discard result.raw.setCursorPosCallback(cursorPos)

  # OpenGL

  if not isGLInit:
    if not glInit():
      echo("failed to init opengl")
    isGLInit = true

  # ImGui

  if not isIGInit:
    igContext = igCreateContext()
    if not igGlfwInitForOpenGL(result.raw, false):
      echo("failed to init glfw impl")
    if not igOpenGL3Init():
      echo("failed to init opengl impl")
    isIGInit = true

    let io = igGetIO()
    io.fonts.addFontFromFileTTF("res/fonts/recursive.otf", 15)

    igStyleColorsCherry()

proc destroy*(window: Window) =
  window.raw.destroyWindow()
  activeWindowsCount.dec

  if activeWindowsCount <= 0:
    isGLFWInit = false
    igOpenGL3Shutdown()
    igGlfwShutdown()
    igContext.igDestroyContext()
    isIGInit = false
    glfwTerminate()

proc isOpen*(window: Window): bool =
  result = not window.raw.windowShouldClose()
  if GLFWKey.Q.isPressed() and GLFWKey.LeftControl.isPressed():
    result = false

proc newFrame*(window: Window) =
  var nowTime = glfwGetTime()
  deltaTime = nowTime - pastTime
  pastTime = nowTime

  glfwPollEvents()
  iomanager.updateGamePad()
  igOpenGL3NewFrame()
  igGlfwNewFrame()
  igNewFrame()

proc swapBuffers*(window: Window) =
  igRender()
  igOpenGL3RenderDrawData(igGetDrawData())
  window.raw.swapBuffers()
