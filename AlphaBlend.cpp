#include "StdAfx.h"

#ifdef __USE_GDIPLUS__

//#include "AlphaBlend.h"

#include "IAlgorithm.h"
#include "AlgorithmFactory.h"

#include "Ladybug_png.h"

#include <memory>
#include <functional>

namespace
{
	void ComRelease(IUnknown *aIntf)
	{
		if( aIntf )
			aIntf->Release();
	}
}

typedef enum tagALPHAMODE
{
	AM_ALPHA_IGNORE = 0,		//Использование только альфа-компоненты пикселя
	AM_ALPHA_ADD,				//Добавление значения альфы к альфа-компоненте пикселя
	AM_ALPHA_SUBTRACT,			//Вычитание значения альфы из альфа-компоненты пикселя
	AM_ALPHA_REPLACE			//Замена альфа-компоненты пикселя на указанную альфу
} ALPHAMODE;

class AlphaBlend2: public IAlgorithm
{
	virtual void processImage(LPIMAGEDESCR pIMGDESCR, LPBYTE pPixels, ULONG lBytesCnt, const RECT &pRC){}

	virtual bool process(HDC hDC, const RECT &rcPicture, const RECT &rcCanvas, HWND hWndCallback)
	{
		// memory...
		IStream *spStream = 0;

		if( !SUCCEEDED(::CreateStreamOnHGlobal(0, TRUE, &spStream)) )
			return false;

		std::unique_ptr<IStream, std::function<void(IUnknown *)>> tStream(spStream, ComRelease);

		if( !SUCCEEDED(spStream->Write(get_Ladybug_png_buf(), get_Ladybug_png_size(), 0) ) )
			return false;
		//

		Image pImage(tStream.get());
		//

		HDC hTmpDC;
		HBITMAP hTmpBmp, hOldBmp;
		ULONG lW;
		ULONG lH;
	
		if (pImage.GetLastStatus() == Ok)
		{
			lW = pImage.GetWidth();
			lH = pImage.GetHeight();

			hTmpDC = CreateCompatibleDC(hDC);
			hTmpBmp = CreateCompatibleBitmap(hDC, lW, lH);
			hOldBmp = (HBITMAP)SelectObject(hTmpDC, hTmpBmp);

			Graphics pGraphics(hTmpDC);
			pGraphics.DrawImage(&pImage, Rect(0, 0, lW, lH));

			RECT rcCanvas2 = {(rcCanvas.right - (lW * 2)) >> 1, (rcCanvas.bottom - (lH * 2)) >> 1, (rcCanvas.right + (lW * 2)) >> 1,
				(rcCanvas.bottom + (lH * 2)) >> 1};
			DoAlphaBlend(hDC, rcPicture.right, rcPicture.bottom, hTmpDC, lW, lH, 128, AM_ALPHA_IGNORE, &rcCanvas2, hWndCallback);

			DeleteObject(SelectObject(hTmpDC, hOldBmp));
			DeleteDC(hTmpDC);
		}

		return true;
	}


	//Фильтр "Наложение по альфе"
	//Параметры:
	//	hDstDC			DC назначения
	//	lDstW			Ширина выводимого растра
	//	lDstH			Высота выводимого растра
	//	hSrcDC			DC источника
	//	lSrcW			Ширина растра источника
	//	lSrcH			Высота растра источника
	//	bAlpha			Уровень полупрозначности для накладываемого растра (не учитывается, если задан режим AM_ALPHA_IGNORE)
	//	AlphaMode		Режим работы с альфа-каналом
	//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
	//	hWndCallback	Окно уведомлений о ходе работы (опционально)
	//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки

	BOOL DoAlphaBlend(HDC hDstDC, ULONG lDstW, ULONG lDstH, HDC hSrcDC, ULONG lSrcW, ULONG lSrcH, BYTE bAlpha, ALPHAMODE AlphaMode, LPRECT pRC, HWND hWndCallback)
	{
		ULONG lSrcBytesCnt = 0, lDstBytesCnt = 0;
		ULONG lSrcC, lDstC, lR, lG, lB;
		LONG i, j;

		IMAGEDESCR srcIMGDESCR = {0}, dstIMGDESCR = {0};

		std::vector<BYTE> dstPixels;
		//Получаем пиксели конечного изображения
		if (!GetImagePixels(hDstDC, lDstW, lDstH, dstPixels, dstIMGDESCR))
			return FALSE;

		std::vector<BYTE> srcPixels;

		//Получаем пиксели исходного изображения
		if (!GetImagePixels(hSrcDC, lSrcW, lSrcH, srcPixels, srcIMGDESCR))
			return FALSE;

		//Не работаем с изображениями ниже 24 бит на пиксель
		if ((dstIMGDESCR.cBitCount < 24) || (srcIMGDESCR.cBitCount < 24))
			return FALSE;

		//Если размеры исходного изображения отличаются от заданной области применения фильтра, выполняем мастабирование исходного изображения
		if ((lSrcW != (pRC->right - pRC->left)) || (lSrcH != (pRC->bottom - pRC->top)))
		{
			//LPBYTE pTmpPixels;
			std::vector<BYTE> tmpPixels;
			IMAGEDESCR pTmpBMI = {0};
			ResampleImagePixels(srcPixels, srcIMGDESCR, tmpPixels, pTmpBMI, (pRC->right - pRC->left), (pRC->bottom - pRC->top));
			srcPixels.swap(tmpPixels);

			srcIMGDESCR = pTmpBMI;
		}

		for (j = pRC->top; j < pRC->bottom; j++)
		{
			for (i = pRC->left; i < pRC->right; i++)
			{
				lDstC = GetPixel(dstPixels, &dstIMGDESCR, i, j);
				lSrcC = GetPixel(srcPixels, &srcIMGDESCR, i - pRC->left, j - pRC->top);

				switch (AlphaMode)
				{
					case AM_ALPHA_IGNORE:
						lR = (R_BGRA(lDstC) * (255 - A_BGRA(lSrcC)) + R_BGRA(lSrcC) * A_BGRA(lSrcC)) / 255;
						lG = (G_BGRA(lDstC) * (255 - A_BGRA(lSrcC)) + G_BGRA(lSrcC) * A_BGRA(lSrcC)) / 255;
						lB = (B_BGRA(lDstC) * (255 - A_BGRA(lSrcC)) + B_BGRA(lSrcC) * A_BGRA(lSrcC)) / 255;
						break;
					case AM_ALPHA_ADD:
						lR = (R_BGRA(lDstC) * (255 - min(255, (A_BGRA(lSrcC) + bAlpha))) + R_BGRA(lSrcC) * min(255, (A_BGRA(lSrcC) + bAlpha))) / 255;
						lG = (G_BGRA(lDstC) * (255 - min(255, (A_BGRA(lSrcC) + bAlpha))) + G_BGRA(lSrcC) * min(255, (A_BGRA(lSrcC) + bAlpha))) / 255;
						lB = (B_BGRA(lDstC) * (255 - min(255, (A_BGRA(lSrcC) + bAlpha))) + B_BGRA(lSrcC) * min(255, (A_BGRA(lSrcC) + bAlpha))) / 255;
						break;
					case AM_ALPHA_SUBTRACT:
						lR = (R_BGRA(lDstC) * (255 - max(0, (A_BGRA(lSrcC) - bAlpha))) + R_BGRA(lSrcC) * max(0, (A_BGRA(lSrcC) - bAlpha))) / 255;
						lG = (G_BGRA(lDstC) * (255 - max(0, (A_BGRA(lSrcC) - bAlpha))) + G_BGRA(lSrcC) * max(0, (A_BGRA(lSrcC) - bAlpha))) / 255;
						lB = (B_BGRA(lDstC) * (255 - max(0, (A_BGRA(lSrcC) - bAlpha))) + B_BGRA(lSrcC) * max(0, (A_BGRA(lSrcC) - bAlpha))) / 255;
						break;
					case AM_ALPHA_REPLACE:
						lR = (R_BGRA(lDstC) * (255 - bAlpha) + R_BGRA(lSrcC) * bAlpha) / 255;
						lG = (G_BGRA(lDstC) * (255 - bAlpha) + G_BGRA(lSrcC) * bAlpha) / 255;
						lB = (B_BGRA(lDstC) * (255 - bAlpha) + B_BGRA(lSrcC) * bAlpha) / 255;
						break;
				}

				SetPixel(&dstPixels[0], &dstIMGDESCR, i, j, BGRA(lB, lG, lR, A_BGRA(lDstC)));
			}

			progressEvent(j, pRC->bottom);
		}

		SetImagePixels(hDstDC, lDstW, lDstH, &dstPixels[0], &dstIMGDESCR);

		return TRUE;
	}
};

AUTO_REGISTER_ALGORITHM( L"Filters|Alpha Blend",  AlphaBlend2 );

#endif


