#include "request.h"

/**
 * Write callback matching the CURLOPT_HEADERFUNCTION prototype.
 * @param  line Incoming header line.
 * @param  s    Discarded.
 * @param  len  Number of incoming bytes.
 * @param  ptr  Pointer to the response object.
 * @return      Returns len or 0 if the header object could not be created.
 */
static size_t cbHead(char* line, size_t s, size_t len, void* ptr) {
	// s is always 1 so discard it
	(void) s;

	Response* r = (Response*) ptr;

	// tokenise the header
	char* key = strtok(line, HEADER_DELIMS);
	char* val = strtok(NULL, HEADER_DELIMS);

	// if val is not null add the header
	if (val && !addResponseHeader(r, key, val)) {
		// memory could not be allocated/re-allocated for the header/headers
		return 0;
	}

	return len;
}

/**
 * Write callback matching the CURLOPT_WRITEFUNCTION prototype.
 * @param  data Incoming bytes.
 * @param  s    Always 1.
 * @param  len  Length of the incoming bytes.
 * @param  ptr  Pointer to the payload (where to store the incoming bytes).
 * @return      Returns len or 0 if the buffer could not be re-allocated.
 */
static size_t cbBody(char* data, size_t s, size_t len, void* ptr) {
	// s is always 1 so discard it
	(void) s;

	struct payload* p = (struct payload*) ptr;

	if (p->len + len >= p->cap) {
		// the payload must be re-sized to accomodate the larger data
		size_t cap = p->cap + len;

		p->data = realloc(p->data, cap);

		if (!p->data) {
			// re-allocation failed
			return 0;
		}

		p->cap = cap;
	}

	strcat(p->data, data);
	p->len += len;

	return len;
}

/**
 * Send a request.
 * @param  curl
 * @param  url
 * @return      A Response* containing the received data.
 */
static Response* sendRequest(CURL* curl, const char* url) {
	Response* r = createResponse();

	if (!r) {
		return NULL;
	}

	// set the URL
	curl_easy_setopt(curl, CURLOPT_URL, url);

	// set head response data handler
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, r);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &cbHead);

	// set body response data handler
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, r->body);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &cbBody);

	// send the request
	r->curlCode = curl_easy_perform(curl);
	populateResponse(curl, r);

	// reset handler settings to defaults
	curl_easy_reset(curl);

	return r;
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

/**
 * Send an HTTP GET request.
 * @param  curl
 * @param  url
 * @return      Remember to free the object once the data has been handled.
 */
static Response* httpGet(CURL* curl, const char* url) {
	return sendRequest(curl, url);
}

/**
 * Send an HTTP POST request with a JSON body.
 * @param  curl
 * @param  url
 * @param  body
 * @return      Remember to free the object once the data has been handled.
 */
static Response* httpPost(CURL* curl, const char* url) {
	return sendRequest(curl, url);
}

/**
 * Send an HTTP PUT request with a JSON body.
 * @param  curl
 * @param  url
 * @param  body
 * @return      Remember to free the object once the data has been handled.
 */
static Response* httpPut(CURL* curl, const char* url) {
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

	return sendRequest(curl, url);
}

/**
 * Send an HTTP PATCH request with a JSON body.
 * @param  curl
 * @param  url
 * @param  body
 * @return      Remember to free the object once the data has been handled.
 */
static Response* httpPatch(CURL* curl, const char* url) {
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");

	return sendRequest(curl, url);
}

/**
 * Send an HTTP DELETE request.
 * @param  curl
 * @param  url
 * @return      Remember to free the object once the data has been handled.
 */
static Response* httpDelete(CURL* curl, const char* url) {
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

	return sendRequest(curl, url);
}

/**
 * Send a JSON blob to the server.
 * @param  curl     Curl easy handler.
 * @param  url      Complete URL. Eg.: "localhost:3000/publish/abc123".
 * @param  pwHeader Complete password header. I.e. "Channel-Password: <pw>".
 * @param  json     The JSON blob.
 * @return          Response* or NULL if the headers could not be attached.
 */
Response* publishData(CURL* curl, const char* url,
					const char* pwHeader, const char* json) {
	// attach the required headers
	struct curl_slist* headers = attachHeaders(
		curl, 2, "Content-Type: application/json", pwHeader
	);

	if (!headers) {
		return NULL;
	}

	// attach body
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);

	// send the request
	Response* r = httpPost(curl, url);

	// free memory held by the header list
	curl_slist_free_all(headers);

	return r;
}

/**
 * Construct a URL to <base>/publish/<cID>.
 * Trailing slash on the base URL is handled appropriately.
 * @param  base Eg. "localhost:3000".
 * @param  cID  The channel ID.
 * @return      A heap allocated string.
 */
char* createPublishURL(const char* base, const char* cID) {
	size_t baseLen = strlen(base);
	size_t bytes = baseLen + strlen(cID) + strlen(PUB_ENDPOINT) + 1;
	bool trailingSlash = (base[baseLen - 1] == '/');

	if (!trailingSlash) {
		// add an extra byte because the base url does
		// not contain a trailing slash
		bytes += 1;
	}

	char* url = malloc(bytes);

	if (!url) {
		// out of memory
		return NULL;
	}

	// concatenate the URL
	strcpy(url, base);

	if (!trailingSlash) {
		// append a slash after the base URL and null terminator for strcat
		url[baseLen] = '/';
		url[baseLen + 1] = '\0';
	}

	strcat(url, PUB_ENDPOINT);
	strcat(url, cID);

	return url;
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
