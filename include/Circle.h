#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "axioms.h"


class Circle : public Object {
public:
    explicit Circle(float cx, float cy, float rad, int r, bool mov) : radius(rad), res(r) {
        setPosition(cx, cy);
        setMass(1.0f);
        setCharge(0.0f);
        enable_movement = mov;
    }
    void draw() override {
        glBegin(GL_TRIANGLE_FAN);

        glVertex2f(get_position_x(), get_position_y());

        for (int i = 0; i <= res; i++) {
            float angle = 2.0f * PI * i / res;
            float x = get_position_x() + radius * cosf(angle);
            float y = get_position_y() + radius * sinf(angle);
            glVertex2f(x, y);
        }

        glEnd();
    }

    void update(float deltaTime, const gravitational_field& field, float aspect = 1.0f) override {
        basicUpdate(deltaTime, field, aspect);
    }

    void update(float deltaTime, const electric_field& field, float aspect = 1.0f) {
        float angle_rad = field.direction * PI / 180.0f;
        float fx = charge * field.magnitude * cosf(angle_rad);
        float fy = charge * field.magnitude * sinf(angle_rad);
        
        applyForce(fx, fy);
    }


    bool checkCollision(const Object& other) const override {
        const Circle* otherCircle = dynamic_cast<const Circle*>(&other);
        if (!otherCircle) return false;
        
        float dx = get_position_x() - otherCircle->get_position_x();
        float dy = get_position_y() - otherCircle->get_position_y();
        float distance = sqrtf(dx * dx + dy * dy);
        return distance < (radius + otherCircle->radius);
    }
    
    void resolveCollision(Object& other) override {
        Circle* otherCircle = dynamic_cast<Circle*>(&other);
        if (!otherCircle) return;

        float dx = otherCircle->get_position_x() - get_position_x();
        float dy = otherCircle->get_position_y() - get_position_y();
        float distance = sqrtf(dx * dx + dy * dy);
        
        if (distance == 0) return;
        
        float nx = dx / distance;
        float ny = dy / distance;
        
        float overlap = (radius + otherCircle->radius) - distance;
        if (overlap > 0) {
            float separation = overlap * 0.5f;
            
            if (this->getMovementStatus() && otherCircle->getMovementStatus()) {
                setPosition(get_position_x() - separation * nx, get_position_y() - separation * ny);
                otherCircle->setPosition(otherCircle->get_position_x() + separation * nx, 
                                       otherCircle->get_position_y() + separation * ny);
            } else if (this->getMovementStatus() && !otherCircle->getMovementStatus()) {
                setPosition(get_position_x() - overlap * nx, get_position_y() - overlap * ny);
            } else if (!this->getMovementStatus() && otherCircle->getMovementStatus()) {
                otherCircle->setPosition(otherCircle->get_position_x() + overlap * nx, 
                                       otherCircle->get_position_y() + overlap * ny);
            }
        }
        
        const float* thisVel = get_velocity();
        const float* otherVel = otherCircle->get_velocity();
        float dvx = otherVel[0] - thisVel[0];
        float dvy = otherVel[1] - thisVel[1];
        
        float velocityAlongNormal = dvx * nx + dvy * ny;
        
        if (velocityAlongNormal > 0) return;
        
        float restitution = 0.8f;
        
        float j = -(1 + restitution) * velocityAlongNormal;
        
        float effectiveMass;
        if (this->getMovementStatus() && otherCircle->getMovementStatus()) {
            effectiveMass = 1.0f / (1.0f / mass + 1.0f / otherCircle->mass);
        } else if (this->getMovementStatus() && !otherCircle->getMovementStatus()) {
            effectiveMass = mass;
        } else if (!this->getMovementStatus() && otherCircle->getMovementStatus()) {
            effectiveMass = otherCircle->mass;
        } else {
            return;
        }
        
        j *= effectiveMass;
        
        float impulseX = j * nx;
        float impulseY = j * ny;
        
        if (this->getMovementStatus()) {
            setVelocity(thisVel[0] - impulseX / mass, thisVel[1] - impulseY / mass);
        }
        if (otherCircle->getMovementStatus()) {
            otherCircle->setVelocity(otherVel[0] + impulseX / otherCircle->mass,
                       otherVel[1] + impulseY / otherCircle->mass);
        }

    }
    
    float getCenterX() const { return get_position_x(); }
    float getCenterY() const { return get_position_y(); }
    float getRadius() const { return radius; }
    
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