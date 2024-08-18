#pragma once

#include <Controller/AsyncExecutor.hh>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

template<typename GlobalName>
class GlobalAgent {
    slint::ComponentHandle<UiEntryName> uiEntry;

public:
    const GlobalName* operator->() { return &uiEntry->global<GlobalName>(); }

protected:
    GlobalAgent(slint::ComponentHandle<UiEntryName>& uiEntry)
        : uiEntry(uiEntry) {}
};

EVENTO_UI_END