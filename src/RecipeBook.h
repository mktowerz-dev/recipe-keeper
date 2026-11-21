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

    // Replaces the recipe at position index with updated (then re-sorts).
    // Returns false (and changes nothing) if index is out of range, the new name
    // is blank, or a *different* recipe already has that name.
    bool updateRecipe(int index, const Recipe& updated);

    // Removes the recipe at position index. Returns false if there isn't one.
    bool deleteRecipe(int index);

    // All recipes, sorted alphabetically ignoring capital letters.
    const std::vector<Recipe>& getRecipes() const;

    // Position of the recipe with this exact name (ignoring capitals),
    // or -1 if there isn't one.
    int findByName(const std::string& name) const;

    // Writes every recipe to a text file. Returns false if the file can't be written.
    bool saveToFile(const std::string& path) const;

    // Replaces the current recipes with the ones in the file.
    // Returns false if the file can't be opened (for example, the first time
    // the program runs, before anything has been saved).
    bool loadFromFile(const std::string& path);

private:
    std::vector<Recipe> recipes;

    void sortByName();
};
