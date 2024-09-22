#pragma once

#include <Infrastructure/Network/ResponseStruct.h>
#include <app.h>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace evento::convert {

namespace details {

slint::SharedString convertTimeRange(const std::string& startTimeStr, const std::string& endTimeStr);

slint::SharedString firstUnicode(const std::string& str);

} // namespace details

EventStruct from(const EventEntity& entity);

std::shared_ptr<slint::VectorModel<EventStruct>> from(const std::vector<EventEntity>& list);

ContributorStruct from(const std::filesystem::path& avatar, const std::string& htmlUrl);

FeedbackStruct from(const std::optional<FeedbackEntity>& entity);

} // namespace evento::convert