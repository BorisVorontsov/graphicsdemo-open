#include "StdAfx.h"

#include "AlphaBlend.h"

#include "IAlgorithm.h"
#include "AlgorithmFactory.h"

//������ "��������� �� �����"
//���������:
//	hDstDC			DC ����������
//	lDstW			������ ���������� ������
//	lDstH			������ ���������� ������
//	hSrcDC			DC ���������
//	lSrcW			������ ������ ���������
//	lSrcH			������ ������ ���������
//	bAlpha			������� ���������������� ��� �������������� ������ (�� �����������, ���� ����� ����� AM_ALPHA_IGNORE)
//	AlphaMode		����� ������ � �����-�������
//	pRC				��������� �� ��������� RECT, ������������ ������� ����������� ��� ���������
//	hWndCallback	���� ����������� � ���� ������ (�����������)
//������������ ��������: TRUE � ������ ������, FALSE � ������ ������

BOOL AlphaBlend(HDC hDstDC, ULONG lDstW, ULONG lDstH, HDC hSrcDC, ULONG lSrcW, ULONG lSrcH, BYTE bAlpha, ALPHAMODE AlphaMode, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pSrcPixels = NULL, pDstPixels = NULL;
	ULONG lSrcBytesCnt = 0, lDstBytesCnt = 0;
	LPBITMAPINFO pSrcBMI = NULL, pDstBMI = NULL;
	ULONG lSrcC, lDstC, lR, lG, lB;
	LONG i, j;
	BOOL bResult;

	volatile ONPROGRESSPARAMS ONPP = {0};

	//�������� ������� ��������� �����������
	if (!GetImagePixels(hDstDC, lDstW, lDstH, &pDstPixels, &lDstBytesCnt, &pDstBMI)) {
		bResult = FALSE;
		goto AB_Exit;
	}

	//�������� ������� ��������� �����������
	if (!GetImagePixels(hSrcDC, lSrcW, lSrcH, &pSrcPixels, &lSrcBytesCnt, &pSrcBMI)) {
		bResult = FALSE;
		goto AB_Exit;
	}

	//�� �������� � ������������� ���� 24 ��� �� �������
	if ((pDstBMI->bmiHeader.biBitCount < 24) || (pSrcBMI->bmiHeader.biBitCount < 24)) {
		bResult = FALSE;
		goto AB_Exit;
	}

	//���� ������� ��������� ����������� ���������� �� �������� ������� ���������� �������, ��������� �������������� ��������� �����������
	if ((lSrcW != (pRC->right - pRC->left)) || (lSrcH != (pRC->bottom - pRC->top)))
	{
		LPBYTE pTmpPixels;
		LPBITMAPINFO pTmpBMI;
		ResampleImagePixels(pSrcPixels, pSrcBMI, &pTmpPixels, &pTmpBMI, (pRC->right - pRC->left), (pRC->bottom - pRC->top));
		delete[] pSrcPixels;
		delete pSrcBMI;
		pSrcPixels = pTmpPixels;
		pSrcBMI = pTmpBMI;
		lSrcW = pSrcBMI->bmiHeader.biWidth;
		lSrcH = pSrcBMI->bmiHeader.biHeight;
	}

	for (j = pRC->top; j < pRC->bottom; j++)
	{
		for (i = pRC->left; i < pRC->right; i++)
		{
			lDstC = GetPixel(pDstPixels, pDstBMI, i, j);
			lSrcC = GetPixel(pSrcPixels, pSrcBMI, i - pRC->left, j - pRC->top);

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

			SetPixel(pDstPixels, pDstBMI, i, j, BGRA(lB, lG, lR, A_BGRA(lDstC)));
		}

		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)j / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
	}

	SetImagePixels(hDstDC, lDstW, lDstH, pDstPixels, pDstBMI);

	bResult = TRUE;

AB_Exit:

	if (pDstPixels)
		delete[] pDstPixels;
	if (pDstBMI)
		delete pDstBMI;

	if (pSrcPixels)
		delete[] pSrcPixels;
	if (pSrcBMI)
		delete pSrcBMI;

	return bResult;
}

//#ifdef __USE_GDIPLUS__
#if 0

class AlphaBlendWrapp: public IAlgorithm
{
	virtual void processImage(LPBITMAPINFO pBMI, LPBYTE pPixels, ULONG lBytesCnt, const RECT &pRC){}

	virtual bool process(HDC hDC, const RECT &rcPicture, const RECT &rcCanvas, HWND hWndCallback)
	{
		Graphics *pGraphics = NULL;
		Image *pImage = new Image(lpPic2Path);
		HDC hTmpDC;
		HBITMAP hTmpBmp, hOldBmp;
		ULONG lW;
		ULONG lH;
	
		if (pImage->GetLastStatus() == Ok)
		{
			lW = pImage->GetWidth();
			lH = pImage->GetHeight();

			hTmpDC = CreateCompatibleDC(hDC);
			hTmpBmp = CreateCompatibleBitmap(hDC, lW, lH);
			hOldBmp = (HBITMAP)SelectObject(hTmpDC, hTmpBmp);
			pGraphics = new Graphics(hTmpDC);
			pGraphics->DrawImage(pImage, Rect(0, 0, lW, lH));

			RECT rcCanvas2 = {(rcCanvas.right - (lW * 2)) >> 1, (rcCanvas.bottom - (lH * 2)) >> 1, (rcCanvas.right + (lW * 2)) >> 1,
				(rcCanvas.bottom + (lH * 2)) >> 1};
			AlphaBlend(hDC, rcPicture.right, rcPicture.bottom, hTmpDC, lW, lH, 128, AM_ALPHA_IGNORE, &rcCanvas2, hWndCallback);

			DeleteObject(SelectObject(hTmpDC, hOldBmp));
			DeleteDC(hTmpDC);

			delete pGraphics;
		}
		delete pImage;
	}
};

AUTO_REGISTER_ALGORITHM( L"Filters|Alpha Blend",  AlphaBlendWrapp);

#endif


