#ifndef CURLHELPER_H
#define CURLHELPER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t (*CURL_WriteCallback)(char*, size_t, size_t, void*);

CURL* CURL_Init() {
	CURL* curl_req = curl_easy_init();
	curl_easy_setopt(curl_req, CURLOPT_TIMEOUT, 10);
	curl_easy_setopt(curl_req, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl_req, CURLOPT_FAILONERROR, 1L);

	return curl_req;
}

void CURL_Release(CURL* curl) {
	curl_easy_cleanup(curl);
}

s32 CURL_ParseIntoDataBuffer(CURL* curl_req, const char* url, void* writedata, CURL_WriteCallback callback) {
	CURLcode res;
	CURLcode url_res;
	curl_easy_setopt(curl_req, CURLOPT_URL, url);
	curl_easy_setopt(curl_req, CURLOPT_WRITEDATA, writedata);
	curl_easy_setopt(curl_req, CURLOPT_WRITEFUNCTION, callback);
	url_res = curl_easy_getinfo(curl_req, CURLINFO_RESPONSE_CODE);

	res = curl_easy_perform(curl_req);
	if (res != CURLE_OK || url_res != CURLE_OK) {
		LOG_ERROR("curl_easy_operation() failed : %s\n", curl_easy_strerror(res));
		return 0;
	}

	return 1;
}

#ifdef __cplusplus
}
#endif

#endif
