#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>
#include <Infrastructure/Utils/Result.h>
#include <vector>

EVENTO_UI_START

class HistoryPage : public BasicView, private GlobalAgent<HistoryPageBridge> {
public:
    HistoryPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);

private:
    void onCreate() override;

    void onShow() override;

    void loadHistoryEvents(int page, int size);

    Task<Result<std::vector<EventFeedbackStruct>>> loadHistoryEventsTask(int page, int size);

    void feedbackEvent(int eventId, int rating, std::string content);

    std::vector<EventFeedbackStruct> eventFeedbacks;
};

EVENTO_UI_END