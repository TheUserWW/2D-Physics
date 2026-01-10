#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <bits/stdc++.h>
#include "Circle.h"
#include "textInfo.h"
#include <vector>
#include <memory>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

std::vector<std::unique_ptr<Object>> objList;

gravitational_field gf;


int main(void)
{
    GLFWwindow* window;


    gf.magnitude = 9.8;
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

    // 初始化ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // 启用键盘控制
    
    // 设置ImGui样式
    ImGui::StyleColorsDark();
    
    // 初始化ImGui平台后端
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

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

    objList.push_back(std::make_unique<Circle>(0.0f, 0.0f, 0.1f, 100));
    objList.push_back(std::make_unique<Circle>(1.1f, 1.0f, 0.1f, 100));

    // 设置圆的质量
    objList[0]->setMass(2.0f);
    objList[1]->setMass(2.0f);
    // 时间变量
    double lastTime = glfwGetTime();
    

    // 重新获取当前窗口的宽高比（使用已有的变量）
    glfwGetFramebufferSize(window, &width, &height);
    aspect = (float)width / (float)height;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // 开始ImGui帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // 计算时间增量
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        
        // Create embedded sidebar style
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetIO().DisplaySize.y));
        
        ImGui::Begin("Physics Controls", nullptr, 
                     ImGuiWindowFlags_NoTitleBar | 
                     ImGuiWindowFlags_NoResize | 
                     ImGuiWindowFlags_NoMove | 
                     ImGuiWindowFlags_NoCollapse);
        
        // Gravity Field Control Section
        if (ImGui::CollapsingHeader("Gravity Field", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Magnitude: %.2f m/s²", gf.magnitude);
            ImGui::SliderFloat("##GravityMagnitude", &gf.magnitude, 0.0f, 20.0f, "%.2f");
            
            ImGui::Text("Direction: %.1f°", gf.direction);
            ImGui::SliderFloat("##GravityDirection", &gf.direction, 0.0f, 360.0f, "%.1f°");
            
            // Direction indicators
            ImGui::Text("Direction Reference:");
            ImGui::Text("↑ North: 90°");
            ImGui::Text("→ East: 0°");
            ImGui::Text("↓ South: 270°");
            ImGui::Text("← West: 180°");
            
            // Quick direction buttons
            ImGui::Text("Quick Direction:");
            if (ImGui::Button("Up##1")) { gf.direction = 90.0f; } ImGui::SameLine();
            if (ImGui::Button("Down##1")) { gf.direction = 270.0f; } ImGui::SameLine();
            if (ImGui::Button("Left##1")) { gf.direction = 180.0f; } ImGui::SameLine();
            if (ImGui::Button("Right##1")) { gf.direction = 0.0f; }
            
            // Preset gravity fields
            ImGui::Text("Presets:");
            if (ImGui::Button("Zero Gravity")) {
                gf.magnitude = 0.0f;
            }
            ImGui::SameLine();
            if (ImGui::Button("Earth Gravity")) {
                gf.magnitude = 9.8f;
                gf.direction = 270.0f;
            }
        }
        
        // Object Information Section
        if (ImGui::CollapsingHeader("Objects", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Total Objects: %zu", objList.size());
            
            for (size_t i = 0; i < objList.size(); ++i) {
                ImGui::Text("Object %zu: Mass = %.2f kg", i + 1, objList[i]->get_mass());
            }
        }
        
        // Simulation Info Section
        if (ImGui::CollapsingHeader("Simulation Info")) {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("Delta Time: %.3f s", deltaTime);
        }
        
        ImGui::End();
        
        // 保留键盘控制（作为备用）
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


        objList[0]->update(deltaTime, gf, aspect);
        objList[1]->update(deltaTime, gf, aspect);

        
        // 检测并处理碰撞
        if (objList[0]->checkCollision(*objList[1])) {
            objList[0]->resolveCollision(*objList[1]);
        }
        
        // 绘制圆
        objList[0]->draw();
        objList[1]->draw();
        
        // 渲染ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    // 清理ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}