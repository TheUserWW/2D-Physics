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
#include "../include/Circle.h"
#include "../include/polygon.h"
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
bool circleCreationMode = false;
bool circleButtonPressed = false;
bool mouseWasPressed = false;
float newCircleRadius = 0.1f;
float newCircleMass = 1.0f;
float newCircleCharge = 0.0f;
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
            const float dy = list.at(j)->get_position_y() - list.at(i)->get_position_y();

            const float distance = sqrtf(dx * dx + dy * dy);
            
            if (distance < 0.001f) continue;
            
            float forceMagnitude = G * list[i]->get_mass() * list[j]->get_mass() / (distance * distance);
            float fx = forceMagnitude * dx / distance;
            float fy = forceMagnitude * dy / distance;
            
            if (list.at(i)->getMovementStatus()) {
                list.at(i)->applyForce(fx, fy);
            }
            if (list.at(j)->getMovementStatus()) {
                list.at(j)->applyForce(-fx, -fy);
            }
        }
    }
}

void ApplyCoulombForce(std::vector<std::unique_ptr<Object>>& list) {
    for (size_t i = 0; i < list.size(); i++) {
        for (size_t j = i + 1; j < list.size(); j++) {
            if (list.at(i)->getMovementStatus()) {
                list.at(i)->applyCoulombForce(*list[j]);
            }
            if (list.at(j)->getMovementStatus()) {
                list.at(j)->applyCoulombForce(*list[i]);
            }
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


    if (!glfwInit())
        return -1;



    GLFWwindow *window = glfwCreateWindow(1920, 1200, "2D Physics", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

#ifdef _WIN32
    glfwPollEvents();
    Sleep(100);

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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    const float uiWidthPixels = 300.0f;
    const float simulationWidth = width - uiWidthPixels;
    glViewport(uiWidthPixels, 0, simulationWidth, height);
    
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

    double lastTime = glfwGetTime();
    float timeScale = 1.0f;
    bool vSyncEnabled = true;
    bool still = false;
    glfwSwapInterval(vSyncEnabled ? 1 : 0);

    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        const float uiWidthPixels = 300.0f;
        const float simulationWidth = width - uiWidthPixels;
        glViewport(uiWidthPixels, 0, simulationWidth, height);
        
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

    while (!glfwWindowShouldClose(window))
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime) * timeScale;
        lastTime = currentTime;
        
        const float uiWidth = 300.0f;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(uiWidth, ImGui::GetIO().DisplaySize.y));
        
        ImGui::Begin("Physics Controls", nullptr, 
                     ImGuiWindowFlags_NoTitleBar | 
                     ImGuiWindowFlags_NoResize | 
                     ImGuiWindowFlags_NoMove | 
                     ImGuiWindowFlags_NoCollapse);
        
        if (ImGui::CollapsingHeader("Gravity Field", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Magnitude: %.2f m/s²", gf.magnitude);
            ImGui::SliderFloat("##GravityMagnitude", &gf.magnitude, 0.0f, 20.0f, "%.2f");
            
            ImGui::Text("Direction: %.1f°", gf.direction);
            ImGui::SliderFloat("##GravityDirection", &gf.direction, 0.0f, 360.0f, "%.1f°");
            
            ImGui::Text("Direction Reference:");
            ImGui::Text("↑ North: 90°");
            ImGui::Text("→ East: 0°");
            ImGui::Text("↓ South: 270°");
            ImGui::Text("← West: 180°");
            
            ImGui::Text("Quick Direction:");
            if (ImGui::Button("Up##1")) { gf.direction = 90.0f; } ImGui::SameLine();
            if (ImGui::Button("Down##1")) { gf.direction = 270.0f; } ImGui::SameLine();
            if (ImGui::Button("Left##1")) { gf.direction = 180.0f; } ImGui::SameLine();
            if (ImGui::Button("Right##1")) { gf.direction = 0.0f; }
            
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


        if (ImGui::CollapsingHeader("Electric Field", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Magnitude: %.2f N/C", ef.magnitude);
            ImGui::SliderFloat("##ElectricMagnitude", &ef.magnitude, 0.0f, 20.0f, "%.2f");

            ImGui::Text("Direction: %.1f°", gf.direction);
            ImGui::SliderFloat("##ElectricFieldDirection", &ef.direction, 0.0f, 360.0f, "%.1f°");

            ImGui::Text("Quick Direction:");
            if (ImGui::Button("Up##2")) { ef.direction = 90.0f; } ImGui::SameLine();
            if (ImGui::Button("Down##2")) { ef.direction = 270.0f; } ImGui::SameLine();
            if (ImGui::Button("Left##2")) { ef.direction = 180.0f; } ImGui::SameLine();
            if (ImGui::Button("Right##2")) { ef.direction = 0.0f; }


            ImGui::Text("Presets:");
            if (ImGui::Button("Zero Electric Field")) {
                ef.magnitude = 0.0f;
            }
        }



        if (ImGui::CollapsingHeader("Objects", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Total Objects: %zu", objList.size());
            
            for (size_t i = 0; i < objList.size(); ++i) {
                ImGui::Text("Object %zu:", i + 1);
                ImGui::SameLine();
                
                float currentMass = objList[i]->get_mass();
                std::string massLabel = "##Mass" + std::to_string(i);
                ImGui::SetNextItemWidth(100.0f);
                if (ImGui::DragFloat(massLabel.c_str(), &currentMass, 0.1f, 0.1f, 100.0f, "%.2f kg")) {
                    objList[i]->setMass(currentMass);
                }
                ImGui::SameLine();
                
                std::string deleteButtonLabel = "Delete##" + std::to_string(i);
                if (ImGui::Button(deleteButtonLabel.c_str())) {
                    objList.erase(objList.begin() + i);
                    break;
                }
            }
        }
        
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
        
        if (ImGui::CollapsingHeader("Creation Tools", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (circleCreationMode) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
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

            ImGui::Checkbox("Movable", &still);
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
            
            ImGui::Text("Charge: %.2f C", newCircleCharge);
            ImGui::SliderFloat("##CircleCharge", &newCircleCharge, -10.0f, 10.0f, "%.2f");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(90.0f);
            ImGui::InputFloat("##CircleChargeInput", &newCircleCharge, 0.1f, 1.0f, "%.2f");
            
            ImGui::Text("Quick Charge:");
            if (ImGui::Button("Neutral##Charge")) { newCircleCharge = 0.0f; } ImGui::SameLine();
            if (ImGui::Button("+1C##Charge")) { newCircleCharge = 1.0f; } ImGui::SameLine();
            if (ImGui::Button("-1C##Charge")) { newCircleCharge = -1.0f; } ImGui::SameLine();
            if (ImGui::Button("+5C##Charge")) { newCircleCharge = 5.0f; } ImGui::SameLine();
            if (ImGui::Button("-5C##Charge")) { newCircleCharge = -5.0f; }
        }
        
        ImGui::End();
        


        glClear(GL_COLOR_BUFFER_BIT);


        if (!objList.empty()) {
            ApplyUniversalGravitation(objList);
            ApplyCoulombForce(objList);
        }
        
        for (int i = 0; i < objList.size(); i++) {
            if (objList.at(i)->getMovementStatus()) {
                objList.at(i)->update(deltaTime, gf, aspect);
                
                if (ef.magnitude > 0.0f) {
                    Circle* circle = dynamic_cast<Circle*>(objList.at(i).get());
                    if (circle) {
                        circle->update(deltaTime, ef, aspect);
                    }
                }
            }
        }
        for (size_t j = 0; j < objList.size(); j++) {
            for (size_t k = j + 1; k < objList.size(); k++) {
                if (objList.at(j)->checkCollision(*objList[k])) {
                    objList.at(j)->resolveCollision(*objList[k]);
                }
            }
        }


        for (size_t k = 0; k < objList.size(); k ++) {
            objList.at(k)->draw();
        }


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);

        if (!circleCreationMode) {
            const int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            
            int windowWidth, windowHeight;
            glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
            
                float glX, glY;
                const float uiWidthPixels = 300.0f;
                const float simulationWidth = windowWidth - uiWidthPixels;
                
                float adjustedMouseX = mouseX - uiWidthPixels;
                if (adjustedMouseX < 0) adjustedMouseX = 0;
                
                float currentAspect = (float)simulationWidth / (float)windowHeight;
                if (currentAspect > 1.0f) {
                    glX = (adjustedMouseX / simulationWidth * 2.0f - 1.0f) * currentAspect;
                    glY = 1.0f - mouseY / windowHeight * 2.0f;
                } else {
                    glX = adjustedMouseX / simulationWidth * 2.0f - 1.0f;
                    glY = (1.0f - mouseY / windowHeight * 2.0f) / currentAspect;
                }
            
            if (mouseState == GLFW_PRESS && !isDragging) {
                for (size_t i = 0; i < objList.size(); ++i) {
                    const float dx = glX - objList.at(i)->get_position_x();
                    const float dy = glY - objList.at(i)->get_position_y();
                    float distance = sqrt(dx*dx + dy*dy);
                    
                    if (distance < 0.1f) {
                        isDragging = true;
                        draggedObjectIndex = i;
                        dragOffsetX = dx;
                        dragOffsetY = dy;
                        objList.at(i)->setVelocity(0.0f, 0.0f);
                        break;
                    }
                }
            }
            else if (mouseState == GLFW_PRESS && isDragging && draggedObjectIndex != -1) {
                objList.at(draggedObjectIndex)->setPosition(glX - dragOffsetX, glY - dragOffsetY);
                objList.at(draggedObjectIndex)->setVelocity(0.0f, 0.0f);
                
                lastDragX = glX - dragOffsetX;
                lastDragY = glY - dragOffsetY;
                lastDragTime = glfwGetTime();
            }
            else if (mouseState == GLFW_RELEASE && isDragging) {
                double currentTime = glfwGetTime();
                float dragDeltaTime = static_cast<float>(currentTime - lastDragTime);
                
                if (dragDeltaTime > 0.0f && draggedObjectIndex != -1) {
                    float velocityX = (glX - lastDragX) / dragDeltaTime;
                    float velocityY = (glY - lastDragY) / dragDeltaTime;
                    
                    const float maxVelocity = 5.0f;
                    velocityX = std::clamp(velocityX, -maxVelocity, maxVelocity);
                    velocityY = std::clamp(velocityY, -maxVelocity, maxVelocity);
                    
                    objList.at(draggedObjectIndex)->setVelocity(velocityX, velocityY);
                }
                
                isDragging = false;
                draggedObjectIndex = -1;
            }
        }
        
        if (circleCreationMode) {
            int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            
            if (mouseState == GLFW_PRESS && !mouseWasPressed) {
                ImGuiIO& io = ImGui::GetIO();
                if (!io.WantCaptureMouse) {
                    double mouseX, mouseY;
                    glfwGetCursorPos(window, &mouseX, &mouseY);
                    
                    int windowWidth, windowHeight;
                    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
                    
                float glX, glY;
                const float uiWidthPixels = 300.0f;
                const float simulationWidth = windowWidth - uiWidthPixels;
                
                float adjustedMouseX = mouseX - uiWidthPixels;
                if (adjustedMouseX < 0) adjustedMouseX = 0;
                
                float currentAspect = (float)simulationWidth / (float)windowHeight;
                if (currentAspect > 1.0f) {
                    glX = (adjustedMouseX / simulationWidth * 2.0f - 1.0f) * currentAspect;
                    glY = 1.0f - mouseY / windowHeight * 2.0f;
                } else {
                    glX = adjustedMouseX / simulationWidth * 2.0f - 1.0f;
                    glY = (1.0f - mouseY / windowHeight * 2.0f) / currentAspect;
                }
                    objList.emplace_back(std::make_unique<Circle>(glX, glY, newCircleRadius, 100, still));
                    objList.back()->setMass(newCircleMass);
                    objList.back()->setCharge(newCircleCharge);
                }
            }
            
            mouseWasPressed = (mouseState == GLFW_PRESS);
        } else {
            mouseWasPressed = false;
        }

        glfwPollEvents();
    }

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