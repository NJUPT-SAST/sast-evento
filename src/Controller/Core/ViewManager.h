#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hpp>
#include <Controller/Core/UiBase.h>
#include <memory>
#include <set>
#include <stack>
#include <string>

EVENTO_UI_START

class ViewManager : public GlobalAgent<::ViewManager> {
    slint::ComponentHandle<UiEntryName> uiEntry;
    std::unordered_map<ViewName, std::shared_ptr<BasicView>> views;
    bool inEventLoop = false;
    std::stack<ViewName> viewStack;
    std::set<ViewName> visibleViews;

public:
    ViewManager(slint::ComponentHandle<UiEntryName> uiEntry);
    ViewManager(ViewManager&) = delete;
    ~ViewManager();

    // register view to manager to auto-call on* functions
    void attach(ViewName name, std::shared_ptr<BasicView> object);

    void show();
    void run();
    void hide();
    // called from other thread. for tray
    [[deprecated]] void exit();

    static std::string getViewName(ViewName target);
    static bool isOverlay(ViewName target);

    // view will be added to stack and show on top, overlay won't hide prior view.
    // auto refresh
    void pushView(ViewName newView);
    // view will be pop from stack, new top will be show if it not.
    // auto refresh
    void popView();
    void cleanStack();

private:
    void showView(ViewName target);
    void hideView(ViewName target);
    bool isViewShow(ViewName target);
    void refreshShowCache();

    void onEnterEventLoop();
    void onExitEventLoop();

    class StylishLog {
    public:
        static void actionTriggered(std::string actionName, std::string viewName = "All View");
        static void viewVisibilityChanged(std::string actionName, std::string viewName);
    };

    using Action = std::function<void(BasicView&)>;
    void call(Action& action);
    void call(Action& action, ViewName target);
    struct actions {
        static inline Action onCreate = [](BasicView& view) { view.onCreate(); };
        static inline Action onStart = [](BasicView& view) { view.onStart(); };
        // static inline Action onLogin = [](BasicView& view) { view.onLogin(); };
        static inline Action onShow = [](BasicView& view) { view.onShow(); };
        static inline Action onHide = [](BasicView& view) { view.onHide(); };
        // static inline Action onLogout = [](BasicView& view) { view.onLogout(); };
        static inline Action onStop = [](BasicView& view) { view.onStop(); };
        static inline Action onDestroy = [](BasicView& view) { view.onDestroy(); };
    };
};

EVENTO_UI_END