#pragma once
#ifndef BALL_H
#define BALL_H
#define PI 3.1415

#include <glm/glm.hpp>
#include <memory>
#include <glad/glad.h>
#include <vector>
#include "stb_image.h"
#include <iostream>

class Ball {
public:
    glm::vec3 position;
    glm::vec3 ini_position;
    glm::vec3 velocity;
    float radius;
    bool active;
    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;
    unsigned int texture; 
    const int Y_SEGMENTS = 50;
    const int X_SEGMENTS = 50;
    const glm::vec3 gravity = glm::vec3(0.0f, -0.981f, 0.0f); // Earth's gravity in the y direction
    const float airResistanceCoefficient = 0.047f; // Simplified air resistance coefficient


    // Constructor to initialize bullet parameters
    Ball(glm::vec3 pos, glm::vec3 vel, float rad, const char* texturePath)
        : position(pos), ini_position(pos), velocity(vel), radius(rad), active(true) {
        texture = loadTexture(texturePath);
        // Generate and bind VAO and VBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO); // Uncomment this if using EBO

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // Reserve enough space for vertices
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * Y_SEGMENTS * X_SEGMENTS, nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // Uncomment this if using EBO
        // Reserve space for indices
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Y_SEGMENTS * X_SEGMENTS * 6, nullptr, GL_DYNAMIC_DRAW); // Adjust index count as needed

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }

    // Update Ball's position and velocity according to gravity, air resistance and delta time
    void applyPhysics(float deltaTime) {
        // std::cout << "小球还能不能动啦： " << active << std::endl;
        // std::cout << "小球velocity：" << velocity.x << std::endl;
        if (active) {
            // std::cout << "ball position" << position.x << " " << position.y << " " << position.z << std::endl;
            // Update velocity with gravity effect
            velocity += gravity * deltaTime;

            // Simulate air resistance
            float speed = glm::length(velocity);
            if (speed > 0.0f) {
                glm::vec3 dragForce = -airResistanceCoefficient * speed * velocity;
                glm::vec3 dragAcceleration = dragForce; // Assuming unit mass
                velocity += dragAcceleration * deltaTime;
            }

            // Update position with the new velocity
            position += velocity * deltaTime;
        }
    }


    void draw(Shader &shader) {
        glm::vec3 ds = getDisplacement();
        // std::cout << "displacement" << ds.x << " " << ds.y << " " << ds.z;
        // shader.setVec3("displacement", glm::vec3(0.0f, -14.0f, 0.0f));
        shader.setMat4("model", glm::mat4(1.0f));
        shader.setInt("reverse_normals", 0);

        // 更新顶点数据
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        for (int lat = 0; lat <= Y_SEGMENTS; lat++) {
            float theta = lat * PI / Y_SEGMENTS;
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            for (int lon = 0; lon <= X_SEGMENTS; lon++) {
                float phi = lon * 2 * PI / X_SEGMENTS;
                float sinPhi = sin(phi);
                float cosPhi = cos(phi);

                float x = cosPhi * sinTheta;
                float y = cosTheta;
                float z = sinPhi * sinTheta;

                vertices.push_back(position.x + radius * x);
                vertices.push_back(position.y + radius * y);
                vertices.push_back(position.z + radius * z);
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                vertices.push_back(1.0f * lon / X_SEGMENTS);
                vertices.push_back(1.0f * lat / Y_SEGMENTS);
            }
        }

        for (int lat = 0; lat < Y_SEGMENTS; lat++) {
            for (int lon = 0; lon < X_SEGMENTS; lon++) {
                int first = lat * (X_SEGMENTS + 1) + lon;
                int second = first + 1;
                int third = (lat + 1) * (X_SEGMENTS + 1) + lon;
                int fourth = third + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(third);

                indices.push_back(second);
                indices.push_back(fourth);
                indices.push_back(third);
            }
        }

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // Uncomment this if using EBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);

        // Setup texture and draw
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    }

    unsigned int loadTexture(const char* path)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }


    // Deactivate the ball (for instance, when it falls out of bounds)
    void deactivate() {
        active = false;
        velocity = glm::vec3(0.0f);
    }

    glm::vec3 getVelocity() {
        return velocity;
    }

    float getRadius() {
        return radius;
    }

    glm::vec3 getPosition() {
        return position;
    }

    void setVelocity(glm::vec3 vel) {
        velocity = vel;
    }

    glm::vec3 getDisplacement() {
        return position - ini_position;
    }

    void setActive(bool act) {
        active = act;
    }

    void setTexture(unsigned int texture) {
        this->texture = texture;
    }
};

#endif // BULLET_H
