# memory_patch

This is a template-using C++ library that allows you to:

* Safely apply arbitary edits to memory locations
* Make sure no memory location is patched twice, by different sources
* Ensure the memory you are patching validates (has expected data)
* Redirect complete functions, including trampoline generation
* Redirect single assembly call instructions inside other functions
* Undo all of this again with nifty RAII-style C++ idioms
* Should work on Linux and Windows (but I have only tested Linux)
* Should work on both 32bit and 64bit
* Can be linked dynamically or statically (recommended)

Requirements:

* C++11

## Components

All listed components work in accord with RAII. You create an instance of a
patcher and it will perform whatever it is configured to immediately; once
you let it go out of scope, the patch will be undone.

It is up to you to make sure the addresses are correct and no other threads
walk over your work.

All patchers use a static map to track patches you have applied and make sure
that no two instances clobber each other. Think: loadable user-written modules
or scripts that can request memory edits to their host process.

Additionally, all patchers allow you to (optionally) define a expectation of what
memory should look like BEFORE the patch is applied; for example to make sure
you are patching the right location or the right binary version. Something like
that.

You probably want to have a look over the list below and then have a look at
memory_patch.h to get started. It's really quite simple in the end.

### Simple

A memory patcher that just edits continuous bytes in memory.

Example:

```cpp
int32_t hey = 1;
{
	memory_patch::Simple patcher(&hey, "\xff\x00\x00\x00");
	assert(hey == 0xff);
}
// patcher goes out of scope and gets unapplied.
assert(hey == 0x01);
```

### Hook (HookV)

A trampoline-generating hooking mechanism, that will clobber the function
prologue with a redirect to your own call. The generated trampoline can be
used to call into the original function in it's stead.

The `Hook` variant uses C++ templates to make it look all nice and proper;
`HookV` is the void* variant if you cannot be bothered.

On Linux, Hook automatically figures out the patch length so it does not cut
into the middle of a instruction.

Example:

```cpp
int a() { return 1; }
int b() { return 2; }

{
	memory_patch::Hook<int()> patcher(a, b);
	assert(a() == 2);
}

assert(a() == 1);
```

### CallHook (CallHookV)

A hook type that can be used to override a call/jmp destination inside the middle
of a function, without redirecting the complete target function.

This one is slightly more complicated to use and will only be of use to you if you are
staring at disassembly. You will have to figure out the exact offset of the actual
call instruction you want to patch, and then plug that in.

```asm
...
// in aFunc() somewhere
0x08011007 push ebx
0x08011008 push edx
0x08012009 call SomeOtherFunc
...
```

```cpp
void mySomeOtherFunc(int whatever ..)
{
	// Do something else, but only when called from inside aFunc()
}

memory_patch::CallHook<void(int)> patcher(0x08012009, mySomeOtherFunc);
aFunc();
```

## Acknowledgements

This library uses subhook under the hood for the trampoline hooks, which is
licenced under the BSD 2-clause.

Find it here: https://github.com/Zeex/subhook

## Help

If you have questions, please just open a issue (or email me)!
