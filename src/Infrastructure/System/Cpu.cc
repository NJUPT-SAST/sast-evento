#include "Cpu.h"
#include "Os.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <psapi.h>
#include <intrin.h>
#include <iphlpapi.h>
#include <pdh.h>
#include <tlhelp32.h>
#include <wincon.h>
#include <wbemidl.h>
#include <comdef.h>
// clang-format on
#elif __linux__
#include <csignal>
#include <dirent.h>
#include <iterator>
#include <limits.h>
#include <sys/statfs.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>

#elif __APPLE__
#include <mach/mach_init.h>
#include <mach/task_info.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/mount.h>
#include <sys/param.h>
#endif

#include <spdlog/spdlog.h>

auto getCurrentCpuUsage() -> float {
    float cpuUsage = 0.0;

#ifdef _WIN32
    PDH_HQUERY query;
    PdhOpenQuery(nullptr, 0, &query);

    PDH_HCOUNTER counter;
    PdhAddCounter(query, "\\Processor(_Total)\\% Processor Time", 0, &counter);
    PdhCollectQueryData(query);

    PDH_FMT_COUNTERVALUE counterValue;
    PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr, &counterValue);

    cpuUsage = static_cast<float>(counterValue.doubleValue);

    PdhCloseQuery(query);
#elif __linux__
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        spdlog::error("GetCpuUsage error: open /proc/stat error");
        return cpuUsage;
    }
    std::string line;
    std::getline(file, line);

    std::istringstream iss(line);
    std::vector<std::string> tokens(std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>());

    unsigned long totalTime = 0;
    for (size_t i = 1; i < tokens.size(); i++) {
        totalTime += std::stoul(tokens[i]);
    }

    unsigned long idleTime = std::stoul(tokens[4]);

    float usage = static_cast<float>(totalTime - idleTime) / totalTime;
    cpuUsage = usage * 100.0;
#elif __APPLE__
    host_cpu_load_info_data_t cpu_load;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    if (host_statistics64(mach_host_self(),
                          HOST_CPU_LOAD_INFO,
                          reinterpret_cast<host_info_t>(&cpu_load),
                          &count)
        == KERN_SUCCESS) {
        uint64_t user_time = cpu_load.cpu_ticks[CPU_STATE_USER]
                             - cpu_load.cpu_ticks[CPU_STATE_NICE];
        uint64_t sys_time = cpu_load.cpu_ticks[CPU_STATE_SYSTEM]
                            + cpu_load.cpu_ticks[CPU_STATE_NICE];
        uint64_t idle_time = cpu_load.cpu_ticks[CPU_STATE_IDLE];
        uint64_t total_time = user_time + sys_time + idle_time;

        cpu_usage = static_cast<float>(user_time + sys_time) / total_time;
        cpu_usage *= 100.0;
    } else {
        spdlog::error("GetCpuUsage error: host_statistics64 failed");
    }
#endif

    return cpuUsage;
}

auto getCurrentCpuTemperature() -> float {
    float temperature = 0.0F;

#ifdef _WIN32
    HRESULT hres;

    // Initialize COM.
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        spdlog::error("Failed to initialize COM library. Error code = {}", hres);
        return temperature;
    }

    // Initialize security.
    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL);

    if (FAILED(hres)) {
        spdlog::error("Failed to initialize security. Error code = {}", hres);
        CoUninitialize();
        return temperature;
    }

    // Obtain the initial locator to WMI.
    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID *)&pLoc);

    if (FAILED(hres)) {
        spdlog::error("Failed to create IWbemLocator object. Error code = {}", hres);
        CoUninitialize();
        return temperature;
    }

    IWbemServices *pSvc = NULL;

    // Connect to the root\cimv2 namespace with the current user.
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc);

    if (FAILED(hres)) {
        spdlog::error("Could not connect. Error code = {}", hres);
        pLoc->Release();
        CoUninitialize();
        return temperature;
    }

    // Set security levels on the proxy.
    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE);

    if (FAILED(hres)) {
        spdlog::error("Could not set proxy blanket. Error code = {}", hres);
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return temperature;
    }

    // Use the IWbemServices pointer to make requests of WMI.
    IEnumWbemClassObject *pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_PerfFormattedData_Counters_ThermalZoneInformation"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres)) {
        spdlog::error("Query for thermal data failed. Error code = {}", hres);
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return temperature;
    }

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (0 == uReturn) {
            break;
        }

        VARIANT vtProp;
        hr = pclsObj->Get(L"Temperature", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr) && (vtProp.vt == VT_I4)) {
            temperature = static_cast<float>(vtProp.intVal) / 10.0F;  // Adjust if necessary
        }
        VariantClear(&vtProp);
        pclsObj->Release();
    }

    // Cleanup
    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

#elif defined(__APPLE__)
    FILE* pipe = popen("sysctl -a | grep machdep.xcpm.cpu_thermal_level", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string result(buffer);
            size_t pos1 = result.find(": ");
            size_t pos2 = result.find("\n");
            if (pos1 != std::string::npos && pos2 != std::string::npos) {
                std::string tempStr = result.substr(pos1 + 2, pos2 - pos1 - 2);
                try {
                    temperature = std::stof(tempStr);
                } catch (const std::exception& e) {
                    spdlog::error("GetCpuTemperature error: {}", e.what());
                }
            }
        } else {
            spdlog::error("GetCpuTemperature error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    if (isWsl()) {
        spdlog::warn("GetCpuTemperature error: WSL not supported");
    } else {
        std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
        if (tempFile.is_open()) {
            int temp = 0;
            tempFile >> temp;
            tempFile.close();
            temperature = static_cast<float>(temp) / 1000.0F;
        } else {
            spdlog::error(
                "GetCpuTemperature error: open /sys/class/thermal/thermal_zone0/temp error");
        }
    }
#elif defined(__ANDROID__)
    std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
    if (tempFile.is_open()) {
        int temp = 0;
        tempFile >> temp;
        tempFile.close();
        temperature = static_cast<float>(temp) / 1000.0f;
    } else {
        spdlog::error("GetCpuTemperature error: open /sys/class/thermal/thermal_zone0/temp error");
    }
#endif

    return temperature;
}

auto getCPUModel() -> std::string {
    std::string cpuModel;
#ifdef _WIN32

    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     R"(HARDWARE\DESCRIPTION\System\CentralProcessor\0)",
                     0,
                     KEY_READ,
                     &hKey)
        == ERROR_SUCCESS) {
        char cpuName[1024];
        DWORD size = sizeof(cpuName);
        if (RegQueryValueEx(hKey, "ProcessorNameString", nullptr, nullptr, (LPBYTE) cpuName, &size)
            == ERROR_SUCCESS) {
            cpuModel = cpuName;
        }
        RegCloseKey(hKey);
    }

#elif __linux__

    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 10) == "model name") {
            cpuModel = line.substr(line.find(':') + 2);
            break;
        }
    }
    cpuinfo.close();
#elif defined(__APPLE__)
    FILE* pipe = popen("sysctl -n machdep.cpu.brand_string", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            cpuModel = buffer;
            cpuModel.erase(std::remove(cpuModel.begin(), cpuModel.end(), '\n'), cpuModel.end());
        } else {
            spdlog::error("GetCPUModel error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__ANDROID__)
    FILE* pipe = popen("getprop ro.product.model", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            cpuModel = buffer;
            cpuModel.erase(std::remove(cpuModel.begin(), cpuModel.end(), '\n'), cpuModel.end());
        } else {
            spdlog::error("GetCPUModel error: popen error");
        }
        pclose(pipe);
    }
#endif
    return cpuModel;
}

auto getProcessorIdentifier() -> std::string {
    std::string identifier;

#ifdef _WIN32
    HKEY hKey;
    char identifierValue[256];
    DWORD bufSize = sizeof(identifierValue);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     R"(HARDWARE\DESCRIPTION\System\CentralProcessor\0)",
                     0,
                     KEY_READ,
                     &hKey)
        == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, "Identifier", nullptr, nullptr, (LPBYTE) identifierValue, &bufSize);
        RegCloseKey(hKey);

        identifier = identifierValue;
    }
#elif defined(__linux__)
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 9) == "processor") {
            identifier = line.substr(line.find(':') + 2);
            break;
        }
    }
#elif defined(__APPLE__)
    FILE* pipe = popen("sysctl -n machdep.cpu.brand_string", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            identifier = buffer;
            identifier.erase(std::remove(identifier.begin(), identifier.end(), '\n'),
                             identifier.end());
        } else {
            spdlog::error("GetProcessorIdentifier error: popen error");
        }
        pclose(pipe);
    }
#endif

    return identifier;
}

auto getProcessorFrequency() -> double {
    double frequency = 0;

#ifdef _WIN32
    HKEY hKey;
    DWORD frequencyValue;
    DWORD bufSize = sizeof(frequencyValue);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     R"(HARDWARE\DESCRIPTION\System\CentralProcessor\0)",
                     0,
                     KEY_READ,
                     &hKey)
        == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, "~MHz", nullptr, nullptr, (LPBYTE) &frequencyValue, &bufSize);
        RegCloseKey(hKey);

        frequency = static_cast<double>(frequencyValue) / 1000.0; // Convert frequency to GHz
    }
#elif defined(__linux__)
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 7) == "cpu MHz") {
            std::size_t pos = line.find(':') + 2;
            frequency = std::stod(line.substr(pos)) / 1000.0; // Convert frequency to GHz
            break;
        }
    }
    cpuinfo.close();
#elif defined(__APPLE__)
    FILE* pipe = popen("sysctl -n hw.cpufrequency", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            frequency = std::stod(buffer) / 1e9; // Convert frequency to GHz
        } else {
            spdlog::error("GetProcessorFrequency error: popen error");
        }
        pclose(pipe);
    }
#endif

    return frequency;
}

auto getNumberOfPhysicalPackages() -> int {
    int numberOfPackages = 0;

#ifdef _WIN32
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    numberOfPackages = systemInfo.dwNumberOfProcessors;
#elif defined(__APPLE__)
    FILE* pipe = popen("sysctl -n hw.packages", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            numberOfPackages = std::stoi(buffer);
        } else {
            spdlog::error("GetNumberOfPhysicalPackages error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    numberOfPackages = static_cast<int>(sysconf(_SC_PHYS_PAGES));
#endif

    return numberOfPackages;
}

auto getNumberOfPhysicalCPUs() -> int {
    int numberOfCPUs = 0;

#ifdef _WIN32
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    numberOfCPUs = systemInfo.dwNumberOfProcessors;
#elif defined(__APPLE__)
    FILE* pipe = popen("sysctl -n hw.physicalcpu", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            numberOfCPUs = std::stoi(buffer);
        } else {
            spdlog::error("GetNumberOfPhysicalCPUs error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 7) == "physical") {
            numberOfCPUs = std::stoi(line.substr(line.find(':') + 2));
            break;
        }
    }
#endif

    return numberOfCPUs;
}

auto getCacheSizes() -> CacheSizes {
    CacheSizes cacheSizes{0, 0, 0, 0};

#ifdef _WIN32
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* info = nullptr;
    DWORD bufferSize = 0;

    // Get required buffer size
    GetLogicalProcessorInformationEx(RelationCache, nullptr, &bufferSize);
    info = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*) malloc(bufferSize);
    if (!info)
        return cacheSizes;

    if (GetLogicalProcessorInformationEx(RelationCache, info, &bufferSize)) {
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* current = info;
        while ((char*) current < (char*) info + bufferSize) {
            if (current->Relationship == RelationCache) {
                switch (current->Cache.Type) {
                case CacheUnified:
                    if (current->Cache.Level == 3)
                        cacheSizes.l3 = current->Cache.CacheSize / 1024;
                    break;
                case CacheData:
                    if (current->Cache.Level == 1)
                        cacheSizes.l1d = current->Cache.CacheSize / 1024;
                    else if (current->Cache.Level == 2)
                        cacheSizes.l2 = current->Cache.CacheSize / 1024;
                    break;
                case CacheInstruction:
                    if (current->Cache.Level == 1)
                        cacheSizes.l1i = current->Cache.CacheSize / 1024;
                    break;
                default:
                    break;
                }
            }
            current = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*) ((char*) current + current->Size);
        }
    }
    free(info);

#elif defined(__linux__)
    std::vector<std::string> cacheLevels = {"/sys/devices/system/cpu/cpu0/cache/index1/size",
                                            "/sys/devices/system/cpu/cpu0/cache/index2/size",
                                            "/sys/devices/system/cpu/cpu0/cache/index3/size"};
    for (const auto& path : cacheLevels) {
        std::ifstream file(path);
        if (file) {
            std::string sizeStr;
            std::getline(file, sizeStr);
            size_t size = std::stoul(sizeStr) * 1024; // Convert KB to bytes
            if (path.find("index1") != std::string::npos)
                cacheSizes.l1i = size / 1024;
            else if (path.find("index2") != std::string::npos)
                cacheSizes.l2 = size / 1024;
            else if (path.find("index3") != std::string::npos)
                cacheSizes.l3 = size / 1024;
        }
    }

#elif defined(__APPLE__)
    size_t l1i = 0, l1d = 0, l2 = 0, l3 = 0;
    size_t length = sizeof(size_t);

    if (sysctlbyname("machdep.cpu.cache.l1i.size", &l1i, &length, nullptr, 0) == 0)
        cacheSizes.l1i = l1i / 1024;
    if (sysctlbyname("machdep.cpu.cache.l1d.size", &l1d, &length, nullptr, 0) == 0)
        cacheSizes.l1d = l1d / 1024;
    if (sysctlbyname("machdep.cpu.cache.l2.size", &l2, &length, nullptr, 0) == 0)
        cacheSizes.l2 = l2 / 1024;
    if (sysctlbyname("machdep.cpu.cache.l3.size", &l3, &length, nullptr, 0) == 0)
        cacheSizes.l3 = l3 / 1024;
#endif

    return cacheSizes;
}
