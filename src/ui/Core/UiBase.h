#pragma once

#define EVENTO_UI_START namespace evento {
#define EVENTO_UI_END }

#include "app.h"   // IWYU pragma: export
#include <slint.h> // IWYU pragma: export

EVENTO_UI_START

using UiEntryName = App;

EVENTO_UI_END