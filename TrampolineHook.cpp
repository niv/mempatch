#include "memory_patch.h"

#include <assert.h>
#include <stdexcept>
#include <string.h>
#include <sys/mman.h>

#include <subhook.h>

namespace memory_patch {

TrampolineHookV::TrampolineHookV(void* addr, const void* func,
                                 size_t e_l, const void* e) :
	Base(addr, 18 /* wild guess on trampoline size */, e_l, e),

	// NB: Subhook doesn't actually touch the target func so we can discard
	//     the const.
	m_hook(reinterpret_cast<subhook_t>(subhook_new(addr, const_cast<void*>(func), {})))
{
	if (subhook_install(reinterpret_cast<subhook_t>(m_hook)) < 0)
		throw std::runtime_error("HookV failed to install");
}

void* TrampolineHookV::GetTrampoline() const
{
	return subhook_get_trampoline(reinterpret_cast<subhook_t>(m_hook));
}

TrampolineHookV::~TrampolineHookV()
{
	subhook_remove(reinterpret_cast<subhook_t>(m_hook));
	subhook_free(reinterpret_cast<subhook_t>(m_hook));
}

}
