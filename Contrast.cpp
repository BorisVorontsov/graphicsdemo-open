#include "StdAfx.h"

#include "Contrast.h"
//Фильтр "Контрастность"
//Параметры:
//  hDC             DC назначения
//  lOffset         Степень изменения контрастности. Допустимые значения: от -100 до 100
//  pRC             Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL Contrast(HDC hDC, ULONG lW, ULONG lH, LONG lOffset, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	ULONG lAB = 0;
	ULONG lColor;
	LONG i, j, lR, lG, lB, lLevel = abs(lOffset);

	volatile ONPROGRESSPARAMS ONPP = {0};

	if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
		if (pPixels)
			delete[] pPixels;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}

	for (j = pRC->top; j < pRC->bottom; j++) {
		for (i = pRC->left; i < pRC->right; i++) {
			lColor = GetPixel(pPixels, pBMI, i, j);

			lR = R_BGRA(lColor);
			lG = G_BGRA(lColor);
			lB = B_BGRA(lColor);

			lAB += (ULONG)(lR * 0.299 + lG * 0.587 + lB * 0.114);
		}
	}

	lAB /= ((pRC->right - pRC->left) * (pRC->bottom - pRC->top));

	for (j = pRC->top; j < pRC->bottom; j++)
	{
		for (i = pRC->left; i < pRC->right; i++)
		{
			lColor = GetPixel(pPixels, pBMI, i, j);

			lR = R_BGRA(lColor);
			lG = G_BGRA(lColor);
			lB = B_BGRA(lColor);

			lR = (ULONG)CheckBounds((LONG)(lR + ((lR - /*127.0*/(LONG)lAB) * ((double)((lOffset < 0)?-lLevel:lLevel) /
				(double)(201 - (-lLevel + 100))))), (LONG)0, (LONG)255);
			lG = (ULONG)CheckBounds((LONG)(lG + ((lG - /*127.0*/(LONG)lAB) * ((double)((lOffset < 0)?-lLevel:lLevel) /
				(double)(201 - (-lLevel + 100))))), (LONG)0, (LONG)255);
			lB = (ULONG)CheckBounds((LONG)(lB + ((lB - /*127.0*/(LONG)lAB) * ((double)((lOffset < 0)?-lLevel:lLevel) /
				(double)(201 - (-lLevel + 100))))), (LONG)0, (LONG)255);

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

