//////////////////////////////////////////////////////////////////////
//		Модуль для работы с растровыми изображениями
//		© BV, 2007 - 2010
//////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#define _WIN32_WINNT	0x0501

#include <windows.h>

#ifdef UNICODE

#ifndef _UNICODE
#define _UNICODE
#endif

#endif

#include <tchar.h>

#include "gddefs.h"

#ifdef __USE_OPENCL__

#define __NO_STD_VECTOR
#define __NO_STD_STRING

#include <fstream>

#include <malloc.h>
#include <CL/cl.hpp>

#pragma comment (lib, "opencl.lib")

//Количество потоков для одновременной обработки (чем больше, тем быстрее)
//ВАЖНО: количество потоков должно быть кратно размерности соответствующего вектора матрицы глобальных потоков (GlobalThreads)
#define OCL_WG_SIZE		64

#define BLUR_CL_FILE	"Blur_OpenCL.cl"
#endif

#include "graphicshf.h"
#include "graphics.h"

//-----------------------------------------------------------------------------------------
//Графические эффекты (фильтры)

//Фильтр "Пятно"
//Матрица:
//   n  n  n  n  n
//   n  1  1  1  n
//   n  1  1  1  n
//   n  1  1  1  n
//   n  n  n  n  n / 1
//Параметры:
//	hDC				DC назначения
//	lW				Высота DC
//	lH				Ширина DC
//	lLevel			Радиус размывания
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL Blur(HDC hDC, ULONG lW, ULONG lH, ULONG lLevel, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	ULONG lColor, lR, lG, lB, lPixels;
	LONG x, x1, x2, x3;
	LONG y, y1, y2, y3;

	//Параметры событий
	volatile ONPROGRESSPARAMS ONPP = {0};

	//Получаем пиксели изображения
	if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
		if (pPixels)
			delete[] pPixels;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}

	y = pRC->top;
	while (y < pRC->bottom)
	{
		y1 = y - (lLevel >> 1);
		if (y1 < pRC->top) y1 = pRC->top;
		y2 = y + (lLevel >> 1);
		if (y2 > (pRC->bottom - 1)) y2 = (pRC->bottom - 1);
		x = pRC->left;
		while (x < pRC->right)
		{
			x1 = x - (lLevel >> 1);
			if (x1 < pRC->left) x1 = pRC->left;
			x2 = x + (lLevel >> 1);
			if (x2 > (pRC->right - 1)) x2 = (pRC->right - 1);
			lR = 0;
			lG = 0;
			lB = 0;
			for (x3 = x1; x3 <= x2; x3++)
			{
				for (y3 = y1; y3 <= y2; y3++)
				{
					lColor = GetPixel(pPixels, pBMI, x3, y3);
					lR += R_BGR(lColor);
					lG += G_BGR(lColor);
					lB += B_BGR(lColor);
				}
			}
			lPixels = (x2 - x1 + 1) * (y2 - y1 + 1);
			lR /= lPixels;
			lG /= lPixels;
			lB /= lPixels;
			SetPixel(pPixels, pBMI, x, y, BGR(lB, lG, lR));
			x++;
		}
		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)y / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
		y++;
	}

	//Присваиваем измененные пиксели
	SetImagePixels(hDC, lW, lH, pPixels, pBMI);

	delete[] pPixels;
	delete pBMI;

	return TRUE;
}

#ifdef __USE_OPENCL__

//Фильтр "Пятно", выполненный на OpenCL (все вычисления происходят на графическом процессоре, что многократно повышает скорость)
//Параметры:
//	hDC				DC назначения
//	lW				Высота DC
//	lH				Ширина DC
//	lLevel			Радиус размывания
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL Blur_OCL(HDC hDC, ULONG lW, ULONG lH, ULONG lLevel)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;

	cl_int intErr;
	
	std::string strExePath, strSrcPath, strSrcCode;
	cl::vector<cl::Event> vEvents(2);

    //Инициализация OpenCL...
	//Получаем список платформ, поддерживающих OpenCL
    cl::vector<cl::Platform> vPlatforms;
    cl::Platform::get(&vPlatforms);
    if (vPlatforms.size() == 0) {
        MessageBox(NULL, TEXT("Платформ OpenCL не обнаружено!"), TEXT("Ошибка"), MB_ICONEXCLAMATION);
		return FALSE;
    }

	//Берем первую платформу
	cl_context_properties cpContextProps[3] = 
        {CL_CONTEXT_PLATFORM, (cl_context_properties)(vPlatforms[0])(), 0};

	//Пытаемся создать среду исполнения на основе графического процессора
    cl::Context cContext(CL_DEVICE_TYPE_GPU, cpContextProps, NULL, NULL, &intErr);
    if (intErr != CL_SUCCESS) {
		TCHAR lpMsg[255] = {0};
		TCHAR lpPfName[128] = {0}, lpPfVendor[128] = {0};
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, vPlatforms[0].getInfo<CL_PLATFORM_NAME>().c_str(), -1, lpPfName,
			sizeof(lpPfName) / sizeof(TCHAR));
		MultiByteToWideChar(CP_ACP, 0, vPlatforms[0].getInfo<CL_PLATFORM_VENDOR>().c_str(), -1, lpPfVendor,
			sizeof(lpPfVendor) / sizeof(TCHAR));
#else
		_tcscpy(lpPfName, vPlatforms[0].getInfo<CL_PLATFORM_NAME>().c_str());
		_tcscpy(lpPfVendor, vPlatforms[0].getInfo<CL_PLATFORM_VENDOR>().c_str());
#endif
		_stprintf(lpMsg, TEXT("Не удалось создать среду исполнения с устройством GPU на базе выбранной платформы ( %s, %s )!\nВсего найдено платформ: %i"),
			lpPfName, lpPfVendor, vPlatforms.size());
        MessageBox(NULL, lpMsg, TEXT("Ошибка"), MB_ICONEXCLAMATION);
		return FALSE;
    }

	if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
		if (pPixels)
			delete[] pPixels;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}

	strExePath.resize(MAX_PATH, '\0');
	GetModuleFileNameA(NULL, (LPSTR)strExePath.c_str(), (DWORD)strExePath.size());
	strExePath.resize(strExePath.rfind('\\') + 1);

	strSrcPath = strExePath;
	strSrcPath += BLUR_CL_FILE;

	//Загружаем код OpenCL
	if (LoadOpenCLSources(strSrcPath, strSrcCode))
	{
		//Получаем список устройств в среде исполнения
		cl::vector<cl::Device> vDevices = cContext.getInfo<CL_CONTEXT_DEVICES>();

		//Создаем объект "код"
		cl::Program::Sources sSources(1, std::make_pair(strSrcCode.c_str(), strSrcCode.size()));

		//Создаем объект "программа", из устройства и кода
		cl::Program pProgram(cContext, sSources);

		//Компилируем...
		if (pProgram.build(vDevices) == CL_SUCCESS)
		{
			//Создаем нашу функцию OpenCL (__kernel void Blur)
			cl::Kernel kKernelBlur(pProgram, "Blur", NULL);

			//Выделяем память под входной буфер
			cl::Buffer bInputImageBuffer = cl::Buffer(cContext, CL_MEM_READ_ONLY, lBytesCnt, NULL, NULL);

			//Аналогично, под буфер для результата
			cl::Buffer bOutputImageBuffer = cl::Buffer(cContext, CL_MEM_WRITE_ONLY, lBytesCnt, NULL, NULL);

			//Создаем очередь
			cl::CommandQueue cqQueue(cContext, vDevices[0], 0, NULL);

			//Записываем растр во входной буфер
			cqQueue.enqueueWriteBuffer(bInputImageBuffer, 1, 0, lBytesCnt, pPixels, 0, 0);

			//Входной буфер
			kKernelBlur.setArg(0, bInputImageBuffer);
			//Результат
			kKernelBlur.setArg(1, bOutputImageBuffer);
			//Ширина растра
			kKernelBlur.setArg(2, lW);
			//Высота
			kKernelBlur.setArg(3, lH);
			//Уровень
			kKernelBlur.setArg(4, lLevel);

			//Потоки из набор потоков для обработки изображения будут выполняться для каждого столбца растра (lW * lH для каждого пикселя, в случае двухмерного GT)
			size_t szGlobalThreads = lW;
			//Задаем количество потоков для обработки изображения (блок OCL_WG_SIZE * OCL_WG_SIZE, в случае двухмерного LT)
			size_t szLocalThreads = OCL_WG_SIZE;

			cl::NDRange ndrGlobalThreads(szGlobalThreads);

			//На всякий случай проверяем, поддерживает ли GPU заданое нами количество потоков
			size_t szKernelWGSize = kKernelBlur.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(vDevices[0], NULL);
			if (szLocalThreads > szKernelWGSize)
				szLocalThreads = szKernelWGSize;
			cl::NDRange ndrLocalThreads(szLocalThreads);

			//Запускаем код OpenCL на выполнение
			cqQueue.enqueueNDRangeKernel(kKernelBlur, cl::NullRange, ndrGlobalThreads, ndrLocalThreads, 0, &vEvents[0]);

			vEvents[0].wait();
	
			//Читаем из выходного буфера в наш растр...
			cqQueue.enqueueReadBuffer(bOutputImageBuffer, 1, 0, lBytesCnt, pPixels, 0, &vEvents[1]);

			vEvents[1].wait();

			cqQueue.finish();
		}
	}

	SetImagePixels(hDC, lW, lH, pPixels, pBMI);

	delete[] pPixels;
	delete pBMI;

	return TRUE;
}

#endif

//Фильтр "RGB-баланс"
//Параметры:
//	hDC				DC назначения
//	lW				Высота DC
//	lH				Ширина DC
//	lROffset		Степень смещения красного. Допустимые значения: от 0 до [-]255
//	lGOffset		Степень смещения зеленого. Допустимые значения: от 0 до [-]255
//	lBOffset		Степень смещения синего. Допустимые значения: от 0 до [-]255
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL RGBBalance(HDC hDC, ULONG lW, ULONG lH, LONG lROffset, LONG lGOffset, LONG lBOffset, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	ULONG lColor, lR, lG, lB;
	LONG i, j;

	volatile ONPROGRESSPARAMS ONPP = {0};

	if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
		if (pPixels)
			delete[] pPixels;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}

    for (j = pRC->top; j < pRC->bottom; j++)
    {
		for (i = pRC->left; i < pRC->right; i++)
		{
			lColor = GetPixel(pPixels, pBMI, i, j);

			lR = R_BGR(lColor);
			lG = G_BGR(lColor);
			lB = B_BGR(lColor);

			lR = (ULONG)CheckBounds((LONG)(lR += lROffset), (LONG)0, (LONG)255);
			lG = (ULONG)CheckBounds((LONG)(lG += lGOffset), (LONG)0, (LONG)255);
			lB = (ULONG)CheckBounds((LONG)(lB += lBOffset), (LONG)0, (LONG)255);

			SetPixel(pPixels, pBMI, i, j, BGR(lB, lG, lR));
		}
		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)j / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
    }

	SetImagePixels(hDC, lW, lH, pPixels, pBMI);

	delete[] pPixels;
	delete pBMI;

	return TRUE;
}

//Фильтр "Серые тона"
//Параметры:
//	hDC				DC назначения
//	lW				Высота DC
//	lH				Ширина DC
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL GrayScale(HDC hDC, ULONG lW, ULONG lH, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	ULONG lColor, lR, lG, lB, lS;
	LONG i, j;

	volatile ONPROGRESSPARAMS ONPP = {0};

	if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
		if (pPixels)
			delete[] pPixels;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}

    for (j = pRC->top; j < pRC->bottom; j++)
    {
		for (i = pRC->left; i < pRC->right; i++)
		{
			lColor = GetPixel(pPixels, pBMI, i, j);

			//BGR -> RGB
			lR = R_BGR(lColor);
			lG = G_BGR(lColor);
			lB = B_BGR(lColor);

			lS = (ULONG)(lR * 0.299 + lG * 0.587 + lB * 0.114);

			//SSS -> BGR
			SetPixel(pPixels, pBMI, i, j, BGR(lS, lS, lS));
		}
		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)j / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
    }

	SetImagePixels(hDC, lW, lH, pPixels, pBMI);

	delete[] pPixels;
	delete pBMI;

	return TRUE;
}

//Фильтр "Гамма-коррекция"
//Параметры:
//	hDC				DC назначения
//	lW				Высота DC
//	lH				Ширина DC
//	dblGamma		Степень гаммы. Допустимые значения: от 0.1 до 1.9
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL GammaCorrection(HDC hDC, ULONG lW, ULONG lH, double dblGamma, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	ULONG lColor, lR, lG, lB;
	LONG i, j;

	volatile ONPROGRESSPARAMS ONPP = {0};

	if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
		if (pPixels)
			delete[] pPixels;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}

    for (j = pRC->top; j < pRC->bottom; j++)
    {
		for (i = pRC->left; i < pRC->right; i++)
		{
			lColor = GetPixel(pPixels, pBMI, i, j);

			lR = R_BGR(lColor);
			lG = G_BGR(lColor);
			lB = B_BGR(lColor);

			lR = (ULONG)CheckBounds((LONG)((255.0 * pow((double)lR / 255.0, 1.0 / dblGamma))
				+ 0.5), (LONG)0, (LONG)255);
			lG = (ULONG)CheckBounds((LONG)((255.0 * pow((double)lG / 255.0, 1.0 / dblGamma))
				+ 0.5), (LONG)0, (LONG)255);
			lB = (ULONG)CheckBounds((LONG)((255.0 * pow((double)lB / 255.0, 1.0 / dblGamma))
				+ 0.5), (LONG)0, (LONG)255);

			SetPixel(pPixels, pBMI, i, j, BGR(lB, lG, lR));
		}
		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)j / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
    }

	SetImagePixels(hDC, lW, lH, pPixels, pBMI);

	delete[] pPixels;
	delete pBMI;

	return TRUE;
}

//Фильтр "Поиск граней"
//Матрица:
//  -1 -1 -1
//  -1  8 -1
//  -1 -1 -1 / 1 (сумма коэффициентов) [ опционально + color ]
//Параметры:
//	hDC				DC назначения
//	lW				Высота DC
//	lH				Ширина DC
//	crBkColor		Цвет, который будет использоваться как цвет фона
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL EdgeDetection(HDC hDC, ULONG lW, ULONG lH, COLORREF crBkColor, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels1 = NULL, pPixels2 = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	ULONG lColor[9], lR, lG, lB;
	LONG i, j, x, y;

	volatile ONPROGRESSPARAMS ONPP = {0};

	if (!GetImagePixels(hDC, lW, lH, &pPixels1, &lBytesCnt, &pBMI)) {
		if (pPixels1)
			delete[] pPixels1;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}
	pPixels2 = new BYTE[lBytesCnt];
	CopyMemory(pPixels2, pPixels1, lBytesCnt);

    for (j = pRC->top; j < pRC->bottom; j++)
    {
		for (i = pRC->left; i < pRC->right; i++)
		{
            x = (i == pRC->left)?pRC->left + 1:i;
			x = (i < (pRC->right - 1))?x:i - 1;
            y = (j == pRC->top)?pRC->top + 1:j;
			y = (j < (pRC->bottom - 1))?y:j - 1;

			lColor[0] = GetPixel(pPixels2, pBMI, x, y);
			lColor[1] = GetPixel(pPixels2, pBMI, x, y - 1);
			lColor[2] = GetPixel(pPixels2, pBMI, x + 1, y - 1);
			lColor[3] = GetPixel(pPixels2, pBMI, x + 1, y);
			lColor[4] = GetPixel(pPixels2, pBMI, x + 1, y + 1);
			lColor[5] = GetPixel(pPixels2, pBMI, x, y + 1);
			lColor[6] = GetPixel(pPixels2, pBMI, x - 1, y + 1);
			lColor[7] = GetPixel(pPixels2, pBMI, x - 1, y);
			lColor[8] = GetPixel(pPixels2, pBMI, x - 1, y - 1);

			lR = (-1 * R_BGR(lColor[1]) - 1 * R_BGR(lColor[2]) - 1 * R_BGR(lColor[3]) - 1 * R_BGR(lColor[4])
				 - 1 * R_BGR(lColor[5]) - 1 * R_BGR(lColor[6]) - 1 * R_BGR(lColor[7]) - 1 * R_BGR(lColor[8])
				 + (8 * R_BGR(lColor[0]))) / 1;// + GetRValue(crBkColor);
			lG = (-1 * G_BGR(lColor[1]) - 1 * G_BGR(lColor[2]) - 1 * G_BGR(lColor[3]) - 1 * G_BGR(lColor[4])
				 - 1 * G_BGR(lColor[5]) - 1 * G_BGR(lColor[6]) - 1 * G_BGR(lColor[7]) - 1 * G_BGR(lColor[8])
				 + (8 * G_BGR(lColor[0]))) / 1;// + GetGValue(crBkColor);
			lB = (-1 * B_BGR(lColor[1]) - 1 * B_BGR(lColor[2]) - 1 * B_BGR(lColor[3]) - 1 * B_BGR(lColor[4])
				 - 1 * B_BGR(lColor[5]) - 1 * B_BGR(lColor[6]) - 1 * B_BGR(lColor[7]) - 1 * B_BGR(lColor[8])
				 + (8 * B_BGR(lColor[0]))) / 1;// + GetBValue(crBkColor);

			lR = (ULONG)CheckBounds((LONG)lR, (LONG)0, (LONG)255);
			lG = (ULONG)CheckBounds((LONG)lG, (LONG)0, (LONG)255);
			lB = (ULONG)CheckBounds((LONG)lB, (LONG)0, (LONG)255);

			SetPixel(pPixels1, pBMI, i, j, BGR(lB, lG, lR));
		}
		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)j / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
    }

	SetImagePixels(hDC, lW, lH, pPixels1, pBMI);

	delete[] pPixels1;
	delete[] pPixels2;
	delete pBMI;

	return TRUE;
}

//Фильтр "Медиана"
//Параметры:
//	hDC				DC назначения
//	lW				Высота DC
//	lH				Ширина DC
//	lLevel			Радиус обработки
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL Median(HDC hDC, ULONG lW, ULONG lH, ULONG lLevel, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	LONG x, x1, x2, x3;
	LONG y, y1, y2, y3;
	ULONG lColor, lPixels, n;
	LPBYTE pRGBArr;

	volatile ONPROGRESSPARAMS ONPP = {0};

	if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
		if (pPixels)
			delete[] pPixels;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}

	y = pRC->top;
	while (y < pRC->bottom)
	{
		y1 = y - (lLevel >> 1);
		if (y1 < pRC->top) y1 = pRC->top;
		y2 = y + (lLevel >> 1);
		if (y2 > (pRC->bottom - 1)) y2 = (pRC->bottom - 1);
		x = pRC->left;
		while (x < pRC->right)
		{
			x1 = x - (lLevel >> 1);
			if (x1 < pRC->left) x1 = pRC->left;
			x2 = x + (lLevel >> 1);
			if (x2 > (pRC->right - 1)) x2 = (pRC->right - 1);
			lPixels = (x2 - x1 + 1) * (y2 - y1 + 1);
			//В целях оптимизации выделяем один массив на три компонента
			pRGBArr = new BYTE[lPixels * 3];
			n = 0;
			for (x3 = x1; x3 <= x2; x3++)
			{
				for (y3 = y1; y3 <= y2; y3++)
				{
					lColor = GetPixel(pPixels, pBMI, x3, y3);
					pRGBArr[n] = R_BGR(lColor);
					(pRGBArr + lPixels)[n] = G_BGR(lColor);
					(pRGBArr + (lPixels << 1))[n] = B_BGR(lColor);
					n++;
				}
			}
			SortArray_Shell(pRGBArr, lPixels);
			SortArray_Shell((pRGBArr + lPixels), lPixels);
			SortArray_Shell((pRGBArr + (lPixels << 1)), lPixels);
			//Вот по этому фильтр и называется медиана (серидина) -- берется середина
			//отсортированных массивов R/G/B
			n = ((lPixels - 1) >> 1);
			SetPixel(pPixels, pBMI, x, y, BGR((pRGBArr + (lPixels << 1))[n], (pRGBArr + lPixels)[n], pRGBArr[n]));
			delete[] pRGBArr;
			x++;
		}
		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)y / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
		y++;
	}

	SetImagePixels(hDC, lW, lH, pPixels, pBMI);

	delete[] pPixels;
	delete pBMI;

	return TRUE;
}

//Фильтр "Контрастность"
//Параметры:
//  hDC             DC назначения
//  lOffset         Степень изменения контрастности. Допустимые значения: от -100 до 100
//  pRC             Указатель на структуру RECT, определяющую область изображения для изменения
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL Contrast(HDC hDC, ULONG lW, ULONG lH, LONG lOffset, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
    ULONG lAB = 0;
	ULONG lColor;
	LONG i, j, lR, lG, lB, lLevel = abs(lOffset);

    volatile ONPROGRESSPARAMS ONPP = {0};

	if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
		if (pPixels)
			delete[] pPixels;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}

    for (j = pRC->top; j < pRC->bottom; j++) {
        for (i = pRC->left; i < pRC->right; i++) {
            lColor = GetPixel(pPixels, pBMI, i, j);

            lR = R_BGR(lColor);
            lG = G_BGR(lColor);
            lB = B_BGR(lColor);

            lAB += (ULONG)(lR * 0.299 + lG * 0.587 + lB * 0.114);
        }
    }

    lAB /= ((pRC->right - pRC->left) * (pRC->bottom - pRC->top));

    for (j = pRC->top; j < pRC->bottom; j++)
    {
        for (i = pRC->left; i < pRC->right; i++)
        {
            lColor = GetPixel(pPixels, pBMI, i, j);

            lR = R_BGR(lColor);
            lG = G_BGR(lColor);
            lB = B_BGR(lColor);

            lR = (ULONG)CheckBounds((LONG)(lR + ((lR - /*127.0*/(LONG)lAB) * ((double)((lOffset < 0)?-lLevel:lLevel) /
                (double)(201 - (-lLevel + 100))))), (LONG)0, (LONG)255);
            lG = (ULONG)CheckBounds((LONG)(lG + ((lG - /*127.0*/(LONG)lAB) * ((double)((lOffset < 0)?-lLevel:lLevel) /
                (double)(201 - (-lLevel + 100))))), (LONG)0, (LONG)255);
            lB = (ULONG)CheckBounds((LONG)(lB + ((lB - /*127.0*/(LONG)lAB) * ((double)((lOffset < 0)?-lLevel:lLevel) /
                (double)(201 - (-lLevel + 100))))), (LONG)0, (LONG)255);

            SetPixel(pPixels, pBMI, i, j, BGR(lB, lG, lR));
        }
        if (hWndCallback)
        {
            ONPP.dwPercents = (DWORD)(((double)j / (double)pRC->bottom) * 100);
            SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
                (LPARAM)&ONPP);
        }
    }

	SetImagePixels(hDC, lW, lH, pPixels, pBMI);

	delete[] pPixels;
	delete pBMI;

    return TRUE;
}

//-----------------------------------------------------------------------------------------
//Трансформация

//Сдвиг изображения
//Параметры:
//	hDC				DC назначения
//	lW				Высота DC
//	lH				Ширина DC
//	lX				Сдвиг (правого нижнего угла) по оси X
//	lY				Сдвиг (правого нижнего угла) по оси Y
//	crBkColor		Цвет, который будет использоваться как цвет фона
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL Shear(HDC hDC, ULONG lW, ULONG lH, LONG lX, LONG lY, COLORREF crBkColor, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels1 = NULL, pPixels2 = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	LONG i, j, x, x1, y, y1, sx, sy;

	volatile ONPROGRESSPARAMS ONPP = {0};

	if (!GetImagePixels(hDC, lW, lH, &pPixels1, &lBytesCnt, &pBMI)) {
		if (pPixels1)
			delete[] pPixels1;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}
	pPixels2 = new BYTE[lBytesCnt];
	CopyMemory(pPixels2, pPixels1, lBytesCnt);

	//RGB[A] -> BGR[A]
	ReverseBytes((LPBYTE)&crBkColor, 3);

	//Заливаем область для изменения цветом фона
    for (j = pRC->top; j < pRC->bottom; j++)
    {
		for (i = pRC->left; i < pRC->right; i++)
		{
			SetPixel(pPixels1, pBMI, i, j, crBkColor);
		}
	}

	//Вычисляем шаги для x и y
	if ((lX != 0)) sx = ((pRC->bottom - pRC->top) / abs(lX));
	if ((lY != 0)) sy = ((pRC->right - pRC->left) / abs(lY));

	x1 = 0;
    for (j = pRC->top; j < pRC->bottom; j++)
    {
		if ((lX != 0))
			if ((j % sx) == 0) x1++;
		y1 = 0;
		for (i = pRC->left; i < pRC->right; i++)
		{
			if ((lY != 0))
				if ((i % sy) == 0) y1++;

			if (lX != 0)
			{
				x = (lX > 0)?i - x1:i + x1;
			}
			else x = i;
			if (lY != 0)
			{
				y = (lY > 0)?j - y1:j + y1;
			}
			else y = j;

			if ((x < pRC->left) || (x > (pRC->right - 1))) continue;
			if ((y < pRC->top) || (y > (pRC->bottom - 1))) continue;

			SetPixel(pPixels1, pBMI, i, j, GetPixel(pPixels2, pBMI, x, y));
		}
		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)j / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
    }

	SetImagePixels(hDC, lW, lH, pPixels1, pBMI);

	delete[] pPixels1;
	delete[] pPixels2;
	delete pBMI;

	return TRUE;
}

//Поворот изображения на произвольный угол
//Формула:
//	x' = cos(rad) * (x - x0) +/- sin(rad) * (y - y0) + x0
//	y' = cos(rad) * (y - y0) -/+ sin(rad) * (x - x0) + y0
//Параметры:
//	hDC				DC назначения
//	lW				Высота DC
//	lH				Ширина DC
//	lX				Центр поворота по оси X
//	lY				Центр поворота по оси Y
//	lAngle			Угол поворота
//	lDirection		Направление (по часовой стрелке/против часовой стрелки)
//	crBkColor		Цвет, который будет использоваться как цвет фона
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL Rotate(HDC hDC, ULONG lW, ULONG lH, LONG lX, LONG lY, LONG lAngle, LONG lDirection, COLORREF crBkColor, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels1 = NULL, pPixels2 = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	LONG i, j, x, y;
	double dblRad;

	volatile ONPROGRESSPARAMS ONPP = {0};

	if (!GetImagePixels(hDC, lW, lH, &pPixels1, &lBytesCnt, &pBMI)) {
		if (pPixels1)
			delete[] pPixels1;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}
	pPixels2 = new BYTE[lBytesCnt];
	CopyMemory(pPixels2, pPixels1, lBytesCnt);

	dblRad = (double)(lAngle * (3.14159265358979 / 180));

	lX += pRC->left;
	lY += pRC->top;

	ReverseBytes((LPBYTE)&crBkColor, 3);

    for (j = pRC->top; j < pRC->bottom; j++)
    {
		for (i = pRC->left; i < pRC->right; i++)
		{
			SetPixel(pPixels1, pBMI, i, j, crBkColor);
		}
	}

    for (j = pRC->top; j < pRC->bottom; j++)
    {
		for (i = pRC->left; i < pRC->right; i++)
		{
			switch (lDirection)
			{
				case ROTATE_DIRECTION_CW:
					x = (LONG)(cos(dblRad) * (i - lX) + sin(dblRad) * (j - lY) + lX);
					y = (LONG)(cos(dblRad) * (j - lY) - sin(dblRad) * (i - lX) + lY);
					break;
				case ROTATE_DIRECTION_CCW:
					x = (LONG)(cos(dblRad) * (i - lX) - sin(dblRad) * (j - lY) + lX);
					y = (LONG)(cos(dblRad) * (j - lY) + sin(dblRad) * (i - lX) + lY);
					break;
				default:
					//
					break;
			}

			if ((x < pRC->left) || (x > (pRC->right - 1))) continue;
			if ((y < pRC->top) || (y > (pRC->bottom - 1))) continue;

			SetPixel(pPixels1, pBMI, i, j, GetPixel(pPixels2, pBMI, x, y));
		}
		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)j / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
    }

	SetImagePixels(hDC, lW, lH, pPixels1, pBMI);

	delete[] pPixels1;
	delete[] pPixels2;
	delete pBMI;

	return TRUE;
}

//Волны
//Формула:
//	x' = sin(y * 2Pi / freq) * amp + x
//	y' = sin(x * 2Pi / freq) * amp + y
//Параметры:
//	hDC				DC назначения
//	lW				Высота DC
//	lH				Ширина DC
//	lAmplitude		Амплитуда волн
//	lFrequency		Частота волн
//	lDirection		Направление (вертикальные волны/горизонтальные волны)
//	crBkColor		Цвет, который будет использоваться как цвет фона
//	pRC				Указатель на структуру RECT, определяющую область изображения для изменения
//Возвращаемое значение: TRUE в случае успеха, FALSE в случае ошибки
BOOL Waves(HDC hDC, ULONG lW, ULONG lH, LONG lAmplitude, LONG lFrequency, LONG lDirection, COLORREF crBkColor, LPRECT pRC, HWND hWndCallback)
{
	LPBYTE pPixels1 = NULL, pPixels2 = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;
	LONG i, j, x, y;
	double dblFunc;

	volatile ONPROGRESSPARAMS ONPP = {0};

	if (!GetImagePixels(hDC, lW, lH, &pPixels1, &lBytesCnt, &pBMI)) {
		if (pPixels1)
			delete[] pPixels1;
		if (pBMI)
			delete pBMI;
		return FALSE;
	}
	pPixels2 = new BYTE[lBytesCnt];
	CopyMemory(pPixels2, pPixels1, lBytesCnt);

	dblFunc = (double)((2 * 3.14159265358979) / lFrequency);

	ReverseBytes((LPBYTE)&crBkColor, 3);

    for (j = pRC->top; j < pRC->bottom; j++)
    {
		for (i = pRC->left; i < pRC->right; i++)
		{
			SetPixel(pPixels1, pBMI, i, j, crBkColor);
		}
	}

    for (j = pRC->top; j < pRC->bottom; j++)
    {
		for (i = pRC->left; i < pRC->right; i++)
		{
			switch (lDirection)
			{
				case WAVES_DIRECTION_NS:
					x = (LONG)(sin(j * dblFunc) * lAmplitude) + i;
					y = j;
					break;
				case WAVES_DIRECTION_WE:
					x = i;
					y = (LONG)(sin(i * dblFunc) * lAmplitude) + j;
					break;
				default:
					//
					break;
			}

			if ((x < pRC->left) || (x > (pRC->right - 1))) continue;
			if ((y < pRC->top) || (y > (pRC->bottom - 1))) continue;

			SetPixel(pPixels1, pBMI, i, j, GetPixel(pPixels2, pBMI, x, y));
		}
		if (hWndCallback)
		{
			ONPP.dwPercents = (DWORD)(((double)j / (double)pRC->bottom) * 100);
			SendMessage(hWndCallback, WM_GRAPHICSEVENT, MAKEWPARAM(EVENT_ON_PROGRESS, 0),
				(LPARAM)&ONPP);
		}
    }

	SetImagePixels(hDC, lW, lH, pPixels1, pBMI);

	delete[] pPixels1;
	delete[] pPixels2;
	delete pBMI;

	return TRUE;
}

