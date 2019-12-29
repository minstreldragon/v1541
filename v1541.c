#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "blocks.h"

#define BLOCKNUM 683
#define BLOCKSVALID 532
#define MAXSTRLEN 200


int file[BLOCKNUM];
int folge[BLOCKNUM];

void command_summary(void);

void readblock(FILE *fp, int track, int sector, char *block)
{
	long int blockno;
	
	printf("reading track %d sector %d\n", track, sector);
	makeblock(track, sector, &blockno);
	if (fseek(fp, blockno*256, SEEK_SET))
	{
		fprintf(stderr, "Error in fseek!\n");
		exit(3);
	}
	fread(block, sizeof(char), 256, fp);
}


int findTrackSector(FILE *fp, int *ftrack, int *fsector, int *totalblocks, int filenum)
{
	int track = 18;	// directory track
	int sector = 1; // directory sector
	int offset;
	int type;
	char *fname;

	char block[256];

	do
	{
		readblock(fp, track, sector, block);
		for (offset=2; offset < 256; offset += 0x20)
		{
			type = (unsigned char) block[offset];
			if ((type >= 0x80) && (filenum == 0))
			{
				*ftrack = block[offset+1];
				*fsector = block[offset+2];
				*totalblocks = (int)(unsigned char)block[offset+28]+(int)(unsigned char)block[offset+29]*256;
				fname = strtok(&block[offset+3], "\xa0\x00");
				printf("%02d: \"%s\"\n", filenum, fname);
				return (1);
			}
			filenum--;
			if (filenum < 0) return (*ftrack = *fsector = 0);
		}
		track = block[0];
		sector = block[1];
	} while (track);

}


void extract_file(FILE *fpin, int tokenc, char *tokenv[])
{
	FILE *fpout;
	int track, sector;
	int bytesnum;
	long int totalbytes;
	int totalblocks;
	char block[256];

	if (tokenc < 3)
	{
		command_summary();
		return;
	}
	findTrackSector(fpin, &track, &sector, &totalblocks, atoi(tokenv[1]));
	if (track == 0)
	{
		fprintf(stderr, "Couldn't open .d64 file #%s\n", tokenv[1]);
		return;
	}
	if ((fpout=fopen(tokenv[2],"wb")) == NULL)
	{
		fprintf(stderr, "Couldn't open output file %s\n", tokenv[2]);
		return;
	}

	printf("totalblocks: %d\n", totalblocks);
	totalbytes = 0;
	do
	{
		readblock(fpin, track, sector, block);
		track = block[0];
		sector = block[1];
		bytesnum = (track == 0) ? (unsigned char)(sector-1) : 254;
		totalbytes += (long int) bytesnum;
		fwrite(block+2, sizeof(char), bytesnum, fpout);
		printf("writing %d bytes", bytesnum);
	} while (track);
	fclose(fpout);

	printf("# of bytes in file: %ld\n", totalbytes);
}

void extract_file_track(FILE *fpin, int tokenc, char *tokenv[])
{
	FILE *fpout;
	int track, sector;
	int bytesnum;
	long int totalbytes;
	int totalblocks;
	char block[256];

	if (tokenc < 4)
	{
		command_summary();
		return;
	}
	track = atoi(tokenv[1]);
	sector = atoi(tokenv[2]);
	if ((track == 0) || (track > 35))
	{
		fprintf(stderr, "Illegal Tracknumber\n", tokenv[1]);
		return;
	}
	if ((fpout=fopen(tokenv[3],"wb")) == NULL)
	{
		fprintf(stderr, "Couldn't open output file %s\n", tokenv[2]);
		return;
	}

	totalbytes = 0;
	do
	{
		readblock(fpin, track, sector, block);
		track = block[0];
		sector = block[1];
		bytesnum = (track == 0) ? (unsigned char)(sector-1) : 254;
		totalbytes += (long int) bytesnum;
		fwrite(block+2, sizeof(char), bytesnum, fpout);
		printf("writing %d bytes", bytesnum);
	} while (track);
	fclose(fpout);

	printf("# of bytes in file: %ld\n", totalbytes);
}

void substitute_file(FILE *fpin, int tokenc, char *tokenv[])
{
	FILE *fpsub;
	int track, sector;
	int bytesnum;
	long int totalbytes, subsbytes;
	int totalblocks;
	char block[256];

	if (tokenc < 3)
	{
		command_summary();
		return;
	}
	findTrackSector(fpin, &track, &sector, &totalblocks, atoi(tokenv[1]));
	if (track == 0)
	{
		fprintf(stderr, "Couldn't open .d64 file #%s\n", tokenv[1]);
		return;
	}
	if ((fpsub=fopen(tokenv[2],"rb")) == NULL)
	{
		fprintf(stderr, "Couldn't open DOS file %s\n", tokenv[2]);
		return;
	}

	fseek(fpsub, 0L, SEEK_END);
	subsbytes = ftell(fpsub);
	printf("Bytes in substitute file: %ld\n", subsbytes);
	rewind(fpsub);

	totalbytes = 0;
	do
	{
		readblock(fpin, track, sector, block);
		track = block[0];
		sector = block[1];
		bytesnum = (track == 0) ? (unsigned char)(sector-1) : 254;
		totalbytes += (long int) bytesnum;
		bytesnum = (bytesnum < subsbytes) ? bytesnum : subsbytes;
		fread(block+2, sizeof(char), bytesnum, fpsub);
		subsbytes -= bytesnum;
		fseek(fpin, -254L, SEEK_CUR);
		fwrite(block+2, sizeof(char), bytesnum, fpin);
	} while (track);
	fclose(fpsub);

	printf("# of bytes in file: %ld\n", totalbytes);
	if (subsbytes)
		printf("Couldn't substitute %ld excess bytes\n", subsbytes);
}

void list_directory(FILE *fp)
{
	int track = 18;	// directory track
	int sector = 1; // directory sector
	int offset;
	int type;
	int filenum = 0;

	char *fname;
	char block[256];

	do
	{
		readblock(fp, track, sector, block);
		for (offset=2; offset < 256; offset += 0x20)
		{
			type = (unsigned char) block[offset];
			if (type >= 0x80)
			{
				fname = strtok(&block[offset+3], "\xa0\x00");
				printf("%02d: \"%s\"\n", filenum, fname);
			}
			filenum++;
		}
		track = block[0];
		sector = block[1];
	} while (track);
}


void usage(void)
{
	fprintf(stderr, "v1541 <diskimage.d64>");
	exit(1);
}

void command_summary(void)
{
	printf("\nVirtual 1541 Command Summary\n\n");
	printf("d                print directory\n");
	printf("e <num> <file>   extract    <file #> <output file>\n");
	printf("t <num> <file>   extract    <track> <sector> <output file>\n");
	printf("s <num> <file>   substitute <file #> <output file>\n");
	printf("q                quit\n\n");
}
	
void main(int argc, char **argv)
{
	long int startbyte, anzahlbytes, endbyte; 
	long int pos;
	int i;
	int byte;
	int commandloop = 1;
	char cmdstring[MAXSTRLEN];
	char seps[] = " \t\n,";
	char *token, *tokenv[20];
	int tokenc;

	FILE *fpin;

	if (argc < 2)
		usage();

	if ((fpin=fopen(argv[1],"rb+")) == NULL)
	{
		fprintf(stderr, "Couldn't open input file %s\n", argv[1]);
		exit(2);
	}

	while (commandloop)
	{
		gets(cmdstring);

		tokenc = 0;
		token = strtok(cmdstring, seps);
		for (i=0; token != NULL; i++)
		{
			tokenc++;
			tokenv[i] = token;
			token = strtok(NULL, seps); // find next token
		}
		
		switch (tolower(*tokenv[0]))
		{
			case 'd': list_directory(fpin); break;
			case 'e': extract_file(fpin, tokenc, tokenv); break;
			case 't': extract_file_track(fpin, tokenc, tokenv); break;
			case 's': substitute_file(fpin, tokenc, tokenv); break;
			case 'q': commandloop = 0; break;
			default: command_summary();
		}
	}
}
