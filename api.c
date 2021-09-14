#include "api.h"

/**
 * Get the channels from the API and parse them as JSON.
 * @param  ptr Dereferenced and assigned the result of cJSON_Parse.
 * @return     0 on success, non-zero corresponding to errors in error.h.
 */
int getChannelsJSON(cJSON** ptr) {
	CURL* curl = curl_easy_init();

	if (!curl) {
		return ARE_OUT_OF_MEM;
	}

	// prevent the request from taking too long
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, REQ_TIMEOUT);

#ifdef CURL_SKIP_VERIFY
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

	Response* res = getChannels(curl, API_URL);

	// no longer require the curl handle
	curl_easy_cleanup(curl);

	if (!res) {
		return ARE_OUT_OF_MEM;
	}

	CURLcode cc = res->curlCode;

	if (cc != CURLE_OK) {
		freeResponse(res);

		if (cc == CURLE_OPERATION_TIMEDOUT) {
			// request took longer than REQ_TIMEOUT
			return ARE_REQ_TIMEOUT;
		}

		// TODO: error messages

		return ARE_CURL;
	}

	if (res->status >= 400) {
		// something went wrong with the request
		// TODO: error messages
		freeResponse(res);

		return ARE_REQ;
	}

	cJSON* array = cJSON_Parse(res->body->data);

	// no longer require the response object
	freeResponse(res);

	if (!array) {
		return ARE_OUT_OF_MEM;
	}

	*ptr = array;

	return 0;
}
