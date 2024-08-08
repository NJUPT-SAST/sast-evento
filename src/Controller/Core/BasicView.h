#pragma once

#include <Controller/Core/GlobalAgent.hpp>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class ViewManager;

class BasicView {
    friend class ViewManager;

public:
    virtual ~BasicView() = default;

private:
    virtual void onCreate() {};
    virtual void onStart() {};
    // virtual void onLogin() {};
    virtual void onShow() {};
    virtual void onHide() {};
    // virtual void onLogout() {};
    virtual void onStop() {};
    virtual void onDestroy() {};
};

EVENTO_UI_END