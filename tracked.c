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
 * Calculate the current sector time from the current lap time.
 * @param  t
 * @param  time The current lap time
 * @return      True if the lap has been completed and false otherwise.
 */
int addSector(Tracked* t, int time) {
	if (t->currSector >= t->sectorCount) {
		resetSectors(t);
	}

	for (int i = 0; i < t->currSector; i++) {
		// subtract each previous sector from the given lap time
		time -= t->sectors[i];
	}

	t->sectors[t->currSector++] = time;

	return time;
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
