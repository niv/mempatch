#include "memory_patch.h"

#include <string.h>
#include <assert.h>
#include <stdexcept>

#include <subhook.h>

extern "C" void* subhook_unprotect(void* address, size_t size);

namespace memory_patch {

Simple::Simple(void* addr, size_t len, const void* data,
               size_t expect_len, const void* expect) :
	Base(addr, len, expect_len, expect), m_modified(data)
{
	if (nullptr == subhook_unprotect(addr, len))
		throw std::runtime_error("subhook_unprotect failed");

	m_original = malloc(m_len);
	memcpy(m_original, m_addr, m_len);
	memcpy(m_addr, m_modified, m_len);
}

Simple::~Simple()
{
	assert(m_original);
	memcpy(m_addr, m_original, m_len);
	free(m_original);
}

}