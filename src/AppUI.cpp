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
static bool showAddForm   = false;  // true while the "New Recipe" form is on screen

// What the user is typing into the "New Recipe" form.
static std::string newName;
static std::string newIngredients;  // one ingredient per line
static std::string newSteps;        // one step per line
static std::string formError;       // message shown if the form can't be saved
static std::string saveError;       // message shown if writing the save file failed

static void clearAddForm()
{
    newName.clear();
    newIngredients.clear();
    newSteps.clear();
    formError.clear();
}

// ---------------------------------------------------------------------------
// Left side: the "New Recipe" button and the A-Z list of recipe names.
// ---------------------------------------------------------------------------
static void drawRecipeList(const RecipeBook& book)
{
    ImGui::SeparatorText("My Recipes");

    // ImVec2(-1, 0) = stretch to full width, normal height.
    if (ImGui::Button("+ New Recipe", ImVec2(-1, 0)))
    {
        showAddForm = true;
        selectedIndex = -1;
        clearAddForm();
    }
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
            showAddForm = false;
        }
        ImGui::PopID();
    }
}

// ---------------------------------------------------------------------------
// Right side, option 1: show one recipe.
// ---------------------------------------------------------------------------
static void drawRecipeDetails(const Recipe& recipe)
{
    ImGui::SeparatorText(recipe.name.c_str());

    ImGui::Text("Ingredients");
    for (const std::string& ingredient : recipe.ingredients)
        ImGui::BulletText("%s", ingredient.c_str());

    ImGui::Spacing();
    ImGui::Text("Steps");
    for (int i = 0; i < static_cast<int>(recipe.steps.size()); i++)
        ImGui::TextWrapped("%d. %s", i + 1, recipe.steps[i].c_str());
}

// ---------------------------------------------------------------------------
// Right side, option 2: the form for adding a new recipe.
// ---------------------------------------------------------------------------
static void drawAddForm(RecipeBook& book, const std::string& saveFilePath)
{
    ImGui::SeparatorText("New Recipe");

    ImGui::Text("Name");
    ImGui::SetNextItemWidth(-1);  // -1 = fill the available width
    ImGui::InputText("##name", &newName);  // "##" hides the label but keeps a unique ID

    ImGui::Text("Ingredients (one per line)");
    ImGui::InputTextMultiline("##ingredients", &newIngredients, ImVec2(-1, 150));

    ImGui::Text("Steps (one per line, in order)");
    ImGui::InputTextMultiline("##steps", &newSteps, ImVec2(-1, 200));

    if (ImGui::Button("Add Recipe"))
    {
        Recipe recipe;
        recipe.name = newName;
        recipe.ingredients = splitLines(newIngredients);
        recipe.steps = splitLines(newSteps);

        if (trim(recipe.name).empty())
        {
            formError = "Please enter a recipe name.";
        }
        else if (!book.addRecipe(recipe))
        {
            formError = "A recipe with that name already exists.";
        }
        else
        {
            // Write all recipes to the file right away, so nothing is lost.
            if (book.saveToFile(saveFilePath))
                saveError.clear();
            else
                saveError = "Couldn't save to " + saveFilePath;

            // Added! Show the new recipe. We look it up by name because the
            // list was re-sorted, so its position may not be at the end.
            selectedIndex = book.findByName(recipe.name);
            showAddForm = false;
            clearAddForm();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        showAddForm = false;
        clearAddForm();
    }

    if (!formError.empty())
        ImGui::TextColored(ImVec4(0.8f, 0.1f, 0.1f, 1.0f), "%s", formError.c_str());
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
    if (showAddForm)
        drawAddForm(book, saveFilePath);
    else if (selectedIndex >= 0 && selectedIndex < static_cast<int>(recipes.size()))
        drawRecipeDetails(recipes[selectedIndex]);
    else
        ImGui::TextDisabled("Pick a recipe on the left, or click \"+ New Recipe\".");

    if (!saveError.empty())
        ImGui::TextColored(ImVec4(0.8f, 0.1f, 0.1f, 1.0f), "%s", saveError.c_str());
    ImGui::EndChild();

    ImGui::End();
}
