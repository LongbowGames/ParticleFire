#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellscalingapi.h>

#include "ParticleFire.h"
#include "resource.h"

static HINSTANCE g_hInst = NULL;

// ------------------------------------------------------------
// Window class name used for both preview + fullscreen host
// ------------------------------------------------------------
static const wchar_t* kClassName = L"ScreenSaverHostClass";

// ------------------------------------------------------------
// Forward declarations
// ------------------------------------------------------------
LRESULT CALLBACK HostWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ------------------------------------------------------------
// FULLSCREEN MODE
// ------------------------------------------------------------
static void RunFullScreen()
{
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    HWND hwnd = CreateWindowExW(
        //0,
        WS_EX_TOPMOST,
        kClassName,
        L"ScreenSaver",
        //WS_OVERLAPPEDWINDOW,
        WS_POPUP | WS_VISIBLE,
        x, y, w, h,
        NULL,
        NULL,
        g_hInst,
        NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// ------------------------------------------------------------
// PREVIEW MODE (hosted inside Explorer preview window)
// ------------------------------------------------------------
static void RunPreview(HWND hwndParent)
{
    RECT rc;
    GetClientRect(hwndParent, &rc);

    // Get DPI scale
    // This is annoying. The size of this preview depends on the scale of the monitor that the preview opened on,
    // and which monitor is the default.
    float scale = 1.0f;
    HMONITOR hMon = MonitorFromWindow(hwndParent, MONITOR_DEFAULTTONEAREST);
    UINT dpiX, dpiY;
    if (SUCCEEDED(GetDpiForMonitor(hMon, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)))
    {
        scale = dpiX / 96.0f;
    }
    UINT parentDpi = GetDpiForWindow(hwndParent);
    scale /= parentDpi / 96.0f;

    int width = (int)((rc.right - rc.left) / scale);
    int height = (int)((rc.bottom - rc.top) / scale);

    HWND hwnd = CreateWindowExW(
        WS_EX_TOOLWINDOW,
        kClassName,
        L"Preview",
        WS_POPUP | WS_VISIBLE | WS_CHILD,
        0, 0,
        width,
        height,
        NULL,
        NULL,
        g_hInst,
        NULL
    );

    if (hwnd)
    {
        SetParent(hwnd, hwndParent);
        SetWindowPos(hwnd, HWND_TOP, 0, 0, width, height, SWP_NOACTIVATE);
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// ------------------------------------------------------------
// CONFIG MODE
// ------------------------------------------------------------
static void RunConfig(HWND hwndParent)
{
    DialogBoxParamW(
        g_hInst,
        MAKEINTRESOURCEW(IDD_DIALOG1), // your dialog resource ID
        hwndParent,
        (DLGPROC)ScreenSaverConfigureDialog,
        0
    );
}

LRESULT CALLBACK HostWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return ScreenSaverProc(hwnd, msg, wParam, lParam);
}

// ------------------------------------------------------------
// ENTRY POINT (REPLACES scrnsavw.lib COMPLETELY)
// ------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR lpCmdLine, int)
{
    g_hInst = hInstance;

    // Register window class
    WNDCLASSW wc = {};
    wc.lpfnWndProc = HostWndProc;
    wc.hInstance = g_hInst;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassW(&wc);

    std::wstring cmd = lpCmdLine ? lpCmdLine : GetCommandLineW();

    // lowercase for simple parsing
    for (auto& c : cmd) c = towlower(c);

    auto c_pos = cmd.find(L"/c");
    if (c_pos != std::wstring::npos)
    {
        // Force System DPI for config dialog
        SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
        SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
        HWND parent = GetForegroundWindow();

        c_pos += 2;

        if (c_pos <= cmd.length() && cmd[c_pos] == L':') {
            auto parentParam = cmd.substr(c_pos+1);

            try {
                parent = HWND(std::stoll(parentParam));
            }
            catch (...) {
                // Ignore and default to foreground window
            }
        }

        RunConfig(parent);
        return 0;
    }

    if (cmd.find(L"/p") != std::wstring::npos)
    {
        size_t pos = cmd.find(L"/p");
        if (pos != std::wstring::npos)
        {
            const wchar_t* p = cmd.c_str() + pos + 2;

            while (*p == L' ') ++p; // trim whitespace

            ULONG_PTR value = wcstoull(p, nullptr, 10);
            HWND hwndParent = (HWND)value;

            RunPreview(hwndParent);
        }

        return 0;
    }

    RunFullScreen();
    return 0;
}