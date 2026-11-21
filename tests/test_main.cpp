// Automated tests for the recipe logic (no window needed).
// Run: build\Debug\RecipeTests.exe
// Each check prints PASS or FAIL; the program returns 1 if anything failed.

#include "RecipeBook.h"
#include "TextUtils.h"
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

int main()
{
    testTextUtils();
    testAddRecipe();
    testSortedOrder();

    std::cout << "\n" << (failures == 0 ? "All tests passed." : "Some tests FAILED.") << "\n";
    return failures == 0 ? 0 : 1;
}
