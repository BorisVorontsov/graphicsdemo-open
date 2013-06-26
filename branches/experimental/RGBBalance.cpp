#include "StdAfx.h"

#include "IAlgorithm.h"
#include "AlgorithmFactory.h"

//Фильтр "RGB-баланс"
//Параметры:
//	hDC				DC назначения
//	lW				Ширина DC
//	lH				Высота DC
//	lROffset		Степень смещения красного. Допустимые значения: от 0 до [-]255
//	lGOffset		Степень смещения зеленого. Допустимые значения: от 0 до [-]255
//	lBOffset		Степень смещения синего. Допустимые значения: от 0 до [-]255
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//	hWndCallback	Окно уведомлений о ходе работы (опционально)
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки


class RGBBalance: public IAlgorithm
{
	LONG mROffset;
	LONG mGOffset;
	LONG mBOffset;

	virtual void processImage(LPBITMAPINFO pBMI, LPBYTE pPixels, ULONG lBytesCnt, const RECT &pRC)
	{
		ULONG lColor, lR, lG, lB;
		LONG i, j;

		for (j = pRC.top; j < pRC.bottom; j++)
		{
			for (i = pRC.left; i < pRC.right; i++)
			{
				lColor = GetPixel(pPixels, pBMI, i, j);

				lR = R_BGRA(lColor);
				lG = G_BGRA(lColor);
				lB = B_BGRA(lColor);

				lR = (ULONG)CheckBounds((LONG)(lR += mROffset), (LONG)0, (LONG)255);
				lG = (ULONG)CheckBounds((LONG)(lG += mGOffset), (LONG)0, (LONG)255);
				lB = (ULONG)CheckBounds((LONG)(lB += mBOffset), (LONG)0, (LONG)255);

				SetPixel(pPixels, pBMI, i, j, BGR(lB, lG, lR));
			}

			progressEvent(j, pRC.bottom);
		}
	}
public:
	RGBBalance(LONG aROffset, LONG aGOffset, LONG aBOffset)
		:mROffset(aROffset), mGOffset(aGOffset), mBOffset(aBOffset)
	{
	}
};


AUTO_REGISTER_ALGORITHM3(L"Filters|RGB Balance", RGBBalance, -12, 4, -6);
