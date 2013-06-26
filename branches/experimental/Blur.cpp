#include "StdAfx.h"

#include "IAlgorithm.h"
#include "AlgorithmFactory.h"

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

class Blur: public IAlgorithm
{
	ULONG mLevel;

	virtual void processImage(LPBITMAPINFO pBMI, LPBYTE pPixels, ULONG lBytesCnt, const RECT &pRC)
	{
		LONG x, x1, x2, x3;
		LONG y, y1, y2, y3;
		ULONG lColor, lR, lG, lB, lPixels;

		y = pRC.top;
		while (y < pRC.bottom)
		{
			y1 = y - (mLevel >> 1);
			if (y1 < pRC.top) y1 = pRC.top;
			y2 = y + (mLevel >> 1);
			if (y2 > (pRC.bottom - 1)) y2 = (pRC.bottom - 1);
			x = pRC.left;
			while (x < pRC.right)
			{
				x1 = x - (mLevel >> 1);
				if (x1 < pRC.left) x1 = pRC.left;
				x2 = x + (mLevel >> 1);
				if (x2 > (pRC.right - 1)) x2 = (pRC.right - 1);
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

			progressEvent(y, pRC.bottom);
			y++;
		}
	}
public:
	Blur(ULONG alLevel)
		:mLevel(alLevel)
	{
	}
};

AUTO_REGISTER_ALGORITHM1( L"Filters|Blur|8px",  Blur, 8);
AUTO_REGISTER_ALGORITHM1( L"Filters|Blur|16px",  Blur, 16);
AUTO_REGISTER_ALGORITHM1( L"Filters|Blur|24px",  Blur, 24);

