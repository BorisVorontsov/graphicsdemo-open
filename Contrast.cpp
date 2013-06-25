#include "StdAfx.h"

#include "IAlgorithm.h"
#include "AlgorithmFactory.h"
//Фильтр "Контрастность"
//Параметры:
//  hDC             DC назначения
//  lOffset         Степень изменения контрастности. Допустимые значения: от -100 до 100
//  pRC             Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
class Contrast: public IAlgorithm
{
	LONG mOffset;

	virtual void processImage(LPBITMAPINFO pBMI, LPBYTE pPixels, ULONG lBytesCnt, LPRECT pRC)
	{
		ULONG lAB = 0;
		ULONG lColor;
		LONG i, j, lR, lG, lB, lLevel = abs(mOffset);

		for (j = pRC->top; j < pRC->bottom; j++) {
			for (i = pRC->left; i < pRC->right; i++) {
				lColor = GetPixel(pPixels, pBMI, i, j);

				lR = R_BGRA(lColor);
				lG = G_BGRA(lColor);
				lB = B_BGRA(lColor);

				lAB += (ULONG)(lR * 0.299 + lG * 0.587 + lB * 0.114);
			}
		}

		lAB /= ((pRC->right - pRC->left) * (pRC->bottom - pRC->top));

		for (j = pRC->top; j < pRC->bottom; j++)
		{
			for (i = pRC->left; i < pRC->right; i++)
			{
				lColor = GetPixel(pPixels, pBMI, i, j);

				lR = R_BGRA(lColor);
				lG = G_BGRA(lColor);
				lB = B_BGRA(lColor);

				lR = (ULONG)CheckBounds((LONG)(lR + ((lR - /*127.0*/(LONG)lAB) * ((double)((mOffset < 0)?-lLevel:lLevel) /
					(double)(201 - (-lLevel + 100))))), (LONG)0, (LONG)255);
				lG = (ULONG)CheckBounds((LONG)(lG + ((lG - /*127.0*/(LONG)lAB) * ((double)((mOffset < 0)?-lLevel:lLevel) /
					(double)(201 - (-lLevel + 100))))), (LONG)0, (LONG)255);
				lB = (ULONG)CheckBounds((LONG)(lB + ((lB - /*127.0*/(LONG)lAB) * ((double)((mOffset < 0)?-lLevel:lLevel) /
					(double)(201 - (-lLevel + 100))))), (LONG)0, (LONG)255);

				SetPixel(pPixels, pBMI, i, j, BGR(lB, lG, lR));
			}

			progressEvent(j, pRC->bottom);
		}
	}
public:
	Contrast(LONG aOffset)
		:mOffset(aOffset)
	{
	}
};


AUTO_REGISTER_ALGORITHM1(L"Filters|Contrast|-100", Contrast, -100);
AUTO_REGISTER_ALGORITHM1(L"Filters|Contrast|-50", Contrast, -50);
AUTO_REGISTER_ALGORITHM1(L"Filters|Contrast|-32", Contrast, -32);
AUTO_REGISTER_ALGORITHM1(L"Filters|Contrast|32", Contrast, 32);
AUTO_REGISTER_ALGORITHM1(L"Filters|Contrast|50", Contrast, 50);
AUTO_REGISTER_ALGORITHM1(L"Filters|Contrast|100", Contrast, 100);

