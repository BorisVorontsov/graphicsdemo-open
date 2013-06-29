#include "StdAfx.h"


#ifdef __USE_GDIPLUS__

#include "ISaveLoad.h"

#include "Sample_jpg.h"

#include "resource.h"

void ComRelease(IUnknown *pUnk)
{
	if( pUnk )
		pUnk->Release();
}

class GdiPlusSaveLoad : public ISaveLoadImpl
{
private:
	std::auto_ptr<Image> m_pCurImage;

	//Вспомогательная функция, для получения UID encoder'а по формату изображения
	bool getEncoderClsid(LPCTSTR lpFmt, CLSID* pClsid)
	{
	   UINT uNumOfEncs = 0;
	   UINT uEncArrSize = 0;

	   ImageCodecInfo* pImageCodecInfo = nullptr;
	   GetImageEncodersSize(&uNumOfEncs, &uEncArrSize);

	   if(uEncArrSize == 0)
		  return false;

	   pImageCodecInfo = (ImageCodecInfo*)new BYTE[uEncArrSize];
	   if(pImageCodecInfo == nullptr)
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

public:
	GdiPlusSaveLoad() : m_pCurImage(nullptr)
	{
		ULONG_PTR gdipToken;
		GdiplusStartupInput gdipStartupInput;
		GdiplusStartup(&gdipToken, &gdipStartupInput, 0);
	}

	~GdiPlusSaveLoad()
	{

	}

	//Отрисовка загруженного изображения
	//Параметры:
	//	hDC		DC для вывода
	//Метод не возвращает значений
	void drawImage(HDC hDC)
	{
		if (!m_pCurImage.get()) return;

		Graphics pGraphics(hDC);

		if (m_pCurImage->GetLastStatus() == Ok)
		{
			Rect rcImage(0, 0, m_pCurImage->GetWidth(), m_pCurImage->GetHeight());
			pGraphics.DrawImage(m_pCurImage.get(), rcImage);
		}
	}

	//Загрузка изображения посредством GDI+
	//Парметры:
	//	strPath		имя файла изображения
	//Метод не возвращает значений
	virtual void loadImageFromFile(const std::wstring &strPath)
	{
		m_pCurImage.reset(new Image(strPath.c_str()));
	}

	//Загрузка демонстрационного изображения
	virtual void loadStandardImage()
	{

		IStream *pStream = 0;

		if( !SUCCEEDED(::CreateStreamOnHGlobal(0, TRUE, &pStream)) )
			return;

		std::unique_ptr<IStream, std::function<void(IUnknown *)>> upStream(pStream, ComRelease);

		if( !SUCCEEDED(pStream->Write(get_Sample_jpg_buf(), get_Sample_jpg_size(), 0) ) )
			return;

		m_pCurImage.reset(new Image(upStream.get()));
	}

	virtual SIZE getImageDimensions()
	{
		SIZE sz = {};

		if (m_pCurImage.get())
		{
			sz.cx = m_pCurImage->GetWidth();
			sz.cy = m_pCurImage->GetHeight();
		}

		return sz;
	}

	virtual std::wstring getLoadFilter()
	{
		return loadResString(IDS_ODGDIPFILTER);
	}

	virtual std::wstring getSaveFilter()
	{
		return loadResString(IDS_SDGDIPFILTER);
	}

	virtual void saveImageToFile(HDC hDC, const std::wstring &strFile, int nFilterIdx)
	{
		std::wstring strFilePath = strFile;
		std::wstring::size_type nPos = strFile.find_last_of(L".");
		std::wstring strExt;

		if( nPos != std::string::npos )
			strExt = strFile.substr(nPos);
		
		std::transform(strExt.begin(), strExt.end(), strExt.begin(), towlower);

		std::wstring strOldExt = strExt;

		switch (nFilterIdx)
		{
			case 1: //BMP
				if (strExt != L".bmp")
					strExt = L".bmp";
				break;

			case 2: //JPEG
				if (strExt != L".jpeg" && strExt != L".jpg")
					strExt = L".jpg";
				break;
			case 3: //GIF
				if (strExt != L".gif")
					strExt = L".gif";
				break;
			case 4: //TIFF
				if (strExt != L".tiff" && strExt != L".tif")
					strExt = L".tif";
				break;
			case 5: //PNG
				if (strExt != L".png")
					strExt = L".png";
				break;
		}

		if( strOldExt != strExt )
			strFilePath += strExt;

		saveImage(hDC, strFilePath, strExt);
	}

	//Сохранение изображения посредством GDI+
	//Параметры:
	//	hDC				DC, с которого метод должен взять изображение
	//	strFilePath		имя файла для сохранения изображения
	//Метод не возвращает значений
	void saveImage(HDC hDC, const std::wstring &strFilePath, const std::wstring &strExt)
	{
		HDC hCDC;
		HBITMAP hTmpBitmap, hOldBitmap;
		BITMAP BM = {};
		Image *pImage;
		CLSID EncClsid;
		EncoderParameters *pEncParams = nullptr;
		ULONG uEncParamsSize;
		LPTSTR lpFmt;

		//Получаем информацию о текущем изображении
		GetObject(GetCurrentObject(hDC, OBJ_BITMAP), sizeof(BITMAP), &BM);

		hCDC = CreateCompatibleDC(hDC);
		hTmpBitmap = CreateCompatibleBitmap(hDC, BM.bmWidth, BM.bmHeight);
		hOldBitmap = (HBITMAP)SelectObject(hCDC, hTmpBitmap);

		BitBlt(hCDC, 0, 0, BM.bmWidth, BM.bmHeight, hDC, 0, 0, SRCCOPY);
		
		pImage = Bitmap::FromHBITMAP(hTmpBitmap, nullptr);

		if ( strExt == L".bmp" )
			lpFmt = TEXT("image/bmp");
		else if ( strExt == L".jpg" || strExt == L".jpeg" )
			lpFmt = TEXT("image/jpeg");
		else if ( strExt == L".gif" )
			lpFmt = TEXT("image/gif");
		else if ( strExt == L".tif" || strExt == L".tiff" )
			lpFmt = TEXT("image/tiff");
		else if ( strExt == L".png" )
			lpFmt = TEXT("image/png");
		else
			lpFmt = TEXT("image/bmp");

		//Выбираем соответствующий формату кодек
		getEncoderClsid(lpFmt, &EncClsid);

		Bitmap TmpBmp(1, 1);
		uEncParamsSize = TmpBmp.GetEncoderParameterListSize(&EncClsid);
		if (uEncParamsSize)
			pEncParams = (EncoderParameters*)new BYTE[uEncParamsSize];

		//Выставляем параметры сохранения для различных кодеков
		if (strExt == L".jpg" || strExt == L".jpeg")
		{
			ULONG uEncQualLvl = 100; //Качество сохранения JPEG

			pEncParams->Count = 1;
			pEncParams->Parameter[0].Guid = EncoderQuality;
			pEncParams->Parameter[0].Type = EncoderParameterValueTypeLong;
			pEncParams->Parameter[0].NumberOfValues = 1;
			pEncParams->Parameter[0].Value = &uEncQualLvl;
		}
		else if (strExt == L".tif" || strExt == L".tiff")
		{
			ULONG uCompression = EncoderValueCompressionLZW; //Алгоритм сжатия TIFF
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

		pImage->Save(strFilePath.c_str(), &EncClsid, pEncParams);

		DeleteObject(SelectObject(hCDC, hOldBitmap));
		DeleteDC(hCDC);

		if (pEncParams)
			delete[] pEncParams;
		delete pImage;
	} 

};

ISaveLoadImpl *ISaveLoadImpl::create()
{
	return new GdiPlusSaveLoad;
}

#endif