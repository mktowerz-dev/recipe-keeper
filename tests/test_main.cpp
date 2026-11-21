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

    check(joinLines({ "2 eggs", "1 cup milk" }) == "2 eggs\n1 cup milk",
          "joinLines puts each entry on its own line");
    check(splitLines(joinLines(lines)) == lines, "joinLines then splitLines gives back the same list");
}

static void testUpdateRecipe()
{
    RecipeBook book;
    book.addRecipe(makeRecipe("Pancakes"));
    book.addRecipe(makeRecipe("Waffles"));

    // Change only the ingredients (same name) - allowed.
    Recipe changed = makeRecipe("Pancakes");
    changed.ingredients = { "3 eggs" };
    int index = book.findByName("Pancakes");
    check(book.updateRecipe(index, changed), "editing a recipe's ingredients works");
    check(book.getRecipes()[book.findByName("Pancakes")].ingredients.size() == 1,
          "edited ingredients were saved");

    // Rename "Waffles" to "Belgian Waffles" - the list should re-sort.
    check(book.updateRecipe(book.findByName("Waffles"), makeRecipe("Belgian Waffles")),
          "renaming a recipe works");
    check(book.getRecipes()[0].name == "Belgian Waffles", "renamed recipe is re-sorted A-Z");
    check(book.findByName("Waffles") == -1, "old name is gone after renaming");

    // Only changing capitals of its own name is fine.
    check(book.updateRecipe(book.findByName("Pancakes"), makeRecipe("PANCAKES")),
          "changing only the capitals of a name works");

    // Problems that should be rejected.
    check(!book.updateRecipe(book.findByName("PANCAKES"), makeRecipe("belgian waffles")),
          "renaming to another recipe's name is rejected");
    check(!book.updateRecipe(0, makeRecipe("   ")), "renaming to a blank name is rejected");
    check(!book.updateRecipe(99, makeRecipe("Nope")), "editing a position that doesn't exist is rejected");
    check(book.getRecipes().size() == 2, "rejected edits didn't add or remove recipes");
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

static void testDeleteRecipe()
{
    RecipeBook book;
    book.addRecipe(makeRecipe("Apple Pie"));
    book.addRecipe(makeRecipe("Brownies"));
    book.addRecipe(makeRecipe("Cookies"));

    check(book.deleteRecipe(book.findByName("Brownies")), "deleting a recipe works");
    check(book.getRecipes().size() == 2, "book has one fewer recipe after deleting");
    check(book.findByName("Brownies") == -1, "deleted recipe can't be found anymore");
    check(book.getRecipes()[0].name == "Apple Pie" && book.getRecipes()[1].name == "Cookies",
          "the other recipes are still there, still A-Z");

    check(!book.deleteRecipe(-1), "deleting position -1 is rejected");
    check(!book.deleteRecipe(5), "deleting a position that doesn't exist is rejected");
    check(book.getRecipes().size() == 2, "rejected deletes didn't remove anything");
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
    testUpdateRecipe();
    testDeleteRecipe();
    testSaveAndLoad();

    std::cout << "\n" << (failures == 0 ? "All tests passed." : "Some tests FAILED.") << "\n";
    return failures == 0 ? 0 : 1;
}
