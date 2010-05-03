/////////////////////////////////////////////////////////////////
//		"Graphics Demo"
//		© BV (Boris Vorontsov, mailto:borisvorontsov@gmail.com)
/////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS

#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <stdio.h>
#include <math.h>

#ifdef UNICODE

#ifndef _UNICODE
#define _UNICODE
#endif

#endif

#include <tchar.h>

#include "gddefs.h"

#ifdef __USE_GDIPLUS__

#include <gdiplus.h>
using namespace Gdiplus;

#pragma comment (lib, "gdiplus.lib")

#endif

#include "graphicsdemo.h"
#include "graphics.h"
#include "strparser.h"
#include "resource.h"

#pragma comment (lib, "comctl32.lib")

#pragma comment (linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' " \
	"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

static HINSTANCE hAppInstance;

static volatile SYSTEM_INFO SI = {0};

static TCHAR lpAppPath[MAX_PATH] = {0};
static TCHAR lpPicPath[MAX_PATH] = {0};

static LPCTSTR lpGDWndClass = TEXT("GraphicsDemo_WndClass");
static HWND hMainWnd;

static LONG_PTR pOldCanvasProc;

static HDC hDBDC = NULL;
static HBITMAP hDBBitmap, hOldDBBitmap;

static volatile HANDLE hImgProcThread;
static UINT uImgProcThreadID;

static volatile IMGPROCINFO IPI = {0};

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR lpCmdLine,
                       int nCmdShow)
{
	WNDCLASSEX wcex = {0};
	MSG msg;

	hAppInstance = hInstance;
	InitCommonControls();

#ifdef __USE_GDIPLUS__

	ULONG_PTR gdipToken;
	GdiplusStartupInput gdipStartupInput;
	GdiplusStartup(&gdipToken, &gdipStartupInput, 0);

#endif

	GetSystemInfo((LPSYSTEM_INFO)&SI);

	GetAppPath(lpAppPath, MAX_PATH);
	SP_ExtractDirectory(lpAppPath, lpPicPath);
	SP_AddDirSep(lpPicPath, lpPicPath);

#ifndef __USE_GDIPLUS__
	_tcscat(lpPicPath, TEXT("Sample.bmp"));
#else
	_tcscat(lpPicPath, TEXT("Sample.jpg"));
#endif

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

INT_PTR CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)APP_NAME);

			pOldCanvasProc = SetWindowLongPtr(GetDlgItem(hWnd, IDC_STCCANVAS), GWLP_WNDPROC, (LONG_PTR)CanvasProc);

			IPI.hWndMain = hWnd;
			IPI.hWndCanvas = GetDlgItem(hWnd, IDC_STCCANVAS);
			IPI.hWndProgress = CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_PROGRESS),
				IPI.hWndMain, ProgressWndProc, 0);

#ifdef __USE_OPENCL__
			EnableMenuItem(GetMenu(hWnd), IDM_FILTERS_BLUR_OCL, MF_BYCOMMAND | MF_ENABLED);
#endif

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
					TCHAR lpODFile[MAX_PATH] = {0};
					TCHAR lpODFilter[MAX_PATH] = {0};

#ifndef __USE_GDIPLUS__
					LoadString(hAppInstance, IDS_ODSTDFILTER, lpODFilter, MAX_PATH);
#else
					LoadString(hAppInstance, IDS_ODGDIPFILTER, lpODFilter, MAX_PATH);
#endif

					for (ULONG i = 0; i < MAX_PATH; i++) {
						if (lpODFilter[i] == '|')
							lpODFilter[i] = '\0';
					}
					_tcscpy(lpODFile, lpPicPath);
					if (GetOpenDialog(hAppInstance, hWnd, TEXT("Load Picture"), lpODFile,
						MAX_PATH - 1, lpODFilter, 1, FALSE))
					{
						_tcscpy(lpPicPath, lpODFile);
						PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_CLEAR, 0), 0);
					}
					break;
				}
				case IDM_FILE_SAVEPICTUREAS:
				{
					TCHAR lpSDFile[MAX_PATH] = {0};
					TCHAR lpSDFilter[MAX_PATH] = {0};
					TCHAR lpExt[64] = {0};
					DWORD dwFilterIndex = 1;

#ifndef __USE_GDIPLUS__
					LoadString(hAppInstance, IDS_SDSTDFILTER, lpSDFilter, MAX_PATH);
#else
					LoadString(hAppInstance, IDS_SDGDIPFILTER, lpSDFilter, MAX_PATH);
#endif

					for (ULONG i = 0; i < MAX_PATH; i++) {
						if (lpSDFilter[i] == '|')
							lpSDFilter[i] = '\0';
					}

					SP_ExtractName(lpPicPath, lpSDFile);
					SP_ExtractLeftPart(lpSDFile, lpSDFile, '.');

					if (GetSaveDialog(hAppInstance, hWnd, TEXT("Save Picture As"), lpSDFile,
						MAX_PATH - 1, lpSDFilter, &dwFilterIndex, NULL))
					{
						SP_ExtractRightPart(lpSDFile, lpExt, '.');
#ifndef __USE_GDIPLUS__
						if (_tcsicmp(lpExt, TEXT("bmp")) != 0)
							_tcscat(lpSDFile, TEXT(".bmp"));
#else
						switch (dwFilterIndex)
						{
							case 1: //BMP
								if (_tcsicmp(lpExt, TEXT("bmp")) != 0)
									_tcscat(lpSDFile, TEXT(".bmp"));
								break;

							case 2: //JPEG
								if (_tcsicmp(lpExt, TEXT("jpeg")) != 0) {
									if (_tcsicmp(lpExt, TEXT("jpg")) != 0)
										_tcscat(lpSDFile, TEXT(".jpg"));
								}
								break;
							case 3: //GIF
								if (_tcsicmp(lpExt, TEXT("gif")) != 0)
									_tcscat(lpSDFile, TEXT(".gif"));
								break;
							case 4: //TIFF
								if (_tcsicmp(lpExt, TEXT("tiff")) != 0) {
									if (_tcsicmp(lpExt, TEXT("tif")) != 0)
										_tcscat(lpSDFile, TEXT(".tif"));
								}
								break;
							case 5: //PNG
								if (_tcsicmp(lpExt, TEXT("png")) != 0)
									_tcscat(lpSDFile, TEXT(".png"));
								break;
						}
#endif
						SavePicture(hDBDC, lpSDFile);
					}
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
					
					LoadPicture(GetDlgItem(hWnd, IDC_STCCANVAS), hTmpDC, lpPicPath);
					
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
					MSGBOXPARAMS MBP = {0};
					MBP.cbSize = sizeof(MBP);
					MBP.hInstance = hAppInstance;
					MBP.dwStyle = MB_USERICON | MB_OK;
					MBP.hwndOwner = hWnd;
					MBP.lpszIcon = MAKEINTRESOURCE(IDI_ICON);
					MBP.lpszText = TEXT("Copyright © BV, 2007 - 2010 (borisvorontsov@gmail.com)");
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
							NULL, 0, &uImgProcThreadID);
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
					LPONPROGRESSPARAMS pONPP = (LPONPROGRESSPARAMS)lParam;
					SendDlgItemMessage(hWnd, IDC_PGBEP, PBM_SETPOS, pONPP->dwPercents, 0);
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

UINT WINAPI ImgProcThreadMain(LPVOID pArg)
{
	if (SI.dwNumberOfProcessors >= 2)
		SetThreadIdealProcessor(GetCurrentThread(), 1);

	HDC hDC = (IPI.hDBDC)?IPI.hDBDC:GetDC(IPI.hWndCanvas);

	RECT rcCanvas, rcPicture;
	CopyRect(&rcCanvas, (const LPRECT)&IPI.rcPicture);
	CopyRect(&rcPicture, (const LPRECT)&IPI.rcPicture);
	rcCanvas.left += 8;
	rcCanvas.top += 8;
	rcCanvas.right = rcCanvas.right - 8;
	rcCanvas.bottom = rcCanvas.bottom - 8;
	
	if (IPI.dwFltIndex != IDM_FILTERS_BLUR_OCL)
		ShowWindow(IPI.hWndProgress, SW_SHOW);

#ifdef __TESTING_MODE__
	LARGE_INTEGER intFreq, intStart, intEnd;
	TCHAR lpResult[128] = {0};
	QueryPerformanceFrequency(&intFreq);
	QueryPerformanceCounter(&intStart);
#endif

	switch (IPI.dwFltIndex)
	{
		case IDM_FILTERS_BLUR:
			Blur(hDC, rcPicture.right, rcPicture.bottom, 24, &rcCanvas, IPI.hWndProgress);
			break;
#ifdef __USE_OPENCL__
		case IDM_FILTERS_BLUR_OCL:
			Blur_OCL(hDC, rcPicture.right, rcPicture.bottom, 24);
			break;
#endif
		case IDM_FILTERS_RGBBALANCE:
			RGBBalance(hDC, rcPicture.right, rcPicture.bottom, -12, 4, -6, &rcCanvas, IPI.hWndProgress);
			break;
		case IDM_FILTERS_GRAYSCALE:
			GrayScale(hDC, rcPicture.right, rcPicture.bottom, &rcCanvas, IPI.hWndProgress);
			break;
		case IDM_FILTERS_GAMMACORRECTION:
			GammaCorrection(hDC, rcPicture.right, rcPicture.bottom, 0.9, &rcCanvas, IPI.hWndProgress);
			break;
		case IDM_FILTERS_EDGEDETECTION:
			EdgeDetection(hDC, rcPicture.right, rcPicture.bottom, RGB(255, 255, 255), &rcCanvas,
				IPI.hWndProgress);
			break;
		case IDM_FILTERS_MEDIAN:
			Median(hDC, rcPicture.right, rcPicture.bottom, 8, &rcCanvas, IPI.hWndProgress);
			break;
		case IDM_FILTERS_CONTRAST:
			Contrast(hDC, rcPicture.right, rcPicture.bottom, 32, &rcCanvas, IPI.hWndProgress);
			break;
		case IDM_TRANSFORMATION_SHEAR:
			Shear(hDC, rcPicture.right, rcPicture.bottom, -48, 0, RGB(255, 255, 255), &rcCanvas, IPI.hWndProgress);
			break;
		case IDM_TRANSFORMATION_ROTATE:
			Rotate(hDC, rcPicture.right, rcPicture.bottom, rcCanvas.right / 2 - 4, rcCanvas.bottom / 2 - 4,
				34, ROTATE_DIRECTION_CW, RGB(255, 255, 255), &rcCanvas, IPI.hWndProgress);
			break;
		case IDM_TRANSFORMATION_WAVES:
			Waves(hDC, rcPicture.right, rcPicture.bottom, 6, 48, WAVES_DIRECTION_WE, RGB(255, 255, 255),
				&rcCanvas, IPI.hWndProgress);
			break;
		default:
			//
			break;
	}

#ifdef __TESTING_MODE__
	QueryPerformanceCounter(&intEnd);
#endif

	if (IsWindowVisible(IPI.hWndProgress))
		ShowWindow(IPI.hWndProgress, SW_HIDE);

	if (IPI.hDBDC)
		RedrawWindow(IPI.hWndCanvas, NULL, NULL, RDW_NOERASE | RDW_INVALIDATE);

	if (!IPI.hDBDC)
		ReleaseDC(IPI.hWndCanvas, hDC);

#ifdef __TESTING_MODE__
	_stprintf(lpResult, TEXT("Время фильтра: %.02f сек."), (double)(intEnd.QuadPart - intStart.QuadPart) /
		(double)intFreq.QuadPart);
	MessageBox(NULL, lpResult, TEXT("Результат выполнения"), MB_ICONINFORMATION);
#endif

	hImgProcThread = NULL;
	return 0;
}

#ifndef __USE_GDIPLUS__

void LoadPicture_Std(HWND hWndCanvas, HDC hDCCanvas, LPCTSTR lpFileName, HDC hDCSrc)
{
	LONG lW, lH;
	HDC hDCCanvas1 = (hDCCanvas)?hDCCanvas:GetDC(hWndCanvas);

	if (hDCSrc == NULL)
	{
		HBITMAP hBitmap, hOldBitmap;
		BITMAP BMD = {0};
		hBitmap = (HBITMAP)LoadImage(hAppInstance, lpFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		GetObject(hBitmap, sizeof(BMD), &BMD);
		
		lW = BMD.bmWidth;
		lH = BMD.bmHeight;
		if (lW > GD_MAX_CANV_WIDTH) lW = GD_MAX_CANV_WIDTH;
		if (lH > GD_MAX_CANV_HEIGHT) lH = GD_MAX_CANV_HEIGHT;

		IPI.rcPicture.left = 0;
		IPI.rcPicture.top = 0;
		IPI.rcPicture.right = lW;
		IPI.rcPicture.bottom = lH;

		SetWindowPos(hWndCanvas, NULL, GD_MIN_CANV_LEFT, GD_MIN_CANV_TOP, lW, lH, SWP_NOZORDER);

		hDCSrc = CreateCompatibleDC(hDCCanvas1);
		hOldBitmap = (HBITMAP)SelectObject(hDCSrc, hBitmap);

		SetStretchBltMode(hDCCanvas1, STRETCH_HALFTONE);
		StretchBlt(hDCCanvas1, 0, 0, lW, lH, hDCSrc, 0, 0, BMD.bmWidth, BMD.bmHeight, SRCCOPY);

		DeleteObject(SelectObject(hDCSrc, hOldBitmap));
		DeleteDC(hDCSrc);
	}
	else
	{
		RECT rcCanvas = {0};
		GetClientRect(hWndCanvas, &rcCanvas);
		lW = rcCanvas.right;
		lH = rcCanvas.bottom;

		BitBlt(hDCCanvas1, 0, 0, lW, lH, hDCSrc, 0, 0, SRCCOPY);
	}

	if (!hDCCanvas)
		ReleaseDC(hWndCanvas, hDCCanvas1);
}

void SavePicture_Std(HDC hDCCanvas, LPCTSTR lpFileName)
{
	HANDLE hFile;
	BITMAPFILEHEADER BFH = {0};
	BITMAPINFO BMI = {0}, BMITmp = {0};
	LPBYTE pData = NULL;
	ULONG lDataSize, lColors, lPaletteSize, lWritten;
	HDC hCDC;
	HBITMAP hTmpBitmap, hOldBitmap;

	hFile = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (hFile != INVALID_HANDLE_VALUE)
	{
		hCDC = CreateCompatibleDC(hDCCanvas);
		hTmpBitmap = CreateCompatibleBitmap(hDCCanvas, GD_MAX_CANV_WIDTH, GD_MAX_CANV_HEIGHT);
		hOldBitmap = (HBITMAP)SelectObject(hCDC, hTmpBitmap);

		BitBlt(hCDC, 0, 0, GD_MAX_CANV_WIDTH, GD_MAX_CANV_HEIGHT, hDCCanvas, 0, 0, SRCCOPY);

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

#else

//Загрузка изображения посредством GDI+
//Парметры:
//	hWndCanvas		окно, для которого предназначено изображение. Функция автоматически изменит размер окна под размер изображения
//	hDCCanvas		DC, на котором будет отрисовано изображение. Если передать NULL, функция использует DC окна hWndCanvas
//	lpFileName		имя файла изображения
//Функция не возвращает значений
void LoadPicture_Gdiplus(HWND hWndCanvas, HDC hDCCanvas, LPCTSTR lpFileName)
{
	Graphics *pGraphics = 0;
	Image *pImage = 0;
	LONG lW = 0, lH = 0;

	HDC hDCCanvas1 = (hDCCanvas)?hDCCanvas:GetDC(hWndCanvas);
	
	pGraphics = new Graphics(hDCCanvas1);
	pImage = new Image(lpFileName);
	
	if (pImage->GetLastStatus() == Ok)
	{
		lW = pImage->GetWidth();
		lH = pImage->GetHeight();
		if (lW > GD_MAX_CANV_WIDTH) lW = GD_MAX_CANV_WIDTH;
		if (lH > GD_MAX_CANV_HEIGHT) lH = GD_MAX_CANV_HEIGHT;
		
		IPI.rcPicture.left = 0;
		IPI.rcPicture.top = 0;
		IPI.rcPicture.right = lW;
		IPI.rcPicture.bottom = lH;

		SetWindowPos(hWndCanvas, NULL, GD_MIN_CANV_LEFT, GD_MIN_CANV_TOP, lW, lH, SWP_NOZORDER);

		Rect rcImage(0, 0, IPI.rcPicture.right, IPI.rcPicture.bottom);
		pGraphics->DrawImage(pImage, rcImage);
	}

	delete pImage;
	delete pGraphics;
	if (!hDCCanvas)
		ReleaseDC(hWndCanvas, hDCCanvas1);
}

//Сохранение изображения посредством GDI+
//Параметры:
//	hDCCanvas		DC, с которого функция должна взять изображение
//	lpFileName		имя файла для сохранения изображения
//Функция не возвращает значений
void SavePicture_Gdiplus(HDC hDCCanvas, LPCTSTR lpFileName)
{
	HDC hCDC;
	HBITMAP hTmpBitmap, hOldBitmap;
	Image *pImage;
    CLSID EncClsid;
    EncoderParameters *pEncParams = NULL;
	ULONG uEncParamsSize;
	TCHAR lpExt[64] = {0};
	LPTSTR lpFmt;

    hCDC = CreateCompatibleDC(hDCCanvas);
	hTmpBitmap = CreateCompatibleBitmap(hDCCanvas, GD_MAX_CANV_WIDTH, GD_MAX_CANV_HEIGHT);
	hOldBitmap = (HBITMAP)SelectObject(hCDC, hTmpBitmap);

	BitBlt(hCDC, 0, 0, GD_MAX_CANV_WIDTH, GD_MAX_CANV_HEIGHT, hDCCanvas, 0, 0, SRCCOPY);

	pImage = Bitmap::FromHBITMAP(hTmpBitmap, NULL);

	SP_ExtractRightPart(lpFileName, lpExt, '.');

	if (_tcsicmp(lpExt, TEXT("bmp")) == 0)
		lpFmt = TEXT("image/bmp");
	else if ((_tcsicmp(lpExt, TEXT("jpg")) == 0) || (_tcsicmp(lpExt, TEXT("jpeg")) == 0))
		lpFmt = TEXT("image/jpeg");
	else if (_tcsicmp(lpExt, TEXT("gif")) == 0)
		lpFmt = TEXT("image/gif");
	else if ((_tcsicmp(lpExt, TEXT("tif")) == 0) || (_tcsicmp(lpExt, TEXT("tiff")) == 0))
		lpFmt = TEXT("image/tiff");
	else if (_tcsicmp(lpExt, TEXT("png")) == 0)
		lpFmt = TEXT("image/png");
	else
		lpFmt = TEXT("image/bmp");

    GetEncoderClsid(lpFmt, &EncClsid);

	Bitmap TmpBmp(1, 1);
	uEncParamsSize = TmpBmp.GetEncoderParameterListSize(&EncClsid);
	if (uEncParamsSize)
		pEncParams = (EncoderParameters*)new BYTE[uEncParamsSize];

	if ((_tcsicmp(lpExt, TEXT("jpg")) == 0) || (_tcsicmp(lpExt, TEXT("jpeg")) == 0))
	{
		ULONG uEncQualLvl = 100;

		pEncParams->Count = 1;
		pEncParams->Parameter[0].Guid = EncoderQuality;
		pEncParams->Parameter[0].Type = EncoderParameterValueTypeLong;
		pEncParams->Parameter[0].NumberOfValues = 1;
		pEncParams->Parameter[0].Value = &uEncQualLvl;
	}
	else if ((_tcsicmp(lpExt, TEXT("tif")) == 0) || (_tcsicmp(lpExt, TEXT("tiff")) == 0))
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

    pImage->Save(lpFileName, &EncClsid, pEncParams);

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

#endif

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

SIZE_T GetOpenDialog(HINSTANCE hInstance,
					HWND hWnd,
					LPCTSTR lpTitle,
					LPTSTR lpFileName,
					DWORD dwFNSize,
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
	OFN.nMaxFile = dwFNSize;
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
					DWORD dwFNSize,
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
	OFN.nMaxFile = dwFNSize;
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