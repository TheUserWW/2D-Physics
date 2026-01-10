#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <bits/stdc++.h>
#include "Circle.h"
#include "textInfo.h"



gravitational_field gf;


int main(void)
{
    GLFWwindow* window;


    gf.magnitude = 0;
    gf.direction = 270;


    /* Initialize the library */
    if (!glfwInit())
        return -1;



    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1920, 1200, "2D Physics", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout<<"Error"<<std::endl;;
    }

    std::cout<<glGetString(GL_VERSION)<<std::endl;

    // 设置正交投影矩阵以保持正确的宽高比
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    // 设置正交投影，根据窗口宽高比调整
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)width / (float)height;
    if (aspect > 1.0) {
        glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0);
    } else {
        glOrtho(-1.0, 1.0, -1.0/aspect, 1.0/aspect, -1.0, 1.0);
    }
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    Circle *c = new Circle(0.0,0.0,0.1,100);
    Circle *c2 = new Circle(1.1,1.0,0.1,100);

    // 设置圆的质量
    c->setMass(7.75);
    c2->setMass(7.7);
    // 时间变量
    double lastTime = glfwGetTime();
    

    // 重新获取当前窗口的宽高比（使用已有的变量）
    glfwGetFramebufferSize(window, &width, &height);
    aspect = (float)width / (float)height;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // 计算时间增量
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        
        // 处理键盘输入来改变重力场
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            gf.direction = 90.0f; // 向上
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            gf.direction = 270.0f; // 向下
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            gf.direction = 180.0f; // 向左
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            gf.direction = 0.0f; // 向右
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            gf.direction = 0.0f;
            gf.magnitude = 0.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
            gf.magnitude += 0.1f; // 增加重力强度
        }
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
            gf.magnitude = std::max(0.0f, gf.magnitude - 0.1f); // 减少重力强度
        }

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);


        // 在更新前应用相互引力
        c->applyGravitationalForce(*c2);
        c2->applyGravitationalForce(*c);
        
        // 调试信息：显示两个圆之间的距离
        float dx = c2->getCenterX() - c->getCenterX();
        float dy = c2->getCenterY() - c->getCenterY();
        float distance = sqrtf(dx*dx + dy*dy);
        std::cout << "distance: " << distance << std::endl;
        
        c->update(deltaTime, gf, aspect);
        c2->update(deltaTime, gf, aspect);

        
        // 检测并处理碰撞
        if (c->checkCollision(*c2)) {
            c->resolveCollision(*c2);
        }
        
        // 绘制圆
        c->draw();
        c2->draw();
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}