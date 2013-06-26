#include "StdAfx.h"

#include "IAlgorithm.h"
#include "AlgorithmFactory.h"

//Фильтр "Медиана"
//Параметры:
//	hDC				DC назначения
//	lW				Ширина DC
//	lH				Высота DC
//	lLevel			Радиус обработки
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки

class Median: public IAlgorithm
{
	ULONG mlLevel;

	virtual void processImage(LPBITMAPINFO pBMI, LPBYTE pPixels, ULONG lBytesCnt, const RECT &pRC)
	{
		LONG x, x1, x2, x3;
		LONG y, y1, y2, y3;
		ULONG lColor, lPixels, n;
		LPBYTE pRGBArr;

		y = pRC.top;
		while (y < pRC.bottom)
		{
			y1 = y - (mlLevel >> 1);
			if (y1 < pRC.top) y1 = pRC.top;
			y2 = y + (mlLevel >> 1);
			if (y2 > (pRC.bottom - 1)) y2 = (pRC.bottom - 1);
			x = pRC.left;
			while (x < pRC.right)
			{
				x1 = x - (mlLevel >> 1);
				if (x1 < pRC.left) x1 = pRC.left;
				x2 = x + (mlLevel >> 1);
				if (x2 > (pRC.right - 1)) x2 = (pRC.right - 1);
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

			progressEvent(y, pRC.bottom);
			y++;
		}
	}
public:
	Median(ULONG alLevel)
		:mlLevel(alLevel)
	{
	}
};

AUTO_REGISTER_ALGORITHM1( L"Filters|Median|4px",  Median, 8 );
AUTO_REGISTER_ALGORITHM1( L"Filters|Median|8px",  Median, 8 );
AUTO_REGISTER_ALGORITHM1( L"Filters|Median|16px",  Median, 8 );
