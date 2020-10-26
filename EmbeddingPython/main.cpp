// This should be included before any std header
#include "Exporter.h"

#include <iostream>



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

class C
{
public:
	static void Print()
	{
		std::cout << "All is good!" << std::endl;
	}

	C() = default;
	C(int i): i(i){}

	void Print2(std::string s)
	{
		std::cout << "Called! " << i << s << std::endl;
	}



	int i;
	short j;
};

C* instance_returner(int i)
{
	static C c(i);
	return &c;
}


struct Vec
{
	const char* x;
	double y;
	int z;
};


void f(Vec v)
{
	std::cout << v.x << " " << v.y << " " << v.z << std::endl;
}

Vec g(float i)
{
	return Vec{ "default string", i * 200, int(i * 300) };
}

void h(float f)
{
	std::cout << f << std::endl;
}

void i(const char* c)
{
	std::cout << c << std::endl;
}


int main()
{
	// https://docs.python.org/3/extending/extending.html
	// https://www.codeproject.com/Articles/820116/Embedding-Python-program-in-a-C-Cplusplus-code
	//
	// https://stackoverflow.com/questions/19282054/parsing-user-defined-types-using-pyarg-parsetuple
	// 
	// https://docs.python.org/3/c-api/
	// https://docs.python.org/3.8/c-api/arg.html
	// https://docs.python.org/3/extending/newtypes_tutorial.html

	PY_EXPORT_GLOBAL_FUNCTION(add_5, test);
	PY_EXPORT_GLOBAL_FUNCTION(crazy_function, test);
	PY_EXPORT_STATIC_FUNCTION(C::Print, Print, test);
	PY_EXPORT_MEMBER_FUNCTION(C::Print2, Print2, instance_returner, test);
	
	PY_EXPORT_TYPE(Vec, test, (x)(y)(z));

	PY_EXPORT_GLOBAL_FUNCTION(f, test);
	PY_EXPORT_GLOBAL_FUNCTION(g, test);
	PY_EXPORT_GLOBAL_FUNCTION(h, test);
	PY_EXPORT_GLOBAL_FUNCTION(i, test);
	
	PY_EXPORT_MODULE(test);
	
	Py_Initialize();

	const char scriptName[] = "test.py";
	FILE* script = _Py_fopen(scriptName, "r");
	PyRun_AnyFile(script, scriptName);

	Py_Finalize();
	
	return 0;
}
