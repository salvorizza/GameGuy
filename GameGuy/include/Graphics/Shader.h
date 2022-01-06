#pragma once

#include <cstdint>
#include <cstdio>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <map>
#include <string>

namespace GameGuy {

	class Shader {
	public:
		Shader(const char* vertexSource, const char* fragmentSource);
		~Shader();

		void start();
		void stop();

		void uploadUniform(const char* uniformName, const glm::mat4& mat);

	private:
		int32_t getLocation(const char* uniformName);
	private:
		uint32_t mRendererID;
		std::map<std::string, int32_t> mLocationsMap;
	};

}