#ifndef GRAPHICSDEMO_H
#define GRAPHICSDEMO_H

/*
#ifndef _INC_TIME
#include <time.h>
#endif
*/

#define	APP_NAME				TEXT("Graphics Demo")

#define GD_MIN_CANV_LEFT		10
#define GD_MIN_CANV_TOP			10

typedef struct tagIMGPROCINFO
{
	DWORD dwFltIndex;			//Индекс выбранного фильтра
	HWND hWndMain;				//Главное окно
	HWND hWndCanvas;			//Окно, на котором будет отбражен эффект
	HDC hDBDC;					//DC двойной буферизации, к которому будет применен эффект. Если не задан, то поток обработки возьмет DC окна hWndCanvas
	HWND hWndProgress;			//Окно прогресса обработки
	HWND hWndPerfomanceInfo;
	POINT ptViewportOffset;		//Сдвиг вьюпорта относительно начальных координат вторичного буфера
	SIZE szCanvas;				//Размеры холста
	DWORD dwReserved;
} IMGPROCINFO, *LPIMGPROCINFO;

INT_PTR CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CanvasProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ProgressWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PerformanceWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
UINT WINAPI ImgProcThreadMain(LPVOID pArg);

BOOL GetAppPath(LPTSTR lpPath, DWORD dwPathLen, BOOL bAddQuotes = FALSE);

#endif
