#include "StdAfx.h"

#include "IAlgorithm.h"
#include "AlgorithmFactory.h"

//������ "RGB-������"
//���������:
//	hDC				DC ����������
//	lW				������ DC
//	lH				������ DC
//	lROffset		������� �������� ��������. ���������� ��������: �� 0 �� [-]255
//	lGOffset		������� �������� ��������. ���������� ��������: �� 0 �� [-]255
//	lBOffset		������� �������� ������. ���������� ��������: �� 0 �� [-]255
//	pRC				��������� �� ��������� RECT, ������������ ������� ����������� ��� ���������
//	hWndCallback	���� ����������� � ���� ������ (�����������)
//������������ ��������: TRUE � ������ ������, FALSE � ������ ������


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
