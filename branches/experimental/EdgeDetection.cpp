#include "StdAfx.h"

#include "IAlgorithm.h"
#include "AlgorithmFactory.h"

//Фильтр "Поиск граней"
//Матрица:
//  -1 -1 -1
//  -1  8 -1
//  -1 -1 -1 / 1 (сумма коэффициентов) [ опционально + color ]
//Параметры:
//	hDC				DC назначения
//	lW				Ширина DC
//	lH				Высота DC
//	crBkColor		Цвет, который будет использоваться как цвет фона
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки

class EdgeDetection: public IAlgorithm
{
	COLORREF mcrBkColor;

	virtual void processImage(LPIMAGEDESCR pIMGDESCR, LPBYTE pPixels, ULONG lBytesCnt, const RECT &pRC)
	{
		LPBYTE pPixels2 = nullptr;
		ULONG lColor[9], lR, lG, lB;
		LONG i, j, x, y;

		pPixels2 = new BYTE[lBytesCnt];
		memcpy(pPixels2, pPixels, lBytesCnt);

		for (j = pRC.top; j < pRC.bottom; j++)
		{
			for (i = pRC.left; i < pRC.right; i++)
			{
				x = (i == pRC.left)?pRC.left + 1:i;
				x = (i < (pRC.right - 1))?x:i - 1;
				y = (j == pRC.top)?pRC.top + 1:j;
				y = (j < (pRC.bottom - 1))?y:j - 1;

				lColor[0] = GetPixel(pPixels2, pIMGDESCR, x, y);
				lColor[1] = GetPixel(pPixels2, pIMGDESCR, x, y - 1);
				lColor[2] = GetPixel(pPixels2, pIMGDESCR, x + 1, y - 1);
				lColor[3] = GetPixel(pPixels2, pIMGDESCR, x + 1, y);
				lColor[4] = GetPixel(pPixels2, pIMGDESCR, x + 1, y + 1);
				lColor[5] = GetPixel(pPixels2, pIMGDESCR, x, y + 1);
				lColor[6] = GetPixel(pPixels2, pIMGDESCR, x - 1, y + 1);
				lColor[7] = GetPixel(pPixels2, pIMGDESCR, x - 1, y);
				lColor[8] = GetPixel(pPixels2, pIMGDESCR, x - 1, y - 1);

				lR = (-1 * R_BGRA(lColor[1]) - 1 * R_BGRA(lColor[2]) - 1 * R_BGRA(lColor[3]) - 1 * R_BGRA(lColor[4])
					 - 1 * R_BGRA(lColor[5]) - 1 * R_BGRA(lColor[6]) - 1 * R_BGRA(lColor[7]) - 1 * R_BGRA(lColor[8])
					 + (8 * R_BGRA(lColor[0]))) / 1;// + GetRValue(crBkColor);
				lG = (-1 * G_BGRA(lColor[1]) - 1 * G_BGRA(lColor[2]) - 1 * G_BGRA(lColor[3]) - 1 * G_BGRA(lColor[4])
					 - 1 * G_BGRA(lColor[5]) - 1 * G_BGRA(lColor[6]) - 1 * G_BGRA(lColor[7]) - 1 * G_BGRA(lColor[8])
					 + (8 * G_BGRA(lColor[0]))) / 1;// + GetGValue(crBkColor);
				lB = (-1 * B_BGRA(lColor[1]) - 1 * B_BGRA(lColor[2]) - 1 * B_BGRA(lColor[3]) - 1 * B_BGRA(lColor[4])
					 - 1 * B_BGRA(lColor[5]) - 1 * B_BGRA(lColor[6]) - 1 * B_BGRA(lColor[7]) - 1 * B_BGRA(lColor[8])
					 + (8 * B_BGRA(lColor[0]))) / 1;// + GetBValue(crBkColor);

				lR = (ULONG)CheckBounds((LONG)lR, (LONG)0, (LONG)255);
				lG = (ULONG)CheckBounds((LONG)lG, (LONG)0, (LONG)255);
				lB = (ULONG)CheckBounds((LONG)lB, (LONG)0, (LONG)255);

				SetPixel(pPixels, pIMGDESCR, i, j, BGR(lB, lG, lR));
			}

			progressEvent(j, pRC.bottom);
		}
	}

public:
	EdgeDetection(COLORREF acrBkColor)
		:mcrBkColor(acrBkColor)
	{
	}
};

AUTO_REGISTER_ALGORITHM1( L"Фильтры|Edge Detection",  EdgeDetection, RGB(255, 255, 255) );
