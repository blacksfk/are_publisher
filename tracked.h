#ifndef TRACKED_H
#define TRACKED_H

#include <stdlib.h>
#include <stdbool.h>

#define DEFAULT_SECTOR_COUNT 3

typedef struct tracked {
	int* sectors;
	int currSector;
	int sectorCount;
} Tracked;

Tracked* createTracked(int);
int addSector(Tracked*, int);
void resetSectors(Tracked*);
bool setSectorCount(Tracked*, int);
void freeTracked(Tracked*);

#endif
