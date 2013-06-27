#include "StdAfx.h"

#include "Blur_OpenCL.h"

#ifdef __USE_OPENCL__

//������ "�����", ����������� �� OpenCL (��� ���������� ���������� �� ����������� ����������, ��� ����������� �������� ��������)
//���������:
//	hDC				DC ����������
//	lW				������ DC
//	lH				������ DC
//	lLevel			������ ����������
//������������ ��������: TRUE � ������ ������, FALSE � ������ ������
BOOL Blur_OCL(HDC hDC, ULONG lW, ULONG lH, ULONG lLevel)
{
	LPBYTE pPixels = NULL;
	ULONG lBytesCnt = 0;
	LPBITMAPINFO pBMI = NULL;

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
		return FALSE;
	}

	//����� ������ ���������
	cl_context_properties cpContextProps[3] = 
		{CL_CONTEXT_PLATFORM, (cl_context_properties)(vPlatforms[0])(), 0};

	//�������� ������� ����� ���������� �� ������ ������������ ����������
	cl::Context cContext(CL_DEVICE_TYPE_GPU, cpContextProps, NULL, NULL, &intErr);
	if (intErr != CL_SUCCESS) {
		TCHAR lpMsg[255] = {};
		TCHAR lpPfName[128] = {}, lpPfVendor[128] = {};
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
			kKernelBlur.setArg(4, lLevel);

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

	return TRUE;
}

#endif
