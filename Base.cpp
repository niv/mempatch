#include "memory_patch.h"

#include <unordered_map>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <assert.h>
#include <string.h>
#include <execinfo.h>

namespace memory_patch {

struct PatchData {
	size_t m_len;
	std::string m_backtrace;
};

// pointer value -> patchdata (for overlap checking)
static std::unordered_map<uintptr_t, PatchData> s_patched;

Base::Base(void* addr, size_t len, size_t expect_len, const void* expect) :
	m_addr(addr), m_len(len)
{
	assert(addr);
	assert(len);

	// Make sure the memory at the given location matches what we expect.
	if (expect_len > 0 && expect != nullptr && memcmp(addr, expect, expect_len)) {
		std::stringstream serr;
		serr << "memory location at 0x" <<
		     std::setfill('0') << std::setw(8) << std::hex << addr <<
		     " did not have expected value (incompatible binary or something else patched it)";

		throw std::runtime_error(serr.str());
	}

	// Make sure the given addr and len do not overlap with any other
	// existing patch.
	for (auto& pair : s_patched) {
		if (std::max((uintptr_t)addr, pair.first) <=
		        std::min((uintptr_t)addr + len, pair.first + pair.second.m_len)) {
			std::stringstream serr;
			serr << "memory location at 0x" <<
			     std::setfill('0') << std::setw(8) << std::hex << addr <<
			     " already patched by:\n" << pair.second.m_backtrace;

			throw std::runtime_error(serr.str());
		}
	}

	// Also store a backtrace of whoever has hooked us.
	std::string sbt;

	void* callstack[32];
	int i = 0,
	    frames = backtrace(callstack, 32);

	if (frames > 0) {
		char** bt = backtrace_symbols(callstack, frames);
		assert(bt);
		for (i = 0; i < frames; ++i) {
			sbt += std::string(bt[i]) + "\n";
		}
		free(bt);
	} else
		sbt = "(no backtrace available)";

	PatchData pd = { len, sbt };
	// printf("patchadd addr=%x len=%d, sbt=%s\n", addr, len, sbt.c_str());
	// printf("spatched size: %d\n", s_patched.size());

	// gdb bug that throws a FPE. See #61143
	s_patched.reserve(1);
	s_patched[(uintptr_t)addr] = pd;
}

Base::~Base()
{
	s_patched.erase((uintptr_t)m_addr);
}

}
