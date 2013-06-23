#pragma once

#define ROTATE_DIRECTION_CW		0
#define ROTATE_DIRECTION_CCW	1

BOOL Rotate(HDC hDC, ULONG lW, ULONG lH, LONG lX, LONG lY, LONG lAngle, LONG lDirection, COLORREF crBkColor,
			LPRECT pRC, HWND hWndCallback = NULL);

