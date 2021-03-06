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

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/pop_back.hpp>
#include <boost/preprocessor/seq/reverse.hpp>

#include <xxhash_cx/xxhash_cx.h>

#include "FunctionTypes.hpp"


using xxhash::literals::operator ""_xxh64;


#define _PY_EXPORTER_FIELD(T, fieldName) { #fieldName, cpp_python_embedder::get_member_type_number<decltype(T::##fieldName)>(), offsetof(T, fieldName), 0, nullptr },
#define _PY_EXPORTER_FIELD_EXPANDER(_, T, fieldName) _PY_EXPORTER_FIELD(T, fieldName)
#define _PY_EXPORTER_FIELDS(T, seq) BOOST_PP_SEQ_FOR_EACH(_PY_EXPORTER_FIELD_EXPANDER, T, seq) { nullptr, 0, 0, 0, nullptr }

#define _PY_EXPORTER_ENUMERATOR(E, enumerator) { #enumerator, E##::##enumerator },
#define _PY_EXPORTER_ENUMERATOR_EXPANDER(_, E, enumerator) _PY_EXPORTER_ENUMERATOR(E, enumerator)
#define _PY_EXPORTER_ENUMERATORS(E, seq) BOOST_PP_SEQ_FOR_EACH(_PY_EXPORTER_ENUMERATOR_EXPANDER, E, seq) { nullptr, static_cast<E>(0) }

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





#define PY_EXPORT_GLOBAL_FUNCTION_PTR(funcPtr, funcName, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterFunction<decltype(funcPtr), funcPtr>(#funcName)
#define PY_EXPORT_STATIC_FUNCTION_PTR(funcPtr, funcName, moduleName) PY_EXPORT_GLOBAL_FUNCTION_PTR(funcPtr, funcName, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_PTR_AS_STATIC_FUNCTION_PTR(funcPtr, funcName, instanceReturnerPtr, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterMemberFunctionAsStaticFunction<decltype(funcPtr), funcPtr, decltype(instanceReturnerPtr), instanceReturnerPtr>(#funcName)
#define PY_EXPORT_MEMBER_FUNCTION_PTR_AS_STATIC_FUNCTION(funcPtr, funcName, instanceReturner, moduleName) PY_EXPORT_MEMBER_FUNCTION_PTR_AS_STATIC_FUNCTION_PTR(funcPtr, funcName, &##instanceReturner, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_PTR(func, funcName, instanceReturnerPtr, moduleName) PY_EXPORT_MEMBER_FUNCTION_PTR_AS_STATIC_FUNCTION_PTR(&##func, funcName, instanceReturnerPtr, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_PTR_AS_STATIC_FUNCTION_LAMBDA(funcPtr, funcName, instanceReturner, moduleName) static auto T##funcName##moduleName##lambda = instanceReturner; \
	cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterMemberFunctionAsStaticFunctionLambda<decltype(funcPtr), funcPtr, decltype(&decltype(T##funcName##moduleName##lambda##)::operator()), &decltype(T##funcName##moduleName##lambda##)::operator(), decltype(&##T##funcName##moduleName##lambda), &##T##funcName##moduleName##lambda>(#funcName)
#define PY_EXPORT_MEMBER_FUNCTION_PTR(funcPtr, funcName, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterMemberFunction<decltype(funcPtr), funcPtr>(#funcName)


#define PY_EXPORT_GLOBAL_FUNCTION_NAME(func, funcName, moduleName) PY_EXPORT_GLOBAL_FUNCTION_PTR(&##func, funcName, moduleName)
#define PY_EXPORT_STATIC_FUNCTION_NAME(T, func, funcName, moduleName) PY_EXPORT_GLOBAL_FUNCTION_NAME(T##::##func, funcName, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_NAME(T, func, funcName, instanceReturner, moduleName) PY_EXPORT_MEMBER_FUNCTION_PTR_AS_STATIC_FUNCTION(&##T##::##func, funcName, instanceReturner, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA_NAME(T, func, funcName, instanceReturner, moduleName) PY_EXPORT_MEMBER_FUNCTION_PTR_AS_STATIC_FUNCTION_LAMBDA(&##T##::##func, funcName, instanceReturner, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_NAME(T, func, funcName, moduleName) PY_EXPORT_MEMBER_FUNCTION_PTR(&##T##::##func, funcName, moduleName)

#define PY_EXPORT_TEMPLATE_GLOBAL_FUNCTION_NAME(func, funcName, moduleName, templateParamSeq) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterTemplateFunction<#funcName##_xxh64, _PY_EXPORTER_TEMPLATE_INSTANCES(func, templateParamSeq)>(#funcName)
#define PY_EXPORT_TEMPLATE_STATIC_FUNCTION_NAME(T, func, funcName, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_GLOBAL_FUNCTION_NAME(T##::##func, funcName, moduleName, templateParamSeq)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION_NAME(T, func, funcName, instanceReturner, moduleName, templateParamSeq) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterTemplateMemberFunctionAsStaticFunction<#T###funcName##_xxh64, decltype(&##instanceReturner), &##instanceReturner, _PY_EXPORTER_TEMPLATE_INSTANCES(T##::##func, templateParamSeq)>(#funcName)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA_NAME(T, func, funcName, instanceReturner, moduleName, templateParamSeq) static auto T##funcName##moduleName##lambda = instanceReturner; \
	cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterTemplateMemberFunctionAsStaticFunctionLambda<#T###funcName##_xxh64, decltype(&decltype(T##funcName##moduleName##lambda##)::operator()), &decltype(T##funcName##moduleName##lambda##)::operator(), decltype(&##T##funcName##moduleName##lambda), &##T##funcName##moduleName##lambda, _PY_EXPORTER_TEMPLATE_INSTANCES(T##::##func, templateParamSeq)>(#funcName)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_NAME(T, func, funcName, moduleName, templateParamSeq) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterTemplateMemberFunction<#T###funcName##_xxh64, _PY_EXPORTER_TEMPLATE_INSTANCES(T##::##func, templateParamSeq)>(#funcName)

#define PY_EXPORT_TYPE_NAME(T, typeName, moduleName, fieldSeq) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterType<T, std::integer_sequence<size_t, _PY_EXPORTER_FIELD_OFFSETS(T, fieldSeq)>, _PY_EXPORTER_FIELD_TYPES(T, fieldSeq)>(#typeName, { _PY_EXPORTER_FIELDS(T, fieldSeq) })
#define PY_EXPORT_TYPE_1FIELD_NAME(T, typeName, moduleName, field) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterType<T, std::integer_sequence<size_t, offsetof(T, field)>, decltype(T##::##field)>(#typeName, {{ #field, cpp_python_embedder::get_member_type_number<decltype(T::##field)>(), offsetof(T, field), 0, nullptr }, { nullptr, 0, 0, 0, nullptr }})
#define PY_EXPORT_TYPE_0FIELD_NAME(T, typeName, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterType<T, std::integer_sequence<size_t>>(#typeName, {{ nullptr, 0, 0, 0, nullptr }})

#define PY_EXPORT_ENUM_NAME(E, enumName, moduleName, enumeratorSeq) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterEnum<E>(#enumName, { _PY_EXPORTER_ENUMERATORS(E, enumeratorSeq) })

#define PY_EXPORT_MODULE_NAME(moduleName, newName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::Export(#newName)


#define PY_EXPORT_GLOBAL_FUNCTION(func, moduleName) PY_EXPORT_GLOBAL_FUNCTION_NAME(func, func, moduleName)
#define PY_EXPORT_STATIC_FUNCTION(T, func, moduleName) PY_EXPORT_STATIC_FUNCTION_NAME(T, func, func, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION(T, func, instanceReturner, moduleName) PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_NAME(T, func, func, instanceReturner, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA(T, func, instanceReturner, moduleName) PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA_NAME(T, func, func, instanceReturner, moduleName)
#define PY_EXPORT_MEMBER_FUNCTION(T, func, moduleName) PY_EXPORT_MEMBER_FUNCTION_NAME(T, func, func, moduleName)

#define PY_EXPORT_GLOBAL_OPERATOR(func, operatorType, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterGlobalOperator<decltype(&##func), &##func, operatorType>()
#define PY_EXPORT_MEMBER_OPERATOR(T, func, operatorType, moduleName) cpp_python_embedder::Exporter<#moduleName##_xxh64>::RegisterMemberOperator<decltype(&##T##::##func), &##T##::##func, T, operatorType>()

#define PY_EXPORT_TEMPLATE_GLOBAL_FUNCTION(func, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_GLOBAL_FUNCTION_NAME(func, func, moduleName, templateParamSeq)
#define PY_EXPORT_TEMPLATE_STATIC_FUNCTION(T, func, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_STATIC_FUNCTION_NAME(T, func, func, moduleName, templateParamSeq)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION(T, func, instanceReturner, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION_NAME(T, func, func, instanceReturner, moduleName, templateParamSeq)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA(T, func, instanceReturner, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA_NAME(T, func, func, instanceReturner, moduleName, templateParamSeq)
#define PY_EXPORT_TEMPLATE_MEMBER_FUNCTION(T, func, moduleName, templateParamSeq) PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_NAME(T, func, func, moduleName, templateParamSeq)

#define PY_EXPORT_TYPE(T, moduleName, fieldSeq) PY_EXPORT_TYPE_NAME(T, T, moduleName, fieldSeq)
#define PY_EXPORT_TYPE_1FIELD(T, moduleName, field) PY_EXPORT_TYPE_1FIELD_NAME(T, T, moduleName, field)
#define PY_EXPORT_TYPE_0FIELD(T, moduleName) PY_EXPORT_TYPE_0FIELD_NAME(T, T, moduleName)

#define PY_EXPORT_ENUM(E, moduleName, enumeratorSeq) PY_EXPORT_ENUM_NAME(E, E, moduleName, enumeratorSeq)

#define PY_EXPORT_MODULE(moduleName) PY_EXPORT_MODULE_NAME(moduleName, moduleName)



namespace cpp_python_embedder
{
// TODO: Support user-defined data types as field
// TODO: Support array as field (python list)
// TODO: Support more operator overloading https://docs.python.org/3/c-api/typeobj.html#number-object-structures
// TODO: Support template operator overloading (maybe?)

// TODO: Separate files and #include
// TODO: Use std::apply instead of mp_for_each and get rid of boost
// (INCREF, DECREF) https://docs.python.org/3/c-api/intro.html#objects-types-and-reference-counts
// http://edcjones.tripod.com/refcount.html


	
enum class [[nodiscard]] EOperatorType
{
	// operator +
	ADD = offsetof(PyNumberMethods, nb_add),
	// operator -
	SUBTRACT = offsetof(PyNumberMethods, nb_subtract),
	// operator *
	MULTIPLY = offsetof(PyNumberMethods, nb_multiply),
	// operator /
	DIVIDE = offsetof(PyNumberMethods, nb_true_divide),
	// operator %
	REMAINDER = offsetof(PyNumberMethods, nb_remainder),
	// operator +
	POSITIVE = offsetof(PyNumberMethods, nb_positive),
	// operator -
	NEGATIVE = offsetof(PyNumberMethods, nb_negative),
	// operator ^
	XOR = offsetof(PyNumberMethods, nb_xor),
	// operator &
	AND = offsetof(PyNumberMethods, nb_and),
	// operator |
	OR = offsetof(PyNumberMethods, nb_or),
	// operator ~
	INVERT = offsetof(PyNumberMethods, nb_invert),
	// operator <<
	LEFT_SHIFT = offsetof(PyNumberMethods, nb_lshift),
	// operator >>
	RIGHT_SHIFT = offsetof(PyNumberMethods, nb_rshift),

	// operator +=
	INPLACE_ADD = offsetof(PyNumberMethods, nb_inplace_add),
	// operator -=
	INPLACE_SUBTRACT = offsetof(PyNumberMethods, nb_inplace_subtract),
	// operator *=
	INPLACE_MULTIPLY = offsetof(PyNumberMethods, nb_inplace_multiply),
	// operator /=
	INPLACE_DIVIDE = offsetof(PyNumberMethods, nb_inplace_true_divide),
	// operator %=
	INPLACE_REMAINDER = offsetof(PyNumberMethods, nb_inplace_remainder),
	// operator ^=
	INPLACE_XOR = offsetof(PyNumberMethods, nb_inplace_xor),
	// operator &=
	INPLACE_AND = offsetof(PyNumberMethods, nb_inplace_and),
	// operator |=
	INPLACE_OR = offsetof(PyNumberMethods, nb_inplace_or),
	// operator <<=
	INPLACE_LEFT_SHIFT = offsetof(PyNumberMethods, nb_inplace_lshift),
	// operator >>=
	INPLACE_RIGHT_SHIFT = offsetof(PyNumberMethods, nb_inplace_rshift),

	// operator [integral]
	INT = offsetof(PyNumberMethods, nb_int),
	// operator [floating_point]
	FLOAT = offsetof(PyNumberMethods, nb_float),

	//// operator [], mp_subscript & mp_ass_subscript
	//SUBSCRIPT,

	//// operator (), tp_call
	//CALL,
	//
	//// operator ==, py_EQ
	//EQUAL,
	//// operator !=, py_NE
	//NOT_EQUAL,
	//// operator <, py_LT
	//LESS,
	//// operator <=, py_LE
	//LESS_OR_EQUAL,
	//// operator >, py_GT
	//GREATER,
	//// operator >=, py_GE
	//GREATER_OR_EQUAL
};


	
inline namespace python_embedder_detail
{
	using HashValueType = unsigned long long;


	template<typename T, typename = void>
	struct remove_const_ref
	{
		using type = std::remove_const_t<std::remove_reference_t<T>>;
	};

	template<typename T>
	struct remove_const_ref<T, std::enable_if_t<std::is_lvalue_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>>>
	{
		using type = T;
	};

	template<typename T>
	using remove_const_ref_t = typename remove_const_ref<T>::type;


	template<typename T, typename = void>
	struct ref_to_ptr
	{
		using type = T;
	};

	template<typename T>
	struct ref_to_ptr<T, std::enable_if_t<std::is_lvalue_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>>>
	{
		using type = std::remove_reference_t<T>*;
	};

	template<typename T>
	using ref_to_ptr_t = typename ref_to_ptr<T>::type;


	
	template<typename T, typename = void>
	struct is_supported_custom_type : std::false_type
	{};

	template<typename T>
	struct is_supported_custom_type<T, std::enable_if_t<std::is_class_v<T> && std::is_default_constructible_v<T> && std::is_copy_assignable_v<T> && std::is_copy_constructible_v<T>>
	> : std::true_type
	{};

	template<typename T>
	constexpr bool is_supported_custom_type_v = is_supported_custom_type<T>::value;


	template<typename T, typename = void>
	struct is_supported_field_type : std::false_type
	{};

	template<typename T>
	struct is_supported_field_type<T, std::enable_if_t<
		std::is_fundamental_v<T> || std::is_enum_v<T> || std::is_same_v<T, const char*> || std::is_same_v<T, std::string>
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
		is_supported_custom_type_v<remove_const_ref_t<T>> || is_supported_custom_type_v<std::remove_reference_t<T>>
	>> : std::true_type
	{};

	template<typename T>
	constexpr bool is_supported_parameter_type_v = is_supported_parameter_type<T>::value;


	template<typename T, typename = void>
	struct is_supported_return_type : std::false_type
	{};

	template<typename T>
	struct is_supported_return_type<T, std::enable_if_t<
		std::is_void_v<T> || is_supported_field_type_v<T> || 
		is_supported_custom_type_v<remove_const_ref_t<T>> || is_supported_custom_type_v<std::remove_reference_t<T>>
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


	template<typename E>
	struct EnumeratorInfo
	{
		const char* name;
		E value;
	};

	
	
	template<template<typename, typename = void> typename Checker, typename ParameterTypes, size_t... Indices>
	auto validity_checker_helper(std::index_sequence<Indices...>)
		-> std::conjunction<Checker<std::tuple_element_t<Indices, ParameterTypes>>...>;

	template<template<typename, typename> typename Checker, typename ParameterTypes, size_t ParameterCount>
	using ValidityChecker = decltype(validity_checker_helper<Checker, ParameterTypes>(std::make_index_sequence<ParameterCount>()));


	template<typename ParameterTypes, size_t... Indices>
	auto parameter_type_remove_const_ref_ref_to_ptr_helper(std::index_sequence<Indices...>)
		-> std::tuple<remove_const_ref_t<ref_to_ptr_t<std::tuple_element_t<Indices, ParameterTypes>>>...>;

	template<typename ParameterTypes>
	using RemoveConstRefRefToPtrParameterTuple = decltype(parameter_type_remove_const_ref_ref_to_ptr_helper<ParameterTypes>(std::make_index_sequence<std::tuple_size_v<ParameterTypes>>()));

	
	template<typename ParameterTypes, size_t... Indices>
	auto parameter_type_remove_const_ref_helper(std::index_sequence<Indices...>)
		->std::tuple<remove_const_ref_t<std::tuple_element_t<Indices, ParameterTypes>>...>;

	template<typename ParameterTypes>
	using RemoveConstRefParameterTuple = decltype(parameter_type_remove_const_ref_helper<ParameterTypes>(std::make_index_sequence<std::tuple_size_v<ParameterTypes>>()));


	template<typename T, typename = void>
	struct OriginalValueGetter
	{
		static T GetOriginalValue(T t) { return t; }
	};

	template<typename T>
	struct OriginalValueGetter<T, std::enable_if_t<std::is_pointer_v<T>>>
	{
		static std::remove_pointer_t<T>& GetOriginalValue(T t) { return *t; }
	};

	
	
	template<typename RefToPtrParameterTuple, typename ParameterTuple, size_t... Indices>
	[[nodiscard]] ParameterTuple restore_ref_helper(const RefToPtrParameterTuple& params, std::index_sequence<Indices...>);
	
	template<typename RefToPtrParameterTuple, typename ParameterTuple>
	[[nodiscard]] ParameterTuple restore_ref(const RefToPtrParameterTuple& params) { return restore_ref_helper<RefToPtrParameterTuple, ParameterTuple>(params, std::make_index_sequence<std::tuple_size_v<ParameterTuple>>()); }
	

	template<typename... Args>
	using TypeList = std::tuple<Args...>;

	

	template<typename T, T... Numbers>
	[[nodiscard]] constexpr T get_nth_element(std::integer_sequence<T, Numbers...>, T i)
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
		T* pT;
		bool isOwner = false;


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
		static int ParseValuePtr(PyObject* pyObject, T** pT);

		static PyObject* BuildValue(T* pT);
		static PyObject* BuildValuePtr(T* pT);
	};
	


	template<typename T>
	[[nodiscard]] static constexpr const char* get_type_format_string(bool treatCharAsNumber = false, bool isReturnType = false);

	template<typename T>
	[[nodiscard]] static constexpr int get_member_type_number();

	

	template<typename ParameterTypeTuple, size_t i>
	[[nodiscard]] bool parse_argument_helper(PyObject* args, ParameterTypeTuple& parsedArguments);
	
	template<typename ParameterTypeTuple, size_t... ArgumentIndices>
	[[nodiscard]] constexpr bool parse_arguments(PyObject* args, ParameterTypeTuple& parsedArguments, std::index_sequence<ArgumentIndices...>);


	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, typename ParameterTypeTuple>
	[[nodiscard]] PyObject* execute_get_return_value(const ParameterTypeTuple& arguments);

	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, typename ClassReferenceType, typename ParameterTypeTuple, typename InstanceType>
	[[nodiscard]] PyObject* execute_and_get_return_value_ref_possible(const ParameterTypeTuple& arguments, InstanceType* pInstance);


	template<typename FieldTypeTuple, typename ExportedClass, typename Offsets, size_t i>
	[[nodiscard]] bool parse_field_helper(ExportedClass* self, PyObject* args);

	template<typename FieldTypeTuple, typename ExportedClass, typename Offsets, size_t... FieldIndices>
	[[nodiscard]] bool parse_fields(ExportedClass* self, PyObject* args, std::index_sequence<FieldIndices...>);
	
	

	template<typename FunctionPtrType, FunctionPtrType FunctionPtr>
	[[nodiscard]] auto get_function_replicated_function();

	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction>
	[[nodiscard]] auto get_member_function_as_static_function_replicated_function();
	
	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename LambdaPtrType, LambdaPtrType LambdaPtr>
	[[nodiscard]] auto get_member_function_as_static_function_lambda_replicated_function();

	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename Class>
	[[nodiscard]] auto get_member_function_replicated_function();


	
	template<typename Pairs, HashValueType FunctionNameHashValue, size_t i>
	void template_function_map_register_helper(std::unordered_map<HashValueType, PyObject* (*)(PyObject*, PyObject*)>& functionMap);

	template<typename Pairs, HashValueType FunctionNameHashValue, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, size_t i>
	void template_member_function_as_static_function_map_register_helper(std::unordered_map<HashValueType, PyObject* (*)(PyObject*, PyObject*)>& functionMap);

	template<typename Pairs, HashValueType FunctionNameHashValue, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename LambdaPtrType, LambdaPtrType LambdaPtr, size_t i>
	void template_member_function_as_static_function_lambda_map_register_helper(std::unordered_map<HashValueType, PyObject* (*)(PyObject*, PyObject*)>& functionMap);

	template<typename Pairs, HashValueType FunctionNameHashValue, typename Class, size_t i>
	void template_member_function_map_register_helper(std::unordered_map<HashValueType, PyObject* (*)(PyObject*, PyObject*)>& functionMap);
	
	
	template<typename Pairs, HashValueType FunctionNameHashValue, size_t... Indices>
	void template_function_register_to_map(std::unordered_map<HashValueType, PyObject* (*)(PyObject*, PyObject*)>& functionMap, std::index_sequence<Indices...>);

	template<typename Pairs, HashValueType FunctionNameHashValue, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, size_t... Indices>
	void template_member_function_as_static_function_register_to_map(std::unordered_map<HashValueType, PyObject* (*)(PyObject*, PyObject*)>& functionMap, std::index_sequence<Indices...>);

	template<typename Pairs, HashValueType FunctionNameHashValue, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename LambdaPtrType, LambdaPtrType LambdaPtr, size_t... Indices>
	void template_member_function_as_static_function_lambda_register_to_map(std::unordered_map<HashValueType, PyObject* (*)(PyObject*, PyObject*)>& functionMap, std::index_sequence<Indices...>);

	template<typename Pairs, HashValueType FunctionNameHashValue, typename Class, size_t... Indices>
	void template_member_function_register_to_map(std::unordered_map<HashValueType, PyObject* (*)(PyObject*, PyObject*)>& functionMap, std::index_sequence<Indices...>);

	

	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename RefToPtrParameterTypeTuple, typename ParameterTypeTuple>
	struct FunctionDispatcher
	{
		static PyObject* ReplicatedFunction(PyObject* self, PyObject* args);
	};

	
	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename RefToPtrParameterTypeTuple, typename ParameterTypeTuple,
		typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename InstanceReturnFunctionReturnType, size_t InstanceReturnFunctionParameterCount, typename InstanceReturnFunctionParameterTuple>
	struct MemberFunctionAsStaticFunctionDispatcher
	{
		static PyObject* ReplicatedFunction(PyObject* self, PyObject* args);
	};

	
	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename RefToPtrParameterTypeTuple, typename ParameterTypeTuple,
		typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename InstanceReturnFunctionReturnType, size_t InstanceReturnFunctionParameterCount, typename InstanceReturnFunctionParameterTypeTuple,
		typename LambdaPtrType, LambdaPtrType LambdaPtr
	>
	struct MemberFunctionAsStaticFunctionLambdaDispatcher
	{
		static PyObject* ReplicatedFunction(PyObject* self, PyObject* args);
	};


	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename RefToPtrParameterTypeTuple, typename ParameterTypeTuple, typename Class>
	struct MemberFunctionDispatcher
	{
		static PyObject* ReplicatedFunction(PyObject* self, PyObject* args);
	};


	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename RefToPtrParameterTypeTuple, typename ParameterTypeTuple, typename Class, EOperatorType Operator>
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


	template<typename FunctionPtrType, FunctionPtrType FunctionPtr>
	static void RegisterMemberFunction(const char* functionName);


	template<typename FunctionPtrType, FunctionPtrType FunctionPtr>
	static void RegisterGlobalOperator(EOperatorType operatorType);

	
	template<typename FunctionPtrType, FunctionPtrType FunctionPtr, typename Class, EOperatorType OperatorType>
	static void RegisterMemberOperator();


	template<HashValueType FunctionNameHashValue, typename... TemplateInstanceInfos>
	static void RegisterTemplateFunction(const char* functionName);


	template<HashValueType FunctionNameHashValue, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename... TemplateInstanceInfos>
	static void RegisterTemplateMemberFunctionAsStaticFunction(const char* functionName);


	template<HashValueType FunctionNameHashValue, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename LambdaPtrType, LambdaPtrType LambdaPtr, typename... TemplateInstanceInfos>
	static void RegisterTemplateMemberFunctionAsStaticFunctionLambda(const char* functionName);


	template<HashValueType FunctionNameHashValue, typename... TemplateInstanceInfos>
	static void RegisterTemplateMemberFunction(const char* functionName);


	template<typename T, typename Offsets, typename... FieldTypes>
	static void RegisterType(const char* typeName, std::initializer_list<PyMemberDef> members);


	template<typename E>
	static void RegisterEnum(const char* enumName, std::initializer_list<EnumeratorInfo<E>> enumerators);


	static void Export(const std::string& moduleName_);


	static PyObject* Init();


private:
	inline static std::string moduleName;
	
	inline static std::vector<PyMethodDef> methods;
	inline static std::vector<PyTypeObject*> types;
	inline static std::vector<std::string> enums;
	
	inline static PyModuleDef moduleDef;
};





template <typename RefToPtrParameterTuple, typename ParameterTuple, size_t... Indices>
ParameterTuple python_embedder_detail::restore_ref_helper(const RefToPtrParameterTuple& params, std::index_sequence<Indices...>)
{
	static_assert(std::tuple_size_v<RefToPtrParameterTuple> == std::tuple_size_v<ParameterTuple>);
	return ParameterTuple{ OriginalValueGetter<std::tuple_element_t<Indices, RefToPtrParameterTuple>>::GetOriginalValue(std::get<Indices>(params))... };
}

	

template <typename T>
PyObject* PyExportedClass<T>::CustomNew(PyTypeObject* type, [[maybe_unused]] PyObject* args, [[maybe_unused]] PyObject* keywords)
{
	PyExportedClass* const self = reinterpret_cast<PyExportedClass*>(type->tp_alloc(type, 0));
	self->pT = new(_NORMAL_BLOCK, __FILE__, __LINE__) T;
	self->isOwner = true;

	return reinterpret_cast<PyObject*>(self);
}


	
template <typename T>
template <typename Offsets, typename ... FieldTypes>
int PyExportedClass<T>::CustomInit(PyObject* self_, PyObject* args, [[maybe_unused]] PyObject* keywords)
{
	if (PyTuple_Size(args) == 0)
	{
		return 1;
	}
	
	PyExportedClass* const self = reinterpret_cast<PyExportedClass*>(self_);

	using FieldTypeTuple = TypeList<FieldTypes...>;
	constexpr size_t fieldCount = std::tuple_size_v<FieldTypeTuple>;

	if (!parse_fields<FieldTypeTuple, PyExportedClass, Offsets>(self, args, std::make_index_sequence<fieldCount>()))
	{
		return -1;
	}

	return 0;
}

	

template <typename T>
void PyExportedClass<T>::CustomDealloc(PyObject* self)
{
	PyExportedClass* exportedSelf = reinterpret_cast<PyExportedClass*>(self);
	if (exportedSelf->isOwner)
	{
		delete exportedSelf->pT;
	}
	Py_TYPE(self)->tp_free(self);
}



template <typename T>
int Converter<T>::ParseValue(PyObject* pyObject, T* pT)
{
	new (pT) T{ *reinterpret_cast<PyExportedClass<T>*>(pyObject)->pT };

	return 1;
}


template <typename T>
int Converter<T>::ParseValuePtr(PyObject* pyObject, T** pT)
{
	*pT = reinterpret_cast<PyExportedClass<T>*>(pyObject)->pT;

	return 1;
}



template <typename T>
PyObject* Converter<T>::BuildValue(T* pT)
{
	PyObject* const object = PyObject_CallObject(reinterpret_cast<PyObject*>(&PyExportedClass<T>::typeInfo), nullptr);
	PyExportedClass<T>* const newObj = reinterpret_cast<PyExportedClass<T>*>(object);
	*newObj->pT = *pT;

	// TODO: To support custom objects, manually parse the arguments and use pyobject_callobject

	return object;
}


template <typename T>
PyObject* Converter<T>::BuildValuePtr(T* pT)
{
	PyExportedClass<T>* const newObj = PyObject_New(PyExportedClass<T>, &PyExportedClass<T>::typeInfo);
	PyObject* const object = reinterpret_cast<PyObject*>(newObj);
	PyObject_Init(object, &PyExportedClass<T>::typeInfo);
	newObj->isOwner = false;
	newObj->pT = pT;

	return object;
}



template <typename T>
constexpr const char* python_embedder_detail::get_type_format_string(const bool treatCharAsNumber, const bool isReturnType)
{
	if constexpr (std::is_same_v<T, bool>)
	{
		return isReturnType ? "i" : "p";
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
	else if constexpr (std::is_enum_v<T>)
	{
		return get_type_format_string<std::underlying_type_t<T>>(treatCharAsNumber, isReturnType);
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



template<typename ParameterTypeTuple, size_t... ArgumentIndices>
constexpr bool python_embedder_detail::parse_arguments(PyObject* args, ParameterTypeTuple& parsedArguments, std::index_sequence<ArgumentIndices...>)
{
	if constexpr (sizeof...(ArgumentIndices) == 0)
	{
		return true;
	}
	else
	{
		const bool parseResults[] = { parse_argument_helper<ParameterTypeTuple, ArgumentIndices>(args, parsedArguments)... };

		return std::all_of(std::begin(parseResults), std::end(parseResults), [](auto e) { return e; });
	}
}


template<typename ParameterTypeTuple, size_t i>
bool python_embedder_detail::parse_argument_helper(PyObject* args, ParameterTypeTuple& parsedArguments)
{
	using NthParameterType = std::tuple_element_t<i, ParameterTypeTuple>;

	PyObject* const pyObjectValue = PyTuple_GetItem(args, i);
	TempVarType<NthParameterType> value;

	if constexpr (std::is_fundamental_v<NthParameterType> || std::is_enum_v<NthParameterType> || std::is_same_v<NthParameterType, const char*>)
	{
		const char* format = get_type_format_string<NthParameterType>();

		if (!PyArg_Parse(pyObjectValue, format, &value))
		{
			return false;
		}
	}
	else if constexpr (std::is_same_v<NthParameterType, std::string>)
	{
		const char* temp;
		if (!PyArg_Parse(pyObjectValue, "s", &temp))
		{
			return false;
		}

		value = temp;
	}
	else if constexpr (std::is_pointer_v<NthParameterType>)
	{
		if (!PyArg_Parse(pyObjectValue, "O&", &Converter<NthParameterType>::ParseValuePtr, &value))
		{
			return false;
		}
	}
	else
	{
		if (!PyArg_Parse(pyObjectValue, "O&", &Converter<NthParameterType>::ParseValue, &value))
		{
			return false;
		}
	}

	std::get<i>(parsedArguments) = std::move(static_cast<NthParameterType>(value));

	return true;
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
		TempVarType<ReturnType> returnValue = static_cast<TempVarType<ReturnType>>(std::apply(FunctionPtr, arguments));

		if constexpr (std::is_fundamental_v<ReturnType> || std::is_enum_v<ReturnType> || std::is_same_v<ReturnType, const char*>)
		{
			const char* format = get_type_format_string<ReturnType>(false, true);

			return Py_BuildValue(format, returnValue);
		}
		else if constexpr (std::is_same_v<ReturnType, std::string>)
		{
			return Py_BuildValue("s", returnValue.c_str());
		}
		else if constexpr (std::is_lvalue_reference_v<ReturnType>)
		{
			return Py_BuildValue("O&", &Converter<std::remove_reference_t<ReturnType>>::BuildValuePtr, &returnValue);
		}
		else
		{
			return Py_BuildValue("O&", &Converter<ReturnType>::BuildValue, &returnValue);
		}
	}
}


	
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, typename ClassReferenceType, typename ParameterTypeTuple, typename InstanceType>
PyObject* python_embedder_detail::execute_and_get_return_value_ref_possible(const ParameterTypeTuple& arguments, InstanceType* pInstance)
{
	if constexpr (std::is_same_v<ReturnType, void>)
	{
		std::apply(FunctionPtr, arguments);

		return Py_None;
	}
	else if constexpr (std::is_same_v<ReturnType, ClassReferenceType>)
	{
		std::apply(FunctionPtr, arguments);

		if constexpr (std::is_same_v<InstanceType, PyObject>)
		{
			return pInstance;
		}
		else
		{
			return Py_BuildValue("O&", &Converter<std::remove_reference_t<ReturnType>>::BuildValue, pInstance);
		}
	}
	else
	{
		TempVarType<ReturnType> returnValue = static_cast<TempVarType<ReturnType>>(std::apply(FunctionPtr, arguments));

		if constexpr (std::is_fundamental_v<ReturnType> || std::is_enum_v<ReturnType> || std::is_same_v<ReturnType, const char*>)
		{
			const char* format = get_type_format_string<ReturnType>(false, true);

			return Py_BuildValue(format, returnValue);
		}
		else if constexpr (std::is_same_v<ReturnType, std::string>)
		{
			return Py_BuildValue("s", returnValue.c_str());
		}
		else if constexpr (std::is_lvalue_reference_v<ReturnType>)
		{
			return Py_BuildValue("O&", &Converter<std::remove_reference_t<ReturnType>>::BuildValuePtr, &returnValue);
		}
		else
		{
			return Py_BuildValue("O&", &Converter<ReturnType>::BuildValue, &returnValue);
		}
	}
}


	
template <typename FieldTypeTuple, typename ExportedClass, typename Offsets, size_t i>
bool python_embedder_detail::parse_field_helper(ExportedClass* self, PyObject* args)
{
	using NthFieldType = std::tuple_element_t<i, FieldTypeTuple>;

	PyObject* const pyObjectValue = PyTuple_GetItem(args, i);
	TempVarType<NthFieldType> value;

	const char* format = get_type_format_string<NthFieldType>(true);

	if (!PyArg_Parse(pyObjectValue, format, &value))
	{
		return false;
	}

	new (reinterpret_cast<char*>(self->pT) + get_nth_element<size_t>(Offsets{}, i)) NthFieldType(static_cast<NthFieldType>(value));

	return true;
}



template <typename FieldTypeTuple, typename ExportedClass, typename Offsets, size_t... FieldIndices>
bool python_embedder_detail::parse_fields(ExportedClass* self, PyObject* args, std::index_sequence<FieldIndices...>)
{
	if constexpr (sizeof...(FieldIndices) == 0)
	{
		return true;
	}
	else
	{
		const bool parseResults[] = { parse_field_helper<FieldTypeTuple, ExportedClass, Offsets, FieldIndices>(self, args)... };

		return std::all_of(std::begin(parseResults), std::end(parseResults), [](auto e) { return e; });
	}
}

	

template <typename FunctionPtrType, FunctionPtrType FunctionPtr>
auto python_embedder_detail::get_function_replicated_function()
{
	using namespace function_types;

	static_assert(std::is_function_v<std::remove_pointer_t<FunctionPtrType>>);
	static_assert(!std::is_member_function_pointer_v<FunctionPtrType>);
	
	constexpr size_t parameterCount = FunctionTypes<FunctionPtrType>::parameterCount;
	using ParameterTypeTuple = RemoveConstRefParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters>;
	using RefToPtrParameterTypeTuple = RemoveConstRefRefToPtrParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters>;
	using ReturnType = typename FunctionTypes<FunctionPtrType>::ReturnType;
	
	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount>::value);
	static_assert(is_supported_return_type_v<ReturnType>);

	using InstantiatedDispatcher = FunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, RefToPtrParameterTypeTuple, ParameterTypeTuple>;

	return &InstantiatedDispatcher::ReplicatedFunction;
}

	

template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction>
auto python_embedder_detail::get_member_function_as_static_function_replicated_function()
{
	using namespace function_types;

	static_assert(std::is_member_function_pointer_v<FunctionPtrType>);
	static_assert(std::is_function_v<std::remove_pointer_t<InstanceReturnFunctionType>>);

	constexpr size_t parameterCount = FunctionTypes<FunctionPtrType>::parameterCount;
	using ParameterTypeTuple = RemoveConstRefParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters>;
	using RefToPtrParameterTypeTuple = RemoveConstRefRefToPtrParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters>;
	using ReturnType = typename FunctionTypes<FunctionPtrType>::ReturnType;

	constexpr size_t instanceReturnerParameterCount = FunctionTypes<InstanceReturnFunctionType>::parameterCount;
	using InstanceReturnerParameterTypeTuple = RemoveConstRefRefToPtrParameterTuple<typename FunctionTypes<InstanceReturnFunctionType>::Parameters>;
	using InstanceReturnerReturnType = typename FunctionTypes<InstanceReturnFunctionType>::ReturnType;

	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount>::value);
	static_assert(is_supported_return_type_v<ReturnType> || std::is_same_v<ReturnType, std::remove_pointer_t<InstanceReturnerReturnType>&>);

	static_assert(ValidityChecker<is_supported_parameter_type, InstanceReturnerParameterTypeTuple, instanceReturnerParameterCount>::value);
	static_assert(std::is_same_v<InstanceReturnerReturnType, typename FunctionTypes<FunctionPtrType>::Class*>);
	
	using InstantiatedDispatcher = MemberFunctionAsStaticFunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, RefToPtrParameterTypeTuple, ParameterTypeTuple,
		InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnerReturnType, instanceReturnerParameterCount, InstanceReturnerParameterTypeTuple>;

	return &InstantiatedDispatcher::ReplicatedFunction;
}

	

template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename LambdaPtrType, LambdaPtrType LambdaPtr>
auto python_embedder_detail::get_member_function_as_static_function_lambda_replicated_function()
{
	using namespace function_types;

	static_assert(std::is_member_function_pointer_v<FunctionPtrType>);

	constexpr size_t parameterCount = FunctionTypes<FunctionPtrType>::parameterCount;
	using ParameterTypeTuple = RemoveConstRefParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters>;
	using RefToPtrParameterTypeTuple = RemoveConstRefRefToPtrParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters>;
	using ReturnType = typename FunctionTypes<FunctionPtrType>::ReturnType;

	constexpr size_t instanceReturnerParameterCount = FunctionTypes<InstanceReturnFunctionType>::parameterCount;
	using InstanceReturnerParameterTypeTuple = RemoveConstRefRefToPtrParameterTuple<typename FunctionTypes<InstanceReturnFunctionType>::Parameters>;
	using InstanceReturnerReturnType = typename FunctionTypes<InstanceReturnFunctionType>::ReturnType;
	
	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount>::value);
	static_assert(is_supported_return_type_v<ReturnType> || std::is_same_v<ReturnType, std::remove_pointer_t<InstanceReturnerReturnType>&>);

	static_assert(ValidityChecker<is_supported_parameter_type, InstanceReturnerParameterTypeTuple, instanceReturnerParameterCount>::value);
	static_assert(std::is_same_v<InstanceReturnerReturnType, typename FunctionTypes<FunctionPtrType>::Class*>);

	using InstantiatedDispatcher = MemberFunctionAsStaticFunctionLambdaDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, RefToPtrParameterTypeTuple, ParameterTypeTuple,
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
	using ParameterTypeTuple = RemoveConstRefParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters>;
	using RefToPtrParameterTypeTuple = RemoveConstRefRefToPtrParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters>;
	using ReturnType = typename FunctionTypes<FunctionPtrType>::ReturnType;

	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount>::value);
	static_assert(is_supported_return_type_v<ReturnType> || std::is_same_v<ReturnType, Class&>);

	using InstantiatedDispatcher = MemberFunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, RefToPtrParameterTypeTuple, ParameterTypeTuple, Class>;

	return &InstantiatedDispatcher::ReplicatedFunction;
}


	
template <typename Pairs, HashValueType FunctionNameHashValue, size_t i>
void python_embedder_detail::template_function_map_register_helper(std::unordered_map<HashValueType, PyObject* (*)(PyObject*, PyObject*)>& functionMap)
{
	using Pair = std::tuple_element_t<i, Pairs>;
	using FunctionPtrType = typename Pair::FunctionPtrType;
	constexpr FunctionPtrType functionPtr = Pair::functionPtr;
	constexpr HashValueType templateParametersHashValue = Pair::templateParametersHashValue;

	functionMap[FunctionNameHashValue ^ templateParametersHashValue] = get_function_replicated_function<FunctionPtrType, functionPtr>();
}


	
template <typename Pairs, HashValueType FunctionNameHashValue, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, size_t i>
void python_embedder_detail::template_member_function_as_static_function_map_register_helper(std::unordered_map<HashValueType, PyObject*(*)(PyObject*, PyObject*)>& functionMap)
{
	using Pair = std::tuple_element_t<i, Pairs>;
	using FunctionPtrType = typename Pair::FunctionPtrType;
	constexpr FunctionPtrType functionPtr = Pair::functionPtr;
	constexpr HashValueType templateParametersHashValue = Pair::templateParametersHashValue;

	functionMap[FunctionNameHashValue ^ templateParametersHashValue] = get_member_function_as_static_function_replicated_function<FunctionPtrType, functionPtr, InstanceReturnFunctionType, InstanceReturnFunction>();
}

	

template <typename Pairs, HashValueType FunctionNameHashValue, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename LambdaPtrType, LambdaPtrType LambdaPtr, size_t i>
void python_embedder_detail::template_member_function_as_static_function_lambda_map_register_helper(std::unordered_map<HashValueType, PyObject*(*)(PyObject*, PyObject*)>& functionMap)
{
	using Pair = std::tuple_element_t<i, Pairs>;
	using FunctionPtrType = typename Pair::FunctionPtrType;
	constexpr FunctionPtrType functionPtr = Pair::functionPtr;
	constexpr HashValueType templateParametersHashValue = Pair::templateParametersHashValue;

	functionMap[FunctionNameHashValue ^ templateParametersHashValue] = get_member_function_as_static_function_lambda_replicated_function<FunctionPtrType, functionPtr, InstanceReturnFunctionType, InstanceReturnFunction, LambdaPtrType, LambdaPtr>();
}


	
template <typename Pairs, HashValueType FunctionNameHashValue, typename Class, size_t i>
void python_embedder_detail::template_member_function_map_register_helper(std::unordered_map<HashValueType, PyObject*(*)(PyObject*, PyObject*)>& functionMap)
{
	using Pair = std::tuple_element_t<i, Pairs>;
	using FunctionPtrType = typename Pair::FunctionPtrType;
	constexpr FunctionPtrType functionPtr = Pair::functionPtr;
	constexpr HashValueType templateParametersHashValue = Pair::templateParametersHashValue;

	functionMap[FunctionNameHashValue ^ templateParametersHashValue] = get_member_function_replicated_function<FunctionPtrType, functionPtr, Class>();
}



template <typename Pairs, HashValueType FunctionNameHashValue, size_t... Indices>
void python_embedder_detail::template_function_register_to_map(std::unordered_map<HashValueType, PyObject*(*)(PyObject*, PyObject*)>& functionMap, std::index_sequence<Indices...>)
{
	using dummy = int[];
	(void) dummy { (template_function_map_register_helper<Pairs, FunctionNameHashValue, Indices>(functionMap), 0)... };
}

	

template <typename Pairs, HashValueType FunctionNameHashValue, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, size_t... Indices>
void python_embedder_detail::template_member_function_as_static_function_register_to_map(std::unordered_map<HashValueType, PyObject*(*)(PyObject*, PyObject*)>& functionMap, std::index_sequence<Indices...>)
{
	using dummy = int[];
	(void) dummy { (template_member_function_as_static_function_map_register_helper<Pairs, FunctionNameHashValue, InstanceReturnFunctionType, InstanceReturnFunction, Indices>(functionMap), 0)... };
}

	

template <typename Pairs, HashValueType FunctionNameHashValue, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename LambdaPtrType, LambdaPtrType LambdaPtr, size_t... Indices>
void python_embedder_detail::template_member_function_as_static_function_lambda_register_to_map(std::unordered_map<HashValueType, PyObject*(*)(PyObject*, PyObject*)>& functionMap, std::index_sequence<Indices...>)
{
	using dummy = int[];
	(void) dummy { (template_member_function_as_static_function_lambda_map_register_helper<Pairs, FunctionNameHashValue, InstanceReturnFunctionType, InstanceReturnFunction, LambdaPtrType, LambdaPtr, Indices>(functionMap), 0)... };
}


	
template <typename Pairs, HashValueType FunctionNameHashValue, typename Class, size_t... Indices>
void python_embedder_detail::template_member_function_register_to_map(std::unordered_map<HashValueType, PyObject*(*)(PyObject*, PyObject*)>& functionMap, std::index_sequence<Indices...>)
{
	using dummy = int[];
	(void) dummy { (template_member_function_map_register_helper<Pairs, FunctionNameHashValue, Class, Indices>(functionMap), 0)... };
}



template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename RefToPtrParameterTypeTuple, typename ParameterTypeTuple>
PyObject* FunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, RefToPtrParameterTypeTuple, ParameterTypeTuple>
	::ReplicatedFunction([[maybe_unused]] PyObject* self, PyObject* args)
{
	RefToPtrParameterTypeTuple arguments;
	
	if (!parse_arguments<RefToPtrParameterTypeTuple>(args, arguments, std::make_index_sequence<ParameterCount>()))
	{
		return nullptr;
	}

	ParameterTypeTuple originalArguments = restore_ref<RefToPtrParameterTypeTuple, ParameterTypeTuple>(arguments);
	
	PyObject* toReturn = execute_get_return_value<FunctionPtrType, FunctionPtr, ReturnType>(originalArguments);
	Py_INCREF(toReturn);
	return toReturn;
}

	

template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename RefToPtrParameterTypeTuple, typename ParameterTypeTuple, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename InstanceReturnFunctionReturnType, size_t InstanceReturnFunctionParameterCount, typename InstanceReturnFunctionParameterTypeTuple>
PyObject* MemberFunctionAsStaticFunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, RefToPtrParameterTypeTuple, ParameterTypeTuple, InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnFunctionReturnType, InstanceReturnFunctionParameterCount, InstanceReturnFunctionParameterTypeTuple>
	::ReplicatedFunction([[maybe_unused]] PyObject* self, PyObject* args)
{
	InstanceReturnFunctionParameterTypeTuple instanceReturnerArguments;

	PyObject* const instanceReturnFunctionArgs = PyTuple_GetItem(args, 0);
	
	if (!parse_arguments<InstanceReturnFunctionParameterTypeTuple>(instanceReturnFunctionArgs, instanceReturnerArguments, std::make_index_sequence<InstanceReturnFunctionParameterCount>()))
	{
		return nullptr;
	}
	

	RefToPtrParameterTypeTuple arguments;

	PyObject* const realArgs = PyTuple_GetSlice(args, 1, ParameterCount + 1);

	if (!parse_arguments<RefToPtrParameterTypeTuple>(realArgs, arguments, std::make_index_sequence<ParameterCount>()))
	{
		Py_DECREF(realArgs);
		return nullptr;
	}
	Py_DECREF(realArgs);
	
	ParameterTypeTuple originalArguments = restore_ref<RefToPtrParameterTypeTuple, ParameterTypeTuple>(arguments);
	
	InstanceReturnFunctionReturnType pInstance = std::apply(InstanceReturnFunction, instanceReturnerArguments);
	auto thisPtrAddedArguments = std::tuple_cat(std::make_tuple(pInstance), originalArguments);

	PyObject* toReturn = execute_and_get_return_value_ref_possible<FunctionPtrType, FunctionPtr, ReturnType, std::remove_pointer_t<InstanceReturnFunctionReturnType>&>
		(thisPtrAddedArguments, pInstance);

	Py_INCREF(toReturn);
	return toReturn;
}


	
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename RefToPtrParameterTypeTuple, typename ParameterTypeTuple, typename InstanceReturnFunctionType, InstanceReturnFunctionType InstanceReturnFunction, typename InstanceReturnFunctionReturnType, size_t InstanceReturnFunctionParameterCount, typename InstanceReturnFunctionParameterTypeTuple, typename LambdaPtrType, LambdaPtrType LambdaPtr>
PyObject* MemberFunctionAsStaticFunctionLambdaDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, RefToPtrParameterTypeTuple, ParameterTypeTuple, InstanceReturnFunctionType, InstanceReturnFunction, InstanceReturnFunctionReturnType, InstanceReturnFunctionParameterCount, InstanceReturnFunctionParameterTypeTuple, LambdaPtrType, LambdaPtr>
	::ReplicatedFunction([[maybe_unused]] PyObject* self, PyObject* args)
{
	InstanceReturnFunctionParameterTypeTuple instanceReturnerArguments;

	PyObject* const instanceReturnFunctionArgs = PyTuple_GetItem(args, 0);

	if (!parse_arguments<InstanceReturnFunctionParameterTypeTuple>(instanceReturnFunctionArgs, instanceReturnerArguments, std::make_index_sequence<InstanceReturnFunctionParameterCount>()))
	{
		return nullptr;
	}


	RefToPtrParameterTypeTuple arguments;

	PyObject* const realArgs = PyTuple_GetSlice(args, 1, ParameterCount + 1);

	if (!parse_arguments<RefToPtrParameterTypeTuple>(realArgs, arguments, std::make_index_sequence<ParameterCount>()))
	{
		Py_DECREF(realArgs);
		return nullptr;
	}
	Py_DECREF(realArgs);
	
	ParameterTypeTuple originalArguments = restore_ref<RefToPtrParameterTypeTuple, ParameterTypeTuple>(arguments);
	
	auto thisPtrAddedInstanceReturnerArguments = std::tuple_cat(std::make_tuple(LambdaPtr), instanceReturnerArguments);
	InstanceReturnFunctionReturnType pInstance = std::apply(InstanceReturnFunction, thisPtrAddedInstanceReturnerArguments);
	
	auto thisPtrAddedArguments = std::tuple_cat(std::make_tuple(pInstance), originalArguments);

	PyObject* toReturn = execute_and_get_return_value_ref_possible<FunctionPtrType, FunctionPtr, ReturnType, std::remove_pointer_t<InstanceReturnFunctionReturnType>&>
		(thisPtrAddedArguments, pInstance);

	Py_INCREF(toReturn);
	return toReturn;
}



template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename RefToPtrParameterTypeTuple, typename ParameterTypeTuple, typename Class>
PyObject* MemberFunctionDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, RefToPtrParameterTypeTuple, ParameterTypeTuple, Class>
	::ReplicatedFunction(PyObject* self, PyObject* args)
{
	RefToPtrParameterTypeTuple arguments;

	if (!parse_arguments<RefToPtrParameterTypeTuple>(args, arguments, std::make_index_sequence<ParameterCount>()))
	{
		return nullptr;
	}
	
	ParameterTypeTuple originalArguments = restore_ref<RefToPtrParameterTypeTuple, ParameterTypeTuple>(arguments);
	
	auto thisPtrAddedArguments = std::tuple_cat(std::make_tuple(reinterpret_cast<PyExportedClass<Class>*>(self)->pT), originalArguments);

	PyObject* toReturn = execute_and_get_return_value_ref_possible<FunctionPtrType, FunctionPtr, ReturnType, Class&>(thisPtrAddedArguments, self);

	Py_INCREF(toReturn);
	return toReturn;
}

	

template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename RefToPtrParameterTypeTuple, typename ParameterTypeTuple, typename Class, EOperatorType Operator>
PyObject* MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, RefToPtrParameterTypeTuple, ParameterTypeTuple, Class, Operator>
	::BinaryReplicatedFunction(PyObject* self, PyObject* arg)
{
	RefToPtrParameterTypeTuple arguments;

	PyObject* const argTuple = PyTuple_Pack(1, arg);

	if (!parse_arguments<RefToPtrParameterTypeTuple>(argTuple, arguments, std::make_index_sequence<ParameterCount>()))
	{
		Py_DECREF(argTuple);
		return nullptr;
	}
	Py_DECREF(argTuple);

	ParameterTypeTuple originalArguments = restore_ref<RefToPtrParameterTypeTuple, ParameterTypeTuple>(arguments);

	auto thisPtrAddedArguments = std::tuple_cat(std::make_tuple(reinterpret_cast<PyExportedClass<Class>*>(self)->pT), originalArguments);

	PyObject* toReturn = execute_and_get_return_value_ref_possible<FunctionPtrType, FunctionPtr, ReturnType, Class&>(thisPtrAddedArguments, self);

	Py_INCREF(toReturn);
	return toReturn;
}

	

template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename ReturnType, size_t ParameterCount, typename RefToPtrParameterTypeTuple, typename ParameterTypeTuple, typename Class, EOperatorType Operator>
PyObject* MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, ParameterCount, RefToPtrParameterTypeTuple, ParameterTypeTuple, Class, Operator>
	::UnaryReplicatedFunction(PyObject* self)
{
	auto thisPtrAddedArguments = std::make_tuple(reinterpret_cast<PyExportedClass<Class>*>(self)->pT);

	PyObject* toReturn = execute_and_get_return_value_ref_possible<FunctionPtrType, FunctionPtr, ReturnType, Class&>(thisPtrAddedArguments, self);

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
template <typename FunctionPtrType, FunctionPtrType FunctionPtr>
void Exporter<ModuleNameHashValue>::RegisterMemberFunction(const char* functionName)
{
	using Class = typename function_types::FunctionTypes<FunctionPtrType>::Class;
	
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
template <typename FunctionPtrType, FunctionPtrType FunctionPtr, typename Class, EOperatorType OperatorType>
void Exporter<ModuleNameHashValue>::RegisterMemberOperator()
{
	using namespace function_types;

	static_assert(is_supported_custom_type_v<Class>);

	constexpr size_t parameterCount = FunctionTypes<FunctionPtrType>::parameterCount;
	using ParameterTypeTuple = RemoveConstRefParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters>;
	using RefToPtrParameterTypeTuple = RemoveConstRefRefToPtrParameterTuple<typename FunctionTypes<FunctionPtrType>::Parameters>;
	using ReturnType = typename FunctionTypes<FunctionPtrType>::ReturnType;
	
	static_assert(ValidityChecker<is_supported_parameter_type, ParameterTypeTuple, parameterCount>::value);
	static_assert(is_supported_return_type_v<ReturnType> || std::is_same_v<ReturnType, Class&>);


	if constexpr (parameterCount == 0)  // unary
	{
		*reinterpret_cast<unaryfunc*>(reinterpret_cast<char*>(&PyExportedClass<Class>::numberMethods) + static_cast<size_t>(OperatorType))
			= &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, RefToPtrParameterTypeTuple, ParameterTypeTuple, Class, OperatorType>::UnaryReplicatedFunction;
	}
	else if constexpr (parameterCount == 1)  // binary
	{
		*reinterpret_cast<binaryfunc*>(reinterpret_cast<char*>(&PyExportedClass<Class>::numberMethods) + static_cast<size_t>(OperatorType))
			= &MemberOperatorDispatcher<FunctionPtrType, FunctionPtr, ReturnType, parameterCount, RefToPtrParameterTypeTuple, ParameterTypeTuple, Class, OperatorType>::BinaryReplicatedFunction;
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

	template_function_register_to_map<Pairs, FunctionNameHashValue>(functionMap, std::make_index_sequence<pairCount>());
	
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

	template_member_function_as_static_function_register_to_map<Pairs, FunctionNameHashValue, InstanceReturnFunctionType, InstanceReturnFunction>(functionMap, std::make_index_sequence<pairCount>());

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

	template_member_function_as_static_function_lambda_register_to_map<Pairs, FunctionNameHashValue, InstanceReturnFunctionType, InstanceReturnFunction, LambdaPtrType, LambdaPtr>(functionMap, std::make_index_sequence<pairCount>());

	methods.push_back(PyMethodDef{ functionName, &FunctionMapper::ReplicatedFunction, METH_VARARGS, nullptr });
}

	

template <HashValueType ModuleNameHashValue>
template <HashValueType FunctionNameHashValue, typename... TemplateInstanceInfos>
void Exporter<ModuleNameHashValue>::RegisterTemplateMemberFunction(const char* functionName)
{
	using Pairs = TypeList<TemplateInstanceInfos...>;
	constexpr size_t pairCount = std::tuple_size_v<Pairs>;
	
	using Class = typename function_types::FunctionTypes<typename std::tuple_element_t<0, Pairs>::FunctionPtrType>::Class;
	
	if (!PyExportedClass<Class>::methods.empty() && PyExportedClass<Class>::methods.back().ml_name == nullptr)
	{
		throw std::runtime_error("All member functions must be exported first before its type");
	}
	

	using FunctionMapper = TemplateFunctionDispatcher<FunctionNameHashValue>;
	auto& functionMap = FunctionMapper::instantiatedFunctions;

	template_member_function_register_to_map<Pairs, FunctionNameHashValue, Class>(functionMap, std::make_index_sequence<pairCount>());

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
template <typename E>
void Exporter<ModuleNameHashValue>::RegisterEnum(const char* enumName, std::initializer_list<EnumeratorInfo<E>> enumerators)
{
	static_assert(std::is_enum_v<E>);

	std::string enumDeclaration = "class ";
	enumDeclaration += enumName;
	enumDeclaration += "(IntEnum):\n";
	for (EnumeratorInfo<E> enumerator : enumerators)
	{
		if (enumerator.name == nullptr)
		{
			break;
		}

		enumDeclaration += "\t";
		enumDeclaration += enumerator.name;
		enumDeclaration += " = ";
		enumDeclaration += std::to_string(static_cast<std::underlying_type_t<E>>(enumerator.value));
		enumDeclaration += "\n";
	}
	
	enums.push_back(std::move(enumDeclaration));
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
	PyRun_SimpleString("from enum import IntEnum");
	for (const std::string& enumDeclaration : enums)
	{
		PyRun_SimpleString(enumDeclaration.c_str());
	}

	return pyModule;
}
}
