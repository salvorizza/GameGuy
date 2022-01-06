#pragma once

#include "Graphics/Shader.h"
#include "Graphics/VertexArray.h"

#include <glm/glm.hpp>

namespace GameGuy {

	


	class BatchRenderer {
	public:
		struct QuadVertex {
			glm::vec2 Position;
			uint32_t Color;
		};

	public:
		BatchRenderer();
		~BatchRenderer();

		void begin(const glm::mat4& projMatrix);
		void end();
		void flush();

		void drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
	public:
		static const size_t QUAD_VERTEX_SIZE = sizeof(QuadVertex);
		static const size_t QUAD_SIZE = QUAD_VERTEX_SIZE * 4;
		static const size_t MAX_QUADS = 400;
		static const size_t QUAD_BUFFER_SIZE = QUAD_SIZE * MAX_QUADS;
		static const size_t QUAD_MAX_NUM_INDICES = MAX_QUADS * 6;
	private:
		std::shared_ptr<VertexArray> mVAO;
		std::shared_ptr<VertexBuffer> mVBO;
		std::shared_ptr<IndexBuffer> mIBO;
		std::shared_ptr<Shader> mShader;

		QuadVertex* mVerticesBase;
		QuadVertex* mCurrentVertex;
		uint32_t mNumIndices;
		glm::mat4 mProjectionMatrix;
	};

}