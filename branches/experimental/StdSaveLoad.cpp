#include "StdAfx.h"

#ifndef __USE_GDIPLUS__

#include "ISaveLoad.h"

#include "resource.h"

#include <algorithm>
#include <string>


class StdSaveLoad : public ISaveLoadImpl
{
public:
	//Загрузка изображения посредством GDI+
	//Парметры:
	//	hWndCanvas		окно, для которого предназначено изображение. Функция автоматически изменит размер окна под размер изображения
	//	hDCCanvas		DC, на котором будет отрисовано изображение. Если передать NULL, функция использует DC окна hWndCanvas
	//	lpFileName		имя файла изображения
	//Функция не возвращает значений

	virtual void DoLoadImage(HBITMAP hBitmap, const RECT &aLimit, RECT &aResultDims, HWND hwnd, HDC hdc)
	{
		LONG lW, lH;
		//HDC hDCCanvas1 = (hDCCanvas)?hDCCanvas:GetDC(hWndCanvas);
		HDC hDCCanvas1 = (hdc)?hdc:GetDC(hwnd);

		HBITMAP hOldBitmap;
		BITMAP BMD = {0};
		
		GetObject(hBitmap, sizeof(BMD), &BMD);
		
		lW = BMD.bmWidth;
		lH = BMD.bmHeight;
		if (lW > aLimit.right) 
			lW = aLimit.right;
		if (lH > aLimit.bottom) 
			lH = aLimit.bottom;

		aResultDims.left = 0;
		aResultDims.top = 0;
		aResultDims.right = lW;
		aResultDims.bottom = lH;

		if (hwnd)
			SetWindowPos(hwnd, NULL, aLimit.left, aLimit.top, lW, lH, SWP_NOZORDER);

		HDC hDCSrc = CreateCompatibleDC(hDCCanvas1);
		hOldBitmap = (HBITMAP)SelectObject(hDCSrc, hBitmap);

		SetStretchBltMode(hDCCanvas1, STRETCH_HALFTONE);
		StretchBlt(hDCCanvas1, 0, 0, lW, lH, hDCSrc, 0, 0, BMD.bmWidth, BMD.bmHeight, SRCCOPY);

		DeleteObject(SelectObject(hDCSrc, hOldBitmap));
		DeleteDC(hDCSrc);

		if (!hdc)
			ReleaseDC(hwnd, hDCCanvas1);
	}

	virtual void LoadImageFromFile(const RECT &aLimit, RECT &aResultDims, HWND hwnd, HDC hdc, const std::wstring &aPth)
	{
		HBITMAP hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), aPth.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		DoLoadImage(hBitmap, aLimit, aResultDims, hwnd, hdc);
	}

	virtual void LoadStandartImage(const RECT &aLimit, RECT &aResultDims, HWND hwnd, HDC hdc)
	{
		// resources...
		HBITMAP hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_DEFAULTBITMAP), IMAGE_BITMAP, 0, 0, 0);
		DoLoadImage(hBitmap, aLimit, aResultDims, hwnd, hdc);
		
	}

	std::wstring LoadRes(int id)
	{
		const int MaxString = 1024;
		wchar_t tPtr[MaxString];
		const int tLen = LoadString(GetModuleHandle(NULL), id, tPtr, MaxString);
		if( tPtr && tLen > 0 )
			return std::wstring(tPtr, tPtr + tLen);

		return std::wstring();
	}

	virtual std::wstring getLoadFilter()
	{
		return LoadRes(IDS_ODSTDFILTER);
	}

	virtual std::wstring getSaveFilter()
	{
		return LoadRes(IDS_SDSTDFILTER);
	}

	virtual void SavePictureToFile(HDC hDCCanvas, const RECT &aLimit, const std::wstring &aFile, int aFilterIdx)
	{
		std::wstring tFilePath = aFile;
		std::wstring::size_type tPos = aFile.find_last_of(L".");
		std::wstring tExt;

		if( tPos != std::string::npos )
			tExt = aFile.substr(tPos);
		
		std::transform(tExt.begin(), tExt.end(), tExt.begin(), towlower);

		std::wstring tOldExt = tExt;

		if (tExt != L".bmp")
			tExt = L".bmp";

		if( tOldExt != tExt )
			tFilePath += tExt;

		DoSavePicture(hDCCanvas, aLimit, tFilePath, tExt);
	}



	//Сохранение изображения посредством GDI+
	//Параметры:
	//	hDCCanvas		DC, с которого функция должна взять изображение
	//	lpFileName		имя файла для сохранения изображения
	//Функция не возвращает значений
	void DoSavePicture(HDC hDCCanvas, const RECT &aLimit, const std::wstring &aFilePath, const std::wstring &aExt)
	{
		HANDLE hFile;
		BITMAPFILEHEADER BFH = {0};
		BITMAPINFO BMI = {0}, BMITmp = {0};
		LPBYTE pData = NULL;
		ULONG lDataSize, lColors, lPaletteSize, lWritten;
		HDC hCDC;
		HBITMAP hTmpBitmap, hOldBitmap;

		hFile = CreateFile(aFilePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
		if (hFile != INVALID_HANDLE_VALUE)
		{
			hCDC = CreateCompatibleDC(hDCCanvas);
			hTmpBitmap = CreateCompatibleBitmap(hDCCanvas, aLimit.right - aLimit.left, aLimit.bottom - aLimit.top);
			hOldBitmap = (HBITMAP)SelectObject(hCDC, hTmpBitmap);

			BitBlt(hCDC, 0, 0, aLimit.right - aLimit.left, aLimit.bottom - aLimit.top, hDCCanvas, 0, 0, SRCCOPY);

			BMITmp.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			GetDIBits(hCDC, hTmpBitmap, 0, 0, NULL, &BMITmp, DIB_RGB_COLORS);

			lDataSize = BMITmp.bmiHeader.biSizeImage;
			pData = new BYTE[lDataSize];

			BMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			BMI.bmiHeader.biWidth = BMITmp.bmiHeader.biWidth;
			BMI.bmiHeader.biHeight = BMITmp.bmiHeader.biHeight;
			BMI.bmiHeader.biBitCount = BMITmp.bmiHeader.biBitCount;
			BMI.bmiHeader.biPlanes = BMITmp.bmiHeader.biPlanes;
			GetDIBits(hCDC, hTmpBitmap, 0, BMI.bmiHeader.biHeight, pData, &BMI, DIB_RGB_COLORS);

			if (BMI.bmiHeader.biClrUsed != 0)
				lColors = BMI.bmiHeader.biClrUsed;
			else if (BMI.bmiHeader.biBitCount > 8)
				lColors = 0;
			else
				lColors = (ULONG)pow(2.0, (int)BMI.bmiHeader.biBitCount);
			lPaletteSize = (lColors * sizeof(RGBQUAD));

			BFH.bfType = 0x4D42; //BM
			BFH.bfOffBits = (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + lPaletteSize);
			BFH.bfSize = (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + lDataSize);
			WriteFile(hFile, &BFH, sizeof(BITMAPFILEHEADER), &lWritten, NULL);
			WriteFile(hFile, &BMI.bmiHeader, sizeof(BITMAPINFOHEADER), &lWritten, NULL);
			WriteFile(hFile, pData, lDataSize, &lWritten, NULL);

			if (pData)
				delete[] pData;

			DeleteObject(SelectObject(hCDC, hOldBitmap));
			DeleteDC(hCDC);

			SetEndOfFile(hFile);
			CloseHandle(hFile);
		}
	} 


};

ISaveLoadImpl *ISaveLoadImpl::create()
{
	return new StdSaveLoad;
}


#endif