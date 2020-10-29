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


using xxhash::literals::operator ""_xxh64;


#define _PY_EXPORTER_FIELD(T, fieldName) { #fieldName, cpp_python_embedder::get_member_type_number<decltype(T::##fieldName)>(), offsetof(cpp_python_embedder::PyExportedClass<T>, t) + offsetof(T, fieldName), 0, nullptr },
#define _PY_EXPORTER_FIELD_EXPANDER(_, T, fieldName) _PY_EXPORTER_FIELD(T, fieldName)
#define _PY_EXPORTER_FIELDS(T, seq) BOOST_PP_SEQ_FOR_EACH(_PY_EXPORTER_FIELD_EXPANDER, T, seq) { nullptr, 0, 0, 0, nullptr }

#define _PY_EXPORTER_FIELD_TYPE(T, fieldName) decltype(T::fieldName),
#define _PY_EXPORTER_FIELD_TYPE_EXPANDER(_, T, fieldName) _PY_EXPORTER_FIELD_TYPE(T, fieldName)
#define _PY_EXPORTER_HEADS_FIELD_TYPES(T, seq) BOOST_PP_SEQ_FOR_EACH(_PY_EXPORTER_FIELD_TYPE_EXPANDER, T, seq)
#define _PY_EXPORTER_FIELD_TYPES(T, seq) _PY_EXPORTER_HEADS_FIELD_TYPES(T, BOOST_PP_SEQ_POP_BACK(seq)) decltype(T::BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(seq)))

#define _PY_EXPORTER_FIELD_OFFSET(T, fieldName) offsetof(T, fieldName),
#define _PY_EXPORTER_FIELD_OFFSET_EXPANDER(_, T, fieldName) _PY_EXPORTER_FIELD_OFFSET(T, fieldName)
#define _PY_EXPORTER_HEADS_FIELD_OFFSETS(T, seq) BOOST_PP_SEQ_FOR_EACH(_PY_EXPORTER_FIELD_OFFSET_EXPANDER, T, seq)
#define _PY_EXPORTER_FIELD_OFFSETS(T, seq) _PY_EXPORTER_HEADS_FIELD_OFFSETS(T, BOOST_PP_SEQ_POP_BACK(seq)) offsetof(T, BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(seq)))

#define _PY_EXPORTER_REMOVE_PARENTHESIS_EXPAND_HELPER(...) __VA_ARGS__
#define _PY_EXPORTER_REMOVE_PARENTHESIS_HELPER(x) x
#define _PY_EXPORTER_REMOVE_PARENTHESIS(x) _PY_EXPORTER_REMOVE_PARENTHESIS_HELPER(_PY_EXPORTER_REMOVE_PARENTHESIS_EXPAND_HELPER x)

#define _PY_EXPORTER_TEMPLATE_INSTANCE(func, templateParam) cpp_python_embedder::FunctionPtrTypePair<decltype(&##func<_PY_EXPORTER_REMOVE_PARENTHESIS(templateParam)>), &##func<_PY_EXPORTER_REMOVE_PARENTHESIS(templateParam)>, cpp_python_embedder::TypeList<_PY_EXPORTER_REMOVE_PARENTHESIS(templateParam)>>,
#define _PY_EXPORTER_TEMPLATE_INSTANCE_EXPANDER(_, func, templateParam) _PY_EXPORTER_TEMPLATE_INSTANCE(func, templateParam)
#define _PY_EXPORTER_HEADS_TEMPLATE_SPECIALIZES(func, seq) BOOST_PP_SEQ_FOR_EACH(_PY_EXPORTER_TEMPLATE_INSTANCE_EXPANDER, func, seq)
#define _PY_EXPORTER_TEMPLATE_INSTANCES(func, seq) _PY_EXPORTER_HEADS_TEMPLATE_SPECIALIZES(func, BOOST_PP_SEQ_POP_BACK(seq)) cpp_python_embedder::FunctionPtrTypePair<decltype(&##func<_PY_EXPORTER_REMOVE_PARENTHESIS(BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(seq)))>), &##func<_PY_EXPORTER_REMOVE_PARENTHESIS(BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(seq)))>, cpp_python_embedder::TypeList<_PY_EXPORTER_REMOVE_PARENTHESIS(BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(seq)))>>



#define PY_EXPORT_GLOBAL_FUNCTION_NAME(func, funcName, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterFunction<decltype(&##func), &##func>(#funcName)
#define PY_EXPORT_STATIC_FUNCTION_NAME(T, func, funcName, moduleName) PY_EXPORT_GLOBAL_FUNCTION_NAME(T##::##func, funcName, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_NAME(T, func, funcName, instanceReturner, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterMemberFunctionAsStaticFunction<decltype(&##T##::##func), &##T##::##func, decltype(&##instanceReturner), &##instanceReturner>(#funcName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA_NAME(T, func, funcName, instanceReturner, moduleName) static auto T##funcName##moduleName##lambda = instanceReturner; \
	cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterMemberFunctionAsStaticFunctionLambda<decltype(&##T##::##func), &##T##::##func, decltype(&decltype(T##funcName##moduleName##lambda##)::operator()), &decltype(T##funcName##moduleName##lambda##)::operator(), decltype(&##T##funcName##moduleName##lambda), &##T##funcName##moduleName##lambda>(#funcName)
#define PY_EXPORT_MEMBER_FUNCTION_NAME(T, func, funcName, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterMemberFunction<decltype(&##T##::##func), &##T##::##func, T>(#funcName)

#define PY_EXPORT_TEMPLATE_GLOBAL_FUNCTION_NAME(func, funcName, moduleName, templateParamSeq) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterTemplateFunction<#funcName##_xxh64, _PY_EXPORTER_TEMPLATE_INSTANCES(func, templateParamSeq)>(#funcName)
#define PY_EXPORT_TEMPLATE_STATIC_FUNCTION_NAME(T, func, funcName, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_GLOBAL_FUNCTION_NAME(T##::##func, funcName, moduleName, templateParamSeq)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION_NAME(T, func, funcName, instanceReturner, moduleName, templateParamSeq)  // TODO
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_NAME(T, func, funcName, moduleName, templateParamSeq)  // TODO

#define PY_EXPORT_TYPE_NAME(T, typeName, moduleName, fieldSeq) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterType<T, std::integer_sequence<size_t, _PY_EXPORTER_FIELD_OFFSETS(T, fieldSeq)>, _PY_EXPORTER_FIELD_TYPES(T, fieldSeq)>(#typeName, { _PY_EXPORTER_FIELDS(T, fieldSeq) })

#define PY_EXPORT_MODULE_NAME(moduleName, newName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::Export(#newName)


#define PY_EXPORT_GLOBAL_FUNCTION(func, moduleName) PY_EXPORT_GLOBAL_FUNCTION_NAME(func, func, moduleName)
#define PY_EXPORT_STATIC_FUNCTION(T, func, moduleName) PY_EXPORT_STATIC_FUNCTION_NAME(T, func, func, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION(T, func, instanceReturner, moduleName) PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_NAME(T, func, func, instanceReturner, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA(T, func, instanceReturner, moduleName) PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA_NAME(T, func, func, instanceReturner, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION(T, func, moduleName) PY_EXPORT_MEMBER_FUNCTION_NAME(T, func, func, moduleName)

#define PY_EXPORT_GLOBAL_OPERATOR(func, operatorType, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterGlobalOperator<decltype(&##func), &##func>(operatorType);
#define PY_EXPORT_MEMBER_OPERATOR(T, func, operatorType, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterMemberOperator<decltype(&##func), &##func, T>(operatorType);

#define PY_EXPORT_TEMPLATE_GLOBAL_FUNCTION(func, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_GLOBAL_FUNCTION_NAME(func, func, moduleName, templateParamSeq)
#define PY_EXPORT_TEMPLATE_STATIC_FUNCTION(T, func, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_STATIC_FUNCTION_NAME(T, func, func, moduleName, templateParamSeq)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION(T, func, instanceReturner, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION_NAME(T, func, func, instanceReturner, moduleName, templateParamSeq)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION(T, func, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_NAME(T, func, func, moduleName, templateParamSeq)

#define PY_EXPORT_TYPE(T, moduleName, fieldSeq) PY_EXPORT_TYPE_NAME(T, T, moduleName, fieldSeq)

#define PY_EXPORT_MODULE(moduleName) PY_EXPORT_MODULE_NAME(moduleName, moduleName)



namespace cpp_python_embedder
{
// TODO: Support user-defined data types as field
// TODO: Support array as field (python list)
// TODO: Support operator overloading https://docs.python.org/3/c-api/typeobj.html#number-object-structures tp_as_number
// TODO: (possibly) Support template https://stackoverflow.com/questions/38843599/function-templates-in-python

// TODO: Remove duplicate codes using constexpr function or macro
// TODO: Separate files and #include
// TODO: Memory Leak (INCREF, DECREF) https://docs.python.org/3/c-api/intro.html#objects-types-and-reference-counts
// http://edcjones.tripod.com/refcount.html


	
enum class [[nodiscard]] EOperatorType
	{
		// operator +, nb_add
		ADD,
		// operator -, nb_subtract
		SUBTRACT,
		// operator *, nb_multiply
		MULTIPLY,
		// operator /, nb_true_divide
		DIVIDE,
		// operator %, nb_remainder
		REMAINDER,
		// operator +, nb_positive
		POSITIVE,
		// operator -, nb_negative
		NEGATIVE,
		// operator ^, nb_xor
		XOR,
		// operator &, nb_and
		AND,
		// operator |, nb_or
		OR,
		// operator ~, nb_invert
		INVERT,
		// operator <<, nb_lshift
		LEFT_SHIFT,
		// operator >>, nb_rshift
		RIGHT_SHIFT,

		// operator +=, nb_inplace_add
		INPLACE_ADD,
		// operator -=, nb_inplace_subtract
		INPLACE_SUBTRACT,
		// operator *=, nb_inplace_multiply
		INPLACE_MULTIPLY,
		// operator /=, nb_inplace_true_divide
		INPLACE_DIVIDE,
		// operator %=, nb_inplace_remainder
		INPLACE_REMAINDER,
		// operator ^=, nb_inplace_xor
		INPLACE_XOR,
		// operator &=, nb_inplace_and
		INPLACE_AND,
		// operator |=, nb_inplace_or
		INPLACE_OR,
		// operator <<=, nb_inplace_lshift
		INPLACE_LEFT_SHIFT,
		// operator >>=, nb_inplace_rshift
		INPLACE_RIGHT_SHIFT,

		// operator [integral], nb_int
		INT,
		// operator [floating_point], nb_float
		FLOAT,

		// operator [], mp_subscript & mp_ass_subscript
		SUBSCRIPT,

		// operator (), tp_call
		CALL,
		
		// operator ==, py_EQ
		EQUAL,
		// operator !=, py_NE
		NOT_EQUAL,
		// operator <, py_LT
		LESS,
		// operator <=, py_LE
		LESS_OR_EQUAL,
		// operator >, py_GT
		GREATER,
		// operator >=, py_GE
		GREATER_OR_EQUAL
	};


	
inline namespace python_embedder_detail
{
	template<typename T, typename = void>
	struct is_const_ref : std::false_type
	{};

	template<typename T>
	struct is_const_ref<T, std::enable_if_t<std::is_lvalue_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>>
	> : std::true_type
	{};

	template<typename T>
	constexpr bool is_const_ref_v = is_const_ref<T>::value;


	template<typename T>
	struct remove_const_ref
	{
		using type = std::remove_const_t<std::remove_reference_t<T>>;
	};

	template<typename T>
	using remove_const_ref_t = typename remove_const_ref<T>::type;


	template<typename T, typename = void>
	struct is_supported_custom_type : std::false_type
	{};

	template<typename T>
	struct is_supported_custom_type<T, std::enable_if_t<std::is_class_v<T> && std::is_trivially_copy_assignable_v<T> && std::is_trivially_copy_constructible_v<T> && std::is_default_constructible_v<T>>
	> : std::true_type
	{};

	template<typename T>
	constexpr bool is_supported_custom_type_v = is_supported_custom_type<T>::value;


	template<typename T, typename = void>
	struct is_supported_field_type : std::false_type
	{};

	template<typename T>
	struct is_supported_field_type < T, std::enable_if_t<
		std::is_fundamental_v<T> || std::is_same_v<T, const char*> || std::is_same_v<T, std::string>
	>> : std::true_type
	{};

	template<typename T>
	constexpr bool is_supported_field_type_v = is_supported_field_type<T>::value;
	

	template<typename T, typename = void>
	struct is_supported_parameter_type : std::false_type
	{};

	template<typename T>
	struct is_supported_parameter_type<T, std::enable_if_t<
		is_supported_field_type_v<T> ||
		(is_supported_custom_type_v<T> && (std::is_move_assignable_v<T> || std::is_copy_assignable_v<T>)) ||
		(is_const_ref_v<T> && 
			(is_supported_field_type_v<remove_const_ref_t<T>> ||
			(is_supported_custom_type_v<remove_const_ref_t<T>> && (std::is_move_assignable_v<remove_const_ref_t<T>> || std::is_copy_assignable_v<remove_const_ref_t<T>>)))
		)
	>> : std::true_type
	{};

	template<typename T>
	constexpr bool is_supported_parameter_type_v = is_supported_parameter_type<T>::value;


	template<typename T, typename = void>
	struct is_supported_return_type : std::false_type
	{};

	template<typename T>
	struct is_supported_return_type<T, std::enable_if_t<
		std::is_void_v<T> ||
		std::is_fundamental_v<T> || std::is_same_v<T, const char*> || std::is_same_v<T, std::string> ||
		(is_supported_custom_type_v<T> && (std::is_move_assignable_v<T> || std::is_copy_assignable_v<T>))
	>> : std::true_type
	{};

	template<typename T>
	constexpr bool is_supported_return_type_v = is_supported_return_type<T>::value;



	template<typename FunctionPtrType_, FunctionPtrType_ FunctionPtr_, typename TemplateParameters_>
	struct FunctionPtrTypePair
	{
		using FunctionPtrType = FunctionPtrType_;
		static constexpr FunctionPtrType functionPtr = FunctionPtr_;
		using TemplateParameters = TemplateParameters_;
	};


	
	template<template<typename, typename = void> typename Checker, typename ParameterTypes, size_t... Indices>
	auto validity_checker_helper(ParameterTypes, std::index_sequence<Indices...>)
		-> std::conjunction<Checker<std::tuple_element_t<Indices, ParameterTypes>>...>;

	template<template<typename, typename> typename Checker, typename ParameterTypes, size_t ParameterCount>
	using ValidityChecker = decltype(validity_checker_helper<Checker>(std::declval<ParameterTypes>(), std::make_index_sequence<ParameterCount>()));

	
	template<typename ParameterTypes, size_t... Indices>
	auto get_parameter_tuple_helper(ParameterTypes, std::index_sequence<Indices...>)
		-> std::tuple<remove_const_ref_t<typename boost::mpl::at_c<ParameterTypes, Indices>::type>...>;

	template<typename ParameterTypes, size_t ParameterCount>
	using ParameterTuple = decltype(get_parameter_tuple_helper(std::declval<ParameterTypes>(), std::make_index_sequence<ParameterCount>()));

	
	template<typename FirstParameterType, typename... ParameterTypes>
	std::tuple<remove_const_ref_t<ParameterTypes>...> get_tails_parameter_tuple_helper(std::tuple<FirstParameterType, ParameterTypes...>);

	template<typename ParameterTypes, size_t ParameterCount>
	using TailsParameterTuple = decltype(get_tails_parameter_tuple_helper(std::declval<ParameterTuple<ParameterTypes, ParameterCount>>()));


	template<typename... Args>
	using TypeList = std::tuple<Args...>;



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



	template<typename T>
	struct PyExportedClass
	{
		PyObject_HEAD
		T t;


		inline static PyTypeObject typeInfo;
		inline static std::vector<PyMemberDef> fields;
		inline static std::vector<PyMethodDef> methods;
		inline static PyNumberMethods numberMethods;

		
		static PyObject* CustomNew(PyTypeObject* type, PyObject* args, PyObject* keywords);
		
		template<typename Offsets, typename... FieldTypes>
		static int CustomInit(PyObject* self_, PyObject* args, PyObject* keywords);
		
		static void CustomDealloc(PyObject* self);
	};


	
	template<typename T>
	struct Converter
	{
		static int ParseValue(PyObject* pyObject, T* pT);

		static PyObject* BuildValue(T* pT);
	};
	


	template<typename T>
	[[nodiscard]] static constexpr const char* get_argument_type_format_string(bool treatCharAsNumber = false);

	template<typename T>
	[[nodiscard]] static constexpr int get_member_type_number();

	template<typename T>
	[[nodiscard]] static constexpr const char* get_template_parameter_type_name();

	

	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple>
	struct FunctionDispatcher
	{
		static PyObject* ReplicatedFunction(PyObject* self, PyObject* args);
	};

	
	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple,
		typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename InstanceReturnFunctionReturnType, size_t InstanceReturnFunctionParameterCount, typename InstanceReturnFunctionParameterTuple>
	struct MemberFunctionAsStaticFunctionDispatcher
	{
		static PyObject* ReplicatedFunction(PyObject* self, PyObject* args);
	};

	
	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple,
		typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename InstanceReturnFunctionReturnType, size_t InstanceReturnFunctionParameterCount, typename InstanceReturnFunctionParameterTypeTuple,
		typename LambdaPtrType, LambdaPtrType LambdaPtr
	>
	struct MemberFunctionAsStaticFunctionLambdaDispatcher
	{
		static PyObject* ReplicatedFunction(PyObject* self, PyObject* args);
	};


	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple, typename Class>
	struct MemberFunctionDispatcher
	{
		static PyObject* ReplicatedFunction(PyObject* self, PyObject* args);
	};


	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple, typename Class, EOperatorType Operator>
	struct MemberOperatorDispatcher
	{
		static PyObject* BinaryReplicatedFunction(PyObject* self, PyObject* args);
		static PyObject* UnaryReplicatedFunction(PyObject* self);
	};


	template<unsigned long long FunctionNameHashValue>
	struct TemplateFunctionDispatcher
	{
		static PyObject* ReplicatedFunction(PyObject* self, PyObject* args);

		inline static std::unordered_map<unsigned long long, PyObject* (*)(PyObject*, PyObject*)> instantiatedFunctions;
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

	
	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename LambdaPtrType, LambdaPtrType LambdaPtr>
	static void RegisterMemberFunctionAsStaticFunctionLambda(const char* functionName);


	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename Class>
	static void RegisterMemberFunction(const char* functionName);


	template<typename FunctionPtrType, FunctionPtrType FunctionPtr>
	static void RegisterGlobalOperator(EOperatorType operatorType);

	
	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename Class>
	static void RegisterMemberOperator(EOperatorType operatorType);


	template<unsigned long long FunctionNameHashValue, typename... FunctionPtrTypePairs>
	static void RegisterTemplateFunction(const char* functionName);


	template<typename T, typename Offsets, typename... FieldTypes>
	static void RegisterType(const char* typeName, std::initializer_list<PyMemberDef> members);


	static void Export(const std::string& moduleName_);


	static PyObject* Init();


private:
	inline static std::string moduleName;
	
	inline static std::vector<PyMethodDef> methods;
	inline static std::vector<PyTypeObject*> types;
	
	inline static PyModuleDef moduleDef;
};









	
template <typename T>
PyObject* PyExportedClass<T>::CustomNew(PyTypeObject* type, [[maybe_unused]] PyObject* args, [[maybe_unused]] PyObject* keywords)
{
	PyExportedClass* const self = reinterpret_cast<PyExportedClass*>(type->tp_alloc(type, 0));

	self->t = T{};

	return reinterpret_cast<PyObject*>(self);
}


template <typename T>
template <typename Offsets, typename ... FieldTypes>
int PyExportedClass<T>::CustomInit(PyObject* self_, PyObject* args, [[maybe_unused]] PyObject* keywords)
{
	PyExportedClass* const self = reinterpret_cast<PyExportedClass*>(self_);

	using FieldTypeTuple = TypeList<FieldTypes...>;
	constexpr size_t fieldCount = std::tuple_size_v<FieldTypeTuple>;

	bool parseResult = true;
	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<fieldCount>>([&](auto i)
		{
			using NthFieldType = std::tuple_element_t<i, FieldTypeTuple>;

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


template <typename T>
void PyExportedClass<T>::CustomDealloc(PyObject* self)
{
	Py_TYPE(self)->tp_free(self);
}



template <typename T>
int Converter<T>::ParseValue(PyObject* pyObject, T* pT)
{
	new (pT) T{ reinterpret_cast<PyExportedClass<T>*>(pyObject)->t };

	return 1;
}


template <typename T>
PyObject* Converter<T>::BuildValue(T* pT)
{
	PyExportedClass<T>* const toReturn = PyObject_New(PyExportedClass<T>, &PyExportedClass<T>::typeInfo);
	PyObject* const object = reinterpret_cast<PyObject*>(toReturn);
	
	PyObject_Init(object, &PyExportedClass<T>::typeInfo);
	toReturn->t = *pT;

	// TODO: To support custom objects, manually parse the arguments and use pyobject_callobject

	return object;
}


template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple>
PyObject* FunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple>::ReplicatedFunction([[maybe_unused]] PyObject* self, PyObject* args)
{
	ParameterTypeTuple arguments;
	
	bool parseResult = true;
	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<ParameterCount>>([&](auto i)
		{
			using NthParameterType = std::tuple_element_t<i, ParameterTypeTuple>;

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
PyObject* MemberFunctionAsStaticFunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple, InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnFunctionReturnType, InstanceReturnFunctionParameterCount, InstanceReturnFunctionParameterTypeTuple>
	::ReplicatedFunction([[maybe_unused]] PyObject* self, PyObject* args)
{
	InstanceReturnFunctionParameterTypeTuple instanceReturnerArguments;

	PyObject* const instanceReturnFunctionArgs = PyTuple_GetItem(args, 0);
	
	bool parseResult = true;
	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<InstanceReturnFunctionParameterCount>>([&](auto i)
		{
			using NthParameterType = std::tuple_element_t<i, InstanceReturnFunctionParameterTypeTuple>;

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

	Py_DECREF(realArgs);

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


	
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename InstanceReturnFunctionReturnType, size_t InstanceReturnFunctionParameterCount, typename InstanceReturnFunctionParameterTypeTuple, typename LambdaPtrType, LambdaPtrType LambdaPtr>
PyObject* MemberFunctionAsStaticFunctionLambdaDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple, InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnFunctionReturnType, InstanceReturnFunctionParameterCount, InstanceReturnFunctionParameterTypeTuple, LambdaPtrType, LambdaPtr>::ReplicatedFunction([[maybe_unused]] PyObject* self, PyObject* args)
{
	InstanceReturnFunctionParameterTypeTuple instanceReturnerArguments;

	PyObject* const instanceReturnFunctionArgs = PyTuple_GetItem(args, 0);

	bool parseResult = true;
	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<InstanceReturnFunctionParameterCount - 1>>([&](auto i)
		{
			using NthParameterType = std::tuple_element_t<i, InstanceReturnFunctionParameterTypeTuple>;

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

	Py_DECREF(realArgs);

	if (!parseResult)
	{
		return nullptr;
	}


	auto thisPtrAddedInstanceReturnerArguments = std::tuple_cat(std::make_tuple(LambdaPtr), instanceReturnerArguments);
	InstanceReturnFunctionReturnType instance = std::apply(InstanceReturnFunction, thisPtrAddedInstanceReturnerArguments);
	
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
PyObject* MemberFunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple, Class>::ReplicatedFunction(PyObject* self, PyObject* args)
{
	ParameterTypeTuple arguments;

	bool parseResult = true;
	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<ParameterCount - 1>>([&](auto i)
		{
			using NthParameterType = std::tuple_element_t<i, ParameterTypeTuple>;

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


template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple, typename Class, EOperatorType Operator>
PyObject* MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple, Class, Operator>::BinaryReplicatedFunction(PyObject* self, PyObject* args)
{
	// TODO

	if constexpr (std::is_same_v<ReturnType, Class&>)
	{
		return self;
	}
}


template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple, typename Class, EOperatorType Operator>
PyObject* MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple, Class, Operator>::UnaryReplicatedFunction(PyObject* self)
{
	// TODO
	
	if constexpr (std::is_same_v<ReturnType, Class&>)
	{
		return self;
	}
}


	
template<unsigned long long FunctionNameHashValue>
PyObject* TemplateFunctionDispatcher<FunctionNameHashValue>::ReplicatedFunction(PyObject* self, PyObject* args)
{
	const char* templateParameterTypes;
	PyArg_Parse(PyTuple_GetItem(args, 0), "s", &templateParameterTypes);

	PyObject* const realArgs = PyTuple_GetSlice(args, 1, PySequence_Length(args));

	const auto funcToCall = instantiatedFunctions.at(xxhash::xxh64(templateParameterTypes, std::strlen(templateParameterTypes) + 1, XXHASH_CX_XXH64_SEED));
	
	PyObject* const toReturn = funcToCall(self, realArgs);
	Py_DECREF(realArgs);
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

	

template <typename T>
constexpr const char* python_embedder_detail::get_template_parameter_type_name()
{
	if constexpr (std::is_same_v<T, bool>)
	{
		return "bool";
	}
	else if constexpr (std::is_same_v<T, char>)
	{
		return "char";
	}
	else if constexpr (std::is_same_v<T, unsigned char>)
	{
		return "unsigned char";
	}
	else if constexpr (std::is_same_v<T, short>)
	{
		return "short";
	}
	else if constexpr (std::is_same_v<T, unsigned short>)
	{
		return "unsigned short";
	}
	else if constexpr (std::is_same_v<T, int>)
	{
		return "int";
	}
	else if constexpr (std::is_same_v<T, unsigned int>)
	{
		return "unsigned int";
	}
	else if constexpr (std::is_same_v<T, long>)
	{
		return "long";
	}
	else if constexpr (std::is_same_v<T, unsigned long>)
	{
		return "unsigned long";
	}
	else if constexpr (std::is_same_v<T, long long>)
	{
		return "long long";
	}
	else if constexpr (std::is_same_v<T, unsigned long long>)
	{
		return "unsigned long long";
	}
	else if constexpr (std::is_same_v<T, float>)
	{
		return "float";
	}
	else if constexpr (std::is_same_v<T, double>)
	{
		return "double";
	}
	else if constexpr (std::is_same_v<T, long double>)
	{
		return "long double";
	}
	else if constexpr (std::is_same_v<T, const char*>)
	{
		return "const char*";
	}
	else if constexpr (std::is_same_v<T, std::string>)
	{
		return "string";
	}
	else
	{
		_ASSERT(false);
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
	using ParameterTypes = parameter_types<FunctionPtrType>;
	using ParameterTypeTuple = ParameterTuple<ParameterTypes, parameterCount>;
	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount>::value);
	static_assert(is_supported_return_type_v<ReturnType>);
	
	using InstantiatedDispatcher = FunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple>;

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
	using ParameterTypes = parameter_types<FunctionPtrType>;
	using ParameterTypeTuple = TailsParameterTuple<ParameterTypes, parameterCount>;
	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount - 1>::value);
	static_assert(is_supported_return_type_v<ReturnType>);

	constexpr size_t instanceReturnerParameterCount = function_arity<InstanceReturnFunctionType>::value;
	using InstanceReturnerReturnType = typename result_type<InstanceReturnFunctionType>::type;
	using InstanceReturnerParameterTypes = parameter_types<InstanceReturnFunctionType>;
	using InstanceReturnerParameterTypeTuple = ParameterTuple<InstanceReturnerParameterTypes, instanceReturnerParameterCount>;
	static_assert(ValidityChecker<is_supported_parameter_type, InstanceReturnerParameterTypeTuple, instanceReturnerParameterCount>::value);
	static_assert(std::is_same_v<InstanceReturnerReturnType, remove_const_ref_t<typename boost::mpl::at_c<ParameterTypes, 0>::type>*>);
	
	using InstantiatedDispatcher = MemberFunctionAsStaticFunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple,
		InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnerReturnType, instanceReturnerParameterCount, InstanceReturnerParameterTypeTuple>;

	methods.push_back(PyMethodDef{ functionName, &InstantiatedDispatcher::ReplicatedFunction, METH_VARARGS, nullptr });
}


template <unsigned long long ModuleNameHashValue>
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename LambdaPtrType, LambdaPtrType LambdaPtr>
void Exporter<ModuleNameHashValue>::RegisterMemberFunctionAsStaticFunctionLambda(const char* functionName)
{
	using namespace boost::function_types;

	static_assert(is_member_function_pointer<FunctionPtrType>::value);

	constexpr size_t parameterCount = function_arity<FunctionPtrType>::value;
	using ReturnType = typename result_type<FunctionPtrType>::type;
	using ParameterTypes = parameter_types<FunctionPtrType>;
	using ParameterTypeTuple = TailsParameterTuple<ParameterTypes, parameterCount>;
	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount - 1>::value);
	static_assert(is_supported_return_type_v<ReturnType>);

	constexpr size_t instanceReturnerParameterCount = function_arity<InstanceReturnFunctionType>::value;
	using InstanceReturnerReturnType = typename result_type<InstanceReturnFunctionType>::type;
	using InstanceReturnerParameterTypes = parameter_types<InstanceReturnFunctionType>;
	using InstanceReturnerParameterTypeTuple = TailsParameterTuple<InstanceReturnerParameterTypes, instanceReturnerParameterCount>;
	static_assert(ValidityChecker<is_supported_parameter_type, InstanceReturnerParameterTypeTuple, instanceReturnerParameterCount - 1>::value);
	static_assert(std::is_same_v<InstanceReturnerReturnType, remove_const_ref_t<typename boost::mpl::at_c<ParameterTypes, 0>::type>*>);

	using InstantiatedDispatcher = MemberFunctionAsStaticFunctionLambdaDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple,
		InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnerReturnType, instanceReturnerParameterCount, InstanceReturnerParameterTypeTuple,
		LambdaPtrType, LambdaPtr
	>;

	methods.push_back(PyMethodDef{ functionName, &InstantiatedDispatcher::ReplicatedFunction, METH_VARARGS, nullptr });
}


template <unsigned long long ModuleNameHashValue>
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename Class>
void Exporter<ModuleNameHashValue>::RegisterMemberFunction(const char* functionName)
{
	using namespace boost::function_types;

	static_assert(is_member_function_pointer<FunctionPtrType>::value);
	static_assert(is_supported_custom_type_v<Class>);

	constexpr size_t parameterCount = function_arity<FunctionPtrType>::value;
	using ReturnType = typename result_type<FunctionPtrType>::type;
	using ParameterTypes = parameter_types<FunctionPtrType>;
	using ParameterTypeTuple = TailsParameterTuple<ParameterTypes, parameterCount>;
	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount - 1>::value);
	static_assert(is_supported_return_type_v<ReturnType>);

	using InstantiatedDispatcher = MemberFunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class>;

	if (!PyExportedClass<Class>::methods.empty() && PyExportedClass<Class>::methods.back().ml_name == nullptr)
	{
		throw std::runtime_error("All member functions must be exported first before its type");
	}
	
	PyExportedClass<Class>::methods.push_back(PyMethodDef{
		functionName,
		&InstantiatedDispatcher::ReplicatedFunction,
		METH_VARARGS,
		nullptr
		}
	);
}


template <unsigned long long ModuleNameHashValue>
template <typename FunctionPtrType, FunctionPtrType FunctionPtr>
void Exporter<ModuleNameHashValue>::RegisterGlobalOperator(EOperatorType operatorType)
{
	// TODO
}



template <unsigned long long ModuleNameHashValue>
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename Class>
void Exporter<ModuleNameHashValue>::RegisterMemberOperator(const EOperatorType operatorType)
{
	using namespace boost::function_types;

	static_assert(is_supported_custom_type_v<Class>);

	constexpr size_t parameterCount = function_arity<FunctionPtrType>::value;
	using ReturnType = typename result_type<FunctionPtrType>::type;
	using ParameterTypes = parameter_types<FunctionPtrType>;
	using ParameterTypeTuple = TailsParameterTuple<ParameterTypes, parameterCount>;
	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount>::value);
	static_assert(is_supported_return_type_v<ReturnType> || std::is_same_v<ReturnType&>);

	
	switch (operatorType) 
	{
		case EOperatorType::ADD:
			PyExportedClass<Class>::numberMethods.nb_add = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::ADD>::BinaryReplicatedFunction;
			break;

		case EOperatorType::SUBTRACT:
			PyExportedClass<Class>::numberMethods.nb_subtract = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::SUBTRACT>::BinaryReplicatedFunction;
			break;

		case EOperatorType::MULTIPLY:
			PyExportedClass<Class>::numberMethods.nb_multiply = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::MULTIPLY>::BinaryReplicatedFunction;
			break;

		case EOperatorType::DIVIDE:
			PyExportedClass<Class>::numberMethods.nb_true_divide = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::DIVIDE>::BinaryReplicatedFunction;
			break;

		case EOperatorType::REMAINDER:
			PyExportedClass<Class>::numberMethods.nb_remainder = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::REMAINDER>::BinaryReplicatedFunction;
			break;

		case EOperatorType::POSITIVE:
			PyExportedClass<Class>::numberMethods.nb_positive = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::POSITIVE>::UnaryReplicatedFunction;
			break;

		case EOperatorType::NEGATIVE:
			PyExportedClass<Class>::numberMethods.nb_negative = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::NEGATIVE>::UnaryReplicatedFunction;
			break;

		case EOperatorType::XOR:
			PyExportedClass<Class>::numberMethods.nb_xor = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::XOR>::BinaryReplicatedFunction;
			break;

		case EOperatorType::AND:
			PyExportedClass<Class>::numberMethods.nb_and = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::AND>::BinaryReplicatedFunction;
			break;

		case EOperatorType::OR:
			PyExportedClass<Class>::numberMethods.nb_or = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::OR>::BinaryReplicatedFunction;
			break;

		case EOperatorType::INVERT:
			PyExportedClass<Class>::numberMethods.nb_invert = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::INVERT>::UnaryReplicatedFunction;
			break;

		case EOperatorType::LEFT_SHIFT:
			PyExportedClass<Class>::numberMethods.nb_lshift = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::LEFT_SHIFT>::BinaryReplicatedFunction;
			break;

		case EOperatorType::RIGHT_SHIFT:
			PyExportedClass<Class>::numberMethods.nb_rshift = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::RIGHT_SHIFT>::BinaryReplicatedFunction;
			break;

		case EOperatorType::INPLACE_ADD:
			PyExportedClass<Class>::numberMethods.nb_inplace_add = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::INPLACE_ADD>::BinaryReplicatedFunction;
			break;

		case EOperatorType::INPLACE_SUBTRACT:
			PyExportedClass<Class>::numberMethods.nb_inplace_subtract = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::INPLACE_SUBTRACT>::BinaryReplicatedFunction;
			break;

		case EOperatorType::INPLACE_MULTIPLY:
			PyExportedClass<Class>::numberMethods.nb_inplace_multiply = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::INPLACE_MULTIPLY>::BinaryReplicatedFunction;
			break;

		case EOperatorType::INPLACE_DIVIDE:
			PyExportedClass<Class>::numberMethods.nb_inplace_true_divide = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::INPLACE_DIVIDE>::BinaryReplicatedFunction;
			break;

		case EOperatorType::INPLACE_REMAINDER:
			PyExportedClass<Class>::numberMethods.nb_inplace_remainder = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::INPLACE_REMAINDER>::BinaryReplicatedFunction;
			break;

		case EOperatorType::INPLACE_XOR:
			PyExportedClass<Class>::numberMethods.nb_inplace_xor = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::INPLACE_XOR>::BinaryReplicatedFunction;
			break;

		case EOperatorType::INPLACE_AND:
			PyExportedClass<Class>::numberMethods.nb_inplace_and = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::INPLACE_AND>::BinaryReplicatedFunction;
			break;

		case EOperatorType::INPLACE_OR:
			PyExportedClass<Class>::numberMethods.nb_inplace_or = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::INPLACE_OR>::BinaryReplicatedFunction;
			break;

		case EOperatorType::INPLACE_LEFT_SHIFT:
			PyExportedClass<Class>::numberMethods.nb_inplace_lshift = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::INPLACE_LEFT_SHIFT>::BinaryReplicatedFunction;
			break;

		case EOperatorType::INPLACE_RIGHT_SHIFT:
			PyExportedClass<Class>::numberMethods.nb_inplace_rshift = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::INPLACE_RIGHT_SHIFT>::BinaryReplicatedFunction;
			break;

		case EOperatorType::INT:
			PyExportedClass<Class>::numberMethods.nb_int = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::INT>::UnaryReplicatedFunction;
			break;

		case EOperatorType::FLOAT:
			PyExportedClass<Class>::numberMethods.nb_float = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::FLOAT>::UnaryReplicatedFunction;
			break;

		case EOperatorType::SUBSCRIPT:
			break;

		case EOperatorType::EQUAL:
			break;

		case EOperatorType::NOT_EQUAL:
			break;

		case EOperatorType::LESS:
			break;

		case EOperatorType::LESS_OR_EQUAL:
			break;

		case EOperatorType::GREATER:
			break;

		case EOperatorType::GREATER_OR_EQUAL:
			break;

		default:
			break;
	}
}


	
template <unsigned long long ModuleNameHashValue>
template <unsigned long long FunctionNameHashValue, typename... FunctionPtrTypePairs>
void Exporter<ModuleNameHashValue>::RegisterTemplateFunction(const char* functionName)
{
	using namespace boost::function_types;
	
	using Pairs = TypeList<FunctionPtrTypePairs...>;
	constexpr size_t pairCount = std::tuple_size_v<Pairs>;

	using InstantiatedDispatcher = TemplateFunctionDispatcher<FunctionNameHashValue>;
	auto& functionMap = InstantiatedDispatcher::instantiatedFunctions;

	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<pairCount>>([&](auto i)
		{
			std::string functionSignatureString;
		
			using Pair = std::tuple_element_t<i, Pairs>;
			using FunctionPtrType = typename Pair::FunctionPtrType;
			constexpr FunctionPtrType functionPtr = Pair::functionPtr;
			using TemplateParameters = typename Pair::TemplateParameters;

			static_assert(is_function_pointer<FunctionPtrType>::value);
			static_assert(!is_member_function_pointer<FunctionPtrType>::value);

			constexpr size_t parameterCount = function_arity<FunctionPtrType>::value;
			using ReturnType = typename result_type<FunctionPtrType>::type;
			using ParameterTypes = parameter_types<FunctionPtrType>;
			using ParameterTypeTuple = ParameterTuple<ParameterTypes, parameterCount>;
			static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount>::value);
			static_assert(is_supported_return_type_v<ReturnType>);
		
			using InstantiatedDispatcher = FunctionDispatcher<FunctionPtrType, functionPtr, ReturnType, parameterCount, ParameterTypeTuple>;

			boost::mp11::mp_for_each<boost::mp11::mp_iota_c<std::tuple_size_v<TemplateParameters>>>([&](auto j)
				{
					using TemplateParameter = std::tuple_element_t<j, TemplateParameters>;

					if constexpr (std::is_fundamental_v<TemplateParameter> || std::is_same_v<TemplateParameter, const char*>)
					{
						functionSignatureString += get_template_parameter_type_name<TemplateParameter>();
					}
					else
					{
						functionSignatureString += PyExportedClass<TemplateParameter>::typeInfo.tp_name;
					}
					functionSignatureString += ", ";
				}
			);
		
			// Removing trailing ", "
			functionSignatureString.pop_back();
			functionSignatureString.pop_back();
		
			functionMap[xxhash::xxh64(functionSignatureString.c_str(), functionSignatureString.size() + 1, XXHASH_CX_XXH64_SEED)] = &InstantiatedDispatcher::ReplicatedFunction;
		}
	);

	// TODO: How to distinguish different functions with same template parameter list of static / member?
	// maybe concat the function name at the front
	
	methods.push_back(PyMethodDef{ functionName, &InstantiatedDispatcher::ReplicatedFunction, METH_VARARGS, nullptr });
}



template <unsigned long long ModuleNameHashValue>
template <typename T, typename Offsets, typename... FieldTypes>
void Exporter<ModuleNameHashValue>::RegisterType(const char* typeName, std::initializer_list<PyMemberDef> members)
{
	static_assert(is_supported_custom_type_v<T>);
	static_assert(ValidityChecker<is_supported_field_type, std::tuple<FieldTypes...>, sizeof...(FieldTypes)>::value);
	
	using PyExportedType = PyExportedClass<T>;

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
	typeObject.tp_as_number = &PyExportedType::numberMethods;

	PyExportedType::typeInfo = typeObject;
	types.push_back(&PyExportedType::typeInfo);
}


	
template <unsigned long long ModuleNameHashValue>
void Exporter<ModuleNameHashValue>::Export(const std::string& moduleName_)
{
	moduleName = moduleName_;
	methods.push_back(PyMethodDef{ nullptr, nullptr, 0, nullptr });
	PyImport_AppendInittab(moduleName.c_str(), Init);
}



template <unsigned long long ModuleNameHashValue>
PyObject* Exporter<ModuleNameHashValue>::Init()
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
}
