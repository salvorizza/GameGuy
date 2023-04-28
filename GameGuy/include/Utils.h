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
	};

	struct HTTPResponse {
		int32_t Status;
		DataBuffer Body;

		HTTPResponse()
			:	Status(-1),
				Body()
		{}

		HTTPResponse(int32_t status, DataBuffer body)
			:	Status(status),
				Body(body)
		{}
	};

	errno_t ReadFile(const char* fileName, DataBuffer& outBuffer);
	errno_t WriteFile(const char* fileName, DataBuffer buffer);

	CURL* HTTPInit();
	char* HTTPURLEncode(CURL* curl, const char* string);
	HTTPResponse HTTPGet(CURL* curl, const char* HTTPBaseUrl);
	void HTTPClose(CURL* curl);

	void DeleteBuffer(DataBuffer& buffer);

}