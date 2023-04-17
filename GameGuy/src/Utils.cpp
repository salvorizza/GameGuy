#include "Utils.h"

namespace GameGuy {

	errno_t ReadFile(const char* fileName, char** data, size_t* outSize)
	{
		FILE* f = NULL;
		errno_t err;

		err = fopen_s(&f, fileName, "rb");
		if (err == 0) {
			fseek(f, 0, SEEK_END);
			*outSize = ftell(f);
			fseek(f, 0, SEEK_SET);

			*data = (char*)malloc(sizeof(char) * (*outSize + 1));
			if (*data) {
				memset(*data, 0, *outSize + 1);
				fread_s(*data, *outSize, *outSize, 1, f);
			}

			fclose(f);
		}

		return err;
	}
}