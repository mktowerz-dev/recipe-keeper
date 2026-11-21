#pragma once
#include "RecipeBook.h"

// Draws the whole app (recipe list, details, forms) for one frame.
// Called about 60 times a second from the main loop in main.cpp.
void drawUI(RecipeBook& book);
