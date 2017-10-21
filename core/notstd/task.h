#pragma once

#include "tuple.hpp"
#include <stdlib.h>

namespace notstd {

	class Task
	{
	public:
		virtual void Run() = 0;
		virtual void Release() = 0;
	};

	// Method

	template <class T, class Method, class Params>
	class MethodMessage : public Task
	{
	public:
		MethodMessage(T* pObj, Method meth, const Params& params)
			: m_pObj(pObj)
			, m_Meth(meth)
			, m_Params(params)
		{
		}

		virtual ~MethodMessage()
		{
		}

		virtual void Run() {
			if (m_pObj)
				DispatchToMethod(m_pObj, m_Meth, m_Params);
		}

		virtual void Release()
		{
			this->~MethodMessage();
			::free(this);
		}

	private:
		T *m_pObj;
		Method m_Meth;
		Params m_Params;
	};

	template <class T, class Method>
	inline Task* NewMessageTask(T *pObj, Method method)
	{
		typedef MethodMessage<T, Method, Tuple0> MethodMessageType;
		MethodMessageType *pMessage =
			reinterpret_cast<MethodMessageType*>(::malloc(sizeof(MethodMessageType)));
		return static_cast<Task*>(new (pMessage)MethodMessage<T, Method, Tuple0>(pObj, method, MakeTuple()));
	}

	template <class T, class Method, class A>
	inline Task* NewMessageTask(T *pObj, Method method, const A &a)
	{
		typedef MethodMessage<T, Method, Tuple1<A> > MethodMessageType;
		MethodMessageType *pMessage =
			reinterpret_cast<MethodMessageType*>(
			::malloc(sizeof(MethodMessageType)));

		return static_cast<Task*>(new (pMessage)MethodMessageType(pObj, method, MakeTuple(a)));
	}

	template <class T, class Method, class A, class B>
	inline Task* NewMessageTask(T *pObj, Method method,
		const A &a, const B &b)
	{
		typedef MethodMessage<T, Method, Tuple2<A, B> > MethodMessageType;
		MethodMessageType *pMessage = reinterpret_cast<MethodMessageType*>(
			::malloc(sizeof(MethodMessageType)));
		return static_cast<Task*>(new (pMessage)MethodMessageType(pObj, method, MakeTuple(a, b)));
	}

	template <class T, class Method, class A, class B, class C>
	inline Task* NewMessageTask(T *pObj, Method method,
		const A &a, const B &b, const C &c)
	{
		typedef MethodMessage<T, Method, Tuple3<A, B, C> > MethodMessageType;
		MethodMessageType *pMessage = reinterpret_cast<MethodMessageType*>(
			::malloc(sizeof(MethodMessageType)));
		return static_cast<Task*>(new (pMessage)MethodMessageType(pObj, method,
			MakeTuple(a, b, c)));
	}

	template <class T, class Method, class A, class B, class C, class D>
	inline Task* NewMessageTask(T *pObj, Method method,
		const A &a, const B &b, const C &c,
		const D &d)
	{
		typedef MethodMessage<T, Method, Tuple4<A, B, C, D> > MethodMessageType;
		MethodMessageType *pMessage = reinterpret_cast<MethodMessageType*>(
			::malloc(sizeof(MethodMessageType)));
		return static_cast<Task*>(new (pMessage)MethodMessageType(pObj, method,
			MakeTuple(a, b, c, d)));
	}

	template <class T, class Method, class A, class B, class C, class D, class E>
	inline Task* NewMessageTask(T *pObj, Method method,
		const A &a, const B &b, const C &c,
		const D &d, const E &e)
	{
		typedef MethodMessage<T, Method, Tuple5<A, B, C, D, E> > MethodMessageType;
		MethodMessageType *pMessage = reinterpret_cast<MethodMessageType*>(
			::malloc(sizeof(MethodMessageType)));
		return static_cast<Task*>(new (pMessage)MethodMessageType(pObj, method,
			MakeTuple(a, b, c, d, e)));
	}

	template <class T, class Method, class A, class B, class C, class D, class E, class F>
	inline Task* NewMessageTask(T *pObj, Method method,
		const A &a, const B &b, const C &c,
		const D &d, const E &e, const F &f)
	{
		typedef MethodMessage<T, Method, Tuple6<A, B, C, D, E, F> > MethodMessageType;
		MethodMessageType *pMessage = reinterpret_cast<MethodMessageType*>(
			::malloc(sizeof(MethodMessageType)));
		return static_cast<Task*>(new (pMessage)MethodMessageType(pObj, method,
			MakeTuple(a, b, c, d, e, f)));
	}

	template <class T, class Method, class A, class B, class C, class D, class E, class F, class G>
	inline Task* NewMessageTask(T *pObj, Method method,
		const A &a, const B &b, const C &c,
		const D &d, const E &e, const F &f,
		const G &g)
	{
		typedef MethodMessage<T, Method, Tuple7<A, B, C, D, E, F, G> > MethodMessageType;
		MethodMessageType *pMessage = reinterpret_cast<MethodMessageType*>(
			::malloc(sizeof(MethodMessageType)));
		return static_cast<Task*>(new (pMessage)MethodMessageType(pObj, method,
			MakeTuple(a, b, c, d, e, f, g)));
	}

	template <class T, class Method, class A, class B, class C, class D, class E, class F, class G, class H>
	inline Task* NewMessageTask(T *pObj, Method method,
		const A &a, const B &b, const C &c,
		const D &d, const E &e, const F &f,
		const G &g, const H &h)
	{
		typedef MethodMessage<T, Method, Tuple8<A, B, C, D, E, F, G, H> > MethodMessageType;
		MethodMessageType *pMessage = reinterpret_cast<MethodMessageType*>(
			::malloc(sizeof(MethodMessageType)));
		return static_cast<Task*>(new (pMessage)MethodMessageType(pObj, method,
			MakeTuple(a, b, c, d, e, f, g, h)));
	}

	// Function

	template <class Function, class Params>
	class FunctionMessage : public Task
	{
	public:
		FunctionMessage(Function function, const Params &params)
			: m_Function(function)
			, m_Params(params)
		{
		}

		virtual ~FunctionMessage()
		{
		}

		virtual void Release()
		{
			this->~FunctionMessage();
			::free(this);
		}

		virtual void Run()
		{
			if (m_Function)
				DispatchToFunction(m_Function, m_Params);
		}

	private:
		Function m_Function;
		Params m_Params;
	};

	template <class Function>
	inline Task* NewFunctionTask(Function function) {
		typedef FunctionMessage<Function, Tuple0> FunctionMessageType;
		FunctionMessageType *pMessage =
			reinterpret_cast<FunctionMessageType*>(
			::malloc(sizeof(FunctionMessageType)));
		return static_cast<Task*>(new (pMessage)FunctionMessageType(function, MakeTuple()));
	}

	template <class Function, class A>
	inline Task* NewFunctionTask(Function function, const A &a)
	{
		typedef FunctionMessage<Function, Tuple1<A> > FunctionMessageType;
		FunctionMessageType *pMessage =
			reinterpret_cast<FunctionMessageType*>(
			::malloc(sizeof(FunctionMessageType)));
		return static_cast<Task*>(new (pMessage)FunctionMessageType(function, MakeTuple(a)));
	}

	template <class Function, class A, class B>
	inline Task* NewFunctionTask(Function function,
		const A &a, const B &b)
	{
		typedef FunctionMessage<Function, Tuple2<A, B> > FunctionMessageType;
		FunctionMessageType *pMessage =
			reinterpret_cast<FunctionMessageType*>(
			::malloc(sizeof(FunctionMessageType)));
		return static_cast<Task*>(new (pMessage)FunctionMessageType(function,
			MakeTuple(a, b)));
	}

	template <class Function, class A, class B, class C>
	inline Task* NewFunctionTask(Function function,
		const A &a, const B &b, const C &c)
	{
		typedef FunctionMessage<Function, Tuple3<A, B, C> > FunctionMessageType;
		FunctionMessageType *pMessage =
			reinterpret_cast<FunctionMessageType*>(
			::malloc(sizeof(FunctionMessageType)));
		return static_cast<Task*>(new (pMessage)FunctionMessageType(function,
			MakeTuple(a, b, c)));
	}

	template <class Function, class A, class B, class C, class D>
	inline Task* NewFunctionTask(Function function,
		const A &a, const B &b, const C &c,
		const D &d)
	{
		typedef FunctionMessage<Function, Tuple4<A, B, C, D> > FunctionMessageType;
		FunctionMessageType *pMessage =
			reinterpret_cast<FunctionMessageType*>(
			::malloc(sizeof(FunctionMessageType)));
		return static_cast<Task*>(new (pMessage)FunctionMessageType(function,
			MakeTuple(a, b, c, d)));
	}

	template <class Function, class A, class B, class C, class D, class E>
	inline Task* NewFunctionTask(Function function,
		const A &a, const B &b, const C &c,
		const D &d, const E &e)
	{
		typedef FunctionMessage<Function, Tuple5<A, B, C, D, E> > FunctionMessageType;
		FunctionMessageType *pMessage =
			reinterpret_cast<FunctionMessageType*>(
			::malloc(sizeof(FunctionMessageType)));
		return static_cast<Task*>(new (pMessage)FunctionMessageType(function,
			MakeTuple(a, b, c, d, e)));
	}

	template <class Function, class A, class B, class C, class D, class E, class F>
	inline Task* NewFunctionTask(Function function,
		const A &a, const B &b, const C &c,
		const D &d, const E &e, const F &f)
	{
		typedef FunctionMessage<Function, Tuple6<A, B, C, D, E, F> > FunctionMessageType;
		FunctionMessageType *pMessage =
			reinterpret_cast<FunctionMessageType*>(
			::malloc(sizeof(FunctionMessageType)));
		return static_cast<Task*>(new (pMessage)FunctionMessageType(function,
			MakeTuple(a, b, c, d, e, f)));
	}

	template <class Function, class A, class B, class C, class D, class E, class F, class G>
	inline Task* NewFunctionTask(Function function,
		const A &a, const B &b, const C &c,
		const D &d, const E &e, const F &f,
		const G &g)
	{
		typedef FunctionMessage<Function, Tuple7<A, B, C, D, E, F, G> > FunctionMessageType;
		FunctionMessageType *pMessage =
			reinterpret_cast<FunctionMessageType*>(
			::malloc(sizeof(FunctionMessageType)));
		return static_cast<Task*>(new (pMessage)FunctionMessageType(function,
			MakeTuple(a, b, c, d, e, f, g)));
	}

	template <class Function, class A, class B, class C, class D, class E, class F, class G, class H>
	inline Task* NewFunctionTask(Function function,
		const A &a, const B &b, const C &c,
		const D &d, const E &e, const F &f,
		const G &g, const H &h)
	{
		typedef FunctionMessage<Function, Tuple8<A, B, C, D, E, F, G, H> > FunctionMessageType;
		FunctionMessageType *pMessage =
			reinterpret_cast<FunctionMessageType*>(
			::malloc(sizeof(FunctionMessageType)));
		return static_cast<Task*>(new (pMessage)FunctionMessageType(function,
			MakeTuple(a, b, c, d, e, f, g, h)));
	}

	///////////////////////////////////////////////////////////////////////////////
}
