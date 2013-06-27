#include "StdAfx.h"

#include "Rotate.h"

//������� ����������� �� ������������ ����
//�������:
//	x' = cos(rad) * (x - x0) +/- sin(rad) * (y - y0) + x0
//	y' = cos(rad) * (y - y0) -/+ sin(rad) * (x - x0) + y0
//���������:
//	hDC				DC ����������
//	lW				������ DC
//	lH				������ DC
//	lX				����� �������� �� ��� X
//	lY				����� �������� �� ��� Y
//	lAngle			���� ��������
//	lDirection		����������� (�� ������� �������/������ ������� �������)
//	crBkColor		����, ������� ����� �������������� ��� ���� ����
//	pRC				��������� �� ��������� RECT, ������������ ������� ����������� ��� ���������
//	hWndCallback	���� ����������� � ���� ������ (�����������)
//������������ ��������: TRUE � ������ ������, FALSE � ������ ������
BOOL Rotate(HDC hDC, ULONG lW, ULONG lH, LONG lX, LONG lY, LONG lAngle, LONG lDirection, COLORREF crBkColor, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels1 = NULL, pPixels2 = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	LONG i, j, x, y;
	double dblRad;

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

	dblRad = (double)(lAngle * (3.14159265358979 / 180));

	lX += pRC->left;
	lY += pRC->top;

	ReverseBytes((LPBYTE)&crBkColor, 3);

	for (j = pRC->top; j < pRC->bottom; j++)
	{
		for (i = pRC->left; i < pRC->right; i++)
		{
			SetPixel(pPixels1, pBMI, i, j, crBkColor);
		}
	}

	for (j = pRC->top; j < pRC->bottom; j++)
	{
		for (i = pRC->left; i < pRC->right; i++)
		{
			switch (lDirection)
			{
				case ROTATE_DIRECTION_CW:
					x = (LONG)(cos(dblRad) * (i - lX) + sin(dblRad) * (j - lY) + lX);
					y = (LONG)(cos(dblRad) * (j - lY) - sin(dblRad) * (i - lX) + lY);
					break;
				case ROTATE_DIRECTION_CCW:
					x = (LONG)(cos(dblRad) * (i - lX) - sin(dblRad) * (j - lY) + lX);
					y = (LONG)(cos(dblRad) * (j - lY) + sin(dblRad) * (i - lX) + lY);
					break;
				default:
					//
					break;
			}

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
