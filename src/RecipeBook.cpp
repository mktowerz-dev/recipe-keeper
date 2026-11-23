#include "RecipeBook.h"
#include "TextUtils.h"
#include <algorithm>
#include <fstream>

// ===========================================================================
// Recipes
// ===========================================================================

bool RecipeBook::addRecipe(const Recipe& recipe)
{
    Recipe cleaned = recipe;
    cleaned.name = trim(cleaned.name);

    if (cleaned.name.empty())
        return false;                       // a recipe needs a name
    if (findByName(cleaned.name) != -1)
        return false;                       // no two recipes with the same name

    cleanUpCategories(cleaned);
    recipes.push_back(cleaned);
    sortByName();
    return true;
}

bool RecipeBook::updateRecipe(int index, const Recipe& updated)
{
    if (index < 0 || index >= static_cast<int>(recipes.size()))
        return false;                       // no recipe at that position

    Recipe cleaned = updated;
    cleaned.name = trim(cleaned.name);
    if (cleaned.name.empty())
        return false;

    // It's fine if the name matches the recipe we're editing (e.g. only the
    // ingredients changed), but not if it matches a different recipe.
    int existing = findByName(cleaned.name);
    if (existing != -1 && existing != index)
        return false;

    cleanUpCategories(cleaned);
    recipes[index] = cleaned;
    sortByName();                           // the name may have changed
    return true;
}

bool RecipeBook::deleteRecipe(int index)
{
    if (index < 0 || index >= static_cast<int>(recipes.size()))
        return false;

    // erase() needs an iterator (a "pointer" into the vector), so we start at
    // begin() and move forward 'index' spots. The list stays sorted.
    recipes.erase(recipes.begin() + index);
    return true;
}

const std::vector<Recipe>& RecipeBook::getRecipes() const
{
    return recipes;
}

int RecipeBook::findByName(const std::string& name) const
{
    std::string target = toLower(trim(name));
    for (size_t i = 0; i < recipes.size(); i++)
    {
        if (toLower(recipes[i].name) == target)
            return static_cast<int>(i);
    }
    return -1;
}

// Returns true if recipe a should come before recipe b (A-Z, ignoring capitals).
static bool comesBefore(const Recipe& a, const Recipe& b)
{
    return toLower(a.name) < toLower(b.name);
}

void RecipeBook::sortByName()
{
    std::sort(recipes.begin(), recipes.end(), comesBefore);
}

// ===========================================================================
// Categories
// ===========================================================================

bool RecipeBook::addCategory(const std::string& name)
{
    std::string cleaned = trim(name);
    if (cleaned.empty() || findCategory(cleaned) != -1)
        return false;

    categories.push_back(cleaned);
    sortAlphabetically(categories);
    return true;
}

bool RecipeBook::deleteCategory(const std::string& name)
{
    int index = findCategory(name);
    if (index == -1)
        return false;

    std::string target = toLower(categories[index]);
    categories.erase(categories.begin() + index);

    // Take the category off every recipe. We go through each recipe's list
    // backwards so erasing an entry doesn't make us skip the next one.
    for (Recipe& recipe : recipes)
    {
        for (int i = static_cast<int>(recipe.categories.size()) - 1; i >= 0; i--)
        {
            if (toLower(recipe.categories[i]) == target)
                recipe.categories.erase(recipe.categories.begin() + i);
        }
    }
    return true;
}

const std::vector<std::string>& RecipeBook::getCategories() const
{
    return categories;
}

// Position of the category with this name (ignoring capitals), or -1.
int RecipeBook::findCategory(const std::string& name) const
{
    std::string target = toLower(trim(name));
    for (size_t i = 0; i < categories.size(); i++)
    {
        if (toLower(categories[i]) == target)
            return static_cast<int>(i);
    }
    return -1;
}

// Tidies up a recipe's category list before it's stored:
//  - removes blanks and duplicates
//  - creates any category that doesn't exist yet
//  - uses the book's spelling ("breakfast" becomes "Breakfast")
//  - sorts the list A-Z
void RecipeBook::cleanUpCategories(Recipe& recipe)
{
    std::vector<std::string> cleaned;
    for (const std::string& raw : recipe.categories)
    {
        std::string name = trim(raw);
        if (name.empty() || containsIgnoreCase(cleaned, name))
            continue;                       // skip blanks and repeats

        addCategory(name);                  // does nothing if it already exists
        cleaned.push_back(categories[findCategory(name)]);
    }
    sortAlphabetically(cleaned);
    recipe.categories = cleaned;
}

// ===========================================================================
// Saving and loading
//
// The save file is plain text, one piece of information per line, so you can
// open it in Notepad and read it. Categories are listed first, then recipes:
//
//     CATEGORY Breakfast
//     CATEGORY Desserts
//
//     RECIPE
//     NAME Pancakes
//     IN_CATEGORY Breakfast
//     INGREDIENT 2 eggs
//     STEP Mix everything together.
//     END
//
// Each line starts with a label that says what the rest of the line is.
// ===========================================================================
bool RecipeBook::saveToFile(const std::string& path) const
{
    std::ofstream file(path);
    if (!file)
        return false;

    for (const std::string& category : categories)
        file << "CATEGORY " << category << "\n";
    file << "\n";

    for (const Recipe& recipe : recipes)
    {
        file << "RECIPE\n";
        file << "NAME " << recipe.name << "\n";
        for (const std::string& category : recipe.categories)
            file << "IN_CATEGORY " << category << "\n";
        for (const std::string& ingredient : recipe.ingredients)
            file << "INGREDIENT " << ingredient << "\n";
        for (const std::string& step : recipe.steps)
            file << "STEP " << step << "\n";
        file << "END\n\n";
    }
    return static_cast<bool>(file);  // false if something went wrong while writing
}

// If line starts with label + a space, puts the rest of the line in value and
// returns true. Example: readField("NAME Pancakes", "NAME", value) -> value = "Pancakes"
static bool readField(const std::string& line, const std::string& label, std::string& value)
{
    std::string prefix = label + " ";
    if (line.compare(0, prefix.size(), prefix) != 0)
        return false;
    value = line.substr(prefix.size());
    return true;
}

bool RecipeBook::loadFromFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
        return false;

    recipes.clear();
    categories.clear();
    Recipe current;          // the recipe we're in the middle of reading
    std::string line;
    std::string value;

    while (std::getline(file, line))
    {
        if (readField(line, "CATEGORY", value))
            addCategory(value);
        else if (line == "RECIPE")
            current = Recipe();                     // start a fresh, empty recipe
        else if (readField(line, "NAME", value))
            current.name = value;
        else if (readField(line, "IN_CATEGORY", value))
            current.categories.push_back(value);
        else if (readField(line, "INGREDIENT", value))
            current.ingredients.push_back(value);
        else if (readField(line, "STEP", value))
            current.steps.push_back(value);
        else if (line == "END")
            addRecipe(current);                     // also sorts and skips bad/duplicate names
        // Anything else (like blank lines) is ignored.
    }
    return true;
}
