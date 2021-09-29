#include "api.h"

/**
 * Create a URL from a variable number of strings. The first is assumed
 * to the base URL. The strings following are assumed to apart of the path.
 * Eg.:
 * createURL(5, "https://example.com", "/the", "answer/", "/is/", "42")
 * -> "https://example.com/the/answer/is/42"
 * @param  count The number of strings provided.
 * @return       A heap-allocated string.
 */
static char* createURL(int count, ...) {
	va_list argList;
	size_t bytes = 0, cap = INIT_URL_STR_LEN;
	char* url = malloc(cap);

	if (!url) {
		// out of memory
		return NULL;
	}

	va_start(argList, count);

	// loop through the provided strings
	for (int i = 0; i < count; i++) {
		char* str = va_arg(argList, char*);
		size_t len = strlen(str);
		bool appendLeading = (str[0] != '/' && i != 0);

		if (str[len - 1] == '/') {
			// skip the trailing slash
			len -= 1;
		}

		size_t newLen = len + bytes;

		if (appendLeading) {
			// add an extra byte to accomodate a leading slash
			newLen += 1;
		}

		if (newLen > cap) {
			// allocate another INIT_URL_STR_LEN bytes
			cap += INIT_URL_STR_LEN;

			char* ptr = realloc(url, cap);

			if (!ptr) {
				// re-allocation failed
				free(url);
				va_end(argList);

				return NULL;
			}

			url = ptr;
		}

		if (appendLeading) {
			// append a slash before copying the string
			url[bytes++] = '/';
		}

		// copy the string
		for (size_t j = 0; j < len; j++) {
			url[bytes + j] = str[j];
		}

		bytes += len;
	}

	va_end(argList);

	if (bytes == cap) {
		// add a byte for the null terminator
		char* ptr = realloc(url, cap + 1);

		if (!ptr) {
			free(url);

			return NULL;
		}
	}

	// append a null terminator
	url[bytes] = '\0';

	return url;
}

/**
 * Attach a variable number of headers to the curl handle.
 * @param  curl
 * @param  count The number of headers to attach.
 * @return       A pointer to the attached headers. Remember to free it!
 */
static struct curl_slist* attachHeaders(CURL* curl, int count, ...) {
	va_list argList;
	struct curl_slist* headers = NULL;

	va_start(argList, count);

	for (int i = 0; i < count; i++) {
		struct curl_slist* temp = curl_slist_append(headers, va_arg(argList, char*));

		if (!temp) {
			curl_slist_free_all(headers);

			return NULL;
		}

		headers = temp;
	}

	va_end(argList);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	return headers;
}

static void commonOptions(CURL* curl) {
	// prevent the request from taking too long
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, REQ_TIMEOUT);

#ifdef CURL_SKIP_VERIFY
	// skip TLS verification
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
}

/**
 * Creates the password header. I.e. "Channel-Password: <password>".
 * @param  password
 * @return          A heap allocated string.
 */
char* createPasswordHeader(const char* password) {
	// one extra for null terminator
	char* header = malloc(strlen(HEADER_CHAN_PW) + strlen(password) + 1);

	if (!header) {
		// out of memory
		return NULL;
	}

	strcpy(header, HEADER_CHAN_PW);
	strcat(header, password);

	return header;
}

/**
 * Initialise a curl handle to publish data. Sets the URL, Content-Type header,
 * Channel-Password header, and write callbacks. Returns a pointer to the attached
 * headers that must be freed after the curl handle is no longer required.
 * @param  curl Curl easy handle.
 * @param  base Base URL. Eg.: "localhost:3000". Trailing slash optional.
 * @param  cID  The channel ID to post the data to.
 * @param  pw   The password of the channel.
 * @return      Must be freed when the curl handle is no longer required.
 */
struct curl_slist* publishInit(CURL* curl, const char* base,
								const char* cID, const char* pwHeader) {
	struct curl_slist* headers = attachHeaders(
		curl, 2, "Content-Type: application/json", pwHeader
	);

	if (!headers) {
		return NULL;
	}

	char* url = createURL(3, base, PUB_ENDPOINT, cID);

	if (!url) {
		curl_easy_reset(curl);
		curl_slist_free_all(headers);

		return NULL;
	}

	// attach the url
	curl_easy_setopt(curl, CURLOPT_URL, url);

	return headers;
}

/**
 * Sends the JSON body to the already initialised and set URL.
 * @param  curl Curl easy handle. Must be initialised with publishInit.
 * @param  json The JSON string to attach as the request body.
 * @return      Must be freed once used.
 */
int publish(CURL* curl, const char* json) {
	// attach the body
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);

	// run the request
	Response* res = performRequest(curl);

	if (!res) {
		return ARE_OUT_OF_MEM;
	}

	CURLcode cc = res->curlCode;
	int status = res->status;

	// response no longer required
	freeResponse(res);

	if (cc != 0) {
		return ARE_CURL;
	}

	if (status >= 400) {
		return ARE_REQ;
	}

	return 0;
}

/**
 * Get the channels from the API and parse them as JSON.
 * @param  ptr Dereferenced and assigned the result of cJSON_Parse.
 * @return     0 on success, non-zero corresponding to errors in error.h.
 */
int getChannels(cJSON** ptr) {
	CURL* curl = curl_easy_init();

	if (!curl) {
		return ARE_OUT_OF_MEM;
	}

	// set timeout and peer verification status
	commonOptions(curl);

	// allocate a url string
	char* url = createURL(2, API_URL, CHAN_ENDPOINT);

	if (!url) {
		curl_easy_cleanup(curl);

		return ARE_OUT_OF_MEM;
	}

	// attach the url
	curl_easy_setopt(curl, CURLOPT_URL, url);
	free(url);

	// run the request
	Response* res = performRequest(curl);

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

		return ARE_CURL;
	}

	if (res->status >= 400) {
		// something went wrong with the request
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

/**
 * Verify the provided password is correct by "logging in" to a channel.
 * @param  id Channel ID.
 * @param  pw The password to test.
 * @return    Zero on success, non-zero on failure (as described in error.h).
 */
int channelLogin(char* id, char* pw) {
	CURL* curl = curl_easy_init();

	if (!curl) {
		return ARE_OUT_OF_MEM;
	}

	// attach common options
	commonOptions(curl);

	// create a url string
	char* url = createURL(4, API_URL, CHAN_ENDPOINT, id, "login");

	if (!url) {
		curl_easy_cleanup(curl);

		return ARE_OUT_OF_MEM;
	}

	// attach the url
	curl_easy_setopt(curl, CURLOPT_URL, url);
	free(url);

	// attach the password in a JSON body
	cJSON* body = cJSON_CreateObject();

	if (!body) {
		curl_easy_cleanup(curl);

		return ARE_OUT_OF_MEM;
	}

	if (!cJSON_AddStringToObject(body, "password", pw)) {
		curl_easy_cleanup(curl);

		return ARE_OUT_OF_MEM;
	}

	// attach the body to the request
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

	// set and attach json content type header
	struct curl_slist* headers = attachHeaders(
		curl, 1, "Content-Type: application/json");

	if (!headers) {
		curl_easy_cleanup(curl);

		return ARE_OUT_OF_MEM;
	}

	// run the request
	Response* res = performRequest(curl);

	// no longer need the curl handle, json body, and headers
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	cJSON_Delete(body);

	if (!res) {
		return ARE_OUT_OF_MEM;
	}

	CURLcode cc = res->curlCode;
	int status = res->status;

	freeResponse(res);

	if (cc != 0) {
		// curl handling error
		if (cc == CURLE_OPERATION_TIMEDOUT) {
			// request timed out
			return ARE_REQ_TIMEOUT;
		}

		return ARE_CURL;
	}

	if (status >= 400) {
		return ARE_REQ;
	}

	return 0;
}
