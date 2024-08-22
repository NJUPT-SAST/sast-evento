#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>
#include <string>

EVENTO_UI_START

class AboutPage : public BasicView, private GlobalAgent<AboutPageBridge> {
public:
    AboutPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    AboutPage(AboutPage&) = delete;

private:
    void onCreate() override;
    // void onClick() override;
    void openWeb(std::string_view& url);
};

EVENTO_UI_END