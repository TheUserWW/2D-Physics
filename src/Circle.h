//
// Created by wcx16 on 26-1-10.
//
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "axioms.h"


class Circle : public Object {
public:
    Circle(float cx, float cy, float rad, int r) {
        this->center_x = cx;
        this->center_y = cy;
        this->radius = rad;
        this->res = r;
        this->velocity[0] = 0.0f;
        this->velocity[1] = 0.0f;
        this->acceleration[0] = 0.0f;
        this->acceleration[1] = 0.0f;
    }
    void draw() override {
        glBegin(GL_TRIANGLE_FAN);

        // 绘制圆心
        glVertex2f(center_x, center_y);

        // 绘制圆周上的点
        for (int i = 0; i <= res; i++) {
            // 计算角度（确保圆形闭合，从0到2π）
            float angle = 2.0f * PI * i / res;
            float x = center_x + radius * cosf(angle);
            float y = center_y + radius * sinf(angle);
            glVertex2f(x, y);
        }

        glEnd();
    }

    void update(float deltaTime, const gravitational_field& field, float aspect = 1.0f) override {
        // 重置加速度（只保留重力场的影响）
        acceleration[0] = 0.0f;
        acceleration[1] = 0.0f;
        
        // 应用重力场
        applyGravitationalField(field);
        
        // 更新速度
        velocity[0] += acceleration[0] * deltaTime;
        velocity[1] += acceleration[1] * deltaTime;
        
        // 更新位置
        center_x += velocity[0] * deltaTime;
        center_y += velocity[1] * deltaTime;
        
        // 根据宽高比计算实际的边界范围
        float x_bound, y_bound;
        if (aspect > 1.0f) {
            x_bound = aspect;
            y_bound = 1.0f;
        } else {
            x_bound = 1.0f;
            y_bound = 1.0f / aspect;
        }
        
        // 边界检测，让圆在窗口内反弹
        if (center_x + radius > x_bound || center_x - radius < -x_bound) {
            velocity[0] = -velocity[0] * 0.8f; // 反弹并减少能量
        }
        if (center_y + radius > y_bound || center_y - radius < -y_bound) {
            velocity[1] = -velocity[1] * 0.8f; // 反弹并减少能量
        }
        
        // 限制位置在边界内
        center_x = std::max(-x_bound + radius, std::min(x_bound - radius, center_x));
        center_y = std::max(-y_bound + radius, std::min(y_bound - radius, center_y));
    }


    void setVelocity(float vx, float vy) {
        velocity[0] = vx;
        velocity[1] = vy;
    }

    void setAcceleration(float ax, float ay) {
        acceleration[0] = ax;
        acceleration[1] = ay;
    }

    void applyForce(float fx, float fy) {
        acceleration[0] += fx / get_mass();
        acceleration[1] += fy / get_mass();
    }
    
    // 碰撞检测相关方法 - override Object's abstract methods
    bool checkCollision(const Object& other) const override {
        // 尝试将other转换为Circle，如果不是Circle则返回false
        const Circle* otherCircle = dynamic_cast<const Circle*>(&other);
        if (!otherCircle) return false;
        
        float dx = center_x - otherCircle->center_x;
        float dy = center_y - otherCircle->center_y;
        float distance = sqrtf(dx * dx + dy * dy);
        return distance < (radius + otherCircle->radius);
    }
    
    void resolveCollision(Object& other) override {
        // 尝试将other转换为Circle，如果不是Circle则返回
        Circle* otherCircle = dynamic_cast<Circle*>(&other);
        if (!otherCircle) return;
        
        // 计算碰撞法线
        float dx = otherCircle->center_x - center_x;
        float dy = otherCircle->center_y - center_y;
        float distance = sqrtf(dx * dx + dy * dy);
        
        if (distance == 0) return; // 避免除以零
        
        float nx = dx / distance;
        float ny = dy / distance;
        
        // 分离重叠的圆
        float overlap = (radius + otherCircle->radius) - distance;
        if (overlap > 0) {
            float separation = overlap * 0.5f;
            center_x -= separation * nx;
            center_y -= separation * ny;
            otherCircle->center_x += separation * nx;
            otherCircle->center_y += separation * ny;
        }
        
        // 计算相对速度
        float dvx = otherCircle->velocity[0] - velocity[0];
        float dvy = otherCircle->velocity[1] - velocity[1];
        
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
        
        velocity[0] -= impulseX / mass;
        velocity[1] -= impulseY / mass;
        otherCircle->velocity[0] += impulseX / otherCircle->mass;
        otherCircle->velocity[1] += impulseY / otherCircle->mass;
    }
    
    // 获取圆心坐标
    float getCenterX() const { return center_x; }
    float getCenterY() const { return center_y; }
    float getRadius() const { return radius; }
    
private:
    float center_x;
    float center_y;
    float radius;
    int res;
};