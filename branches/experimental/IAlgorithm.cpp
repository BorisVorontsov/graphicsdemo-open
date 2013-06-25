#include "StdAfx.h"

#include "IAlgorithm.h"


IAlgorithm::IAlgorithm()
	:mWndCallback(0)
{
}

IAlgorithm::~IAlgorithm(void)
{
}

bool IAlgorithm::process(HDC hDC, ULONG lW, ULONG lH, LPRECT pRC, HWND hWndCallback)
{
	mWndCallback = hWndCallback;

	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;

	//Получаем пиксели изображения
	if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
		if (pPixels)
			delete[] pPixels;
		if (pBMI)
			delete pBMI;
		return false;
	}

	processImage(pBMI, pPixels, lBytesCnt, pRC);

	//Присваиваем измененные пиксели
	SetImagePixels(hDC, lW, lH, pPixels, pBMI);

	delete[] pPixels;
	delete pBMI;

	mWndCallback = 0;

	return true;
}

void IAlgorithm::progressEvent(int aCurrent, int aMax)
{
	const int tPersent = static_cast<int>(static_cast<double>(aCurrent) / static_cast<double>(aMax) * 100);
	progressEvent(tPersent);
}

void IAlgorithm::progressEvent(int aPercent)
{
	//Параметры событий
	//ONPROGRESSPARAMS ONPP = {0};

	if (mWndCallback)
	{
		//ONPP.dwPercents = aPercent;

		// TODO: PostMessage
		PostMessage(mWndCallback, 
			WM_GRAPHICSEVENT, 
			MAKEWPARAM(EVENT_ON_PROGRESS, 0),
			(LPARAM)aPercent);
	}
}

void IAlgorithm::finishedEvent()
{
}
