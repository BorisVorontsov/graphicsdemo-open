#include "StdAfx.h"


#include "IAlgorithm.h"
#include "AlgorithmFactory.h"

//Алгоритм
//Параметры:
//	hDC				DC назначения
//	lW				Ширина DC
//	lH				Высота DC
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
class GrayScale: public IAlgorithm
{
	virtual void processImage(LPBITMAPINFO pBMI, LPBYTE pPixels, ULONG lBytesCnt, const RECT &pRC)
	{
		ULONG lColor, lR, lG, lB, lS;
		LONG i, j;

		for (j = pRC.top; j < pRC.bottom; j++)
		{
			for (i = pRC.left; i < pRC.right; i++)
			{
				lColor = GetPixel(pPixels, pBMI, i, j);

				//BGR -> RGB
				lR = R_BGRA(lColor);
				lG = G_BGRA(lColor);
				lB = B_BGRA(lColor);

				lS = (ULONG)(lR * 0.299 + lG * 0.587 + lB * 0.114);

				//SSS -> BGR
				SetPixel(pPixels, pBMI, i, j, BGR(lS, lS, lS));
			}

			progressEvent(j, pRC.bottom);
		}
	}
public:

};


//Алгоритм
//Параметры:
//	hDC				DC назначения
//	lW				Ширина DC
//	lH				Высота DC
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
//Оптимизированный фильтр "Серые тона" (hsilgos)

//Таблица предвычисленных значений серого для компонент 
struct CalculatedCoeffs
{
	enum 
	{
		ColorCount	= 256,
		red_coeff   = 6968,
		green_coeff = 23434,
		blue_coeff	= 32768 - red_coeff - green_coeff
	};

	int r[ColorCount];
	int g[ColorCount];
	int b[ColorCount];

	CalculatedCoeffs()
	{
		for( int i = 0 ; i < ColorCount; ++i )
		{
			r[i] = i * red_coeff;
			g[i] = i * green_coeff;
			b[i] = i * blue_coeff;
		}
	}

	unsigned char get(unsigned char ar, unsigned char ag, unsigned char ab) const
	{
		return (unsigned char)((r[ar] + g[ag] + b[ab]) >> 15);
	}
};

class GrayScaleFast: public IAlgorithm
{

	virtual void processImage(LPBITMAPINFO pBMI, LPBYTE pPixels, ULONG lBytesCnt, const RECT &pRC)
	{
		BYTE r, g, b, s;

		static const CalculatedCoeffs precalc;

		ULONG lStep = pBMI->bmiHeader.biBitCount / 8;
		for (ULONG i = 0; i < lBytesCnt; i+= lStep)
		{

			r = pPixels[i];
			g = pPixels[i + 1];
			b = pPixels[i + 2];

			s = precalc.get(r, g, b);

			pPixels[i] = s;
			pPixels[i + 1] = s;
			pPixels[i + 2] = s;

			progressEvent(i, lBytesCnt);
		}
	}
public:

};


AUTO_REGISTER_ALGORITHM( L"Filters|GrayScale|Slow",  GrayScale );
AUTO_REGISTER_ALGORITHM( L"Filters|GrayScale|Fast",  GrayScaleFast );

