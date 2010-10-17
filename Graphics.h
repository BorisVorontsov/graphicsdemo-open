#ifndef GRAPHICS_H
#define GRAPHICS_H

//LOWORD(wParam): event ID, HIWORD(wParam): reserved, lParam: LPON_event_name_PARAMS 
#define WM_GRAPHICSEVENT		WM_USER + 0xFF

#define EVENT_ON_PROGRESS		0x10

typedef struct tagONPROGRESSPARAMS
{
	DWORD dwPercents;
} ONPROGRESSPARAMS, *LPONPROGRESSPARAMS;

BOOL EmptyFilter(HDC hDC, ULONG lW, ULONG lH, /*..., */LPRECT pRC, HWND hWndCallback = NULL);

BOOL Blur(HDC hDC, ULONG lW, ULONG lH, ULONG lLevel, LPRECT pRC, HWND hWndCallback = NULL);
#ifdef __USE_OPENCL__
BOOL Blur_OCL(HDC hDC, ULONG lW, ULONG lH, ULONG lLevel);
#endif
BOOL RGBBalance(HDC hDC, ULONG lW, ULONG lH, LONG lROffset, LONG lGOffset, LONG lBOffset, LPRECT pRC, HWND hWndCallback = NULL);
BOOL GrayScale(HDC hDC, ULONG lW, ULONG lH, LPRECT pRC, HWND hWndCallback = NULL);
BOOL GammaCorrection(HDC hDC, ULONG lW, ULONG lH, double dblGamma, LPRECT pRC, HWND hWndCallback = NULL);
BOOL EdgeDetection(HDC hDC, ULONG lW, ULONG lH, COLORREF crBkColor, LPRECT pRC, HWND hWndCallback = NULL);
BOOL Median(HDC hDC, ULONG lW, ULONG lH, ULONG lLevel, LPRECT pRC, HWND hWndCallback = NULL);
BOOL Contrast(HDC hDC, ULONG lW, ULONG lH, LONG lOffset, LPRECT pRC, HWND hWndCallback = NULL);

typedef enum tagALPHAMODE
{
	AM_ALPHA_IGNORE = 0,		//Использование только альфа-компоненты пикселя
	AM_ALPHA_ADD,				//Добавление значения альфы к альфа-компоненте пикселя
	AM_ALPHA_SUBTRACT,			//Вычитание значения альфы из альфа-компоненты пикселя
	AM_ALPHA_REPLACE			//Замена альфа-компоненты пикселя на указанную альфу
} ALPHAMODE;

BOOL AlphaBlend(HDC hDstDC, ULONG lDstW, ULONG lDstH, HDC hSrcDC, ULONG lSrcW, ULONG lSrcH, BYTE bAlpha, ALPHAMODE AlphaMode, LPRECT pRC, HWND hWndCallback = NULL);

BOOL Shear(HDC hDC, ULONG lW, ULONG lH, LONG lX, LONG lY, COLORREF crBkColor, LPRECT pRC, HWND hWndCallback = NULL);

#define ROTATE_DIRECTION_CW		0
#define ROTATE_DIRECTION_CCW	1

BOOL Rotate(HDC hDC, ULONG lW, ULONG lH, LONG lX, LONG lY, LONG lAngle, LONG lDirection, COLORREF crBkColor,
			LPRECT pRC, HWND hWndCallback = NULL);

#define WAVES_DIRECTION_NS		0
#define WAVES_DIRECTION_WE		1

BOOL Waves(HDC hDC, ULONG lW, ULONG lH, LONG lAmplitude, LONG lFrequency, LONG lDirection, COLORREF crBkColor,
		   LPRECT pRC, HWND hWndCallback = NULL);

#endif