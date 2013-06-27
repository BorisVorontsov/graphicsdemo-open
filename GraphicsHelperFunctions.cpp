//////////////////////////////////////////////////////////////////////
//		������� ��� ������ Graphics (Graphics Helper Functions)
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "GraphicsHelperFunctions.h"

//���������� ��������������� �������
//���������:
//	hDC				DC � ������������
//	lW				������ DC/�����������
//	lH				������ DC/�����������
//	ppPixels		��������� �� ����������, ����������� ������ ��������
//	pBytesCnt		��������� �� ����������, ����������� ������ (� ������) �������
//	ppIMGDESCR		��������� �� ����������, ����������� ��������� IMAGEDESCR, � ����������� �� �����������
//�����: ������ ppPixels � ��������� ppIMGDESCR ���������� �������� ����������� new, � ���������� ������������ ��
//������������ ��������, ����������� ������ delete[] ��� ppPixels � delete ��� ppIMGDESCR
//������������ ��������: ���������� ���������� ����� ������ � ������ ������, 0 � ������ ������
int GetImagePixels(HDC hDC, ULONG lW, ULONG lH, LPBYTE *ppPixels, ULONG *pBytesCnt, LPIMAGEDESCR *ppIMGDESCR)
{
	HDC hTmpDC;
	HBITMAP hOldBitmap, hTmpBitmap;
	BITMAPINFO BMI = {};
	int intScanLines;

	USHORT BPP = GetDeviceCaps(hDC, BITSPIXEL);

	//�� �������� � ������������� ���� 24 bpp (TrueColor)
	if ((BPP != 24) && (BPP != 32)) return 0;

	switch (BPP)
	{
		case 24:
			*pBytesCnt = (((BPP / 8) * lW + 3) / 4) * 4;
			*pBytesCnt += ((*pBytesCnt * lH + 3) / 4) * 4;
			break;
		case 32:
			*pBytesCnt = lW * lH * (BPP / 8);
			break;
	}

	*ppPixels = new BYTE[*pBytesCnt];

	hTmpDC = CreateCompatibleDC(hDC);
	hTmpBitmap = CreateCompatibleBitmap(hDC, lW, lH);
	hOldBitmap = (HBITMAP)SelectObject(hTmpDC, hTmpBitmap);
	BitBlt(hTmpDC, 0, 0, lW, lH, hDC, 0, 0, SRCCOPY);

	BMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BMI.bmiHeader.biWidth = lW;
	BMI.bmiHeader.biHeight = lH;
	BMI.bmiHeader.biPlanes = 1;
	BMI.bmiHeader.biBitCount = BPP;
	BMI.bmiHeader.biCompression = BI_RGB;

	intScanLines = GetDIBits(hTmpDC, hTmpBitmap, 0, lH, *ppPixels, &BMI, DIB_RGB_COLORS);

	*ppIMGDESCR = new IMAGEDESCR;
	ZeroMemory(*ppIMGDESCR, sizeof(IMAGEDESCR));

	((LPIMAGEDESCR)*ppIMGDESCR)->width = BMI.bmiHeader.biWidth;
	((LPIMAGEDESCR)*ppIMGDESCR)->height = BMI.bmiHeader.biHeight;
	((LPIMAGEDESCR)*ppIMGDESCR)->cBitCount = (char)BMI.bmiHeader.biBitCount;

	SelectObject(hTmpDC, hOldBitmap);
	DeleteObject(hTmpBitmap);
	DeleteDC(hTmpDC);

	return intScanLines;
}

//���������� ��������������� �������
//���������:
//	hDC				DC � ������������
//	lW				������ DC/�����������
//	lH				������ DC/�����������
//	pPixels			������ ��������
//	pIMGDESCR		��������� �� ��������� IMAGEDESCR, � ����������� �� �����������
//������������ ��������: ���������� ����������� ����� ������ � ������ ������, 0 � ������ ������
int SetImagePixels(HDC hDC, ULONG lW, ULONG lH, LPCBYTE pPixels, LPIMAGEDESCR pIMGDESCR)
{
	int intScanLines;
	BITMAPINFO BMI = {};

	if ((pIMGDESCR->cBitCount != 24) && (pIMGDESCR->cBitCount != 32)) return 0;

	BMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BMI.bmiHeader.biWidth = pIMGDESCR->width;
	BMI.bmiHeader.biHeight = pIMGDESCR->height;
	BMI.bmiHeader.biPlanes = 1;
	BMI.bmiHeader.biBitCount = pIMGDESCR->cBitCount;
	BMI.bmiHeader.biCompression = BI_RGB;

	intScanLines = StretchDIBits(hDC, 0, 0, lW, lH, 0, 0, lW, lH, pPixels, &BMI, DIB_RGB_COLORS, SRCCOPY);

	return intScanLines;
}

//���������� ��������������� �������
//���������:
//	pPixels			��������� �� ������ ��������
//	pIMGDESCR		��������� �� ��������� IMAGEDESCR, ��������� � �������� ��������
//	x				x-���������� �������
//	y				y-���������� �������
//������������ ��������: ������� �� �������� ����������� � BGR[A] � ������ ������,
//GP_INVALIDPIXEL � ������ ������
COLORREF GetPixel(LPCBYTE pPixels, LPIMAGEDESCR pIMGDESCR, LONG x, LONG y)
{
	if ((x < 0) || (x >= pIMGDESCR->width)) return GP_INVALIDPIXEL;
	if ((y < 0) || (y >= pIMGDESCR->height)) return GP_INVALIDPIXEL;

	LPBYTE pPixel;
	LONG lBPS, lDelta;
	ULONG lResult = GP_INVALIDPIXEL;
	lBPS = (pIMGDESCR->width * (pIMGDESCR->cBitCount >> 3));
	if (pIMGDESCR->height < 0)
	{
		pPixel = (LPBYTE)pPixels;
		lDelta = lBPS;
	}
	else
	{
		lDelta = -lBPS;
		pPixel = (LPBYTE)pPixels + (pIMGDESCR->height - 1) * lBPS;
	}
	pPixel += y * lDelta;
	switch (pIMGDESCR->cBitCount)
	{
		case 24:
			pPixel += x * 3;
			lResult = BGR(pPixel[0], pPixel[1], pPixel[2]);
			break;
		case 32:
			lResult = ((ULONG *)pPixel)[x];
			break;
	}

	return lResult;
}

//���������� ��������������� �������
//���������:
//	pPixels			��������� �� ������ ��������
//	pIMGDESCR		��������� �� ��������� IMAGEDESCR, ��������� � �������� ��������
//	x				x-���������� �������
//	y				y-���������� �������
//	crValue			�������� ������� � BGR[A]
//������� �� ���������� ��������
void SetPixel(LPBYTE pPixels, LPIMAGEDESCR pIMGDESCR, LONG x, LONG y, COLORREF crValue)
{
	if ((x < 0) || (x >= pIMGDESCR->width)) return;
	if ((y < 0) || (y >= pIMGDESCR->height)) return;

	LPBYTE pPixel;
	LONG lBPS, lDelta;
	lBPS = (pIMGDESCR->width * (pIMGDESCR->cBitCount >> 3));
	if (pIMGDESCR->height < 0)
	{
		pPixel = pPixels;
		lDelta = lBPS;
	}
	else
	{
		lDelta = -lBPS;
		pPixel = pPixels + (pIMGDESCR->height - 1) * lBPS;
	}
	pPixel += y * lDelta;
	switch (pIMGDESCR->cBitCount)
	{
		case 24:
			((RGBTRIPLE *)pPixel)[x] = *((RGBTRIPLE *)&crValue);
			break;
		case 32:
			((ULONG *)pPixel)[x] = crValue;
			break;
	}
}

//���������� ��������������� �������
//���������:
//	pSrcPixels		��������� �� ������ �������� ��� ���������������
//	pSrcIMGDESCR	��������� �� ��������� IMAGEDESCR, ��������� � �������� ��������
//	ppDstPixels		��������� �� ����������, ����������� �������������� ������ ��������
//	ppDstIMGDESCR	��������� �� ����������, ����������� ��������� �� ��������� IMAGEDESCR, ����������� �������������� ������ ��������
//	lNewW			����� ������ �����������
//	lNewH			����� ������ �����������
//�����: ������ ppDstPixels � ��������� pDstIMGDESCR ���������� �������� ����������� new, � ���������� ������������ ��
//������������ ��������, ����������� ������ delete[] ��� ppDstPixels � delete ��� pDstIMGDESCR
//������� �� ���������� ��������
void ResampleImagePixels(LPCBYTE pSrcPixels, LPIMAGEDESCR pSrcIMGDESCR, LPBYTE *ppDstPixels, LPIMAGEDESCR *pDstIMGDESCR, ULONG lNewW, ULONG lNewH)
{
	ULONG lNewSize = lNewW * lNewH * (pSrcIMGDESCR->cBitCount >> 3);
	*ppDstPixels = new BYTE[lNewSize];
	memset(*ppDstPixels, 0, lNewSize);

	*pDstIMGDESCR = new IMAGEDESCR;
	memcpy(*pDstIMGDESCR, pSrcIMGDESCR, sizeof(IMAGEDESCR));

	((LPIMAGEDESCR)*pDstIMGDESCR)->width = lNewW;
	((LPIMAGEDESCR)*pDstIMGDESCR)->height = lNewH;

	ULONG lSrcW = pSrcIMGDESCR->width;
	ULONG lSrcH = pSrcIMGDESCR->height;

	double dblXR = (double)lSrcW / (double)lNewW;
	double dblYR = (double)lSrcH / (double)lNewH;
	ULONG lSrcPos, lDstPos;

	for (ULONG j = 0; j < lNewH; j++)
	{
		for (ULONG i = 0; i < lNewW; i++)
		{

			lSrcPos = ULONG((floor(j * dblYR) * lSrcW) + floor(i * dblXR));
			lDstPos = (j * lNewW) + i;

			switch (pSrcIMGDESCR->cBitCount)
			{
				case 24:
					((RGBTRIPLE *)*ppDstPixels)[lDstPos] = ((RGBTRIPLE *)pSrcPixels)[lSrcPos];
					break;
				case 32:
					((ULONG *)*ppDstPixels)[lDstPos] = ((ULONG *)pSrcPixels)[lSrcPos];
					break;
			}
		}
	}
}

//���������� ��������������� �������
//���������:
//	pData			��������� �� �������� ������, ������� ������� �����������
//	dwDataSize		������ ��������� ������� (� ������)
//������� �� ���������� ��������
void ReverseBytes(LPBYTE pData, DWORD dwDataSize)
{
	LPBYTE pTmp = new BYTE[dwDataSize];
	CopyMemory(pTmp, pData, dwDataSize);
	for (DWORD i = 0; i < dwDataSize; i++) {
		pData[i] = pTmp[(dwDataSize - 1) - i];
	}
	delete[] pTmp;
}

//���������� ��������������� �������
//���������:
//	tValue			�������� ��� ��������
//	tMin			���������� ���������� �������� ��� tValue
//	tMax			����������� ���������� �������� ��� tValue
//������������ ��������: �������� tValue, ���� ���������� tValue � ������ �����������
//� ������������ ������
#ifdef __USE_EXPORT_KEYWORD__
export
#else
template long CheckBounds(long, long, long);
//template long CheckBounds(unsigned long, unsigned long, unsigned long);
//template short CheckBounds(short, short, short);
//template short CheckBounds(unsigned short, unsigned short, unsigned short);
//template char CheckBounds(char, char, char);
//template char CheckBounds(unsigned char, unsigned char, unsigned char);
#endif
template <class T>
T CheckBounds(T tValue, T tMin, T tMax)
{
	if (tValue < tMin) tValue = tMin;
	if (tValue > tMax) tValue = tMax;
	return tValue;
}

//���������� ��������������� �������
//���������:
//	pArr			��������� �� ������ ��� ����������
//	intArrSize		������ �������
//������� �� ���������� ��������
#ifdef __USE_EXPORT_KEYWORD__
export
#else
//template void SortArray_Shell(long *, const int);
//template void SortArray_Shell(unsigned long *, const int);
//template void SortArray_Shell(short *, const int);
//template void SortArray_Shell(unsigned short *, const int);
//template void SortArray_Shell(char *, const int);
template void SortArray_Shell(unsigned char *, const int);
#endif
template <class T>
void SortArray_Shell(T *pArr, const int intArrSize)
{
	int i, j;
	int intStep;
	T tTmp;

	intStep = (intArrSize >> 1);

	while (intStep)
	{
		for (i = 0; i < (intArrSize - intStep); i++)
		{
			for (j = i; (j >= 0) && (pArr[j] > pArr[j + intStep]); j--)
			{
				tTmp = pArr[j];
				pArr[j] = pArr[j + intStep];
				pArr[j + intStep] = tTmp;
			}                 
		}
		intStep >>= 1;
	} 
}

#ifdef __USE_OPENCL__

//���������� ��������������� �������
//���������:
//	strFileName		��� ����� � ����� OpenCL
//	strSrcCode		����� ��� ����
//���������� true � ������ ������, false � ������ ������
bool LoadOpenCLSources(std::string strFileName, std::string& strSrcCode)
{
	size_t szFileSize;

	std::fstream fSrcCode(strFileName.c_str(), std::ios::in | std::ios::binary);

	if (fSrcCode.is_open()) 
	{
		fSrcCode.seekg(0, std::ios::end);
		szFileSize = (size_t)fSrcCode.tellg();
		fSrcCode.seekg(0, std::ios::beg);

		strSrcCode.resize(szFileSize, '\0');
		fSrcCode.read((LPSTR)strSrcCode.c_str(), szFileSize);
		fSrcCode.close();

		return true;
	}
	else
		strSrcCode = "";

	return false;
}

#endif