#ifndef GRAPHICSHF_H
#define GRAPHICSHF_H

#define A_BGRA(Color)			((BYTE)((Color) >> 24))
#define R_BGRA(Color)			((BYTE)((Color) >> 16))
#define G_BGRA(Color)			((BYTE)(((WORD)(Color)) >> 8))
#define B_BGRA(Color)			((BYTE)(Color))

#define BGR(B, G, R)			(((BYTE)B) | ((BYTE)G << 8) | ((BYTE)R << 16))
#define BGRA(B, G, R, A)		(((BYTE)B) | ((BYTE)G << 8) | ((BYTE)R << 16) | ((BYTE)A << 24))

#define GP_INVALIDPIXEL			0xFFFFFFFF

int GetImagePixels(HDC hDC, ULONG lW, ULONG lH, LPBYTE *ppPixels, ULONG *pBytesCnt, LPBITMAPINFO *ppBMI);
int SetImagePixels(HDC hDC, ULONG lW, ULONG lH, LPCBYTE pPixels, LPBITMAPINFO pBMI);

COLORREF GetPixel(LPCBYTE pPixels, LPBITMAPINFO pBMI, LONG x, LONG y);
void SetPixel(LPBYTE pPixels, LPBITMAPINFO pBMI, LONG x, LONG y, COLORREF crValue);

void ResampleImagePixels(LPCBYTE pSrcPixels, LPBITMAPINFO pSrcBMI, LPBYTE *ppDstPixels, LPBITMAPINFO *ppDstBMI, ULONG lNewW, ULONG lNewH);

void ReverseBytes(LPBYTE pData, DWORD dwDataSize);
template <class T>
	T CheckBounds(T tValue, T tMin, T tMax);
template <class T>
	void SortArray_Shell(T *pArr, const int intArrSize);

#ifdef __USE_OPENCL__

#ifndef _STRING_
#include <string>
#endif

bool LoadOpenCLSources(std::string strFileName, std::string& strSrcCode);
#endif

#endif