#pragma once

#include <Infrastructure/Network/ResponseStruct.h>
#include <app.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace evento::convert {

namespace details {

slint::SharedString convertTimeRange(const std::string& startTimeStr,
                                     const std::string& endTimeStr) {
    std::istringstream ssStart(startTimeStr);
    std::istringstream ssEnd(endTimeStr);
    std::chrono::sys_seconds startTp, endTp;

    ssStart >> std::chrono::parse("%Y-%m-%dT%H:%M:%SZ", startTp);
    if (ssStart.fail()) {
        spdlog::warn("Failed to parse start-time string: {}", startTimeStr);
        return " ";
    }
    ssEnd >> std::chrono::parse("%Y-%m-%dT%H:%M:%SZ", endTp);
    if (ssEnd.fail()) {
        spdlog::warn("Failed to parse end-time string: {}", endTimeStr);
        return " ";
    }

    auto startTimer = std::chrono::system_clock::to_time_t(startTp);
    auto startDate = *std::gmtime(&startTimer);

    auto endTimer = std::chrono::system_clock::to_time_t(endTp);
    auto endDate = *std::gmtime(&endTimer);

    std::string startStr = std::format("{:02}.{:02} {:02}:{:02}",
                                       startDate.tm_mon + 1,
                                       startDate.tm_mday,
                                       startDate.tm_hour,
                                       startDate.tm_min);
    std::string endStr = std::format("{:02}.{:02} {:02}:{:02}",
                                     endDate.tm_mon + 1,
                                     endDate.tm_mday,
                                     endDate.tm_hour,
                                     endDate.tm_min);

    if (startDate.tm_year != endDate.tm_year) {
        return slint::SharedString{std::format("{:04} {} - {:04} {}",
                                               startDate.tm_year + 1900,
                                               startStr,
                                               endDate.tm_year + 1900,
                                               endStr)};
    }
    if (startDate.tm_mon != endDate.tm_mon || startDate.tm_mday != endDate.tm_mday) {
        return slint::SharedString{startStr + " - " + endStr};
    }
    return slint::SharedString{startStr + " - " + endStr.substr(6)};
}

slint::SharedString firstUnicode(const std::string& str) {
    if (str.empty()) {
        return " ";
    }
    size_t length = 0;
    auto firstByte = static_cast<unsigned char>(str[0]);
    if ((firstByte & 0x80) == 0) {
        // ASCII character
        length = 1;
    } else if ((firstByte & 0xE0) == 0xC0) {
        // Two-byte character
        length = 2;
    } else if ((firstByte & 0xF0) == 0xE0) {
        // Three-byte character
        length = 3;
    } else if ((firstByte & 0xF8) == 0xF0) {
        // Four-byte character
        length = 4;
    } else {
        // Invalid Unicode
        return " ";
    }
    if (str.size() < length) {
        return " ";
    }
    return slint::SharedString{str.substr(0, length)};
}

} // namespace details

using EventEntityList = std::vector<EventEntity>;
using EventStructModel = std::shared_ptr<slint::VectorModel<EventStruct>>;

auto from(const auto& obj) {
    return obj;
}

EventStruct from(const EventEntity& entity) {
    return {
        .id = entity.id,
        .summary = slint::SharedString(entity.summary),
        .summary_abbr = details::firstUnicode(entity.summary),
        .description = slint::SharedString(entity.description),
        .time = details::convertTimeRange(entity.start, entity.end),
        .location = slint::SharedString(entity.location),
        .tag = slint::SharedString(entity.tag),
        .larkMeetingRoomName = slint::SharedString(entity.larkMeetingRoomName),
        .larkDepartmentName = slint::SharedString(entity.larkDepartmentName),
        .state = static_cast<EventState>(entity.state),
        .is_subscribed = entity.isSubscribed,
        .is_checkIn = entity.isCheckedIn,
    };
}

EventStructModel from(const EventEntityList& list) {
    std::vector<EventStruct> model;
    model.reserve(list.size());
    for (auto& entity : list) {
        model.push_back(from(entity));
    }
    return std::make_shared<slint::VectorModel<EventStruct>>(model);
}

} // namespace evento::convert