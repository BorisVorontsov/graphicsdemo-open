#include "StdAfx.h"

#include "EmptyFilter.h"

//Шаблон для новых фильтров
BOOL EmptyFilter(HDC hDC, ULONG lW, ULONG lH, /*...дополнительные параметры..., */LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	ULONG lColor, lA, lR, lG, lB;
	LONG i, j;

	volatile ONPROGRESSPARAMS ONPP = {};

	if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
		if (pPixels)
			delete[] pPixels;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}

	//-----------------------------------------------------------------------------------------------
	//Тело фильтра
	for (j = pRC->top; j < pRC->bottom; j++)
	{
		for (i = pRC->left; i < pRC->right; i++)
		{
			lColor = GetPixel(pPixels, pBMI, i, j);

			lA = A_BGRA(lColor);
			lR = R_BGRA(lColor);
			lG = G_BGRA(lColor);
			lB = B_BGRA(lColor);

			//...

			SetPixel(pPixels, pBMI, i, j, BGRA(lB, lG, lR, lA));
		}
		//Отображение статуса выполнения (опциональный блок)
		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)j / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
	}
	//-----------------------------------------------------------------------------------------------

	SetImagePixels(hDC, lW, lH, pPixels, pBMI);

	delete[] pPixels;
	delete pBMI;

	return TRUE;
}
