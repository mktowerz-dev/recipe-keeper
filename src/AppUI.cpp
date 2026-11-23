#include "AppUI.h"
#include "TextUtils.h"
#include "imgui.h"
#include "imgui_stdlib.h"   // lets ImGui::InputText work with std::string
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Screen state. These are 'static' so they keep their values between frames
// (drawUI runs ~60 times a second) and are only visible inside this file.
// ---------------------------------------------------------------------------

// What the right side of the window is showing.
enum class Screen
{
    Recipe,      // the selected recipe (or a hint if nothing is selected)
    Form,        // the add/edit recipe form
    Categories   // the "Manage Categories" screen
};
static Screen screen = Screen::Recipe;

static int selectedIndex = -1;          // which recipe is selected in the list (-1 = none)
static int editingIndex  = -1;          // recipe being edited, or -1 when adding a new one
static std::string openCategory;        // category opened in the Categories tab ("" = show the list of categories)

// What the user is typing into the add/edit form.
static std::string formName;
static std::string formIngredients;     // one ingredient per line
static std::string formSteps;           // one step per line
static std::vector<std::string> formCategories;  // categories that are checked
static std::string formError;           // message shown if the form can't be saved

// The "Manage Categories" screen.
static std::string newCategoryName;
static std::string categoryError;
static std::string categoryToDelete;    // which category the delete popup is asking about

static std::string saveError;           // message shown if writing the save file failed

static const ImVec4 errorColor(0.8f, 0.1f, 0.1f, 1.0f);

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------

static void clearForm()
{
    formName.clear();
    formIngredients.clear();
    formSteps.clear();
    formCategories.clear();
    formError.clear();
}

static void closeForm()
{
    screen = Screen::Recipe;
    editingIndex = -1;
    clearForm();
}

// Shows an empty form for a brand-new recipe.
static void openAddForm()
{
    clearForm();
    editingIndex = -1;
    selectedIndex = -1;
    screen = Screen::Form;
}

// Shows the form filled in with an existing recipe so it can be changed.
static void openEditForm(const RecipeBook& book, int index)
{
    const Recipe& recipe = book.getRecipes()[index];
    clearForm();
    formName = recipe.name;
    formIngredients = joinLines(recipe.ingredients);
    formSteps = joinLines(recipe.steps);
    formCategories = recipe.categories;
    editingIndex = index;
    screen = Screen::Form;
}

// Writes everything to the save file and remembers if it failed.
static void saveBook(const RecipeBook& book, const std::string& saveFilePath)
{
    if (book.saveToFile(saveFilePath))
        saveError.clear();
    else
        saveError = "Couldn't save to " + saveFilePath;
}

// Removes value from list (ignoring capital letters).
static void removeIgnoreCase(std::vector<std::string>& list, const std::string& value)
{
    for (int i = static_cast<int>(list.size()) - 1; i >= 0; i--)
    {
        if (toLower(list[i]) == toLower(value))
            list.erase(list.begin() + i);
    }
}

static int countRecipesIn(const RecipeBook& book, const std::string& category)
{
    int count = 0;
    for (const Recipe& recipe : book.getRecipes())
    {
        if (containsIgnoreCase(recipe.categories, category))
            count++;
    }
    return count;
}

// ---------------------------------------------------------------------------
// Left side: a clickable A-Z list of recipe names.
// If category is "", every recipe is listed; otherwise only that category's.
// ---------------------------------------------------------------------------
static void drawRecipeRows(const RecipeBook& book, const std::string& category)
{
    const std::vector<Recipe>& recipes = book.getRecipes();
    int shownCount = 0;
    for (int i = 0; i < static_cast<int>(recipes.size()); i++)
    {
        if (!category.empty() && !containsIgnoreCase(recipes[i].categories, category))
            continue;  // not in this category

        ImGui::PushID(i);  // gives each row a unique ID, even if two names look alike
        bool isSelected = (i == selectedIndex);
        if (ImGui::Selectable(recipes[i].name.c_str(), isSelected))
        {
            selectedIndex = i;
            closeForm();
        }
        ImGui::PopID();
        shownCount++;
    }

    if (recipes.empty())
        ImGui::TextDisabled("No recipes yet.");
    else if (shownCount == 0)
        ImGui::TextDisabled("No recipes in this category.");
}

// Categories tab: either the A-Z list of categories, or the recipes inside
// the one that was clicked (openCategory).
static void drawCategoriesTab(const RecipeBook& book)
{
    if (openCategory.empty())
    {
        if (ImGui::Button("Manage Categories", ImVec2(-1, 0)))
            screen = Screen::Categories;
        ImGui::Spacing();

        const std::vector<std::string>& allCategories = book.getCategories();
        if (allCategories.empty())
            ImGui::TextDisabled("No categories yet.");

        for (int i = 0; i < static_cast<int>(allCategories.size()); i++)
        {
            // Shows like "Desserts (2)"
            std::string label = allCategories[i] + " (" +
                                std::to_string(countRecipesIn(book, allCategories[i])) + ")";
            ImGui::PushID(i);
            if (ImGui::Selectable(label.c_str()))
                openCategory = allCategories[i];
            ImGui::PopID();
        }
    }
    else
    {
        if (ImGui::Button("< Back to Categories", ImVec2(-1, 0)))
            openCategory.clear();
        ImGui::SeparatorText(openCategory.c_str());
        drawRecipeRows(book, openCategory);
    }
}

// ---------------------------------------------------------------------------
// The whole left panel: "+ New Recipe" and the three tabs.
// ---------------------------------------------------------------------------
static void drawRecipeList(const RecipeBook& book)
{
    // ImVec2(-1, 0) = stretch to full width, normal height.
    if (ImGui::Button("+ New Recipe", ImVec2(-1, 0)))
        openAddForm();
    ImGui::Spacing();

    // A tab bar: only the code inside the selected tab's "if" runs each frame.
    if (ImGui::BeginTabBar("ListTabs"))
    {
        if (ImGui::BeginTabItem("Recipes"))
        {
            drawRecipeRows(book, "");  // "" = all recipes
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Favorites"))
        {
            ImGui::TextDisabled("Favorites coming soon.");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Categories"))
        {
            drawCategoriesTab(book);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

// ---------------------------------------------------------------------------
// The "Are you sure?" popup for deleting a recipe.
// Returns true if the recipe was deleted this frame.
// ---------------------------------------------------------------------------
static bool drawDeletePopup(RecipeBook& book, int index, const std::string& saveFilePath)
{
    bool deleted = false;

    // A "modal" popup blocks the rest of the app until it's closed.
    if (ImGui::BeginPopupModal("Delete Recipe?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Delete \"%s\"?", book.getRecipes()[index].name.c_str());
        ImGui::Text("This can't be undone.");
        ImGui::Spacing();

        if (ImGui::Button("Yes, Delete"))
        {
            book.deleteRecipe(index);
            saveBook(book, saveFilePath);
            selectedIndex = -1;  // nothing is selected anymore
            deleted = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
    return deleted;
}

// ---------------------------------------------------------------------------
// Right side, screen 1: show one recipe, with Edit and Delete buttons.
// ---------------------------------------------------------------------------
static void drawRecipeDetails(RecipeBook& book, int index, const std::string& saveFilePath)
{
    const Recipe& recipe = book.getRecipes()[index];
    ImGui::SeparatorText(recipe.name.c_str());

    if (ImGui::Button("Edit"))
        openEditForm(book, index);
    ImGui::SameLine();
    if (ImGui::Button("Delete"))
        ImGui::OpenPopup("Delete Recipe?");  // shown by drawDeletePopup below

    // If the recipe was just deleted, 'recipe' no longer exists, so stop here
    // instead of drawing its ingredients.
    if (drawDeletePopup(book, index, saveFilePath))
        return;

    // "Categories: Breakfast, Desserts"
    if (!recipe.categories.empty())
    {
        std::string list;
        for (size_t i = 0; i < recipe.categories.size(); i++)
        {
            if (i > 0)
                list += ", ";
            list += recipe.categories[i];
        }
        ImGui::TextDisabled("Categories: %s", list.c_str());
    }
    ImGui::Spacing();

    ImGui::Text("Ingredients");
    for (const std::string& ingredient : recipe.ingredients)
    {
        ImGui::Bullet();
        ImGui::TextWrapped("%s", ingredient.c_str());  // wraps long lines
    }

    ImGui::Spacing();
    ImGui::Text("Steps");
    for (int i = 0; i < static_cast<int>(recipe.steps.size()); i++)
        ImGui::TextWrapped("%d. %s", i + 1, recipe.steps[i].c_str());
}

// ---------------------------------------------------------------------------
// Part of the form: one checkbox per category.
// ---------------------------------------------------------------------------
static void drawCategoryCheckboxes(const RecipeBook& book)
{
    ImGui::Text("Categories");
    const std::vector<std::string>& allCategories = book.getCategories();
    if (allCategories.empty())
    {
        ImGui::TextDisabled("No categories yet. Create some with \"Manage Categories\".");
        return;
    }

    const int perRow = 4;  // how many checkboxes fit on one line
    for (int i = 0; i < static_cast<int>(allCategories.size()); i++)
    {
        const std::string& category = allCategories[i];
        bool isChecked = containsIgnoreCase(formCategories, category);

        ImGui::PushID(i);
        // Checkbox returns true on the frame it's clicked; isChecked is the new value.
        if (ImGui::Checkbox(category.c_str(), &isChecked))
        {
            if (isChecked)
                formCategories.push_back(category);
            else
                removeIgnoreCase(formCategories, category);
        }
        ImGui::PopID();

        bool endOfRow = ((i + 1) % perRow == 0);
        bool isLast = (i + 1 == static_cast<int>(allCategories.size()));
        if (!endOfRow && !isLast)
            ImGui::SameLine(0, 20);  // next checkbox on the same line, 20 pixels apart
    }
}

// ---------------------------------------------------------------------------
// Right side, screen 2: the form for adding a new recipe OR editing one.
// editingIndex tells us which: -1 means "new", anything else is the recipe
// being edited.
// ---------------------------------------------------------------------------
static void drawRecipeForm(RecipeBook& book, const std::string& saveFilePath)
{
    bool isEditing = (editingIndex != -1);
    ImGui::SeparatorText(isEditing ? "Edit Recipe" : "New Recipe");

    ImGui::Text("Name");
    ImGui::SetNextItemWidth(-1);  // -1 = fill the available width
    ImGui::InputText("##name", &formName);  // "##" hides the label but keeps a unique ID

    drawCategoryCheckboxes(book);

    // WordWrap makes long lines wrap onto the next line inside the box
    // instead of running off the right edge.
    const ImGuiInputTextFlags wrap = ImGuiInputTextFlags_WordWrap;

    ImGui::Text("Ingredients (press Enter between ingredients)");
    ImGui::InputTextMultiline("##ingredients", &formIngredients, ImVec2(-1, 220), wrap);

    ImGui::Text("Steps (press Enter between steps)");
    ImGui::InputTextMultiline("##steps", &formSteps, ImVec2(-1, 340), wrap);

    if (ImGui::Button(isEditing ? "Save Changes" : "Add Recipe"))
    {
        Recipe recipe;
        recipe.name = formName;
        recipe.ingredients = splitLines(formIngredients);
        recipe.steps = splitLines(formSteps);
        recipe.categories = formCategories;

        if (trim(recipe.name).empty())
        {
            formError = "Please enter a recipe name.";
        }
        else
        {
            bool worked = isEditing ? book.updateRecipe(editingIndex, recipe)
                                    : book.addRecipe(recipe);
            if (!worked)
            {
                formError = "Another recipe already has that name.";
            }
            else
            {
                saveBook(book, saveFilePath);  // write to the file right away

                // Show the recipe. We look it up by name because the list was
                // re-sorted, so its position may have changed.
                selectedIndex = book.findByName(recipe.name);
                closeForm();
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        if (!isEditing)
            selectedIndex = -1;  // when editing, go back to that recipe instead
        closeForm();
    }

    if (!formError.empty())
        ImGui::TextColored(errorColor, "%s", formError.c_str());
}

// ---------------------------------------------------------------------------
// Right side, screen 3: create and delete categories.
// ---------------------------------------------------------------------------
static void drawCategoryManager(RecipeBook& book, const std::string& saveFilePath)
{
    ImGui::SeparatorText("Manage Categories");
    ImGui::TextWrapped("Categories group your recipes, like \"Breakfast\" or \"Desserts\". "
                       "A recipe can be in more than one. Pick a recipe's categories when you add or edit it.");
    ImGui::Spacing();

    // Add a new category.
    ImGui::SetNextItemWidth(300);
    ImGui::InputText("##newCategory", &newCategoryName);
    ImGui::SameLine();
    if (ImGui::Button("Add Category"))
    {
        if (trim(newCategoryName).empty())
        {
            categoryError = "Please type a category name.";
        }
        else if (!book.addCategory(newCategoryName))
        {
            categoryError = "That category already exists.";
        }
        else
        {
            saveBook(book, saveFilePath);
            newCategoryName.clear();
            categoryError.clear();
        }
    }
    if (!categoryError.empty())
        ImGui::TextColored(errorColor, "%s", categoryError.c_str());
    ImGui::Spacing();

    // List every category with a Delete button.
    const std::vector<std::string>& allCategories = book.getCategories();
    if (allCategories.empty())
        ImGui::TextDisabled("No categories yet.");

    bool openDeletePopup = false;
    for (int i = 0; i < static_cast<int>(allCategories.size()); i++)
    {
        ImGui::PushID(i);
        if (ImGui::SmallButton("Delete"))
        {
            categoryToDelete = allCategories[i];
            openDeletePopup = true;  // opened below, outside PushID, so the IDs match
        }
        ImGui::PopID();
        ImGui::SameLine();
        int count = countRecipesIn(book, allCategories[i]);
        ImGui::Text("%s  (%d %s)", allCategories[i].c_str(), count, count == 1 ? "recipe" : "recipes");
    }
    if (openDeletePopup)
        ImGui::OpenPopup("Delete Category?");

    if (ImGui::BeginPopupModal("Delete Category?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Delete the category \"%s\"?", categoryToDelete.c_str());
        ImGui::Text("Its recipes will NOT be deleted. They just won't be in this category anymore.");
        ImGui::Spacing();

        if (ImGui::Button("Yes, Delete"))
        {
            book.deleteCategory(categoryToDelete);
            if (toLower(openCategory) == toLower(categoryToDelete))
                openCategory.clear();  // that category was open on the left; go back to the list
            saveBook(book, saveFilePath);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::Spacing();
    if (ImGui::Button("Done"))
        screen = Screen::Recipe;
}

// ---------------------------------------------------------------------------
// The whole screen.
// ---------------------------------------------------------------------------
void drawUI(RecipeBook& book, const std::string& saveFilePath)
{
    // One ImGui window that fills the entire app window.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin("Recipe Keeper", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    // Left panel: 300 pixels wide, full height, with a border.
    ImGui::BeginChild("ListPanel", ImVec2(300, 0), ImGuiChildFlags_Borders);
    drawRecipeList(book);
    ImGui::EndChild();

    ImGui::SameLine();

    // Right panel: takes up the rest of the space.
    ImGui::BeginChild("DetailPanel", ImVec2(0, 0), ImGuiChildFlags_Borders);
    const std::vector<Recipe>& recipes = book.getRecipes();
    if (screen == Screen::Form)
        drawRecipeForm(book, saveFilePath);
    else if (screen == Screen::Categories)
        drawCategoryManager(book, saveFilePath);
    else if (selectedIndex >= 0 && selectedIndex < static_cast<int>(recipes.size()))
        drawRecipeDetails(book, selectedIndex, saveFilePath);
    else
        ImGui::TextDisabled("Pick a recipe on the left, or click \"+ New Recipe\".");

    if (!saveError.empty())
        ImGui::TextColored(errorColor, "%s", saveError.c_str());
    ImGui::EndChild();

    ImGui::End();
}
