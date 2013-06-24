#pragma once

BOOL GrayScale(HDC hDC, ULONG lW, ULONG lH, LPRECT pRC, HWND hWndCallback = NULL);

struct CalculatedCoeffs
{
	enum 
	{
		ColorCount	= 256,
		red_coeff   = 6968,
		green_coeff = 23434,
		blue_coeff	= 32768 - red_coeff - green_coeff
	};

	int r[ColorCount];
	int g[ColorCount];
	int b[ColorCount];

	CalculatedCoeffs()
	{
		for( int i = 0 ; i < ColorCount; ++i )
		{
			r[i] = i * red_coeff;
			g[i] = i * green_coeff;
			b[i] = i * blue_coeff;
		}
	}

	unsigned char get(unsigned char ar, unsigned char ag, unsigned char ab) const
	{
		return (unsigned char)((r[ar] + g[ag] + b[ab]) >> 15);
	}
};

BOOL GrayScale_Fast(HDC hDC, ULONG lW, ULONG lH, LPRECT pRC, HWND hWndCallback = NULL);
