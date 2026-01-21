#ifndef AXIOMS_H
#define AXIOMS_H
#include "../Dependencies/cPhysics/include/cphysics.h"
#include <cmath>

class Object {

public:
    Object() {
        double pos[3] = {0.0, 0.0, 0.0};
        double vel[3] = {0.0, 0.0, 0.0};
        double acc[3] = {0.0, 0.0, 0.0};
        entity = new_entity("object", 1.0, 0.0, pos, vel, acc, 0.8, true, false);
        enable_movement = true;
    }
    virtual ~Object() = default;

    float get_mass() const { return static_cast<float>(entity.mass); }
    const float* get_velocity() const {
        static float vel[2];
        vel[0] = static_cast<float>(entity.velocity[0]);
        vel[1] = static_cast<float>(entity.velocity[1]);
        return vel;
    }
    const float* get_acceleration() const {
        static float acc[2];
        acc[0] = static_cast<float>(entity.acceleration[0]);
        acc[1] = static_cast<float>(entity.acceleration[1]);
        return acc;
    }
    float get_position_x() const { return static_cast<float>(entity.position[0]); }
    float get_position_y() const { return static_cast<float>(entity.position[1]); }

    void applyGravitationalField(const gravitational_field& field) {
        // 使用cphysics.h中的引力场结构
        float fx = static_cast<float>(field.magnitude * field.direction[0]);
        float fy = static_cast<float>(field.magnitude * field.direction[1]);
        
        entity.acceleration[0] += fx / entity.mass;
        entity.acceleration[1] += fy / entity.mass;
    }
    
    void setMass(float m) {
        entity.mass = m;
    }
    
    void setCharge(float c) {
        entity.charge = c;
    }
    
    float get_charge() const { return static_cast<float>(entity.charge); }
    
    void setPosition(float x, float y) {
        entity.position[0] = x;
        entity.position[1] = y;
    }
    
    void setVelocity(float vx, float vy) {
        entity.velocity[0] = vx;
        entity.velocity[1] = vy;
    }
    
    void setAcceleration(float ax, float ay) {
        entity.acceleration[0] = ax;
        entity.acceleration[1] = ay;
    }
    
    void applyForce(float fx, float fy) {
        entity.acceleration[0] += fx / entity.mass;
        entity.acceleration[1] += fy / entity.mass;
    }

    void handleBoundaryCollision(float aspect = 1.0f, float restitution = 0.8f) {
        float x_bound, y_bound;
        if (aspect > 1.0f) {
            x_bound = aspect;
            y_bound = 1.0f;
        } else {
            x_bound = 1.0f;
            y_bound = 1.0f / aspect;
        }
        
        float left, right, top, bottom;
        getBoundingBox(left, right, top, bottom);
        
        if (right > x_bound || left < -x_bound) {
            entity.velocity[0] = -entity.velocity[0] * restitution;
        }
        if (top > y_bound || bottom < -y_bound) {
            entity.velocity[1] = -entity.velocity[1] * restitution;
        }
        
        constrainToBounds(x_bound, y_bound);
    }

    void applyCoulombForce(const Object& other) {
        if (entity.charge == 0.0 || other.entity.charge == 0.0) return;

        float dx = static_cast<float>(other.entity.position[0] - entity.position[0]);
        float dy = static_cast<float>(other.entity.position[1] - entity.position[1]);
        float distance_sq = dx * dx + dy * dy;

        const float MIN_DISTANCE_SQ = 0.01f;
        if (distance_sq < MIN_DISTANCE_SQ) {
            distance_sq = MIN_DISTANCE_SQ;
        }
        
        float distance = sqrtf(distance_sq);
        
        float force_magnitude = static_cast<float>(K * entity.charge * other.entity.charge / distance_sq);
        

        const float MAX_FORCE = 1000.0f;
        if (force_magnitude > MAX_FORCE) {
            force_magnitude = MAX_FORCE;
        } else if (force_magnitude < -MAX_FORCE) {
            force_magnitude = -MAX_FORCE;
        }

        float nx = dx / distance;
        float ny = dy / distance;

        if (entity.charge * other.entity.charge > 0) {
            force_magnitude = -force_magnitude;
        }

        float fx = force_magnitude * nx;
        float fy = force_magnitude * ny;
        
        applyForce(fx, fy);
    }

    void basicUpdate(float deltaTime, const gravitational_field& field, float aspect = 1.0f) {
        if (!enable_movement) {
            entity.acceleration[0] = 0.0;
            entity.acceleration[1] = 0.0;
            return;
        }
        
        double savedAccX = entity.acceleration[0];
        double savedAccY = entity.acceleration[1];
        
        applyGravitationalField(field);
        
        entity.acceleration[0] += savedAccX;
        entity.acceleration[1] += savedAccY;
        
        entity.velocity[0] += entity.acceleration[0] * deltaTime;
        entity.velocity[1] += entity.acceleration[1] * deltaTime;
        
        entity.position[0] += entity.velocity[0] * deltaTime;
        entity.position[1] += entity.velocity[1] * deltaTime;
        
        entity.acceleration[0] = 0.0;
        entity.acceleration[1] = 0.0;
        
        handleBoundaryCollision(aspect);
    }

    bool getMovementStatus() const { return enable_movement; }
    
    // 提供访问entity的方法
    Entity& getEntity() { return entity; }
    const Entity& getEntity() const { return entity; }

    virtual void update(float deltaTime, const gravitational_field& field, float aspect = 1.0f) = 0;
    virtual void draw() = 0;
    virtual bool checkCollision(const Object& other) const = 0;
    virtual void resolveCollision(Object& other) = 0;
    virtual void getBoundingBox(float& left, float& right, float& top, float& bottom) const = 0;
    virtual void constrainToBounds(float x_bound, float y_bound) = 0;

    
protected:
    Entity entity; // 使用cPhysics的Entity结构体存储属性
    bool enable_movement;
};

#endif