#include "RecipeBook.h"
#include "TextUtils.h"
#include <algorithm>

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

// Returns true if recipe a should come before recipe b (A-Z, ignoring capitals).
static bool comesBefore(const Recipe& a, const Recipe& b)
{
    return toLower(a.name) < toLower(b.name);
}

void RecipeBook::sortByName()
{
    std::sort(recipes.begin(), recipes.end(), comesBefore);
}
