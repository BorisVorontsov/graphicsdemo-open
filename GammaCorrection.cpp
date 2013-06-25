#include "StdAfx.h"

#include "IAlgorithm.h"
#include "AlgorithmFactory.h"

//Фильтр "Гамма-коррекция"
//Параметры:
//	hDC				DC назначения
//	lW				Ширина DC
//	lH				Высота DC
//	dblGamma		Степень гаммы. Допустимые значения: от 0.1 до 1.9
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки

class GammaCorrection: public IAlgorithm
{
	double mdblGamma;

	virtual void processImage(LPBITMAPINFO pBMI, LPBYTE pPixels, ULONG lBytesCnt, LPRECT pRC)
	{
		ULONG lColor, lR, lG, lB;
		LONG i, j;

		for (j = pRC->top; j < pRC->bottom; j++)
		{
			for (i = pRC->left; i < pRC->right; i++)
			{
				lColor = GetPixel(pPixels, pBMI, i, j);

				lR = R_BGRA(lColor);
				lG = G_BGRA(lColor);
				lB = B_BGRA(lColor);

				lR = (ULONG)CheckBounds((LONG)((255.0 * pow((double)lR / 255.0, 1.0 / mdblGamma))
					+ 0.5), (LONG)0, (LONG)255);
				lG = (ULONG)CheckBounds((LONG)((255.0 * pow((double)lG / 255.0, 1.0 / mdblGamma))
					+ 0.5), (LONG)0, (LONG)255);
				lB = (ULONG)CheckBounds((LONG)((255.0 * pow((double)lB / 255.0, 1.0 / mdblGamma))
					+ 0.5), (LONG)0, (LONG)255);

				SetPixel(pPixels, pBMI, i, j, BGR(lB, lG, lR));
			}
			progressEvent(j, pRC->bottom);
		}
	}
public:
	GammaCorrection(double dblGamma)
		:mdblGamma(dblGamma)
	{
	}
};


AUTO_REGISTER_ALGORITHM1( L"Filters|Gamma Correction|0.1",  GammaCorrection, 0.1 );
AUTO_REGISTER_ALGORITHM1( L"Filters|Gamma Correction|0.5",  GammaCorrection, 0.5 );
AUTO_REGISTER_ALGORITHM1( L"Filters|Gamma Correction|1.0",  GammaCorrection, 1.0 );
AUTO_REGISTER_ALGORITHM1( L"Filters|Gamma Correction|1.5",  GammaCorrection, 1.5 );
AUTO_REGISTER_ALGORITHM1( L"Filters|Gamma Correction|1.9",  GammaCorrection, 1.9 );
