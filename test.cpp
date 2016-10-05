#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <memory_patch.h>
#include <stdint.h>

TEST_CASE("Simple works", "[Simple]")
{
	int32_t mem = 0;

	{
		memory_patch::Simple patcher(&mem, 4, "\x01\x00\x00\x00");
		REQUIRE(mem == 1);
	}

	REQUIRE(mem == 0);
}

TEST_CASE("Base does not allow overlapping patches", "[Base]")
{
	int32_t mem = 0;

	memory_patch::Simple patcher(&mem, 4, "\x01\x00\x00\x00");
	REQUIRE_THROWS(
	    memory_patch::Simple patcher(&mem, 4, "\x01\x00\x00\x00");
	);
}

int original() { return 0; }
int hooked() { return 1; }
int outer() { return original(); }

TEST_CASE("TrampolineHook works", "[TrampolineHook]")
{
	REQUIRE(original() == 0);
	REQUIRE(hooked() == 1);

	{
		memory_patch::TrampolineHook<int()> patcher(original, hooked);

		REQUIRE(original() == 1);
		REQUIRE(hooked() == 1);
		REQUIRE(patcher.CallOriginal() == 0);
	}

	REQUIRE(original() == 0);
}

TEST_CASE("CallHook works", "[CallHook]") {

	REQUIRE(outer() == 0);
	{
		// ? probably need to start disassembling to make this test case not
		// suck on other systems or compilers doing stuff differently
		uintptr_t addr = (uintptr_t)outer + 3;
		memory_patch::CallHook<int()> patcher(addr, hooked);
		REQUIRE(outer() == 1);
	}

	REQUIRE(outer() == 0);
}
