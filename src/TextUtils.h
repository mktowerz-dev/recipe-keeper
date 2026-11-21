#pragma once
#include <string>
#include <vector>

// Small text helpers used by both the recipe logic and the UI.

// Returns a copy of text with every letter made lowercase ("Chicken" -> "chicken").
std::string toLower(const std::string& text);

// Returns a copy of text without spaces/tabs/newlines at the start and end.
std::string trim(const std::string& text);

// Splits text into lines, trims each one, and skips blank lines.
// "2 eggs\n\n1 cup milk" -> {"2 eggs", "1 cup milk"}
std::vector<std::string> splitLines(const std::string& text);
