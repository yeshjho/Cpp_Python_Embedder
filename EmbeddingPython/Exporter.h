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


#define PY_EXPORT_STATIC_FUNCTION(func, funcName, moduleName) Exporter<#moduleName##_xxh64>::RegisterFunction<decltype(&##func), &##func>(#funcName)
#define PY_EXPORT_GLOBAL_FUNCTION(funcName, moduleName) PY_EXPORT_STATIC_FUNCTION(funcName, funcName, moduleName);
#define PY_EXPORT_MEMBER_FUNCTION(func, funcName, instanceReturner, moduleName) Exporter<#moduleName##_xxh64>::RegisterMemberFunction<decltype(&##func), &##func, decltype(&##instanceReturner), &##instanceReturner>(#funcName)

#define PY_EXPORT_TYPE(T, moduleName, seq) Exporter<#moduleName##_xxh64>::RegisterType<T, std::integer_sequence<size_t, PY_EXPORTER_FIELD_OFFSETS(T, seq)>, PY_EXPORTER_FIELD_TYPES(T, seq)>(#T, { PY_EXPORTER_FIELDS(T, seq) })

#define PY_EXPORT_MODULE(moduleName) Exporter<#moduleName##_xxh64>::Export(#moduleName)


//#define PY_EXPORT_MEMBER_FUNCTION(func, funcName, instanceReturner, moduleName) static auto funcName##instanceReturnFunction = instanceReturner; \
//	Exporter<#moduleName##_xxh64>::RegisterMemberFunction<decltype(&##func), &##func, decltype(&##funcName##instanceReturnFunction), &##funcName##instanceReturnFunction>(#funcName)
// TODO: Support Lambda https://qiita.com/angeart/items/94734d68999eca575881



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



	template<typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>&& std::is_default_constructible_v<T>>>
	struct PyExportedClass
	{
		PyObject_HEAD
		T t;


		inline static PyTypeObject typeInfo;
		inline static std::vector<PyMemberDef> fields;

		
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
	PyExportedClass<T> value{};
	value.t = *pT;
	
	const size_t SIZE = PyExportedClass<T>::fields.size();

	PyObject* const args = PyTuple_New(SIZE);
	
	for (size_t i = 0; i < SIZE; ++i)
	{
		const PyMemberDef& def = PyExportedClass<T>::fields.at(i);

		const Py_ssize_t offset = def.offset;

		void* const address = reinterpret_cast<char*>(&(value.t)) + offset;
		PyObject* argument = nullptr;
		switch (def.type)
		{
			case T_BOOL:
				argument = Py_BuildValue("p", *reinterpret_cast<bool*>(address));
				break;
			
			case T_BYTE:
				argument = Py_BuildValue("C", *reinterpret_cast<char*>(address));
				break;

			case T_UBYTE:
				argument = Py_BuildValue("B", *reinterpret_cast<unsigned char*>(address));
				break;

			case T_SHORT:
				argument = Py_BuildValue("h", *reinterpret_cast<short*>(address));
				break;

			case T_USHORT:
				argument = Py_BuildValue("H", *reinterpret_cast<unsigned short*>(address));
				break;

			case T_INT:
				argument = Py_BuildValue("i", *reinterpret_cast<int*>(address));
				break;

			case T_UINT:
				argument = Py_BuildValue("I", *reinterpret_cast<unsigned int*>(address));
				break;

			case T_LONG:
				argument = Py_BuildValue("l", *reinterpret_cast<long*>(address));
				break;

			case T_ULONG:
				argument = Py_BuildValue("k", *reinterpret_cast<unsigned long*>(address));
				break;

			case T_LONGLONG:
				argument = Py_BuildValue("L", *reinterpret_cast<long long*>(address));
				break;

			case T_ULONGLONG:
				argument = Py_BuildValue("K", *reinterpret_cast<unsigned long long*>(address));
				break;

			case T_FLOAT:
				argument = Py_BuildValue("f", *reinterpret_cast<float*>(address));
				break;

			case T_DOUBLE:
				argument = Py_BuildValue("d", *reinterpret_cast<double*>(address));
				break;

			case T_STRING:
				argument = Py_BuildValue("s", *reinterpret_cast<const char**>(address));
				break;

			default:
				_ASSERT(false);
				break;
		}
		PyTuple_SetItem(args, i, argument);
		//Py_DECREF(argument);
	}

	PyObject* const typeObject = reinterpret_cast<PyObject*>(&PyExportedClass<T>::typeInfo);
	PyObject* toReturn = PyObject_CallObject(typeObject, args);
	
	Py_DECREF(args);

	return toReturn;
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



template <typename T>
constexpr const char* python_embedder_detail::get_argument_type_format_string(const bool treatCharAsNumber)
{
	if constexpr (std::is_same_v<T, bool>)
	{
		return "p";
	}
	// Need an assumption that the argument will never be a number, rather a string with single character
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
	// This time, assume it as a number
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


template <unsigned long long ModuleNameHashValue>
template <typename T, typename Offsets, typename... FieldTypes>
void Exporter<ModuleNameHashValue>::RegisterType(const char* typeName, std::initializer_list<PyMemberDef> members)
{
	using PyExportedType = python_embedder_detail::PyExportedClass<T>;

	PyExportedType::fields = members;
	
	PyTypeObject typeObject{ PyVarObject_HEAD_INIT(nullptr, 0) };
	typeObject.tp_name = typeName;
	typeObject.tp_basicsize = sizeof(PyExportedType);
	typeObject.tp_itemsize = 0;
	typeObject.tp_flags = Py_TPFLAGS_DEFAULT;
	typeObject.tp_new = &PyExportedType::CustomNew;
	typeObject.tp_init = &PyExportedType::template CustomInit<Offsets, FieldTypes...>;
	typeObject.tp_dealloc = &PyExportedType::CustomDealloc;
	typeObject.tp_members = PyExportedType::fields.data();

	PyExportedType::typeInfo = typeObject;
	types.push_back(typeObject);
}
