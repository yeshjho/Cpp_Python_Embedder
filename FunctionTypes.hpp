/***********************************************************************************
 * Function_Types
 * https://github.com/yeshjho/Function_Types
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
#include <tuple>



namespace function_types
{
template<typename>
struct FunctionTypes;

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_ (Class_::*) (Parameters_...)>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = false;
	static constexpr bool isVolatile = false;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = false;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) const>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = true;
	static constexpr bool isVolatile = false;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = false;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) volatile>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = false;
	static constexpr bool isVolatile = true;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = false;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) const volatile>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = true;
	static constexpr bool isVolatile = true;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = false;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) &>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = false;
	static constexpr bool isVolatile = false;
	static constexpr bool isLRef = true;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = false;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) &&>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = false;
	static constexpr bool isVolatile = false;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = true;
	static constexpr bool isNoExcept = false;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) const &>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = true;
	static constexpr bool isVolatile = false;
	static constexpr bool isLRef = true;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = false;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) const &&>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = true;
	static constexpr bool isVolatile = false;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = true;
	static constexpr bool isNoExcept = false;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) volatile &>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = false;
	static constexpr bool isVolatile = true;
	static constexpr bool isLRef = true;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = false;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) volatile &&>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = false;
	static constexpr bool isVolatile = true;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = true;
	static constexpr bool isNoExcept = false;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) const volatile &>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = true;
	static constexpr bool isVolatile = true;
	static constexpr bool isLRef = true;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = false;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) const volatile &&>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = true;
	static constexpr bool isVolatile = true;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = true;
	static constexpr bool isNoExcept = false;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) noexcept>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = false;
	static constexpr bool isVolatile = false;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = true;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) const noexcept>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = true;
	static constexpr bool isVolatile = false;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = true;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) volatile noexcept>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = false;
	static constexpr bool isVolatile = true;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = true;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) const volatile noexcept>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = true;
	static constexpr bool isVolatile = true;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = true;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) & noexcept>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = false;
	static constexpr bool isVolatile = false;
	static constexpr bool isLRef = true;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = true;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) && noexcept>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = false;
	static constexpr bool isVolatile = false;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = true;
	static constexpr bool isNoExcept = true;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) const & noexcept>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = true;
	static constexpr bool isVolatile = false;
	static constexpr bool isLRef = true;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = true;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) const && noexcept>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = true;
	static constexpr bool isVolatile = false;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = true;
	static constexpr bool isNoExcept = true;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) volatile & noexcept>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = false;
	static constexpr bool isVolatile = true;
	static constexpr bool isLRef = true;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = true;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) volatile && noexcept>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = false;
	static constexpr bool isVolatile = true;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = true;
	static constexpr bool isNoExcept = true;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) const volatile & noexcept>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = true;
	static constexpr bool isVolatile = true;
	static constexpr bool isLRef = true;
	static constexpr bool isRRef = false;
	static constexpr bool isNoExcept = true;
	static constexpr bool isMemberFunction = true;
};

template<typename ReturnType_, typename Class_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(Class_::*) (Parameters_...) const volatile && noexcept>
{
	using ReturnType = ReturnType_;
	using Class = Class_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isConst = true;
	static constexpr bool isVolatile = true;
	static constexpr bool isLRef = false;
	static constexpr bool isRRef = true;
	static constexpr bool isNoExcept = true;
	static constexpr bool isMemberFunction = true;
};


template<typename ReturnType_, typename ...Parameters_>
struct FunctionTypes<ReturnType_(*) (Parameters_...)>
{
	using ReturnType = ReturnType_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isNoExcept = false;
	static constexpr bool isMemberFunction = false;
};

template<typename ReturnType_, typename ...Parameters_>
struct FunctionTypes<ReturnType_ (*) (Parameters_...) noexcept>
{
	using ReturnType = ReturnType_;
	using Parameters = std::tuple<Parameters_...>;
	static constexpr size_t parameterCount = sizeof...(Parameters_);
	static constexpr bool isNoExcept = true;
	static constexpr bool isMemberFunction = false;
};
}
