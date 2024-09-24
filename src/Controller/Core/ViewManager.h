#pragma once

#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>
#include <any>
#include <cassert>
#include <set>
#include <stack>
#include <string>

EVENTO_UI_START

// adding a new page should do:
// - push to viewStack
// - re-calculate visibleViews
// - add data to viewData
// - call onShow
//
// pop a page should do:
// - pop viewStack
// - re-calculate visibleViews
// - pop data to current index(after pop)
// - call onHide/Show
class ViewManager : private GlobalAgent<ViewManagerBridge> {
    friend class UiBridge;
    UiBridge& bridge;
    std::string logOrigin = "ViewManager";

    std::stack<ViewName> viewStack;
    std::stack<std::any> viewData;

    std::set<ViewName> visibleViews;

public:
    ViewManager(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    ViewManager(ViewManager&) = delete;

    // invoked before event loop running
    void initStack(ViewName newView, std::any data = {});

    // view will be added to stack and show on top, overlay won't hide prior view.
    void navigateTo(ViewName newView, std::any data = {});

    // stack will be clean (init view left), and push new view.
    void cleanNavigateTo(ViewName newView, std::any data = {});

    // current page pop and new view pushed.
    void replaceNavigateTo(ViewName newView, std::any data = {});

    // view will be pop from stack, new top will be show if it not.
    void priorView();

    // check view visibility
    bool isVisible(ViewName target);

    /**
    * get data corresponding to current view, set by all navigate function,
    * useful when needed to transfer data between view or save data for current view in case of a view opened twice,
    * pop view will destroy its data, data type wrapped by std::any
    * 
    * @param T(template argument) target view data type
    * @example getDate<MyDataType>();
    */
    template<typename T>
    T getData() {
        assert(!viewData.empty() && "no view data");
        return std::any_cast<T>(viewData.top());
    }

private:
    void pushView(ViewName newView, std::any&& data);
    void popView();

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