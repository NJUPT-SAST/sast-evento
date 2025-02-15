#pragma once

#include <string>
#include <utility>
#include <vector>

struct MemoryInfo {
    struct MemorySlot {
        std::string capacity;
        std::string clockSpeed;
        std::string type;

        MemorySlot() = default;
        MemorySlot(std::string capacity, std::string clockSpeed, std::string type)
            : capacity(std::move(capacity))
            , clockSpeed(std::move(clockSpeed))
            , type(std::move(type)) {}
    };

    std::vector<MemorySlot> slots;
    unsigned long long virtualMemoryMax;
    unsigned long long virtualMemoryUsed;
    unsigned long long swapMemoryTotal;
    unsigned long long swapMemoryUsed;
};

/**
 * @brief Get the memory usage percentage.
 * 获取内存使用率百分比
 *
 * @return The memory usage percentage.
 *         内存使用率百分比
 */
auto getMemoryUsage() -> float;

/**
 * @brief Get the total memory size.
 * 获取总内存大小
 *
 * @return The total memory size.
 *         总内存大小
 */
auto getTotalMemorySize() -> unsigned long long;

/**
 * @brief Get the available memory size.
 * 获取可用内存大小
 *
 * @return The available memory size.
 *         可用内存大小
 */
auto getAvailableMemorySize() -> unsigned long long;

/**
 * @brief Get the physical memory slot info.
 * 获取物理内存槽信息
 *
 * @return The physical memory slot info.
 *         物理内存槽信息
 */
auto getPhysicalMemoryInfo() -> MemoryInfo::MemorySlot;

/**
 * @brief Get the virtual memory max size.
 * 获取虚拟内存最大值
 *
 * @return The virtual memory max size.
 *         虚拟内存最大值
 */
auto getVirtualMemoryMax() -> unsigned long long;

/**
 * @brief Get the virtual memory used size.
 * 获取虚拟内存已用值
 *
 * @return The virtual memory used size.
 *         虚拟内存已用值
 */
auto getVirtualMemoryUsed() -> unsigned long long;

/**
 * @brief Get the swap memory total size.
 * 获取交换内存总值
 *
 * @return The swap memory total size.
 *         交换内存总值
 */
auto getSwapMemoryTotal() -> unsigned long long;

/**
 * @brief Get the swap memory used size.
 * 获取交换内存已用值
 *
 * @return The swap memory used size.
 *         交换内存已用值
 */
auto getSwapMemoryUsed() -> unsigned long long;

/**
 * @brief Get the committed memory.
 * 获取已分配内存
 *
 * @return The committed memory.
 *         已分配内存
 */
auto getCommittedMemory() -> size_t;

/**
 * @brief Get the uncommitted memory.
 * 获取未分配内存
 *
 * @return The uncommitted memory.
 *         未分配内存
 */
auto getUncommittedMemory() -> size_t;
