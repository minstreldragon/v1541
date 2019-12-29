#include <stdio.h>
#include <stdlib.h>


void copy(FILE *fpin, FILE *fpout, int anzahl)
{
	int byte;

	for (; anzahl > 0; anzahl--)
	{
		if ((byte = fgetc(fpin)) == EOF)
		{
			fprintf(stderr,"Error: unexpected EOF!\n");
			exit(2);
		}
		fputc(byte, fpout);
	}
}

void splitblock(long int block, int *track, int *sector)	
{
	if (block < (17 * 21))
	{
		*track = block / 21 + 1;
		*sector = block % 21;
	}
	else if (block < (24 * 19 + 34))
	{
		*track = (block - 34) / 19 + 1;
		*sector = (block - 34) % 19;
	}
	else if (block < (30 * 18 + 58))
	{
		*track = (block - 58) / 18 + 1;
		*sector = (block - 58) % 18;
	}
	else if (block < (35 * 17 + 88))
	{
		*track = (block - 88) / 17 + 1;
		*sector = (block - 88) % 17;
	}
	else
	{
		printf("ERROR: splitblock %d\n", block);
		exit (6);
	}
}

void makeblock(int track, int sector, long int *block)
{
	int blocknum;

	if ((track < 1) || (track > 35))
	{
		fprintf(stderr,"ERROR: Track range, track = %d\n", track);
		exit(4);
	}
	if ((sector < 0) || (sector > 20))
	{
		fprintf(stderr,"ERROR: Sector range, sector = %d\n", sector);
		exit(4);
	}
	track--;
	if (track < 17)
		*block = track * 21 + sector;
	else if (track < 24)
		*block = track * 19 + 34 + sector;
	else if (track < 30)
		*block = track * 18 + 58 + sector;
	else
		*block = track * 17 + 88 + sector;
}
