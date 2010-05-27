//////////////////////////////////////////////////////////////////////
//		������� ��� ������ Graphics (Graphics Helper Functions)
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <fstream>

#include "gddefs.h"
#include "graphicshf.h"

//���������� ��������������� �������
//���������:
//	hDC				DC � ������������
//	lW				������ DC/�����������
//	lH				������ DC/�����������
//	ppPixels		��������� �� ����������, ����������� ������ ��������
//	pBytesCnt		��������� �� ����������, ����������� ������ (� ������) �������
//	ppBMI			��������� �� ����������, ����������� ��������� BITMAPINFO, � ����������� �� �����������
//�����: ������ ppPixels � ��������� ppBMI ���������� �������� ����������� new, � ���������� ������������ ��
//������������ ��������, ����������� ������ delete[] ��� ppPixels � delete ��� ppBMI
//������������ ��������: ���������� ���������� ����� ������ � ������ ������, 0 � ������ ������
int GetImagePixels(HDC hDC, ULONG lW, ULONG lH, LPBYTE *ppPixels, ULONG *pBytesCnt, LPBITMAPINFO *ppBMI)
{
	HDC hTmpDC;
	HBITMAP hOldBitmap, hTmpBitmap;
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

	*ppBMI = new BITMAPINFO;
	ZeroMemory(*ppBMI, sizeof(BITMAPINFO));

    ((LPBITMAPINFO)*ppBMI)->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    ((LPBITMAPINFO)*ppBMI)->bmiHeader.biWidth = lW;
    ((LPBITMAPINFO)*ppBMI)->bmiHeader.biHeight = lH;
    ((LPBITMAPINFO)*ppBMI)->bmiHeader.biPlanes = 1;
    ((LPBITMAPINFO)*ppBMI)->bmiHeader.biBitCount = BPP;
    ((LPBITMAPINFO)*ppBMI)->bmiHeader.biCompression = BI_RGB;

	intScanLines = GetDIBits(hTmpDC, hTmpBitmap, 0, lH, *ppPixels, *ppBMI, DIB_RGB_COLORS);

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
//	pBMI			��������� �� ��������� BITMAPINFO, � ����������� �� �����������
//������������ ��������: ���������� ����������� ����� ������ � ������ ������, 0 � ������ ������
int SetImagePixels(HDC hDC, ULONG lW, ULONG lH, LPBYTE pPixels, LPBITMAPINFO pBMI)
{
	int intScanLines;

	if ((pBMI->bmiHeader.biBitCount != 24) && (pBMI->bmiHeader.biBitCount != 32)) return 0;

	intScanLines = StretchDIBits(hDC, 0, 0, lW, lH, 0, 0, lW, lH, pPixels, pBMI,
		DIB_RGB_COLORS, SRCCOPY);

	return intScanLines;
}

//���������� ��������������� �������
//���������:
//	pPixels			��������� �� ������ ��������
//	pBMI			��������� �� ��������� BITMAPINFO, ��������� � �������� ��������
//	x				x-���������� �������
//	y				y-���������� �������
//������������ ��������: ������� �� �������� ����������� � BGR[A] � ������ ������,
//GP_INVALIDPIXEL � ������ ������
COLORREF GetPixel(LPBYTE pPixels, LPBITMAPINFO pBMI, LONG x, LONG y)
{
	if ((x < 0) || (x >= pBMI->bmiHeader.biWidth)) return GP_INVALIDPIXEL;
	if ((y < 0) || (y >= pBMI->bmiHeader.biHeight)) return GP_INVALIDPIXEL;

	LPBYTE pPixel;
	LONG lBPS, lDelta, lResult = GP_INVALIDPIXEL;
	lBPS = (pBMI->bmiHeader.biWidth * (pBMI->bmiHeader.biBitCount >> 3));
	if (pBMI->bmiHeader.biHeight < 0)
	{
		pPixel = pPixels;
		lDelta = lBPS;
	}
	else
	{
		lDelta = -lBPS;
		pPixel = pPixels + (pBMI->bmiHeader.biHeight - 1) * lBPS;
	}
	pPixel += y * lDelta;
	switch (pBMI->bmiHeader.biBitCount)
	{
		case 24:
			pPixel += x * 3;
			lResult = BGR(pPixel[0], pPixel[1], pPixel[2]);
			break;
		case 32:
			lResult = ((LONG *)pPixel)[x];
			break;
	}
	return lResult;
}

//���������� ��������������� �������
//���������:
//	pPixels			��������� �� ������ ��������
//	pBMI			��������� �� ��������� BITMAPINFO, ��������� � �������� ��������
//	x				x-���������� �������
//	y				y-���������� �������
//	crValue			�������� ������� � BGR[A]
//������� �� ���������� ��������
void SetPixel(LPBYTE pPixels, LPBITMAPINFO pBMI, LONG x, LONG y, COLORREF crValue)
{
	if ((x < 0) || (x >= pBMI->bmiHeader.biWidth)) return;
	if ((y < 0) || (y >= pBMI->bmiHeader.biHeight)) return;

	LPBYTE pPixel;
	LONG lBPS, lDelta;
	lBPS = (pBMI->bmiHeader.biWidth * (pBMI->bmiHeader.biBitCount >> 3));
	if (pBMI->bmiHeader.biHeight < 0)
	{
		pPixel = pPixels;
		lDelta = lBPS;
	}
	else
	{
		lDelta = -lBPS;
		pPixel = pPixels + (pBMI->bmiHeader.biHeight - 1) * lBPS;
	}
	pPixel += y * lDelta;
	switch (pBMI->bmiHeader.biBitCount)
	{
		case 24:
			((RGBTRIPLE *)pPixel)[x] = *((RGBTRIPLE *)&crValue);
			break;
		case 32:
			((LONG *)pPixel)[x] = crValue;
			break;
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