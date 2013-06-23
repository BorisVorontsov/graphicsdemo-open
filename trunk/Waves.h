#pragma once

#define WAVES_DIRECTION_NS		0
#define WAVES_DIRECTION_WE		1

BOOL Waves(HDC hDC, ULONG lW, ULONG lH, LONG lAmplitude, LONG lFrequency, LONG lDirection, COLORREF crBkColor,
		   LPRECT pRC, HWND hWndCallback = NULL);
