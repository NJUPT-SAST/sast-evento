#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hpp>
#include <Controller/Core/UiBase.h>
#include <set>
#include <stack>
#include <string>

EVENTO_UI_START

class ViewManager : public GlobalAgent<::ViewManager> {
    friend class UiBridge;
    UiBridge& bridge;
    std::string logOrigin = "ViewManager";
    std::stack<ViewName> viewStack;
    std::set<ViewName> visibleViews;

public:
    ViewManager(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    ViewManager(ViewManager&) = delete;

    // invoked before event loop running
    void initStack(ViewName newView);

    // view will be added to stack and show on top, overlay won't hide prior view.
    // auto refresh
    void navigateTo(ViewName newView);
    // stack will be clean (init view left), and push new view.
    // auto refresh
    void cleanNavigateTo(ViewName newView);
    // current page pop and new view pushed.
    // auto refresh
    void replaceNavigateTo(ViewName newView);
    // view will be pop from stack, new top will be show if it not.
    // auto refresh
    void priorView();

    // check view visibility
    bool isVisible(ViewName target);

private:
    // sync view visibility from viewStack to visibleViews.
    void syncViewVisibility();

    void showView(ViewName target);
    void hideView(ViewName target);

    // trigger
    void onEnterEventLoop();
    void onExitEventLoop();

    void navAssert();
};

EVENTO_UI_END