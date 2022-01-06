#include "Graphics/BatchRenderer.h"


#include <glad/glad.h>

namespace GameGuy {



	BatchRenderer::BatchRenderer()
		:	mVerticesBase(nullptr),
			mCurrentVertex(nullptr),
			mNumIndices(0)
	{
		uint32_t* indices = new uint32_t[QUAD_MAX_NUM_INDICES];

		for (uint32_t i = 0; i < MAX_QUADS; i++) {
			indices[(i * 6) + 0] = (i * 4) + 0;
			indices[(i * 6) + 1] = (i * 4) + 1;
			indices[(i * 6) + 2] = (i * 4) + 2;
			indices[(i * 6) + 3] = (i * 4) + 2;
			indices[(i * 6) + 4] = (i * 4) + 3;
			indices[(i * 6) + 5] = (i * 4) + 0;
		}

		mIBO = std::make_shared<IndexBuffer>(indices, QUAD_MAX_NUM_INDICES);
		mVBO = std::make_shared<VertexBuffer>();
		mVBO->setLayout({
			BufferElement("aPosition", ShaderType::Float2),
			BufferElement("aColor", ShaderType::Uint4, true)
		});
		mVBO->setData(NULL, QUAD_BUFFER_SIZE, VertexBufferDataUsage::Dynamic);

		mVAO = std::make_shared<VertexArray>();
		mVAO->addVertexBuffer(mVBO);
		mVAO->setIndexBuffer(mIBO);

		const char* vertexSource = "#version 450 core\n"
			"layout(location = 0) in vec2 aPos;\n"
			"layout(location = 1) in vec4 aColor;\n"
			"uniform mat4 uProjectionMatrix;"
			"out vec4 oColor;\n"
			"void main()\n"
			"{\n"
			"	gl_Position = uProjectionMatrix * vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
			"	oColor = aColor;\n"
			"}";

		const char* fragmentSource = "#version 450 core\n"
			"layout(location = 0) in vec4 oColor;\n"
			"out vec4 fragColor;\n"
			"void main()\n"
			"{\n"
			"	fragColor = oColor;\n"
			"}";

		mShader = std::make_shared<Shader>(vertexSource, fragmentSource);

		delete[] indices;
	}

	BatchRenderer::~BatchRenderer()
	{

	}

	void BatchRenderer::begin(const glm::mat4& projMatrix)
	{
		mProjectionMatrix = projMatrix;
		mVerticesBase = (QuadVertex*)mVBO->map();
		mCurrentVertex = mVerticesBase;
		mNumIndices = 0;
	}

	void BatchRenderer::end()
	{
		flush();
	}

	void BatchRenderer::flush()
	{
		mVBO->unMap();

		mShader->start();
		mShader->uploadUniform("uProjectionMatrix", mProjectionMatrix);
		mVAO->bind();
		glDrawElements(GL_TRIANGLES, mNumIndices, GL_UNSIGNED_INT, NULL);
	}

	void BatchRenderer::drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		if (mNumIndices == QUAD_MAX_NUM_INDICES) {
			flush();
			begin(mProjectionMatrix);
		}

		uint32_t a = (uint32_t)(color.a * 255);
		uint32_t b = (uint32_t)(color.b * 255);
		uint32_t g = (uint32_t)(color.g * 255);
		uint32_t r = (uint32_t)(color.r * 255);

		uint32_t u32color = a << 24 | b << 16 | g << 8 | r;


		mCurrentVertex->Position = position;
		mCurrentVertex->Color = u32color;
		mCurrentVertex++;

		mCurrentVertex->Position = position + glm::vec2(size.x, 0);
		mCurrentVertex->Color = u32color;
		mCurrentVertex++;

		mCurrentVertex->Position = position + glm::vec2(size.x, size.y);
		mCurrentVertex->Color = u32color;
		mCurrentVertex++;

		mCurrentVertex->Position = position + glm::vec2(0, size.y);
		mCurrentVertex->Color = u32color;
		mCurrentVertex++;

		mNumIndices += 6;
	}

}