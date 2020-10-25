#pragma once
// This should be included before any std header
#define PY_SSIZE_T_CLEAN
#include <python/Python.h>

#include <stdexcept>
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

#include <xxhash_cx/xxhash_cx.h>


using xxhash::literals::operator ""_xxh64;


#define PY_EXPORT_STATIC_FUNCTION(func, funcName, moduleName) Exporter<#moduleName##_xxh64>::RegisterFunction<decltype(&##func), &##func>(#funcName)
#define PY_EXPORT_GLOBAL_FUNCTION(funcName, moduleName) PY_EXPORT_STATIC_FUNCTION(funcName, funcName, moduleName);
//#define PY_EXPORT_MEMBER_FUNCTION(func, funcName, instanceReturner, moduleName) static auto funcName##instanceReturnFunction = instanceReturner; \
//	Exporter<#moduleName##_xxh64>::RegisterMemberFunction<decltype(&##func), &##func, decltype(&##funcName##instanceReturnFunction), &##funcName##instanceReturnFunction>(#funcName)
// TODO: Support Lambda https://qiita.com/angeart/items/94734d68999eca575881
#define PY_EXPORT_MEMBER_FUNCTION(func, funcName, instanceReturner, moduleName) Exporter<#moduleName##_xxh64>::RegisterMemberFunction<decltype(&##func), &##func, decltype(&##instanceReturner), &##instanceReturner>(#funcName)

#define PY_EXPORT_MODULE(moduleName) Exporter<#moduleName##_xxh64>::Export(#moduleName)


// TODO: Memory Leak (INCREF, DECREF)
// TODO: Define some macros for easy register


// TODO: Let the user define these easily
template<typename T, typename = void>
class Converter;


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



namespace python_embedder_detail
{
	template<typename ParameterTypes, size_t... Indices>
	auto get_parameter_tuple_helper(ParameterTypes, std::index_sequence<Indices...>)
		-> std::tuple<typename boost::mpl::at_c<ParameterTypes, Indices>::type...>;

	template<typename ParameterTypes, size_t ParameterCount>
	using ParameterTuple = decltype(get_parameter_tuple_helper(std::declval<ParameterTypes>(), std::make_index_sequence<ParameterCount>()));

	template<typename FirstParameterType, typename... ParameterTypes>
	std::tuple<ParameterTypes...> get_tails_parameter_tuple_helper(std::tuple<FirstParameterType, ParameterTypes...>);

	template<typename ParameterTypes, size_t ParameterCount>
	using TailsParameterTuple = decltype(get_tails_parameter_tuple_helper(std::declval<ParameterTuple<ParameterTypes, ParameterCount>>()));
	

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




	template<typename T>
	[[nodiscard]] static constexpr const char* get_fundamental_format_string() noexcept;

	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple>
	class Dispatcher
	{
	public:
		static PyObject* ReplicatedFunction(PyObject* self, PyObject* args);
	};

	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple,
		typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename InstanceReturnFunctionReturnType, size_t InstanceReturnFunctionParameterCount, typename InstanceReturnFunctionParameterTuple>
	class MemberDispatcher
	{
	public:
		static PyObject* ReplicatedFunction(PyObject* self, PyObject* args);
	};

}



template<unsigned long long ModuleNameHashValue>
class Exporter
{
public:
	template<typename FunctionPtrType, FunctionPtrType FunctionPtr>
	static void RegisterFunction(const char* functionName);


	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction>
	static void RegisterMemberFunction(const char* functionName);


	template<typename T>
	static void RegisterType(const char* typeName)
	{
		PyTypeObject typeObject{ PyVarObject_HEAD_INIT(nullptr, 0) };
		typeObject.tp_name = typeName;
		typeObject.tp_basicsize = sizeof(T);  // + sizeof(PyObject)?
		typeObject.tp_itemsize = 0;
		typeObject.tp_flags = Py_TPFLAGS_DEFAULT;
		typeObject.tp_new = PyType_GenericNew;
		
		types.push_back(typeObject);
	}


	static void Export(const std::string& moduleName_)
	{
		moduleName = moduleName_;
		methods.push_back(PyMethodDef{ nullptr, nullptr, 0, nullptr });
		PyImport_AppendInittab(moduleName.c_str(), Init);
	}


	static PyObject* Init()
	{
		static bool hasInitialized = false;
		if (hasInitialized)
		{
			throw std::runtime_error(moduleName + " is already initialized");
		}
		hasInitialized = true;
		
		moduleDef = {
			PyModuleDef_HEAD_INIT, moduleName.c_str(), nullptr, -1, methods.data(),
				nullptr, nullptr, nullptr, nullptr
		};
		
		PyObject* const pyModule = PyModule_Create(&moduleDef);

		for (PyTypeObject& type : types)
		{
			Py_INCREF(&type);
			PyModule_AddType(pyModule, &type);
		}
		
		return pyModule;
	}



private:
	inline static std::string moduleName;
	
	inline static std::vector<PyMethodDef> methods;
	inline static std::vector<PyTypeObject> types;
	
	inline static PyModuleDef moduleDef;
};





template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple>
PyObject* python_embedder_detail::Dispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple>::ReplicatedFunction([[maybe_unused]] PyObject* self, PyObject* args)
{
	ParameterTypeTuple arguments;
	
	bool parseResult = true;
	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<ParameterCount>>([&](auto i)
		{
			using NthParameterType = std::tuple_element_t<i, ParameterTypeTuple>;

			static_assert(std::is_default_constructible_v<NthParameterType>);
			static_assert(std::is_move_assignable_v<NthParameterType>);

			PyObject* const pyObjectValue = PyTuple_GetItem(args, i);
			TempVarType<NthParameterType> value;

			if constexpr (std::is_fundamental_v<NthParameterType>)
			{
				const char* format = get_fundamental_format_string<NthParameterType>();

				if (!PyArg_Parse(pyObjectValue, format, &value))
				{
					parseResult = false;
					return;
				}
			}
			else if constexpr (std::is_same_v<NthParameterType, const char*>)
			{
				if (!PyArg_Parse(pyObjectValue, "s", &value))
				{
					parseResult = false;
					return;
				}
			}
			else if constexpr (std::is_same_v<NthParameterType, std::string>)
			{
				const char* temp;
				if (!PyArg_Parse(pyObjectValue, "s", &temp))
				{
					parseResult = false;
					return;
				}

				value = temp;
			}
			else
			{
				if (!PyArg_Parse(pyObjectValue, "O&", &Converter<NthParameterType>::ParseValue, &value))
				{
					parseResult = false;
					return;
				}
			}

			std::get<i>(arguments) = std::move(static_cast<NthParameterType>(value));
		}
	);

	if (!parseResult)
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
			const char* format = get_fundamental_format_string<ReturnType>();

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


template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename InstanceReturnFunctionReturnType, size_t InstanceReturnFunctionParameterCount, typename InstanceReturnFunctionParameterTypeTuple>
PyObject* python_embedder_detail::MemberDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple, InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnFunctionReturnType, InstanceReturnFunctionParameterCount, InstanceReturnFunctionParameterTypeTuple>
	::ReplicatedFunction([[maybe_unused]] PyObject* self, PyObject* args)
{
	InstanceReturnFunctionParameterTypeTuple instanceReturnerArguments;

	PyObject* const instanceReturnFunctionArgs = PyTuple_GetItem(args, 0);
	
	bool parseResult = true;
	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<InstanceReturnFunctionParameterCount>>([&](auto i)
		{
			using NthParameterType = std::tuple_element_t<i, InstanceReturnFunctionParameterTypeTuple>;

			static_assert(std::is_default_constructible_v<NthParameterType>);
			static_assert(std::is_move_assignable_v<NthParameterType>);

			PyObject* const pyObjectValue = PyTuple_GetItem(instanceReturnFunctionArgs, i);
			TempVarType<NthParameterType> value;

			if constexpr (std::is_fundamental_v<NthParameterType>)
			{
				const char* format = get_fundamental_format_string<NthParameterType>();

				if (!PyArg_Parse(pyObjectValue, format, &value))
				{
					parseResult = false;
					return;
				}
			}
			else if constexpr (std::is_same_v<NthParameterType, const char*>)
			{
				if (!PyArg_Parse(pyObjectValue, "s", &value))
				{
					parseResult = false;
					return;
				}
			}
			else if constexpr (std::is_same_v<NthParameterType, std::string>)
			{
				const char* temp;
				if (!PyArg_Parse(pyObjectValue, "s", &temp))
				{
					parseResult = false;
					return;
				}

				value = temp;
			}
			else
			{
				if (!PyArg_Parse(pyObjectValue, "O&", &Converter<NthParameterType>::ParseValue, &value))
				{
					parseResult = false;
					return;
				}
			}

			std::get<i>(instanceReturnerArguments) = std::move(static_cast<NthParameterType>(value));
		}
	);
	
	if (!parseResult)
	{
		return nullptr;
	}
	
	

	ParameterTypeTuple arguments;

	PyObject* const realArgs = PyTuple_GetSlice(args, 1, ParameterCount);

	parseResult = true;
	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<ParameterCount - 1>>([&](auto i)
		{
			using NthParameterType = std::tuple_element_t<i, ParameterTypeTuple>;

			static_assert(std::is_default_constructible_v<NthParameterType>);
			static_assert(std::is_move_assignable_v<NthParameterType>);

			PyObject* const pyObjectValue = PyTuple_GetItem(realArgs, i);
			TempVarType<NthParameterType> value;

			if constexpr (std::is_fundamental_v<NthParameterType>)
			{
				const char* format = get_fundamental_format_string<NthParameterType>();

				if (!PyArg_Parse(pyObjectValue, format, &value))
				{
					parseResult = false;
					return;
				}
			}
			else if constexpr (std::is_same_v<NthParameterType, const char*>)
			{
				if (!PyArg_Parse(pyObjectValue, "s", &value))
				{
					parseResult = false;
					return;
				}
			}
			else if constexpr (std::is_same_v<NthParameterType, std::string>)
			{
				const char* temp;
				if (!PyArg_Parse(pyObjectValue, "s", &temp))
				{
					parseResult = false;
					return;
				}

				value = temp;
			}
			else
			{
				if (!PyArg_Parse(pyObjectValue, "O&", &Converter<NthParameterType>::ParseValue, &value))
				{
					parseResult = false;
					return;
				}
			}

			std::get<i>(arguments) = std::move(static_cast<NthParameterType>(value));
		}
	);

	if (!parseResult)
	{
		return nullptr;
	}

	
	InstanceReturnFunctionReturnType instance = std::apply(InstanceReturnFunction, instanceReturnerArguments);
	auto thisPtrAddedArguments = std::tuple_cat(std::make_tuple(instance), arguments);

	PyObject* toReturn;
	if constexpr (std::is_same_v<ReturnType, void>)
	{
		std::apply(FunctionPtr, thisPtrAddedArguments);

		toReturn = Py_None;
	}
	else
	{
		ReturnType returnValue = std::apply(FunctionPtr, thisPtrAddedArguments);

		if constexpr (std::is_fundamental_v<ReturnType>)
		{
			const char* format = get_fundamental_format_string<ReturnType>();

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



template <typename T>
constexpr const char* python_embedder_detail::get_fundamental_format_string() noexcept
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



template <unsigned long long ModuleNameHashValue>
template <typename FunctionPtrType, FunctionPtrType FunctionPtr>
void Exporter<ModuleNameHashValue>::RegisterFunction(const char* functionName)
{
	using namespace boost::function_types;

	static_assert(is_function_pointer<FunctionPtrType>::value);
	static_assert(!is_member_function_pointer<FunctionPtrType>::value);

	constexpr size_t parameterCount = function_arity<FunctionPtrType>::value;
	using ReturnType = typename result_type<FunctionPtrType>::type;
	using ParameterTypeTuple = python_embedder_detail::ParameterTuple<parameter_types<FunctionPtrType>, parameterCount>;

	using InstantiatedDispatcher = python_embedder_detail::Dispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple>;

	methods.push_back(PyMethodDef{ functionName, &InstantiatedDispatcher::ReplicatedFunction, METH_VARARGS, nullptr });
}


template <unsigned long long ModuleNameHashValue>
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction>
void Exporter<ModuleNameHashValue>::RegisterMemberFunction(const char* functionName)
{
	using namespace boost::function_types;

	static_assert(is_member_function_pointer<FunctionPtrType>::value);
	static_assert(is_function_pointer<InstanceReturnFunctionType>::value);

	constexpr size_t parameterCount = function_arity<FunctionPtrType>::value;
	using ReturnType = typename result_type<FunctionPtrType>::type;
	using ParameterTypeTuple = python_embedder_detail::TailsParameterTuple<parameter_types<FunctionPtrType>, parameterCount>;

	constexpr size_t instanceReturnerParameterCount = function_arity<InstanceReturnFunctionType>::value;
	using InstanceReturnerReturnType = typename result_type<InstanceReturnFunctionType>::type;
	using InstanceReturnerParameterTypeTuple = python_embedder_detail::ParameterTuple<parameter_types<InstanceReturnFunctionType>, instanceReturnerParameterCount>;

	using InstantiatedDispatcher = python_embedder_detail::MemberDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple,
		InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnerReturnType, instanceReturnerParameterCount, InstanceReturnerParameterTypeTuple>;

	methods.push_back(PyMethodDef{ functionName, &InstantiatedDispatcher::ReplicatedFunction, METH_VARARGS, nullptr });
}
