#pragma once

#include <Windows.h>
#include <string>

class IAlgorithm
{
	HWND mWndCallback;
protected:
	virtual void processImage(LPBITMAPINFO pBMI, LPBYTE pPixels, ULONG lBytesCnt, LPRECT pRC) = 0;

	void progressEvent(int aPercent);
	void progressEvent(int aCurrent, int aMax);
	void finishedEvent();
public:
	IAlgorithm();
	virtual ~IAlgorithm();

	virtual bool process(HDC hDC, ULONG lW, ULONG lH, LPRECT pRC, HWND hWndCallback);
};

