#include "StdAfx.h"

#include "IAlgorithm.h"
#include "AlgorithmFactory.h"


//Сдвиг изображения
//Параметры:
//	hDC				DC назначения
//	lW				Ширина DC
//	lH				Высота DC
//	lX				Сдвиг (правого нижнего угла) по оси X
//	lY				Сдвиг (правого нижнего угла) по оси Y
//	crBkColor		Цвет, который будет использоваться как цвет фона
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки


class Shear: public IAlgorithm
{
	LONG mlX;
	LONG mlY; 
	COLORREF mcrBkColor;

	virtual void processImage(LPBITMAPINFO pBMI, LPBYTE pPixels, ULONG lBytesCnt, LPRECT pRC)
	{
		LPBYTE pPixels2 = NULL;
		LONG i, j, x, x1, y, y1, sx, sy;

		pPixels2 = new BYTE[lBytesCnt];
		memcpy(pPixels2, pPixels, lBytesCnt);

		//RGB[A] -> BGR[A]
		ReverseBytes((LPBYTE)&mcrBkColor, 3);

		//Заливаем область для изменения цветом фона
		for (j = pRC->top; j < pRC->bottom; j++)
		{
			for (i = pRC->left; i < pRC->right; i++)
			{
				SetPixel(pPixels, pBMI, i, j, mcrBkColor);
			}
		}

		//Вычисляем шаги для x и y
		if ((mlX != 0)) sx = ((pRC->bottom - pRC->top) / abs(mlX));
		if ((mlY != 0)) sy = ((pRC->right - pRC->left) / abs(mlY));

		x1 = 0;
		for (j = pRC->top; j < pRC->bottom; j++)
		{
			if ((mlX != 0))
				if ((j % sx) == 0) x1++;
			y1 = 0;
			for (i = pRC->left; i < pRC->right; i++)
			{
				if ((mlY != 0))
					if ((i % sy) == 0) y1++;

				if (mlX != 0)
				{
					x = (mlX > 0)?i - x1:i + x1;
				}
				else x = i;
				if (mlY != 0)
				{
					y = (mlY > 0)?j - y1:j + y1;
				}
				else y = j;

				if ((x < pRC->left) || (x > (pRC->right - 1))) continue;
				if ((y < pRC->top) || (y > (pRC->bottom - 1))) continue;

				SetPixel(pPixels, pBMI, i, j, GetPixel(pPixels2, pBMI, x, y));
			}

			progressEvent(j, pRC->bottom);
		}
	}
public:
	Shear(LONG lX, LONG lY, COLORREF crBkColor)
		:mlX(lX), mlY(lY), mcrBkColor(crBkColor)
	{
	}
};


AUTO_REGISTER_ALGORITHM3( L"Transformation|Shear|x: -24, y: +24",  Shear, -24, 24, RGB(255, 255, 255) );
AUTO_REGISTER_ALGORITHM3( L"Transformation|Shear|x: -48, y: +48",  Shear, -48, 48, RGB(255, 255, 255) );
AUTO_REGISTER_ALGORITHM3( L"Transformation|Shear|x: -96, y: +96",  Shear, -96, 96, RGB(255, 255, 255) );
