#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hpp>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class MyEventPage : public BasicView, private GlobalAgent<MyEventPageBridge> {
public:
    MyEventPage(slint::ComponentHandle<UiEntryName> uiEntry);
    MyEventPage(MyEventPage&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END