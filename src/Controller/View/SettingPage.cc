#include <Controller/View/SettingPage.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Utils/Config.h>

EVENTO_UI_START

SettingPage::SettingPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void SettingPage::onCreate() {
    auto& self = *this;

    const auto [minimalToTray, noticeBegin, noticeEnd, autoLogin, themeIdx] = evento::settings;
    self->set_minimal_to_tray(minimalToTray);
    self->set_notice_begin(noticeBegin);
    self->set_notice_end(noticeEnd);
    self->set_theme_index(themeIdx);
    self->set_auto_login(autoLogin);

    self->invoke_change_theme();

    config.insert_or_assign("setting",
                            toml::table{
                                {"minimal-to-tray", minimalToTray},
                                {"notice-begin", noticeBegin},
                                {"notice-end", noticeEnd},
                                {"theme", themeIdx},
                                {"auto-login", autoLogin},
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

    self->on_auto_login_changed([&self = *this]() {
        auto& setting = config["setting"].ref<toml::table>();
        setting.insert_or_assign("auto-login", self->get_auto_login());
        evento::settings.autoLogin = self->get_auto_login();
    });

    self->on_theme_changed([&self = *this]() {
        auto& setting = config["setting"].ref<toml::table>();
        setting.insert_or_assign("theme", self->get_theme_index());
        evento::settings.theme = self->get_theme_index();
    });

    self->on_clear_cache([&self = *this]() {
        networkClient()->clearCache();
        self->set_cache_size(slint::SharedString(networkClient()->getTotalCacheSizeFormatString()));
    });

    self->on_is_windows([]() {
#ifdef PLATFORM_WINDOWS
        return true;
#else
        return false;
#endif
    });
}

void SettingPage::onShow() {
    auto& self = *this;

    self->set_minimal_to_tray(evento::settings.minimalToTray);
    self->set_notice_begin(evento::settings.noticeBegin);
    self->set_notice_end(evento::settings.noticeEnd);
    self->set_auto_login(evento::settings.autoLogin);
    self->set_theme_index(evento::settings.theme);

    self->set_cache_size(slint::SharedString(networkClient()->getTotalCacheSizeFormatString()));
}

void SettingPage::onLogout() {
    networkClient()->clearMemoryCache();
}

EVENTO_UI_END