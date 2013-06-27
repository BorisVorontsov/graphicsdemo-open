#include "StdAfx.h"

#include "IAlgorithm.h"
#include "AlgorithmFactory.h"
//Шаблон для новых фильтров


class EmptyFilter: public IAlgorithm
{
	virtual void processImage(LPIMAGEDESCR pIMGDESCR, LPBYTE pPixels, ULONG lBytesCnt, LPRECT pRC)
	{
		ULONG lColor, lA, lR, lG, lB;
		LONG i, j;

		//Тело фильтра
		for (j = pRC->top; j < pRC->bottom; j++)
		{
			for (i = pRC->left; i < pRC->right; i++)
			{
				lColor = GetPixel(pPixels, pIMGDESCR, i, j);

				lA = A_BGRA(lColor);
				lR = R_BGRA(lColor);
				lG = G_BGRA(lColor);
				lB = B_BGRA(lColor);

				//...

				SetPixel(pPixels, pIMGDESCR, i, j, BGRA(lB, lG, lR, lA));
			}
			//Отображение статуса выполнения (опциональный блок)
			progressEvent(j, pRC->bottom);
		}
		//-----------------------------------------------------------------------------------------------
	}
public:
	EmptyFilter(/*...дополнительные параметры..., */)
	{
	}
};


//AUTO_REGISTER_ALGORITHM( L"menu|path",  EmptyFilter );
//AUTO_REGISTER_ALGORITHM1( L"menu|path",  EmptyFilter, arg1 );
//AUTO_REGISTER_ALGORITHM2( L"menu|path",  EmptyFilter, arg1, arg2 );
