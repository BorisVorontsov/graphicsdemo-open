#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#define _WIN32_WINNT	0x0501

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

#include "GDDefs.h"

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

#ifdef __USE_GDIPLUS__

#include <gdiplus.h>
using namespace Gdiplus;

#pragma comment (lib, "gdiplus.lib")

#endif

#include "GraphicsHelperFunctions.h"

///////////////////////////////////////////////////////////////////////////
//Секция общих определений для функций графических фильтров
///////////////////////////////////////////////////////////////////////////

//LOWORD(wParam): event ID, HIWORD(wParam): reserved, lParam: LPON_event_name_PARAMS 
#define WM_GRAPHICSEVENT		WM_USER + 0xFF

#define EVENT_ON_PROGRESS		0x10

typedef struct tagONPROGRESSPARAMS
{
	DWORD dwPercents;
} ONPROGRESSPARAMS, *LPONPROGRESSPARAMS;

///////////////////////////////////////////////////////////////////////////