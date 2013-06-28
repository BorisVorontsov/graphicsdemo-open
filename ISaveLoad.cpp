#include "StdAfx.h"

#include "ISaveLoad.h"


ISaveLoadImpl::~ISaveLoadImpl()
{
}

std::wstring ISaveLoadImpl::loadResString(int id)
{
	const int MaxString = 1024;

	wchar_t pPtr[MaxString];
	const int intLen = LoadString(GetModuleHandle(nullptr), id, pPtr, MaxString);
	if( pPtr && intLen > 0 )
		return std::wstring(pPtr, pPtr + intLen);

	return std::wstring();
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
					LPCTSTR lpInitialDir = nullptr);
//////////////////////////////////////////////////

ISaveLoad::ISaveLoad()
	:m_pImpl( ISaveLoadImpl::create() )
{
}


ISaveLoad::~ISaveLoad()
{
	delete m_pImpl;
}

void ISaveLoad::reload(HDC hDC)
{
	if( m_strPathToImage.empty() )
	{
		m_pImpl->loadStandardImage();
	}
	else
	{
		m_pImpl->loadImageFromFile(m_strPathToImage);
	}

	m_pImpl->drawImage(hDC);
}

//Метод загружает изображение в форму
//Параметры:
//	hWnd	хендл родительского окна для диалога открытия
//	hDC		хендл устройства, в которое будет загружено изображение
//Возвращаемое значение: true в случае успешной загрузки, false иначе
bool ISaveLoad::loadDlg(HWND hWnd, HDC hDC)
{
	TCHAR lpODFile[MAX_PATH] = {};
	DWORD dwIndicesCnt = 0;

	std::wstring strODFilter = m_pImpl->getLoadFilter();

	for (ULONG i = 0; i < strODFilter.size(); i++) 
	{
		if (strODFilter[i] == '|')
		{
			if ((i < strODFilter.length()) && (strODFilter[i + 1] != '|'))
				dwIndicesCnt++;

			strODFilter[i] = '\0';
		}
	}

	dwIndicesCnt >>= 1;

	if (GetOpenDialog(GetModuleHandle(nullptr), hWnd, TEXT("Загрузка Изображения"), lpODFile,
		MAX_PATH - 1, strODFilter.c_str(), dwIndicesCnt, FALSE))
	{
		m_strPathToImage = lpODFile;
		
		m_pImpl->loadImageFromFile(m_strPathToImage);
		m_pImpl->drawImage(hDC);

		return true;
	}

	return false;
}

//Метод сохраняет изображение из формы в файл
//Параметры:
//	hWnd	хендл родительского окна для диалога сохранения
//	hDC		хендл устройства, с которое будет сохранено изображение
//Возвращаемое значение: true в случае успешной загрузки, false иначе
bool ISaveLoad::saveDlg(HWND hWnd, HDC hDC)
{
	TCHAR lpSDFile[MAX_PATH] = {};
	TCHAR lpSDFilter[MAX_PATH] = {};
	TCHAR lpExt[64] = {};
	DWORD dwFilterIndex = 1;

	std::wstring strSDFilter = m_pImpl->getSaveFilter();

	for (ULONG i = 0; i < strSDFilter.size(); i++) 
	{
		if (strSDFilter[i] == '|')
			strSDFilter[i] = '\0';
	}

	std::wstring strFilePath;

	if (GetSaveDialog(GetModuleHandle(nullptr), hWnd, TEXT("Сохранение Изображения..."), lpSDFile,
		MAX_PATH - 1, strSDFilter.c_str(), &dwFilterIndex, nullptr))
	{
		strFilePath = lpSDFile;

		m_pImpl->saveImageToFile(hDC, strFilePath, dwFilterIndex);

		return true;
	}

	return false;
}

int ISaveLoad::imageWidth()
{
	return m_pImpl->getImageDimensions().cx;
}

int ISaveLoad::imageHeight()
{
	return m_pImpl->getImageDimensions().cy;
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
	OPENFILENAME OFN = {};

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
	OPENFILENAME OFN = {};

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