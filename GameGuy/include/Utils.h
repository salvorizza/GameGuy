#pragma once

#include <vector>

namespace GameGuy {

	errno_t ReadFile(const char* fileName, char** data, size_t* outSize);

}