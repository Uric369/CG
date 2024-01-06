#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <vector>
#include <cstdlib> // 对于rand()和srand()
#include <ctime> // 对于time()


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Room.h"
#include "ParticleGenerator.h"
#include "Light.h"
#include "Ball.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
void renderScene(const Shader& shader);
void renderCube();
void collision_detection(Ball& ball, Model& model);
bool testSphereTriangle(const glm::vec3& center, float radius, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& mesh_normal);
bool testSphereTriangle_test(const glm::vec3& center, float radius, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& mesh_normal, Ball &ball);
glm::vec3 reflectVec3(glm::vec3 A, glm::vec3 B);
void reflectVec3_modified(glm::vec3& A, glm::vec3& B, const glm::vec3& norm);
std::vector<Ball> generateRandomBalls(int numBalls);

// settings
const unsigned int SCR_WIDTH = 2000;
const unsigned int SCR_HEIGHT = 1500;
bool shadows = true;
bool shadowsKeyPressed = false;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 30.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int roomWidth = 20.0f;
int roomHeight = 14.0f;
int roomDepth = 20.0f;
const float m_ball = 1.0f;
const float m_tumbler = 5.0f;
const float e = 0.9;
const float friction = 0.1;
const float speedLimit = 10.0f;
const float ballRadius = 0.5f;
const int ballCount = 30;
const float examBorder = 2.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // build and compile shaders
    // -------------------------
    Shader shader("3.2.2.point_shadows.vs", "3.2.2.point_shadows_fs.vs");
    Shader simpleDepthShader("3.2.2.point_shadows_depth.vs", "3.2.2.point_shadows_depth_fs.vs", "3.2.2.point_shadows_depth.gs");
    Shader particleShader("particle.vs", "particle_fs.vs");
    Shader lightShader("light.vs", "light_fs.vs");
    Shader ballDepthShader("ball_depth.vs", "3.2.2.point_shadows_depth_fs.vs", "3.2.2.point_shadows_depth.gs");
    Shader ballShader("ball.vs", "3.2.2.point_shadows_fs.vs");
    std::vector<const char*> texturePaths = {
    "./texture.jpg",
    "./texture.jpg",
    "./texture.jpg",
    "./wall.png",
    "./wall.png",
    "./wall.png",
    };
    Room room(roomWidth, roomHeight, roomDepth, texturePaths);
    // lighting info
// -------------
    glm::vec3 lightPos(0.0f, roomHeight / 2.0f - 0.5f, 0.0f);

    Light light(lightPos, 2.0f);

    std::vector<Model> tumblers;
    // List of offsets for each tumbler
    std::vector<glm::vec3> offsets = {
        glm::vec3(-5.0f,  -5.0f, -5.0f),
        glm::vec3(5.0f, -5.0f, -5.0f),
        glm::vec3(-5.0f, -5.0f, 5.0f),
        glm::vec3(5.0f, -5.0f, 5.0f),
        glm::vec3(0.0f, -5.0f, 0.0f)
    };
    glm::vec3 commonScale(40.0f);
    // glm::vec3 commonScale(1.0f);
    for (const auto& offset : offsets) {
        tumblers.emplace_back("./model/tumbler.obj", false, commonScale, offset);
    }
    tumblers[0].getTexture();

    Ball ball(glm::vec3(-4.0f, 7.0f, 4.0f), glm::vec3(0.0f, -6.0f, 0.0f), 1.0f, "./ball.png");
    std::vector<Ball> balls = generateRandomBalls(ballCount);


    ParticleGenerator particleGenerator(particleShader, "./fire.jpg", 500);
    // Initialize EmitterState with start position, velocity, and dampening
    EmitterState emitterState(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 1.0f);


    // load textures
    // -------------
    unsigned int woodTexture = loadTexture("./texture.jpg");

    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth cubemap texture
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("diffuseTexture", 0);
    shader.setInt("depthMap", 1);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // move light position over time
        // lightPos.z = static_cast<float>(sin(glfwGetTime() * 0.5) * 3.0);
        // Update ParticleGenerator
       
        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        particleGenerator.Update(deltaTime, emitterState, 2, glm::vec3(0.0f));
        particleGenerator.Draw();

        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        float near_plane = 1.0f;
        float far_plane = 40.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        // 1. render scene to depth cubemap
        // --------------------------------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        simpleDepthShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            simpleDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        simpleDepthShader.setFloat("far_plane", far_plane);
        simpleDepthShader.setVec3("lightPos", lightPos);
        shader.setVec3("displacement", glm::vec3(0.0f, 0.0f, 0.0f));
        renderScene(simpleDepthShader);
        room.Draw(simpleDepthShader);
        for (auto it = tumblers.begin(); it != tumblers.end(); ++it) {
            it->updateWobbling(deltaTime);
            it->Draw(simpleDepthShader);
        }
        std::cout << "当前时间：currentTime " << currentFrame;
        ball.applyPhysics(deltaTime);
        ball.draw(simpleDepthShader);
        for (int i = 0; i < ballCount; i++) {
            balls[i].applyPhysics(deltaTime);
            balls[i].draw(simpleDepthShader);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal 
        // -------------------------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        // set lighting uniforms
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", camera.Position);
        shader.setInt("shadows", shadows); // enable/disable shadows by pressing 'SPACE'
        shader.setFloat("far_plane", far_plane);
        shader.setVec3("displacement", glm::vec3(0.0f, 0.0f, 0.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        renderScene(shader);
        room.Draw(shader);
        for (auto it = tumblers.begin(); it != tumblers.end(); ++it) {
            it->Draw(shader);
        }
        ball.draw(shader);
        for (int i = 0; i < ballCount; i++) {
            balls[i].draw(shader);
        }

        lightShader.use();
        lightShader.setVec3("aPos", lightPos);
        lightShader.setMat4("model", glm::mat4(1.0f)); // Replace with your actual model matrix
        lightShader.setMat4("view", view); // Replace with your actual view matrix
        lightShader.setMat4("projection", projection); // Replace with your actual projection matrix
        // add time component to geometry shader in the form of a uniform
        light.draw();

        for (int i = 0; i < ballCount; i++) {
            if (balls[i].getPosition().y >= examBorder) continue;
            for (auto it = tumblers.begin(); it != tumblers.end(); ++it) {
                collision_detection(balls[i], *it);
            }
        }


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// renders the 3D scene
// --------------------
void renderScene(const Shader& shader)
{
    // room cube
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(5.0f));
    shader.setMat4("model", model);
    glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
    shader.setInt("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !shadowsKeyPressed)
    {
        shadows = !shadows;
        shadowsKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        shadowsKeyPressed = false;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
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

void collision_detection(Ball& ball, Model& model) {
    // 获取子弹（球体）的半径和中心位置
    float radius = ball.getRadius();
    glm::vec3 ball_center = ball.getPosition();
    // std::cout << "当前小球位置： " << ball_center.x << " " << ball_center.y << " " << ball_center.z << std::endl;

       if (model.isSphereBoundingBoxIntersectingAABB(ball_center, radius)) {

        // 遍历模型中的每个网格(mesh)
        for (Mesh& mesh : model.meshes) {
            // 遍历每个网格的三角面片
            for (unsigned int i = 0; i < mesh.indices.size(); i += 3) {
                // 获取三角面片的顶点
                glm::vec3 v1 = mesh.vertices[mesh.indices[i]].Position;
                v1 = model.transformPoint(v1);
                glm::vec3 v2 = mesh.vertices[mesh.indices[i + 1]].Position;
                v2 = model.transformPoint(v2);
                glm::vec3 v3 = mesh.vertices[mesh.indices[i + 2]].Position;
                v3 = model.transformPoint(v3);
                glm::vec3 mesh_normal = normalize(cross((v1 - v3), (v2 - v3)));
                // glm::vec3 mesh_normal = normalize(mesh.vertices[mesh.indices[i]].Normal);
                bool isReversed = dot(ball.getVelocity(), mesh_normal) < 0;
                // std::cout << "ball_Center：" << ball_center.x << " " << ball_center.y << " " << ball_center.z << std::endl;
                // std::cout << "v1：" << v1.x << " " << v1.y << " " << v1.z << std::endl;


                if (isReversed && testSphereTriangle_test(ball_center, radius, v1, v2, v3, mesh_normal, ball)) {
                    std::cout << "collison!!!" << std::endl;
                    // ball.setActive(false);
                    glm::vec3 ballVelocity = ball.getVelocity();
                    glm::vec3 point = (v1 + v2 + v3) / 3.0f;
                    glm::vec3 meshVelocity = model.getPointVelocity(point);
                    // std::cout << "原速度：" << oldspeed.x << " " << oldspeed.y << " " << oldspeed.z << std::endl;
                    reflectVec3_modified(ballVelocity, meshVelocity, mesh_normal);
                    model.setAngularSpeed(meshVelocity, point, mesh_normal);
                    ball.setVelocity(ballVelocity);
                    ball.setTexture(model.getTexture());
                    
                    glm::vec3 pos = ball.getPosition();
                    std::cout << "当前小球位置： " << pos.x << " " << pos.y << " " << pos.z << std::endl;
                    
                    // std::cout << "法向量：" << mesh_normal.x << " " << mesh_normal.y << " " << mesh_normal.z << std::endl;
                    // std::cout << "当前速度： " << newspeed.x << " " << newspeed.y << " " << newspeed.z << std::endl;
                    return; // 发生碰撞，退出检测
                }
            }
        }
     }
}

// 检查球体和三角面片是否相交的函数
bool testSphereTriangle(const glm::vec3& center, float radius, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3 &mesh_normal) {
    // std::cout << "v1: " << v1.x << " " << v1.y << " " << v1.z << std::endl;
    // 你需要实现这个函数，检测球体和三角面片是否相交
    if (glm::distance(center, v1) > 2 * radius && glm::distance(center, v2) > 2 * radius && glm::distance(center, v3) > 2 * radius) {
        return false;
    }
    else {
        float distance = dot((center - v1), mesh_normal);
        std::cout << "mesh_normal_length: " << glm::length(mesh_normal) << std::endl;
        if (distance < radius / 2.0f) {
            glm::vec3 collision_point = center - distance * mesh_normal;
            std::cout << "碰撞点：" << collision_point.x << " " << collision_point.y << " " << collision_point.z << std::endl;
            return true;
        }
    }
    return false;
}

// 检查球体和三角面片是否相交的函数
bool testSphereTriangle_test(const glm::vec3& center, float radius, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& mesh_normal, Ball &ball) {
    // std::cout << "v1: " << v1.x << " " << v1.y << " " << v1.z << std::endl;
    // 你需要实现这个函数，检测球体和三角面片是否相交
    if (glm::distance(center, v1) > 3 * radius && glm::distance(center, v2) > 3 * radius && glm::distance(center, v3) > 3 * radius) {
        return false;
    }
    else {
        std::cout << "mesh_normal_length: " << glm::length(mesh_normal) << std::endl;
        float distance = dot((center - v1), mesh_normal);
        std::cout << "distance: " << distance << std::endl;
        if (distance < radius) {
            glm::vec3 collision_point = center - distance * mesh_normal;
            // ball.setActive(false);
            std::cout << "碰撞点：" << collision_point.x << " " << collision_point.y << " " << collision_point.z << std::endl;
            std::cout << "小球圆心：" << center.x << " " << center.y << " " << center.z << std::endl;
            return true;
        }
    }
    return false;
}

glm::vec3 reflectVec3(glm::vec3 A, glm::vec3 B) {
    glm::vec3 B_normalized = normalize(B); // 确保B是单位向量
    float dotAB = dot(A, B_normalized);
    glm::vec3 proj_AB = dotAB * B_normalized;
    glm::vec3 C = (float)2.0 * proj_AB - A;
    return C;
}


void reflectVec3_modified(glm::vec3& A, glm::vec3& B, const glm::vec3& norm) {
    // 确保norm是单位向量
    glm::vec3 norm_normalized = glm::normalize(norm);

    // 分解A和B为法线方向和切线方向的分量
    float dotAnorm = glm::dot(A, norm_normalized);
    glm::vec3 A_norm = dotAnorm * norm_normalized;
    glm::vec3 A_tang = A - A_norm;

    float dotBnorm = glm::dot(B, norm_normalized);
    glm::vec3 B_norm = dotBnorm * norm_normalized;
    glm::vec3 B_tang = B - B_norm;

    // 非完全弹性碰撞处理
    glm::vec3 A_norm_after = A_norm - (1 + e) * m_tumbler * (A_norm - B_norm) / (m_tumbler + m_ball);
    glm::vec3 B_norm_after = B_norm - (1 + e) * m_ball * (B_norm - A_norm) / (m_tumbler + m_ball);

    // 摩擦处理
    glm::vec3 C_tang = A_tang + B_tang - friction * glm::normalize(A_tang + B_tang);

    // 结合法线方向和切线方向的速度
    // A = A_norm_after + C_tang;  // 更新A为碰撞后的速度
    // B = B_norm_after + B_tang;  // 更新B为碰撞后的速度，假设B仍然具有原始的切线方向速度
    A = A_norm_after + A_tang;  // 更新A为碰撞后的速度
    B = B_norm_after + B_tang;  // 更新A为碰撞后的速度
}


std::vector<Ball> generateRandomBalls(int numBalls) {
    std::vector<Ball> balls;
    srand(static_cast<unsigned int>(time(nullptr))); // 初始化随机数生成器

    for (int i = 0; i < numBalls; ++i) {
        // 随机生成位置和速度
        glm::vec3 position(
            rand() / (float)RAND_MAX * roomWidth - roomWidth / 2.0f,
            rand() / (float)RAND_MAX * roomHeight / 2.0f,
            rand() / (float)RAND_MAX * roomDepth - roomDepth / 2.0f
        );

        glm::vec3 velocity(
            rand() / (float)RAND_MAX * 2 * speedLimit - speedLimit,
            rand() / (float)RAND_MAX * 2 * speedLimit - speedLimit,
            rand() / (float)RAND_MAX * 2 * speedLimit - speedLimit
        );

        balls.emplace_back(position, velocity, ballRadius, "./ball.png");
    }

    return balls;
}

