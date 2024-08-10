#pragma once

#include <Controller/Core/GlobalAgent.hpp>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class BasicView : public std::enable_shared_from_this<BasicView> {
    friend class UiBridge;

protected:
    UiBridge& bridge;
    BasicView(UiBridge& bridge)
        : bridge(bridge) {}

public:
    virtual ~BasicView() = default;

private:
    virtual void onCreate() {};
    virtual void onStart() {};
    virtual void onLogin() {};
    virtual void onShow() {};
    virtual void onHide() {};
    virtual void onLogout() {};
    virtual void onStop() {};
    virtual void onDestroy() {};
};

EVENTO_UI_END