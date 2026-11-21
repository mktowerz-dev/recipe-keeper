#pragma once
#include <string>
#include <vector>

// One recipe: its name, what goes in it, and how to make it.
struct Recipe
{
    std::string name;
    std::vector<std::string> ingredients;  // one entry per ingredient, e.g. "2 cups flour"
    std::vector<std::string> steps;        // in order: steps[0] is step 1
};
