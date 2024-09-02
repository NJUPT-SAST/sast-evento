#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class AboutPage : public BasicView, private GlobalAgent<AboutPageBridge> {
public:
    AboutPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);

private:
    void onCreate() override;
};

EVENTO_UI_END