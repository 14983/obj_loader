#ifndef __IMGUI_UTIL_H__
#define __IMGUI_UTIL_H__

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <vector>

void setupImGUI(GLFWwindow* window);
void setupImGUIFrame();
void endImGUIFrame();
void clearImGUIContext();

#endif