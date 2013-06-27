#include "StdAfx.h"

#include "EdgeDetection.h"

//������ "����� ������"
//�������:
//  -1 -1 -1
//  -1  8 -1
//  -1 -1 -1 / 1 (����� �������������) [ ����������� + color ]
//���������:
//	hDC				DC ����������
//	lW				������ DC
//	lH				������ DC
//	crBkColor		����, ������� ����� �������������� ��� ���� ����
//	pRC				��������� �� ��������� RECT, ������������ ������� ����������� ��� ���������
//	hWndCallback	���� ����������� � ���� ������ (�����������)
//������������ ��������: TRUE � ������ ������, FALSE � ������ ������
BOOL EdgeDetection(HDC hDC, ULONG lW, ULONG lH, COLORREF crBkColor, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels1 = NULL, pPixels2 = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	ULONG lColor[9], lR, lG, lB;
	LONG i, j, x, y;

	volatile ONPROGRESSPARAMS ONPP = {};

	if (!GetImagePixels(hDC, lW, lH, &pPixels1, &lBytesCnt, &pBMI)) {
		if (pPixels1)
			delete[] pPixels1;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}
	pPixels2 = new BYTE[lBytesCnt];
	memcpy(pPixels2, pPixels1, lBytesCnt);

	for (j = pRC->top; j < pRC->bottom; j++)
	{
		for (i = pRC->left; i < pRC->right; i++)
		{
			x = (i == pRC->left)?pRC->left + 1:i;
			x = (i < (pRC->right - 1))?x:i - 1;
			y = (j == pRC->top)?pRC->top + 1:j;
			y = (j < (pRC->bottom - 1))?y:j - 1;

			lColor[0] = GetPixel(pPixels2, pBMI, x, y);
			lColor[1] = GetPixel(pPixels2, pBMI, x, y - 1);
			lColor[2] = GetPixel(pPixels2, pBMI, x + 1, y - 1);
			lColor[3] = GetPixel(pPixels2, pBMI, x + 1, y);
			lColor[4] = GetPixel(pPixels2, pBMI, x + 1, y + 1);
			lColor[5] = GetPixel(pPixels2, pBMI, x, y + 1);
			lColor[6] = GetPixel(pPixels2, pBMI, x - 1, y + 1);
			lColor[7] = GetPixel(pPixels2, pBMI, x - 1, y);
			lColor[8] = GetPixel(pPixels2, pBMI, x - 1, y - 1);

			lR = (-1 * R_BGRA(lColor[1]) - 1 * R_BGRA(lColor[2]) - 1 * R_BGRA(lColor[3]) - 1 * R_BGRA(lColor[4])
				 - 1 * R_BGRA(lColor[5]) - 1 * R_BGRA(lColor[6]) - 1 * R_BGRA(lColor[7]) - 1 * R_BGRA(lColor[8])
				 + (8 * R_BGRA(lColor[0]))) / 1;// + GetRValue(crBkColor);
			lG = (-1 * G_BGRA(lColor[1]) - 1 * G_BGRA(lColor[2]) - 1 * G_BGRA(lColor[3]) - 1 * G_BGRA(lColor[4])
				 - 1 * G_BGRA(lColor[5]) - 1 * G_BGRA(lColor[6]) - 1 * G_BGRA(lColor[7]) - 1 * G_BGRA(lColor[8])
				 + (8 * G_BGRA(lColor[0]))) / 1;// + GetGValue(crBkColor);
			lB = (-1 * B_BGRA(lColor[1]) - 1 * B_BGRA(lColor[2]) - 1 * B_BGRA(lColor[3]) - 1 * B_BGRA(lColor[4])
				 - 1 * B_BGRA(lColor[5]) - 1 * B_BGRA(lColor[6]) - 1 * B_BGRA(lColor[7]) - 1 * B_BGRA(lColor[8])
				 + (8 * B_BGRA(lColor[0]))) / 1;// + GetBValue(crBkColor);

			lR = (ULONG)CheckBounds((LONG)lR, (LONG)0, (LONG)255);
			lG = (ULONG)CheckBounds((LONG)lG, (LONG)0, (LONG)255);
			lB = (ULONG)CheckBounds((LONG)lB, (LONG)0, (LONG)255);

			SetPixel(pPixels1, pBMI, i, j, BGR(lB, lG, lR));
		}
		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)j / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
	}

	SetImagePixels(hDC, lW, lH, pPixels1, pBMI);

	delete[] pPixels1;
	delete[] pPixels2;
	delete pBMI;

	return TRUE;
}
