#include "StdAfx.h"

#include "ISaveLoad.h"


ISaveLoadImpl::~ISaveLoadImpl()
{
}

//////////////////////////////////////////////////

SIZE_T GetOpenDialog(HINSTANCE hInstance,
					HWND hWnd,
					LPCTSTR lpTitle,
					LPTSTR lpFileName,
					SIZE_T szFNSize,
					LPCTSTR lpFilter,
					DWORD dwFilterIndex,
					BOOL bMultiSelect);

SIZE_T GetSaveDialog(HINSTANCE hInstance,
					HWND hWnd,
					LPCTSTR lpTitle,
					LPTSTR lpFileName,
					SIZE_T szFNSize,
					LPCTSTR lpFilter,
					LPDWORD pFilterIndex,
					LPCTSTR lpDefExt,
					LPCTSTR lpInitialDir = NULL);
//////////////////////////////////////////////////

ISaveLoad::ISaveLoad()
	:mImpl( ISaveLoadImpl::create() )
{
}


ISaveLoad::~ISaveLoad()
{
	delete mImpl;
}

void ISaveLoad::reload(const RECT &aLimit, RECT &aResultDims, HWND hwnd, HDC hdc)
{
	if( mPathToImage.empty() )
	{
		mImpl->LoadStandardImage(aLimit, aResultDims, hwnd, hdc);
	}
	else
	{
		mImpl->LoadImageFromFile(aLimit, aResultDims, hwnd, hdc, mPathToImage);
	}
}

bool ISaveLoad::loadDlg(HWND hwnd)
{
	TCHAR lpODFile[MAX_PATH] = {0};

	std::wstring tODFilter = mImpl->getLoadFilter();

	for (ULONG i = 0; i < tODFilter.size(); i++) 
	{
		if (tODFilter[i] == '|')
			tODFilter[i] = '\0';
	}

	if (GetOpenDialog(GetModuleHandle(NULL), hwnd, TEXT("Load Picture"), lpODFile,
		MAX_PATH - 1, tODFilter.c_str(), 1, FALSE))
	{
		mPathToImage = lpODFile;
		
		return true;
	}

	return false;
}

void ISaveLoad::savePicture(HWND hwnd, HDC hDCCanvas, const RECT &aLimit)
{
	TCHAR lpSDFile[MAX_PATH] = {0};
	TCHAR lpSDFilter[MAX_PATH] = {0};
	TCHAR lpExt[64] = {0};
	DWORD dwFilterIndex = 1;

	std::wstring tSaveFlt = mImpl->getSaveFilter();

	for (ULONG i = 0; i < tSaveFlt.size(); i++) 
	{
		if (tSaveFlt[i] == '|')
			tSaveFlt[i] = '\0';
	}

	std::wstring tFilePath;

	if (GetSaveDialog(GetModuleHandle(NULL), hwnd, TEXT("Save Picture As"), lpSDFile,
		MAX_PATH - 1, tSaveFlt.c_str(), &dwFilterIndex, NULL))
	{
		tFilePath = lpSDFile;
		mImpl->SavePictureToFile(hDCCanvas, aLimit, tFilePath, dwFilterIndex);
	}
}


SIZE_T GetOpenDialog(HINSTANCE hInstance,
					HWND hWnd,
					LPCTSTR lpTitle,
					LPTSTR lpFileName,
					SIZE_T szFNSize,
					LPCTSTR lpFilter,
					DWORD dwFilterIndex,
					BOOL bMultiSelect)
{
	OPENFILENAME OFN = {0};

	OFN.lStructSize = sizeof(OFN);
	OFN.hInstance = hInstance;
	OFN.hwndOwner = hWnd;
	OFN.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES;
	if (bMultiSelect)
		OFN.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	OFN.lpstrTitle = lpTitle;
	OFN.lpstrFile = lpFileName;
	OFN.nMaxFile = szFNSize;
	OFN.lpstrFilter = lpFilter;
	OFN.nFilterIndex = dwFilterIndex;

	if (GetOpenFileName(&OFN))
	{
		return _tcslen(lpFileName);
	}
	else return 0;
}

SIZE_T GetSaveDialog(HINSTANCE hInstance,
					HWND hWnd,
					LPCTSTR lpTitle,
					LPTSTR lpFileName,
					SIZE_T szFNSize,
					LPCTSTR lpFilter,
					LPDWORD pFilterIndex,
					LPCTSTR lpDefExt,
					LPCTSTR lpInitialDir)
{
	OPENFILENAME OFN = {0};

	OFN.lStructSize = sizeof(OFN);
	OFN.hInstance = hInstance;
	OFN.hwndOwner = hWnd;
	OFN.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_OVERWRITEPROMPT |
		OFN_NOREADONLYRETURN | OFN_EXPLORER;
	OFN.lpstrTitle = lpTitle;
	OFN.lpstrFile = lpFileName;
	OFN.nMaxFile = szFNSize;
	OFN.lpstrFilter = lpFilter;
	OFN.nFilterIndex = *pFilterIndex;
	OFN.lpstrDefExt = lpDefExt;
	OFN.lpstrInitialDir = lpInitialDir;

	if (GetSaveFileName(&OFN))
	{
		*pFilterIndex = OFN.nFilterIndex;
		return _tcslen(lpFileName);
	}
	else return 0;
}
