#include <Controller/Convert.h>
#include <Infrastructure/Utils/Tools.h>
#include <boost/algorithm/string.hpp>
#include <ranges>
#include <spdlog/spdlog.h>

namespace evento::convert {

namespace details {

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
    boost::algorithm::trim(entity.summary);
    return {
        .id = entity.id,
        .summary = slint::SharedString(entity.summary),
        .summary_abbr = details::firstUnicode(entity.summary),
        .description = slint::SharedString(entity.description),
        .time = details::convertTimeRange(entity.start, entity.end),
        .location = slint::SharedString(std::format("{}{}{}",
                                                    entity.location.value_or(""),
                                                    entity.location.has_value() ? " " : "",
                                                    entity.larkMeetingRoomName.value_or(""))),
        .tag = slint::SharedString(entity.tag),
        .larkDepartmentName = slint::SharedString(entity.larkDepartmentName),
        .state = static_cast<EventState>(entity.state),
        .is_subscribed = entity.isSubscribed,
        .is_check_in = entity.isCheckedIn,
    };
}

std::shared_ptr<slint::VectorModel<EventStruct>> from(const std::vector<EventEntity>& list) {
    auto transformedList = list | std::views::transform([&](const EventEntity& entity) {
                               auto startTime = parseIso8601Utc(entity.start.c_str());
                               auto startDuration = std::chrono::system_clock::from_time_t(startTime)
                                                    - std::chrono::system_clock::now();
                               return std::make_pair(std::chrono::abs(startDuration),
                                                     std::cref(entity));
                           });

    std::map<std::chrono::duration<double>, std::reference_wrapper<const EventEntity>>
        sortedList(transformedList.begin(), transformedList.end());

    std::vector<EventStruct> model;
    model.reserve(list.size());

    for (const auto& [_, entity] : sortedList) {
        model.push_back(from(entity));
    }
    return std::make_shared<slint::VectorModel<EventStruct>>(model);
}

ContributorStruct from(const std::filesystem::path& avatar, const std::string& htmlUrl) {
    return {.avatar = slint::Image::load_from_path(slint::SharedString(avatar.u8string())),
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