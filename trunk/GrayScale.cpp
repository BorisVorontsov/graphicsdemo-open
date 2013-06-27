#include "StdAfx.h"

#include "GrayScale.h"

//Фильтр "Серые тона"
//Параметры:
//	hDC				DC назначения
//	lW				Ширина DC
//	lH				Высота DC
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL GrayScale(HDC hDC, ULONG lW, ULONG lH, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	ULONG lColor, lR, lG, lB, lS;
	LONG i, j;

	volatile ONPROGRESSPARAMS ONPP = {};

	if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
		if (pPixels)
			delete[] pPixels;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}

	for (j = pRC->top; j < pRC->bottom; j++)
	{
		for (i = pRC->left; i < pRC->right; i++)
		{
			lColor = GetPixel(pPixels, pBMI, i, j);

			//BGR -> RGB
			lR = R_BGRA(lColor);
			lG = G_BGRA(lColor);
			lB = B_BGRA(lColor);

			lS = (ULONG)(lR * 0.299 + lG * 0.587 + lB * 0.114);

			//SSS -> BGR
			SetPixel(pPixels, pBMI, i, j, BGR(lS, lS, lS));
		}
		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)j / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
	}

	SetImagePixels(hDC, lW, lH, pPixels, pBMI);

	delete[] pPixels;
	delete pBMI;

	return TRUE;
}

//Оптимизированный фильтр "Серые тона" (hsilgos)

//Таблица предвычисленных значений серого для компонент 
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

//Алгоритм
//Параметры:
//	hDC				DC назначения
//	lW				Ширина DC
//	lH				Высота DC
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL GrayScale_Fast(HDC hDC, ULONG lW, ULONG lH, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	BYTE r, g, b, s;

	volatile ONPROGRESSPARAMS ONPP = {};

	if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
		if (pPixels)
			delete[] pPixels;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}

	static const CalculatedCoeffs precalc;

	ULONG lStep = pBMI->bmiHeader.biBitCount / 8;
	for (ULONG i = 0; i < lBytesCnt; i+= lStep)
	{

		r = pPixels[i];
		g = pPixels[i + 1];
		b = pPixels[i + 2];

		s = precalc.get(r, g, b);

		pPixels[i] = s;
		pPixels[i + 1] = s;
		pPixels[i + 2] = s;

		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)i / (double)lBytesCnt) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}

	}

	SetImagePixels(hDC, lW, lH, pPixels, pBMI);

	delete[] pPixels;
	delete pBMI;

	return TRUE;
}