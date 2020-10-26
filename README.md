# Cpp_Python_Embedder

(temp)

 * Limitations:
 *	1. Parameter Types and Return Types should be one of:
 *		- bool
 *		- char <It assumes python will call it with a single character, not a number>
 *		- unsigned char
 *		- short
 *		- unsigned short
 *		- int
 *		- unsigned int
 *		- long
 *		- unsigned long
 *		- long long
 *		- unsigned long long
 *		- float
 *		- double
 *		- const char*
 *		- std::string
 *		- Custom types Registered by the user
 *		
 *	2. All Fields of a Custom Type should be:
 *		- Registered
 *		- Publicly Accessible
 *		- Of a Type One of the Types listed above, EXCEPT:
 *			- std::string
 *			- Custom Types
 *		<Note that a member of char is treated as a number rather than a character at this context>
 *
 *	3. All Methods of a Custom Type should be Registered before its Type is Registered
 *	
 *	4. Instance returning function of PY_EXPORT_MEMBER_FUNCTION_AS_STATIC_FUNCTION can't be a lambda (WIP)
