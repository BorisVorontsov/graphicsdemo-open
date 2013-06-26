#include "StdAfx.h"

#include "IAlgorithm.h"


IAlgorithm::IAlgorithm()
	:mWndCallback(0)
{
	setPerfomanceMode(false);
}

IAlgorithm::~IAlgorithm(void)
{
}

bool IAlgorithm::process(HDC hDC, const RECT &rcPicture, const RECT &rcCanvas, HWND hWndCallback)
{
	mWndCallback = hWndCallback;

	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	ULONG lW = rcPicture.right;
	ULONG lH = rcPicture.bottom;

	//Получаем пиксели изображения
	if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
		if (pPixels)
			delete[] pPixels;
		if (pBMI)
			delete pBMI;
		return false;
	}

	processImage(pBMI, pPixels, lBytesCnt, rcCanvas);

	//Присваиваем измененные пиксели
	SetImagePixels(hDC, lW, lH, pPixels, pBMI);

	delete[] pPixels;
	delete pBMI;

	mWndCallback = 0;

	return true;
}

void IAlgorithm::doProgressEvent(int aPercent)
{
	if (mWndCallback)
	{
		// TODO: PostMessage
		SendMessage(mWndCallback, 
			WM_GRAPHICSEVENT, 
			MAKEWPARAM(EVENT_ON_PROGRESS, 0),
			(LPARAM)aPercent);
	}
}

void IAlgorithm::doProgressEvent(int aCurrent, int aMax)
{
	const int tPersent = static_cast<int>(static_cast<double>(aCurrent) / static_cast<double>(aMax) * 100);
	doProgressEvent(tPersent);
}

void IAlgorithm::doNothing(int){}
void IAlgorithm::doNothing(int, int){}

void IAlgorithm::progressEvent(int aCurrent, int aMax)
{
	(this->*Progress2)(aCurrent, aMax);
}

void IAlgorithm::progressEvent(int aPercent)
{
	(this->*Progress1)(aPercent);
}


void IAlgorithm::setPerfomanceMode(bool aIsPerfomance)
{
	if( aIsPerfomance )
	{
		Progress1 = &IAlgorithm::doNothing;
		Progress2 = &IAlgorithm::doNothing;
	}
	else
	{
		Progress1 = &IAlgorithm::doProgressEvent;
		Progress2 = &IAlgorithm::doProgressEvent;
	}
}

