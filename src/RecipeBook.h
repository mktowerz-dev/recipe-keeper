#pragma once
#include "Recipe.h"
#include <string>
#include <vector>

// Holds all of the recipes and keeps them sorted A-Z by name.
// This class has no GUI code, so it can be tested on its own.
class RecipeBook
{
public:
    // Adds a recipe. Returns false (and adds nothing) if the name is blank
    // or another recipe already has that name (capital letters don't matter).
    bool addRecipe(const Recipe& recipe);

    // All recipes, sorted alphabetically ignoring capital letters.
    const std::vector<Recipe>& getRecipes() const;

    // Position of the recipe with this exact name (ignoring capitals),
    // or -1 if there isn't one.
    int findByName(const std::string& name) const;

private:
    std::vector<Recipe> recipes;

    void sortByName();
};
