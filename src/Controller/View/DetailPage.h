#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class DetailPage : public BasicView, private GlobalAgent<DetailPageBridge> {
public:
    DetailPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);

private:
    void onCreate() override;

    void onShow() override;

    void loadEvent();
    void loadFeedback();
    void checkIn(int eventId, std::string checkInCode);
    void subscribe(int eventId, bool subscribe);
    void feedbackEvent(int eventId, int rate, std::string content);
};

EVENTO_UI_END