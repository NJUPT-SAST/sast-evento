#pragma once

#include <ui/Core/BasicView.h>
#include <ui/Core/GlobalAgent.hpp>
#include <ui/Core/UiBase.h>

EVENTO_UI_START

class DetailPage : public BasicView, private GlobalAgent<DetailPageBridge> {
public:
    DetailPage(slint::ComponentHandle<UiEntryName> uiEntry);
    DetailPage(DetailPage&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END