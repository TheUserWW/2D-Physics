//
// Created by wcx16 on 26-1-10.
//

#ifndef AXIOMS_H
#define AXIOMS_H
#define PI 3.14159265358979323846f
#define G 6.67430e-11f
#include <cmath>

struct gravitational_field {
    float magnitude;
    float direction;
};

class Object {

public:
    Object() : mass(1.0f) {
        velocity[0] = 0.0f;
        velocity[1] = 0.0f;
        acceleration[0] = 0.0f;
        acceleration[1] = 0.0f;
    }
    float get_mass() const { return mass; }
    const float* get_velocity() const { return velocity;}
    const float* get_acceleration() const { return acceleration;}
    
    void applyGravitationalField(const gravitational_field& field) {
        // 将角度转换为弧度（0-360度转换为0-2π弧度）
        float angle_rad = field.direction * PI / 180.0f;
        
        // 计算重力在x和y方向的分量
        float fx = field.magnitude * cosf(angle_rad);
        float fy = field.magnitude * sinf(angle_rad);
        
        // 应用力（根据牛顿第二定律 F = ma）
        acceleration[0] += fx / mass;
        acceleration[1] += fy / mass;
    }
    
    void setMass(float m) {
        mass = m;
    }
    
    // 计算两个物体之间的万有引力
    void applyGravitationalForce(const Object& other) {
        // 获取当前物体和其他物体的位置（假设子类有getCenterX和getCenterY方法）
        float dx = other.getCenterX() - getCenterX();
        float dy = other.getCenterY() - getCenterY();
        float distanceSq = dx * dx + dy * dy;
        
        // 避免除以零和极小距离的情况
        if (distanceSq < 0.01f) return;
        
        float distance = sqrtf(distanceSq);
        
        // 计算引力大小 F = G * m1 * m2 / r^2
        float forceMagnitude = G * mass * other.mass / distanceSq;
        
        // 限制最大引力，避免数值不稳定
        forceMagnitude = std::min(forceMagnitude, 1000.0f);
        
        // 计算引力方向单位向量
        float forceX = forceMagnitude * dx / distance;
        float forceY = forceMagnitude * dy / distance;
        
        // 应用力到当前物体
        acceleration[0] += forceX / mass;
        acceleration[1] += forceY / mass;
    }
    
    // 虚函数，子类需要实现获取位置的方法
    virtual float getCenterX() const = 0;
    virtual float getCenterY() const = 0;
    
protected:
    float mass;
    float velocity[2];
    float acceleration[2];
};

#endif //AXIOMS_H
