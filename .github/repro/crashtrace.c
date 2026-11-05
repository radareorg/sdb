/* diagnostic only: appended to src/entry.c by the Windows tool reproduction */
#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#pragma comment(lib, "dbghelp.lib")

static LONG CALLBACK sdb_crashtrace(EXCEPTION_POINTERS *ep) {
	static volatile LONG busy = 0;
	if (ep->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION || InterlockedExchange (&busy, 1)) {
		return EXCEPTION_CONTINUE_SEARCH;
	}
	HANDLE proc = GetCurrentProcess ();
	EXCEPTION_RECORD *er = ep->ExceptionRecord;
	fprintf (stderr, "CRASHTRACE pid=%lu tid=%lu code=0x%08lx addr=%p %s %p\n",
		GetCurrentProcessId (), GetCurrentThreadId (), er->ExceptionCode, er->ExceptionAddress,
		er->NumberParameters > 1? (er->ExceptionInformation[0]? "write": "read"): "?",
		er->NumberParameters > 1? (void *)er->ExceptionInformation[1]: NULL);
	SymSetOptions (SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME);
	SymInitialize (proc, NULL, TRUE);
	CONTEXT ctx = *ep->ContextRecord;
	STACKFRAME64 sf = {0};
	sf.AddrPC.Offset = ctx.Rip; sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrFrame.Offset = ctx.Rbp; sf.AddrFrame.Mode = AddrModeFlat;
	sf.AddrStack.Offset = ctx.Rsp; sf.AddrStack.Mode = AddrModeFlat;
	int i;
	for (i = 0; i < 48 && StackWalk64 (IMAGE_FILE_MACHINE_AMD64, proc, GetCurrentThread (), &sf, &ctx,
			NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL) && sf.AddrPC.Offset; i++) {
		char buf[sizeof (SYMBOL_INFO) + 256] = {0};
		SYMBOL_INFO *sym = (SYMBOL_INFO *)buf;
		sym->SizeOfStruct = sizeof (SYMBOL_INFO);
		sym->MaxNameLen = 255;
		DWORD64 off = 0;
		DWORD loff = 0;
		IMAGEHLP_LINE64 line = { sizeof (IMAGEHLP_LINE64) };
		IMAGEHLP_MODULE64 mod = { sizeof (IMAGEHLP_MODULE64) };
		const DWORD64 pc = sf.AddrPC.Offset;
		const char *mname = SymGetModuleInfo64 (proc, pc, &mod)? mod.ModuleName: "?";
		const char *sname = SymFromAddr (proc, pc, &off, sym)? sym->Name: "?";
		if (SymGetLineFromAddr64 (proc, pc, &loff, &line)) {
			fprintf (stderr, "CRASHTRACE #%02d %s!%s+0x%llx %s:%lu\n", i, mname, sname, off, line.FileName, line.LineNumber);
		} else {
			fprintf (stderr, "CRASHTRACE #%02d %s!%s+0x%llx (%p)\n", i, mname, sname, off, (void *)pc);
		}
	}
	char path[MAX_PATH];
	snprintf (path, sizeof (path), "D:\\dumps\\sdb-%lu.dmp", GetCurrentProcessId ());
	HANDLE f = CreateFileA (path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f != INVALID_HANDLE_VALUE) {
		MINIDUMP_EXCEPTION_INFORMATION mei = { GetCurrentThreadId (), ep, FALSE };
		MiniDumpWriteDump (proc, GetCurrentProcessId (), f,
			(MINIDUMP_TYPE)(MiniDumpWithFullMemory | MiniDumpWithHandleData | MiniDumpWithThreadInfo), &mei, NULL, NULL);
		CloseHandle (f);
		fprintf (stderr, "CRASHTRACE dump %s\n", path);
	}
	fflush (stderr);
	return EXCEPTION_CONTINUE_SEARCH;
}

static void __cdecl sdb_crashtrace_exit(void) {
	fprintf (stderr, "CRASHTRACE sdb pid=%lu exiting normally\n", GetCurrentProcessId ());
	fflush (stderr);
}

static void __cdecl sdb_crashtrace_init(void) {
	AddVectoredExceptionHandler (1, sdb_crashtrace);
	atexit (sdb_crashtrace_exit);
}
#pragma section(".CRT$XCU", read)
__declspec(allocate(".CRT$XCU")) static void (__cdecl *sdb_crashtrace_ptr)(void) = sdb_crashtrace_init;
