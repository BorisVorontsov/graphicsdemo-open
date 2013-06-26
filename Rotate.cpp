#include "StdAfx.h"

#include "IAlgorithm.h"
#include "AlgorithmFactory.h"

//Поворот изображения на произвольный угол
//Формула:
//	x' = cos(rad) * (x - x0) +/- sin(rad) * (y - y0) + x0
//	y' = cos(rad) * (y - y0) -/+ sin(rad) * (x - x0) + y0
//Параметры:
//	hDC				DC назначения
//	lW				Ширина DC
//	lH				Высота DC
//	lX				Центр поворота по оси X
//	lY				Центр поворота по оси Y
//	lAngle			Угол поворота
//	lDirection		Направление (по часовой стрелке/против часовой стрелки)
//	crBkColor		Цвет, который будет использоваться как цвет фона
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки

#define ROTATE_DIRECTION_CW		0
#define ROTATE_DIRECTION_CCW	1


class Rotate: public IAlgorithm
{
	LONG mlAngle;
	LONG mlDirection;
	COLORREF mcrBkColor;
	LONG mX;
	LONG mY;

	virtual void processImage(LPBITMAPINFO pBMI, LPBYTE pPixels, ULONG lBytesCnt, const RECT &pRC)
	{
		LPBYTE pPixels2 = NULL;
		LONG i, j, x, y;
		double dblRad;
		//LONG lX, lY;

		if( -1 == mX && -1 == mY)
		{
			mX = (pRC.right + pRC.left )/ 2;
			mY = (pRC.top + pRC.bottom )/ 2;
		}

		pPixels2 = new BYTE[lBytesCnt];
		memcpy(pPixels2, pPixels, lBytesCnt);

		dblRad = (double)(mlAngle * (3.14159265358979 / 180));

		mX += pRC.left;
		mY += pRC.top;

		ReverseBytes((LPBYTE)&mcrBkColor, 3);

		for (j = pRC.top; j < pRC.bottom; j++)
		{
			for (i = pRC.left; i < pRC.right; i++)
			{
				SetPixel(pPixels, pBMI, i, j, mcrBkColor);
			}
		}

		for (j = pRC.top; j < pRC.bottom; j++)
		{
			for (i = pRC.left; i < pRC.right; i++)
			{
				switch (mlDirection)
				{
					case ROTATE_DIRECTION_CW:
						x = (LONG)(cos(dblRad) * (i - mX) + sin(dblRad) * (j - mY) + mX);
						y = (LONG)(cos(dblRad) * (j - mY) - sin(dblRad) * (i - mX) + mY);
						break;
					case ROTATE_DIRECTION_CCW:
						x = (LONG)(cos(dblRad) * (i - mX) - sin(dblRad) * (j - mY) + mX);
						y = (LONG)(cos(dblRad) * (j - mY) + sin(dblRad) * (i - mX) + mY);
						break;
					default:
						//
						break;
				}

				if ((x < pRC.left) || (x > (pRC.right - 1))) continue;
				if ((y < pRC.top) || (y > (pRC.bottom - 1))) continue;

				SetPixel(pPixels, pBMI, i, j, GetPixel(pPixels2, pBMI, x, y));
			}

			progressEvent(j, pRC.bottom);
		}
	}

public:
	Rotate(LONG lAngle, LONG lDirection, COLORREF crBkColor, LONG aX, LONG aY)
		:mlAngle(lAngle), mlDirection(lDirection), mcrBkColor(crBkColor), mX(aX), mY(aY)
	{
	}
};

AUTO_REGISTER_ALGORITHM5( L"Transformation|Rotate|+34°",  Rotate, 34, ROTATE_DIRECTION_CW, RGB(255, 255, 255), -1, -1 );
AUTO_REGISTER_ALGORITHM5( L"Transformation|Rotate|+68°",  Rotate, 68, ROTATE_DIRECTION_CW, RGB(255, 255, 255), -1, -1 );
AUTO_REGISTER_ALGORITHM5( L"Transformation|Rotate|+102°",  Rotate, 102, ROTATE_DIRECTION_CW, RGB(255, 255, 255), -1, -1 );
AUTO_REGISTER_ALGORITHM5( L"Transformation|Rotate|+136°",  Rotate, 136, ROTATE_DIRECTION_CW, RGB(255, 255, 255), -1, -1 );

AUTO_REGISTER_ALGORITHM5( L"Transformation|Rotate|-34°",  Rotate, -34, ROTATE_DIRECTION_CW, RGB(255, 255, 255), -1, -1 );
AUTO_REGISTER_ALGORITHM5( L"Transformation|Rotate|-68°",  Rotate, -68, ROTATE_DIRECTION_CW, RGB(255, 255, 255), -1, -1 );
AUTO_REGISTER_ALGORITHM5( L"Transformation|Rotate|-102°",  Rotate, -102, ROTATE_DIRECTION_CW, RGB(255, 255, 255), -1, -1 );
AUTO_REGISTER_ALGORITHM5( L"Transformation|Rotate|-136°",  Rotate, -136, ROTATE_DIRECTION_CW, RGB(255, 255, 255), -1, -1 );
