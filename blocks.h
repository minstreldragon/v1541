#include <stdio.h>
#include <stdlib.h>
void copy(FILE *fpin, FILE *fpout, int anzahl);

void splitblock(long int block, int *track, int *sector);

void makeblock(int track, int sector, long int *block);
