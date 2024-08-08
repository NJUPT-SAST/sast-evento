#pragma once

#include <Controller/Core/UiBase.h>
#include <Controller/Core/ViewManager.h>

EVENTO_UI_START

class UiBridge {
    ViewManager manager;

public:
    UiBridge(slint::ComponentHandle<UiEntryName>& uiEntry);
    ViewManager* operator->();
};

EVENTO_UI_END