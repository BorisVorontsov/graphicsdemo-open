#pragma once

class IAlgorithm
{
	HWND mWndCallback;

	void doProgressEvent(int aPercent);
	void doProgressEvent(int aCurrent, int aMax);

	void doNothing(int);
	void doNothing(int, int);

	void (IAlgorithm::*Progress1)(int);
	void (IAlgorithm::*Progress2)(int, int);
protected:
	virtual void processImage(LPIMAGEDESCR pIMGDESCR, LPBYTE pPixels, ULONG lBytesCnt, const RECT &pRC) = 0;

	void progressEvent(int aPercent);
	void progressEvent(int aCurrent, int aMax);
public:
	IAlgorithm();
	virtual ~IAlgorithm();

	void setPerfomanceMode(bool aIsPerfomance);
	virtual bool process(HDC hDC, const RECT &rcPicture, const RECT &rcCanvas, HWND hWndCallback);
};

