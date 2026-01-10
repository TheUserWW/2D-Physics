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
    virtual ~Object() = default;
    
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
    
    // Virtual methods that must be implemented by derived classes
    virtual void update(float deltaTime, const gravitational_field& field, float aspect = 1.0f) = 0;
    virtual void draw() = 0;
    virtual bool checkCollision(const Object& other) const = 0;
    virtual void resolveCollision(Object& other) = 0;
    
protected:
    float mass;
    float velocity[2];
    float acceleration[2];
};

#endif //AXIOMS_H
