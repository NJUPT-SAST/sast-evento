#include <Controller/Core/UiUtility.h>
#include <Controller/Core/ViewManager.h>
#include <Controller/UiBridge.h>
#include <algorithm>
#include <cassert>
#include <memory>
#include <set>
#include <spdlog/spdlog.h>

EVENTO_UI_START

ViewManager::ViewManager(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : GlobalAgent(uiEntry)
    , bridge(bridge) {
    auto& self = *this;

    self->on_navigate_to([this](ViewName newView) { return navigateTo(newView); });
    self->on_clean_navigate_to([this](ViewName newView) { return cleanNavigateTo(newView); });
    self->on_replace_navigate_to([this](ViewName newView) { return replaceNavigateTo(newView); });
    self->on_prior_view([this]() { return priorView(); });

    self->on_is_show([&self](ViewName target) {
        // used to trigger re-calculate
        self->get_refresh();
        return self.isVisible(target);
    });
}

void ViewManager::initStack(ViewName newView, std::any data) {
    assert(!bridge.inEventLoop());
    pushView(newView, std::move(data));
    // separate operation reduce (possible) flicking when window start up and keeps invoke order
    // visibleViews.emplace(newView);
    static bool scheduleSync = false;
    if (!scheduleSync) {
        scheduleSync = true;
        slint::invoke_from_event_loop([this] { return syncViewVisibility(); });
    }
}

void ViewManager::navigateTo(ViewName newView, std::any data) {
    auto& self = *this;
    navAssert();
    if (newView == viewStack.top()) {
        return;
    }

    pushView(newView, std::move(data));

    syncViewVisibility();
}

void ViewManager::cleanNavigateTo(ViewName newView, std::any data) {
    auto& self = *this;
    navAssert();
    if (newView == viewStack.top()) {
        return;
    }

    while (viewStack.size() > 1) {
        popView();
    }
    if (newView == viewStack.top()) {
        syncViewVisibility();
        return;
    }
    pushView(newView, std::move(data));

    syncViewVisibility();
}

void ViewManager::replaceNavigateTo(ViewName newView, std::any data) {
    auto& self = *this;
    navAssert();
    if (newView == viewStack.top()) {
        return;
    }

    popView();
    pushView(newView, std::move(data));

    syncViewVisibility();
}

void ViewManager::priorView() {
    auto& self = *this;
    navAssert();

    if (viewStack.size() <= 1) {
        spdlog::debug("ViewManager: pop action canceled: only one view left");
        return;
    }

    popView();

    syncViewVisibility();
}

bool ViewManager::isVisible(ViewName target) {
    return visibleViews.find(target) != visibleViews.end();
}

void ViewManager::pushView(ViewName newView, std::any&& data) {
    viewStack.push(newView);
    viewData.emplace(std::move(data));
}

void ViewManager::popView() {
    viewStack.pop();
    viewData.pop();
}

void ViewManager::syncViewVisibility() {
    static std::set<ViewName> newVisibleViews;
    auto& self = *this;

    // generate newVisibleViews from viewStack
    newVisibleViews.clear();
    auto stack = viewStack;
    bool meetFirstOpaque = false;
    while (!meetFirstOpaque) {
        newVisibleViews.emplace(stack.top());
        meetFirstOpaque = !UiUtility::isTransparent(stack.top());
        stack.pop();
    }

    // diff newVisibleViews with visibleViews
    static std::set<ViewName> totalViews;
    totalViews.clear();
    totalViews.merge(std::set{newVisibleViews});
    totalViews.merge(std::set{visibleViews});
    std::for_each(totalViews.begin(), totalViews.end(), [&, this](ViewName item) {
        bool inNew = newVisibleViews.find(item) != newVisibleViews.end();
        bool inOld = visibleViews.find(item) != visibleViews.end();
        if (!inOld && inNew) {
            showView(item);
        }
        if (inOld && !inNew) {
            hideView(item);
        }
    });

    // request re-calculate is-show()
    self->set_refresh(!self->get_refresh());
}

void ViewManager::showView(ViewName target) {
    auto& self = *this;

    UiUtility::StylishLog::viewVisibilityChanged(logOrigin, "show", UiUtility::getViewName(target));
    visibleViews.emplace(target);
    UiUtility::StylishLog::viewActionTriggered(logOrigin, "onShow", UiUtility::getViewName(target));
    bridge.call(bridge.actions.onShow, target);
}

void ViewManager::hideView(ViewName target) {
    auto& self = *this;

    UiUtility::StylishLog::viewActionTriggered(logOrigin, "onHide", UiUtility::getViewName(target));
    bridge.call(bridge.actions.onHide, target);
    UiUtility::StylishLog::viewVisibilityChanged(logOrigin, "hide", UiUtility::getViewName(target));
    visibleViews.erase(target);
}

void ViewManager::onEnterEventLoop() {}

void ViewManager::onExitEventLoop() {
    // clean up
    spdlog::debug("ViewManager: --- onExitEventLoop: clean up all pages ---");
    while (viewStack.size() > 0) {
        UiUtility::UiUtility::StylishLog::viewActionTriggered(logOrigin,
                                                              "onHide",
                                                              UiUtility::getViewName(
                                                                  viewStack.top()));
        bridge.call(bridge.actions.onHide, viewStack.top());
        viewStack.pop();
    }
    visibleViews.clear();
}

void ViewManager::navAssert() {
    assert(viewStack.size());     // must use initStack() to add at least one view
    assert(bridge.inEventLoop()); // should invoke when event loop running
}

EVENTO_UI_END