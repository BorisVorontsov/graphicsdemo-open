#pragma once

#include <mutex>

namespace pattern
{
	template<class T>
	class SingletonStatic
	{
	public:
		static T& getInstance()
		{
			static T *volatile tInstancePtr = NULL;

			if( !tInstancePtr )
			{
				static std::mutex tMutex;
				std::lock_guard<std::mutex> tlock(tMutex);

				if( !tInstancePtr )
				{
					static T tInstance;
					tInstancePtr = &tInstance;
				}
			}

			return *tInstancePtr;
		}
	};
}
