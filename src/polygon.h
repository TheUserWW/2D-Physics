//
// Created by wcx16 on 2026/1/11.
//

#ifndef OPENGL_POLYGON_H
#define OPENGL_POLYGON_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "axioms.h"

class polygon : public Object{
public:
    polygon(int v, float dfc, float cx = 0.0f, float cy = 0.0f) : vertexs(v), distance_from_cm(dfc) {
        setPosition(cx, cy);
        setMass(1.0f); // 默认质量
    }
    void draw() override {

        
        // 设置四边形颜色（蓝色）
        glColor3f(0.0f, 0.0f, 1.0f); // 蓝色
        
        // 多边形绘制实现
        glBegin(GL_POLYGON);
        
        // 计算多边形的顶点位置
        for (int i = 0; i < vertexs; i++) {
            float angle = 2.0f * PI * i / vertexs;
            float x = get_position_x() + distance_from_cm * cosf(angle);
            float y = get_position_y() + distance_from_cm * sinf(angle);
            glVertex2f(x, y);
        }
        
        glEnd();
        
        // 重置颜色为白色，避免影响其他物体的绘制
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    int get_num_vertex() const {
        return vertexs;
    }
    
    void update(float deltaTime, const gravitational_field& field, float aspect = 1.0f) override {
        // 使用基类的通用更新逻辑
        basicUpdate(deltaTime, field, aspect);
    }
    
    bool checkCollision(const Object& other) const override {
        // 处理多边形与多边形的碰撞 - 使用分离轴定理(SAT)
        const polygon* otherPoly = dynamic_cast<const polygon*>(&other);
        if (otherPoly) {
            return checkPolygonPolygonCollision(*otherPoly);
        }
        
        // 处理多边形与圆形的碰撞 - 使用更精确的检测
        const Circle* otherCircle = dynamic_cast<const Circle*>(&other);
        if (otherCircle) {
            return checkPolygonCircleCollision(*otherCircle);
        }
        
        return false;
    }
    
    void resolveCollision(Object& other) override {
        // 处理多边形与多边形的碰撞解决
        polygon* otherPoly = dynamic_cast<polygon*>(&other);
        if (otherPoly) {
            resolvePolygonPolygonCollision(*otherPoly);
            return;
        }
        
        // 处理多边形与圆形的碰撞解决
        Circle* otherCircle = dynamic_cast<Circle*>(&other);
        if (otherCircle) {
            resolvePolygonCircleCollision(*otherCircle);
        }
    }
    
    void getBoundingBox(float& left, float& right, float& top, float& bottom) const override {
        // 计算多边形的边界框
        left = get_position_x() - distance_from_cm;
        right = get_position_x() + distance_from_cm;
        top = get_position_y() + distance_from_cm;
        bottom = get_position_y() - distance_from_cm;
    }
    
    void constrainToBounds(float x_bound, float y_bound) override {
        // 确保多边形在边界内
        float left, right, top, bottom;
        getBoundingBox(left, right, top, bottom);
        
        if (right > x_bound) {
            setPosition(get_position_x() - (right - x_bound), get_position_y());
        }
        if (left < -x_bound) {
            setPosition(get_position_x() + (-x_bound - left), get_position_y());
        }
        if (top > y_bound) {
            setPosition(get_position_x(), get_position_y() - (top - y_bound));
        }
        if (bottom < -y_bound) {
            setPosition(get_position_x(), get_position_y() + (-y_bound - bottom));
        }
    }


private:
    int vertexs;
    float distance_from_cm;
    
    // 获取多边形的顶点坐标
    void getVertices(std::vector<std::pair<float, float>>& vertices) const {
        vertices.clear();
        for (int i = 0; i < vertexs; i++) {
            float angle = 2.0f * PI * i / vertexs;
            float x = get_position_x() + distance_from_cm * cosf(angle);
            float y = get_position_y() + distance_from_cm * sinf(angle);
            vertices.emplace_back(x, y);
        }
    }
    
    // 分离轴定理(SAT)检测多边形与多边形碰撞
    bool checkPolygonPolygonCollision(const polygon& other) const {
        std::vector<std::pair<float, float>> vertices1, vertices2;
        getVertices(vertices1);
        other.getVertices(vertices2);
        
        // 检查第一个多边形的边
        for (size_t i = 0; i < vertices1.size(); i++) {
            size_t j = (i + 1) % vertices1.size();
            float edgeX = vertices1[j].first - vertices1[i].first;
            float edgeY = vertices1[j].second - vertices1[i].second;
            
            // 计算法线（垂直于边）
            float normalX = -edgeY;
            float normalY = edgeX;
            
            // 归一化法线
            float length = sqrtf(normalX * normalX + normalY * normalY);
            if (length > 0) {
                normalX /= length;
                normalY /= length;
            }
            
            // 投影到法线上
            float min1 = std::numeric_limits<float>::max();
            float max1 = std::numeric_limits<float>::lowest();
            float min2 = std::numeric_limits<float>::max();
            float max2 = std::numeric_limits<float>::lowest();
            
            for (const auto& v : vertices1) {
                float projection = v.first * normalX + v.second * normalY;
                min1 = std::min(min1, projection);
                max1 = std::max(max1, projection);
            }
            
            for (const auto& v : vertices2) {
                float projection = v.first * normalX + v.second * normalY;
                min2 = std::min(min2, projection);
                max2 = std::max(max2, projection);
            }
            
            // 检查投影是否重叠
            if (max1 < min2 || max2 < min1) {
                return false; // 存在分离轴，没有碰撞
            }
        }
        
        return true; // 所有轴都重叠，发生碰撞
    }
    
    // 多边形与圆形碰撞检测
    bool checkPolygonCircleCollision(const Circle& circle) const {
        std::vector<std::pair<float, float>> vertices;
        getVertices(vertices);
        
        float circleX = circle.get_position_x();
        float circleY = circle.get_position_y();
        float radius = circle.getRadius();
        
        // 检查圆心到多边形各边的距离
        for (size_t i = 0; i < vertices.size(); i++) {
            size_t j = (i + 1) % vertices.size();
            float edgeX = vertices[j].first - vertices[i].first;
            float edgeY = vertices[j].second - vertices[i].second;
            
            // 计算边向量长度
            float edgeLengthSquared = edgeX * edgeX + edgeY * edgeY;
            if (edgeLengthSquared == 0) continue;
            
            // 计算圆心到边的投影
            float t = std::max(0.0f, std::min(1.0f, 
                ((circleX - vertices[i].first) * edgeX + 
                 (circleY - vertices[i].second) * edgeY) / edgeLengthSquared));
            
            // 计算投影点
            float projectionX = vertices[i].first + t * edgeX;
            float projectionY = vertices[i].second + t * edgeY;
            
            // 计算距离
            float dx = circleX - projectionX;
            float dy = circleY - projectionY;
            float distanceSquared = dx * dx + dy * dy;
            
            if (distanceSquared <= radius * radius) {
                return true;
            }
        }
        
        return false;
    }
    
    // 基于动量守恒的碰撞解决
    void resolvePolygonPolygonCollision(polygon& other) {
        // 计算碰撞法线（从当前多边形指向另一个多边形）
        float normalX = other.get_position_x() - get_position_x();
        float normalY = other.get_position_y() - get_position_y();
        
        float distance = sqrtf(normalX * normalX + normalY * normalY);
        if (distance == 0) return;
        
        normalX /= distance;
        normalY /= distance;
        
        // 计算相对速度
        float relativeVelocityX = other.get_velocity()[0] - get_velocity()[0];
        float relativeVelocityY = other.get_velocity()[1] - get_velocity()[1];
        
        // 计算速度在碰撞法线上的分量
        float velocityAlongNormal = relativeVelocityX * normalX + relativeVelocityY * normalY;
        
        // 如果物体正在分离，不处理碰撞
        if (velocityAlongNormal > 0) return;
        
        // 计算恢复系数（弹性）
        float restitution = 0.8f;
        
        // 计算冲量
        float mass1 = get_mass();
        float mass2 = other.get_mass();
        float impulseScalar = -(1 + restitution) * velocityAlongNormal / (1/mass1 + 1/mass2);
        
        // 应用冲量
        float impulseX = impulseScalar * normalX;
        float impulseY = impulseScalar * normalY;
        
        setVelocity(get_velocity()[0] - impulseX / mass1, get_velocity()[1] - impulseY / mass1);
        other.setVelocity(other.get_velocity()[0] + impulseX / mass2, other.get_velocity()[1] + impulseY / mass2);
    }
    
    // 多边形与圆形碰撞解决
    void resolvePolygonCircleCollision(Circle& circle) {
        // 计算碰撞法线（从多边形指向圆形）
        float normalX = circle.get_position_x() - get_position_x();
        float normalY = circle.get_position_y() - get_position_y();
        
        float distance = sqrtf(normalX * normalX + normalY * normalY);
        if (distance == 0) return;
        
        normalX /= distance;
        normalY /= distance;
        
        // 计算相对速度
        float relativeVelocityX = circle.get_velocity()[0] - get_velocity()[0];
        float relativeVelocityY = circle.get_velocity()[1] - get_velocity()[1];
        
        // 计算速度在碰撞法线上的分量
        float velocityAlongNormal = relativeVelocityX * normalX + relativeVelocityY * normalY;
        
        // 如果物体正在分离，不处理碰撞
        if (velocityAlongNormal > 0) return;
        
        // 计算恢复系数（弹性）
        float restitution = 0.8f;
        
        // 计算冲量
        float mass1 = get_mass();
        float mass2 = circle.get_mass();
        float impulseScalar = -(1 + restitution) * velocityAlongNormal / (1/mass1 + 1/mass2);
        
        // 应用冲量
        float impulseX = impulseScalar * normalX;
        float impulseY = impulseScalar * normalY;
        
        setVelocity(get_velocity()[0] - impulseX / mass1, get_velocity()[1] - impulseY / mass1);
        circle.setVelocity(circle.get_velocity()[0] + impulseX / mass2, circle.get_velocity()[1] + impulseY / mass2);
    }

};


#endif //OPENGL_POLYGON_H