#include <Controller/AsyncExecutor.hh>
#include <Controller/Convert.hh>
#include <Controller/UiBridge.h>
#include <Controller/View/AboutPage.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <Infrastructure/Utils/Result.h>
#include <Infrastructure/Utils/Tools.h>
#include <Version.h>
#include <filesystem>
#include <memory>
#include <spdlog/spdlog.h>

EVENTO_UI_START

AboutPage::AboutPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void AboutPage::onCreate() {
    auto& self = *this;

    self->set_version("v" VERSION_FULL);
    self->on_open_web([this](slint::SharedString url) { openBrowser(std::string(url)); });
    self->on_load_contributors([this] { loadContributors(); });
    self->on_check_update([this] { checkUpdate(); });
}

void AboutPage::onShow() {
    loadContributors();
}

void AboutPage::loadContributors() {
    auto& self = *this;

    self->set_contributors_status(Status::Loading);

    executor()->asyncExecute(
        networkClient()->getContributors(), [&self = *this, this](Result<ContributorList> result) {
            if (result.isErr()) {
                self->set_contributors_status(Status::Error);
                self.bridge.getMessageManager().showMessage(result.unwrapErr().what(),
                                                            MessageType::Error);
                return;
            }

            auto contributors = result.unwrap();
            self.contributors.clear();
            auto total = contributors.size();

            for (int i = 0; i < contributors.size(); ++i) {
                executor()->asyncExecute(
                    networkClient()->getFile(contributors[i].avatar_url),
                    [&self = *this, i, total, htmlUrl = contributors[i].html_url](
                        Result<std::filesystem::path> result) {
                        if (result.isErr()) {
                            spdlog::warn("download avatar failed: {}", result.unwrapErr().what());
                        } else {
                            self.contributors.emplace_back(
                                evento::convert::from(result.unwrap(), htmlUrl));
                        }

                        if (i == total - 1) {
                            self->set_contributors(
                                std::make_shared<slint::VectorModel<ContributorStruct>>(
                                    self.contributors));
                            self->set_contributors_status(Status::Normal);
                        }
                    });
            }
        });
}

void AboutPage::checkUpdate() {
    auto& self = *this;

    self->set_check_update_status(Status::Loading);

    executor()->asyncExecute(
        networkClient()->getLatestRelease(), [&self = *this](Result<ReleaseEntity> result) {
            if (result.isErr()) {
                self->set_check_update_status(Status::Normal);
                self.bridge.getMessageManager().showMessage(result.unwrapErr().what(),
                                                            MessageType::Error);
                return;
            }

            auto entity = result.unwrap();

            self->set_check_update_status(Status::Normal);

            if (std::string_view(VERSION_SEMANTIC).starts_with(entity.tag_name)) {
                self.bridge.getMessageManager().showMessage("已是最新版本");
                return;
            }

            self->set_update_log(slint::SharedString(entity.name + "\n" + entity.body));
            self->set_show_popup(true);
        });
}

EVENTO_UI_END