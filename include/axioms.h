#ifndef AXIOMS_H
#define AXIOMS_H
#define PI 3.14159265358979323846f
#define G 6.67430e-11f
#define K 8.987551787e9f
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
    Object() : mass(1.0f), position_x(0.0f), position_y(0.0f), charge(0.0f) {
        velocity[0] = 0.0f;
        velocity[1] = 0.0f;
        acceleration[0] = 0.0f;
        acceleration[1] = 0.0f;
    }
    virtual ~Object() = default;

    float get_mass() const { return mass; }
    const float* get_velocity() const { return velocity;}
    const float* get_acceleration() const { return acceleration;}
    float get_position_x() const { return position_x; }
    float get_position_y() const { return position_y; }

    void applyGravitationalField(const gravitational_field& field) {
        float angle_rad = field.direction * PI / 180.0f;
        
        float fx = field.magnitude * cosf(angle_rad);
        float fy = field.magnitude * sinf(angle_rad);
        
        acceleration[0] += fx / mass;
        acceleration[1] += fy / mass;
    }
    
    void setMass(float m) {
        mass = m;
    }
    
    void setCharge(float c) {
        charge = c;
    }
    
    float get_charge() const { return charge; }
    
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
            velocity[0] = -velocity[0] * restitution;
        }
        if (top > y_bound || bottom < -y_bound) {
            velocity[1] = -velocity[1] * restitution;
        }
        
        constrainToBounds(x_bound, y_bound);
    }

    void applyCoulombForce(const Object& other) {
        if (charge == 0.0f || other.get_charge() == 0.0f) return;

        float dx = other.get_position_x() - position_x;
        float dy = other.get_position_y() - position_y;
        float distance_sq = dx * dx + dy * dy;

        const float MIN_DISTANCE_SQ = 0.01f;
        if (distance_sq < MIN_DISTANCE_SQ) {
            distance_sq = MIN_DISTANCE_SQ;
        }
        
        float distance = sqrtf(distance_sq);
        
        float force_magnitude = K * charge * other.get_charge() / distance_sq;
        

        const float MAX_FORCE = 1000.0f;
        if (force_magnitude > MAX_FORCE) {
            force_magnitude = MAX_FORCE;
        } else if (force_magnitude < -MAX_FORCE) {
            force_magnitude = -MAX_FORCE;
        }

        float nx = dx / distance;
        float ny = dy / distance;

        if (charge * other.get_charge() > 0) {
            force_magnitude = -force_magnitude;
        }

        float fx = force_magnitude * nx;
        float fy = force_magnitude * ny;
        
        applyForce(fx, fy);
    }

    void basicUpdate(float deltaTime, const gravitational_field& field, float aspect = 1.0f) {
        if (!enable_movement) {
            acceleration[0] = 0.0f;
            acceleration[1] = 0.0f;
            return;
        }
        
        float savedAccX = acceleration[0];
        float savedAccY = acceleration[1];
        
        applyGravitationalField(field);
        
        acceleration[0] += savedAccX;
        acceleration[1] += savedAccY;
        
        velocity[0] += acceleration[0] * deltaTime;
        velocity[1] += acceleration[1] * deltaTime;
        
        position_x += velocity[0] * deltaTime;
        position_y += velocity[1] * deltaTime;
        
        acceleration[0] = 0.0f;
        acceleration[1] = 0.0f;
        
        handleBoundaryCollision(aspect);
    }

    bool getMovementStatus() const { return enable_movement; }

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
    bool enable_movement;
};

#endif //AXIOMS_H
