//////////////////////////////////////////////////////////////////////
//		Функции для модуля Graphics (Graphics Helper Functions)
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "GraphicsHelperFunctions.h"

#include <vector>

//Внутренняя вспомогательная функция
//Параметры:
//	hDC				DC с изображением
//	lW				Высота DC/изображения
//	lH				Ширина DC/изображения
//	ppPixels		Указатель на переменную, принимающую массив пикселей
//	pBytesCnt		Указатель на переменную, принимающую размер (в байтах) массива
//	ppIMGDESCR		Указатель на переменную, принимающую структуру IMAGEDESCR, с информацией об изображении
//ВАЖНО: массив ppPixels и структура ppIMGDESCR выделяются функцией посредством new, и вызывающий отвественнен за
//освобождение ресурсов, посредством вызова delete[] для ppPixels и delete для ppIMGDESCR
//Возвращаемое значение: количество полученных строк растра в случае успеха, 0 в случае ошибки
//int GetImagePixels(HDC hDC, ULONG lW, ULONG lH, LPBYTE *ppPixels, ULONG *pBytesCnt, LPIMAGEDESCR *ppIMGDESCR)
int GetImagePixels(
	HDC hDC, 
	ULONG lW, 
	ULONG lH, 
	//LPBYTE *ppPixels, 
	std::vector<BYTE> &ppPixels,
	IMAGEDESCR &ppIMGDESCR)
{
	HDC hTmpDC;
	HBITMAP hOldBitmap, hTmpBitmap;
	BITMAPINFO BMI = {};
	int intScanLines;

	USHORT BPP = GetDeviceCaps(hDC, BITSPIXEL);

	//Не работаем с изображениями ниже 24 bpp (TrueColor)
	if ((BPP != 24) && (BPP != 32)) return 0;

	size_t bytesCnt = 0;

	switch (BPP)
	{
		case 24:
			bytesCnt = (((BPP / 8) * lW + 3) / 4) * 4;
			bytesCnt += ((bytesCnt * lH + 3) / 4) * 4;
			break;
		case 32:
			bytesCnt = lW * lH * (BPP / 8);
			break;
	}

	//*ppPixels = new BYTE[*pBytesCnt];
	ppPixels.resize(bytesCnt);

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

	intScanLines = GetDIBits(hTmpDC, hTmpBitmap, 0, lH, &ppPixels[0], &BMI, DIB_RGB_COLORS);

	//*ppIMGDESCR = new IMAGEDESCR;
	//ZeroMemory(*ppIMGDESCR, sizeof(IMAGEDESCR));

	ppIMGDESCR.width = BMI.bmiHeader.biWidth;
	ppIMGDESCR.height = BMI.bmiHeader.biHeight;
	ppIMGDESCR.cBitCount = (char)BMI.bmiHeader.biBitCount;

	SelectObject(hTmpDC, hOldBitmap);
	DeleteObject(hTmpBitmap);
	DeleteDC(hTmpDC);

	return intScanLines;
}

//Внутренняя вспомогательная функция
//Параметры:
//	hDC				DC с изображением
//	lW				Высота DC/изображения
//	lH				Ширина DC/изображения
//	pPixels			Массив пикселей
//	pIMGDESCR		Указатель на структуру IMAGEDESCR, с информацией об изображении
//Возвращаемое значение: количество примененных строк растра в случае успеха, 0 в случае ошибки
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

//Внутренняя вспомогательная функция
//Параметры:
//	pPixels			Указатель на массив пикселей
//	pIMGDESCR		Указатель на структуру IMAGEDESCR, связанную с массивом пикселей
//	x				x-координата пикселя
//	y				y-координата пикселя
//Возвращаемое значение: пиксель по заданным координатам в BGR[A] в случае успеха,
//GP_INVALIDPIXEL в случае ошибки
COLORREF GetPixel(const std::vector<BYTE> &pPixels, LPIMAGEDESCR pIMGDESCR, LONG x, LONG y)
{
	return GetPixel(&pPixels[0], pIMGDESCR, x, y);
}

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

//Внутренняя вспомогательная функция
//Параметры:
//	pPixels			Указатель на массив пикселей
//	pIMGDESCR		Указатель на структуру IMAGEDESCR, связанную с массивом пикселей
//	x				x-координата пикселя
//	y				y-координата пикселя
//	crValue			Значение пикселя в BGR[A]
//Функция не возвращает значений
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

//Внутренняя вспомогательная функция
//Параметры:
//	srcPixels		Указатель на массив пикселей для масштабирования
//	srcIMGDESCR	Указатель на структуру IMAGEDESCR, связанную с массивом пикселей
//	ppDstPixels		Указатель на переменную, принимающую результирующий массив пикселей
//	ppDstIMGDESCR	Указатель на переменную, принимающую указатель на структуру IMAGEDESCR, описывающую результирующий массив пикселей
//	lNewW			Новая шинира изображения
//	lNewH			Новая высота изображения
//ВАЖНО: массив ppDstPixels и структура pDstIMGDESCR выделяются функцией посредством new, и вызывающий отвественнен за
//освобождение ресурсов, посредством вызова delete[] для ppDstPixels и delete для pDstIMGDESCR
//Функция не возвращает значений

void ResampleImagePixels(const std::vector<BYTE> &srcPixels, const IMAGEDESCR &srcIMGDESCR, std::vector<BYTE> &dstPixels, IMAGEDESCR &pDstIMGDESCR, ULONG lNewW, ULONG lNewH)
{
	size_t Depth = (srcIMGDESCR.cBitCount >> 3);
	size_t lNewSize = lNewW * lNewH * Depth;
	dstPixels.resize(lNewSize, 0);

	pDstIMGDESCR		= srcIMGDESCR;
	pDstIMGDESCR.width	= lNewW;
	pDstIMGDESCR.height = lNewH;

	const unsigned char *source_data	= &srcPixels[0];
	unsigned char *target_data			= &dstPixels[0];

	const long old_height = srcIMGDESCR.height;
	const long old_width  = srcIMGDESCR.width;
	const long x_delta = (old_width<<16) / lNewW;
	const long y_delta = (old_height<<16) / lNewH;

	unsigned char* dest_pixel = target_data;

	long y = 0;
	for ( size_t j = 0; j < lNewH; j++ )
	{
		const unsigned char* src_line = &source_data[(y>>16)*old_width*Depth];

		long x = 0;
		for ( size_t i = 0; i < lNewW; i++ )
		{
			const unsigned char* src_pixel = &src_line[(x>>16)*Depth];
			for( size_t cnt = 0; cnt < Depth; ++cnt )
				dest_pixel[cnt] = src_pixel[cnt];
			dest_pixel += Depth;

			x += x_delta;
		}

		y += y_delta;
	}
}

//Внутренняя вспомогательная функция
//Параметры:
//	pData			Указатель на байтовый массив, который следует перевернуть
//	dwDataSize		Размер байтового массива (в байтах)
//Функция не возвращает значений
void ReverseBytes(LPBYTE pData, DWORD dwDataSize)
{
	LPBYTE pTmp = new BYTE[dwDataSize];
	CopyMemory(pTmp, pData, dwDataSize);
	for (DWORD i = 0; i < dwDataSize; i++) {
		pData[i] = pTmp[(dwDataSize - 1) - i];
	}
	delete[] pTmp;
}

//Внутренняя вспомогательная функция
//Параметры:
//	tValue			Значение для проверки
//	tMin			Минимально допустимое значение для tValue
//	tMax			Максимально допустимое значение для tValue
//Возвращаемое значение: исходное tValue, либо измененное tValue с учетом минимальной
//и максимальной границ
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

//Внутренняя вспомогательная функция
//Параметры:
//	pArr			Указатель на массив для сортировки
//	intArrSize		Размер массива
//Функция не возвращает значений
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

//Внутренняя вспомогательная функция
//Параметры:
//	strFileName		Имя файла с кодом OpenCL
//	strSrcCode		Буфер для кода
//Возвращает true в случае успеха, false в случае ошибки
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