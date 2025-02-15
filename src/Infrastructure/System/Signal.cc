#include "Signal.h"
#include <iostream>
#include <vector>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <dbghelp.h>
// clang-format on
#pragma comment(lib, "dbghelp.lib")
#else
#include <cxxabi.h>
#include <execinfo.h>

#endif

SignalHandler& SignalHandler::getInstance() {
    static SignalHandler instance;
    return instance;
}

void SignalHandler::registerHandler(int signal, Callback callback) {
    handlers[signal] = std::move(callback);
    std::signal(signal, SignalHandler::handleSignal);
}

std::string SignalHandler::getStackTrace() {
    std::string result;
#ifdef _WIN32
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    CONTEXT context;
    memset(&context, 0, sizeof(CONTEXT));
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);

    SymInitialize(process, NULL, TRUE);

    STACKFRAME64 stack;
    memset(&stack, 0, sizeof(STACKFRAME64));
    stack.AddrPC.Offset = context.Rip;
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrFrame.Offset = context.Rbp;
    stack.AddrFrame.Mode = AddrModeFlat;
    stack.AddrStack.Offset = context.Rsp;
    stack.AddrStack.Mode = AddrModeFlat;

    for (ULONG frame = 0;; frame++) {
        BOOL more = StackWalk64(IMAGE_FILE_MACHINE_AMD64,
                                process,
                                thread,
                                &stack,
                                &context,
                                NULL,
                                SymFunctionTableAccess64,
                                SymGetModuleBase64,
                                NULL);

        if (!more || stack.AddrPC.Offset == 0) {
            break;
        }

        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO) buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        if (SymFromAddr(process, stack.AddrPC.Offset, NULL, symbol)) {
            result += std::to_string(frame) + ": " + symbol->Name + "\n";
        } else {
            result += std::to_string(frame) + ": Unknown\n";
        }
    }

    SymCleanup(process);
#else
    void* array[50];
    int size = backtrace(array, 50);
    char** messages = backtrace_symbols(array, size);

    for (int i = 1; i < size && messages != NULL; ++i) {
        char *mangled_name = 0, *offset_begin = 0, *offset_end = 0;

        for (char* p = messages[i]; *p; ++p) {
            if (*p == '(') {
                mangled_name = p;
            } else if (*p == '+') {
                offset_begin = p;
            } else if (*p == ')') {
                offset_end = p;
                break;
            }
        }

        if (mangled_name && offset_begin && offset_end && mangled_name < offset_begin) {
            *mangled_name++ = '\0';
            *offset_begin++ = '\0';
            *offset_end++ = '\0';

            int status;
            char* demangled_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);
            if (status == 0) {
                result += std::string(messages[i]) + ": " + demangled_name + "+" + offset_begin
                          + offset_end + "\n";
                free(demangled_name);
            } else {
                result += std::string(messages[i]) + ": " + mangled_name + "+" + offset_begin
                          + offset_end + "\n";
            }
        } else {
            result += std::string(messages[i]) + "\n";
        }
    }
    free(messages);
#endif
    return result;
}

SignalHandler::SignalHandler() = default;

void SignalHandler::handleSignal(int signal) {
    auto& instance = getInstance();
    if (auto it = instance.handlers.find(signal); it != instance.handlers.end()) {
        it->second(signal);
    }
    std::cout << "Stack trace:\n" << getStackTrace() << std::endl;
    std::exit(signal);
}