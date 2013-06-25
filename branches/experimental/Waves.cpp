#include "StdAfx.h"

#include "Waves.h"

//Волны
//Формула:
//	x' = sin(y * 2Pi / freq) * amp + x
//	y' = sin(x * 2Pi / freq) * amp + y
//Параметры:
//	hDC				DC назначения
//	lW				Ширина DC
//	lH				Высота DC
//	lAmplitude		Амплитуда волн
//	lFrequency		Частота волн
//	lDirection		Направление (вертикальные волны/горизонтальные волны)
//	crBkColor		Цвет, который будет использоваться как цвет фона
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL Waves(HDC hDC, ULONG lW, ULONG lH, LONG lAmplitude, LONG lFrequency, LONG lDirection, COLORREF crBkColor, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels1 = NULL, pPixels2 = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	LONG i, j, x, y;
	double dblFunc;

	volatile ONPROGRESSPARAMS ONPP = {0};

	if (!GetImagePixels(hDC, lW, lH, &pPixels1, &lBytesCnt, &pBMI)) {
		if (pPixels1)
			delete[] pPixels1;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}
	pPixels2 = new BYTE[lBytesCnt];
	memcpy(pPixels2, pPixels1, lBytesCnt);

	dblFunc = (double)((2 * 3.14159265358979) / lFrequency);

	ReverseBytes((LPBYTE)&crBkColor, 3);

	for (j = pRC->top; j < pRC->bottom; j++)
	{
		for (i = pRC->left; i < pRC->right; i++)
		{
			SetPixel(pPixels1, pBMI, i, j, crBkColor);
		}
	}

	for (j = pRC->top; j < pRC->bottom; j++)
	{
		for (i = pRC->left; i < pRC->right; i++)
		{
			switch (lDirection)
			{
				case WAVES_DIRECTION_NS:
					x = (LONG)(sin(j * dblFunc) * lAmplitude) + i;
					y = j;
					break;
				case WAVES_DIRECTION_WE:
					x = i;
					y = (LONG)(sin(i * dblFunc) * lAmplitude) + j;
					break;
				default:
					//
					break;
			}

			if ((x < pRC->left) || (x > (pRC->right - 1))) continue;
			if ((y < pRC->top) || (y > (pRC->bottom - 1))) continue;

			SetPixel(pPixels1, pBMI, i, j, GetPixel(pPixels2, pBMI, x, y));
		}
		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)j / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
	}

	SetImagePixels(hDC, lW, lH, pPixels1, pBMI);

	delete[] pPixels1;
	delete[] pPixels2;
	delete pBMI;

	return TRUE;
}
