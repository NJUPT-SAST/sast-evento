#pragma once

#include <ui/Core/BasicView.h>
#include <ui/Core/GlobalAgent.hpp>
#include <ui/Core/UiBase.h>

EVENTO_UI_START

class AboutPage : public BasicView, private GlobalAgent<AboutPageBridge> {
public:
    AboutPage(slint::ComponentHandle<UiEntryName> uiEntry);
    AboutPage(AboutPage&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END