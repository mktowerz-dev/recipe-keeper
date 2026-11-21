cmake_minimum_required(VERSION 3.20)
project(RecipeKeeper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Dear ImGui: the GUI library. CMake downloads it automatically the first
# time you build, so there is nothing to install by hand.
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.9b   # 1.92.x adds word-wrap for multi-line text boxes
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp          # lets InputText work with std::string
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp      # talks to the Windows window
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp       # draws with DirectX 11
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imgui_SOURCE_DIR}/misc/cpp
)

# ---------------------------------------------------------------------------
# Recipe logic (no GUI code), shared by the app and the tests
# ---------------------------------------------------------------------------
add_library(recipe_core STATIC
    src/RecipeBook.cpp
    src/TextUtils.cpp
)
target_include_directories(recipe_core PUBLIC src)

# ---------------------------------------------------------------------------
# The app
# ---------------------------------------------------------------------------
add_executable(RecipeKeeper WIN32
    src/main.cpp
    src/AppUI.cpp
)
target_link_libraries(RecipeKeeper PRIVATE recipe_core imgui d3d11 dxgi d3dcompiler)

# ---------------------------------------------------------------------------
# Automated tests: build\Debug\RecipeTests.exe
# ---------------------------------------------------------------------------
add_executable(RecipeTests tests/test_main.cpp)
target_link_libraries(RecipeTests PRIVATE recipe_core)
