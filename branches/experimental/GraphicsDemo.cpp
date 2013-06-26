//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		"Graphics Demo"
//		© BV (Boris Vorontsov, mailto:borisvorontsov@gmail.com) и участники проекта
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "GraphicsDemo.h"
#include "Graphics.h"
#include "StrParser.h"
#include "Resource.h"

#include "AlgorithmFactory.h"
#include "ISaveLoad.h"

#include <assert.h>

#pragma comment (lib, "comctl32.lib")

#pragma comment (linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' " \
	"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

static HINSTANCE hAppInstance;

static volatile SYSTEM_INFO SI = {0};

static LPCTSTR lpGDWndClass = TEXT("GraphicsDemo_WndClass");
static HWND hMainWnd;

static LONG_PTR pOldCanvasProc;

static HDC hDBDC = NULL;
static HBITMAP hDBBitmap, hOldDBBitmap;

static volatile HANDLE hImgProcThread;
static UINT uImgProcThreadID;

static IMGPROCINFO IPI = {0};

void FillMenu(HWND hWnd);

int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR lpCmdLine,
					   int nCmdShow)
{
	WNDCLASSEX wcex = {0};
	MSG msg;

	hAppInstance = hInstance;
	InitCommonControls();

	GetSystemInfo((LPSYSTEM_INFO)&SI);

	if (SI.dwNumberOfProcessors >= 2)
		SetThreadIdealProcessor(GetCurrentThread(), 0);

	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.lpfnWndProc = DefDlgProc;
	wcex.hInstance = hAppInstance;
	wcex.lpszClassName = lpGDWndClass;
	wcex.cbWndExtra = DLGWINDOWEXTRA;
	wcex.hCursor = LoadCursor(0, IDC_ARROW);
	wcex.hIcon = (HICON)LoadImage(hAppInstance, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 32, 32, 0);
	wcex.hIconSm = (HICON)LoadImage(hAppInstance, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 16, 16, 0);

	RegisterClassEx(&wcex);

	hMainWnd = CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_MAINWND), NULL,
		MainWndProc, 0);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

bool isCheckedMenu(HMENU hMenu, int id)
{
	return  MF_CHECKED == (MF_CHECKED & GetMenuState(hMenu, id, MF_BYCOMMAND));
}

INT_PTR CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			FillMenu(hWnd);

			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)APP_NAME);

			pOldCanvasProc = SetWindowLongPtr(GetDlgItem(hWnd, IDC_STCCANVAS), GWLP_WNDPROC, (LONG_PTR)CanvasProc);

			IPI.hWndMain = hWnd;
			IPI.hWndCanvas = GetDlgItem(hWnd, IDC_STCCANVAS);
			IPI.hWndProgress = CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_PROGRESS),
				IPI.hWndMain, ProgressWndProc, 0);
			IPI.hWndPerfomanceInfo = CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_PERFORMANCEINFO), 
				IPI.hWndMain, PerformanceWndProc, 0);

			PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_CLEAR, 0), 0);
			return TRUE;
		case WM_ERASEBKGND:
		{
			RECT rcClient = {0};
			HDC hDC = (HDC)wParam;
			GetClientRect(hWnd, &rcClient);
			FillRect(hDC, &rcClient, (HBRUSH)GetStockObject(BLACK_BRUSH));
			return TRUE;
		}
		case WM_COMMAND:
		{
			switch LOWORD(wParam)
			{
				case IDM_FILE_LOADPICTURE:
				{
					if( ISaveLoad::getInstance().loadDlg(hWnd) )
						PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_CLEAR, 0), 0);
					break;
				}
				case IDM_FILE_SAVEPICTUREAS:
				{
					RECT tCanvLimit;
					tCanvLimit.top = 0;
					tCanvLimit.left = 0;
					tCanvLimit.right = GD_MAX_CANV_WIDTH;
					tCanvLimit.bottom = GD_MAX_CANV_HEIGHT;
					ISaveLoad::getInstance().savePicture(hWnd, hDBDC, tCanvLimit);
					break;
				}
				case ID_OPTIONS_PERFORMANCEMODE:
				{
					const int tNewState = isCheckedMenu(GetMenu(hWnd), ID_OPTIONS_PERFORMANCEMODE)?MF_UNCHECKED:MF_CHECKED;
					CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_PERFORMANCEMODE, MF_BYCOMMAND|tNewState);
					break;
				}
				case IDM_CLEAR:
				{
					HDC hDC, hTmpDC;
					HBITMAP hTmpBitmap, hOldBitmap;
					RECT rcCanvas;
					
					hDC = GetDC(hWnd);
					hTmpDC = CreateCompatibleDC(hDC);
					hTmpBitmap = CreateCompatibleBitmap(hDC, GD_MAX_CANV_WIDTH, GD_MAX_CANV_HEIGHT);
					hOldBitmap = (HBITMAP)SelectObject(hTmpDC, hTmpBitmap);
					
					RECT tLimits;
					tLimits.top		= GD_MIN_CANV_TOP;
					tLimits.left	= GD_MIN_CANV_LEFT;
					tLimits.right	= GD_MAX_CANV_WIDTH;
					tLimits.bottom	= GD_MAX_CANV_HEIGHT;
					ISaveLoad::getInstance().reload(tLimits, IPI.rcPicture, GetDlgItem(hWnd, IDC_STCCANVAS), hTmpDC);
					
					GetClientRect(GetDlgItem(hWnd, IDC_STCCANVAS), &rcCanvas);
					BitBlt(hDBDC, 0, 0, rcCanvas.right, rcCanvas.bottom, hTmpDC, 0, 0, SRCCOPY);
					RedrawWindow(GetDlgItem(hWnd, IDC_STCCANVAS), NULL, NULL, RDW_NOERASE | RDW_INVALIDATE);
					
					DeleteObject(SelectObject(hTmpDC, hOldBitmap));
					DeleteDC(hTmpDC);
					ReleaseDC(hWnd, hDC);
					break;
				}
				case IDM_HELP_ABOUT:
				{
					TCHAR lpAboutString[256] = {0};
					TCHAR lpTmp[128];
					LoadString(hAppInstance, IDS_ABOUTSTRING1, lpTmp, sizeof(lpTmp) / sizeof(TCHAR));
					_stprintf(lpAboutString, lpTmp, APP_NAME);
					LoadString(hAppInstance, IDS_ABOUTSTRING2, lpTmp, sizeof(lpTmp) / sizeof(TCHAR));
					_tcscat(lpAboutString, lpTmp);
					
					MSGBOXPARAMS MBP = {0};
					MBP.cbSize = sizeof(MBP);
					MBP.hInstance = hAppInstance;
					MBP.dwStyle = MB_USERICON | MB_OK;
					MBP.hwndOwner = hWnd;
					MBP.lpszIcon = MAKEINTRESOURCE(IDI_ICON);
					MBP.lpszText = lpAboutString;
					MBP.lpszCaption = APP_NAME;
					MessageBoxIndirect(&MBP);
					break;
				}
				default:
				{
					if (!hImgProcThread)
					{
						IPI.hDBDC = hDBDC;
						IPI.dwFltIndex = LOWORD(wParam);
						hImgProcThread = (HANDLE)_beginthreadex(NULL, 0, &ImgProcThreadMain,
							hWnd, 0, &uImgProcThreadID);
					}
					break;
				}
			}
			return TRUE;
		}
		case WM_CLOSE:
			DestroyWindow(hWnd);
			return TRUE;
		case WM_DESTROY:
			SetWindowLongPtr(GetDlgItem(hWnd, IDC_STCCANVAS), GWLP_WNDPROC, (LONG_PTR)pOldCanvasProc);

			DeleteObject(SelectObject(hDBDC, hOldDBBitmap));
			DeleteDC(hDBDC);

			PostQuitMessage(0);
			return TRUE;
	}
	return FALSE;
}

LRESULT CALLBACK CanvasProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_ERASEBKGND:
			return TRUE;
		case WM_PAINT:
		{
			PAINTSTRUCT PS;
			RECT rcClient;

			HDC hDC = BeginPaint(hWnd, &PS);
			GetClientRect(hWnd, &rcClient);

			if (hDBDC)
				BitBlt(hDC, 0, 0, rcClient.right, rcClient.bottom, hDBDC, 0, 0, SRCCOPY);

			EndPaint(hWnd, &PS);
			
			break;
		}
		case WM_SIZE:
		{
			if (!hImgProcThread)
			{
				HDC hDC = GetDC(hWnd);
				if (hDBDC)
				{
					DeleteObject(SelectObject(hDBDC, hOldDBBitmap));
					DeleteDC(hDBDC);
				}

				hDBDC = CreateCompatibleDC(hDC);
				hDBBitmap = CreateCompatibleBitmap(hDC, LOWORD(lParam), HIWORD(lParam));
				hOldDBBitmap = (HBITMAP)SelectObject(hDBDC, hDBBitmap);

				ReleaseDC(hWnd, hDC);
				break;
			}
		}
		default:
			return CallWindowProc((WNDPROC)pOldCanvasProc, hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

INT_PTR CALLBACK ProgressWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			SendDlgItemMessage(hWnd, IDC_PGBEP, PBM_SETRANGE32, 0, 100);
			break;
		case WM_SHOWWINDOW:
			if (wParam)
				SendDlgItemMessage(hWnd, IDC_PGBEP, PBM_SETPOS, 0, 0);
			return TRUE;
		case WM_GRAPHICSEVENT:
		{
			switch (LOWORD(wParam))
			{
				case EVENT_ON_PROGRESS:
				{
					//LPONPROGRESSPARAMS pONPP = (LPONPROGRESSPARAMS)lParam;
					int tPercent = (int)lParam;
					SendDlgItemMessage(hWnd, IDC_PGBEP, PBM_SETPOS, tPercent, 0);
					break;
				}
				default:
					//
					break;
			}
			return TRUE;
		}
	}
	return FALSE;
}

INT_PTR CALLBACK PerformanceWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

UINT WINAPI ImgProcThreadMain(LPVOID pArg)
{
	if (SI.dwNumberOfProcessors >= 2)
		SetThreadIdealProcessor(GetCurrentThread(), 1);

	const bool isPerfomanceMode = isCheckedMenu(GetMenu((HWND)pArg), ID_OPTIONS_PERFORMANCEMODE);

	HDC hDC = (IPI.hDBDC)?IPI.hDBDC:GetDC(IPI.hWndCanvas);

	RECT rcCanvas, rcPicture;
	CopyRect(&rcCanvas, (const LPRECT)&IPI.rcPicture);
	CopyRect(&rcPicture, (const LPRECT)&IPI.rcPicture);
	rcCanvas.left += 8;
	rcCanvas.top += 8;
	rcCanvas.right = rcCanvas.right - 8;
	rcCanvas.bottom = rcCanvas.bottom - 8;
	
	if ( !isPerfomanceMode && IPI.dwFltIndex != IDM_FILTERS_BLUR_OCL)
		ShowWindow(IPI.hWndProgress, SW_SHOW);
	else
		ShowWindow(IPI.hWndPerfomanceInfo, SW_SHOW);


	LARGE_INTEGER intFreq, intStart, intEnd;
	TCHAR lpResult[128] = {0};
	if(isPerfomanceMode)
	{
		QueryPerformanceFrequency(&intFreq);
		QueryPerformanceCounter(&intStart);
	}

	std::auto_ptr<IAlgorithm> tPtr = AlgorithmFactory::getInstance().create(IPI.dwFltIndex);
	if( tPtr.get() )
	{
		tPtr->setPerfomanceMode( isPerfomanceMode );
		tPtr->process(hDC, rcPicture, rcCanvas, IPI.hWndProgress);
	}
	else
	{
		switch (IPI.dwFltIndex)
		{
	/*#ifdef __USE_GDIPLUS__
			case IDM_FILTERS_ALPHABLEND:
			{
				Graphics *pGraphics = NULL;
				Image *pImage = new Image(lpPic2Path);
				LONG lW = 0, lH = 0;
				HDC hTmpDC;
				HBITMAP hTmpBmp, hOldBmp;
	
				if (pImage->GetLastStatus() == Ok)
				{
					lW = pImage->GetWidth();
					lH = pImage->GetHeight();

					hTmpDC = CreateCompatibleDC(hDC);
					hTmpBmp = CreateCompatibleBitmap(hDC, lW, lH);
					hOldBmp = (HBITMAP)SelectObject(hTmpDC, hTmpBmp);
					pGraphics = new Graphics(hTmpDC);
					pGraphics->DrawImage(pImage, Rect(0, 0, lW, lH));

					RECT rcCanvas2 = {(rcCanvas.right - (lW * 2)) >> 1, (rcCanvas.bottom - (lH * 2)) >> 1, (rcCanvas.right + (lW * 2)) >> 1,
						(rcCanvas.bottom + (lH * 2)) >> 1};
					AlphaBlend(hDC, rcPicture.right, rcPicture.bottom, hTmpDC, lW, lH, 128, AM_ALPHA_IGNORE, &rcCanvas2, IPI.hWndProgress);

					DeleteObject(SelectObject(hTmpDC, hOldBmp));
					DeleteDC(hTmpDC);

					delete pGraphics;
				}
				delete pImage;

				break;
			}
	#endif*/
			default:
				break;
		}
	}
	

	if(isPerfomanceMode)
	QueryPerformanceCounter(&intEnd);


	if (IsWindowVisible(IPI.hWndProgress))
		ShowWindow(IPI.hWndProgress, SW_HIDE);

	if( IsWindowVisible(IPI.hWndPerfomanceInfo) )
		ShowWindow(IPI.hWndPerfomanceInfo, SW_HIDE);

	if (IPI.hDBDC)
		RedrawWindow(IPI.hWndCanvas, NULL, NULL, RDW_NOERASE | RDW_INVALIDATE);

	if (!IPI.hDBDC)
		ReleaseDC(IPI.hWndCanvas, hDC);

	if(isPerfomanceMode)
	{
		_stprintf(lpResult, TEXT("Время фильтра: %.02f сек."), (double)(intEnd.QuadPart - intStart.QuadPart) /
		(double)intFreq.QuadPart);
		MessageBox(NULL, lpResult, TEXT("Результат выполнения"), MB_ICONINFORMATION);
	}

	hImgProcThread = NULL;
	return 0;
}

//Вспомогательная функция, для получения директории программы
BOOL GetAppPath(LPTSTR lpPath, DWORD dwPathLen, BOOL bAddQuotes)
{
	LPTSTR lpBuff = new TCHAR[dwPathLen];

	if (GetModuleFileName(hAppInstance, lpBuff, dwPathLen) == 0) {
		delete[] lpBuff;
		return FALSE;
	}

	if (_tcschr(lpBuff, ' ') && bAddQuotes) {
		_stprintf(lpPath, TEXT("\"%s\""), lpBuff);
	}
	else {
		_tcscpy(lpPath, lpBuff);
	}

	delete[] lpBuff;
	return TRUE;
}


// Construct menu
std::wstring CorrectMenuName(const std::wstring &aSrc)
{
	std::wstring tResult;

	if( aSrc.empty() )
		return tResult;

	tResult.reserve(aSrc.size());

	bool tPrevAmp = false;
	for( size_t i = 0; i < aSrc.size() - 1; ++i )
	{
		wchar_t tChar = aSrc[i];
		if( tChar == L'&' && !tPrevAmp )
		{

			tPrevAmp = true;
		}
		else
		{
			tPrevAmp = false;

			tResult.push_back(tChar);
		}
	}

	return tResult;
}

int FindSubmenu(HMENU aSrc, const std::wstring &aName)
{
	const int tCount = GetMenuItemCount(aSrc);
	for( int i = 0 ; i < tCount; ++i )
	{
		const int tLen = GetMenuString(aSrc, i, 0, 0, MF_BYPOSITION);
		std::vector<wchar_t> tNameVec(tLen + 1);
		GetMenuString(aSrc, i, &tNameVec[0], tLen + 1, MF_BYPOSITION);

		std::wstring tNameStr(tNameVec.begin(), tNameVec.end());

		if( CorrectMenuName(tNameStr) == aName )
			return i;
	}

	return -1;
}


HMENU CreateMenuSequence(HMENU aMenuRoot, const AlgorithmFactory::MenuSequence &aSequence)
{
	HMENU tMenu = aMenuRoot;
	for( size_t i = 0; i < aSequence.path.size(); ++i )
	{
		const std::wstring &tMenuName = aSequence.path[i];
		const bool tIsSubmenu = (i != (aSequence.path.size() - 1));

		int tSubMenu = FindSubmenu( tMenu, tMenuName );

		if( -1 == tSubMenu )
		{
			// create!
			const int tIndex = tIsSubmenu?0:aSequence.index;
			AppendMenu(tMenu, MF_STRING, tIndex, tMenuName.c_str());
			tSubMenu = FindSubmenu( tMenu, tMenuName );
		}

		assert( -1 != tSubMenu);

		//else
		//{
			// found!
		MENUITEMINFO tInfo = {0};
		tInfo.cbSize = sizeof(MENUITEMINFO);
		tInfo.fMask |= MIIM_SUBMENU;
		BOOL tRes = GetMenuItemInfo(tMenu, tSubMenu, TRUE, &tInfo);

		if( tIsSubmenu && !tInfo.hSubMenu )
		{
			tInfo.hSubMenu = CreateMenu();
			tRes = SetMenuItemInfo(tMenu, tSubMenu, TRUE, &tInfo);
		}

		if( tIsSubmenu )
			tMenu = tInfo.hSubMenu;
	}

	return tMenu;
}


void FillMenu(HWND hWnd)
{
	typedef AlgorithmFactory::MenuList MenuList;
	const MenuList tMenuList = AlgorithmFactory::getInstance().getMenuList();

	const HMENU tMenuRoot = GetMenu(hWnd);

	for( MenuList::const_iterator it = tMenuList.begin(), itEnd = tMenuList.end(); it != itEnd; ++it)
	{
		CreateMenuSequence(tMenuRoot, *it);
	}
}
