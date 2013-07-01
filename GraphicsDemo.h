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
	DWORD dwFltIndex;			//������ ���������� �������
	HWND hWndMain;				//������� ����
	HWND hWndCanvas;			//����, �� ������� ����� �������� ������
	HDC hDBDC;					//DC ������� �����������, � �������� ����� �������� ������. ���� �� �����, �� ����� ��������� ������� DC ���� hWndCanvas
	HWND hWndProgress;			//���� ��������� ���������
	HWND hWndPerfomanceInfo;
	POINT ptViewportOffset;		//����� �������� ������������ ��������� ��������� ���������� ������
	SIZE szCanvas;				//������� ������
	DWORD dwReserved;
} IMGPROCINFO, *LPIMGPROCINFO;

INT_PTR CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CanvasProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ProgressWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PerformanceWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
UINT WINAPI ImgProcThreadMain(LPVOID pArg);

BOOL GetAppPath(LPTSTR lpPath, DWORD dwPathLen, BOOL bAddQuotes = FALSE);

#endif
