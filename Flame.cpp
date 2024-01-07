#include "Flame.h"

namespace Flame {

	Flame::Flame()
	{
		//glGetError();
		mCurVBOIndex = 0;
		mCurTransformFeedbackIndex = 1;
		mFirst = true;
		mTimer = 0;
		const GLchar* varyings[7] = { "Type1","Position1",
			"Velocity1","Age1",
			"Alpha1","Size1",
			"Life1"
		};
		mUpdateShader = new Shader("./flame_update.vs", "./flame_update_fs.vs",
			"./flame_update_gs.vs", varyings, 7);
		
		mRenderShader = new Shader("./flame_render.vs", "./flame_render_fs.vs");
		
		InitRandomTexture(580);
		mSparkTexture = loadTexture("./texture/particle.bmp");
		mStartTexture = loadTexture("./texture/flame.bmp");
		mRenderShader->use();
		mRenderShader->setInt("flameSpark", 0);
		mRenderShader->setInt("flameStart", 1);
		glm::vec3 pos(0.0, 0.0, -3.0f);
		InitFlame(pos);
		this->position = glm::vec3(0.0f, 0.0f, 0.0f);
		this->modelMatrix = glm::mat4(1.0f);
		this->velocity = glm::vec3(0.5f, 0.5f, 0.0f);
		this->radius = 10.0f;
		updateMaxMinVelocity();
	}


	Flame::~Flame()
	{
	}

	bool Flame::InitFlame(glm::vec3& pos)
	{
		FlameParticle particles[MAX_PARTICLES];
		memset(particles, 0, sizeof(particles));
		particles[0].type = PARTICLE_TYPE_LAUNCHER;
		particles[0].position = pos;
		particles[0].lifetimeMills = 0.0f;
		particles[0].velocity = glm::vec3(0.0f, 0.1f, 0.0f);
		GenInitLocation(particles, INIT_PARTICLES);
		glGenTransformFeedbacks(2, mTransformFeedbacks);
		glGenBuffers(2, mParticleBuffers);
		glGenVertexArrays(2, mParticleArrays);
		for (int i = 0; i < 2; i++)
		{
			glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedbacks[i]);
			glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffers[i]);
			glBindVertexArray(mParticleArrays[i]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(particles), particles, GL_DYNAMIC_DRAW);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mParticleBuffers[i]);
		}
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
		glBindVertexArray(0);
		
		mUpdateShader->use();
		glBindTexture(GL_TEXTURE_1D, mRandomTexture);
		mUpdateShader->setInt("gRandomTexture", 0);
		return true;
	}

	void Flame::Render(float frametimeMills,
		glm::mat4 viewMatrix, glm::mat4 projectMatrix)
	{
		mTimer += frametimeMills * 1000.0f;
		UpdateParticles(frametimeMills * 1000.0f);
		RenderParticles(this->modelMatrix, viewMatrix, projectMatrix);
		mCurVBOIndex = mCurTransformFeedbackIndex;
		mCurTransformFeedbackIndex = (mCurTransformFeedbackIndex + 1) & 0x1;
	}

	void Flame::update(float frametimeMills)
	{
		updateMaxMinVelocity();
		this->position += this->velocity * frametimeMills;
		glm::mat4 E = glm::mat4(1.0f);
		this->modelMatrix = glm::translate(E, position);
	}

	void Flame::UpdateParticles(float frametimeMills)
	{
		mUpdateShader->use();
		mUpdateShader->setFloat("gDeltaTimeMillis", frametimeMills);
		//mUpdateShader->setFloat("gTime", mTimer);
		mUpdateShader->setFloat("MAX_LIFE", MAX_LIFE);
		mUpdateShader->setFloat("MIN_LIFE", MIN_LIFE);
		mUpdateShader->setVec3("MAX_VELOC", MAX_VELOC);
		mUpdateShader->setVec3("MIN_VELOC", MIN_VELOC);

		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_1D, mRandomTexture);
		//mUpdateShader->setInt("gRandomTexture",0);

		glEnable(GL_RASTERIZER_DISCARD);
		glBindVertexArray(mParticleArrays[mCurVBOIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffers[mCurVBOIndex]);
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedbacks[mCurTransformFeedbackIndex]);

		glEnableVertexAttribArray(3);//type
		glEnableVertexAttribArray(4);//position
		glEnableVertexAttribArray(5);//velocity
		glEnableVertexAttribArray(6);//lifetime
		glEnableVertexAttribArray(7);//alpha
		glEnableVertexAttribArray(8);//size
		glEnableVertexAttribArray(9);//life
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(FlameParticle), (void*)offsetof(FlameParticle, type));
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(FlameParticle), (void*)offsetof(FlameParticle, position));
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(FlameParticle), (void*)offsetof(FlameParticle, velocity));
		glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(FlameParticle), (void*)offsetof(FlameParticle, lifetimeMills));
		glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(FlameParticle), (void*)offsetof(FlameParticle, alpha));
		glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(FlameParticle), (void*)offsetof(FlameParticle, size));
		glVertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE, sizeof(FlameParticle), (void*)offsetof(FlameParticle, life));
		glBeginTransformFeedback(GL_POINTS);
		if (mFirst)
		{
			glDrawArrays(GL_POINTS, 0, INIT_PARTICLES);
			mFirst = false;
		}
		else {
			glDrawTransformFeedback(GL_POINTS, mTransformFeedbacks[mCurVBOIndex]);
		}
		glEndTransformFeedback();
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
		glDisableVertexAttribArray(5);
		glDisableVertexAttribArray(6);
		glDisableVertexAttribArray(7);
		glDisableVertexAttribArray(8);
		glDisableVertexAttribArray(9);
		glDisable(GL_RASTERIZER_DISCARD);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void Flame::RenderParticles(glm::mat4& worldMatrix,
		glm::mat4& viewMatrix, glm::mat4& projectMatrix)
	{
		glEnable(GL_PROGRAM_POINT_SIZE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		mRenderShader->use();
		mRenderShader->setMat4("model", worldMatrix);
		mRenderShader->setMat4("view", viewMatrix);
		mRenderShader->setMat4("projection", projectMatrix);
		glBindVertexArray(mParticleArrays[mCurTransformFeedbackIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffers[mCurTransformFeedbackIndex]);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FlameParticle), (void*)offsetof(FlameParticle, position));
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(FlameParticle), (void*)offsetof(FlameParticle, alpha));
		glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(FlameParticle), (void*)offsetof(FlameParticle, size));
		glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(FlameParticle), (void*)offsetof(FlameParticle, lifetimeMills));
		glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(FlameParticle), (void*)offsetof(FlameParticle, life));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mSparkTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mStartTexture);
		glDrawTransformFeedback(GL_POINTS, mTransformFeedbacks[mCurTransformFeedbackIndex]);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
		glDisableVertexAttribArray(5);
		glDisableVertexAttribArray(6);
		glDisableVertexAttribArray(7);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDisable(GL_BLEND);
	}

	void Flame::InitRandomTexture(unsigned int size)
	{
		srand(time(NULL));
		glm::vec3* pRandomData = new glm::vec3[size];
		for (int i = 0; i < size; i++)
		{
			pRandomData[i].x = float(rand()) / float(RAND_MAX);
			pRandomData[i].y = float(rand()) / float(RAND_MAX);
			pRandomData[i].z = float(rand()) / float(RAND_MAX);
		}
		glGenTextures(1, &mRandomTexture);
		glBindTexture(GL_TEXTURE_1D, mRandomTexture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, size, 0, GL_RGB, GL_FLOAT, pRandomData);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		delete[] pRandomData;
		pRandomData = nullptr;
	}

	void Flame::GenInitLocation(FlameParticle particles[], int nums)
	{
		srand(time(NULL));
		int n = 10;
		float Adj_value = 0.05f;
		float radius_fire = 10.0f;
		for (int x = 0; x < nums; x++) {
			glm::vec3 record(0.0f);
			for (int y = 0; y < n; y++) {
				record.x += (2.0f * float(rand()) / float(RAND_MAX) - 1.0f);
				record.z += (2.0f * float(rand()) / float(RAND_MAX) - 1.0f);
			}
			record.x *= radius_fire;
			record.z *= radius_fire;
			record.y = center.y;
			particles[x].type = PARTICLE_TYPE_LAUNCHER;
			particles[x].position = record;
			particles[x].velocity = DEL_VELOC * (float(rand()) / float(RAND_MAX)) + MIN_VELOC;
			particles[x].alpha = 1.0f;
			particles[x].size = INIT_SIZE;
			
			particles[x].lifetimeMills = (MAX_LIFE - MIN_LIFE) * (float(rand()) / float(RAND_MAX)) + MIN_LIFE;
			float dist = sqrt(record.x * record.x + record.z * record.z);
			particles[x].life = particles[x].lifetimeMills;
		}
	}

	void Flame::updateMaxMinVelocity()
	{
		float max_coeff = 2.0f;
		float min_coeff = 0.5f;
		if (this->velocity.x >= 0) {
			MAX_VELOC.x = velocity.x * max_coeff;
			MIN_VELOC.x = velocity.x * min_coeff;
		}
		else {
			MAX_VELOC.x = velocity.x * min_coeff;
			MIN_VELOC.x = velocity.x * max_coeff;
		}

		if (this->velocity.y >= 0) {
			MAX_VELOC.y = velocity.y * max_coeff;
			MIN_VELOC.y = velocity.y * min_coeff;
		}
		else {
			MAX_VELOC.y = velocity.y * min_coeff;
			MIN_VELOC.y = velocity.y * max_coeff;
		}

		if (this->velocity.z >= 0) {
			MAX_VELOC.z = velocity.z * max_coeff;
			MIN_VELOC.z = velocity.z * min_coeff;
		}
		else {
			MAX_VELOC.z = velocity.z * min_coeff;
			MIN_VELOC.z = velocity.z * max_coeff;
		}

		MAX_VELOC = -MAX_VELOC;
		MIN_VELOC = -MIN_VELOC;

		DEL_VELOC = MAX_VELOC - MIN_VELOC;
	}

}