#include "StdAfx.h"


#include "IAlgorithm.h"
#include "AlgorithmFactory.h"

#define WAVES_DIRECTION_NS		0
#define WAVES_DIRECTION_WE		1

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


class Waves: public IAlgorithm
{
	LONG mlAmplitude;
	LONG mlFrequency;
	LONG mlDirection;
	COLORREF mcrBkColor;

	virtual void processImage(LPBITMAPINFO pBMI, LPBYTE pPixels, ULONG lBytesCnt, const RECT &pRC)
	{
		LONG i, j, x, y;
		LPBYTE pPixels2 = NULL;
		double dblFunc;

		pPixels2 = new BYTE[lBytesCnt];
		memcpy(pPixels2, pPixels, lBytesCnt);

		dblFunc = (double)((2 * 3.14159265358979) / mlFrequency);

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
					case WAVES_DIRECTION_NS:
						x = (LONG)(sin(j * dblFunc) * mlAmplitude) + i;
						y = j;
						break;
					case WAVES_DIRECTION_WE:
						x = i;
						y = (LONG)(sin(i * dblFunc) * mlAmplitude) + j;
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
	Waves(LONG lAmplitude, LONG lFrequency, LONG lDirection, COLORREF crBkColor)
		:mlAmplitude(lAmplitude), 
		mlFrequency(lFrequency), 
		mlDirection(lDirection),
		mcrBkColor(crBkColor)
	{
	}
};

//6, 48, WAVES_DIRECTION_WE, RGB(255, 255, 255)

AUTO_REGISTER_ALGORITHM4( L"Transformation|Waves|Horizontal(3, 24)",	Waves, 3,	24, WAVES_DIRECTION_NS, RGB(255, 255, 255) );
AUTO_REGISTER_ALGORITHM4( L"Transformation|Waves|Horizontal(6, 48)",	Waves, 6,	48, WAVES_DIRECTION_NS, RGB(255, 255, 255) );
AUTO_REGISTER_ALGORITHM4( L"Transformation|Waves|Horizontal(12, 96)",	Waves, 12,	96, WAVES_DIRECTION_NS, RGB(255, 255, 255) );

AUTO_REGISTER_ALGORITHM4( L"Transformation|Waves|Vertical(3, 24)",	Waves, 3,	24, WAVES_DIRECTION_WE, RGB(255, 255, 255) );
AUTO_REGISTER_ALGORITHM4( L"Transformation|Waves|Vertical(6, 48)",	Waves, 6,	48, WAVES_DIRECTION_WE, RGB(255, 255, 255) );
AUTO_REGISTER_ALGORITHM4( L"Transformation|Waves|Vertical(12, 96)",	Waves, 12,	96, WAVES_DIRECTION_WE, RGB(255, 255, 255) );
