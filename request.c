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
 * Attaches write callbacks and sends the request specified in the curl handle.
 */
Response* performRequest(CURL* curl) {
	Response* r = createResponse();

	if (!r) {
		return NULL;
	}

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
