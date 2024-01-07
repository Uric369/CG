#pragma once
#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <ctime>
#include <math.h>

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
#define MAX_LIFE 2.0f*100
	//�����������
#define MIN_LIFE 1.0f*100  
	//��ʼ�㾫���С
#define INIT_SIZE 30.0f;

	const int MAX_PARTICLES = 1800;//�������ӷ���ϵͳ����������
	//��ʼ��������������
	const int INIT_PARTICLES = 1000;
	//��������
	const glm::vec3 center(0.0f);
	const float r = 0.3f;



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
		void Render(float frametimeMills, glm::mat4 viewMatrix, glm::mat4& projectMatrix);
		void update(float frametimeMills);

		float radius;
		float gravity;

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
		unsigned int loadTexture(const char* path);

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
