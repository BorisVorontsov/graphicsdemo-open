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

	volatile ONPROGRESSPARAMS ONPP = {0};

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
