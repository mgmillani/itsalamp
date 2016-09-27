// witchcraft for usleep
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "parse.h"
#include "color.h"

#include <unistd.h>

#include "debug.h"
//#include "magic.h"

// flags for what must chnage
#define CH_NOTHING 0
#define CH_COLOR   1
#define CH_TEXT    2
#define CH_ICON    4

#define HOUR 3600
#define MINUTE 60

/**
 * Prints the time specified by seconds.
 * If it is greater than 5 minutes, prints only hours and minutes
 */
void printTime(int seconds)
{
	// >= 1h
	if(seconds >= HOUR)
		printf(" %dh%dm\n", seconds / HOUR, (seconds % HOUR) / MINUTE);
	// >= 5m
	else if(seconds >= 5*MINUTE)
		printf(" %dm\n", seconds / MINUTE);
	// >= 1m
	else if(seconds >= 1*MINUTE)
		printf(" %dm%ds\n", seconds / MINUTE, (seconds % MINUTE));
	// just seconds
	else
		printf(" %ds\n", seconds);
}

/**
 * prints a color in hexadecimal format
 */
void printColor(unsigned char color[])
{
	static char alpha[] = "0123456789ABCDEF";
	printf("%c%c%c%c%c%c"
		, alpha[(color[0] >> 4) & 0xf]
		, alpha[(color[0])      & 0xf]
		, alpha[(color[1] >> 4) & 0xf]
		, alpha[(color[1])      & 0xf]
		, alpha[(color[2] >> 4) & 0xf]
		, alpha[(color[2])      & 0xf]
		);
}

/**
 * parses a time string in the format [Hh][Mm][Ss]
 * e.g.: 15m10s for 15 minutes and 10 seconds
 * returns:
 *  the number of seconds on success
 *  -1 on parse error
 */
int parseTime(char *time)
{
	int secs = 0;
	int i;
	int j=0;
	for(i=0 ; time[i] != '\0'; i++)
	{
		switch(time[i])
		{
			case 'h':
			case 'H':
				secs += 3600 * atoi(time+j);
				j = i+1;
				break;
			case 'm':
			case 'M':
				secs += 60 * atoi(time+j);
				j = i+1;
				break;
			case 's':
			case 'S':
				secs += atoi(time+j);
				j = i+1;
				break;
			default:
				// if it is not a digit, then the format is incorrect
				if(time[i] < '0' || time[i] > '9')
					return -1;
		}
	}
	return secs;
}

/**
 * Parses the color specified by str and writes it to the vector color.
 * str must contain a color encoded in hexadecimal (e.g. html format)
 */
int parseRGB(char *str, double *RGB)
{
	int i;
	unsigned char color;
	char most = 1;
	int cl = 0;
	for(i = 0; str[i] != '\0' ; i++)
	{
		// compatibility with html format
		if(str[i] == '#')
			continue;
		// must read precisely 3 channels
		if(cl == 3)
			return 1;
		// reading the most significant part of the channel
		if(most)
		{
			color = FROM_HEXA(toupper(str[i])) << 4;
			most = 0;
		}
		else
		{
			color |= FROM_HEXA(toupper(str[i]));
			RGB[cl] = (double) color / 255.0;
			most = 1;
			cl++;
		}
	}

	// must read precisely 3 channels
	if(cl == 3)
		return 0;
	else
		return 1;
}

int main(int argc, char *argv[])
{
	time_t t0 = time(NULL);

	if(argc < 5)
	{
		ERR("usage: %s <[HOURSh][MINUTSm][SECONDSs]> <INITIAL COLOR> <FINAL COLOR> <ICONS...>", argv[0]);
		exit(1);
	}

	int seconds = parseTime(argv[1]);
	if(seconds < 0)
	{
		ERR("Wrong time format. Use something like '15m10s' (provided: %s).", argv[1]);
		exit(1);
	}

	double color0[3];
	if(parseRGB(argv[2], color0) != 0)
	{
		ERR("Wrong color format. Use something like 'FF00CC'. (provided: %s)", argv[2]);
		exit(1);
	}
	double color1[3];
	if(parseRGB(argv[3], color1) != 0)
	{
		ERR("Wrong color format. Use something like 'FF00CC'. (provided: %s)", argv[3]);
		exit(1);
	}

	TRACE("color0: %.2lf %.2lf %.2lf", color0[0], color0[1], color0[2]);
	TRACE("color1: %.2lf %.2lf %.2lf", color1[0], color1[1], color1[2]);

	int numicons = argc - 4;

	// how much the color should be incremented per second
	double colorStep[3];
	unsigned char color8[3];
	int c;
	for(c = 0; c<3 ; c++)
	{
		colorStep[c] = (color1[c] - color0[c]) / (double) seconds;
		color8[c] = color0[c]*255;
	}

	TRACE("colorStep: %lf %lf %lf", colorStep[0], colorStep[1], colorStep[2]);

	// brightness according to "HSP"
	double R = color0[0];
	double G = color0[1];
	double B = color0[2];
	double brightness0 = sqrt(R*R*Pr + G*G*Pg + B*B*Pb);
	R = color1[0];
	G = color1[1];
	B = color1[2];
	double brightness1 = sqrt(R*R*Pr + G*G*Pg + B*B*Pb);
	double brightStep = (brightness1 - brightness0) / seconds;

	TRACE("b0 = %lf\tb1 = %lf", brightness0, brightness1);

	// how long should we wait before changing the icon
	double iStep;
	iStep = seconds / (double) (numicons);

	time_t t1 = time(NULL);
	double elapsed = difftime(t1,t0);
	double nextIconT = iStep;
	// when do we need to update the minutes
	double nextTextT = seconds % 60;
	int iconI = 4;

	// initial status
	printf("%s:'%s' ", argv[2], argv[iconI]);
	fflush(stdout);
	printTime(seconds);

	while(elapsed < seconds)
	{
		// what do we need to change now
		char change = CH_NOTHING;

		// updates color
		for(c = 0; c<3 ; c++)
			// adds the step, clamping to [0, 1.0]
			color0[c] = MIN(1.0, MAX(colorStep[c] + color0[c], 0));

		brightness0 += brightStep;
		double HSP[3];
		RGBtoHSP(color0, HSP);
		HSP[2] = brightness0;
		HSPtoRGB(HSP, color0);
		// check if there was significant change in color
		for(c = 0; c<3 ; c++)
		{
			if(color0[c]*255 != color8[c])
			{
				change |= CH_COLOR;
				color8[c] = color0[c] * 255;
			}
		}

		// time to change icon
		if(elapsed > nextIconT)
		{
			change |= CH_ICON;
			iconI++;
			nextIconT += iStep;
		}
		// once there are less than 5 minutes left, start counting the seconds
		// minutes are always updated
		if(seconds - elapsed < 5*60 || elapsed > nextTextT)
		{
			change |= CH_TEXT;
			nextTextT += 60;
		}

		// we need to update something
		if(change != CH_NOTHING)
		{
			TRACE("update");
			printColor(color8);
			// if we change the icon, we update everything
			if(change & CH_ICON)
				printf(":'%s'", argv[iconI]);
			printTime(seconds - elapsed);
			fflush(stdout);
		}
		// sleeps for one second
		usleep(1000000);
		t1 = time(NULL);
		elapsed = difftime(t1,t0);
	}

	return 0;

}
