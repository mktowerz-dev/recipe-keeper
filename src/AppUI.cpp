#include "AppUI.h"
#include "TextUtils.h"
#include "imgui.h"
#include "imgui_stdlib.h"   // lets ImGui::InputText work with std::string
#include <string>

// ---------------------------------------------------------------------------
// Screen state. These are 'static' so they keep their values between frames
// (drawUI runs ~60 times a second) and are only visible inside this file.
// ---------------------------------------------------------------------------
static int  selectedIndex = -1;     // which recipe is selected in the list (-1 = none)
static bool showForm      = false;  // true while the add/edit form is on screen
static int  editingIndex  = -1;     // recipe being edited, or -1 when adding a new one

// What the user is typing into the add/edit form.
static std::string formName;
static std::string formIngredients; // one ingredient per line
static std::string formSteps;       // one step per line
static std::string formError;       // message shown if the form can't be saved
static std::string saveError;       // message shown if writing the save file failed

static const ImVec4 errorColor(0.8f, 0.1f, 0.1f, 1.0f);

static void clearForm()
{
    formName.clear();
    formIngredients.clear();
    formSteps.clear();
    formError.clear();
}

static void closeForm()
{
    showForm = false;
    editingIndex = -1;
    clearForm();
}

// Shows an empty form for a brand-new recipe.
static void openAddForm()
{
    clearForm();
    editingIndex = -1;
    selectedIndex = -1;
    showForm = true;
}

// Shows the form filled in with an existing recipe so it can be changed.
static void openEditForm(const RecipeBook& book, int index)
{
    const Recipe& recipe = book.getRecipes()[index];
    clearForm();
    formName = recipe.name;
    formIngredients = joinLines(recipe.ingredients);
    formSteps = joinLines(recipe.steps);
    editingIndex = index;
    showForm = true;
}

// Writes all recipes to the save file and remembers if it failed.
static void saveBook(const RecipeBook& book, const std::string& saveFilePath)
{
    if (book.saveToFile(saveFilePath))
        saveError.clear();
    else
        saveError = "Couldn't save to " + saveFilePath;
}

// ---------------------------------------------------------------------------
// Left side: the "New Recipe" button and the A-Z list of recipe names.
// ---------------------------------------------------------------------------
static void drawRecipeList(const RecipeBook& book)
{
    ImGui::SeparatorText("My Recipes");

    // ImVec2(-1, 0) = stretch to full width, normal height.
    if (ImGui::Button("+ New Recipe", ImVec2(-1, 0)))
        openAddForm();
    ImGui::Spacing();

    const std::vector<Recipe>& recipes = book.getRecipes();
    if (recipes.empty())
        ImGui::TextDisabled("No recipes yet.");

    for (int i = 0; i < static_cast<int>(recipes.size()); i++)
    {
        ImGui::PushID(i);  // gives each row a unique ID, even if two names look alike
        bool isSelected = (i == selectedIndex);
        if (ImGui::Selectable(recipes[i].name.c_str(), isSelected))
        {
            selectedIndex = i;
            closeForm();
        }
        ImGui::PopID();
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
// Right side, option 1: show one recipe, with Edit and Delete buttons.
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
// Right side, option 2: the form for adding a new recipe OR editing one.
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
    if (showForm)
        drawRecipeForm(book, saveFilePath);
    else if (selectedIndex >= 0 && selectedIndex < static_cast<int>(recipes.size()))
        drawRecipeDetails(book, selectedIndex, saveFilePath);
    else
        ImGui::TextDisabled("Pick a recipe on the left, or click \"+ New Recipe\".");

    if (!saveError.empty())
        ImGui::TextColored(errorColor, "%s", saveError.c_str());
    ImGui::EndChild();

    ImGui::End();
}
