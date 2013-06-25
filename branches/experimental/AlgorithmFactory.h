#pragma once

#include <functional>
#include <map>
#include <vector>

#include "IAlgorithm.h"

class AlgorithmFactory
{
	typedef std::function<IAlgorithm*()> AlgoCreator;

	struct AlgoFactory
	{
		int id;
		std::wstring menu;
		AlgoCreator method;
	};

	// menu->algorithm
	typedef std::map<int, AlgoFactory> FactoryMap;
	FactoryMap mAlgorithms;
public:
	AlgorithmFactory();
	~AlgorithmFactory();

	static AlgorithmFactory &getInstance();

	void regAlgorithm(const std::wstring &aMenu, AlgoCreator aFactory);

	struct MenuSequence
	{
		int index;
		std::vector<std::wstring> path;
	};

	typedef std::vector<MenuSequence> MenuList;

	MenuList getMenuList() const;
	std::auto_ptr<IAlgorithm> create(int aMenu);
};


#define TOKEN_JOIN(X, Y)		TOKEN_DO_JOIN(X, Y)
#define TOKEN_DO_JOIN(X, Y)		TOKEN_DO_JOIN2(X, Y)
#define TOKEN_DO_JOIN2(X, Y)	X##Y

// Регистрирует алгоритм в фабрике. 
// menu_path - путь меню в виде menu1|submenu|other submenu
// concrete_type тип регистрируемой фабрики

#define AUTO_REGISTER_ALGORITHM( menu_path ,concrete_type ) \
	namespace																												\
	{																														\
		IAlgorithm *TOKEN_JOIN(create_algorithm, __LINE__)()																\
		{																													\
				return new concrete_type;																					\
		}																													\
																															\
		struct TOKEN_JOIN(FactoryRegistrar, __LINE__)																		\
		{																													\
			TOKEN_JOIN(FactoryRegistrar, __LINE__)()																		\
			{																												\
				AlgorithmFactory::getInstance().regAlgorithm(menu_path, TOKEN_JOIN(create_algorithm, __LINE__));			\
			}																												\
		};																													\
		static TOKEN_JOIN(FactoryRegistrar, __LINE__) TOKEN_JOIN(__global_factory_registrar__, __LINE__ );					\
	}


// Регистрирует алгоритм в фабрике. 
// menu_path - путь меню в виде menu1|submenu|other submenu
// concrete_type тип регистрируемой фабрики
// arg1 - аргумент конструктора

#define AUTO_REGISTER_ALGORITHM1( menu_path ,concrete_type, arg1 ) \
	namespace																												\
	{																														\
		IAlgorithm *TOKEN_JOIN(create_algorithm, __LINE__)()																\
		{																													\
				return new concrete_type(arg1);																				\
		}																													\
																															\
		struct TOKEN_JOIN(FactoryRegistrar, __LINE__)																		\
		{																													\
			TOKEN_JOIN(FactoryRegistrar, __LINE__)()																		\
			{																												\
				AlgorithmFactory::getInstance().regAlgorithm(menu_path, TOKEN_JOIN(create_algorithm, __LINE__));			\
			}																												\
		};																													\
		static TOKEN_JOIN(FactoryRegistrar, __LINE__) TOKEN_JOIN(__global_factory_registrar__, __LINE__ );					\
	}


#define AUTO_REGISTER_ALGORITHM2( menu_path ,concrete_type, arg1, arg2 ) \
	namespace																												\
	{																														\
		IAlgorithm *TOKEN_JOIN(create_algorithm, __LINE__)()																\
		{																													\
				return new concrete_type(arg1, arg2);																				\
		}																													\
																															\
		struct TOKEN_JOIN(FactoryRegistrar, __LINE__)																		\
		{																													\
			TOKEN_JOIN(FactoryRegistrar, __LINE__)()																		\
			{																												\
				AlgorithmFactory::getInstance().regAlgorithm(menu_path, TOKEN_JOIN(create_algorithm, __LINE__));			\
			}																												\
		};																													\
		static TOKEN_JOIN(FactoryRegistrar, __LINE__) TOKEN_JOIN(__global_factory_registrar__, __LINE__ );					\
	}


#define AUTO_REGISTER_ALGORITHM3( menu_path ,concrete_type, arg1, arg2, arg3) \
	namespace																												\
	{																														\
		IAlgorithm *TOKEN_JOIN(create_algorithm, __LINE__)()																\
		{																													\
				return new concrete_type(arg1, arg2, arg3);																	\
		}																													\
																															\
		struct TOKEN_JOIN(FactoryRegistrar, __LINE__)																		\
		{																													\
			TOKEN_JOIN(FactoryRegistrar, __LINE__)()																		\
			{																												\
				AlgorithmFactory::getInstance().regAlgorithm(menu_path, TOKEN_JOIN(create_algorithm, __LINE__));			\
			}																												\
		};																													\
		static TOKEN_JOIN(FactoryRegistrar, __LINE__) TOKEN_JOIN(__global_factory_registrar__, __LINE__ );					\
	}


#define AUTO_REGISTER_ALGORITHM4( menu_path ,concrete_type, arg1, arg2, arg3, arg4) \
	namespace																												\
	{																														\
		IAlgorithm *TOKEN_JOIN(create_algorithm, __LINE__)()																\
		{																													\
				return new concrete_type(arg1, arg2, arg3, arg4);															\
		}																													\
																															\
		struct TOKEN_JOIN(FactoryRegistrar, __LINE__)																		\
		{																													\
			TOKEN_JOIN(FactoryRegistrar, __LINE__)()																		\
			{																												\
				AlgorithmFactory::getInstance().regAlgorithm(menu_path, TOKEN_JOIN(create_algorithm, __LINE__));			\
			}																												\
		};																													\
		static TOKEN_JOIN(FactoryRegistrar, __LINE__) TOKEN_JOIN(__global_factory_registrar__, __LINE__ );					\
	}


#define AUTO_REGISTER_ALGORITHM5( menu_path ,concrete_type, arg1, arg2, arg3, arg4, arg5) \
	namespace																												\
	{																														\
		IAlgorithm *TOKEN_JOIN(create_algorithm, __LINE__)()																\
		{																													\
				return new concrete_type(arg1, arg2, arg3, arg4, arg5);														\
		}																													\
																															\
		struct TOKEN_JOIN(FactoryRegistrar, __LINE__)																		\
		{																													\
			TOKEN_JOIN(FactoryRegistrar, __LINE__)()																		\
			{																												\
				AlgorithmFactory::getInstance().regAlgorithm(menu_path, TOKEN_JOIN(create_algorithm, __LINE__));			\
			}																												\
		};																													\
		static TOKEN_JOIN(FactoryRegistrar, __LINE__) TOKEN_JOIN(__global_factory_registrar__, __LINE__ );					\
	}
