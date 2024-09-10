#include <Controller/View/SettingPage.h>
#include <Infrastructure/Utils/Config.h>

EVENTO_UI_START

SettingPage::SettingPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void SettingPage::onCreate() {
    auto& self = *this;

    const auto [languageIdx, minimalToTray, noticeBegin, noticeEnd, themeIdx] = evento::settings;

    self->set_language_index(languageIdx);
    self->set_minimal_to_tray(minimalToTray);
    self->set_notice_begin(noticeBegin);
    self->set_notice_end(noticeEnd);
    self->set_theme_index(themeIdx);

    self->invoke_change_theme();

    config.insert_or_assign("setting",
                            toml::table{
                                {"language", languageIdx},
                                {"minimal-to-tray", minimalToTray},
                                {"notice-begin", noticeBegin},
                                {"notice-end", noticeEnd},
                                {"theme", themeIdx},
                            });

    self->on_language_changed([&self = *this]() {
        auto& setting = config["setting"].ref<toml::table>();
        setting.insert_or_assign("language", self->get_language_index());
        evento::settings.language = self->get_language_index();
    });

    self->on_minimal_to_tray_changed([&self = *this]() {
        auto& setting = config["setting"].ref<toml::table>();
        setting.insert_or_assign("minimal-to-tray", self->get_minimal_to_tray());
        evento::settings.minimalToTray = self->get_minimal_to_tray();
    });

    self->on_notice_begin_changed([&self = *this]() {
        auto& setting = config["setting"].ref<toml::table>();
        setting.insert_or_assign("notice-begin", self->get_notice_begin());
        evento::settings.noticeBegin = self->get_notice_begin();
    });

    self->on_notice_end_changed([&self = *this]() {
        auto& setting = config["setting"].ref<toml::table>();
        setting.insert_or_assign("notice-end", self->get_notice_end());
        evento::settings.noticeEnd = self->get_notice_end();
    });

    self->on_theme_changed([&self = *this]() {
        auto& setting = config["setting"].ref<toml::table>();
        setting.insert_or_assign("theme", self->get_theme_index());
        evento::settings.theme = self->get_theme_index();
    });
}

EVENTO_UI_END