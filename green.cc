#include <glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void keyProc(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

int main(int argc, char **argv) {
  if (glfwInit() != GLFW_TRUE) {
    std::cerr << "Failed to init GLFW\n";
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  GLFWwindow *w = glfwCreateWindow(800, 600, "NimGL C++", nullptr, nullptr);
  if (!w) {
    std::cerr << "Failed to create Window\n";
    return -1;
  }

  glfwMakeContextCurrent(w);
  glfwSetKeyCallback(w, keyProc);

  if (gladLoadGL() != GL_TRUE) {
    std::cerr << "Failed to load GL\n";
    return -1;
  }

  while (!glfwWindowShouldClose(w)) {
    glfwPollEvents();
    glClearColor(0.68f, 1.0f, 0.34f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(w);
  }

  glfwDestroyWindow(w);
  glfwTerminate();

  return 0;
}
