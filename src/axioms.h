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

struct electric_field {
    float magnitude;
    float direction;
    bool positive;
};

class Object {

public:
    Object() : mass(1.0f), position_x(0.0f), position_y(0.0f) {
        velocity[0] = 0.0f;
        velocity[1] = 0.0f;
        acceleration[0] = 0.0f;
        acceleration[1] = 0.0f;
    }
    virtual ~Object() = default;
    
    // 基本属性访问器
    float get_mass() const { return mass; }
    const float* get_velocity() const { return velocity;}
    const float* get_acceleration() const { return acceleration;}
    float get_position_x() const { return position_x; }
    float get_position_y() const { return position_y; }
    
    // 通用物理方法
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
    
    void setPosition(float x, float y) {
        position_x = x;
        position_y = y;
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
        acceleration[0] += fx / mass;
        acceleration[1] += fy / mass;
    }
    
    // 通用的边界检测和反弹方法
    void handleBoundaryCollision(float aspect = 1.0f, float restitution = 0.8f) {
        // 根据宽高比计算实际的边界范围
        float x_bound, y_bound;
        if (aspect > 1.0f) {
            x_bound = aspect;
            y_bound = 1.0f;
        } else {
            x_bound = 1.0f;
            y_bound = 1.0f / aspect;
        }
        
        // 获取物体的边界（由子类实现）
        float left, right, top, bottom;
        getBoundingBox(left, right, top, bottom);
        
        // 边界检测，让物体在窗口内反弹
        if (right > x_bound || left < -x_bound) {
            velocity[0] = -velocity[0] * restitution; // 反弹并减少能量
        }
        if (top > y_bound || bottom < -y_bound) {
            velocity[1] = -velocity[1] * restitution; // 反弹并减少能量
        }
        
        // 限制位置在边界内（由子类实现）
        constrainToBounds(x_bound, y_bound);
    }
    
    // 通用的更新方法模板
    void basicUpdate(float deltaTime, const gravitational_field& field, float aspect = 1.0f) {
        // 保留当前加速度（包括万有引力产生的加速度）
        float savedAccX = acceleration[0];
        float savedAccY = acceleration[1];
        
        // 应用重力场
        applyGravitationalField(field);
        
        // 将保留的加速度（万有引力）加回来
        acceleration[0] += savedAccX;
        acceleration[1] += savedAccY;
        
        // 更新速度
        velocity[0] += acceleration[0] * deltaTime;
        velocity[1] += acceleration[1] * deltaTime;
        
        // 更新位置
        position_x += velocity[0] * deltaTime;
        position_y += velocity[1] * deltaTime;
        
        // 重置加速度为0，为下一帧做准备
        acceleration[0] = 0.0f;
        acceleration[1] = 0.0f;
        
        // 处理边界碰撞
        handleBoundaryCollision(aspect);
    }





    // Virtual methods that must be implemented by derived classes
    virtual void update(float deltaTime, const gravitational_field& field, float aspect = 1.0f) = 0;
    virtual void draw() = 0;
    virtual bool checkCollision(const Object& other) const = 0;
    virtual void resolveCollision(Object& other) = 0;
    virtual void getBoundingBox(float& left, float& right, float& top, float& bottom) const = 0;
    virtual void constrainToBounds(float x_bound, float y_bound) = 0;

    
protected:
    float mass;
    float position_x;
    float position_y;
    float charge;
    float velocity[2];
    float acceleration[2];
};

#endif //AXIOMS_H