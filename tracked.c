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
 * Calculate the previous sector time from the current lap time.
 * @param  t
 * @param  idx  Sector index of the sector time.
 * @param  time The current lap time.
 * @return      True if the lap has been completed and false otherwise.
 */
int addSector(Tracked* t, int idx, int time) {
	for (int i = 0; i < idx; i++) {
		time -= t->sectors[i];
	}

	t->sectors[idx] = time;

	return time;
}

/**
 * Reset sector times and current sector.
 * @param tracked
 */
void resetSectors(Tracked* t) {
	for (int i = 0; i < t->sectorCount; i++) {
		t->sectors[i] = 0;
	}
}

/**
 * Set the number of sectors.
 * @param  t
 * @param  sectorCount
 * @return             False if re-allocation was unsuccessful. True otherwise.
 */
bool setSectorCount(Tracked* t, int sectorCount) {
	int* temp = malloc(sizeof(int) * sectorCount);

	if (!temp) {
		return false;
	}

	free(t->sectors);
	t->sectors = temp;
	t->sectorCount = sectorCount;

	// reset the sector values
	resetSectors(t);

	return true;
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
