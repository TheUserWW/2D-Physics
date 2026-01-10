//
// Created by wcx16 on 26-1-10.
//

#ifndef TEXTINFO_H
#define TEXTINFO_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

class TextRenderer {
public:
    static void renderText(GLFWwindow* window, const std::string& text, float x, float y, float scale = 1.0f);
    static void renderGravityInfo(GLFWwindow* window, float magnitude, float direction, bool enabled, float x, float y);
};

#endif //TEXTINFO_H
