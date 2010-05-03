#ifndef GRAPHICSDEMO_H
#define GRAPHICSDEMO_H

/*
#ifndef _INC_TIME
#include <time.h>
#endif
*/

#define	APP_NAME			TEXT("Graphics Demo")

#define GD_MIN_CANV_LEFT	10
#define GD_MIN_CANV_TOP		10

#define GD_MAX_CANV_WIDTH	1024
#define GD_MAX_CANV_HEIGHT	768

typedef struct tagIMGPROCINFO
{
	DWORD dwFltIndex;		//Индекс выбранного фильтра
	HWND hWndMain;			//Главное окно
	HWND hWndCanvas;		//Окно, на котором будет отбражен эффект
	HDC hDBDC;				//DC двойной буферизации, к которому будет применен эффект. Если не задан, то поток обработки возьмет DC окна hWndCanvas
	HWND hWndProgress;		//Окно прогресса обработки
	RECT rcPicture;			//Область внутри hDCCanvas для изменения
	DWORD dwReserved;
} IMGPROCINFO, *LPIMGPROCINFO;

INT_PTR CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CanvasProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ProgressWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
UINT WINAPI ImgProcThreadMain(LPVOID pArg);

#ifndef __USE_GDIPLUS__
void LoadPicture_Std(HWND hWndCanvas, HDC hDCCanvas, LPCTSTR lpFileName, HDC hDCSrc = NULL);
#else
void LoadPicture_Gdiplus(HWND hWndCanvas, HDC hDCCanvas, LPCTSTR lpFileName);
#endif

#ifndef __USE_GDIPLUS__
#define LoadPicture			LoadPicture_Std
#else
#define LoadPicture			LoadPicture_Gdiplus
#endif

#ifndef __USE_GDIPLUS__
void SavePicture_Std(HDC hDCCanvas, LPCTSTR lpFileName);
#else
void SavePicture_Gdiplus(HDC hDCCanvas, LPCTSTR lpFileName);
bool GetEncoderClsid(LPCTSTR lpFmt, CLSID* pClsid);
#endif

#ifndef __USE_GDIPLUS__
#define SavePicture			SavePicture_Std
#else
#define SavePicture			SavePicture_Gdiplus
#endif

BOOL GetAppPath(LPTSTR lpPath, DWORD dwPathLen, BOOL bAddQuotes = FALSE);

SIZE_T GetOpenDialog(HINSTANCE hInstance, HWND hWnd, LPCTSTR lpTitle, LPTSTR lpFileName,
					DWORD dwFNSize, LPCTSTR lpFilter, DWORD dwFilterIndex, BOOL bMultiSelect);
SIZE_T GetSaveDialog(HINSTANCE hInstance, HWND hWnd, LPCTSTR lpTitle, LPTSTR lpFileName,
					DWORD dwFNSize, LPCTSTR lpFilter, LPDWORD pFilterIndex, LPCTSTR lpDefExt,
					LPCTSTR lpInitialDir = NULL);

/*
__inline void Randomize(){
	srand((unsigned int)time(0));
}

__inline int Random(int val){
	return (val)?(rand()%(val)):0;
}
*/

#endif
