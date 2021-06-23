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
 * Construct a URL to <base>/publish/<cID>.
 * Trailing slash on the base URL is handled appropriately.
 * @param  base Eg. "localhost:3000".
 * @param  cID  The channel ID.
 * @return      A heap allocated string.
 */
static char* createPublishURL(const char* base, const char* cID) {
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

	char* url = createPublishURL(base, cID);

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
Response* publish(CURL* curl, const char* json) {
	Response* r = createResponse();

	if (!r) {
		return NULL;
	}

	// attach body
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);

	// set head response data handler
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, r);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &cbHead);

	// set body response data handler
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, r->body);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &cbBody);

	// send the request
	r->curlCode = curl_easy_perform(curl);
	populateResponse(curl, r);

	return r;
}
