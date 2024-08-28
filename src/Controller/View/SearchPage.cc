#include <Controller/AsyncExecutor.hh>
#include <Controller/View/SearchPage.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <ranges>
#include <slint.h>

EVENTO_UI_START

SearchPage::SearchPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void SearchPage::onCreate() {
    auto& self = *this;

    self->on_filter_department([&self = *this](slint::SharedString keyword) {
        auto isMatch = [&keyword](const std::string& item) {
            return item.find(keyword) != std::string::npos;
        };

        std::vector<slint::StandardListViewItem> results;

        for (auto const& result : self._departments | std::views::filter(isMatch)) {
            results.emplace_back(slint::SharedString(result));
        }

        auto model = std::make_shared<slint::VectorModel<slint::StandardListViewItem>>(results);

        self->set_department(model);
    });
}

void SearchPage::onShow() {
    auto& self = *this;

    // evento::executor()->asyncExecute(evento::networkClient()->getDepartmentList(),
    //                                  [&self = *this](Result<DepartmentEntityList> result) {

    //                                  });
}

EVENTO_UI_END
