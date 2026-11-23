#pragma once
#include <string>
#include <vector>

// One recipe: its name, what goes in it, how to make it, and how it's organized.
struct Recipe
{
    std::string name;
    std::vector<std::string> ingredients;  // one entry per ingredient, e.g. "2 cups flour"
    std::vector<std::string> steps;        // in order: steps[0] is step 1
    std::vector<std::string> categories;   // e.g. {"Breakfast", "Desserts"}; can be empty
};
