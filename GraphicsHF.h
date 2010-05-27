#ifndef GRAPHICSHF_H
#define GRAPHICSHF_H

#define R_BGR(Color)			((BYTE)((Color) >> 16))
#define G_BGR(Color)			((BYTE)(((WORD)(Color)) >> 8))
#define B_BGR(Color)			((BYTE)(Color))

#define BGR(B, G, R)			(((BYTE)B) | ((BYTE)G << 8) | ((BYTE)R << 16))

#define GP_INVALIDPIXEL			0xFFFFFFFF

int GetImagePixels(HDC hDC, ULONG lW, ULONG lH, LPBYTE *ppPixels, ULONG *pBytesCnt, LPBITMAPINFO *ppBMI);
int SetImagePixels(HDC hDC, ULONG lW, ULONG lH, LPBYTE pPixels, LPBITMAPINFO pBMI);

COLORREF GetPixel(LPBYTE pPixels, LPBITMAPINFO pBMI, LONG x, LONG y);
void SetPixel(LPBYTE pPixels, LPBITMAPINFO pBMI, LONG x, LONG y, COLORREF crValue);

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