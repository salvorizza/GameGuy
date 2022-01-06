#include "Graphics/Shader.h"

#include <glad/glad.h>

namespace GameGuy {
	Shader::Shader(const char* vertexSource, const char* fragmentSource) 
		:	mRendererID(0)
	{
		int32_t isCompiled = 0;
		int32_t maxLength = 0;
		int32_t isLinked = 0;

		uint32_t vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShaderID, 1, &vertexSource, NULL);
		glCompileShader(vertexShaderID);
		glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &maxLength);
			char* infoLog = new char[maxLength];
			glGetShaderInfoLog(vertexShaderID, maxLength, &maxLength, &infoLog[0]);
			printf("Error: \n%s\n\n", infoLog);
			glDeleteShader(vertexShaderID);
			delete[] infoLog;
			return;
		}

		uint32_t fragentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragentShaderID, 1, &fragmentSource, NULL);
		glCompileShader(fragentShaderID);
		glGetShaderiv(fragentShaderID, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE) {
			glGetShaderiv(fragentShaderID, GL_INFO_LOG_LENGTH, &maxLength);
			char* infoLog = new char[maxLength];
			glGetShaderInfoLog(fragentShaderID, maxLength, &maxLength, &infoLog[0]);
			printf("Error: \n%s\n\n", infoLog);
			glDeleteShader(fragentShaderID);
			delete[] infoLog;
			return;
		}
				
		uint32_t program = glCreateProgram();
		glAttachShader(program, vertexShaderID);
		glAttachShader(program, fragentShaderID);
		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE) {
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
			char* infoLog = new char[maxLength];
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
			printf("Error: \n%s\n\n", infoLog);
			glDeleteProgram(program);
			glDeleteShader(vertexShaderID);
			glDeleteShader(fragentShaderID);
			delete[] infoLog;
			return;
		}

		glDetachShader(program, vertexShaderID);
		glDetachShader(program, fragentShaderID);
		glDeleteShader(vertexShaderID);
		glDeleteShader(fragentShaderID);

		mRendererID = program;
	}

	Shader::~Shader() {
		glDeleteProgram(mRendererID);
	}

	void Shader::start(){	
		glUseProgram(mRendererID);
	}

	void Shader::stop() {
		glUseProgram(0);
	}

	void Shader::uploadUniform(const char* uniformName, const glm::mat4& mat)
	{
		int32_t location = getLocation(uniformName);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
	}

	int32_t Shader::getLocation(const char* uniformName)
	{
		auto it = mLocationsMap.find(uniformName);
		if (it == mLocationsMap.end()) {
			int32_t location = glGetUniformLocation(mRendererID, uniformName);
			mLocationsMap[uniformName] = location;
			return location;
		}
		else {
			return it->second;
		}
	}
}