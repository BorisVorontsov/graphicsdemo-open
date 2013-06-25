#include "StdAfx.h"

#include "Shear.h"

//����� �����������
//���������:
//	hDC				DC ����������
//	lW				������ DC
//	lH				������ DC
//	lX				����� (������� ������� ����) �� ��� X
//	lY				����� (������� ������� ����) �� ��� Y
//	crBkColor		����, ������� ����� �������������� ��� ���� ����
//	pRC				��������� �� ��������� RECT, ������������ ������� ����������� ��� ���������
//	hWndCallback	���� ����������� � ���� ������ (�����������)
//������������ ��������: TRUE � ������ ������, FALSE � ������ ������
BOOL Shear(HDC hDC, ULONG lW, ULONG lH, LONG lX, LONG lY, COLORREF crBkColor, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels1 = NULL, pPixels2 = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	LONG i, j, x, x1, y, y1, sx, sy;

	volatile ONPROGRESSPARAMS ONPP = {0};

	if (!GetImagePixels(hDC, lW, lH, &pPixels1, &lBytesCnt, &pBMI)) {
		if (pPixels1)
			delete[] pPixels1;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}
	pPixels2 = new BYTE[lBytesCnt];
	memcpy(pPixels2, pPixels1, lBytesCnt);

	//RGB[A] -> BGR[A]
	ReverseBytes((LPBYTE)&crBkColor, 3);

	//�������� ������� ��� ��������� ������ ����
	for (j = pRC->top; j < pRC->bottom; j++)
	{
		for (i = pRC->left; i < pRC->right; i++)
		{
			SetPixel(pPixels1, pBMI, i, j, crBkColor);
		}
	}

	//��������� ���� ��� x � y
	if ((lX != 0)) sx = ((pRC->bottom - pRC->top) / abs(lX));
	if ((lY != 0)) sy = ((pRC->right - pRC->left) / abs(lY));

	x1 = 0;
	for (j = pRC->top; j < pRC->bottom; j++)
	{
		if ((lX != 0))
			if ((j % sx) == 0) x1++;
		y1 = 0;
		for (i = pRC->left; i < pRC->right; i++)
		{
			if ((lY != 0))
				if ((i % sy) == 0) y1++;

			if (lX != 0)
			{
				x = (lX > 0)?i - x1:i + x1;
			}
			else x = i;
			if (lY != 0)
			{
				y = (lY > 0)?j - y1:j + y1;
			}
			else y = j;

			if ((x < pRC->left) || (x > (pRC->right - 1))) continue;
			if ((y < pRC->top) || (y > (pRC->bottom - 1))) continue;

			SetPixel(pPixels1, pBMI, i, j, GetPixel(pPixels2, pBMI, x, y));
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
