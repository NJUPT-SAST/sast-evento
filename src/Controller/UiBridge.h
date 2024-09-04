#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class UiBridge : GlobalAgent<::UiBridge> {
    friend class ViewManager;
    friend class AccountManager;
    friend class MessageManager;

    slint::ComponentHandle<UiEntryName> uiEntry;
    bool eventLoopRunning = false;

    // all views
    std::unordered_map<ViewName, std::shared_ptr<BasicView>> views;

    // Managers
    std::shared_ptr<ViewManager> viewManager;
    std::shared_ptr<AccountManager> accountManager;
    std::shared_ptr<MessageManager> messageManager;

    std::string logOrigin = "UiBridge";

public:
    UiBridge(slint::ComponentHandle<UiEntryName> uiEntry);
    UiBridge(UiBridge&) = delete;

    // attach view for on* functions and manager access
    void attachView(ViewName name, std::shared_ptr<BasicView> object);

    // getter
    ViewManager& getViewManager();
    AccountManager& getAccountManager();
    MessageManager& getMessageManager();
    [[deprecated("will lead to unknown behavior")]] slint::ComponentHandle<UiEntryName> getUiEntry();

    template<typename T>
    T& getView(ViewName target) {
        return dynamic_cast<T&>(*views.at(target));
    }

    // same as Slint show();
    void show();
    // will show(), run event loop, then hide()
    void run();
    // same as Slint hide();
    void hide();
    // called from other thread (for tray)
    void exit();

    bool inEventLoop() const;

private:
    void attachAllViews();

    void onEnterEventLoop();
    void onExitEventLoop();

    using Action = std::function<void(BasicView&)>;
    void call(Action& action);
    void call(Action& action, ViewName target);

    static struct actions {
        static inline Action onCreate = [](BasicView& view) { view.onCreate(); };
        static inline Action onStart = [](BasicView& view) { view.onStart(); };
        static inline Action onLogin = [](BasicView& view) { view.onLogin(); };
        static inline Action onShow = [](BasicView& view) { view.onShow(); };
        static inline Action onHide = [](BasicView& view) { view.onHide(); };
        static inline Action onLogout = [](BasicView& view) { view.onLogout(); };
        static inline Action onStop = [](BasicView& view) { view.onStop(); };
        static inline Action onDestroy = [](BasicView& view) { view.onDestroy(); };
    } actions;
};

EVENTO_UI_END