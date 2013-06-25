#include "StdAfx.h"
#include "AlgorithmFactory.h"


#include <mutex>
#include <assert.h>
#include <sstream>

AlgorithmFactory::AlgorithmFactory(void)
{
}


AlgorithmFactory::~AlgorithmFactory(void)
{
}

AlgorithmFactory &AlgorithmFactory::getInstance()
{
	static AlgorithmFactory *volatile tInstancePtr = NULL;

	if( !tInstancePtr )
	{
		static std::mutex tMutex;
		std::lock_guard<std::mutex> tlock(tMutex);

		if( !tInstancePtr )
		{
			static AlgorithmFactory tInstance;
			tInstancePtr = &tInstance;
		}
	}

	return *tInstancePtr;
}


void AlgorithmFactory::regAlgorithm(const std::wstring &aMenu, AlgoCreator aFactory)
{
	static int tIndex = 1;

	AlgoFactory tFactory;
	tFactory.id = tIndex++;
	tFactory.menu = aMenu;
	tFactory.method = aFactory;

	mAlgorithms[tFactory.id] = tFactory;
}


std::vector<std::wstring> &split(const std::wstring &s, wchar_t delim, std::vector<std::wstring> &elems) {
    std::wstringstream ss(s);
    std::wstring item;
    while (std::getline(ss, item, delim)) 
	{
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::wstring> split(const std::wstring &s, wchar_t delim) {
    std::vector<std::wstring> elems;
    split(s, delim, elems);
    return elems;
}

AlgorithmFactory::MenuList AlgorithmFactory::getMenuList() const
{
	MenuList tResult;
	tResult.reserve(mAlgorithms.size());

	for( FactoryMap::const_iterator it = mAlgorithms.begin(), itEnd = mAlgorithms.end(); it != itEnd; ++it)
	{
		const std::wstring &tSequenceStr = it->second.menu;
		MenuSequence tSeq;
		tSeq.index = it->first;
		tSeq.path = split(tSequenceStr, L'|');
		tResult.push_back(tSeq);
	}

	return tResult;
}


std::auto_ptr<IAlgorithm> AlgorithmFactory::create(int aMenu)
{
	std::auto_ptr<IAlgorithm> tResult;

	FactoryMap::iterator it = mAlgorithms.find(aMenu);
	if( it != mAlgorithms.end() )
	{
		tResult.reset( (it->second.method)() );
	}

	return tResult;
}

