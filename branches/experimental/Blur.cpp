#include "StdAfx.h"

#include "Blur.h"

//Фильтр "Пятно"
//Матрица:
//   n  n  n  n  n
//   n  1  1  1  n
//   n  1  1  1  n
//   n  1  1  1  n
//   n  n  n  n  n / 1
//Параметры:
//	hDC				DC назначения
//	lW				Высота DC
//	lH				Ширина DC
//	lLevel			Радиус размывания
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL Blur(HDC hDC, ULONG lW, ULONG lH, ULONG lLevel, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	ULONG lColor, lR, lG, lB, lPixels;
	LONG x, x1, x2, x3;
	LONG y, y1, y2, y3;

	//Параметры событий
	volatile ONPROGRESSPARAMS ONPP = {0};

	//Получаем пиксели изображения
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
			lR = 0;
			lG = 0;
			lB = 0;
			for (x3 = x1; x3 <= x2; x3++)
			{
				for (y3 = y1; y3 <= y2; y3++)
				{
					lColor = GetPixel(pPixels, pBMI, x3, y3);
					lR += R_BGRA(lColor);
					lG += G_BGRA(lColor);
					lB += B_BGRA(lColor);
				}
			}
			lPixels = (x2 - x1 + 1) * (y2 - y1 + 1);
			lR /= lPixels;
			lG /= lPixels;
			lB /= lPixels;
			SetPixel(pPixels, pBMI, x, y, BGR(lB, lG, lR));
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

	//Присваиваем измененные пиксели
	SetImagePixels(hDC, lW, lH, pPixels, pBMI);

	delete[] pPixels;
	delete pBMI;

	return TRUE;
}
