#pragma once

typedef enum tagALPHAMODE
{
	AM_ALPHA_IGNORE = 0,		//Использование только альфа-компоненты пикселя
	AM_ALPHA_ADD,				//Добавление значения альфы к альфа-компоненте пикселя
	AM_ALPHA_SUBTRACT,			//Вычитание значения альфы из альфа-компоненты пикселя
	AM_ALPHA_REPLACE			//Замена альфа-компоненты пикселя на указанную альфу
} ALPHAMODE;

//BOOL AlphaBlend(HDC hDstDC, ULONG lDstW, ULONG lDstH, HDC hSrcDC, ULONG lSrcW, ULONG lSrcH, BYTE bAlpha, ALPHAMODE AlphaMode, LPRECT pRC, HWND hWndCallback = nullptr);
