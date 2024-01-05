#pragma once
#ifndef lIGHT_H
#define lIGHT_H
#define PI 3.1415

#include <glm/glm.hpp>
#include <memory>
#include <glad/glad.h>
#include <vector>


class Light {
public:
    glm::vec3 position;
    float radius;
    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;
    const int Y_SEGMENTS = 50;
    const int X_SEGMENTS = 50;

    // Constructor to initialize bullet parameters
    Light(glm::vec3 pos, float rad)
        : position(pos), radius(rad) {
        // 生成球的顶点数据
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 360, nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        // 顶点位置属性
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }


    void draw() {
        // Update vertex data for the lower hemisphere
        std::vector<glm::vec3> vertices;
        std::vector<unsigned int> indices;

        for (int lat = Y_SEGMENTS / 2; lat <= Y_SEGMENTS; lat++) { // Start from the equator and go to the bottom
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

                vertices.push_back(glm::vec3(position.x + radius * x, position.y + radius * y, position.z + radius * z));
            }
        }

        for (int lat = Y_SEGMENTS / 2; lat < Y_SEGMENTS; lat++) { // Only loop over the lower half
            for (int lon = 0; lon < X_SEGMENTS; lon++) {
                int first = (lat - Y_SEGMENTS / 2) * (X_SEGMENTS + 1) + lon; // Adjust index for the lower half
                int second = first + X_SEGMENTS + 1;
                int third = first + 1;
                int fourth = second + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(third);

                indices.push_back(second);
                indices.push_back(fourth);
                indices.push_back(third);
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);

        // Draw the lower hemisphere as solid and set the color to white
        glBindVertexArray(VAO);
        glEnableVertexAttribArray(0);

        // Draw the hemisphere
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // Clean up
        glBindVertexArray(0);
    }


    float getRadius() {
        return radius;
    }

    glm::vec3 getPosition() {
        return position;
    }
};

#endif // BULLET_H
