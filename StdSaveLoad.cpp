#include "StdAfx.h"

#ifndef __USE_GDIPLUS__

#include "ISaveLoad.h"

#include "resource.h"

#include <algorithm>
#include <string>


class StdSaveLoad : public ISaveLoadImpl
{
private:

	HBITMAP m_hCurImage;

public:
	//Отрисовка изображения посредством GDI
	//Парметры:

	//Метод не возвращает значений
	virtual void drawImage(HDC hDC)
	{
		HBITMAP hOldBitmap;
		BITMAP BM = {};
		
		GetObject(m_hCurImage, sizeof(BM), &BM);

		HDC hDCSrc = CreateCompatibleDC(hDC);
		hOldBitmap = (HBITMAP)SelectObject(hDCSrc, m_hCurImage);

		SetStretchBltMode(hDC, STRETCH_HALFTONE);
		StretchBlt(hDC, 0, 0, BM.bmWidth, BM.bmHeight, hDCSrc, 0, 0, BM.bmWidth, BM.bmHeight, SRCCOPY);

		DeleteObject(SelectObject(hDCSrc, hOldBitmap));
		DeleteDC(hDCSrc);
	}

	virtual void loadImageFromFile(const std::wstring &strPath)
	{
		m_hCurImage = (HBITMAP)LoadImage(GetModuleHandle(nullptr), strPath.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	}

	virtual void loadStandardImage()
	{
		m_hCurImage = (HBITMAP)LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDB_DEFAULTBITMAP), IMAGE_BITMAP, 0, 0, 0);
	}

	virtual SIZE getImageDimensions()
	{
		BITMAP BM = {};
		
		GetObject(m_hCurImage, sizeof(BM), &BM);
		SIZE sz = { BM.bmWidth, BM.bmHeight };
		return sz;
	}

	virtual std::wstring getLoadFilter()
	{
		return loadResString(IDS_ODSTDFILTER);
	}

	virtual std::wstring getSaveFilter()
	{
		return loadResString(IDS_SDSTDFILTER);
	}

	virtual void saveImageToFile(HDC hDC, const std::wstring &strFile, int aFilterIdx)
	{
		std::wstring strFilePath = strFile;
		std::wstring::size_type nPos = strFile.find_last_of(L".");
		std::wstring strExt;

		if( nPos != std::string::npos )
			strExt = strFile.substr(nPos);
		
		std::transform(strExt.begin(), strExt.end(), strExt.begin(), towlower);

		std::wstring strOldExt = strExt;

		if (strExt != L".bmp")
			strExt = L".bmp";

		if( strOldExt != strExt )
			strFilePath += strExt;

		saveImage(hDC, strFilePath, strExt);
	}



	//Сохранение изображения посредством GDI
	//Параметры:

	//Метод не возвращает значений
	void saveImage(HDC hDC, const std::wstring &strFilePath, const std::wstring &strExt)
	{
		HANDLE hFile;
		BITMAPFILEHEADER BFH = {};
		BITMAPINFO BMI = {}, BMITmp = {};
		BITMAP BM = {};
		LPBYTE pData = nullptr;
		ULONG lDataSize, lColors, lPaletteSize, lWritten;
		HDC hCDC;
		HBITMAP hTmpBitmap, hOldBitmap;

		hFile = CreateFile(strFilePath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	
		if (hFile != INVALID_HANDLE_VALUE)
		{
			//Получаем информацию о текущем изображении
			GetObject(GetCurrentObject(hDC, OBJ_BITMAP), sizeof(BITMAP), &BM);

			hCDC = CreateCompatibleDC(hDC);
			hTmpBitmap = CreateCompatibleBitmap(hDC, BM.bmWidth, BM.bmHeight);
			hOldBitmap = (HBITMAP)SelectObject(hCDC, hTmpBitmap);

			BitBlt(hCDC, 0, 0, BM.bmWidth, BM.bmHeight, hDC, 0, 0, SRCCOPY);

			BMITmp.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			GetDIBits(hCDC, hTmpBitmap, 0, 0, nullptr, &BMITmp, DIB_RGB_COLORS);

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
			WriteFile(hFile, &BFH, sizeof(BITMAPFILEHEADER), &lWritten, nullptr);
			WriteFile(hFile, &BMI.bmiHeader, sizeof(BITMAPINFOHEADER), &lWritten, nullptr);
			WriteFile(hFile, pData, lDataSize, &lWritten, nullptr);

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