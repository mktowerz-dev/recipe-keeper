// Automated tests for the recipe logic (no window needed).
// Run: build\Debug\RecipeTests.exe
// Each check prints PASS or FAIL; the program returns 1 if anything failed.

#include "RecipeBook.h"
#include "TextUtils.h"
#include <cstdio>
#include <iostream>
#include <string>

static int failures = 0;

// Prints the result of one check and counts failures.
static void check(bool condition, const std::string& description)
{
    if (condition)
    {
        std::cout << "PASS  " << description << "\n";
    }
    else
    {
        std::cout << "FAIL  " << description << "\n";
        failures++;
    }
}

static Recipe makeRecipe(const std::string& name)
{
    Recipe recipe;
    recipe.name = name;
    return recipe;
}

static void testTextUtils()
{
    check(toLower("Chicken ALFREDO") == "chicken alfredo", "toLower makes letters lowercase");
    check(trim("   pancakes \n") == "pancakes", "trim removes spaces at both ends");
    check(trim("   ").empty(), "trim of only spaces is empty");

    std::vector<std::string> lines = splitLines("2 eggs\n\n  1 cup milk  \n");
    check(lines.size() == 2, "splitLines skips blank lines");
    check(lines.size() == 2 && lines[1] == "1 cup milk", "splitLines trims each line");
}

static void testAddRecipe()
{
    RecipeBook book;
    check(book.addRecipe(makeRecipe("Pancakes")), "adding a recipe works");
    check(book.getRecipes().size() == 1, "book has 1 recipe after adding");

    check(!book.addRecipe(makeRecipe("")), "blank name is rejected");
    check(!book.addRecipe(makeRecipe("   ")), "name of only spaces is rejected");
    check(!book.addRecipe(makeRecipe("PANCAKES")), "duplicate name (different capitals) is rejected");
    check(book.getRecipes().size() == 1, "rejected recipes were not added");

    book.addRecipe(makeRecipe("  Waffles  "));
    check(book.findByName("waffles") != -1, "name is trimmed when added");
}

static void testSortedOrder()
{
    RecipeBook book;
    book.addRecipe(makeRecipe("waffles"));
    book.addRecipe(makeRecipe("Apple Pie"));
    book.addRecipe(makeRecipe("brownies"));

    const std::vector<Recipe>& recipes = book.getRecipes();
    check(recipes[0].name == "Apple Pie" &&
          recipes[1].name == "brownies" &&
          recipes[2].name == "waffles",
          "recipes are sorted A-Z ignoring capitals");
}

static void testSaveAndLoad()
{
    const std::string testFile = "test_recipes.txt";

    RecipeBook original;
    Recipe pancakes = makeRecipe("Pancakes");
    pancakes.ingredients = { "2 eggs", "1 cup milk" };
    pancakes.steps = { "Mix everything.", "Cook on a hot pan." };
    original.addRecipe(pancakes);
    original.addRecipe(makeRecipe("Brownies"));  // a recipe with no ingredients or steps

    check(original.saveToFile(testFile), "saving to a file works");

    RecipeBook loaded;
    check(loaded.loadFromFile(testFile), "loading the file works");
    check(loaded.getRecipes().size() == 2, "loaded the same number of recipes");

    int index = loaded.findByName("Pancakes");
    check(index != -1, "loaded recipe can be found by name");
    if (index != -1)
    {
        const Recipe& recipe = loaded.getRecipes()[index];
        check(recipe.ingredients.size() == 2 && recipe.ingredients[1] == "1 cup milk",
              "ingredients survive save + load");
        check(recipe.steps.size() == 2 && recipe.steps[0] == "Mix everything.",
              "steps survive save + load in order");
    }

    std::remove(testFile.c_str());  // clean up the test file

    RecipeBook empty;
    check(!empty.loadFromFile("this_file_does_not_exist.txt"),
          "loading a missing file returns false");
    check(empty.getRecipes().empty(), "a missing file leaves the book empty");
}

int main()
{
    testTextUtils();
    testAddRecipe();
    testSortedOrder();
    testSaveAndLoad();

    std::cout << "\n" << (failures == 0 ? "All tests passed." : "Some tests FAILED.") << "\n";
    return failures == 0 ? 0 : 1;
}
