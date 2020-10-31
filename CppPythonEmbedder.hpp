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

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/pop_back.hpp>
#include <boost/preprocessor/seq/reverse.hpp>

#include <xxhash_cx/xxhash_cx.h>

#include "FunctionTypes.hpp"


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

#define _PY_EXPORTER_STRINGIFY_HELPER(...) #__VA_ARGS__
#define _PY_EXPORTER_STRINGIFY(x) _PY_EXPORTER_REMOVE_PARENTHESIS_HELPER(_PY_EXPORTER_STRINGIFY_HELPER x)

#define _PY_EXPORTER_TEMPLATE_INSTANCE(func, templateParam) cpp_python_embedder::TemplateInstanceInfo<decltype(&(##func<_PY_EXPORTER_REMOVE_PARENTHESIS(templateParam)>)), &(##func<_PY_EXPORTER_REMOVE_PARENTHESIS(templateParam)>), _PY_EXPORTER_STRINGIFY(templateParam)##_xxh64>,
#define _PY_EXPORTER_TEMPLATE_INSTANCE_EXPANDER(_, func, templateParam) _PY_EXPORTER_TEMPLATE_INSTANCE(func, templateParam)
#define _PY_EXPORTER_HEADS_TEMPLATE_SPECIALIZES(func, seq) BOOST_PP_SEQ_FOR_EACH(_PY_EXPORTER_TEMPLATE_INSTANCE_EXPANDER, func, seq)
#define _PY_EXPORTER_TEMPLATE_INSTANCES(func, seq) _PY_EXPORTER_HEADS_TEMPLATE_SPECIALIZES(func, BOOST_PP_SEQ_POP_BACK(seq)) cpp_python_embedder::TemplateInstanceInfo<decltype(&(##func<_PY_EXPORTER_REMOVE_PARENTHESIS(BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(seq)))>)), &(##func<_PY_EXPORTER_REMOVE_PARENTHESIS(BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(seq)))>), _PY_EXPORTER_STRINGIFY(BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_REVERSE(seq)))##_xxh64>



#define PY_EXPORT_GLOBAL_FUNCTION_NAME(func, funcName, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterFunction<decltype(&##func), &##func>(#funcName)
#define PY_EXPORT_STATIC_FUNCTION_NAME(T, func, funcName, moduleName) PY_EXPORT_GLOBAL_FUNCTION_NAME(T##::##func, funcName, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_NAME(T, func, funcName, instanceReturner, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterMemberFunctionAsStaticFunction<decltype(&##T##::##func), &##T##::##func, decltype(&##instanceReturner), &##instanceReturner>(#funcName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA_NAME(T, func, funcName, instanceReturner, moduleName) static auto T##funcName##moduleName##lambda = instanceReturner; \
	cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterMemberFunctionAsStaticFunctionLambda<decltype(&##T##::##func), &##T##::##func, decltype(&decltype(T##funcName##moduleName##lambda##)::operator()), &decltype(T##funcName##moduleName##lambda##)::operator(), decltype(&##T##funcName##moduleName##lambda), &##T##funcName##moduleName##lambda>(#funcName)
#define PY_EXPORT_MEMBER_FUNCTION_NAME(T, func, funcName, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterMemberFunction<decltype(&##T##::##func), &##T##::##func, T>(#funcName)

#define PY_EXPORT_TEMPLATE_GLOBAL_FUNCTION_NAME(func, funcName, moduleName, templateParamSeq) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterTemplateFunction<#funcName##_xxh64, _PY_EXPORTER_TEMPLATE_INSTANCES(func, templateParamSeq)>(#funcName)
#define PY_EXPORT_TEMPLATE_STATIC_FUNCTION_NAME(T, func, funcName, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_GLOBAL_FUNCTION_NAME(T##::##func, funcName, moduleName, templateParamSeq)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION_NAME(T, func, funcName, instanceReturner, moduleName, templateParamSeq) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterTemplateMemberFunctionAsStaticFunction<#T###funcName##_xxh64, decltype(&##instanceReturner), &##instanceReturner, _PY_EXPORTER_TEMPLATE_INSTANCES(T##::##func, templateParamSeq)>(#funcName)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA_NAME(T, func, funcName, instanceReturner, moduleName, templateParamSeq) static auto T##funcName##moduleName##lambda = instanceReturner; \
	cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterTemplateMemberFunctionAsStaticFunctionLambda<#T###funcName##_xxh64, decltype(&decltype(T##funcName##moduleName##lambda##)::operator()), &decltype(T##funcName##moduleName##lambda##)::operator(), decltype(&##T##funcName##moduleName##lambda), &##T##funcName##moduleName##lambda, _PY_EXPORTER_TEMPLATE_INSTANCES(T##::##func, templateParamSeq)>(#funcName)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_NAME(T, func, funcName, moduleName, templateParamSeq) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterTemplateMemberFunction<#T###funcName##_xxh64, T, _PY_EXPORTER_TEMPLATE_INSTANCES(T##::##func, templateParamSeq)>(#funcName)

#define PY_EXPORT_TYPE_NAME(T, typeName, moduleName, fieldSeq) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterType<T, std::integer_sequence<size_t, _PY_EXPORTER_FIELD_OFFSETS(T, fieldSeq)>, _PY_EXPORTER_FIELD_TYPES(T, fieldSeq)>(#typeName, { _PY_EXPORTER_FIELDS(T, fieldSeq) })
#define PY_EXPORT_TYPE_1FIELD_NAME(T, typeName, moduleName, field) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterType<T, std::integer_sequence<size_t, offsetof(T, field)>, decltype(T##::##field)>(#typeName, {{ #field, cpp_python_embedder::get_member_type_number<decltype(T::##field)>(), offsetof(cpp_python_embedder::PyExportedClass<T>, t) + offsetof(T, field), 0, nullptr }, { nullptr, 0, 0, 0, nullptr }})
#define PY_EXPORT_TYPE_0FIELD_NAME(T, typeName, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterType<T, std::integer_sequence<size_t>>(#typeName, {{ nullptr, 0, 0, 0, nullptr }})

#define PY_EXPORT_MODULE_NAME(moduleName, newName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::Export(#newName)


#define PY_EXPORT_GLOBAL_FUNCTION(func, moduleName) PY_EXPORT_GLOBAL_FUNCTION_NAME(func, func, moduleName)
#define PY_EXPORT_STATIC_FUNCTION(T, func, moduleName) PY_EXPORT_STATIC_FUNCTION_NAME(T, func, func, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION(T, func, instanceReturner, moduleName) PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_NAME(T, func, func, instanceReturner, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA(T, func, instanceReturner, moduleName) PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA_NAME(T, func, func, instanceReturner, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION(T, func, moduleName) PY_EXPORT_MEMBER_FUNCTION_NAME(T, func, func, moduleName)

#define PY_EXPORT_GLOBAL_OPERATOR(func, operatorType, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterGlobalOperator<decltype(&##func), &##func>(operatorType);
#define PY_EXPORT_MEMBER_OPERATOR(T, func, operatorType, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterMemberOperator<decltype(&##T##::##func), &##T##::##func, T>(operatorType);

#define PY_EXPORT_TEMPLATE_GLOBAL_FUNCTION(func, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_GLOBAL_FUNCTION_NAME(func, func, moduleName, templateParamSeq)
#define PY_EXPORT_TEMPLATE_STATIC_FUNCTION(T, func, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_STATIC_FUNCTION_NAME(T, func, func, moduleName, templateParamSeq)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION(T, func, instanceReturner, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION_NAME(T, func, func, instanceReturner, moduleName, templateParamSeq)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA(T, func, instanceReturner, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA_NAME(T, func, func, instanceReturner, moduleName, templateParamSeq)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION(T, func, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_NAME(T, func, func, moduleName, templateParamSeq)

#define PY_EXPORT_TYPE(T, moduleName, fieldSeq) PY_EXPORT_TYPE_NAME(T, T, moduleName, fieldSeq)
#define PY_EXPORT_TYPE_1FIELD(T, moduleName, field) PY_EXPORT_TYPE_1FIELD_NAME(T, T, moduleName, field)
#define PY_EXPORT_TYPE_0FIELD(T, moduleName) PY_EXPORT_TYPE_0FIELD_NAME(T, T, moduleName)

#define PY_EXPORT_MODULE(moduleName) PY_EXPORT_MODULE_NAME(moduleName, moduleName)



namespace cpp_python_embedder
{
// TODO: Way to choose between overloads (take explicit function ptr?)
// TODO: Support user-defined data types as field
// TODO: Support array as field (python list)
// TODO: Support operator overloading https://docs.python.org/3/c-api/typeobj.html#number-object-structures
// TODO: Support template operator overloading (maybe?)

// TODO: Separate files and #include
// (INCREF, DECREF) https://docs.python.org/3/c-api/intro.html#objects-types-and-reference-counts
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
	using HashValueType = unsigned long long;


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
		is_supported_field_type_v<remove_const_ref_t<T>> ||
		(is_supported_custom_type_v<remove_const_ref_t<T>> && (std::is_move_assignable_v<remove_const_ref_t<T>> || std::is_copy_assignable_v<remove_const_ref_t<T>>))
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



	template<typename FunctionPtrType_, FunctionPtrType_ FunctionPtr_, HashValueType TemplateParametersHashValue_>
	struct TemplateInstanceInfo
	{
		using FunctionPtrType = FunctionPtrType_;
		static constexpr FunctionPtrType functionPtr = FunctionPtr_;
		static constexpr HashValueType templateParametersHashValue = TemplateParametersHashValue_;
	};

	
	
	template<template<typename, typename = void> typename Checker, typename ParameterTypes, size_t... Indices>
	auto validity_checker_helper(std::index_sequence<Indices...>)
		-> std::conjunction<Checker<std::tuple_element_t<Indices, ParameterTypes>>...>;

	template<template<typename, typename> typename Checker, typename ParameterTypes, size_t ParameterCount>
	using ValidityChecker = decltype(validity_checker_helper<Checker, ParameterTypes>(std::make_index_sequence<ParameterCount>()));


	template<typename ParameterTypes, size_t... Indices>
	auto parameter_type_remove_const_ref_helper(std::index_sequence<Indices...>)
		-> std::tuple<remove_const_ref_t<std::tuple_element_t<Indices, ParameterTypes>>...>;

	template<typename ParameterTypes, size_t ParameterCount>
	using RemoveConstRefParameterTuple = decltype(parameter_type_remove_const_ref_helper<ParameterTypes>(std::make_index_sequence<ParameterCount>()));

	

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

	

	template<size_t ArgumentCount, typename ParameterTypeTuple>
	constexpr bool parse_arguments(PyObject* args, ParameterTypeTuple& parsedArguments);

	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, typename ParameterTypeTuple>
	PyObject* execute_get_return_value(const ParameterTypeTuple& arguments);

	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, typename ClassReferenceType, typename ParameterTypeTuple, typename InstanceType>
	PyObject* execute_and_get_return_value_ref_possible(const ParameterTypeTuple& arguments, const InstanceType& instance, PyObject* self = nullptr);



	template<typename FunctionPtrType, FunctionPtrType FunctionPtr>
	auto get_function_replicated_function();

	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction>
	auto get_member_function_as_static_function_replicated_function();
	
	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename LambdaPtrType, LambdaPtrType LambdaPtr>
	auto get_member_function_as_static_function_lambda_replicated_function();

	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename Class>
	auto get_member_function_replicated_function();
	

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
		static PyObject* BinaryReplicatedFunction(PyObject* self, PyObject* arg);
		static PyObject* UnaryReplicatedFunction(PyObject* self);
	};


	template<HashValueType FunctionNameHashValue>
	struct TemplateFunctionDispatcher
	{
		static PyObject* ReplicatedFunction(PyObject* self, PyObject* args);

		inline static std::unordered_map<HashValueType, PyObject* (*)(PyObject*, PyObject*)> instantiatedFunctions;
	};
}




	
template<HashValueType ModuleNameHashValue>
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


	template<HashValueType FunctionNameHashValue, typename... TemplateInstanceInfos>
	static void RegisterTemplateFunction(const char* functionName);


	template<HashValueType FunctionNameHashValue, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename... TemplateInstanceInfos>
	static void RegisterTemplateMemberFunctionAsStaticFunction(const char* functionName);


	template<HashValueType FunctionNameHashValue, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename LambdaPtrType, LambdaPtrType LambdaPtr, typename... TemplateInstanceInfos>
	static void RegisterTemplateMemberFunctionAsStaticFunctionLambda(const char* functionName);


	template<HashValueType FunctionNameHashValue, typename Class, typename... TemplateInstanceInfos>
	static void RegisterTemplateMemberFunction(const char* functionName);


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



template <size_t ArgumentCount, typename ParameterTypeTuple>
constexpr bool python_embedder_detail::parse_arguments(PyObject* args, ParameterTypeTuple& parsedArguments)
{
	bool parseResult = true;
	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<ArgumentCount>>([&](auto i)
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

			std::get<i>(parsedArguments) = std::move(static_cast<NthParameterType>(value));
		}
	);

	return parseResult;
}

	

template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, typename ParameterTypeTuple>
PyObject* python_embedder_detail::execute_get_return_value(const ParameterTypeTuple& arguments)
{
	if constexpr (std::is_same_v<ReturnType, void>)
	{
		std::apply(FunctionPtr, arguments);

		return Py_None;
	}
	else
	{
		ReturnType returnValue = std::apply(FunctionPtr, arguments);

		if constexpr (std::is_fundamental_v<ReturnType> || std::is_same_v<ReturnType, const char*>)
		{
			const char* format = get_argument_type_format_string<ReturnType>();

			return Py_BuildValue(format, returnValue);
		}
		else if constexpr (std::is_same_v<ReturnType, std::string>)
		{
			return Py_BuildValue("s", returnValue.c_str());
		}
		else
		{
			return Py_BuildValue("O&", &Converter<ReturnType>::BuildValue, returnValue);
		}
	}
}


	
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, typename ClassReferenceType, typename ParameterTypeTuple, typename InstanceType>
PyObject* python_embedder_detail::execute_and_get_return_value_ref_possible(const ParameterTypeTuple& arguments, const InstanceType& instance, PyObject* self)
{
	if constexpr (std::is_same_v<ReturnType, void>)
	{
		std::apply(FunctionPtr, arguments);

		return Py_None;
	}
	else if constexpr (std::is_same_v<ReturnType, ClassReferenceType>)
	{
		std::apply(FunctionPtr, arguments);

		return self ? self : Py_BuildValue("O&", &Converter<std::remove_reference_t<ReturnType>>::BuildValue, instance);
	}
	else
	{
		ReturnType returnValue = std::apply(FunctionPtr, arguments);

		if constexpr (std::is_fundamental_v<ReturnType> || std::is_same_v<ReturnType, const char*>)
		{
			const char* format = get_argument_type_format_string<ReturnType>();

			return Py_BuildValue(format, returnValue);
		}
		else if constexpr (std::is_same_v<ReturnType, std::string>)
		{
			return Py_BuildValue("s", returnValue.c_str());
		}
		else
		{
			return Py_BuildValue("O&", &Converter<ReturnType>::BuildValue, returnValue);
		}
	}
}

	

template <typename FunctionPtrType, FunctionPtrType FunctionPtr>
auto python_embedder_detail::get_function_replicated_function()
{
	using namespace function_types;

	static_assert(std::is_function_v<std::remove_pointer_t<FunctionPtrType>>);
	static_assert(!std::is_member_function_pointer_v<FunctionPtrType>);
	
	constexpr size_t parameterCount = FunctionTypes<FunctionPtrType>::parameterCount;
	using ParameterTypeTuple = RemoveConstRefParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters, parameterCount>;
	using ReturnType = typename FunctionTypes<FunctionPtrType>::ReturnType;
	
	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount>::value);
	static_assert(is_supported_return_type_v<ReturnType>);

	using InstantiatedDispatcher = FunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple>;

	return &InstantiatedDispatcher::ReplicatedFunction;
}

	

template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction>
auto python_embedder_detail::get_member_function_as_static_function_replicated_function()
{
	using namespace function_types;

	static_assert(std::is_member_function_pointer_v<FunctionPtrType>);
	static_assert(std::is_function_v<std::remove_pointer_t<InstanceReturnFunctionType>>);

	constexpr size_t parameterCount = FunctionTypes<FunctionPtrType>::parameterCount;
	using ParameterTypeTuple = RemoveConstRefParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters, parameterCount>;
	using ReturnType = typename FunctionTypes<FunctionPtrType>::ReturnType;

	constexpr size_t instanceReturnerParameterCount = FunctionTypes<InstanceReturnFunctionType>::parameterCount;
	using InstanceReturnerParameterTypeTuple = RemoveConstRefParameterTuple<typename FunctionTypes<InstanceReturnFunctionType>::Parameters, instanceReturnerParameterCount>;
	using InstanceReturnerReturnType = typename FunctionTypes<InstanceReturnFunctionType>::ReturnType;

	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount>::value);
	static_assert(is_supported_return_type_v<ReturnType> || std::is_same_v<ReturnType, std::remove_pointer_t<InstanceReturnerReturnType>&>);

	static_assert(ValidityChecker<is_supported_parameter_type, InstanceReturnerParameterTypeTuple, instanceReturnerParameterCount>::value);
	static_assert(std::is_same_v<InstanceReturnerReturnType, typename FunctionTypes<FunctionPtrType>::Class*>);
	
	using InstantiatedDispatcher = MemberFunctionAsStaticFunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple,
		InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnerReturnType, instanceReturnerParameterCount, InstanceReturnerParameterTypeTuple>;

	return &InstantiatedDispatcher::ReplicatedFunction;
}

	

template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename LambdaPtrType, LambdaPtrType LambdaPtr>
auto python_embedder_detail::get_member_function_as_static_function_lambda_replicated_function()
{
	using namespace function_types;

	static_assert(std::is_member_function_pointer_v<FunctionPtrType>);

	constexpr size_t parameterCount = FunctionTypes<FunctionPtrType>::parameterCount;
	using ParameterTypeTuple = RemoveConstRefParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters, parameterCount>;
	using ReturnType = typename FunctionTypes<FunctionPtrType>::ReturnType;

	constexpr size_t instanceReturnerParameterCount = FunctionTypes<InstanceReturnFunctionType>::parameterCount;
	using InstanceReturnerParameterTypeTuple = RemoveConstRefParameterTuple<typename FunctionTypes<InstanceReturnFunctionType>::Parameters, instanceReturnerParameterCount>;
	using InstanceReturnerReturnType = typename FunctionTypes<InstanceReturnFunctionType>::ReturnType;
	
	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount>::value);
	static_assert(is_supported_return_type_v<ReturnType> || std::is_same_v<ReturnType, std::remove_pointer_t<InstanceReturnerReturnType>&>);

	static_assert(ValidityChecker<is_supported_parameter_type, InstanceReturnerParameterTypeTuple, instanceReturnerParameterCount>::value);
	static_assert(std::is_same_v<InstanceReturnerReturnType, typename FunctionTypes<FunctionPtrType>::Class*>);

	using InstantiatedDispatcher = MemberFunctionAsStaticFunctionLambdaDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple,
		InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnerReturnType, instanceReturnerParameterCount, InstanceReturnerParameterTypeTuple,
		LambdaPtrType, LambdaPtr
	>;

	return &InstantiatedDispatcher::ReplicatedFunction;
}

	

template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename Class>
auto python_embedder_detail::get_member_function_replicated_function()
{
	using namespace function_types;

	static_assert(std::is_member_function_pointer_v<FunctionPtrType>);
	static_assert(is_supported_custom_type_v<Class>);

	constexpr size_t parameterCount = FunctionTypes<FunctionPtrType>::parameterCount;
	using ParameterTypeTuple = RemoveConstRefParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters, parameterCount>;
	using ReturnType = typename FunctionTypes<FunctionPtrType>::ReturnType;

	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount>::value);
	static_assert(is_supported_return_type_v<ReturnType> || std::is_same_v<ReturnType, Class&>);

	using InstantiatedDispatcher = MemberFunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class>;

	return &InstantiatedDispatcher::ReplicatedFunction;
}



template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple>
PyObject* FunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple>
	::ReplicatedFunction([[maybe_unused]] PyObject* self, PyObject* args)
{
	ParameterTypeTuple arguments;
	
	if (!parse_arguments<ParameterCount>(args, arguments))
	{
		return nullptr;
	}

	
	PyObject* toReturn = execute_get_return_value<FunctionPtrType, FunctionPtr, ReturnType>(arguments);
	Py_INCREF(toReturn);
	return toReturn;
}

	

template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename InstanceReturnFunctionReturnType, size_t InstanceReturnFunctionParameterCount, typename InstanceReturnFunctionParameterTypeTuple>
PyObject* MemberFunctionAsStaticFunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple, InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnFunctionReturnType, InstanceReturnFunctionParameterCount, InstanceReturnFunctionParameterTypeTuple>
	::ReplicatedFunction([[maybe_unused]] PyObject* self, PyObject* args)
{
	InstanceReturnFunctionParameterTypeTuple instanceReturnerArguments;

	PyObject* const instanceReturnFunctionArgs = PyTuple_GetItem(args, 0);
	
	if (!parse_arguments<InstanceReturnFunctionParameterCount>(instanceReturnFunctionArgs, instanceReturnerArguments))
	{
		return nullptr;
	}
	

	ParameterTypeTuple arguments;

	PyObject* const realArgs = PyTuple_GetSlice(args, 1, ParameterCount + 1);

	if (!parse_arguments<ParameterCount>(realArgs, arguments))
	{
		Py_DECREF(realArgs);
		return nullptr;
	}
	Py_DECREF(realArgs);

	
	InstanceReturnFunctionReturnType instance = std::apply(InstanceReturnFunction, instanceReturnerArguments);
	auto thisPtrAddedArguments = std::tuple_cat(std::make_tuple(instance), arguments);

	PyObject* toReturn = execute_and_get_return_value_ref_possible<FunctionPtrType, FunctionPtr, ReturnType, std::remove_pointer_t<InstanceReturnFunctionReturnType>&>
		(thisPtrAddedArguments, instance);

	Py_INCREF(toReturn);
	return toReturn;
}


	
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename InstanceReturnFunctionReturnType, size_t InstanceReturnFunctionParameterCount, typename InstanceReturnFunctionParameterTypeTuple, typename LambdaPtrType, LambdaPtrType LambdaPtr>
PyObject* MemberFunctionAsStaticFunctionLambdaDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple, InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnFunctionReturnType, InstanceReturnFunctionParameterCount, InstanceReturnFunctionParameterTypeTuple, LambdaPtrType, LambdaPtr>
	::ReplicatedFunction([[maybe_unused]] PyObject* self, PyObject* args)
{
	InstanceReturnFunctionParameterTypeTuple instanceReturnerArguments;

	PyObject* const instanceReturnFunctionArgs = PyTuple_GetItem(args, 0);

	if (!parse_arguments<InstanceReturnFunctionParameterCount>(instanceReturnFunctionArgs, instanceReturnerArguments))
	{
		return nullptr;
	}


	ParameterTypeTuple arguments;

	PyObject* const realArgs = PyTuple_GetSlice(args, 1, ParameterCount + 1);

	if (!parse_arguments<ParameterCount>(realArgs, arguments))
	{
		Py_DECREF(realArgs);
		return nullptr;
	}
	Py_DECREF(realArgs);

	
	auto thisPtrAddedInstanceReturnerArguments = std::tuple_cat(std::make_tuple(LambdaPtr), instanceReturnerArguments);
	InstanceReturnFunctionReturnType instance = std::apply(InstanceReturnFunction, thisPtrAddedInstanceReturnerArguments);
	
	auto thisPtrAddedArguments = std::tuple_cat(std::make_tuple(instance), arguments);

	PyObject* toReturn = execute_and_get_return_value_ref_possible<FunctionPtrType, FunctionPtr, ReturnType, std::remove_pointer_t<InstanceReturnFunctionReturnType>&>
		(thisPtrAddedArguments, instance);

	Py_INCREF(toReturn);
	return toReturn;
}



template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple, typename Class>
PyObject* MemberFunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple, Class>
	::ReplicatedFunction(PyObject* self, PyObject* args)
{
	ParameterTypeTuple arguments;

	if (!parse_arguments<ParameterCount>(args, arguments))
	{
		return nullptr;
	}

	
	auto thisPtrAddedArguments = std::tuple_cat(std::make_tuple(&reinterpret_cast<PyExportedClass<Class>*>(self)->t), arguments);

	PyObject* toReturn = execute_and_get_return_value_ref_possible<FunctionPtrType, FunctionPtr, ReturnType, Class&>(thisPtrAddedArguments, 1, self);  // 1 is a dummy

	Py_INCREF(toReturn);
	return toReturn;
}

	

template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple, typename Class, EOperatorType Operator>
PyObject* MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple, Class, Operator>
	::BinaryReplicatedFunction(PyObject* self, PyObject* arg)
{
	ParameterTypeTuple arguments;

	PyObject* const argTuple = PyTuple_Pack(1, arg);

	if (!parse_arguments<ParameterCount>(argTuple, arguments))
	{
		Py_DECREF(argTuple);
		return nullptr;
	}
	Py_DECREF(argTuple);

	
	auto thisPtrAddedArguments = std::tuple_cat(std::make_tuple(&reinterpret_cast<PyExportedClass<Class>*>(self)->t), arguments);

	PyObject* toReturn = execute_and_get_return_value_ref_possible<FunctionPtrType, FunctionPtr, ReturnType, Class&>(thisPtrAddedArguments, 1, self);  // 1 is a dummy

	Py_INCREF(toReturn);
	return toReturn;
}

	

template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename ParameterTypeTuple, typename Class, EOperatorType Operator>
PyObject* MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, ParameterTypeTuple, Class, Operator>
	::UnaryReplicatedFunction(PyObject* self)
{
	auto thisPtrAddedArguments = std::make_tuple(&reinterpret_cast<PyExportedClass<Class>*>(self)->t);

	PyObject* toReturn = execute_and_get_return_value_ref_possible<FunctionPtrType, FunctionPtr, ReturnType, Class&>(thisPtrAddedArguments, 1, self);  // 1 is a dummy

	Py_INCREF(toReturn);
	return toReturn;
}


	
template<HashValueType FunctionNameHashValue>
PyObject* TemplateFunctionDispatcher<FunctionNameHashValue>::ReplicatedFunction(PyObject* self, PyObject* args)
{
	const char* templateParameterTypes;
	PyArg_Parse(PyTuple_GetItem(args, 0), "s", &templateParameterTypes);

	PyObject* const realArgs = PyTuple_GetSlice(args, 1, PySequence_Length(args));

	const auto funcToCall = instantiatedFunctions.at(FunctionNameHashValue ^ xxhash::xxh64(templateParameterTypes, std::strlen(templateParameterTypes), XXHASH_CX_XXH64_SEED));
	
	PyObject* const toReturn = funcToCall(self, realArgs);
	Py_DECREF(realArgs);
	return toReturn;
}



template <HashValueType ModuleNameHashValue>
template <typename FunctionPtrType, FunctionPtrType FunctionPtr>
void Exporter<ModuleNameHashValue>::RegisterFunction(const char* functionName)
{
	methods.push_back(PyMethodDef{ functionName, get_function_replicated_function<FunctionPtrType, FunctionPtr>(), METH_VARARGS, nullptr });
}


	
template <unsigned long long ModuleNameHashValue>
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction>
void Exporter<ModuleNameHashValue>::RegisterMemberFunctionAsStaticFunction(const char* functionName)
{
	methods.push_back(PyMethodDef{ functionName, get_member_function_as_static_function_replicated_function<FunctionPtrType, FunctionPtr, InstanceReturnFunctionType, InstanceReturnFunction>(), METH_VARARGS, nullptr });
}


	
template <HashValueType ModuleNameHashValue>
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename LambdaPtrType, LambdaPtrType LambdaPtr>
void Exporter<ModuleNameHashValue>::RegisterMemberFunctionAsStaticFunctionLambda(const char* functionName)
{
	methods.push_back(PyMethodDef{ functionName, get_member_function_as_static_function_lambda_replicated_function<FunctionPtrType, FunctionPtr, InstanceReturnFunctionType, InstanceReturnFunction, LambdaPtrType, LambdaPtr>(), METH_VARARGS, nullptr });
}

	

template <HashValueType ModuleNameHashValue>
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename Class>
void Exporter<ModuleNameHashValue>::RegisterMemberFunction(const char* functionName)
{
	if (!PyExportedClass<Class>::methods.empty() && PyExportedClass<Class>::methods.back().ml_name == nullptr)
	{
		throw std::runtime_error("All member functions must be exported first before its type");
	}
	
	PyExportedClass<Class>::methods.push_back(PyMethodDef{ functionName, get_member_function_replicated_function<FunctionPtrType, FunctionPtr, Class>(), METH_VARARGS, nullptr });
}

	

template <HashValueType ModuleNameHashValue>
template <typename FunctionPtrType, FunctionPtrType FunctionPtr>
void Exporter<ModuleNameHashValue>::RegisterGlobalOperator(EOperatorType operatorType)
{
	// TODO
}



template <HashValueType ModuleNameHashValue>
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename Class>
void Exporter<ModuleNameHashValue>::RegisterMemberOperator(const EOperatorType operatorType)
{
	using namespace function_types;

	static_assert(is_supported_custom_type_v<Class>);

	constexpr size_t parameterCount = FunctionTypes<FunctionPtrType>::parameterCount;
	using ParameterTypeTuple = RemoveConstRefParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters, parameterCount>;
	using ReturnType = typename FunctionTypes<FunctionPtrType>::ReturnType;
	
	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount>::value);
	static_assert(is_supported_return_type_v<ReturnType> || std::is_same_v<ReturnType, Class&>);


	if constexpr (parameterCount == 0)  // unary
	{
		switch (operatorType)
		{
			case EOperatorType::POSITIVE:
				PyExportedClass<Class>::numberMethods.nb_positive = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::POSITIVE>::UnaryReplicatedFunction;
				break;

			case EOperatorType::NEGATIVE:
				PyExportedClass<Class>::numberMethods.nb_negative = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::NEGATIVE>::UnaryReplicatedFunction;
				break;

			case EOperatorType::INVERT:
				PyExportedClass<Class>::numberMethods.nb_invert = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::INVERT>::UnaryReplicatedFunction;
				break;

			case EOperatorType::INT:
				PyExportedClass<Class>::numberMethods.nb_int = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::INT>::UnaryReplicatedFunction;
				break;

			case EOperatorType::FLOAT:
				PyExportedClass<Class>::numberMethods.nb_float = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::FLOAT>::UnaryReplicatedFunction;
				break;
		}
	}
	else if constexpr (parameterCount == 1)  // binary
	{
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

			case EOperatorType::XOR:
				PyExportedClass<Class>::numberMethods.nb_xor = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::XOR>::BinaryReplicatedFunction;
				break;

			case EOperatorType::AND:
				PyExportedClass<Class>::numberMethods.nb_and = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::AND>::BinaryReplicatedFunction;
				break;

			case EOperatorType::OR:
				PyExportedClass<Class>::numberMethods.nb_or = &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, ParameterTypeTuple, Class, EOperatorType::OR>::BinaryReplicatedFunction;
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
}


	
template <HashValueType ModuleNameHashValue>
template <HashValueType FunctionNameHashValue, typename... TemplateInstanceInfos>
void Exporter<ModuleNameHashValue>::RegisterTemplateFunction(const char* functionName)
{
	using Pairs = TypeList<TemplateInstanceInfos...>;
	constexpr size_t pairCount = std::tuple_size_v<Pairs>;

	using FunctionMapper = TemplateFunctionDispatcher<FunctionNameHashValue>;
	auto& functionMap = FunctionMapper::instantiatedFunctions;

	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<pairCount>>([&](auto i)
		{
			using Pair = std::tuple_element_t<i, Pairs>;
			using FunctionPtrType = typename Pair::FunctionPtrType;
			constexpr FunctionPtrType functionPtr = Pair::functionPtr;
			constexpr HashValueType templateParametersHashValue = Pair::templateParametersHashValue;
		
			functionMap[FunctionNameHashValue ^ templateParametersHashValue] = get_function_replicated_function<FunctionPtrType, functionPtr>();
		}
	);
	
	methods.push_back(PyMethodDef{ functionName, &FunctionMapper::ReplicatedFunction, METH_VARARGS, nullptr });
}


	
template <HashValueType ModuleNameHashValue>
template <HashValueType FunctionNameHashValue, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename ... TemplateInstanceInfos>
void Exporter<ModuleNameHashValue>::RegisterTemplateMemberFunctionAsStaticFunction(const char* functionName)
{
	using Pairs = TypeList<TemplateInstanceInfos...>;
	constexpr size_t pairCount = std::tuple_size_v<Pairs>;

	using FunctionMapper = TemplateFunctionDispatcher<FunctionNameHashValue>;
	auto& functionMap = FunctionMapper::instantiatedFunctions;

	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<pairCount>>([&](auto i)
		{
			using Pair = std::tuple_element_t<i, Pairs>;
			using FunctionPtrType = typename Pair::FunctionPtrType;
			constexpr FunctionPtrType functionPtr = Pair::functionPtr;
			constexpr HashValueType templateParametersHashValue = Pair::templateParametersHashValue;

			functionMap[FunctionNameHashValue ^ templateParametersHashValue] = get_member_function_as_static_function_replicated_function<FunctionPtrType, functionPtr, InstanceReturnFunctionType, InstanceReturnFunction>();
		}
	);

	methods.push_back(PyMethodDef{ functionName, &FunctionMapper::ReplicatedFunction, METH_VARARGS, nullptr });
}

	

template <HashValueType ModuleNameHashValue>
template <HashValueType FunctionNameHashValue, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename LambdaPtrType, LambdaPtrType LambdaPtr, typename... TemplateInstanceInfos>
void Exporter<ModuleNameHashValue>::RegisterTemplateMemberFunctionAsStaticFunctionLambda(const char* functionName)
{
	using Pairs = TypeList<TemplateInstanceInfos...>;
	constexpr size_t pairCount = std::tuple_size_v<Pairs>;

	using FunctionMapper = TemplateFunctionDispatcher<FunctionNameHashValue>;
	auto& functionMap = FunctionMapper::instantiatedFunctions;

	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<pairCount>>([&](auto i)
		{
			using Pair = std::tuple_element_t<i, Pairs>;
			using FunctionPtrType = typename Pair::FunctionPtrType;
			constexpr FunctionPtrType functionPtr = Pair::functionPtr;
			constexpr HashValueType templateParametersHashValue = Pair::templateParametersHashValue;

			functionMap[FunctionNameHashValue ^ templateParametersHashValue] = get_member_function_as_static_function_lambda_replicated_function<FunctionPtrType, functionPtr, InstanceReturnFunctionType, InstanceReturnFunction, LambdaPtrType, LambdaPtr>();
		}
	);

	methods.push_back(PyMethodDef{ functionName, &FunctionMapper::ReplicatedFunction, METH_VARARGS, nullptr });
}

	

template <HashValueType ModuleNameHashValue>
template <HashValueType FunctionNameHashValue, typename Class, typename... TemplateInstanceInfos>
void Exporter<ModuleNameHashValue>::RegisterTemplateMemberFunction(const char* functionName)
{
	if (!PyExportedClass<Class>::methods.empty() && PyExportedClass<Class>::methods.back().ml_name == nullptr)
	{
		throw std::runtime_error("All member functions must be exported first before its type");
	}
	
	using Pairs = TypeList<TemplateInstanceInfos...>;
	constexpr size_t pairCount = std::tuple_size_v<Pairs>;

	using FunctionMapper = TemplateFunctionDispatcher<FunctionNameHashValue>;
	auto& functionMap = FunctionMapper::instantiatedFunctions;

	boost::mp11::mp_for_each<boost::mp11::mp_iota_c<pairCount>>([&](auto i)
		{
			using Pair = std::tuple_element_t<i, Pairs>;
			using FunctionPtrType = typename Pair::FunctionPtrType;
			constexpr FunctionPtrType functionPtr = Pair::functionPtr;
			constexpr HashValueType templateParametersHashValue = Pair::templateParametersHashValue;

			functionMap[FunctionNameHashValue ^ templateParametersHashValue] = get_member_function_replicated_function<FunctionPtrType, functionPtr, Class>();
		}
	);

	PyExportedClass<Class>::methods.push_back(PyMethodDef{ functionName, &FunctionMapper::ReplicatedFunction, METH_VARARGS, nullptr });
}



template <HashValueType ModuleNameHashValue>
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


	
template <HashValueType ModuleNameHashValue>
void Exporter<ModuleNameHashValue>::Export(const std::string& moduleName_)
{
	moduleName = moduleName_;
	methods.push_back(PyMethodDef{ nullptr, nullptr, 0, nullptr });
	PyImport_AppendInittab(moduleName.c_str(), Init);
}



template <HashValueType ModuleNameHashValue>
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
