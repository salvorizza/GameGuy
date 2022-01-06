#include "Graphics/FrameBuffer.h"

#include <glad/glad.h>

namespace GameGuy {



	FrameBuffer::FrameBuffer(int32_t width, int32_t height)
		:	mRendererID(0),
			mColorAttachment(0),
			mWidth(width),
			mHeight(height)
	{
		invalidate();
	}

	FrameBuffer::~FrameBuffer()
	{
		glDeleteTextures(1, &mColorAttachment);
		glDeleteFramebuffers(1, &mRendererID);
	}

	void FrameBuffer::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);
	}

	void FrameBuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::resize(int32_t width, int32_t height)
	{
		mWidth = width;
		mHeight = height;

		invalidate();
	}

	void FrameBuffer::invalidate()
	{
		if (mRendererID != 0) {
			glDeleteTextures(1, &mColorAttachment);
			glDeleteFramebuffers(1, &mRendererID);
			mRendererID = mColorAttachment = 0;
		}

		glGenTextures(1, &mColorAttachment);
		glBindTexture(GL_TEXTURE_2D, mColorAttachment);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenFramebuffers(1, &mRendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, mRendererID);

		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,mColorAttachment,0);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			int i = 0;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}