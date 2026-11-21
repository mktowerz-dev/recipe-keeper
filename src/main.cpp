// Recipe Keeper - program entry point
//
// This file is the "plumbing" that opens a Windows window and lets Dear ImGui
// draw buttons, text boxes and lists in it. It is adapted from Dear ImGui's
// official example (examples/example_win32_directx11/main.cpp).
//
// The actual screens live in AppUI.cpp, and the recipe logic in RecipeBook.cpp.
// You will rarely need to change this file.

#include "AppUI.h"
#include "RecipeBook.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include <d3d11.h>
#include <windows.h>

// ---------------------------------------------------------------------------
// Graphics (DirectX 11) objects. Think of these as "the canvas" ImGui draws on.
// ---------------------------------------------------------------------------
static ID3D11Device*           g_device        = nullptr;
static ID3D11DeviceContext*    g_context       = nullptr;
static IDXGISwapChain*         g_swapChain     = nullptr;
static ID3D11RenderTargetView* g_renderTarget  = nullptr;
static UINT                    g_resizeWidth   = 0;
static UINT                    g_resizeHeight  = 0;

static bool createDeviceD3D(HWND hWnd);
static void cleanupDeviceD3D();
static void createRenderTarget();
static void cleanupRenderTarget();
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// WinMain is the starting point for a Windows GUI program (instead of main()).
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // 1. Create the window.
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L,
                       GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
                       L"RecipeKeeperWindow", nullptr };
    RegisterClassExW(&wc);
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Recipe Keeper", WS_OVERLAPPEDWINDOW,
                              100, 100, 1100, 750, nullptr, nullptr, wc.hInstance, nullptr);

    // 2. Connect it to the graphics card.
    if (!createDeviceD3D(hwnd))
    {
        cleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // 3. Start Dear ImGui.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;            // don't write imgui.ini next to the exe
    // Use Windows' built-in Segoe UI font; fall back to ImGui's default if it's missing.
    if (!io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 22.0f))
        io.Fonts->AddFontDefault();
    ImGui::StyleColorsLight();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_device, g_context);

    // 4. All of the user's recipes live here while the program runs.
    RecipeBook book;

    // 5. Main loop: runs ~60 times a second until the window is closed.
    bool done = false;
    while (!done)
    {
        // Handle Windows messages (mouse, keyboard, close button...).
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // If the window was resized, rebuild the canvas at the new size.
        if (g_resizeWidth != 0 && g_resizeHeight != 0)
        {
            cleanupRenderTarget();
            g_swapChain->ResizeBuffers(0, g_resizeWidth, g_resizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_resizeWidth = g_resizeHeight = 0;
            createRenderTarget();
        }

        // Start a new frame, describe the UI, then draw it.
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        drawUI(book);

        ImGui::Render();
        const float clearColor[4] = { 0.94f, 0.92f, 0.88f, 1.0f };
        g_context->OMSetRenderTargets(1, &g_renderTarget, nullptr);
        g_context->ClearRenderTargetView(g_renderTarget, clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_swapChain->Present(1, 0);   // 1 = wait for vsync (limits to screen refresh rate)
    }

    // 6. Clean up.
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    cleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

// ---------------------------------------------------------------------------
// DirectX 11 helpers (from the Dear ImGui example). You don't need to change these.
// ---------------------------------------------------------------------------
static bool createDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL levels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                                levels, 2, D3D11_SDK_VERSION, &sd, &g_swapChain,
                                                &g_device, &featureLevel, &g_context);
    if (res == DXGI_ERROR_UNSUPPORTED) // no GPU support: fall back to software rendering
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
                                            levels, 2, D3D11_SDK_VERSION, &sd, &g_swapChain,
                                            &g_device, &featureLevel, &g_context);
    if (res != S_OK)
        return false;

    createRenderTarget();
    return true;
}

static void cleanupDeviceD3D()
{
    cleanupRenderTarget();
    if (g_swapChain) { g_swapChain->Release(); g_swapChain = nullptr; }
    if (g_context)   { g_context->Release();   g_context = nullptr; }
    if (g_device)    { g_device->Release();    g_device = nullptr; }
}

static void createRenderTarget()
{
    ID3D11Texture2D* backBuffer = nullptr;
    g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    g_device->CreateRenderTargetView(backBuffer, nullptr, &g_renderTarget);
    backBuffer->Release();
}

static void cleanupRenderTarget()
{
    if (g_renderTarget) { g_renderTarget->Release(); g_renderTarget = nullptr; }
}

// Forward Windows messages to ImGui (declared in imgui_impl_win32.h behind a #if 0).
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_resizeWidth = (UINT)LOWORD(lParam);   // handled in the main loop
        g_resizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // disable the ALT menu beep
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}
