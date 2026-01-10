//
// Created by wcx16 on 26-1-10.
//

#include "textInfo.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

// 使用简单的四边形绘制文本（最可靠的方法）
void TextRenderer::renderText(GLFWwindow* window, const std::string& text, float x, float y, float scale) {
    // 保存当前矩阵状态
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glOrtho(0, width, height, 0, -1, 1); // Y轴从上到下
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // 设置文本颜色（白色）
    glColor3f(1.0f, 1.0f, 1.0f);
    
    float charWidth = 8.0f * scale;
    float lineHeight = 15.0f * scale;
    float currentX = x;
    float currentY = y;
    
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        
        if (c == '\n') {
            currentX = x;
            currentY += lineHeight;
            continue;
        }
        
        // 使用四边形绘制字符（简单可靠）
        glBegin(GL_QUADS);
        
        // 每个字符绘制一个小的矩形
        glVertex2f(currentX, currentY);
        glVertex2f(currentX + charWidth, currentY);
        glVertex2f(currentX + charWidth, currentY + lineHeight/3);
        glVertex2f(currentX, currentY + lineHeight/3);
        
        glEnd();
        
        currentX += charWidth;
    }
    
    // 恢复矩阵状态
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void TextRenderer::renderGravityInfo(GLFWwindow* window, float magnitude, float direction, bool enabled, float x, float y) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    
    ss << "Gravity: " << (enabled ? "ON" : "OFF") << "\n";
    ss << "Strength: " << magnitude << " m/s2\n";
    
    // 将角度转换为方向描述
    std::string directionStr;
    if (direction >= 315 || direction < 45) directionStr = "RIGHT";
    else if (direction >= 45 && direction < 135) directionStr = "UP";
    else if (direction >= 135 && direction < 225) directionStr = "LEFT";
    else directionStr = "DOWN";
    
    ss << "Direction: " << directionStr << " (" << direction << "°)\n";
    ss << "Controls:\n";
    ss << "Arrow Keys: Change direction\n";
    ss << "Space: Toggle gravity\n";
    ss << "G/H: Adjust strength";
    
    // 渲染文本
    renderText(window, ss.str(), x, y, 2.0f); // 使用更大的缩放因子
}