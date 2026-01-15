//
// Created by wcx16 on 26-1-10.
//
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "axioms.h"


class Circle : public Object {
public:
    explicit Circle(float cx, float cy, float rad, int r) : radius(rad), res(r) {
        setPosition(cx, cy);
        setMass(1.0f); // 默认质量
    }
    void draw() override {
        glBegin(GL_TRIANGLE_FAN);

        // 绘制圆心
        glVertex2f(get_position_x(), get_position_y());

        // 绘制圆周上的点
        for (int i = 0; i <= res; i++) {
            // 计算角度（确保圆形闭合，从0到2π）
            float angle = 2.0f * PI * i / res;
            float x = get_position_x() + radius * cosf(angle);
            float y = get_position_y() + radius * sinf(angle);
            glVertex2f(x, y);
        }

        glEnd();
    }

    void update(float deltaTime, const gravitational_field& field, float aspect = 1.0f) override {
        // 使用基类的通用更新逻辑
        basicUpdate(deltaTime, field, aspect);
    }

    void update(float deltaTime, const electric_field& field, float aspect = 1.0f) {
        // 计算电场力
        float angle_rad = field.direction * PI / 180.0f;
        float fx = charge * field.magnitude * cosf(angle_rad);
        float fy = charge * field.magnitude * sinf(angle_rad);
        
        // 应用电场力
        applyForce(fx, fy);
        
        // 使用基类的通用更新逻辑
        basicUpdate(deltaTime, gravitational_field{0, 0}, aspect);
    }

    // 这些方法现在在基类中实现，不需要重复定义
    
    // 碰撞检测相关方法 - override Object's abstract methods
    bool checkCollision(const Object& other) const override {
        // 尝试将other转换为Circle，如果不是Circle则返回false
        const Circle* otherCircle = dynamic_cast<const Circle*>(&other);
        if (!otherCircle) return false;
        
        float dx = get_position_x() - otherCircle->get_position_x();
        float dy = get_position_y() - otherCircle->get_position_y();
        float distance = sqrtf(dx * dx + dy * dy);
        return distance < (radius + otherCircle->radius);
    }
    
    void resolveCollision(Object& other) override {
        // 尝试将other转换为Circle，如果不是Circle则返回
        Circle* otherCircle = dynamic_cast<Circle*>(&other);
        if (!otherCircle) return;
        
        // 计算碰撞法线
        float dx = otherCircle->get_position_x() - get_position_x();
        float dy = otherCircle->get_position_y() - get_position_y();
        float distance = sqrtf(dx * dx + dy * dy);
        
        if (distance == 0) return; // 避免除以零
        
        float nx = dx / distance;
        float ny = dy / distance;
        
        // 分离重叠的圆
        float overlap = (radius + otherCircle->radius) - distance;
        if (overlap > 0) {
            float separation = overlap * 0.5f;
            setPosition(get_position_x() - separation * nx, get_position_y() - separation * ny);
            otherCircle->setPosition(otherCircle->get_position_x() + separation * nx, 
                                   otherCircle->get_position_y() + separation * ny);
        }
        
        // 计算相对速度
        const float* thisVel = get_velocity();
        const float* otherVel = otherCircle->get_velocity();
        float dvx = otherVel[0] - thisVel[0];
        float dvy = otherVel[1] - thisVel[1];
        
        // 计算速度在法线方向的分量
        float velocityAlongNormal = dvx * nx + dvy * ny;
        
        // 如果物体正在分离，不处理碰撞
        if (velocityAlongNormal > 0) return;
        
        // 计算恢复系数（弹性）
        float restitution = 0.8f;
        
        // 计算冲量
        float j = -(1 + restitution) * velocityAlongNormal;
        j /= (1.0f / mass + 1.0f / otherCircle->mass);
        
        // 应用冲量
        float impulseX = j * nx;
        float impulseY = j * ny;
        
        // 使用基类的方法更新速度
        setVelocity(thisVel[0] - impulseX / get_mass(), thisVel[1] - impulseY / get_mass());
        otherCircle->setVelocity(otherVel[0] + impulseX / otherCircle->get_mass(), 
                               otherVel[1] + impulseY / otherCircle->get_mass());
    }
    
    // 获取圆心坐标（现在使用基类的位置）
    float getCenterX() const { return get_position_x(); }
    float getCenterY() const { return get_position_y(); }
    float getRadius() const { return radius; }
    
    // 实现边界处理的虚方法
    void getBoundingBox(float& left, float& right, float& top, float& bottom) const override {
        left = get_position_x() - radius;
        right = get_position_x() + radius;
        top = get_position_y() + radius;
        bottom = get_position_y() - radius;
    }
    
    void constrainToBounds(float x_bound, float y_bound) override {
        setPosition(std::max(-x_bound + radius, std::min(x_bound - radius, get_position_x())),
                   std::max(-y_bound + radius, std::min(y_bound - radius, get_position_y())));
    }
    
private:
    float radius;
    int res;
};