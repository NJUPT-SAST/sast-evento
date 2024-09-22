#include <Controller/Convert.h>
#include <chrono>
#include <spdlog/spdlog.h>

namespace evento::convert {

namespace details {

inline time_t parseIso8601Utc(const char* date) {
    struct tm tt = {0};
    double seconds;
    if (sscanf(date,
               "%04d-%02d-%02dT%02d:%02d:%lfZ",
               &tt.tm_year,
               &tt.tm_mon,
               &tt.tm_mday,
               &tt.tm_hour,
               &tt.tm_min,
               &seconds)
        != 6)
        return -1;
    tt.tm_sec = (int) seconds;
    tt.tm_mon -= 1;
    tt.tm_year -= 1900;
    tt.tm_isdst = -1;
#ifdef _MSC_VER
    return _mkgmtime(&tt);
#else
    return timegm(&tt);
#endif
}

slint::SharedString convertTimeRange(const std::string& startTimeStr,
                                     const std::string& endTimeStr) {
    auto startTimer = parseIso8601Utc(startTimeStr.c_str());
    auto startDate = *std::gmtime(&startTimer);

    auto endTimer = parseIso8601Utc(endTimeStr.c_str());
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

std::shared_ptr<slint::VectorModel<EventStruct>> from(const std::vector<EventEntity>& list) {
    std::vector<EventStruct> model;
    model.reserve(list.size());
    for (auto& entity : list) {
        model.push_back(from(entity));
    }
    return std::make_shared<slint::VectorModel<EventStruct>>(model);
}

ContributorStruct from(const std::filesystem::path& avatar, const std::string& htmlUrl) {
    return {.avatar = slint::Image::load_from_path(avatar.string().c_str()),
            .html_url = slint::SharedString(htmlUrl)};
}

FeedbackStruct from(const std::optional<FeedbackEntity>& entity) {
    if (!entity)
        return {.success = true,
                .has_feedbacked = false,
                .rate = 0,
                .content = slint::SharedString("")};
    return {
        .success = true,
        .has_feedbacked = true,
        .rate = entity->rating,
        .content = slint::SharedString(entity->feedback.value_or("")),
    };
}

} // namespace evento::convert