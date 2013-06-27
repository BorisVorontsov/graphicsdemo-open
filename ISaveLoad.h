#pragma once

#include "Singleton.h"

class ISaveLoadImpl
{
public:
	virtual ~ISaveLoadImpl();

	static ISaveLoadImpl* create();

	virtual void LoadStandardImage(const RECT &aLimit, RECT &aResultDims, HWND hwnd, HDC hdc) = 0;
	virtual void LoadImageFromFile(const RECT &aLimit, RECT &aResultDims, HWND hwnd, HDC hdc, const std::wstring &aPth) = 0;

	virtual std::wstring getLoadFilter() = 0;
	virtual std::wstring getSaveFilter() = 0;

	virtual void SavePictureToFile(HDC hDCCanvas, const RECT &aLimit, const std::wstring &aFile, int aFilterIdx) = 0;
};

class ISaveLoad: public pattern::SingletonStatic<ISaveLoad>
{
	ISaveLoadImpl *mImpl;

	std::wstring mPathToImage;
public:
	ISaveLoad();
	~ISaveLoad();

	//void LoadStandardmage( const RECT &aLimit, RECT &aResultDims, HWND hwnd, HDC hdc);
	void reload(const RECT &aLimit, RECT &aResultDims, HWND hwnd, HDC hdc);

	bool loadDlg(HWND hwnd);
	void savePicture(HWND hwnd, HDC hDCCanvas, const RECT &aLimit);
};

