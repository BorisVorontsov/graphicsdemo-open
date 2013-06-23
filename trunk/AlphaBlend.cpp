#include "StdAfx.h"

#include "AlphaBlend.h"

//Фильтр "Наложение по альфе"
//Параметры:
//	hDstDC			DC назначения
//	lDstW			Ширина выводимого растра
//	lDstH			Высота выводимого растра
//	hSrcDC			DC источника
//	lSrcW			Ширина растра источника
//	lSrcH			Высота растра источника
//	bAlpha			Уровень полупрозначности для накладываемого растра (не учитывается, если задан режим AM_ALPHA_IGNORE)
//	AlphaMode		Режим работы с альфа-каналом
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL AlphaBlend(HDC hDstDC, ULONG lDstW, ULONG lDstH, HDC hSrcDC, ULONG lSrcW, ULONG lSrcH, BYTE bAlpha, ALPHAMODE AlphaMode, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pSrcPixels = NULL, pDstPixels = NULL;
	ULONG lSrcBytesCnt = 0, lDstBytesCnt = 0;
	LPBITMAPINFO pSrcBMI = NULL, pDstBMI = NULL;
	ULONG lSrcC, lDstC, lR, lG, lB;
	LONG i, j;
	BOOL bResult;

	volatile ONPROGRESSPARAMS ONPP = {0};

	//Получаем пиксели конечного изображения
	if (!GetImagePixels(hDstDC, lDstW, lDstH, &pDstPixels, &lDstBytesCnt, &pDstBMI)) {
		bResult = FALSE;
		goto AB_Exit;
	}

	//Получаем пиксели исходного изображения
	if (!GetImagePixels(hSrcDC, lSrcW, lSrcH, &pSrcPixels, &lSrcBytesCnt, &pSrcBMI)) {
		bResult = FALSE;
		goto AB_Exit;
	}

	//Не работаем с изображениями ниже 24 бит на пиксель
	if ((pDstBMI->bmiHeader.biBitCount < 24) || (pSrcBMI->bmiHeader.biBitCount < 24)) {
		bResult = FALSE;
		goto AB_Exit;
	}

	//Если размеры исходного изображения отличаются от заданной области применения фильтра, выполняем мастабирование исходного изображения
	if ((lSrcW != (pRC->right - pRC->left)) || (lSrcH != (pRC->bottom - pRC->top)))
	{
		LPBYTE pTmpPixels;
		LPBITMAPINFO pTmpBMI;
		ResampleImagePixels(pSrcPixels, pSrcBMI, &pTmpPixels, &pTmpBMI, (pRC->right - pRC->left), (pRC->bottom - pRC->top));
		delete[] pSrcPixels;
		delete pSrcBMI;
		pSrcPixels = pTmpPixels;
		pSrcBMI = pTmpBMI;
		lSrcW = pSrcBMI->bmiHeader.biWidth;
		lSrcH = pSrcBMI->bmiHeader.biHeight;
	}

	for (j = pRC->top; j < pRC->bottom; j++)
	{
		for (i = pRC->left; i < pRC->right; i++)
		{
			lDstC = GetPixel(pDstPixels, pDstBMI, i, j);
			lSrcC = GetPixel(pSrcPixels, pSrcBMI, i - pRC->left, j - pRC->top);

			switch (AlphaMode)
			{
				case AM_ALPHA_IGNORE:
					lR = (R_BGRA(lDstC) * (255 - A_BGRA(lSrcC)) + R_BGRA(lSrcC) * A_BGRA(lSrcC)) / 255;
					lG = (G_BGRA(lDstC) * (255 - A_BGRA(lSrcC)) + G_BGRA(lSrcC) * A_BGRA(lSrcC)) / 255;
					lB = (B_BGRA(lDstC) * (255 - A_BGRA(lSrcC)) + B_BGRA(lSrcC) * A_BGRA(lSrcC)) / 255;
					break;
				case AM_ALPHA_ADD:
					lR = (R_BGRA(lDstC) * (255 - min(255, (A_BGRA(lSrcC) + bAlpha))) + R_BGRA(lSrcC) * min(255, (A_BGRA(lSrcC) + bAlpha))) / 255;
					lG = (G_BGRA(lDstC) * (255 - min(255, (A_BGRA(lSrcC) + bAlpha))) + G_BGRA(lSrcC) * min(255, (A_BGRA(lSrcC) + bAlpha))) / 255;
					lB = (B_BGRA(lDstC) * (255 - min(255, (A_BGRA(lSrcC) + bAlpha))) + B_BGRA(lSrcC) * min(255, (A_BGRA(lSrcC) + bAlpha))) / 255;
					break;
				case AM_ALPHA_SUBTRACT:
					lR = (R_BGRA(lDstC) * (255 - max(0, (A_BGRA(lSrcC) - bAlpha))) + R_BGRA(lSrcC) * max(0, (A_BGRA(lSrcC) - bAlpha))) / 255;
					lG = (G_BGRA(lDstC) * (255 - max(0, (A_BGRA(lSrcC) - bAlpha))) + G_BGRA(lSrcC) * max(0, (A_BGRA(lSrcC) - bAlpha))) / 255;
					lB = (B_BGRA(lDstC) * (255 - max(0, (A_BGRA(lSrcC) - bAlpha))) + B_BGRA(lSrcC) * max(0, (A_BGRA(lSrcC) - bAlpha))) / 255;
					break;
				case AM_ALPHA_REPLACE:
					lR = (R_BGRA(lDstC) * (255 - bAlpha) + R_BGRA(lSrcC) * bAlpha) / 255;
					lG = (G_BGRA(lDstC) * (255 - bAlpha) + G_BGRA(lSrcC) * bAlpha) / 255;
					lB = (B_BGRA(lDstC) * (255 - bAlpha) + B_BGRA(lSrcC) * bAlpha) / 255;
					break;
			}

			SetPixel(pDstPixels, pDstBMI, i, j, BGRA(lB, lG, lR, A_BGRA(lDstC)));
		}

		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)j / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
	}

	SetImagePixels(hDstDC, lDstW, lDstH, pDstPixels, pDstBMI);

	bResult = TRUE;

AB_Exit:

	if (pDstPixels)
		delete[] pDstPixels;
	if (pDstBMI)
		delete pDstBMI;

	if (pSrcPixels)
		delete[] pSrcPixels;
	if (pSrcBMI)
		delete pSrcBMI;

	return bResult;
}
