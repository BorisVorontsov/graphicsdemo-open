#pragma once

typedef enum tagALPHAMODE
{
	AM_ALPHA_IGNORE = 0,		//������������� ������ �����-���������� �������
	AM_ALPHA_ADD,				//���������� �������� ����� � �����-���������� �������
	AM_ALPHA_SUBTRACT,			//��������� �������� ����� �� �����-���������� �������
	AM_ALPHA_REPLACE			//������ �����-���������� ������� �� ��������� �����
} ALPHAMODE;

//BOOL AlphaBlend(HDC hDstDC, ULONG lDstW, ULONG lDstH, HDC hSrcDC, ULONG lSrcW, ULONG lSrcH, BYTE bAlpha, ALPHAMODE AlphaMode, LPRECT pRC, HWND hWndCallback = nullptr);
