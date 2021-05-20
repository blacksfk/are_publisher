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

	p->len = 0;
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

	r->head = createPayload();

	if (!r->head) {
		free(r);

		return NULL;
	}

	r->body = createPayload();

	if (!r->body) {
		free(r);
		free(r->head);

		return NULL;
	}

	r->code = CURLE_OK;

	return r;
}

/**
 * De-allocate memory used by a response object.
 * @param r
 */
void freeResponse(Response* r) {
	freePayload(r->head);
	freePayload(r->body);
	free(r);
}
