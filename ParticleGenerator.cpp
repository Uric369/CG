/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "ParticleGenerator.h"

ParticleGenerator::ParticleGenerator(const char* texturePath, unsigned int amount)
    : amount(amount), texture(loadTexture(texturePath))
{
    this->init();
}


void ParticleGenerator::Update(float dt, EmitterState& state, unsigned int newParticles, glm::vec3 offset)
{
    std::cout << "dt: " << dt << std::endl;
    std::cout << "vel: " << state.Velocity.x << " " << state.Velocity.y << " " << state.Velocity.z << std::endl;
    std::cout << "pos: " << state.Position.x << " " << state.Position.y << " " << state.Position.z << std::endl;
    state.Position += state.Velocity * dt;
    // add new particles 
    for (unsigned int i = 0; i < newParticles; ++i)
    {
        int unusedParticle = this->firstUnusedParticle();
        this->respawnParticle(this->particles[unusedParticle], state, offset);
    }
    // update all particles
    for (unsigned int i = 0; i < this->amount; ++i)
    {
        Particle& p = this->particles[i];
        p.Life -= dt; // reduce life
        if (p.Life > 0.0f)
        {	// particle is alive, thus update
            std::cout << "位置变化：" << p.Velocity.x * dt << " " << p.Velocity.y * dt << " " << p.Velocity.z * dt << std::endl;
            p.Position -= p.Velocity * dt;
            p.Color.a -= dt * 2.5f;
        }
    }
}

// render all particles
void ParticleGenerator::Draw(Shader &shader)
{
    // use additive blending to give it a 'glow' effect
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    // shader.use();
    for (int i = 0; i < particles.size(); i++)
    {
        if (particles[i].Life > 0.0f)
        {
            std::cout << "第 " << i << " 个粒子" << std::endl;
            shader.setVec3("offset", particles[i].Position);
            std::cout << particles[i].Position.x << " " << particles[i].Position.y << " " << particles[i].Position.z << std::endl;

            shader.setVec3("color", particles[i].Color);
            std::cout << particles[i].Color.x << " " << particles[i].Color.y << " " << particles[i].Color.z << std::endl;

            glBindVertexArray(this->VAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    }
    // don't forget to reset to default blending mode
    // glEnable(GL_PROGRAM_POINT_SIZE);
    // glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleGenerator::init()
{
    // set up mesh and attribute properties
    unsigned int VBO;
    float particle_quad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(this->VAO);
    // fill mesh buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
    // set mesh attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // create this->amount default particle instances
    for (unsigned int i = 0; i < this->amount; ++i)
        this->particles.push_back(Particle());
}

// stores the index of the last particle used (for quick access to next dead particle)
unsigned int lastUsedParticle = 0;
unsigned int ParticleGenerator::firstUnusedParticle()
{
    // first search from last used particle, this will usually return almost instantly
    for (unsigned int i = lastUsedParticle; i < this->amount; ++i) {
        if (this->particles[i].Life <= 0.0f) {
            lastUsedParticle = i;
            std::cout << i << "过期" << std::endl;
            return i;
        }
    }
    // otherwise, do a linear search
    for (unsigned int i = 0; i < lastUsedParticle; ++i) {
        if (this->particles[i].Life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    // all particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
    lastUsedParticle = 0;
    return 0;
}

void ParticleGenerator::respawnParticle(Particle& particle, EmitterState& state, glm::vec3 offset)
{
    // srand(static_cast<unsigned int>(time(nullptr)));
    float random_x = (((rand() % 100) - 200)) / 200.0f;
    float random_y = (((rand() % 100) - 200)) / 100.0f;
    float random_z = (((rand() % 100) - 200)) / 200.0f;
    glm::vec3 random = glm::vec3(random_x, random_y, random_z);
    float rColor = 0.5f + ((rand() % 100) / 200.0f);
    particle.Position = state.Position + random;
    std::cout << "statePos: " << state.Position.x << " " << state.Position.y << " " << state.Position.z << std::endl;
    // std::cout << "random: " << random << std::endl;
    std::cout << "particle.Position: " << particle.Position.x << " " << particle.Position.y << " " << particle.Position.z << std::endl;
    particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
    particle.Life = 0.2f;
    random_x = (((rand() % 100) - 200)) / 200.0f;
    random_y = (((rand() % 100) - 200)) / 100.0f;
    random_z = (((rand() % 100) - 200)) / 200.0f;
    particle.Velocity = state.Velocity + glm::vec3(random_x, random_y, random_z) / 3.0f;
}


unsigned int ParticleGenerator::loadTexture(const char* path)
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


void ParticleGenerator::createSparks(EmitterState& state, unsigned int numberOfSparks, glm::vec3 offset, bool isAdd)
{
    for (unsigned int i = 0; i < numberOfSparks; ++i)
    {
        int unusedParticle = this->firstUnusedParticle();
        Particle& spark = this->particles[amount - numberOfSparks + i];

        // 随机方向和速度
        float spread = 3.0f;
        glm::vec3 randomDir = glm::vec3(
            (rand() % 50 - 100) / 50.0f * spread,
            (rand() % 50 - 100) / 50.0f * spread,
            (rand() % 50 - 100) / 50.0f * spread
        );
        if (isAdd) spark.Velocity = state.Velocity * 0.2f + randomDir;
        else spark.Velocity = state.Velocity * 0.2f - randomDir;

        // 较短的生命周期
        spark.Life = 0.7f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (0.3f - 0.1f)));

        // 较小的大小
        // spark.Size = glm::vec3(0.1f, 0.1f, 0.1f);

        // 明亮的颜色
        float brightness = 0.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.5f));
        spark.Color = glm::vec4(brightness, brightness, brightness, 1.0f);

        // 设置初始位置
        spark.Position = state.Position + offset;
    }
}