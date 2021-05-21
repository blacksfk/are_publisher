#include "response.h"

/**
 * Allocate memory for a payload object.
 */
static struct payload* createPayload() {
	struct payload* p = malloc(sizeof(*p));

	if (!p) {
		return NULL;
	}

	p->data = malloc(BUF_SIZE);

	if (!p->data) {
		free(p);

		return NULL;
	}

	// length accounts for the null terminator
	p->len = 1;
	p->cap = BUF_SIZE;
	p->data[0] = '\0';

	return p;
}

/**
 * De-allocate the memory used by a payload.
 * @param p
 */
static void freePayload(struct payload* p) {
	free(p->data);
	free(p);
}

/**
 * Allocate memory for a response object.
 */
Response* createResponse() {
	Response* r = malloc(sizeof(*r));

	if (!r) {
		return NULL;
	}

	r->headers = calloc(HEADER_COUNT, sizeof(Header*));

	if (!r->headers) {
		free(r);

		return NULL;
	}

	r->body = createPayload();

	if (!r->body) {
		free(r);
		free(r->headers);

		return NULL;
	}

	r->status = 0;
	r->headerCount = 0;
	r->headerCap = HEADER_COUNT;
	r->curlCode = CURLE_OK;

	return r;
}

/**
 * Allocate memory for a header object.
 * @param  k Header name (key).
 * @param  v Header value.
 */
static Header* createHeader(const char* k, const char* v) {
	Header* h = malloc(sizeof(*h));

	if (!h) {
		return NULL;
	}

	h->key = strdup(k);

	if (!h->key) {
		free(h);

		return NULL;
	}

	h->value = strdup(v);

	if (!h->value) {
		free(h->key);
		free(h);

		return NULL;
	}

	return h;
}

/**
 * Allocate memory for a response header object and append it to the array.
 * @param  r Response object.
 * @param  k Header name (key).
 * @param  v Header value.
 * @return   Allocated and appended header. Returns NULL if memory could not be allocated.
 */
Header* addResponseHeader(Response* r, const char* k, const char* v) {
	Header* h = createHeader(k, v);

	if (!h) {
		return NULL;
	}

	if (r->headerCount == r->headerCap) {
		// reached capacity - re-allocate
		// current size of the array + size of HEADER_COUNT header objects
		r->headers = realloc(r->headers, sizeof(r->headers) + sizeof(Header*) * HEADER_COUNT);

		if (!r->headers) {
			free(h);

			return NULL;
		}

		r->headerCap += HEADER_COUNT;
	}

	r->headers[r->headerCount++] = h;

	return h;
}

/**
 * Extract various bits of useful information and set it in the response object.
 * Currently only the status code is extracted.
 * @param curl
 * @param r
 */
void populateResponse(CURL* curl, Response* r) {
	long status;

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	r->status = (int) status;
}

/**
 * De-allocate memory used by a response object.
 * @param r
 */
void freeResponse(Response* r) {
	for (int i = 0; i < r->headerCount; i++) {
		free(r->headers[i]->key);
		free(r->headers[i]->value);
		free(r->headers[i]);
	}

	freePayload(r->body);
	free(r->headers);
	free(r);
}
