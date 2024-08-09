#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hpp>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class AboutPage : public BasicView, private GlobalAgent<AboutPageBridge> {
public:
    AboutPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    AboutPage(AboutPage&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END