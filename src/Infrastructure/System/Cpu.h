#pragma once

#include <string>

/**
 * @brief A structure to hold the sizes of the CPU caches.
 *
 * This structure contains the sizes of the L1 data cache, L1 instruction cache,
 * L2 cache, and L3 cache of the CPU. These values are typically provided in
 * bytes. The cache size information is important for performance tuning and
 * understanding the CPU's capabilities.
 */
struct CacheSizes {
    size_t l1d; ///< The size of the L1 data cache in bytes.
    size_t l1i; ///< The size of the L1 instruction cache in bytes.
    size_t l2;  ///< The size of the L2 cache in bytes.
    size_t l3;  ///< The size of the L3 cache in bytes.
}; ///< Ensure the structure is aligned to a 32-byte boundary.

/**
 * @brief Retrieves the current CPU usage percentage.
 *
 * This function calculates and returns the current percentage of CPU usage.
 * It typically measures how much time the CPU spends in active processing
 * compared to idle time. The CPU usage percentage can be useful for
 * monitoring system performance and detecting high load conditions.
 *
 * @return A float representing the current CPU usage as a percentage (0.0 to
 * 100.0).
 */
[[nodiscard]] auto getCurrentCpuUsage() -> float;

/**
 * @brief Retrieves the current CPU temperature.
 *
 * This function returns the current temperature of the CPU in degrees Celsius.
 * Monitoring the CPU temperature is important for preventing overheating and
 * ensuring optimal performance. If the temperature is too high, it could
 * indicate cooling issues or high load on the system.
 *
 * @return A float representing the CPU temperature in degrees Celsius.
 */
[[nodiscard]] auto getCurrentCpuTemperature() -> float;

/**
 * @brief Retrieves the CPU model name.
 *
 * This function returns a string that contains the model name of the CPU.
 * The CPU model provides information about the manufacturer and specific model
 * (e.g., "Intel Core i7-10700K", "AMD Ryzen 9 5900X"). This information can be
 * useful for system diagnostics and performance evaluations.
 *
 * @return A string representing the CPU model name.
 */
[[nodiscard]] auto getCPUModel() -> std::string;

/**
 * @brief Retrieves the CPU identifier.
 *
 * This function returns a unique string identifier for the CPU. This identifier
 * typically includes details about the CPU architecture, model, stepping, and
 * other low-level characteristics. It can be useful for identifying specific
 * processor versions in a system.
 *
 * @return A string representing the CPU identifier.
 */
[[nodiscard]] auto getProcessorIdentifier() -> std::string;

/**
 * @brief Retrieves the current CPU frequency.
 *
 * This function returns the current operating frequency of the CPU in GHz.
 * The frequency may vary depending on system load, power settings, and
 * the capabilities of the CPU (e.g., turbo boost, power-saving modes).
 * Understanding the CPU frequency is important for performance tuning and
 * optimizing application performance.
 *
 * @return A double representing the CPU frequency in gigahertz (GHz).
 */
[[nodiscard]] auto getProcessorFrequency() -> double;

/**
 * @brief Retrieves the number of physical CPU packages.
 *
 * This function returns the number of physical CPU packages (sockets) installed
 * in the system. A system may have multiple CPU packages, especially in
 * server configurations. Each physical package may contain multiple cores.
 *
 * @return An integer representing the number of physical CPU packages.
 */
[[nodiscard]] auto getNumberOfPhysicalPackages() -> int;

/**
 * @brief Retrieves the number of logical CPUs (cores).
 *
 * This function returns the number of logical CPUs (cores) available in the
 * system. Logical CPUs include both physical cores and additional virtual cores
 * created by technologies like Hyper-Threading (on Intel processors) or SMT
 * (on AMD processors). This value represents the total number of logical
 * processors that the operating system can use.
 *
 * @return An integer representing the total number of logical CPUs (cores).
 */
[[nodiscard]] auto getNumberOfPhysicalCPUs() -> int;

/**
 * @brief Retrieves the sizes of the CPU caches (L1, L2, L3).
 *
 * This function returns a `CacheSizes` structure that contains the sizes of
 * the L1 data cache (L1D), L1 instruction cache (L1I), L2 cache, and L3 cache.
 * Cache sizes play an important role in determining CPU performance, as
 * larger caches can improve data locality and reduce memory latency.
 *
 * @return A `CacheSizes` structure containing the sizes of the L1, L2, and L3
 * caches in bytes.
 */
[[nodiscard]] auto getCacheSizes() -> CacheSizes;
