#pragma once

#include <vector>

//Информация о растре
typedef struct tagIMAGEDESCR
{
	int width;
	int height;
	char cBitCount;
} IMAGEDESCR, *LPIMAGEDESCR;

//Макросы получения компонент цвета из COLORREF значения
#define A_BGRA(Color)			((BYTE)((Color) >> 24))
#define R_BGRA(Color)			((BYTE)((Color) >> 16))
#define G_BGRA(Color)			((BYTE)(((WORD)(Color)) >> 8))
#define B_BGRA(Color)			((BYTE)(Color))

//Макросы получения COLORREF из компонент
#define BGR(B, G, R)			(((BYTE)B) | ((BYTE)G << 8) | ((BYTE)R << 16))
#define BGRA(B, G, R, A)		(((BYTE)B) | ((BYTE)G << 8) | ((BYTE)R << 16) | ((BYTE)A << 24))

//Код ошибки для функций работы с пикселями
#define GP_INVALIDPIXEL			0xFFFFFFFF

int GetImagePixels(HDC hDC, ULONG lW, ULONG lH, std::vector<BYTE> &ppPixels, IMAGEDESCR &ppIMGDESCR);
int SetImagePixels(HDC hDC, ULONG lW, ULONG lH, LPCBYTE pPixels, LPIMAGEDESCR pIMGDESCR);

COLORREF GetPixel(const std::vector<BYTE> &pPixels, LPIMAGEDESCR pIMGDESCR, LONG x, LONG y);
COLORREF GetPixel(LPCBYTE pPixels, LPIMAGEDESCR pIMGDESCR, LONG x, LONG y);
void SetPixel(LPBYTE pPixels, LPIMAGEDESCR pIMGDESCR, LONG x, LONG y, COLORREF crValue);

void ResampleImagePixels(
	const std::vector<BYTE> &srcPixels, 
	const IMAGEDESCR &pSrcIMGDESCR, 
	std::vector<BYTE> &dstPixels, 
	IMAGEDESCR &pDstIMGDESCR, 
	ULONG lNewW, ULONG lNewH);
//void ResampleImagePixels(LPCBYTE pSrcPixels, LPIMAGEDESCR pSrcIMGDESCR, LPBYTE *ppDstPixels, LPIMAGEDESCR *ppDstIMGDESCR, ULONG lNewW, ULONG lNewH);

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
