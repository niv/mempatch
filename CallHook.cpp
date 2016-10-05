#include "memory_patch.h"

#include <assert.h>
#include <stdexcept>

#include <subhook.h>

extern "C" void* subhook_unprotect(void* address, size_t size);

namespace memory_patch {

constexpr auto CallHookVOffset = sizeof(uintptr_t) + 1;

CallHookV::CallHookV(Address addr, const void* func, void* expect) :
	Base(reinterpret_cast<void*>(addr), CallHookVOffset, sizeof(uintptr_t), expect), m_modified(func)
{
	if (nullptr == subhook_unprotect(reinterpret_cast<void*>(addr), CallHookVOffset))
		throw std::runtime_error("subhook_unprotect failed");

	// I'm sure a kitten died somewhere, but at least it died on all
	// x86 systems at once.
	m_original = *(Address*)(addr + 1);
	*(Address*)(addr + 1) = (Address) m_modified - (addr + CallHookVOffset);
}

CallHookV::~CallHookV()
{
	assert(m_original);
	*(Address*)((Address)m_addr + 1) = m_original;
}

}
