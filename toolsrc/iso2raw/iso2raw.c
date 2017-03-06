// this is a mashup of hitmen's iso2raw.c
// and nocash's psx-spx pseudocode: http://nocash.emubase.de/psx-spx.htm
// mashup applied by GreaseMonkey

/*
PSX Images need to be Mode2/XA 2352, which fortunatly can be represented by
traditional bin/cue images. However, mkisofs generates plain 2048bytes/sector
images - this tool converts them into the proper format that can be burned.

example:

mkisofs -volid PLAYSTATION -sysid PLAYSTATION -appid PLAYSTATION -o psx.iso files/ 
iso2raw nolicence psx.iso
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

typedef int BOOL;
#define TRUE 1
#define FALSE 0

static BOOL
BuildCueFile (char *rawFile, char *cueFile)
{
	FILE *fp;
	char buf[10 * 1024];

	if (!rawFile || !cueFile)
		return FALSE;

	fp = fopen (cueFile, "w");
	if (!fp)
	  {
		  printf ("ERROR: Failed to create cue file.\n");
		  return FALSE;
	  }

	(void) sprintf (buf, "FILE \"%s\" BINARY\n", rawFile);
	fputs (buf, fp);
	fputs (" TRACK 01 MODE2/2352\n INDEX 01 00:00:00\n", fp);

	fclose (fp);

	return TRUE;
}

uint32_t edc_table[256];
int GF8_LOG[256];
int GF8_ILOG[256];
int GF8_PRODUCT[43][256];

int subfunc(int a, int b)
{
	if(a>0)
	{
		a=GF8_LOG[a]-b;
		if(a<0)
			a += 255;

		a=GF8_ILOG[a];
	}

	return(a);
}

void init_tables(void)
{
	int i, j;

	// standard "fast-CRC" LUT except with a different polynomial
	for(i=0; i <= 0xFF; i++)
	{
		uint32_t x = i;
		for(j = 0; j <= 7; j++)
		{
			uint32_t carry = x&1;
			x >>= 1;

			if(carry)
				x ^= 0xD8018001;
		}

		edc_table[i]=x;
	}

	GF8_LOG[0x00]=0x00;
	GF8_ILOG[0xFF]=0x00;
	int x=0x01;
	for(i=0x00; i <= 0xFE; i++)
	{
		GF8_LOG[x]=i;
		GF8_ILOG[i]=x;

		int carry8bit = x&0x80;
		x <<= 1;
		if(carry8bit)
			x ^= 0x1D;

		x &= 0xFF;
	}

	for(j=0; j <= 42; j++)
	{
		int xx = GF8_ILOG[44-j];
		int yy = subfunc(xx ^ 1,0x19);

		xx = subfunc(xx,0x01);
		xx = subfunc(xx ^ 1,0x18);
		xx = GF8_LOG[xx];
		yy = GF8_LOG[yy];
		GF8_PRODUCT[j][0]=0x0000;
		for(i=0x01; i <= 0xFF; i++)
		{
			int x=xx+GF8_LOG[i];
			int y=yy+GF8_LOG[i];

			if(x>=255) x -= 255;
			if(y>=255) y -= 255;

			GF8_PRODUCT[j][i]=GF8_ILOG[x]+(GF8_ILOG[y] << 8);
		}
	}
}

void calc_parity(uint8_t *sector, int offs, int len, int j0, int step1, int step2)
{
	int i, j;

	int src=0x00c;
	int dst=0x81c+offs;
	int srcmax=dst;

	for(i = 0; i <= len-1; i++)
	{
		int base=src, x=0x0000, y=0x0000;
		for(j=j0; j <= 42; j++)
		{
			x ^= GF8_PRODUCT[j][sector[src+0]];
			y ^= GF8_PRODUCT[j][sector[src+1]];
			src += step1;
			if((step1 == 2*44) && (src>=srcmax))
				src -= 2*1118;
		}

		sector[dst+2*len+0]=x & 0x0FF; sector[dst+0]=x >> 8;
		sector[dst+2*len+1]=y & 0x0FF; sector[dst+1]=y >> 8;
		dst += 2;
		src = base + step2;
	}
}

void calc_p_parity(uint8_t *sector)
{
	calc_parity(sector,0,43,19,2*43,2);
}

void calc_q_parity(uint8_t *sector)
{
	calc_parity(sector,43*4,26,0,2*44,2*43);
}

void adjust_edc(uint8_t *addr, int len)
{
	int i;
	uint32_t x=0x00000000;

	for(i=0; i <= len-1; i++)
	{
		x ^= (uint32_t)(uint8_t)addr[i];
		x = (x>>8) ^ edc_table[x & 0xFF];
	}

	//append EDC value (little endian)
	addr[0*4+len+0] = x >> 0;
	addr[0*4+len+1] = x >> 8;
	addr[0*4+len+2] = x >> 16;
	addr[0*4+len+3] = x >> 24;
}


static BOOL
Iso2Raw (char *isoFile, char *rawFile)
{
	FILE *fp1;
	FILE *fp2;
	unsigned char sec[16+8+2048+4+276];
	unsigned char *sub = &sec[16];
	unsigned char *buf = &sec[16+8];
	unsigned char *edc = &sec[16+8+2048];
	unsigned char *ecc = &sec[16+8+2048+4];
	unsigned int out;
	int c;
	int thesec = 2*75;
	char dog[10];

	(void) memset (sec, 0xFF, 16);
	sec[0x000] = 0;
	sec[0x00B] = 0;
	sec[0x00C] = 0;
	sec[0x00D] = 2;
	sec[0x00E] = 0;
	sec[0x00F] = 2;

	(void) memset (sub, 0x00, 8);
	(void) memset (edc, 0x01, 4);
	(void) memset (ecc, 0x02, 280);

	fp1 = fopen (isoFile, "rb");

	if (!fp1)
	  {
		  printf ("ERROR: Failed to open ISO file for reading.\n");
		  return FALSE;
	  }

	fp2 = fopen (rawFile, "wb");

	if (!fp2)
	  {
		  printf ("ERROR: Failed to create raw ISO file.\n");
		  return FALSE;
	  }

	while (1)
	  {
		  c = fread (buf, sizeof (unsigned char), 2048, fp1);
		  if (!c)
			  break;

		  sec[0x00C] = (thesec/75)/60;
		  sec[0x00D] = (thesec/75)%60;
		  sec[0x00E] = thesec%75;

		  adjust_edc(sec+0x10,0x800+8);

		  // temporarily clear header
		  uint8_t oldhdr[4];
		  memcpy(oldhdr, &sec[0x00C], 4);
		  memset(&sec[0x00C], 0, 4);
		  calc_p_parity(sec);
		  calc_q_parity(sec);
		  memcpy(&sec[0x00C], oldhdr, 4); // -restore header

		  fwrite (sec, sizeof (unsigned char), 16, fp2);
		  fwrite (sub, sizeof (unsigned char), 8, fp2);
		  fwrite (buf, sizeof (unsigned char), 2048, fp2);
		  fwrite (edc, sizeof (unsigned char), 4, fp2);
		  fwrite (ecc, sizeof (unsigned char), 276, fp2);

		  thesec++;
	  }

	fclose (fp2);
	fclose (fp1);

	return TRUE;
}

static BOOL
CopyLicence (char *licFile, char *rawFile)
{
	FILE *fp1;
	FILE *fp2;
	unsigned char buf[2352];
	int a = 0;
	struct stat sinfo;

	if (!licFile || !rawFile)
		return FALSE;

	fp1 = fopen (licFile, "rb");

	if (!fp1)
	  {
		  printf ("ERROR: Failed to open licence file for reading.\n");
		  return FALSE;
	  }

	if (stat (licFile, &sinfo) != -1)
	  {
		  if (sinfo.st_size != (16 * 2352))
		    {
			    fclose (fp1);
			    printf ("ERROR: Licence file specified is the incorrect size.\n");
			    return FALSE;
		    }
	  }

	fp2 = fopen (rawFile, "r+b");

	if (!fp2)
	  {
		  printf ("ERROR: Failed to open raw ISO image file for writing.\n");
		  return FALSE;
	  }

	if (stat (rawFile, &sinfo) != -1)
	  {
		  div_t dinfo;
		  dinfo = div (sinfo.st_size, 2352);

		  if (dinfo.rem != 0)
		    {
			    fclose (fp1);
			    fclose (fp2);
			    printf ("ERROR: The raw ISO file is not a multiple of 2352 bytes.\n");
			    return FALSE;
		    }
	  }

	fseek (fp2, 0, SEEK_SET);

	while (1)
	  {
		  int c = fread (buf, sizeof (unsigned char), 2352, fp1);

		  if (!c)
			  break;

		  fseek (fp2, a, SEEK_SET);
		  fwrite (buf, sizeof (unsigned char), c, fp2);
		  a += c;
	  }

	fclose (fp1);
	fclose (fp2);

	return TRUE;
}

static BOOL
IsISOValid (char *isoFile)
{
	FILE *fp;
	unsigned char buf[128];
	int c;
	struct stat sinfo;

	fp = fopen (isoFile, "rb");

	if (!fp)
	  {
		  printf ("ERROR: Failed to open ISO image file.\n");
		  return FALSE;
	  }

	c = fread (buf, sizeof (unsigned char), 32, fp);
	if (c != 32)
	  {
		  fclose (fp);
		  printf ("ERROR: Invalid mkisofs ISO image, file too small.\n");
		  return FALSE;
	  }

	if (buf[0] == 0x00 && buf[1] == 0xFF && buf[2] == 0xFF
	    && buf[3] == 0xFF && buf[4] == 0xFF && buf[5] == 0xFF
	    && buf[6] == 0xFF && buf[7] == 0xFF && buf[8] == 0xFF
	    && buf[9] == 0xFF && buf[10] == 0xFF && buf[11] == 0x00
	    && buf[12] == 0x00 && buf[14] == 0x00)
	  {
		  fclose (fp);
		  printf ("ERROR: ISO image already has Raw Sector information.\n");
		  return FALSE;
	  }

	fclose (fp);

	if (stat (isoFile, &sinfo) != -1)
	  {
		  if ((sinfo.st_size % 2048) != 0)
		    {
			    printf ("ERROR: ISO file specified is not 2048 bytes per sector.\n");
			    return FALSE;
		    }
	  }

// XXX Would like to validate if the ISO has
// version numbers enabled
// and if it is mode1/mode2 etc etc...
// could even check if there is a system.cnf file!

	return TRUE;
}

static BOOL
PatchIso (char *licFile, char *rawFile)
{
	if (!CopyLicence (licFile, rawFile))
		return FALSE;

	return TRUE;
}

int
main (int argc, char *argv[])
{
	char *licFile;
	char *isoFile;
	char *temp;
	char *infile;
	char *cueFile;
	char *rawFile;
	int licence = 1;

	printf ("---------------------------------\n");
	printf ("ISO9660 2048->2352 converter tool\n");
	printf ("---------------------------------\n\n");

	if (argc < 2)
	  {
		  printf ("Wrong number of argument.\n\tUsage: iso2raw licence isofile.iso\n\t\tIf you don't want to merge a licence file, write nolicence\n");
		  exit (1);
	  }


	infile = malloc (1024);
	strcpy (infile, argv[2]);
	printf ("Input file: %s\n", infile);

	rawFile = malloc (1024);
	strcpy (rawFile, infile);
	strcat (rawFile, ".bin"); // NO$PSX doesn't actually check the filename properly and assumes bin
	printf ("Output file: %s\n", rawFile);

	cueFile = malloc (1024);
	strcpy (cueFile, infile);
	strcat (cueFile, ".cue");
	printf ("Cue file: %s\n", cueFile);

	if (!strcmp (argv[1], "nolicence"))
		licence = 0;
	else
	  {
		  licFile = argv[1];
		  printf ("Licence file: %s\n", licFile);
	  }

	if (!IsISOValid (infile))
		exit (1);

	init_tables();

	if (!Iso2Raw (infile, rawFile))
		exit (1);

	if (licence)
		if (!PatchIso (licFile, rawFile))
			exit (1);

	if (!BuildCueFile (rawFile, cueFile))
		exit (1);

	printf ("Done \"%s\".\n", cueFile);
}
