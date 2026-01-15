#include <GL/glew.h>
#include <GLFW/glfw3.h>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif
#include <vector>
#include <memory>
#include <iostream>
#include <print>
#include "Circle.h"
#include "polygon.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#ifdef _WIN32
#include <windows.h>
#endif


std::vector<std::unique_ptr<Object>> objList;

#ifdef _WIN32
HICON g_windowIcon = NULL;
#endif

gravitational_field gf;
electric_field ef;

// 圆形生成按钮状态
bool circleCreationMode = false;
bool circleButtonPressed = false;
bool mouseWasPressed = false;

// 圆形创建参数
float newCircleRadius = 0.1f;
float newCircleMass = 1.0f;

// 物体拖拽相关变量
bool isDragging = false;
int draggedObjectIndex = -1;
float dragOffsetX = 0.0f;
float dragOffsetY = 0.0f;
double lastDragTime = 0.0f;
float lastDragX = 0.0f;
float lastDragY = 0.0f;

void ApplyUniversalGravitation(std::vector<std::unique_ptr<Object>>& list) {
    for (size_t i = 0; i < list.size(); i++) {
        for (int j = i + 1; j < list.size(); j++) {
            const float dx = list.at(j)->get_position_x() - list.at(i)->get_position_x();
            const float dy = list.at(i)->get_position_y() - list.at(i)->get_position_y();

            const float distance = sqrtf(dx * dx + dy * dy);
            
            if (distance < 0.001f) continue;
            
            float forceMagnitude = G * list[i]->get_mass() * list[j]->get_mass() / (distance * distance);
            float fx = forceMagnitude * dx / distance;
            float fy = forceMagnitude * dy / distance;
            
            list.at(i)->applyForce(fx, fy);
            list.at(j)->applyForce(-fx, -fy);
        }
    }
}

int main(void)
{
    gf.magnitude = 9.8;
    gf.direction = 270;

    ef.magnitude = 0.0;
    ef.direction = 0.0;
    ef.positive = true;


    /* Initialize the library */
    if (!glfwInit())
        return -1;



    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow *window = glfwCreateWindow(1920, 1200, "2D Physics", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // 在 main 函数中，窗口创建后添加延迟
#ifdef _WIN32
    // 等待窗口完全初始化
    glfwPollEvents();
    Sleep(100); // 短暂延迟确保窗口完全创建

    // 从文件加载自定义图标
    HINSTANCE hInstance = GetModuleHandle(NULL);
    HICON g_windowIcon = (HICON)LoadImageA(
        hInstance,
        "icon.ico",
        IMAGE_ICON,
        0, 0,
        LR_LOADFROMFILE | LR_DEFAULTSIZE
    );

    if (g_windowIcon) {
        HWND hwnd = glfwGetWin32Window(window);
        if (hwnd) {
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)g_windowIcon);
            SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_windowIcon);

            // 强制窗口重绘
            SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }
    }
#endif

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::println("Error");
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
    
    // 设置视口，为UI留出左侧空间
    const float uiWidthPixels = 300.0f; // UI宽度（像素）
    const float simulationWidth = width - uiWidthPixels;
    glViewport(uiWidthPixels, 0, simulationWidth, height);
    
    // 设置正交投影，根据模拟区域宽高比调整
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)simulationWidth / (float)height;
    if (aspect > 1.0) {
        glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0);
    } else {
        glOrtho(-1.0, 1.0, -1.0/aspect, 1.0/aspect, -1.0, 1.0);
    }
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 时间变量
    double lastTime = glfwGetTime();
    float timeScale = 1.0f; // 时间流速控制变量
    bool vSyncEnabled = true; // 垂直同步设置
    
    // 设置初始垂直同步
    glfwSwapInterval(vSyncEnabled ? 1 : 0);

    // 窗口大小变化回调
    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        // 更新视口，为UI留出左侧空间
        const float uiWidthPixels = 300.0f; // UI宽度（像素）
        const float simulationWidth = width - uiWidthPixels;
        glViewport(uiWidthPixels, 0, simulationWidth, height);
        
        // 重新计算正交投影
        float aspect = (float)simulationWidth / (float)height;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if (aspect > 1.0) {
            glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0);
        } else {
            glOrtho(-1.0, 1.0, -1.0/aspect, 1.0/aspect, -1.0, 1.0);
        }
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    });

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // 开始ImGui帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // 计算时间增量
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime) * timeScale;
        lastTime = currentTime;
        
        // 创建侧边栏UI，固定在左侧，不覆盖模拟画面
        const float uiWidth = 300.0f; // UI宽度
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(uiWidth, ImGui::GetIO().DisplaySize.y));
        
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


        if (ImGui::CollapsingHeader("Electric Field (Developing)", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Magnitude: %.2f N/C", ef.magnitude);
            ImGui::SliderFloat("##ElectricMagnitude", &ef.magnitude, 0.0f, 20.0f, "%.2f");
            // Quick direction buttons
            ImGui::Text("Quick Direction:");
            if (ImGui::Button("Up##2")) { ef.direction = 90.0f; } ImGui::SameLine();
            if (ImGui::Button("Down##2")) { ef.direction = 270.0f; } ImGui::SameLine();
            if (ImGui::Button("Left##2")) { ef.direction = 180.0f; } ImGui::SameLine();
            if (ImGui::Button("Right##2")) { ef.direction = 0.0f; }


            // Preset gravity fields
            ImGui::Text("Presets:");
            if (ImGui::Button("Zero Electric Field")) {
                ef.magnitude = 0.0f;
            }
        }



        // Object Information Section
        if (ImGui::CollapsingHeader("Objects", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Total Objects: %zu", objList.size());
            
            for (size_t i = 0; i < objList.size(); ++i) {
                ImGui::Text("Object %zu:", i + 1);
                ImGui::SameLine();
                
                float currentMass = objList[i]->get_mass();
                std::string massLabel = "##Mass" + std::to_string(i);
                ImGui::SetNextItemWidth(100.0f); // 设置质量输入框宽度
                if (ImGui::DragFloat(massLabel.c_str(), &currentMass, 0.1f, 0.1f, 100.0f, "%.2f kg")) {
                    objList[i]->setMass(currentMass);
                }
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
            ImGui::Text("Time Scale: %.2fx", timeScale);
            ImGui::SliderFloat("##TimeScale", &timeScale, 0.0f, 2.0f, "%.2fx");
            ImGui::Checkbox("Vertical Sync", &vSyncEnabled);
            if (ImGui::IsItemEdited()) {
                glfwSwapInterval(vSyncEnabled ? 1 : 0);
            }
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
            
            // Circle parameters
            ImGui::Separator();
            ImGui::Text("Circle Parameters:");
            
            ImGui::Text("Radius: %.3f", newCircleRadius);
            ImGui::SliderFloat("##CircleRadius", &newCircleRadius, 0.01f, 0.5f, "%.3f");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(90.0f);
            ImGui::InputFloat("##CircleRadiusInput", &newCircleRadius, 0.01f, 0.1f, "%.3f");
            
            ImGui::Text("Mass: %.2f kg", newCircleMass);
            ImGui::SliderFloat("##CircleMass", &newCircleMass, 0.1f, 100.0f, "%.2f");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(90.0f);
            ImGui::InputFloat("##CircleMassInput", &newCircleMass, 0.1f, 1.0f, "%.2f");
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


        if (!objList.empty()) {
            ApplyUniversalGravitation(objList);
        }
        
        for (int i = 0; i < objList.size(); i++) {
            objList.at(i)->update(deltaTime, gf, aspect);
        }
        // 改进的碰撞检测：检查所有物体对
        for (size_t j = 0; j < objList.size(); j++) {
            for (size_t k = j + 1; k < objList.size(); k++) {
                if (objList.at(j)->checkCollision(*objList[k])) {
                    objList.at(j)->resolveCollision(*objList[k]);
                }
            }
        }


        //Draw each object
        for (size_t k = 0; k < objList.size(); k ++) {
            objList.at(k)->draw();
        }


        // 渲染ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        // 鼠标拖拽物体功能
        if (!circleCreationMode) {
            int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            
            // 获取窗口尺寸
            int windowWidth, windowHeight;
            glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
            
            // 将鼠标坐标转换为OpenGL坐标（考虑UI区域偏移）
                float glX, glY;
                const float uiWidthPixels = 300.0f;
                const float simulationWidth = windowWidth - uiWidthPixels;
                
                // 调整鼠标X坐标，减去UI区域宽度
                float adjustedMouseX = mouseX - uiWidthPixels;
                if (adjustedMouseX < 0) adjustedMouseX = 0; // 确保不超出模拟区域
                
                float currentAspect = (float)simulationWidth / (float)windowHeight;
                if (currentAspect > 1.0f) {
                    glX = (adjustedMouseX / simulationWidth * 2.0f - 1.0f) * currentAspect;
                    glY = 1.0f - mouseY / windowHeight * 2.0f;
                } else {
                    glX = adjustedMouseX / simulationWidth * 2.0f - 1.0f;
                    glY = (1.0f - mouseY / windowHeight * 2.0f) / currentAspect;
                }
            
            // 开始拖拽
            if (mouseState == GLFW_PRESS && !isDragging) {
                // 检查鼠标是否在某个物体上
                for (size_t i = 0; i < objList.size(); ++i) {
                    // 简单的碰撞检测（假设是圆形）
                    const float dx = glX - objList.at(i)->get_position_x();
                    const float dy = glY - objList.at(i)->get_position_y();
                    float distance = sqrt(dx*dx + dy*dy);
                    
                    // 假设所有物体都是圆形，半径为0.1f（可以根据实际情况调整）
                    if (distance < 0.1f) {
                        isDragging = true;
                        draggedObjectIndex = i;
                        dragOffsetX = dx;
                        dragOffsetY = dy;
                        // 拖拽开始时重置物体速度，避免抽搐
                        objList.at(i)->setVelocity(0.0f, 0.0f);
                        break;
                    }
                }
            }
            // 拖拽中
            else if (mouseState == GLFW_PRESS && isDragging && draggedObjectIndex != -1) {
                // 更新拖拽物体的位置
                objList.at(draggedObjectIndex)->setPosition(glX - dragOffsetX, glY - dragOffsetY);
                // 拖拽过程中保持速度为0，避免物理模拟影响
                objList.at(draggedObjectIndex)->setVelocity(0.0f, 0.0f);
                
                // 记录拖拽位置和时间，用于计算惯性
                lastDragX = glX - dragOffsetX;
                lastDragY = glY - dragOffsetY;
                lastDragTime = glfwGetTime();
            }
            // 结束拖拽
            else if (mouseState == GLFW_RELEASE && isDragging) {
                // 计算拖拽结束时的速度，赋予物体惯性
                double currentTime = glfwGetTime();
                float dragDeltaTime = static_cast<float>(currentTime - lastDragTime);
                
                if (dragDeltaTime > 0.0f && draggedObjectIndex != -1) {
                    float velocityX = (glX - lastDragX) / dragDeltaTime;
                    float velocityY = (glY - lastDragY) / dragDeltaTime;
                    
                    // 限制最大速度，避免惯性过大
                    const float maxVelocity = 5.0f;
                    velocityX = std::clamp(velocityX, -maxVelocity, maxVelocity);
                    velocityY = std::clamp(velocityY, -maxVelocity, maxVelocity);
                    
                    // 赋予物体惯性
                    objList.at(draggedObjectIndex)->setVelocity(velocityX, velocityY);
                }
                
                isDragging = false;
                draggedObjectIndex = -1;
            }
        }
        
        // 鼠标点击检测 - 圆形生成功能
        if (circleCreationMode) {
            // 检查鼠标左键点击
            int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            
            if (mouseState == GLFW_PRESS && !mouseWasPressed) {
                // 检查鼠标是否在UI上，避免在UI上生成圆形
                ImGuiIO& io = ImGui::GetIO();
                if (!io.WantCaptureMouse) {
                    // 获取鼠标位置
                    double mouseX, mouseY;
                    glfwGetCursorPos(window, &mouseX, &mouseY);
                    
                    // 获取窗口尺寸
                    int windowWidth, windowHeight;
                    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
                    
                    // 将鼠标坐标转换为OpenGL坐标（考虑UI区域偏移）
                float glX, glY;
                const float uiWidthPixels = 300.0f;
                const float simulationWidth = windowWidth - uiWidthPixels;
                
                // 调整鼠标X坐标，减去UI区域宽度
                float adjustedMouseX = mouseX - uiWidthPixels;
                if (adjustedMouseX < 0) adjustedMouseX = 0; // 确保不超出模拟区域
                
                float currentAspect = (float)simulationWidth / (float)windowHeight;
                if (currentAspect > 1.0f) {
                    glX = (adjustedMouseX / simulationWidth * 2.0f - 1.0f) * currentAspect;
                    glY = 1.0f - mouseY / windowHeight * 2.0f;
                } else {
                    glX = adjustedMouseX / simulationWidth * 2.0f - 1.0f;
                    glY = (1.0f - mouseY / windowHeight * 2.0f) / currentAspect;
                }
                    
                    // 在鼠标位置生成圆形
                    objList.emplace_back(std::make_unique<Circle>(glX, glY, newCircleRadius, 100));
                    objList.back()->setMass(newCircleMass);
                }
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