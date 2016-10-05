#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string>

// A collection of memory patching helpers that will do adjustments
// to memory locations for the scope lifetime of the helper object.
namespace memory_patch {

using Address = uintptr_t;


// A base class that will ensure no continuous range of bytes in memory
// is ever touched by two patchers.
// Will throw a std::runtime_error on conflict.
class Base
{
public:
	Base(void* addr, size_t len,
	     size_t expect_len = 0, const void* expect = nullptr);
	virtual ~Base();

protected:
	void* m_addr;
	size_t m_len;

private:
	Base& operator=(const Base&) = delete;
	Base(const Base&) = delete;
};

// This is a simple memory patcher that adjusts a continuous range of bytes in
// memory to a fixed value.
class Simple : public Base
{
public:
	Simple(void* addr, size_t len, const void* data,
	       size_t expect_len = 0, const void* expect = nullptr);
	virtual ~Simple();

protected:
	const void* m_modified;
	void* m_original;
};



// A inline-call style patcher. This is the pointer version; you probably
// want to use the templated one instead for type safety and comfort reasons.
class CallHookV : public Base
{
public:
	CallHookV(Address addr, const void* func, void* expect = 0);
	~CallHookV();

protected:
	Address m_original; // byte data of original call. not a valid callable addr.
	const void* m_modified;
};

template <class>
class CallHook;

// Patches a asm call instruction with a redirect.
template <typename Ret, typename ... Args>
class CallHook<Ret(Args...)> : public CallHookV
{
public:
	typedef Ret(*Signature)(Args...);

	CallHook(Address addr, const Signature func, void* expect = 0) :
		CallHookV(addr, reinterpret_cast<const void*>(func), expect) {}
};



// A trampoline-based call redirector that can take whole functions.
// You probably want the templated "TrampolineHook" instead for type safety and comfort.
class TrampolineHookV : public Base
{
public:
	TrampolineHookV(void* addr, const void* func,
	                size_t expect_len = 0, const void* expect = nullptr);
	~TrampolineHookV();

	// Returns the trampoline that was generated. This is what you need to
	// call to talk to the original function.
	void* GetTrampoline() const;

protected:
	void* m_hook; // opaque to you
};

template <class>
class TrampolineHook;

// A complex patcher that generates a function trampoline at the destination
// and redirects calls to your desired location. Will figure out required
// instruction length by disassembling.
template <typename Ret, typename ... Args>
class TrampolineHook<Ret(Args...)> : public TrampolineHookV
{
public:
	typedef Ret(*Signature)(Args...);

	TrampolineHook(Signature addr, const Signature func,
	               size_t expect_len = 0, const void* expect = nullptr) :
		TrampolineHookV(reinterpret_cast<void*>(addr),
		                reinterpret_cast<const void*>(func),
		                expect_len, expect) {}

	// Alternative ctor syntax that allows memory patching by integer address.
	TrampolineHook(Address addr, const Signature func,
	               size_t expect_len = 0, const void* expect = nullptr):
		TrampolineHookV(reinterpret_cast<void*>(addr),
		                reinterpret_cast<const void*>(func),
		                expect_len, expect) {}

	// Returns the original function.
	Signature GetOriginal() const
	{
		return reinterpret_cast<Signature>(GetTrampoline());
	}

	Ret CallOriginal(Args ... args)
	{
		return GetOriginal()(args...);
	}
};

}
