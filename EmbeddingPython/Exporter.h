#pragma once
// This should be included before any std header
#define PY_SSIZE_T_CLEAN
#include <python/Python.h>

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <boost/mp11/algorithm.hpp>  // mp_for_each, mp_iota_c

#include <boost/mpl/at.hpp>  // at_c

#include <boost/function_types/is_function_pointer.hpp>
#include <boost/function_types/is_member_function_pointer.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>


// TODO: Memory Leak (INCREF, DECREF)


// TODO: Let the user define these easily
template<typename T, typename = void>
class Converter
{
public:
	static int ParseValue(PyObject* pyObject, T* pT)
	{

	}

	static PyObject* BuildValue(T* t)
	{

	}
};



template<typename T>
class Converter<T, std::enable_if_t<std::is_trivially_copyable_v<T>>>
{
public:
	static int ParseValue(PyObject* pyObject, T* pT)
	{
		*pT = *reinterpret_cast<T*>(Py_TYPE(pyObject));

		return 1;
	}

	static PyObject* BuildValue(T* t)
	{

	}
};



namespace detail
{
	template<typename ParameterTypes, ::std::size_t... Indices>
	auto get_parameter_tuple_helper(ParameterTypes, ::std::index_sequence<Indices...>) -> ::std::tuple<typename boost::mpl::at_c<ParameterTypes, Indices>::type...>;

	template<typename ParameterTypes, ::std::size_t ParameterCount>
	using ParameterTuple = decltype(get_parameter_tuple_helper(::std::declval<ParameterTypes>(), ::std::make_index_sequence<ParameterCount>()));


	template<typename ParameterType, typename = void>
	struct TempVar
	{
		using type = ParameterType;
	};

	template<typename ParameterType>
	struct TempVar<ParameterType, std::enable_if_t<sizeof(ParameterType) < sizeof(int)>>
	{
		using type = int;
	};

	template<typename ParameterType>
	using TempVarType = typename TempVar<ParameterType>::type;
}




// TODO: compile time string hash, make the module name as a template parameter

class Exporter
{
public:
	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypes>
	class Dispatcher
	{
	public:
		static PyObject* ReplicatedFunction([[maybe_unused]] PyObject* self, PyObject* args)
		{
			detail::ParameterTuple<ParameterTypes, ParameterCount> arguments;

			bool result = true;
			boost::mp11::mp_for_each<boost::mp11::mp_iota_c<ParameterCount>>([&](auto i)
				{
					using NthParameterType = typename boost::mpl::at_c<ParameterTypes, i>::type;

					static_assert(std::is_default_constructible_v<NthParameterType>);
					static_assert(std::is_move_assignable_v<NthParameterType>);
				
					PyObject* const pyObjectValue = PyTuple_GetItem(args, i);
					detail::TempVarType<NthParameterType> value;

					if constexpr (std::is_fundamental_v<NthParameterType>)
					{
						const char* format = getFundamentalFormatString<NthParameterType>();

						if (!PyArg_Parse(pyObjectValue, format, &value))
						{
							result = false;
							return;
						}
					}
					else if constexpr (std::is_same_v<NthParameterType, const char*>)
					{
						if (!PyArg_Parse(pyObjectValue, "s", &value))
						{
							result = false;
							return;
						}
					}
					else if constexpr (std::is_same_v<NthParameterType, std::string>)
					{
						const char* temp;
						if (!PyArg_Parse(pyObjectValue, "s", &temp))
						{
							result = false;
							return;
						}

						value = temp;
					}
					else
					{
						if (!PyArg_Parse(pyObjectValue, "O&", &Converter<NthParameterType>::ParseValue, &value))
						{
							result = false;
							return;
						}
					}
				
					std::get<i>(arguments) = std::move(static_cast<NthParameterType>(value));
				}
			);

			if (!result)
			{
				return nullptr;
			}

			PyObject* toReturn;
			if constexpr (std::is_same_v<ReturnType, void>)
			{
				std::apply(FunctionPtr, arguments);

				toReturn = Py_None;
			}
			else
			{
				ReturnType returnValue = std::apply(FunctionPtr, arguments);
				
				if constexpr (std::is_fundamental_v<ReturnType>)
				{
					const char* format = getFundamentalFormatString<ReturnType>();

					toReturn = Py_BuildValue(format, returnValue);
				}
				else if constexpr (std::is_same_v<ReturnType, const char*>)
				{
					toReturn = Py_BuildValue("s", returnValue);
				}
				else if constexpr (std::is_same_v<ReturnType, std::string>)
				{
					toReturn = Py_BuildValue("s", returnValue.c_str());
				}
				else
				{
					toReturn = Py_BuildValue("O&", &Converter<ReturnType>::BuildValue, returnValue);
				}
			}
			
			Py_INCREF(toReturn);
			return toReturn;
		}
	};


	template<typename FunctionPtrType, FunctionPtrType FunctionPtr>
	static void RegisterFunction(const std::string& moduleName, const char* functionName)
	{
		using namespace boost::function_types;

		static_assert(is_function_pointer<FunctionPtrType>::value);
		static_assert(!is_member_function_pointer<FunctionPtrType>::value);

		constexpr size_t parameterCount = function_arity<FunctionPtrType>::value;
		using ReturnType = typename result_type<FunctionPtrType>::type;
		using ParameterTypes = parameter_types<FunctionPtrType>;

		using InstantiatedDispatcher = Dispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypes>;
		
		methods[moduleName].push_back(PyMethodDef{ functionName, &InstantiatedDispatcher::ReplicatedFunction, METH_VARARGS, nullptr });
	}


	template<typename Function, typename ClassType>
	static void RegisterMemberFunction(const char* moduleName, const char* functionName, Function* functionPtr, ClassType* instance)
	{

	}


	template<typename Function, typename InstanceReturner>
	static void RegisterMemberFunction(const char* moduleName, const char* functionName, Function* functionPtr, InstanceReturner instance)
	{

	}


	static void Export(const std::string& moduleName_)
	{
		moduleName = moduleName_;
		methods.at(moduleName_).push_back(PyMethodDef{ nullptr, nullptr, 0, nullptr });
		PyImport_AppendInittab(moduleName.c_str(), Init);
	}


	static PyObject* Init()
	{
		moduleDef = {
			PyModuleDef_HEAD_INIT, moduleName.c_str(), nullptr, -1, methods.at(moduleName).data(),
				nullptr, nullptr, nullptr, nullptr
		};
		
		return PyModule_Create(&moduleDef);
	}



private:
	inline static std::string moduleName;
	
	inline static std::unordered_map<std::string, std::vector<PyMethodDef>> methods;
	inline static PyModuleDef moduleDef;



	template<typename T>
	[[nodiscard]] static constexpr const char* getFundamentalFormatString() noexcept
	{
		static_assert(std::is_fundamental_v<T>);

		if constexpr (std::is_same_v<T, bool>)
		{
			return "p";
		}
		// Need an assumption that the argument will never be a number, rather a string with single character
		else if constexpr (std::is_same_v<T, char>)
		{
			return "C";
		}
		else if constexpr (std::is_same_v<T, unsigned char>)
		{
			return "B";
		}
		else if constexpr (std::is_same_v<T, short>)
		{
			return "h";
		}
		else if constexpr (std::is_same_v<T, unsigned short>)
		{
			return "H";
		}
		else if constexpr (std::is_same_v<T, int>)
		{
			return "i";
		}
		else if constexpr (std::is_same_v<T, unsigned int>)
		{
			return "I";
		}
		else if constexpr (std::is_same_v<T, long>)
		{
			return "l";
		}
		else if constexpr (std::is_same_v<T, unsigned long>)
		{
			return "k";
		}
		else if constexpr (std::is_same_v<T, long long>)
		{
			return "L";
		}
		else if constexpr (std::is_same_v<T, unsigned long long>)
		{
			return "K";
		}
		else if constexpr (std::is_same_v<T, float>)
		{
			return "f";
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			return "d";
		}
		/// long double is not supported by python API
		else
		{
			return nullptr;
		}
	}
};
