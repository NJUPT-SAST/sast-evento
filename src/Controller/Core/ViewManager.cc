#include <Controller/Core/ViewManager.h>
#include <spdlog/spdlog.h>

EVENTO_UI_START

ViewManager::ViewManager(slint::ComponentHandle<UiEntryName> uiEntry)
    : GlobalAgent(uiEntry)
    , uiEntry(uiEntry) {
    auto& self = *this;

    self->on_push([this](ViewName newView) { return pushView(newView); });
    self->on_pop([this] { return popView(); });
    self->on_is_show([&self](ViewName target) {
        // used to trigger re-calculate
        self->get_refresh();
        return self.isViewShow(target);
    });
    self->on_clean_push([this](ViewName newView) {
        cleanStack();
        pushView(newView);
    });
    uiEntry->window().on_close_requested([this] {
        onExitEventLoop();
        return slint::CloseRequestResponse::HideWindow;
    });
}

ViewManager::~ViewManager() {
    if (inEventLoop) {
        exit();
    }
};

void ViewManager::attach(ViewName name, std::unique_ptr<BasicView> object) {
    views.emplace(std::make_pair(name, std::move(object)));
}

void ViewManager::show() {
    uiEntry->show();
}

void ViewManager::run() {
    StylishLog::actionTriggered("onCreate");
    call(actions::onCreate);

    show();
    spdlog::debug("--- enter slint event loop ---");
    inEventLoop = true;
    slint::invoke_from_event_loop([this] { return onEnterEventLoop(); });
    slint::run_event_loop();
    inEventLoop = false;
    spdlog::debug("--- exit slint event loop ---");
    hide();

    StylishLog::actionTriggered("onDestroy");
    call(actions::onDestroy);
}

void ViewManager::hide() {
    uiEntry->hide();
}

void ViewManager::exit() {
    StylishLog::actionTriggered("onStop");
    slint::invoke_from_event_loop([&self = *this] { self.call(actions::onStop); });
    slint::quit_event_loop();
}

std::string ViewManager::getViewName(ViewName target) {
    static std::unordered_map<ViewName, std::string> mapper{
        {ViewName::DiscoveryPage, "DiscoveryPage"},
        {ViewName::SearchPage, "SearchPage"},
        {ViewName::HistoryPage, "HistoryPage"},
        {ViewName::MyEventPage, "MyEventPage"},
        {ViewName::DetailPage, "DetailPage"},
        {ViewName::AboutPage, "AboutPage"},
        {ViewName::SettingPage, "SettingPage"},
        {ViewName::LoginOverlay, "LoginOverlay"},
        {ViewName::MenuOverlay, "MenuOverlay"},
    };
    return mapper.find(target) == mapper.end() ? "[Unknown View]" : mapper.at(target);
}

bool ViewManager::isOverlay(ViewName target) {
    return overlayList.find(target) != overlayList.end();
}

void ViewManager::pushView(ViewName newView) {
    auto& self = *this;

    if (viewStack.size() != 0 && newView == viewStack.top()) {
        return;
    }

    if (!inEventLoop) {
        viewStack.push(newView);
        StylishLog::viewVisibilityChanged("show", getViewName(newView));
        visibleViews.emplace(newView);
        return;
    }

    if (!isOverlay(newView) && viewStack.size() != 0) {
        hideView(viewStack.top());
    }
    viewStack.push(newView);
    showView(newView);

    refreshShowCache();
}

void ViewManager::popView() {
    auto& self = *this;

    if (!inEventLoop) {
        StylishLog::viewVisibilityChanged("hide", getViewName(viewStack.top()));
        visibleViews.erase(viewStack.top());
        viewStack.pop();
        return;
    }

    if (viewStack.size() <= 1) {
        spdlog::error("ViewManager: failed to pop page stack when only one page left");
        return;
    }

    hideView(viewStack.top());
    viewStack.pop();
    if (!isViewShow(viewStack.top())) {
        showView(viewStack.top());
    }

    refreshShowCache();
}

void ViewManager::cleanStack() {
    while (viewStack.size() > 1) {
        popView();
    }
}

void ViewManager::showView(ViewName target) {
    auto& self = *this;

    StylishLog::viewVisibilityChanged("show", getViewName(target));
    visibleViews.emplace(target);
    StylishLog::actionTriggered("onShow", getViewName(target));
    call(actions::onShow, target);
}

void ViewManager::hideView(ViewName target) {
    auto& self = *this;

    StylishLog::actionTriggered("onHide", getViewName(target));
    call(actions::onHide, target);
    StylishLog::viewVisibilityChanged("hide", getViewName(target));
    visibleViews.erase(target);
}

bool ViewManager::isViewShow(ViewName target) {
    return visibleViews.find(target) != visibleViews.end();
}

void ViewManager::refreshShowCache() {
    auto& self = *this;

    self->set_refresh(!self->get_refresh());
}

void ViewManager::onEnterEventLoop() {
    auto& self = *this;

    spdlog::debug("ViewManager: (internal) enter_event_loop triggered");

    StylishLog::actionTriggered("onStart");
    self.call(actions::onStart);

    auto stack = viewStack;
    bool meetFirstPage = false;
    while (stack.size() > 0 && !meetFirstPage) {
        StylishLog::actionTriggered("onShow", getViewName(stack.top()));
        call(actions::onShow, stack.top());
        meetFirstPage = !isOverlay(stack.top());
        stack.pop();
    }
}

void ViewManager::onExitEventLoop() {
    auto& self = *this;

    // clean up
    while (viewStack.size() > 0) {
        StylishLog::actionTriggered("onHide", getViewName(viewStack.top()));
        call(actions::onHide, viewStack.top());
        viewStack.pop();
    }
    visibleViews.clear();

    StylishLog::actionTriggered("onStop");
    call(actions::onStop);
}

void ViewManager::StylishLog::actionTriggered(std::string actionName, std::string viewName) {
    spdlog::debug("ViewManager: [{}] triggered [{}]", viewName, actionName);
}

void ViewManager::StylishLog::viewVisibilityChanged(std::string actionName, std::string viewName) {
    spdlog::debug("ViewManager: [{}] [{}]", actionName, viewName);
}

void ViewManager::call(Action& action) {
    std::for_each(views.begin(),
                  views.end(),
                  [&action](const std::pair<const ViewName, std::unique_ptr<BasicView>>& view) {
                      action(*view.second.get());
                  });
}

void ViewManager::call(Action& action, ViewName target) {
    action(*(views.at(target).get()));
}

EVENTO_UI_END