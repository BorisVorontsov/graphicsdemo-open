#pragma once

#include "Singleton.h"

class ISaveLoadImpl
{
public:
	virtual ~ISaveLoadImpl();

	static ISaveLoadImpl* create();

	virtual void drawImage(HDC hDC) = 0;
	virtual void loadStandardImage() = 0;
	virtual void loadImageFromFile(const std::wstring &strPath) = 0;

	virtual SIZE getImageDimensions() = 0;

	virtual std::wstring getLoadFilter() = 0;
	virtual std::wstring getSaveFilter() = 0;

	virtual void saveImageToFile(HDC hDC, const std::wstring &strFile, int nFilterIdx) = 0;

protected:
	std::wstring loadResString(int id);
};

class ISaveLoad: public pattern::SingletonStatic<ISaveLoad>
{
private:
	ISaveLoadImpl *m_pImpl;
	std::wstring m_strPathToImage;

public:
	ISaveLoad();
	~ISaveLoad();

	void reload(HDC hDC);

	bool loadDlg(HWND hWnd, HDC hDC);
	bool saveDlg(HWND hWnd, HDC hDC);

	int imageWidth();
	int imageHeight();
};

