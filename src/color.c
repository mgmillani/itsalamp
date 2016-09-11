#include <math.h>

#include "color.h"

void RGBtoHSP(double RGB[], double HSP[])
{
	double R = RGB[0];
	double G = RGB[1];
	double B = RGB[2];
	double *H = HSP;
	double *S = HSP+1;
	double *P = HSP+2;
	//  Calculate the Perceived brightness.
	*P=sqrt(R*R*Pr + G*G*Pg + B*B*Pb);

	//  Calculate the Hue and Saturation.  (This part works
	//  the same way as in the HSV/B and HSL systems???.)
	if (R==G && R==B)
	{
		*H = 0;
		*S = 0;
		return;
	}
	if (R>=G && R>=B)
	{
		// R is largest
		if (B>=G)
		{
			// *H=6./6.-1./6.*(B-G)/(R-G);
			*H = 1 - (B-G)/( 6 * (R-G) );
			*S = 1 - G/R;
		}
		else
		{
			// *H=0/6.+1./6.*(G-B)/(R-B);
			*H = (G-B)/( 6 * (R-B) );
			*S = 1 - B/R;
		}
	}
	else if (G>=R && G>=B)
	{
		// G is largest
		if (R>=B)
		{
			// *H=2./6.-1./6.*(R-B)/(G-B);
			*H = 2./6. - (R-B)/( 6 * (G-B) );
			*S = 1 - B/G;
		}
		else
		{
			*H = 2./6. + (B-R)/( 6 * (G-R) );
			*S = 1 - R/G;
		}
	}
	else
	{
		// B is largest
		if (G>=R)
		{
			// *H=4./6.-1./6.*(G-R)/(B-R);
			*H = 4./6. -(G-R)/( 6 * (B-R) );
			*S = 1 - R/B;
		}
		else
		{
			// *H = 4./6. + 1./6.*(R-G)/(B-G);
			*H = 4./6. + (R-G)/( 6 * (B-G) );
			*S = 1. - G/B;
		}
	}
}

void HSPtoRGB(double HSP[], double RGB[])
{
	double H = HSP[0];
	double S = HSP[1];
	double P = HSP[2];
	double *R = RGB;
	double *G = RGB+1;
	double *B = RGB+2;

	double part;
	double minOverMax = 1 - S;

	if (minOverMax > 0)
	{
		if ( H < 1/6. )     //  R>G>B
		{
			H *= 6;
			part = 1 + H*(1/minOverMax - 1);
			*B = P/sqrt(Pr/(minOverMax*minOverMax) + Pg*part*part + Pb);
			*R = (*B)/minOverMax;
			*G = (*B) + H*( (*R) - (*B) );
		}
		else if ( H < 2/6. )     //  G>R>B
		{
			H = 6*(2/6 - H);
			part = 1. + H*(1/minOverMax - 1);
			*B = P/sqrt(Pg/(minOverMax*minOverMax) + Pr*part*part + Pb);
			*G = (*B)/minOverMax;
			*R = (*B) + H*( (*G) - (*B) );
		}
		else if ( H < 3./6.)     //  G>B>R
		{
			H = 6*( H - 2/6. );
			part = 1 + H*(1/minOverMax - 1);
			*R = P/sqrt(Pg/(minOverMax*minOverMax) + Pb*part*part + Pr);
			*G = (*R)/minOverMax;
			*B = (*R) + H*( (*G) - (*R) );
		}
		else if ( H < 4/6. )     //  B>G>R
		{
			H = 6*( 4/6. - H);
			part = 1. + H*(1/minOverMax - 1);
			*R = P/sqrt(Pb/minOverMax/minOverMax+Pg*part*part+Pr);
			*B = (*R)/minOverMax;
			*G = (*R)+H*((*B)-(*R));
		}
		else if ( H < 5/6.)     //  B>R>G
		{
			H = 6*( H - 4/6.);
			part = 1 + H*(1/minOverMax - 1);
			*G = P/sqrt(Pb/(minOverMax*minOverMax) + Pr*part*part + Pg);
			*B = (*G)/minOverMax;
			*R = (*G) + H*( (*B) - (*G) );
		}
		else                   //  R>B>G
		{
			H = 6 * (1 - H);
			part = 1 + H*(1/minOverMax - 1);
			*G = P/sqrt(Pr/(minOverMax*minOverMax) + Pb*part*part + Pg);
			*R = (*G)/minOverMax;
			*B = (*G) + H*( (*R) - (*G) );
		}
	}
	else
	{
		if ( H < 1/6. ) //  R>G>B
		{
			H *= 6;
			*R = sqrt(P*P/(Pr + Pg*H*H));
			*G = (*R)*H;
			*B = 0;
		}
		else if ( H < 2/6. ) //  G>R>B
		{
			H = 6 * (2/6. - H);
			*G = sqrt(P*P/(Pg + Pr*H*H));
			*R = (*G)*H;
			*B = 0;
		}
		else if ( H < 3/6. ) //  G>B>R
		{
			H = 6 * ( H - 2/6.);
			*G = sqrt(P*P/(Pg + Pb*H*H));
			*B = (*G)*H;
			*R = 0;
		}
		else if ( H < 4./6. ) //  B>G>R
		{
			H = 6 * ( 4/6. - H );
			*B = sqrt(P*P/(Pb + Pg*H*H));
			*G = (*B)*H;
			*R = 0;
		}
		else if ( H < 5/6. ) //  B>R>G
		{
			H =  6 * ( H - 4/6. );
			*B = sqrt(P*P/(Pb + Pr*H*H));
			*R = (*B)*H;
			*G = 0;
		}
		else                   //  R>B>G
		{
			H =  6 * (1 - H);
			*R = sqrt(P*P/(Pr + Pb*H*H));
			*B = (*R)*H;
			*G = 0;
		}
	}

	// sees if the output RGB has some invalid value like 2
	int c;
	double max = 0;
	for(c = 0; c < 3 ; c++)
	{
		if(RGB[c] > max)
			max = RGB[c];
	}
	if(max > 1)
		for(c = 0; c < 3 ; c++)
		{
			RGB[c] /= max;
		}
}
