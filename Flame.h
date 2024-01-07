#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <iostream>
#include <ctime>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

namespace Flame {
#define PARTICLE_TYPE_LAUNCHER 0.0f
#define PARTICLE_TYPE_SHELL 1.0f
	//����ٶ�
//#define MAX_VELOC glm::vec3(0.0,5.0,0.0)
//	//��С�ٶ�
//#define MIN_VELOC glm::vec3(0.0,3.0,0.0)
	//�����С�ٶȲ��
//#define DEL_VELOC glm::vec3(0.0,2.0,0.0)
	//���������
#define MAX_LIFE 2.0f*50
	//�����������
#define MIN_LIFE 1.0f*50  
	//��ʼ�㾫���С
#define INIT_SIZE 10.0f;

	const int MAX_PARTICLES = 800;//�������ӷ���ϵͳ����������
	//��ʼ��������������
	const int INIT_PARTICLES = 200;
	//��������
	const glm::vec3 center(0.0f);
	const float r = 4.0f;



	struct FlameParticle
	{
		float type;
		glm::vec3 position;
		glm::vec3 velocity;
		float lifetimeMills;//����
		float alpha;//alphaͨ��
		float size;//���ӵ㾫���С
		float life;//����
	};

	class Flame
	{
	public:
		Flame();
		~Flame();
		void Render(float frametimeMills, glm::mat4 viewMatrix, glm::mat4 projectMatrix);
		void update(float frametimeMills);

		float radius;

		glm::vec3 position;
		glm::vec3 velocity;
		glm::mat4 modelMatrix;

		glm::vec3 MAX_VELOC;
		glm::vec3 MIN_VELOC;
		glm::vec3 DEL_VELOC;

	private:
		bool InitFlame(glm::vec3& pos);
		void UpdateParticles(float frametimeMills);//�������ӵ�λ�õ�
		void InitRandomTexture(unsigned int size);//����1ά�������
		void RenderParticles(glm::mat4& worldMatrix, glm::mat4& viewMatrix, glm::mat4& projectMatrix);
		void GenInitLocation(FlameParticle partciles[], int nums);//���ɳ�ʼ����
		void updateMaxMinVelocity();
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

		unsigned int mCurVBOIndex, mCurTransformFeedbackIndex;
		GLuint mParticleBuffers[2]; //���ӷ���ϵͳ���������㻺����
		GLuint mParticleArrays[2];
		GLuint mTransformFeedbacks[2];//���ӷ���ϵͳ��Ӧ��TransformFeedback
		GLuint mRandomTexture;//���һά����
		unsigned int mSparkTexture;//Alpha����
		unsigned int mStartTexture;
		float mTimer;//���ӷ������Ѿ������ʱ��
		bool mFirst;
		Shader* mUpdateShader;//�������ӵ�GPUProgram
		Shader* mRenderShader;//��Ⱦ���ӵ�GPUProgram
	};

}
