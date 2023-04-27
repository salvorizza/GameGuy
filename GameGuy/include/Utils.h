#pragma once

#include <stdint.h>
#include <string.h>

#include <curl/curl.h>

namespace GameGuy {

	struct DataBuffer {
		uint8_t* Data;
		size_t Size;

		DataBuffer()
			:	Data(NULL),
				Size(0)
		{}

		DataBuffer(uint8_t* data, size_t size)
			:	Data(data),
				Size(size)
		{}

		~DataBuffer() {
			int i = 0;
		}
	};

	inline CURL* gCurl = NULL;

	errno_t ReadFile(const char* fileName, DataBuffer& outBuffer);
	errno_t WriteFile(const char* fileName, DataBuffer buffer);

	void HTTPInit();
	char* HTTPURLEncode(const char* string);
	int32_t HTTPGet(const char* HTTPBaseUrl, DataBuffer& outBuffer);
	void HTTPClose();

	void DeleteBuffer(DataBuffer& buffer);

}