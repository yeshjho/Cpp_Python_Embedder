/***********************************************************************************
 * Cpp_Python_Embedder
 * https://github.com/yeshjho/Cpp_Python_Embedder
 *
 *
 * MIT License
 * 
 * Copyright (c) 2020 Joonho Hwang
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ***********************************************************************************/

#pragma once
// This should be included before any std header
#define PY_SSIZE_T_CLEAN
#include <python/Python.h>
#include <python/structmember.h>

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

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/pop_back.hpp>
#include <boost/preprocessor/seq/reverse.hpp>

#include <xxhash_cx/xxhash_cx.h>


namespace cpp_python_embedder {
	
using xxhash::literals::operator ""_xxh64;


#define PY_EXPORTER_FIELD(T, memberName) { #memberName, python_embedder_detail::get_member_type_number<decltype(T::##memberName)>(), offsetof(python_embedder_detail::PyExportedClass<T>, t) + offsetof(T, memberName), 0, nullptr },
#define PY_EXPORTER_FIELD_EXPANDER(_, T, memberName) PY_EXPORTER_FIELD(T, memberName)
#define PY_EXPORTER_FIELDS(T, seq) BOOST_PP_SEQ_FOR_EACH(PY_EXPORTER_FIELD_EXPANDER, T, seq) { nullptr, 0, 0, 0, nullptr }

#define PY_EXPORTER_FIELD_TYPE(T, memberName) decltype(T::memberName),
#define PY_EXPORTER_FIELD_TYPE_EXPANDER(_, T, memberName) PY_EXPORTER_FIELD_TYPE(T, memberName)
#define PY_EXPORTER_HEADS_FIELD_TYPES(T, seq) BOOST_PP_SEQ_FOR_EACH(PY_EXPORTER_FIELD_TYPE_EXPANDER, T, seq)
#define PY_EXPORTER_FIELD_TYPES(T, seq) PY_EXPORTER_HEADS_FIELD_TYPES(T, BOOST_PP_SEQ_POP_BACK(seq)) decltype(T::BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(seq)))

#define PY_EXPORTER_FIELD_OFFSET(T, memberName) offsetof(T, memberName),
#define PY_EXPORTER_FIELD_OFFSET_EXPANDER(_, T, memberName) PY_EXPORTER_FIELD_OFFSET(T, memberName)
#define PY_EXPORTER_HEADS_FIELD_OFFSETS(T, seq) BOOST_PP_SEQ_FOR_EACH(PY_EXPORTER_FIELD_OFFSET_EXPANDER, T, seq)
#define PY_EXPORTER_FIELD_OFFSETS(T, seq) PY_EXPORTER_HEADS_FIELD_OFFSETS(T, BOOST_PP_SEQ_POP_BACK(seq)) offsetof(T, BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(seq)))


#define PY_EXPORT_STATIC_FUNCTION_NAME(T, func, funcName, moduleName) Exporter<#moduleName##_xxh64>::RegisterFunction<decltype(&##T##::##func), &##T##::##func>(#funcName)
#define PY_EXPORT_STATIC_FUNCTION(T, func, moduleName) PY_EXPORT_STATIC_FUNCTION_NAME(T, func, func, moduleName)
#define PY_EXPORT_GLOBAL_FUNCTION_NAME(func, funcName, moduleName) Exporter<#moduleName##_xxh64>::RegisterFunction<decltype(&##func), &##func>(#funcName)
#define PY_EXPORT_GLOBAL_FUNCTION(func, moduleName) PY_EXPORT_GLOBAL_FUNCTION_NAME(func, func, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_NAME(T, func, funcName, instanceReturner, moduleName) Exporter<#moduleName##_xxh64>::RegisterMemberFunctionAsStaticFunction<decltype(&##T##::##func), &##T##::##func, decltype(&##instanceReturner), &##instanceReturner>(#funcName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION(T, func, instanceReturner, moduleName) PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_NAME(T, func, func, instanceReturner, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_NAME(T, func, funcName, moduleName) Exporter<#moduleName##_xxh64>::RegisterMemberFunction<decltype(&##T##::##func), &##T##::##func, T>(#funcName)
#define PY_EXPORT_MEMBER_FUNCTION(T, func, moduleName) PY_EXPORT_MEMBER_FUNCTION_NAME(T, func, func, moduleName)

#define PY_EXPORT_TYPE_NAME(T, typeName, moduleName, fieldSeq) Exporter<#moduleName##_xxh64>::RegisterType<T, std::integer_sequence<size_t, PY_EXPORTER_FIELD_OFFSETS(T, fieldSeq)>, PY_EXPORTER_FIELD_TYPES(T, fieldSeq)>(#typeName, { PY_EXPORTER_FIELDS(T, fieldSeq) })
#define PY_EXPORT_TYPE(T, moduleName, fieldSeq) PY_EXPORT_TYPE_NAME(T, T, moduleName, fieldSeq)

#define PY_EXPORT_MODULE_NAME(moduleName, newName) Exporter<#moduleName##_xxh64>::Export(#newName)
#define PY_EXPORT_MODULE(moduleName) PY_EXPORT_MODULE_NAME(moduleName, moduleName)


//#define PY_EXPORT_MEMBER_FUNCTION(func, funcName, instanceReturner, moduleName) static auto funcName##instanceReturnFunction = instanceReturner; \
//	Exporter<#moduleName##_xxh64>::RegisterMemberFunction<decltype(&##func), &##func, decltype(&##funcName##instanceReturnFunction), &##funcName##instanceReturnFunction>(#funcName)
// TODO: Support Lambda https://qiita.com/angeart/items/94734d68999eca575881

// TODO: Support user-defined data types as field
// TODO: Support array as field (python list)
// TODO: Support operator overloading https://docs.python.org/3/c-api/typeobj.html#number-object-structures tp_as_number
// TODO: Interpret const& as a value while passing parameters

// TODO: Memory Leak (INCREF, DECREF) https://docs.python.org/3/c-api/intro.html#objects-types-and-reference-counts
// http://edcjones.tripod.com/refcount.html


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


	template<typename... Args>
	using FieldTuple = std::tuple<Args...>;



	template<typename T, T... Numbers>
	constexpr T get_nth_element(std::integer_sequence<T, Numbers...>, T i)
	{
		constexpr T arr[] = { Numbers... };
		return arr[i];
	}


	
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



	template<typename T, typename = std::enable_if_t<std::is_trivially_copy_assignable_v<T> && std::is_trivially_copy_constructible_v<T> && std::is_default_constructible_v<T>>>
	struct PyExportedClass
	{
		PyObject_HEAD
		T t;


		inline static PyTypeObject typeInfo;
		inline static std::vector<PyMemberDef> fields;
		inline static std::vector<PyMethodDef> methods;

		
		static PyObject* CustomNew(PyTypeObject* type, PyObject* args, PyObject* keywords);
		
		template<typename Offsets, typename... FieldTypes>
		static int CustomInit(PyObject* self_, PyObject* args, PyObject* keywords);
		
		static void CustomDealloc(PyObject* self);
	};


	
	template<typename T>
	class Converter
	{
	public:
		static int ParseValue(PyObject* pyObject, T* pT);

		static PyObject* BuildValue(T* pT);
	};
	


	template<typename T>
	[[nodiscard]] static constexpr const char* get_argument_type_format_string(bool treatCharAsNumber = false);

	template<typename T>
	[[nodiscard]] static constexpr int get_member_type_number();

	

	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple>
	class Dispatcher
	{
	public:
		static PyObject* ReplicatedFunction(PyObject* self, PyObject* args);
	};

	
	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple,
		typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename InstanceReturnFunctionReturnType, size_t InstanceReturnFunctionParameterCount, typename InstanceReturnFunctionParameterTuple>
	class MemberAsStaticDispatcher
	{
	public:
		static PyObject* ReplicatedFunction(PyObject* self, PyObject* args);
	};


	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple, typename Class>
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
	static void RegisterMemberFunctionAsStaticFunction(const char* functionName);


	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename Class>
	static void RegisterMemberFunction(const char* functionName);


	template<typename T, typename Offsets, typename... FieldTypes>
	static void RegisterType(const char* typeName, std::initializer_list<PyMemberDef> members);


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

		for (PyTypeObject* type : types)
		{
			if (PyType_Ready(type) != 0)
			{
				return nullptr;
			}
			Py_INCREF(type);
			PyModule_AddType(pyModule, type);
		}
		
		return pyModule;
	}


private:
	inline static std::string moduleName;
	
	inline static std::vector<PyMethodDef> methods;
	inline static std::vector<PyTypeObject*> types;
	
	inline static PyModuleDef moduleDef;
};





template <typename T, typename _>
PyObject* python_embedder_detail::PyExportedClass<T, _>::CustomNew(PyTypeObject* type, [[maybe_unused]] PyObject* args, [[maybe_unused]] PyObject* keywords)
{
	PyExportedClass* const self = reinterpret_cast<PyExportedClass*>(type->tp_alloc(type, 0));

	self->t = T{};

	return reinterpret_cast<PyObject*>(self);
}


template <typename T, typename _>
template <typename Offsets, typename ... FieldTypes>
int python_embedder_detail::PyExportedClass<T, _>::CustomInit(PyObject* self_, PyObject* args, [[maybe_unused]] PyObject* keywords)
{
	PyExportedClass* const self = reinterpret_cast<PyExportedClass*>(self_);

	using FieldTypeTuple = FieldTuple<FieldTypes...>;
	constexpr size_t fieldCount = std::tuple_size_v<FieldTypeTuple>;

	bool parseResult = true;
	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<fieldCount>>([&](auto i)
		{
			using NthFieldType = std::tuple_element_t<i, FieldTypeTuple>;

			static_assert(std::is_default_constructible_v<NthFieldType>);
			static_assert(std::is_move_assignable_v<NthFieldType>);
			static_assert(std::is_fundamental_v<NthFieldType> || std::is_same_v<NthFieldType, const char*>);

			PyObject* const pyObjectValue = PyTuple_GetItem(args, i);
			TempVarType<NthFieldType> value;

			const char* format = get_argument_type_format_string<NthFieldType>(true);

			if (!PyArg_Parse(pyObjectValue, format, &value))
			{
				parseResult = false;
				return;
			}

			new (reinterpret_cast<char*>(&(self->t)) + get_nth_element<size_t>(Offsets{}, i)) NthFieldType(static_cast<NthFieldType>(value));
		}
	);

	if (!parseResult)
	{
		return -1;
	}

	return 0;
}


template <typename T, typename _>
void python_embedder_detail::PyExportedClass<T, _>::CustomDealloc(PyObject* self)
{
	Py_TYPE(self)->tp_free(self);
}



template <typename T>
int python_embedder_detail::Converter<T>::ParseValue(PyObject* pyObject, T* pT)
{
	new (pT) T{ reinterpret_cast<PyExportedClass<T>*>(pyObject)->t };

	return 1;
}


template <typename T>
PyObject* python_embedder_detail::Converter<T>::BuildValue(T* pT)
{
	PyExportedClass<T>* const toReturn = PyObject_New(PyExportedClass<T>, &PyExportedClass<T>::typeInfo);
	PyObject* const object = reinterpret_cast<PyObject*>(toReturn);
	
	PyObject_Init(object, &PyExportedClass<T>::typeInfo);
	toReturn->t = *pT;

	// TODO: To support custom objects, manually parse the arguments and use pyobject_callobject

	return object;
}


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

			if constexpr (std::is_fundamental_v<NthParameterType> || std::is_same_v<NthParameterType, const char*>)
			{
				const char* format = get_argument_type_format_string<NthParameterType>();

				if (!PyArg_Parse(pyObjectValue, format, &value))
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

		if constexpr (std::is_fundamental_v<ReturnType> || std::is_same_v<ReturnType, const char*>)
		{
			const char* format = get_argument_type_format_string<ReturnType>();

			toReturn = Py_BuildValue(format, returnValue);
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
PyObject* python_embedder_detail::MemberAsStaticDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple, InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnFunctionReturnType, InstanceReturnFunctionParameterCount, InstanceReturnFunctionParameterTypeTuple>
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

			if constexpr (std::is_fundamental_v<NthParameterType> || std::is_same_v<NthParameterType, const char*>)
			{
				const char* format = get_argument_type_format_string<NthParameterType>();

				if (!PyArg_Parse(pyObjectValue, format, &value))
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

			if constexpr (std::is_fundamental_v<NthParameterType> || std::is_same_v<NthParameterType, const char*>)
			{
				const char* format = get_argument_type_format_string<NthParameterType>();

				if (!PyArg_Parse(pyObjectValue, format, &value))
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

		if constexpr (std::is_fundamental_v<ReturnType> || std::is_same_v<ReturnType, const char*>)
		{
			const char* format = get_argument_type_format_string<ReturnType>();

			toReturn = Py_BuildValue(format, returnValue);
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



template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple, typename Class>
PyObject* python_embedder_detail::MemberDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple, Class>::ReplicatedFunction(PyObject* self, PyObject* args)
{
	ParameterTypeTuple arguments;

	bool parseResult = true;
	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<ParameterCount - 1>>([&](auto i)
		{
			using NthParameterType = std::tuple_element_t<i, ParameterTypeTuple>;

			static_assert(std::is_default_constructible_v<NthParameterType>);
			static_assert(std::is_move_assignable_v<NthParameterType>);

			PyObject* const pyObjectValue = PyTuple_GetItem(args, i);
			TempVarType<NthParameterType> value;

			if constexpr (std::is_fundamental_v<NthParameterType> || std::is_same_v<NthParameterType, const char*>)
			{
				const char* format = get_argument_type_format_string<NthParameterType>();

				if (!PyArg_Parse(pyObjectValue, format, &value))
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

	Class instance;
	Converter<Class>::ParseValue(self, &instance);

	auto thisPtrAddedArguments = std::tuple_cat(std::make_tuple(&instance), arguments);

	PyObject* toReturn;
	if constexpr (std::is_same_v<ReturnType, void>)
	{
		std::apply(FunctionPtr, thisPtrAddedArguments);

		toReturn = Py_None;
	}
	else
	{
		ReturnType returnValue = std::apply(FunctionPtr, thisPtrAddedArguments);

		if constexpr (std::is_fundamental_v<ReturnType> || std::is_same_v<ReturnType, const char*>)
		{
			const char* format = get_argument_type_format_string<ReturnType>();

			toReturn = Py_BuildValue(format, returnValue);
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
constexpr const char* python_embedder_detail::get_argument_type_format_string(const bool treatCharAsNumber)
{
	if constexpr (std::is_same_v<T, bool>)
	{
		return "p";
	}
	else if constexpr (std::is_same_v<T, char>)
	{
		return treatCharAsNumber ? "B" : "C";
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
	else if constexpr (std::is_same_v<T, const char*>)
	{
		return "s";
	}
	else
	{
		_ASSERT(false);
		return nullptr;
	}
}



template <typename T>
constexpr int python_embedder_detail::get_member_type_number()
{
	if constexpr (std::is_same_v<T, bool>)
	{
		return T_BOOL;
	}
	// Assume it as a number
	else if constexpr (std::is_same_v<T, char>)
	{
		return T_BYTE;
	}
	else if constexpr (std::is_same_v<T, unsigned char>)
	{
		return T_UBYTE;
	}
	else if constexpr (std::is_same_v<T, short>)
	{
		return T_SHORT;
	}
	else if constexpr (std::is_same_v<T, unsigned short>)
	{
		return T_USHORT;
	}
	else if constexpr (std::is_same_v<T, int>)
	{
		return T_INT;
	}
	else if constexpr (std::is_same_v<T, unsigned int>)
	{
		return T_UINT;
	}
	else if constexpr (std::is_same_v<T, long>)
	{
		return T_LONG;
	}
	else if constexpr (std::is_same_v<T, unsigned long>)
	{
		return T_ULONG;
	}
	else if constexpr (std::is_same_v<T, long long>)
	{
		return T_LONGLONG;
	}
	else if constexpr (std::is_same_v<T, unsigned long long>)
	{
		return T_ULONGLONG;
	}
	else if constexpr (std::is_same_v<T, float>)
	{
		return T_FLOAT;
	}
	else if constexpr (std::is_same_v<T, double>)
	{
		return T_DOUBLE;
	}
	/// long double is not supported by python API
	else if constexpr (std::is_same_v<T, const char*>)
	{
		return T_STRING;
	}
	else
	{
		_ASSERT(false);
		return -1;
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
void Exporter<ModuleNameHashValue>::RegisterMemberFunctionAsStaticFunction(const char* functionName)
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

	using InstantiatedDispatcher = python_embedder_detail::MemberAsStaticDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple,
		InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnerReturnType, instanceReturnerParameterCount, InstanceReturnerParameterTypeTuple>;

	methods.push_back(PyMethodDef{ functionName, &InstantiatedDispatcher::ReplicatedFunction, METH_VARARGS, nullptr });
}


template <unsigned long long ModuleNameHashValue>
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename Class>
void Exporter<ModuleNameHashValue>::RegisterMemberFunction(const char* functionName)
{
	using namespace boost::function_types;

	static_assert(is_member_function_pointer<FunctionPtrType>::value);

	constexpr size_t parameterCount = function_arity<FunctionPtrType>::value;
	using ReturnType = typename result_type<FunctionPtrType>::type;
	using ParameterTypeTuple = python_embedder_detail::TailsParameterTuple<parameter_types<FunctionPtrType>, parameterCount>;

	using InstantiatedDispatcher = python_embedder_detail::MemberDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class>;

	if (python_embedder_detail::PyExportedClass<Class>::methods.back.ml_name == nullptr)
	{
		throw std::runtime_error("All member functions must be exported first before its type");
	}
	
	python_embedder_detail::PyExportedClass<Class>::methods.push_back(PyMethodDef{
		functionName,
		&InstantiatedDispatcher::ReplicatedFunction,
		METH_VARARGS,
		nullptr
		}
	);
}


template <unsigned long long ModuleNameHashValue>
template <typename T, typename Offsets, typename... FieldTypes>
void Exporter<ModuleNameHashValue>::RegisterType(const char* typeName, std::initializer_list<PyMemberDef> members)
{
	using PyExportedType = python_embedder_detail::PyExportedClass<T>;

	PyExportedType::fields = members;
	PyExportedType::methods.push_back(PyMethodDef{ nullptr, nullptr, 0, nullptr });
	
	PyTypeObject typeObject{ PyVarObject_HEAD_INIT(nullptr, 0) };
	typeObject.tp_name = typeName;
	typeObject.tp_basicsize = sizeof(PyExportedType);
	typeObject.tp_itemsize = 0;
	typeObject.tp_flags = Py_TPFLAGS_DEFAULT;
	typeObject.tp_new = &PyExportedType::CustomNew;
	typeObject.tp_init = &PyExportedType::template CustomInit<Offsets, FieldTypes...>;
	typeObject.tp_dealloc = &PyExportedType::CustomDealloc;
	typeObject.tp_members = PyExportedType::fields.data();
	typeObject.tp_methods = PyExportedType::methods.data();

	PyExportedType::typeInfo = typeObject;
	types.push_back(&PyExportedType::typeInfo);
}

}
