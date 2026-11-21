# Recipe Keeper - notes for Claude

## What this is
A Windows desktop recipe manager written in C++17 with a GUI (Dear ImGui + DirectX 11),
built for a Programming Fundamentals II "AI-Assisted Build Project" (6 weeks, Nov-Dec 2026).
The student (Miguel) is new to GitHub and to GUI programming.

## Rules for helping
- The student must be able to explain EVERY line in Q&A. Build in small steps, explain
  what each new piece does and why, and prefer simple, readable code over clever code.
- Use beginner-friendly C++: structs, classes, std::vector, std::string, loops.
  Avoid templates, smart-pointer tricks, and heavy STL/lambda use unless explained.
- Clear names, small functions, comments where needed (code quality is graded).
- Remind the student to log prompts in AI_LOG.md (exact wording) and to commit after
  each working step. Do not commit or push for them unless asked.

## Planned features (from the approved proposal)
1. Add recipes (name, ingredients, steps)
2. View recipes A-Z
3. Edit any part of a recipe
4. Categories (a recipe can be in several; categories listed A-Z)
5. Favorites
6. Case-insensitive partial-name search
7. Delete recipes and categories
8. Save/load to a file

## Layout
- `src/main.cpp` - window + DirectX + ImGui setup ("plumbing"); rarely changes
- `src/AppUI.cpp/.h` - all screens/widgets (drawUI)
- `src/RecipeBook.cpp/.h` - recipe storage and logic (no GUI code here)
- `src/Recipe.h` - the Recipe struct
- `src/TextUtils.cpp/.h` - toLower, trim, splitLines
- `tests/` - small assert-based tests for the logic (target: RecipeTests)

## Build
Visual Studio 2022/2026 with C++ workload. Open the folder in Visual Studio, or:
`cmake -S . -B build` then `cmake --build build --config Debug`.
Run tests: `build\Debug\RecipeTests.exe`.
