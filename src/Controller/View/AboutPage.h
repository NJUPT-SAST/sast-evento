#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>

EVENTO_UI_START

namespace fs = std::filesystem;

class AboutPage : public BasicView, private GlobalAgent<AboutPageBridge> {
public:
    AboutPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);

private:
    void onCreate() override;
    void onShow() override;

    void loadContributors();
    void checkUpdate();

    std::vector<slint::Image> avatars;
};

EVENTO_UI_END