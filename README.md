# Recipe Keeper

A Windows desktop app (C++ with a graphical interface) that keeps all my cooking and
baking recipes in one place so they're easy to find, edit, and organize.

> Status: in progress (Programming Fundamentals II major project)

## Planned features

1. **Add recipes** with a name, ingredients, and step-by-step instructions
2. **View** all recipes in alphabetical order
3. **Edit** any part of a recipe without starting over
4. **Categories** (e.g. Breakfast, Desserts); a recipe can be in several
5. **Favorites** list
6. **Search** by part of a name, ignoring capital letters ("chick" finds "Chicken Alfredo")
7. **Delete** recipes and categories
8. **Save and load** to a file so recipes are kept between runs

## Built with

- C++17
- [Dear ImGui](https://github.com/ocornut/imgui) for the graphical interface
  (buttons, text fields, lists), drawn with DirectX 11
- CMake (downloads Dear ImGui automatically)

## How to build and run

Requirements: Windows 10/11 and **Visual Studio 2022 or newer** with the
**"Desktop development with C++"** workload.

1. Clone this repository.
2. In Visual Studio choose **File > Open > Folder...** and pick the project folder.
   Visual Studio detects `CMakeLists.txt` and sets everything up (the first time
   it downloads Dear ImGui, which takes a minute).
3. In the green ▶ dropdown at the top, select **RecipeKeeper.exe**, then press **F5**.

Command line alternative (from a "Developer PowerShell for VS"):

```
cmake -S . -B build
cmake --build build --config Debug
build\Debug\RecipeKeeper.exe
```

## Screenshot

_Coming soon._

## AI use

See [AI_LOG.md](AI_LOG.md) for the prompts I used, what the AI produced, and what I changed.
