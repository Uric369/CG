#pragma once
/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#ifndef PARTICLE_GENERATOR_H
#define PARTICLE_GENERATOR_H
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "stb_image.h"

#include "Shader.h"


// Represents a single particle and its state
struct Particle {
    glm::vec3 Position, Velocity;
    glm::vec4 Color;
    float     Life;

    Particle() : Position(0.0f), Velocity(0.0f), Color(glm::vec4(1.0f, 0.2f, 0.1f, 1.0f)), Life(0.0f) { }
};

// Represents the state of an emitter that emits particles
struct EmitterState {
    glm::vec3 Position;
    glm::vec3 Velocity;
    float Dampening;

    EmitterState(glm::vec3 pos, glm::vec3 vel, float damp) : Position(pos), Velocity(vel), Dampening(damp) {}
};


// ParticleGenerator acts as a container for rendering a large number of 
// particles by repeatedly spawning and updating particles and killing 
// them after a given amount of time.
class ParticleGenerator
{
public:
    // constructor
    ParticleGenerator(const char* texturePath, unsigned int amount);
    // update all particles
    void Update(float dt, EmitterState& state, unsigned int newParticles, glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f));
    // render all particles
    void Draw(Shader &shader);
    void createSparks(EmitterState& state, unsigned int numberOfSparks, glm::vec3 offset, bool isAdd);
private:
    // state
    std::vector<Particle> particles;
    unsigned int amount;
    // render stat
    unsigned int texture;
    unsigned int VAO;
    // initializes buffer and vertex attributes
    void init();
    // returns the first Particle index that's currently unused e.g. Life <= 0.0f or 0 if no particle is currently inactive
    unsigned int firstUnusedParticle();
    // respawns particle
    void respawnParticle(Particle& particle, EmitterState& state, glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f));

    unsigned int loadTexture(const char* path);
   
};

#endif