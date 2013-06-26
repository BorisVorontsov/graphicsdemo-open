#include "StdAfx.h"


#ifdef __USE_GDIPLUS__

#include "ISaveLoad.h"

#include "Sample_jpg.h"

#include "resource.h"

#include <memory>
#include <algorithm>
#include <string>



void ComRelease(IUnknown *aIntf)
{
	if( aIntf )
		aIntf->Release();
}

class GdiPlusSaveLoad : public ISaveLoadImpl
{
public:
	GdiPlusSaveLoad()
	{
		ULONG_PTR gdipToken;
		GdiplusStartupInput gdipStartupInput;
		GdiplusStartup(&gdipToken, &gdipStartupInput, 0);
	}

	~GdiPlusSaveLoad()
	{
	}


	//Загрузка изображения посредством GDI+
	//Парметры:
	//	hWndCanvas		окно, для которого предназначено изображение. Функция автоматически изменит размер окна под размер изображения
	//	hDCCanvas		DC, на котором будет отрисовано изображение. Если передать NULL, функция использует DC окна hWndCanvas
	//	lpFileName		имя файла изображения
	//Функция не возвращает значений
	void DoLoadImage(Image &pImage, const RECT &aLimit, RECT &aResultDims, HWND hwnd, HDC hdc)
	{
		LONG lW = 0, lH = 0;

		HDC hDCCanvas1 = (hdc)?hdc:GetDC(hwnd);
	
		Graphics pGraphics(hDCCanvas1);

		if (pImage.GetLastStatus() == Ok)
		{
			lW = pImage.GetWidth();
			lH = pImage.GetHeight();
			if (lW > aLimit.right) 
				lW = aLimit.right;

			if (lH > aLimit.bottom) 
				lH = aLimit.bottom;
		
			aResultDims.left	= 0;
			aResultDims.top		= 0;
			aResultDims.right	= lW;
			aResultDims.bottom	= lH;

			if (hwnd)
				SetWindowPos(hwnd, NULL, aLimit.left, aLimit.top, lW, lH, SWP_NOZORDER);

			Rect rcImage(0, 0, aResultDims.right, aResultDims.bottom);
			pGraphics.DrawImage(&pImage, rcImage);
		}

		if (!hdc)
			ReleaseDC(hwnd, hDCCanvas1);
	}

	virtual void LoadImageFromFile(const RECT &aLimit, RECT &aResultDims, HWND hwnd, HDC hdc, const std::wstring &aPth)
	{
		Image pImage(aPth.c_str());
	
		DoLoadImage(pImage, aLimit, aResultDims, hwnd, hdc);
	}

	virtual void LoadStandartImage(const RECT &aLimit, RECT &aResultDims, HWND hwnd, HDC hdc)
	{
		// memory...
		IStream *spStream = 0;

		if( !SUCCEEDED(::CreateStreamOnHGlobal(0, TRUE, &spStream)) )
			return;

		std::unique_ptr<IStream, std::function<void(IUnknown *)>> tStream(spStream, ComRelease);

		if( !SUCCEEDED(spStream->Write(get_Sample_jpg_buf(), get_Sample_jpg_size(), 0) ) )
			return;
		//

		Image pImage(tStream.get());
	
		DoLoadImage(pImage, aLimit, aResultDims, hwnd, hdc);
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
		return LoadRes(IDS_ODGDIPFILTER);
	}

	virtual std::wstring getSaveFilter()
	{
		return LoadRes(IDS_SDGDIPFILTER);
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

		switch (aFilterIdx)
		{
			case 1: //BMP
				if (tExt != L".bmp")
					tExt = L".bmp";
				break;

			case 2: //JPEG
				if (tExt != L".jpeg" && tExt != L".jpg")
					tExt = L".jpg";
				break;
			case 3: //GIF
				if (tExt != L".gif")
					tExt = L".gif";
				break;
			case 4: //TIFF
				if (tExt != L".tiff" && tExt != L".tif")
					tExt = L".tif";
				break;
			case 5: //PNG
				if (tExt != L".png")
					tExt = L".png";
				break;
		}

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

		HDC hCDC;
		HBITMAP hTmpBitmap, hOldBitmap;
		Image *pImage;
		CLSID EncClsid;
		EncoderParameters *pEncParams = NULL;
		ULONG uEncParamsSize;
		//TCHAR lpExt[64] = {0};
		LPTSTR lpFmt;

		hCDC = CreateCompatibleDC(hDCCanvas);
		hTmpBitmap = CreateCompatibleBitmap(hDCCanvas, aLimit.right - aLimit.left, aLimit.bottom - aLimit.top);
		hOldBitmap = (HBITMAP)SelectObject(hCDC, hTmpBitmap);

		BitBlt(hCDC, 0, 0, aLimit.right - aLimit.left, aLimit.bottom - aLimit.top, hDCCanvas, 0, 0, SRCCOPY);
		
		pImage = Bitmap::FromHBITMAP(hTmpBitmap, NULL);

		//SP_ExtractRightPart(lpFileName, lpExt, '.');

		if ( aExt == L".bmp" )
			lpFmt = TEXT("image/bmp");
		else if ( aExt == L".jpg" || aExt == L".jpeg" )
			lpFmt = TEXT("image/jpeg");
		else if ( aExt == L".gif" )
			lpFmt = TEXT("image/gif");
		else if ( aExt == L".tif" || aExt == L".tiff" )
			lpFmt = TEXT("image/tiff");
		else if ( aExt == L".png" )
			lpFmt = TEXT("image/png");
		else
			lpFmt = TEXT("image/bmp");

		GetEncoderClsid(lpFmt, &EncClsid);

		Bitmap TmpBmp(1, 1);
		uEncParamsSize = TmpBmp.GetEncoderParameterListSize(&EncClsid);
		if (uEncParamsSize)
			pEncParams = (EncoderParameters*)new BYTE[uEncParamsSize];

		if (aExt == L".jpg" || aExt == L".jpeg")
		{
			ULONG uEncQualLvl = 100;

			pEncParams->Count = 1;
			pEncParams->Parameter[0].Guid = EncoderQuality;
			pEncParams->Parameter[0].Type = EncoderParameterValueTypeLong;
			pEncParams->Parameter[0].NumberOfValues = 1;
			pEncParams->Parameter[0].Value = &uEncQualLvl;
		}
		else if (aExt == L".tif" || aExt == L".tiff")
		{
			ULONG uCompression = EncoderValueCompressionLZW;
			ULONG uColorDepth = 32;

			pEncParams->Count = 2;
			pEncParams->Parameter[0].Guid = EncoderCompression;
			pEncParams->Parameter[0].Type = EncoderParameterValueTypeLong;
			pEncParams->Parameter[0].NumberOfValues = 1;
			pEncParams->Parameter[0].Value = &uCompression;

			pEncParams->Parameter[1].Guid = EncoderColorDepth;
			pEncParams->Parameter[1].Type = EncoderParameterValueTypeLong;
			pEncParams->Parameter[1].NumberOfValues = 1;
			pEncParams->Parameter[1].Value = &uColorDepth;
		}
		else
		{
			if (pEncParams)
				pEncParams->Count = 0;
		}

		pImage->Save(aFilePath.c_str(), &EncClsid, pEncParams);

		DeleteObject(SelectObject(hCDC, hOldBitmap));
		DeleteDC(hCDC);

		if (pEncParams)
			delete[] pEncParams;
		delete pImage;
	} 
	

	//Вспомогательная функция, для получения UID encoder'а по формату изображения
	bool GetEncoderClsid(LPCTSTR lpFmt, CLSID* pClsid)
	{
	   UINT uNumOfEncs = 0;
	   UINT uEncArrSize = 0;

	   ImageCodecInfo* pImageCodecInfo = NULL;
	   GetImageEncodersSize(&uNumOfEncs, &uEncArrSize);

	   if(uEncArrSize == 0)
		  return false;

	   pImageCodecInfo = (ImageCodecInfo*)new BYTE[uEncArrSize];
	   if(pImageCodecInfo == NULL)
		  return false;

	   GetImageEncoders(uNumOfEncs, uEncArrSize, pImageCodecInfo);

	   for(UINT i = 0; i < uNumOfEncs; i++)
	   {
		  if(_tcscmp(pImageCodecInfo[i].MimeType, lpFmt) == 0)
		  {
			 *pClsid = pImageCodecInfo[i].Clsid;
			 delete[] pImageCodecInfo;
			 return true;
		  }    
	   }

	   delete[] pImageCodecInfo;
	   return false;
	}

};

ISaveLoadImpl *ISaveLoadImpl::create()
{
	return new GdiPlusSaveLoad;
}

#endif