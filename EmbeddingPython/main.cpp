// This should be included before any std header
#include "Exporter.h"

#include <iostream>

#include <xxhash_cx/xxhash_cx.h>


//template<typename T>
//static PyObject* spam_system([[maybe_unused]] PyObject* self, PyObject* args)
//{
//	int command;
//
//	if (!PyArg_ParseTuple(args, "i", &command))
//	{
//		return nullptr;
//	}
//
//	std::cout << command;
//	
//	return PyLong_FromLong(0);
//}
//
//static struct PyMethodDef methods[]
//{
//	{ "say_hi", &spam_system<int>, METH_VARARGS, "" },
//	{ nullptr, nullptr, 0, nullptr }
//};
//
//static struct PyModuleDef moduleDef{
//	PyModuleDef_HEAD_INIT, "test", "test module", -1, methods,
//	nullptr, nullptr, nullptr, nullptr
//};
//
//
//PyMODINIT_FUNC PyInit_void()
//{
//	return PyModule_Create(&moduleDef);
//}


std::string add_5(int i)
{
	return std::string("str: 5") + std::to_string(i);
}

// 14 parameters
void crazy_function(bool b, char c, unsigned char C, short s, unsigned short S, int i, unsigned int I,
	long l, unsigned long L, long long ll, unsigned long long LL, float f, double d, std::string str)
{
	std::cout << std::boolalpha << b << std::endl << std::noboolalpha <<
		c << std::endl << int(C) << std::endl << s << std::endl << S << std::endl <<
		i << std::endl << I << std::endl << l << std::endl << L << std::endl <<
		ll << std::endl << LL << std::endl << f << std::endl << d << std::endl <<
		str << std::endl;
}

int main()
{
	// https://docs.python.org/3/extending/extending.html
	// https://www.codeproject.com/Articles/820116/Embedding-Python-program-in-a-C-Cplusplus-code
	//
	// https://stackoverflow.com/questions/19282054/parsing-user-defined-types-using-pyarg-parsetuple
	// https://docs.python.org/3.8/c-api/arg.html

	
	using xxhash::literals::operator ""_xxh64;

	Exporter<"test"_xxh64>::RegisterFunction<decltype(&add_5), &add_5>("add_5");
	Exporter<"test"_xxh64>::RegisterFunction<decltype(&crazy_function), &crazy_function>("crazy_function");
	Exporter<"test"_xxh64>::Export("test");
	
	Py_Initialize();

	const char scriptName[] = "test.py";
	FILE* script = _Py_fopen(scriptName, "r");
	PyRun_AnyFile(script, scriptName);

	Py_Finalize();
	
	return 0;
}
