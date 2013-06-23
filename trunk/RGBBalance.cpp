#include "StdAfx.h"

#include "RGBBalance.h"

//Фильтр "RGB-баланс"
//Параметры:
//	hDC				DC назначения
//	lW				Ширина DC
//	lH				Высота DC
//	lROffset		Степень смещения красного. Допустимые значения: от 0 до [-]255
//	lGOffset		Степень смещения зеленого. Допустимые значения: от 0 до [-]255
//	lBOffset		Степень смещения синего. Допустимые значения: от 0 до [-]255
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL RGBBalance(HDC hDC, ULONG lW, ULONG lH, LONG lROffset, LONG lGOffset, LONG lBOffset, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	ULONG lColor, lR, lG, lB;
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

			lR = R_BGRA(lColor);
			lG = G_BGRA(lColor);
			lB = B_BGRA(lColor);

			lR = (ULONG)CheckBounds((LONG)(lR += lROffset), (LONG)0, (LONG)255);
			lG = (ULONG)CheckBounds((LONG)(lG += lGOffset), (LONG)0, (LONG)255);
			lB = (ULONG)CheckBounds((LONG)(lB += lBOffset), (LONG)0, (LONG)255);

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
