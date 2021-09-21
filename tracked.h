#ifndef TRACKED_H
#define TRACKED_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct tracked {
	int* sectors;
	int currSector;
	int sectorCount;
} Tracked;

Tracked* createTracked(int);
bool addSector(Tracked*, int);
void resetSectors(Tracked*);
void freeTracked(Tracked*);

#endif
