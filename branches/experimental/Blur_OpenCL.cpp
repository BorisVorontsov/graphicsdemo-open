#include "StdAfx.h"

#ifdef __USE_OPENCL__

#include "IAlgorithm.h"
#include "AlgorithmFactory.h"

//������ "�����", ����������� �� OpenCL (��� ���������� ���������� �� ����������� ����������, ��� ����������� �������� ��������)
//���������:
//	hDC				DC ����������
//	lW				������ DC
//	lH				������ DC
//	lLevel			������ ����������
//������������ ��������: TRUE � ������ ������, FALSE � ������ ������

class Blur_OCL
{
	ULONG mlLevel;

	virtual void processImage(LPBITMAPINFO pBMI, LPBYTE pPixels, ULONG lBytesCnt, LPRECT pRC){}

	virtual bool process(HDC hDC, const RECT &rcPicture, const RECT &rcCanvas, HWND /*hWndCallback*/)
	{
		LPBYTE pPixels = NULL;
		ULONG lBytesCnt = 0;
		LPBITMAPINFO pBMI = NULL;
		ULONG lW = rcPicture.right;
		ULONG lH = rcPicture.bottom;

		cl_int intErr;
	
		std::string strExePath, strSrcPath, strSrcCode;
		cl::vector<cl::Event *> vEvents(2);

		//������� ������� �������� ������ CL-���� � �������� ��������� ����������
		vEvents.push_back(new cl::Event);
		vEvents.push_back(new cl::Event);

		//������������� OpenCL...
		//�������� ������ ��������, �������������� OpenCL
		cl::vector<cl::Platform> vPlatforms;
		cl::Platform::get(&vPlatforms);
		if (vPlatforms.size() == 0) {
			MessageBox(NULL, TEXT("�������� OpenCL �� ����������!"), TEXT("������"), MB_ICONEXCLAMATION);
			return false;
		}

		//����� ������ ���������
		cl_context_properties cpContextProps[3] = 
			{CL_CONTEXT_PLATFORM, (cl_context_properties)(vPlatforms[0])(), 0};

		//�������� ������� ����� ���������� �� ������ ������������ ����������
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
			_stprintf(lpMsg, TEXT("�� ������� ������� ����� ���������� � ����������� GPU �� ���� ��������� ��������� ( %s, %s )!\n����� ������� ��������: %i"),
				lpPfName, lpPfVendor, vPlatforms.size());
			MessageBox(NULL, lpMsg, TEXT("������"), MB_ICONEXCLAMATION);
			return false;
		}

		if (!GetImagePixels(hDC, lW, lH, &pPixels, &lBytesCnt, &pBMI)) {
			if (pPixels)
				delete[] pPixels;
			if (pBMI)
				delete pBMI;
			return false;
		}

		strExePath.resize(MAX_PATH, '\0');
		GetModuleFileNameA(NULL, (LPSTR)strExePath.c_str(), (DWORD)strExePath.size());
		strExePath.resize(strExePath.rfind('\\') + 1);

		strSrcPath = strExePath;
		strSrcPath += BLUR_CL_FILE;

		//��������� ��� OpenCL
		if (LoadOpenCLSources(strSrcPath, strSrcCode))
		{
			//�������� ������ ��������� � ����� ����������
			cl::vector<cl::Device> vDevices = cContext.getInfo<CL_CONTEXT_DEVICES>();

			//������� ������ "���"
			cl::Program::Sources sSources(1, std::make_pair(strSrcCode.c_str(), strSrcCode.size()));

			//������� ������ "���������", �� ���������� � ����
			cl::Program pProgram(cContext, sSources);

			//�����������...
			if (pProgram.build(vDevices) == CL_SUCCESS)
			{
				//������� ���� ������� OpenCL (__kernel void Blur)
				cl::Kernel kKernelBlur(pProgram, "Blur", NULL);

				//�������� ������ ��� ������� �����
				cl::Buffer bInputImageBuffer = cl::Buffer(cContext, CL_MEM_READ_ONLY, lBytesCnt, NULL, NULL);

				//����������, ��� ����� ��� ����������
				cl::Buffer bOutputImageBuffer = cl::Buffer(cContext, CL_MEM_WRITE_ONLY, lBytesCnt, NULL, NULL);

				//������� �������
				cl::CommandQueue cqQueue(cContext, vDevices[0], 0, NULL);

				//���������� ����� �� ������� �����
				cqQueue.enqueueWriteBuffer(bInputImageBuffer, 1, 0, lBytesCnt, pPixels, 0, 0);

				//������� �����
				kKernelBlur.setArg(0, bInputImageBuffer);
				//���������
				kKernelBlur.setArg(1, bOutputImageBuffer);
				//������ ������
				kKernelBlur.setArg(2, lW);
				//������
				kKernelBlur.setArg(3, lH);
				//�������
				kKernelBlur.setArg(4, mlLevel);

				//������ �� ����� ������� ��� ��������� ����������� ����� ����������� ��� ������� ������� ������ (lW * lH ��� ������� �������, � ������ ����������� GT)
				size_t szGlobalThreads = lW;
				//������ ���������� ������� ��� ��������� ����������� (���� OCL_WG_SIZE * OCL_WG_SIZE, � ������ ����������� LT)
				size_t szLocalThreads = OCL_WG_SIZE;

				cl::NDRange ndrGlobalThreads(szGlobalThreads);

				//�� ������ ������ ���������, ������������ �� GPU ������� ���� ���������� �������
				size_t szKernelWGSize = kKernelBlur.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(vDevices[0], NULL);
				if (szLocalThreads > szKernelWGSize)
					szLocalThreads = szKernelWGSize;
				cl::NDRange ndrLocalThreads(szLocalThreads);

				//��������� ��� OpenCL �� ����������
				cqQueue.enqueueNDRangeKernel(kKernelBlur, cl::NullRange, ndrGlobalThreads, ndrLocalThreads, 0, vEvents[0]);

				vEvents[0]->wait();
	
				//������ �� ��������� ������ � ��� �����...
				cqQueue.enqueueReadBuffer(bOutputImageBuffer, 1, 0, lBytesCnt, pPixels, 0, vEvents[1]);

				vEvents[1]->wait();

				cqQueue.finish();
			}
		}

		SetImagePixels(hDC, lW, lH, pPixels, pBMI);

		delete[] pPixels;
		delete pBMI;

		return true;
	}
public:
	Blur_OCL(ULONG lLevel)
		:mlLevel(lLevel)
	{
	}
};


AUTO_REGISTER_ALGORITHM1( L"Filters|Blur (OpenCL)|8px",  Blur_OCL, 8);
AUTO_REGISTER_ALGORITHM1( L"Filters|Blur (OpenCL)|16px",  Blur_OCL, 16);
AUTO_REGISTER_ALGORITHM1( L"Filters|Blur (OpenCL)|24px",  Blur_OCL, 24);

#endif
