// witchcraft for usleep
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "parse.h"
#include "color.h"

#include <sys/select.h>
#include <unistd.h>

#include "debug.h"
//#include "magic.h"

// flags for what must change
#define CH_NOTHING 0
#define CH_COLOR   1
#define CH_TEXT    2
#define CH_ICON    4

#define HOUR 3600
#define MINUTE 60

typedef struct s_timer
{
	int seconds;             // total number of seconds
	double elapsed;          // time elapsed since the beginning
	double nextIconT;	       // when do we need to change the icon
	double nextTextT;        // when do we need to change the tooltip text
	double color0[3];        // initial color
	double color[3];         // current color
	double color1[3];        // final color
	double colorStep[3];     // color change per second
	unsigned char color8[3]; // color in 8-bit per channel
	double iStep;
	double brightness0;      // original brightness
	double brightness;       // current brightness
	double brightStep;       // brightness change per second
	int iconI;               // index of the icon in argv
}t_timer;

void setupTimer(t_timer *t, int numicons, time_t t0)
{
	// how much the color should be incremented per second
	int c;
	for(c = 0; c<3 ; c++)
	{
		t->colorStep[c] = (t->color1[c] - t->color0[c]) / (double) t->seconds;
		t->color8[c] = t->color0[c]*255;
		t->color[c] = t->color0[c];
	}
	TRACE("colorStep: %lf %lf %lf", t->colorStep[0], t->colorStep[1], t->colorStep[2]);
	// brightness according to "HSP"
	double R = t->color0[0];
	double G = t->color0[1];
	double B = t->color0[2];
	t->color[0] = R;
	t->color[1] = G;
	t->color[2] = B;
	t->brightness0 = sqrt(R*R*Pr + G*G*Pg + B*B*Pb);
	t->brightness = t->brightness0;
	R = t->color1[0];
	G = t->color1[1];
	B = t->color1[2];
	double brightness1 = sqrt(R*R*Pr + G*G*Pg + B*B*Pb);
	t->brightStep = (brightness1 - t->brightness0) / t->seconds;

	TRACE("b0 = %lf\tb1 = %lf", t->brightness0, brightness1);

	// how long should we wait before changing the icon
	t->iStep = t->seconds / (double) (numicons);

	time_t t1 = time(NULL);
	t->elapsed = difftime(t1,t0);
	t->nextIconT = t->iStep;
	// when do we need to update the minutes
	t->nextTextT = t->seconds % 60;
	t->iconI = 0;
}

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
	printf("#%c%c%c%c%c%c"
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
	int valid = 0;
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
				valid = 1;
				break;
			case 'm':
			case 'M':
				secs += 60 * atoi(time+j);
				j = i+1;
				valid = 1;
				break;
			case 's':
			case 'S':
				secs += atoi(time+j);
				j = i+1;
				valid = 1;
				break;
			default:
				// if it is not a digit, then the format is incorrect
				if(time[i] < '0' || time[i] > '9')
					return -1;
		}
	}
	return valid ? secs : -1;
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
			color |= FROM_HEXA(toupper(str[i])); // program flow guarantees that the value is initialized by now
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

int waitForInput(t_timer *t, int numicons, time_t *t0, char **icon)
{
	fd_set rfds;
	struct timeval tv;
	int retval;
	int restart = 0;

	if(!feof(stdin))
	{
		FD_ZERO(&rfds);
		FD_SET(0, &rfds); // stdin = 0

		// up to one second
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		retval = select(1, &rfds, NULL, NULL, &tv);
	}
	else
	{
		retval = 0;
		TRACE("Sleep");
		usleep(1000*1000);
	}

	// if there is something to read
	if(retval > 0)
	{
		int textLen = 1024;
		char text[textLen];
		fgets(text, textLen, stdin);
		int i;
		for(i=0 ; text[i] != '\0' ; i++)
		{
			text[i] = tolower(text[i]);
			if(text[i] == '\n')
				text[i] = '\0';
		}
		TRACE("text: %s", text);
		// reset timer
		if(strcmp(text, "reset") == 0)
		{
			time(t0);
			setupTimer(t, numicons, *t0);
			printColor(t->color8);
			printf(":'%s' ", icon[t->iconI]);
			printTime(t->seconds - t->elapsed);
			fflush(stdout);
			restart = 1;
		}
		else
		{
			// check if the user changed the time
			int seconds = parseTime(text);
			if(seconds != -1)
			{
				t->seconds = seconds;
				time(t0);
				setupTimer(t, numicons, *t0);
				printColor(t->color8);
				printf(":'%s' ", icon[t->iconI]);
				printTime(t->seconds - t->elapsed);
				fflush(stdout);
				restart = 1;
			}
		}
	}
	else if(retval == -1)
	{
		perror("waitForInput");
	}

	return restart;
}

int main(int argc, char *argv[])
{
	time_t t0 = time(NULL);

	if(argc < 5)
	{
		ERR("usage: %s <[HOURSh][MINUTSm][SECONDSs]> <INITIAL COLOR> <FINAL COLOR> <ICONS...>", argv[0]);
		exit(1);
	}
	t_timer t;

	t.seconds = parseTime(argv[1]);
	if(t.seconds < 0)
	{
		ERR("Wrong time format. Use something like '15m10s' (provided: %s).", argv[1]);
		exit(1);
	}

	if(parseRGB(argv[2], t.color0) != 0)
	{
		ERR("Wrong color format. Use something like 'FF00CC'. (provided: %s)", argv[2]);
		exit(1);
	}

	if(parseRGB(argv[3], t.color1) != 0)
	{
		ERR("Wrong color format. Use something like 'FF00CC'. (provided: %s)", argv[3]);
		exit(1);
	}

	TRACE("color0: %.2lf %.2lf %.2lf", t.color0[0], t.color0[1], t.color0[2]);
	TRACE("color1: %.2lf %.2lf %.2lf", t.color1[0], t.color1[1], t.color1[2]);

	int numicons = argc - 4;

	setupTimer(&t, numicons, t0);

	char **icon = argv+4;

	// initial status
	printf("#%s:'%s' ", argv[2], icon[t.iconI]);
	printTime(t.seconds);
	fflush(stdout);

	time_t t1;

	while(t.elapsed < t.seconds)
	{
		// sleeps for one second, waiting for input on stdin
		while(waitForInput(&t, numicons, &t0, icon))
			;
		t1 = time(NULL);
		t.elapsed = difftime(t1,t0);

		// what do we need to change now
		char change = CH_NOTHING;

		// updates color
		int c;
		for(c = 0; c<3 ; c++)
			// adds the step, clamping to [0, 1.0]
			t.color[c] = MIN(1.0, MAX(t.colorStep[c] + t.color[c], 0));

		t.brightness += t.brightStep;
		double HSP[3];
		RGBtoHSP(t.color, HSP);
		HSP[2] = t.brightness;
		HSPtoRGB(HSP, t.color);
		// check if there was significant change in color
		for(c = 0; c<3 ; c++)
		{
			if(t.color[c]*255 != t.color8[c])
			{
				change |= CH_COLOR;
				t.color8[c] = t.color[c] * 255;
			}
		}

		// time to change icon
		if(t.elapsed > t.nextIconT)
		{
			change |= CH_ICON;
			t.iconI++;
			t.nextIconT += t.iStep;
		}
		// once there are less than 5 minutes left, start counting the seconds
		// minutes are always updated
		if(t.seconds - t.elapsed < 5*60 || t.elapsed > t.nextTextT)
		{
			change |= CH_TEXT;
			t.nextTextT += 60;
		}

		// we need to update something
		if(change != CH_NOTHING)
		{
			TRACE("update");
			printColor(t.color8);
			// if we change the icon, we update everything
			if(change & CH_ICON)
				printf(":'%s'", icon[t.iconI]);
			printTime(t.seconds - t.elapsed);
			fflush(stdout);
		}
	}

	return 0;

}
