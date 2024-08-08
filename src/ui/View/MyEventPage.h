#pragma once

#include <ui/Core/BasicView.h>
#include <ui/Core/GlobalAgent.hpp>
#include <ui/Core/UiBase.h>

EVENTO_UI_START

class MyEventPage : public BasicView, private GlobalAgent<MyEventPageBridge> {
public:
    MyEventPage(slint::ComponentHandle<UiEntryName> uiEntry);
    MyEventPage(MyEventPage&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END