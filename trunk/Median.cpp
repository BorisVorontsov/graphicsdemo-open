#include "StdAfx.h"

#include "Median.h"

//Фильтр "Медиана"
//Параметры:
//	hDC				DC назначения
//	lW				Ширина DC
//	lH				Высота DC
//	lLevel			Радиус обработки
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL Median(HDC hDC, ULONG lW, ULONG lH, ULONG lLevel, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	LONG x, x1, x2, x3;
	LONG y, y1, y2, y3;
	ULONG lColor, lPixels, n;
	LPBYTE pRGBArr;

	volatile ONPROGRESSPARAMS ONPP = {};

	if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
		if (pPixels)
			delete[] pPixels;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}

	y = pRC->top;
	while (y < pRC->bottom)
	{
		y1 = y - (lLevel >> 1);
		if (y1 < pRC->top) y1 = pRC->top;
		y2 = y + (lLevel >> 1);
		if (y2 > (pRC->bottom - 1)) y2 = (pRC->bottom - 1);
		x = pRC->left;
		while (x < pRC->right)
		{
			x1 = x - (lLevel >> 1);
			if (x1 < pRC->left) x1 = pRC->left;
			x2 = x + (lLevel >> 1);
			if (x2 > (pRC->right - 1)) x2 = (pRC->right - 1);
			lPixels = (x2 - x1 + 1) * (y2 - y1 + 1);
			//В целях оптимизации выделяем один массив на три компонента
			pRGBArr = new BYTE[lPixels * 3];
			n = 0;
			for (x3 = x1; x3 <= x2; x3++)
			{
				for (y3 = y1; y3 <= y2; y3++)
				{
					lColor = GetPixel(pPixels, pBMI, x3, y3);
					pRGBArr[n] = R_BGRA(lColor);
					(pRGBArr + lPixels)[n] = G_BGRA(lColor);
					(pRGBArr + (lPixels << 1))[n] = B_BGRA(lColor);
					n++;
				}
			}
			SortArray_Shell(pRGBArr, lPixels);
			SortArray_Shell((pRGBArr + lPixels), lPixels);
			SortArray_Shell((pRGBArr + (lPixels << 1)), lPixels);
			//Вот по этому фильтр и называется медиана (серидина) -- берется середина
			//отсортированных массивов R/G/B
			n = ((lPixels - 1) >> 1);
			SetPixel(pPixels, pBMI, x, y, BGR((pRGBArr + (lPixels << 1))[n], (pRGBArr + lPixels)[n], pRGBArr[n]));
			delete[] pRGBArr;
			x++;
		}
		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)y / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
		y++;
	}

	SetImagePixels(hDC, lW, lH, pPixels, pBMI);

	delete[] pPixels;
	delete pBMI;

	return TRUE;
}
