#include "RecipeBook.h"
#include "TextUtils.h"
#include <algorithm>
#include <fstream>

bool RecipeBook::addRecipe(const Recipe& recipe)
{
    Recipe cleaned = recipe;
    cleaned.name = trim(cleaned.name);

    if (cleaned.name.empty())
        return false;                       // a recipe needs a name
    if (findByName(cleaned.name) != -1)
        return false;                       // no two recipes with the same name

    recipes.push_back(cleaned);
    sortByName();
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

// ---------------------------------------------------------------------------
// Saving and loading
//
// The save file is plain text, one piece of information per line, so you can
// open it in Notepad and read it:
//
//     RECIPE
//     NAME Pancakes
//     INGREDIENT 2 eggs
//     INGREDIENT 1 cup milk
//     STEP Mix everything together.
//     STEP Cook on a hot pan.
//     END
//
// Each line starts with a label that says what the rest of the line is.
// ---------------------------------------------------------------------------
bool RecipeBook::saveToFile(const std::string& path) const
{
    std::ofstream file(path);
    if (!file)
        return false;

    for (const Recipe& recipe : recipes)
    {
        file << "RECIPE\n";
        file << "NAME " << recipe.name << "\n";
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
    Recipe current;          // the recipe we're in the middle of reading
    std::string line;
    std::string value;

    while (std::getline(file, line))
    {
        if (line == "RECIPE")
            current = Recipe();                     // start a fresh, empty recipe
        else if (readField(line, "NAME", value))
            current.name = value;
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

// Returns true if recipe a should come before recipe b (A-Z, ignoring capitals).
static bool comesBefore(const Recipe& a, const Recipe& b)
{
    return toLower(a.name) < toLower(b.name);
}

void RecipeBook::sortByName()
{
    std::sort(recipes.begin(), recipes.end(), comesBefore);
}
