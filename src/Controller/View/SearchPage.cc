#include <Controller/AsyncExecutor.hh>
#include <Controller/Convert.h>
#include <Controller/UiBridge.h>
#include <Controller/View/SearchPage.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <ranges>

EVENTO_UI_START

SearchPage::SearchPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void SearchPage::onCreate() {
    auto& self = *this;

    self->on_filter_department([&self = *this](slint::SharedString keyword) {
        auto isMatch = [&keyword](slint::SharedString item) { return item.starts_with(keyword); };

        std::vector<slint::StandardListViewItem> results;

        for (auto const& result : self.departments | std::views::filter(isMatch)) {
            results.emplace_back(slint::StandardListViewItem{result});
        }

        auto model = std::make_shared<slint::VectorModel<slint::StandardListViewItem>>(results);

        self->set_department(model);
    });

    self->on_load_department_list([&self = *this] { self.loadDepartmentList(); });
    self->on_load_department_events([&self = *this](int page) { self.loadDepartmentEvents(page); });
}

void SearchPage::onShow() {
    loadDepartmentList();
}

void SearchPage::loadDepartmentList() {
    auto& self = *this;

    self->set_list_state(PageState::Loading);

    executor()->asyncExecute(
        networkClient()->getDepartmentList(), [&self = *this](Result<DepartmentEntityList> result) {
            if (result.isErr()) {
                self->set_list_state(PageState::Error);
                self.bridge.getMessageManager().showMessage(result.unwrapErr().what(),
                                                            MessageType::Error);
                return;
            }

            for (auto& entity : result.unwrap()) {
                self.departments.emplace_back(entity.name);
            }

            std::vector<slint::StandardListViewItem> model;

            model.reserve(self.departments.size());
            for (auto const& item : self.departments) {
                model.emplace_back(slint::StandardListViewItem{item});
            }

            self->set_department(
                std::make_shared<slint::VectorModel<slint::StandardListViewItem>>(model));

            if (!model.empty()) {
                self->set_current_department_index(0);
            }

            self->set_list_state(PageState::Normal);
        });
}

void SearchPage::loadDepartmentEvents(int page) {
    auto& self = *this;

    self->set_list_state(PageState::Loading);

    executor()->asyncExecute(
        networkClient()->getDepartmentEventList(std::string(self->get_current_department().text),
                                                page,
                                                self->get_page_size()),
        [&self = *this](Result<EventQueryRes> result) {
            if (result.isErr()) {
                self->set_events_state(PageState::Error);
                self.bridge.getMessageManager().showMessage(result.unwrapErr().what(),
                                                            MessageType::Error);
                return;
            }

            auto res = result.unwrap();

            self->set_total(res.total);

            self->set_event_model(convert::from(res.elements));

            self->set_events_state(PageState::Normal);
        });
}

EVENTO_UI_END
