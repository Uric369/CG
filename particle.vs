#version 330 core
layout (location = 0) in vec4 vertex; 
out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 projection;
uniform vec3 offset;
uniform vec4 color;
uniform mat4 model;
uniform mat4 view;

void main()
{
    float scale = 0.1f;
    TexCoords = vertex.zw;
    ParticleColor = color;
    gl_Position = projection * view * model * vec4((vertex.xyz * scale) + offset, 1.0);
}