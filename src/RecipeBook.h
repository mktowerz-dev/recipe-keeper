#pragma once
#include "Recipe.h"
#include <string>
#include <vector>

// Holds all of the recipes and categories, and keeps both sorted A-Z.
// This class has no GUI code, so it can be tested on its own.
class RecipeBook
{
public:
    // ----- Recipes -----

    // Adds a recipe. Returns false (and adds nothing) if the name is blank
    // or another recipe already has that name (capital letters don't matter).
    // Any category the recipe uses that doesn't exist yet is created.
    bool addRecipe(const Recipe& recipe);

    // Replaces the recipe at position index with updated (then re-sorts).
    // Returns false (and changes nothing) if index is out of range, the new name
    // is blank, or a *different* recipe already has that name.
    bool updateRecipe(int index, const Recipe& updated);

    // Removes the recipe at position index. Returns false if there isn't one.
    bool deleteRecipe(int index);

    // Marks the recipe at position index as a favorite (isFavorite = true)
    // or takes it off favorites (false). Returns false if there isn't one.
    bool setFavorite(int index, bool isFavorite);

    // All recipes, sorted alphabetically ignoring capital letters.
    const std::vector<Recipe>& getRecipes() const;

    // Position of the recipe with this exact name (ignoring capitals),
    // or -1 if there isn't one.
    int findByName(const std::string& name) const;

    // ----- Categories -----

    // Creates a new, empty category. Returns false if the name is blank
    // or the category already exists (capital letters don't matter).
    bool addCategory(const std::string& name);

    // Deletes a category and takes it off every recipe that was in it.
    // The recipes themselves are NOT deleted. Returns false if it doesn't exist.
    bool deleteCategory(const std::string& name);

    // All category names, sorted A-Z.
    const std::vector<std::string>& getCategories() const;

    // ----- Saving and loading -----

    // Writes every category and recipe to a text file.
    // Returns false if the file can't be written.
    bool saveToFile(const std::string& path) const;

    // Replaces the current recipes and categories with the ones in the file.
    // Returns false if the file can't be opened (for example, the first time
    // the program runs, before anything has been saved).
    bool loadFromFile(const std::string& path);

private:
    std::vector<Recipe> recipes;
    std::vector<std::string> categories;

    void sortByName();
    int findCategory(const std::string& name) const;
    void cleanUpCategories(Recipe& recipe);
};
