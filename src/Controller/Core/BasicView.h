#pragma once

#include <Controller/Core/AccountManager.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/MessageManager.h>
#include <Controller/Core/UiBase.h>
#include <Controller/Core/ViewManager.h>

EVENTO_UI_START

class BasicView : public std::enable_shared_from_this<BasicView> {
    friend class UiBridge;

protected:
    UiBridge& bridge;
    BasicView(UiBridge& bridge)
        : bridge(bridge) {}

public:
    BasicView(const BasicView&) = delete;
    BasicView& operator=(const BasicView&) = delete;
    BasicView(BasicView&&) = delete;
    BasicView& operator=(BasicView&&) = delete;

    virtual ~BasicView() = default;

private:
    // all fuctions will be called in ui thread,
    // by default, called when event loop running except onCreate and onDestroy

    // called before event loop start
    virtual void onCreate() {};

    // called when event loop begin to run, after it started
    virtual void onStart() {};

    // called when:
    // 1. user requested login success,
    // 2. accountManager resume from last login with valid refreshToken
    virtual void onLogin() {};

    // called when view begin to show, after view shown
    virtual void onShow() {};

    // called when view begin to hide, before view hidden
    virtual void onHide() {};

    // called when user request logout
    virtual void onLogout() {};

    // called when event loop begin to stop, before it stopped
    virtual void onStop() {};

    // called after event loop stoped
    virtual void onDestroy() {};
};

EVENTO_UI_END