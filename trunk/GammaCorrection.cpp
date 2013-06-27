#include "StdAfx.h"

#include "GammaCorrection.h"

//Фильтр "Гамма-коррекция"
//Параметры:
//	hDC				DC назначения
//	lW				Ширина DC
//	lH				Высота DC
//	dblGamma		Степень гаммы. Допустимые значения: от 0.1 до 1.9
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL GammaCorrection(HDC hDC, ULONG lW, ULONG lH, double dblGamma, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	ULONG lColor, lR, lG, lB;
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

			lR = R_BGRA(lColor);
			lG = G_BGRA(lColor);
			lB = B_BGRA(lColor);

			lR = (ULONG)CheckBounds((LONG)((255.0 * pow((double)lR / 255.0, 1.0 / dblGamma))
				+ 0.5), (LONG)0, (LONG)255);
			lG = (ULONG)CheckBounds((LONG)((255.0 * pow((double)lG / 255.0, 1.0 / dblGamma))
				+ 0.5), (LONG)0, (LONG)255);
			lB = (ULONG)CheckBounds((LONG)((255.0 * pow((double)lB / 255.0, 1.0 / dblGamma))
				+ 0.5), (LONG)0, (LONG)255);

			SetPixel(pPixels, pBMI, i, j, BGR(lB, lG, lR));
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
