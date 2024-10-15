#include "Os.h"

#include <array>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#include <fstream>
#elif __APPLE__
#include <sys/utsname.h>
#endif

#include <spdlog/spdlog.h>

auto OperatingSystemInfo::toJson() const -> std::string {
    std::stringstream stringstream;
    stringstream << "{\n";
    stringstream << R"(  "osName": ")" << osName << "\",\n";
    stringstream << R"(  "osVersion": ")" << osVersion << "\",\n";
    stringstream << R"(  "kernelVersion": ")" << kernelVersion << "\"\n";
    stringstream << R"(  "architecture": ")" << architecture << "\"\n";
    stringstream << "}\n";
    return stringstream.str();
}

auto getComputerName() -> std::optional<std::string> {
    constexpr size_t bufferSize = 256;
    std::array<char, bufferSize> buffer;

#if defined(_WIN32)
    auto size = static_cast<DWORD>(buffer.size());
    if (BOOL result = GetComputerNameA(buffer.data(), &size); result) {
        return std::string(buffer.data());
    }
#elif defined(__APPLE__)
    CFStringRef name = SCDynamicStoreCopyComputerName(NULL, NULL);
    if (name != NULL) {
        CFStringGetCString(name, buffer.data(), buffer.size(),
                           kCFStringEncodingUTF8);
        CFRelease(name);
        return std::string(buffer.data());
    }
#elif defined(__linux__) || defined(__linux)
    if (gethostname(buffer.data(), buffer.size()) == 0) {
        return std::string(buffer.data());
    }
#elif defined(__ANDROID__)
    return std::nullopt;
#endif

    return std::nullopt;
}

auto parseFile(const std::string& filePath)
    -> std::pair<std::string, std::string> {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        spdlog::error("Cannot open file: {}", filePath);
        return {};
    }

    std::pair<std::string, std::string> osInfo;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;  // Skip empty lines and comments
        }

        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);

            // Remove double quotes from the value
            if (!value.empty() && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }

            if (key == "PRETTY_NAME") {
                osInfo.first = value;
            } else if (key == "VERSION") {
                osInfo.second = value;
            }
        }
    }

    return osInfo;
}

auto getOperatingSystemInfo() -> OperatingSystemInfo {
    OperatingSystemInfo osInfo;

#ifdef _WIN32
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (GetVersionEx((LPOSVERSIONINFO)&osvi) != 0) {
        osInfo.osName = "Windows";
        osInfo.osVersion = std::format("{}.{} (Build {})", osvi.dwMajorVersion,
                                       osvi.dwMinorVersion, osvi.dwBuildNumber);
    } else {
        spdlog::error("Failed to get OS version");
    }
#elif __linux__
    auto osReleaseInfo = parseFile("/etc/os-release");
    if (!osReleaseInfo.first.empty()) {
        osInfo.osName = osReleaseInfo.first;
        osInfo.osVersion = osReleaseInfo.second;
    } else {
        auto lsbReleaseInfo = parseFile("/etc/lsb-release");
        if (!lsbReleaseInfo.first.empty()) {
            osInfo.osName = lsbReleaseInfo.first;
            osInfo.osVersion = lsbReleaseInfo.second;
        } else {
            std::ifstream redhatReleaseFile("/etc/redhat-release");
            if (redhatReleaseFile.is_open()) {
                std::string line;
                std::getline(redhatReleaseFile, line);
                osInfo.osName = line;
                redhatReleaseFile.close();
            }
        }
    }

    if (osInfo.osName.empty()) {
        spdlog::error("Failed to get OS name");
    }

    std::ifstream kernelVersionFile("/proc/version");
    if (kernelVersionFile.is_open()) {
        std::string line;
        std::getline(kernelVersionFile, line);
        osInfo.kernelVersion = line.substr(0, line.find(" "));
        kernelVersionFile.close();
    } else {
        spdlog::error("Failed to open /proc/version");
    }
#elif __APPLE__
    struct utsname info;
    if (uname(&info) == 0) {
        osInfo.osName = info.sysname;
        osInfo.osVersion = info.release;
        osInfo.kernelVersion = info.version;
    }
#endif

#if defined(__i386__) || defined(__i386)
    const std::string ARCHITECTURE = "x86";
#elif defined(__x86_64__)
    const std::string ARCHITECTURE = "x86_64";
#elif defined(__arm__)
    const std::string ARCHITECTURE = "ARM";
#elif defined(__aarch64__)
    const std::string ARCHITECTURE = "ARM64";
#else
    const std::string ARCHITECTURE = "Unknown architecture";
#endif
    osInfo.architecture = ARCHITECTURE;

    const std::string COMPILER =
#if defined(__clang__)
        std::format("Clang {}.{}.{}", __clang_major__, __clang_minor__,
                    __clang_patchlevel__);
#elif defined(__GNUC__)
        std::format("GCC {}.{}.{}", __GNUC__, __GNUC_MINOR__,
                    __GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
        std::format("MSVC {}", _MSC_FULL_VER);
#else
        "Unknown compiler";
#endif
    osInfo.compiler = COMPILER;

    osInfo.computerName = getComputerName().value_or("Unknown computer name");

    return osInfo;
}

auto isWsl() -> bool {
    std::ifstream procVersion("/proc/version");
    std::string line;
    if (procVersion.is_open()) {
        std::getline(procVersion, line);
        procVersion.close();
        // Check if the line contains "Microsoft" which is a typical indicator
        // of WSL
        return line.find("microsoft") != std::string::npos ||
               line.find("WSL") != std::string::npos;
    }
    return false;
}
