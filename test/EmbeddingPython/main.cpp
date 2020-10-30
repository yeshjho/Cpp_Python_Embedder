// This should be included before any std header
#include "../../CppPythonEmbedder.hpp"
#include <glm/glm.hpp>
#include <iostream>


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

	void Print2(const std::string& s) const
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
	float x;
	float y;
	float z;

	Vec add(Vec c) const
	{
		return { x + c.x, y + c.y, z + c.z };
	}

	Vec& increment(int i)
	{
		x += i;
		y += i;
		z += i;

		return *this;
	}

	Vec& operator+=(const Vec& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;

		return *this;
	}

	Vec operator-() const
	{
		return { -x, -y, -z };
	}
};


void f(Vec v)
{
	std::cout << v.x << " " << v.y << " " << v.z << std::endl;
}

void h(float f)
{
	std::cout << f << std::endl;
}

glm::vec3 i(float i)
{
	return glm::vec3{ i * 100.f, i * 200.f, i * 300.f };
}

void j(glm::vec3 v)
{
	std::cout << v.x << " " << v.y << " " << v.z << std::endl;
}


class T
{
public:
	template<typename A, typename B, typename C>
	static void f(int i)
	{
		std::cout << i << " " << __FUNCSIG__ << std::endl;
	}

	template<typename T>
	float g()
	{
		std::cout << __FUNCSIG__ << std::endl;

		return 123.456f;
	}

	template<typename T>
	void h()
	{
		std::cout << __FUNCSIG__ << std::endl;
	}

	int i;
	int j;
};

class Zero
{
public:
	void Z() const
	{
		std::cout << "zero" << std::endl;
	}
};

class One
{
public:
	int o = 77777;

	void O() const
	{
		std::cout << o << std::endl;
	}
};

T* t_returner()
{
	static T t;
	return &t;
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
	// https://docs.python.org/3/c-api/typeobj.html

	
	PY_EXPORT_GLOBAL_FUNCTION(add_5, test);
	PY_EXPORT_GLOBAL_FUNCTION(crazy_function, test);
	PY_EXPORT_STATIC_FUNCTION(C, Print, test);

	int ii = 555;
	PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA(C, Print2, [=]() {
		static C c(ii);
		return &c;
	}, test);
	// PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION(C, Print2, instance_returner, test);

	PY_EXPORT_MEMBER_FUNCTION(Vec, add, test);
	PY_EXPORT_MEMBER_FUNCTION_NAME(Vec, add, q, test);
	PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA(Vec, increment, []() { static Vec v; return &v; }, test);
	//PY_EXPORT_MEMBER_FUNCTION(Vec, increment, test);
	PY_EXPORT_MEMBER_OPERATOR(Vec, operator+=, cpp_python_embedder::EOperatorType::INPLACE_ADD, test);
	PY_EXPORT_MEMBER_OPERATOR(Vec, operator-, cpp_python_embedder::EOperatorType::NEGATIVE, test);
	PY_EXPORT_TYPE(Vec, test, (x)(y)(z));

	PY_EXPORT_TYPE_NAME(glm::vec3, vec3, test, (x)(y)(z));

	PY_EXPORT_GLOBAL_FUNCTION_NAME((glm::dot<3, float, glm::defaultp>), dot, test);
	
	PY_EXPORT_TEMPLATE_STATIC_FUNCTION_NAME(T, f, qwer, test, ((int, float, double))((std::string, long, char))((unsigned char, long long, short)));
	PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION(T, g, t_returner, test, ((glm::vec3))((long double)));
	// PY_EXPORT_TEMPLATE_MEMBER_FUNCTION_AS_STATIC_FUNCTION_LAMBDA(T, g, []() { static T t; return &t; }, test, ((glm::vec3))((long double)));

	PY_EXPORT_TEMPLATE_MEMBER_FUNCTION(T, h, test, ((int))((float)));

	PY_EXPORT_MEMBER_FUNCTION(Zero, Z, test);
	PY_EXPORT_MEMBER_FUNCTION(One, O, test);
	
	PY_EXPORT_TYPE_0FIELD(Zero, test);
	PY_EXPORT_TYPE_1FIELD(One, test, o);
	PY_EXPORT_TYPE(T, test, (i)(j));


	
	
	PY_EXPORT_GLOBAL_FUNCTION(f, test);
	PY_EXPORT_GLOBAL_FUNCTION(h, test);
	PY_EXPORT_GLOBAL_FUNCTION(i, test);
	PY_EXPORT_GLOBAL_FUNCTION(j, test);
	
	PY_EXPORT_MODULE(test);
	
	Py_Initialize();

	const char scriptName[] = "test.py";
	FILE* script = _Py_fopen(scriptName, "rb");
	PyRun_AnyFile(script, scriptName);
	
	Py_Finalize();
	
	return 0;
}
