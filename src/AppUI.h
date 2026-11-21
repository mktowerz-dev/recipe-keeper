#pragma once
#include "RecipeBook.h"
#include <string>

// Draws the whole app (recipe list, details, forms) for one frame.
// Called about 60 times a second from the main loop in main.cpp.
// saveFilePath is where recipes are saved whenever something changes.
void drawUI(RecipeBook& book, const std::string& saveFilePath);
