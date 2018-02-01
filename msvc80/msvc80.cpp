//
// Copyright (c) 2018 Nico Bendlin <nico@nicode.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#if defined(_WIN32) && !defined(_WIN64)
#if defined(_MSC_VER) && (_MSC_VER >= 1300) && !defined(__INTEL_COMPILER)
#if !defined(_DLL)

// The Microsoft-specific C++ keyword __identifier is used to replace the
// functions that are imported by the static Microsoft C runtime library.

#pragma warning(push, 1)
#include <windows.h>
#pragma warning(pop)
#pragma warning(disable: 4483)  // expected C++ keyword
#pragma warning(disable: 4514)  // unreferenced inline function has been removed

#define PIMP_STR(x) #x
#define PIMP_IMPORT(fname, stack) __identifier(PIMP_STR(_imp__##fname##@##stack))

#define PIMP_WRAP(mname, fname, stack, rtype, ...)\
static rtype WINAPI Probe_##fname(__VA_ARGS__);\
typedef rtype (WINAPI *PFN##fname)(__VA_ARGS__);\
extern "C" PFN##fname PIMP_IMPORT(fname, stack) = Probe_##fname;\
static rtype WINAPI Emulate_##fname(__VA_ARGS__);\
rtype WINAPI Probe_##fname(__VA_ARGS__)\
{\
	if (PIMP_IMPORT(fname, stack) == Probe_##fname) {\
		PFN##fname Real_##fname = (PFN##fname)(ULONG_PTR)GetProcAddress(\
			GetModuleHandleA( #mname ), #fname);\
		PIMP_IMPORT(fname, stack) = (Real_##fname != NULL)\
			? Real_##fname\
			: Emulate_##fname;\
	}\
	return (PIMP_IMPORT(fname, stack)(PIMP_ARGS));\
}\
rtype WINAPI Emulate_##fname(__VA_ARGS__)

//
// Last version that supports Windows 95 - but imports IsDebuggerPresent.
//
#if (_MSC_VER >= 1400)  // Visual C++ 2005

extern "C" WINBASEAPI DWORD WINAPI GetProcessFlags(DWORD);

#define PIMP_ARGS                                    dwProcessId
PIMP_WRAP(KERNEL32, GetProcessFlags, 4, DWORD, DWORD dwProcessId)
{
	UNREFERENCED_PARAMETER(dwProcessId);
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return (0);
}
#undef PIMP_ARGS

#define PIMP_ARGS
PIMP_WRAP(KERNEL32, IsDebuggerPresent, 0, BOOL, VOID)
{
	return ((BOOL)(GetProcessFlags(GetCurrentProcessId()) & 1));  // PDB32_DEBUGGED
}
#undef PIMP_ARGS

#endif

#endif
#endif
#endif
