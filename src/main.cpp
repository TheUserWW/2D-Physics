#include <GL/glew.h>
#include <GLFW/glfw3.h>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif
#include <bits/stdc++.h>
#include "Circle.h"
#include "polygon.h"
#include "textInfo.h"
#include <vector>
#include <memory>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <locale>
#include <codecvt>

#ifdef _WIN32
#include <windows.h>
#endif

std::vector<std::unique_ptr<Object>> objList;

#ifdef _WIN32
HICON g_windowIcon = NULL;
#endif

gravitational_field gf;

// 圆形生成按钮状态
bool circleCreationMode = false;
bool circleButtonPressed = false;
bool mouseWasPressed = false;


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

#ifdef _WIN32
    g_windowIcon = (HICON)LoadImageA(NULL, "d:\\Documents\\C++ project\\2DPhysics\\assets\\icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    if (g_windowIcon) {
        HWND hwnd = glfwGetWin32Window(window);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)g_windowIcon);
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_windowIcon);
    }
#endif

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
                ImGui::SameLine();
                
                // 添加删除按钮
                std::string deleteButtonLabel = "Delete##" + std::to_string(i);
                if (ImGui::Button(deleteButtonLabel.c_str())) {
                    // 标记要删除的物体索引
                    objList.erase(objList.begin() + i);
                    // 由于删除了一个元素，需要调整索引并重新开始循环
                    break;
                }
            }
        }
        
        // Simulation Info Section
        if (ImGui::CollapsingHeader("Simulation Info")) {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("Delta Time: %.3f s", deltaTime);
        }
        
        // Creation Tools Section
        if (ImGui::CollapsingHeader("Creation Tools", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Circle creation button
            if (circleCreationMode) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // Active state green
                if (ImGui::Button("Circle##CircleTool", ImVec2(60, 60))) {
                    circleCreationMode = false;
                    circleButtonPressed = false;
                }
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::Text("Active - Click screen to create circle");
            } else {
                if (ImGui::Button("Circle##CircleTool", ImVec2(60, 60))) {
                    circleCreationMode = true;
                    circleButtonPressed = true;
                }
                ImGui::SameLine();
                ImGui::Text("Click to activate circle creation tool");
            }
            
            ImGui::Text("Status: %s", circleCreationMode ? "Active" : "Inactive");
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


        for (int i = 0; i < objList.size(); i++) {
            objList[i]->update(deltaTime, gf, aspect);
        }
        // 改进的碰撞检测：检查所有物体对
        for (int j = 0; j < objList.size(); j++) {
            for (int k = j + 1; k < objList.size(); k++) {
                if (objList[j]->checkCollision(*objList[k])) {
                    objList[j]->resolveCollision(*objList[k]);
                }
            }
        }
        for (int k = 0; k < objList.size(); k ++) {
            objList[k]->draw();
        }
        // 渲染ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        // 鼠标点击检测 - 圆形生成功能
        if (circleCreationMode) {
            // 检查鼠标左键点击
            int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            
            if (mouseState == GLFW_PRESS && !mouseWasPressed) {
                // 获取鼠标位置
                double mouseX, mouseY;
                glfwGetCursorPos(window, &mouseX, &mouseY);
                
                // 获取窗口尺寸
                int windowWidth, windowHeight;
                glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
                
                // 将鼠标坐标转换为OpenGL坐标
                float glX, glY;
                if (aspect > 1.0f) {
                    glX = (mouseX / windowWidth * 2.0f - 1.0f) * aspect;
                    glY = 1.0f - mouseY / windowHeight * 2.0f;
                } else {
                    glX = mouseX / windowWidth * 2.0f - 1.0f;
                    glY = (1.0f - mouseY / windowHeight * 2.0f) / aspect;
                }
                
                // 在鼠标位置生成圆形
                objList.push_back(std::make_unique<Circle>(glX, glY, 0.1f, 100));
                objList.back()->setMass(1.0f);
                
            }
            
            mouseWasPressed = (mouseState == GLFW_PRESS);
        } else {
            mouseWasPressed = false;
        }

        /* Poll for and process events */
        glfwPollEvents();
    }

    // 清理ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

#ifdef _WIN32
    if (g_windowIcon) {
        DestroyIcon(g_windowIcon);
        g_windowIcon = NULL;
    }
#endif

    glfwTerminate();
    return 0;
}