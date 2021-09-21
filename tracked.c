#include "tracked.h"

/**
 * Allocate and initialise a tracked object on the heap.
 * @param  sectorCount The total number of sectors on the current circuit.
 */
Tracked* createTracked(int sectorCount) {
	Tracked* t = malloc(sizeof(*t));

	if (!t) {
		return NULL;
	}

	t->sectorCount = sectorCount;
	t->sectors = malloc(sizeof(int) * sectorCount);

	if (!t->sectors) {
		freeTracked(t);

		return NULL;
	}

	resetSectors(t);

	return t;
}

/**
 * Add a sector time to a tracked object.
 * @param  t
 * @param  time
 * @return         True if the lap has been completed and false otherwise.
 */
bool addSector(Tracked* t, int time) {
	t->sectors[t->currSector++] = time;

	return t->currSector >= t->sectorCount;
}

/**
 * Reset sector times and current sector.
 * @param tracked
 */
void resetSectors(Tracked* t) {
	t->currSector = 0;

	for (int i = 0; i < t->sectorCount; i++) {
		t->sectors[i] = 0;
	}
}

/**
 * Free the memory held by a tracked object.
 * @param t
 */
void freeTracked(Tracked* t) {
	if (!t) {
		return;
	}

	free(t->sectors);
	free(t);
}
