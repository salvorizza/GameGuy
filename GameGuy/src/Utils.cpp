#include "Utils.h"

namespace GameGuy {

	errno_t ReadFile(const char* fileName, DataBuffer& outBuffer)
	{
		FILE* f = NULL;
		errno_t err;

		err = fopen_s(&f, fileName, "rb");
		if (err == 0) {
			fseek(f, 0, SEEK_END);
			outBuffer.Size = ftell(f);
			fseek(f, 0, SEEK_SET);

			outBuffer.Data = (uint8_t*)malloc(sizeof(uint8_t) * outBuffer.Size);
			if (outBuffer.Data) {
				fread_s(outBuffer.Data, outBuffer.Size, outBuffer.Size, 1, f);
			}

			fclose(f);
		}

		return err;
	}

	errno_t WriteFile(const char* fileName, DataBuffer buffer)
	{
		FILE* f = NULL;
		errno_t err;

		err = fopen_s(&f, fileName, "wb");
		if (err == 0) {
			fwrite(buffer.Data, buffer.Size, 1, f);
			fclose(f);
		}

		return err;
	}

	CURL* HTTPInit()
	{
		return curl_easy_init();
	}

	char* HTTPURLEncode(CURL* curl, const char* string)
	{
		char* encodedURLResource = curl_easy_escape(curl, string, strlen(string));
		return encodedURLResource;
	}

	void HTTPClose(CURL* curl)
	{
		curl_easy_cleanup(curl);
	}

	void DeleteBuffer(DataBuffer& buffer)
	{
		free(buffer.Data);
		buffer.Size = 0;
		buffer.Data = NULL;
	}

	static size_t writefunc(void* ptr, size_t size, size_t nmemb, DataBuffer* v)
	{
		size_t newLength = size * nmemb;

		size_t oldSize = v->Size;
		size_t newSize = oldSize + newLength;

		uint8_t* newData = (uint8_t*)malloc(newSize);

		if (oldSize != 0) {
			memset(newData, newSize, 0);
			memcpy_s(newData, newSize, v->Data, v->Size);
			free(v->Data);
		}
		v->Data = newData;
		v->Size = newSize;

		if(v->Data)
			memcpy_s(&v->Data[oldSize], newLength, ptr, newLength);

		return newLength;
	}

	HTTPResponse HTTPGet(CURL* curl, const char* HTTPUrl)
	{
		HTTPResponse response;

		long http_code = 0;

		curl_easy_setopt(curl, CURLOPT_URL, HTTPUrl);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.Body);
		CURLcode code = curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

		if (code != CURLE_OK) {
			return response;
		}

		response.Status = http_code;

		return response;
	}
}