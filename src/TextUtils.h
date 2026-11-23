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

// The opposite of splitLines: puts each entry on its own line.
// {"2 eggs", "1 cup milk"} -> "2 eggs\n1 cup milk"
std::string joinLines(const std::vector<std::string>& lines);

// True if list has an entry equal to value, ignoring capital letters.
// containsIgnoreCase({"Breakfast"}, "breakfast") -> true
bool containsIgnoreCase(const std::vector<std::string>& list, const std::string& value);

// Sorts a list of words A-Z, ignoring capital letters.
void sortAlphabetically(std::vector<std::string>& list);
