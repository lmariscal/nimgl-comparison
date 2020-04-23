// Written by Leonardo Mariscal <leo@ldmd.mx>, 2019

#include "window.h"

#include "opengl.h"
#include "input_manager.h"

#include <iostream>
#include <GLFW/glfw3.h>

namespace vx {

bool Window::glad_is_init = false;
bool Window::glfw_is_init = false;
u8 Window::window_refs = 0;

void errorEvent(i32 error, const char *description) {
  printf("[glfw] %d: %s\n", error, description);
}

void keyEvent(GLFWwindow *window, i32 key, i32 scancode, i32 action, i32 mods) {
  InputManager::keyEvent((vx::Key)key, action != GLFW_RELEASE);
}

void mouseButtonEvent(GLFWwindow *window, i32 button, i32 action, i32 mods) {
  InputManager::mouseButtonEvent((vx::Mouse)button, action != GLFW_RELEASE);
}

void mouseEvent(GLFWwindow *window, f64 x_pos, f64 y_pos) {
  InputManager::mouseEvent({ x_pos, y_pos });
}

Window::Window(uvec2 size): size_(size) {
  if (!glfw_is_init) {
    if (!glfwInit()) {
      printf("[glfw] failed to init glfw\n");
      return;
    }
    glfw_is_init = true;
  }

  window_ = glfwCreateWindow(size_.x, size_.y, "Bravoxel", nullptr, nullptr);
  if (!window_) {
    printf("[glfw] failed to create window\n");
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(window_);
  glfwSetErrorCallback(errorEvent);
  glfwSetKeyCallback(window_, keyEvent);
  glfwSetMouseButtonCallback(window_, mouseButtonEvent);
  glfwSetCursorPosCallback(window_, mouseEvent);

  f64 x_pos, y_pos;
  glfwGetCursorPos(window_, &x_pos, &y_pos);
  InputManager::mouseEvent({ x_pos, y_pos });

#ifdef __EMSCRIPTEN__
  glad_is_init = true;
#else
  if (!glad_is_init) {
    if (!gladLoadGL()) {
      printf("[glad] failed to load OpenGL\n");
      window_ = nullptr;
      return;
    }
    glad_is_init = true;
  }
#endif

  window_refs++;
}

Window::~Window() {
  glfwDestroyWindow(window_);

  if (--window_refs > 0)
    return;
  glfwTerminate();
  glfw_is_init = false;
}

bool Window::isOpen() {
  return !glfwWindowShouldClose(window_) &&
         !(InputManager::isDown(Key::Q) && InputManager::isDown(Key::LeftCtrl));
}

void Window::update() {
  glfwPollEvents();

  f64 now = glfwGetTime();
  if (now > future_time_) {
    mspf = mspf_count_ / (f64)frames_count_;
    future_time_ = now + 0.03;
    frames_count_ = 0;
    mspf_count_ = 0;
  }
  mspf_count_ += (now - last_time_) * 1000;
  last_time_ = now;
  frames_count_++;
}

void Window::swapBuffers() {
  glfwSwapBuffers(window_);
}

const uvec2& Window::getSize() {
  return size_;
}

}
