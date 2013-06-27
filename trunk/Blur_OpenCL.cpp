#include "StdAfx.h"

#include "Blur_OpenCL.h"

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
	cl::vector<cl::Event *> vEvents(2);

	//Создаем события ожидания работы CL-кода и ожидания получения результата
	vEvents.push_back(new cl::Event);
	vEvents.push_back(new cl::Event);

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
			cqQueue.enqueueNDRangeKernel(kKernelBlur, cl::NullRange, ndrGlobalThreads, ndrLocalThreads, 0, vEvents[0]);

			vEvents[0]->wait();
	
			//Читаем из выходного буфера в наш растр...
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
