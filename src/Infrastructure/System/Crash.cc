#include "Crash.h"
#include "Cpu.h"
#include "Memory.h"
#include "Os.h"
#include "Wm.h"
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <unordered_map>


#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <dbghelp.h>
// clang-format on
#endif

#include <cpptrace/cpptrace.hpp>

auto getSystemInfoStr() -> std::string {
    std::stringstream sss;

    auto osInfo = getOperatingSystemInfo();
    sss << "System Information:\n";
    sss << "-------------------\n";
    sss << "Operating system: " << osInfo.osName << " " << osInfo.osVersion << "\n";
    sss << "Architecture: " << osInfo.architecture << "\n";
    sss << "Kernel version: " << osInfo.kernelVersion << "\n";
    sss << "Computer name: " << osInfo.computerName << "\n";
    sss << "Compiler: " << osInfo.compiler << "\n";
    sss << "WSL: " << (isWsl() ? "Yes" : "No") << "\n\n";

    sss << "CPU Information:\n";
    sss << "----------------\n";
    sss << "Usage: " << getCurrentCpuUsage() << "%\n";
    sss << "Temperature: " << getCurrentCpuTemperature() << " °C\n";
    sss << "Model: " << getCPUModel() << "\n";
    sss << "Identifier: " << getProcessorIdentifier() << "\n";
    sss << "Packages: " << getNumberOfPhysicalPackages() << "\n\n";
    sss << "Frequency: " << getProcessorFrequency() << " GHz\n";
    sss << "Cores: " << getNumberOfPhysicalCPUs() << "\n";
    auto cache = getCacheSizes();
    sss << "Cache sizes:\n";
    sss << "  L1D: " << cache.l1d << " KB\n";
    sss << "  L1I: " << cache.l1i << " KB\n";
    sss << "  L2: " << cache.l2 << " KB\n";
    sss << "  L3: " << cache.l3 << " KB\n\n";

    sss << "Memory Status:\n";
    sss << "--------------\n";
    sss << "Usage: " << getMemoryUsage() << "%\n";
    sss << "Total: " << getTotalMemorySize() << " MB\n";
    sss << "Free: " << getAvailableMemorySize() << " MB\n";
    sss << "Virtual memory max: " << getVirtualMemoryMax() << " MB\n";
    sss << "Virtual memory used: " << getVirtualMemoryUsed() << " MB\n";
    sss << "Swap memory total: " << getSwapMemoryTotal() << " MB\n";
    sss << "Swap memory used: " << getSwapMemoryUsed() << " MB\n";
    sss << "Committed memory: " << getCommittedMemory() << " MB\n";

#ifdef _WIN32
    sss << "Window Manager Information:\n";
    sss << "---------------------------\n";
    auto wmInfo = getSystemInfo();
    sss << "Desktop Environment: " << wmInfo.desktopEnvironment << "\n";
    sss << "Window Manager: " << wmInfo.windowManager << "\n";
    sss << "WM Theme: " << wmInfo.wmTheme << "\n";
    sss << "Icons: " << wmInfo.icons << "\n";
    sss << "Font: " << wmInfo.font << "\n";
    sss << "Cursor: " << wmInfo.cursor << "\n";
#endif
    return sss.str();
}

auto getChinaTimestampString() -> std::string {
    auto now = std::chrono::system_clock::now();
    std::time_t nowC = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;
    if (localtime_s(&localTime, &nowC) != 0) {
    }
    std::stringstream sss;
    sss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return sss.str();
}

auto Environ() -> std::unordered_map<std::string, std::string> {
    std::unordered_map<std::string, std::string> env;
    for (char** envp = environ; *envp != nullptr; ++envp) {
        std::string envStr(*envp);
        size_t pos = envStr.find('=');
        if (pos != std::string::npos) {
            env[envStr.substr(0, pos)] = envStr.substr(pos + 1);
        }
    }
    return env;
}

void saveCrashLog(std::string_view error_msg) {
    std::string systemInfo = getSystemInfoStr();
    std::string environmentInfo;
    for (const auto& [key, value] : Environ()) {
        environmentInfo += key + ": " + value + "\n";
    }

    std::stringstream sss;
    sss << "Program crashed at: " << getChinaTimestampString() << "\n";
    sss << "Error message: " << error_msg << "\n\n";

    sss << "==================== Stack Trace ====================\n";
    // TODO: Boost stacktrace could not be included in the project
    // sss << boost::stacktrace::stacktrace() << "\n\n";
    sss << cpptrace::generate_trace() << "\n\n";
    cpptrace::generate_trace().print();

    sss << "==================== System Information ====================\n";
    sss << systemInfo << "\n";

    sss << "================= Environment Variables ===================\n";
    if (environmentInfo.empty()) {
        sss << "Failed to get environment information.\n";
    } else {
        sss << environmentInfo << "\n";
    }

    std::stringstream ssss;
    auto now = std::chrono::system_clock::now();
    std::time_t nowC = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;
    if (localtime_s(&localTime, &nowC) != 0) {
        // Handle error
        return;
    }
    ssss << "crash_report/crash_" << std::put_time(&localTime, "%Y%m%d_%H%M%S") << ".log";
    std::filesystem::path dirPath("crash_report");
    if (!std::filesystem::exists(dirPath)) {
        std::filesystem::create_directory(dirPath);
    }
    std::ofstream ofs(ssss.str());
    if (ofs.good()) {
        ofs << sss.str();
        ofs.close();
    }

    // Create a dump file
#ifdef _WIN32
    std::stringstream wss;
    wss << "crash_report/crash_" << std::put_time(&localTime, "%Y%m%d_%H%M%S") << ".dmp";
    std::string dumpFile = wss.str();
    HANDLE hFile = CreateFile(dumpFile.c_str(),
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              nullptr,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }
    MINIDUMP_EXCEPTION_INFORMATION mdei;
    mdei.ThreadId = GetCurrentThreadId();
    EXCEPTION_POINTERS* pep = nullptr;
    mdei.ExceptionPointers = pep;
    mdei.ClientPointers = FALSE;
    MiniDumpWriteDump(GetCurrentProcess(),
                      GetCurrentProcessId(),
                      hFile,
                      MiniDumpNormal,
                      (pep != nullptr) ? &mdei : nullptr,
                      nullptr,
                      nullptr);
    CloseHandle(hFile);
#endif
}
