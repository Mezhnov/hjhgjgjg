/*
 * VORTEX Desktop Environment
 * Modern Desktop Shell in pure C++
 * With Wallpaper Background Support (URL Loading)
 */

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <urlmon.h>      // For URL downloading
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <sstream>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "urlmon.lib")    // For URLDownloadToFile

 // GDI+ for modern graphics
#include <gdiplus.h>
using namespace Gdiplus;

// ============================================================================
// CONFIGURATION - Wallpaper URL
// ============================================================================

// Укажите здесь ссылку на картинку для обоев
const wchar_t* WALLPAPER_URL = L"https://images.unsplash.com/photo-1534796636912-3b95b3ab5986?w=1920&q=80";

// Локальный путь для сохранения скачанной картинки
const wchar_t* WALLPAPER_CACHE_PATH = L"vortex_wallpaper_cache.jpg";

// ============================================================================
// COLORS - Modern Dark Theme
// ============================================================================

namespace Colors {
    constexpr COLORREF Background = 0x0a0a0f;
    constexpr COLORREF Surface = 0x1a1a24;
    constexpr COLORREF SurfaceLight = 0x2a2a3a;
    constexpr COLORREF SurfaceHover = 0x3a3a4a;
    constexpr COLORREF Accent = 0x6366f1;
    constexpr COLORREF AccentLight = 0x818cf8;
    constexpr COLORREF AccentDark = 0x4f46e5;
    constexpr COLORREF Text = 0xffffff;
    constexpr COLORREF TextDim = 0x9ca3af;
    constexpr COLORREF Border = 0x2a2a3a;
    constexpr COLORREF Success = 0x22c55e;
    constexpr COLORREF Warning = 0xf59e0b;
    constexpr COLORREF Error = 0xef4444;
    constexpr COLORREF Dock = 0x1a1a24;
    constexpr COLORREF DockHover = 0x2a2a3a;
    constexpr COLORREF Widget = 0x1a1a24;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

Color ColorFromRGB(COLORREF clr, BYTE alpha = 255) {
    return Color(alpha, GetRValue(clr), GetGValue(clr), GetBValue(clr));
}

int GetScreenWidth() { return GetSystemMetrics(SM_CXSCREEN); }
int GetScreenHeight() { return GetSystemMetrics(SM_CYSCREEN); }

// Rounded rectangle with GDI+
void FillRoundRect(Graphics& g, int x, int y, int w, int h, int radius, Color color) {
    SolidBrush brush(color);
    GraphicsPath path;
    path.AddArc(x, y, radius * 2, radius * 2, 180, 90);
    path.AddArc(x + w - radius * 2, y, radius * 2, radius * 2, 270, 90);
    path.AddArc(x + w - radius * 2, y + h - radius * 2, radius * 2, radius * 2, 0, 90);
    path.AddArc(x, y + h - radius * 2, radius * 2, radius * 2, 90, 90);
    path.CloseFigure();
    g.FillPath(&brush, &path);
}

void DrawRoundRect(Graphics& g, int x, int y, int w, int h, int radius, Color color, float width = 1.0f) {
    Pen pen(color, width);
    GraphicsPath path;
    path.AddArc(x, y, radius * 2, radius * 2, 180, 90);
    path.AddArc(x + w - radius * 2, y, radius * 2, radius * 2, 270, 90);
    path.AddArc(x + w - radius * 2, y + h - radius * 2, radius * 2, radius * 2, 0, 90);
    path.AddArc(x, y + h - radius * 2, radius * 2, radius * 2, 90, 90);
    path.CloseFigure();
    g.DrawPath(&pen, &path);
}

// ============================================================================
// APP ICON DATA
// ============================================================================

struct AppIcon {
    std::wstring name;
    std::wstring executable;
    int iconIndex;
    Color color;
    bool hovered;
    RECT bounds;
};

std::vector<AppIcon> g_dockApps = {
    {L"Files", L"explorer.exe", 0, Color(255, 59, 130, 246), false},
    {L"Browser", L"msedge.exe", 0, Color(255, 34, 197, 94), false},
    {L"Terminal", L"cmd.exe", 0, Color(255, 139, 92, 246), false},
    {L"NEXUS IDE", L"notepad.exe", 0, Color(255, 249, 115, 22), false},
    {L"Settings", L"ms-settings:", 0, Color(255, 107, 114, 128), false},
    {L"Store", L"ms-windows-store:", 0, Color(255, 6, 182, 212), false},
    {L"Photos", L"ms-photos:", 0, Color(255, 236, 72, 153), false},
    {L"Music", L"wmplayer.exe", 0, Color(255, 168, 85, 247), false},
};

// ============================================================================
// WIDGETS
// ============================================================================

struct Widget {
    std::wstring title;
    std::wstring content;
    RECT bounds;
    bool hovered;
};

std::vector<Widget> g_widgets;

void UpdateWidgets() {
    g_widgets.clear();

    int screenW = GetScreenWidth();

    // Time widget
    Widget timeWidget;
    timeWidget.title = L"TIME";
    timeWidget.bounds = { screenW - 320, 40, screenW - 20, 180 };

    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);

    wchar_t timeStr[64], dateStr[64];
    wcsftime(timeStr, 64, L"%H:%M", &timeinfo);
    wcsftime(dateStr, 64, L"%A, %B %d", &timeinfo);

    timeWidget.content = std::wstring(timeStr) + L"\n" + dateStr;
    timeWidget.hovered = false;
    g_widgets.push_back(timeWidget);

    // System widget
    Widget sysWidget;
    sysWidget.title = L"SYSTEM";
    sysWidget.bounds = { screenW - 320, 200, screenW - 20, 340 };

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);

    DWORD cpuLoad = 0;
    HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
    if (hNtdll) {
        typedef ULONG(WINAPI* NtQuerySystemInformation_t)(ULONG, PVOID, ULONG, PULONG);
        auto NtQuerySystemInformation = (NtQuerySystemInformation_t)GetProcAddress(hNtdll, "NtQuerySystemInformation");
    }

    wchar_t sysStr[256];
    swprintf_s(sysStr, L"CPU: %d%%\nRAM: %d%%\nDisk: 45%%", 35, (int)memStatus.dwMemoryLoad);
    sysWidget.content = sysStr;
    sysWidget.hovered = false;
    g_widgets.push_back(sysWidget);

    // Weather widget (simulated)
    Widget weatherWidget;
    weatherWidget.title = L"WEATHER";
    weatherWidget.bounds = { screenW - 320, 360, screenW - 20, 480 };
    weatherWidget.content = L"☀️ 22°C\nSunny\nMoscow, RU";
    weatherWidget.hovered = false;
    g_widgets.push_back(weatherWidget);
}

// ============================================================================
// GLOBALS
// ============================================================================

HWND g_hDesktop = nullptr;
HWND g_hDock = nullptr;
HWND g_hLauncher = nullptr;
HWND g_hWallpaper = nullptr;

bool g_showLauncher = false;
int g_hoveredApp = -1;
int g_hoveredWidget = -1;
bool g_dockHovered = false;
float g_dockScale = 1.0f;

ULONG_PTR g_gdiplusToken;
Bitmap* g_wallpaperBitmap = nullptr;  // Wallpaper image
bool g_wallpaperLoaded = false;
bool g_wallpaperLoading = false;

// ============================================================================
// WALLPAPER FUNCTIONS - URL Support
// ============================================================================

// Get path for cached wallpaper
std::wstring GetWallpaperCachePath() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::wstring exeDir = exePath;
    size_t lastSlash = exeDir.find_last_of(L"\\");
    if (lastSlash != std::wstring::npos) {
        exeDir = exeDir.substr(0, lastSlash);
    }
    return exeDir + L"\\" + WALLPAPER_CACHE_PATH;
}

// Download wallpaper from URL
bool DownloadWallpaperFromURL(const wchar_t* url, const wchar_t* localPath) {
    HRESULT hr = URLDownloadToFileW(
        nullptr,           // Caller - null for standalone
        url,               // URL to download
        localPath,         // Local file path
        0,                 // Reserved
        nullptr            // Status callback (null = no progress)
    );
    
    return SUCCEEDED(hr);
}

// Load wallpaper from file
bool LoadWallpaperFromFile(const wchar_t* filePath) {
    if (g_wallpaperBitmap) {
        delete g_wallpaperBitmap;
        g_wallpaperBitmap = nullptr;
    }

    // Check if file exists
    DWORD attrib = GetFileAttributesW(filePath);
    if (attrib == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    g_wallpaperBitmap = new Bitmap(filePath);
    if (g_wallpaperBitmap->GetLastStatus() != Ok) {
        delete g_wallpaperBitmap;
        g_wallpaperBitmap = nullptr;
        return false;
    }

    return true;
}

// Load wallpaper from URL (downloads and caches)
bool LoadWallpaperFromURL(const wchar_t* url) {
    std::wstring cachePath = GetWallpaperCachePath();
    
    // Download the image
    if (DownloadWallpaperFromURL(url, cachePath.c_str())) {
        // Load the downloaded image
        return LoadWallpaperFromFile(cachePath.c_str());
    }
    
    return false;
}

// Thread function for async wallpaper loading
DWORD WINAPI LoadWallpaperThread(LPVOID param) {
    g_wallpaperLoading = true;
    
    if (LoadWallpaperFromURL(WALLPAPER_URL)) {
        g_wallpaperLoaded = true;
        // Force redraw
        if (g_hDesktop) {
            InvalidateRect(g_hDesktop, nullptr, FALSE);
        }
    }
    
    g_wallpaperLoading = false;
    return 0;
}

// Start async wallpaper loading
void StartWallpaperLoadAsync() {
    CreateThread(nullptr, 0, LoadWallpaperThread, nullptr, 0, nullptr);
}

void DrawWallpaper(Graphics& g, int screenW, int screenH) {
    if (g_wallpaperBitmap && g_wallpaperLoaded) {
        // Calculate scaling to cover the entire screen (cover mode)
        REAL imgW = (REAL)g_wallpaperBitmap->GetWidth();
        REAL imgH = (REAL)g_wallpaperBitmap->GetHeight();
        REAL screenWR = (REAL)screenW;
        REAL screenHR = (REAL)screenH;

        REAL scaleX = screenWR / imgW;
        REAL scaleY = screenHR / imgH;
        REAL scale = max(scaleX, scaleY);  // Cover entire screen

        REAL newW = imgW * scale;
        REAL newH = imgH * scale;
        REAL offsetX = (screenWR - newW) / 2.0f;
        REAL offsetY = (screenHR - newH) / 2.0f;

        // Draw scaled wallpaper
        g.DrawImage(g_wallpaperBitmap, offsetX, offsetY, newW, newH);
    } else {
        // Fallback: gradient background if no wallpaper loaded
        LinearGradientBrush wallBrush(
            Point(0, 0), Point(screenW, screenH),
            Color(255, 15, 23, 42), Color(255, 30, 41, 59)
        );
        g.FillRectangle(&wallBrush, 0, 0, screenW, screenH);

        // Draw decorative circles
        for (int i = 0; i < 5; i++) {
            int cx = 200 + i * 300;
            int cy = 200 + (i % 3) * 150;
            int cr = 100 + i * 50;
            Color circleColor(10, 99, 102, 241);
            g.FillEllipse(&SolidBrush(circleColor), cx - cr, cy - cr, cr * 2, cr * 2);
        }
        
        // Show loading indicator if wallpaper is being loaded
        if (g_wallpaperLoading) {
            FontFamily fontFamily(L"Segoe UI");
            Font loadingFont(&fontFamily, 14, FontStyleRegular, UnitPixel);
            SolidBrush loadingBrush(Color(200, 255, 255, 255));
            
            StringFormat format;
            format.SetAlignment(StringAlignmentCenter);
            format.SetLineAlignment(StringAlignmentCenter);
            
            RectF loadingRect((REAL)screenW / 2 - 100, (REAL)screenH / 2 - 20, 200, 40);
            g.DrawString(L"Loading wallpaper...", -1, &loadingFont, loadingRect, &format, &loadingBrush);
        }
    }
}

// ============================================================================
// DRAW FUNCTIONS
// ============================================================================

void DrawWidget(Graphics& g, const Widget& widget, bool hovered) {
    int x = widget.bounds.left;
    int y = widget.bounds.top;
    int w = widget.bounds.right - widget.bounds.left;
    int h = widget.bounds.bottom - widget.bounds.top;

    // Background with glass effect
    Color bgColor = hovered ? Color(40, 26, 26, 36) : Color(30, 26, 26, 36);
    FillRoundRect(g, x, y, w, h, 16, bgColor);
    DrawRoundRect(g, x, y, w, h, 16, ColorFromRGB(Colors::Border), 1.0f);

    // Title
    FontFamily fontFamily(L"Segoe UI");
    Font titleFont(&fontFamily, 11, FontStyleRegular, UnitPixel);
    Font contentFont(&fontFamily, 24, FontStyleRegular, UnitPixel);
    Font smallFont(&fontFamily, 14, FontStyleRegular, UnitPixel);

    SolidBrush titleBrush(ColorFromRGB(Colors::TextDim));
    SolidBrush textBrush(ColorFromRGB(Colors::Text));

    StringFormat format;
    format.SetAlignment(StringAlignmentNear);
    format.SetLineAlignment(StringAlignmentNear);

    RectF titleRect((REAL)x + 16, (REAL)y + 12, (REAL)w - 32, 20);
    g.DrawString(widget.title.c_str(), -1, &titleFont, titleRect, &format, &titleBrush);

    // Content
    std::vector<std::wstring> lines;
    std::wistringstream stream(widget.content);
    std::wstring line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }

    if (widget.title == L"TIME") {
        // Big time
        RectF timeRect((REAL)x + 16, (REAL)y + 40, (REAL)w - 32, 50);
        g.DrawString(lines[0].c_str(), -1, &contentFont, timeRect, &format, &textBrush);

        // Date
        if (lines.size() > 1) {
            RectF dateRect((REAL)x + 16, (REAL)y + 90, (REAL)w - 32, 24);
            g.DrawString(lines[1].c_str(), -1, &smallFont, dateRect, &format, &titleBrush);
        }
    }
    else {
        // Multi-line content
        REAL lineY = (REAL)y + 40;
        for (const auto& l : lines) {
            RectF lineRect((REAL)x + 16, lineY, (REAL)w - 32, 24);
            g.DrawString(l.c_str(), -1, &smallFont, lineRect, &format, &textBrush);
            lineY += 26;
        }
    }
}

void DrawDock(Graphics& g, int screenW, int screenH) {
    int iconSize = 52;
    int iconSpacing = 8;
    int padding = 12;
    int dockHeight = iconSize + padding * 2;
    int totalWidth = (int)g_dockApps.size() * (iconSize + iconSpacing) - iconSpacing + padding * 2;
    int dockX = (screenW - totalWidth) / 2;
    int dockY = screenH - dockHeight - 20;

    // Dock background with glass effect
    FillRoundRect(g, dockX, dockY, totalWidth, dockHeight, 20, Color(180, 26, 26, 36));
    DrawRoundRect(g, dockX, dockY, totalWidth, dockHeight, 20, ColorFromRGB(Colors::Border), 1.5f);

    // Draw icons
    int iconX = dockX + padding;
    for (size_t i = 0; i < g_dockApps.size(); i++) {
        AppIcon& app = g_dockApps[i];
        float scale = app.hovered ? 1.25f : 1.0f;
        int scaledSize = (int)(iconSize * scale);
        int offsetX = (scaledSize - iconSize) / 2;
        int offsetY = (scaledSize - iconSize) / 2;

        int x = iconX - offsetX;
        int y = dockY + padding - offsetY;

        app.bounds = { x, y, x + scaledSize, y + scaledSize };

        // Icon letter/emoji
        FontFamily fontFamily(L"Segoe UI");
        Font iconFont(&fontFamily, app.hovered ? 22 : 18, FontStyleBold, UnitPixel);
        SolidBrush textBrush(Color(255, 255, 255, 255));

        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);

        const wchar_t* letters[] = { L"📁", L"🌐", L"💻", L"⚡", L"⚙", L"🛒", L"📷", L"🎵" };
        RectF textRect((REAL)x, (REAL)y, (REAL)scaledSize, (REAL)scaledSize);
        g.DrawString(letters[i], -1, &iconFont, textRect, &format, &textBrush);

        // Hover indicator
        if (app.hovered) {
            FillRoundRect(g, iconX + iconSize / 2 - 3, dockY + dockHeight - 8, 6, 4, 2, ColorFromRGB(Colors::Text));
        }
        else {
            FillRoundRect(g, iconX + iconSize / 2 - 2, dockY + dockHeight - 6, 4, 3, 2, Color(150, 150, 150, 150));
        }

        iconX += iconSize + iconSpacing;
    }
}

void DrawLauncher(Graphics& g, int screenW, int screenH) {
    if (!g_showLauncher) return;

    // Blur overlay
    SolidBrush overlayBrush(Color(200, 10, 10, 15));
    g.FillRectangle(&overlayBrush, 0, 0, screenW, screenH);

    // Launcher panel
    int launcherW = 700;
    int launcherH = 500;
    int launcherX = (screenW - launcherW) / 2;
    int launcherY = (screenH - launcherH) / 2 - 50;

    FillRoundRect(g, launcherX, launcherY, launcherW, launcherH, 24, Color(240, 26, 26, 36));
    DrawRoundRect(g, launcherX, launcherY, launcherW, launcherH, 24, ColorFromRGB(Colors::Border), 2.0f);

    // Search bar
    int searchBarX = launcherX + 24;
    int searchBarY = launcherY + 24;
    int searchBarW = launcherW - 48;
    int searchBarH = 48;

    FillRoundRect(g, searchBarX, searchBarY, searchBarW, searchBarH, 12, ColorFromRGB(Colors::SurfaceLight));
    DrawRoundRect(g, searchBarX, searchBarY, searchBarW, searchBarH, 12, ColorFromRGB(Colors::Border), 1.0f);

    FontFamily fontFamily(L"Segoe UI");
    Font searchFont(&fontFamily, 16, FontStyleRegular, UnitPixel);
    SolidBrush placeholderBrush(ColorFromRGB(Colors::TextDim));

    StringFormat format;
    format.SetAlignment(StringAlignmentNear);
    format.SetLineAlignment(StringAlignmentCenter);

    RectF searchRect((REAL)searchBarX + 48, (REAL)searchBarY, (REAL)searchBarW - 64, (REAL)searchBarH);
    g.DrawString(L"Search applications...", -1, &searchFont, searchRect, &format, &placeholderBrush);

    // Search icon
    Font iconFont(&fontFamily, 18, FontStyleRegular, UnitPixel);
    RectF iconRect((REAL)searchBarX + 16, (REAL)searchBarY, 24, (REAL)searchBarH);
    g.DrawString(L"🔍", -1, &iconFont, iconRect, &format, &placeholderBrush);

    // App grid
    int gridX = launcherX + 24;
    int gridY = launcherY + 100;
    int cols = 5;
    int cellW = 120;
    int cellH = 100;
    int spacing = 16;

    const wchar_t* appNames[] = {
        L"Files", L"Browser", L"Terminal", L"NEXUS IDE", L"Settings",
        L"Store", L"Photos", L"Music", L"Mail", L"Calendar",
        L"Notes", L"Calculator", L"Camera", L"Maps", L"Weather"
    };

    const wchar_t* appIcons[] = {
        L"📁", L"🌐", L"💻", L"⚡", L"⚙",
        L"🛒", L"📷", L"🎵", L"📧", L"📅",
        L"📝", L"🧮", L"📸", L"🗺", L"🌤"
    };

    Color appColors[] = {
        Color(255, 59, 130, 246), Color(255, 34, 197, 94), Color(255, 139, 92, 246),
        Color(255, 249, 115, 22), Color(255, 107, 114, 128), Color(255, 6, 182, 212),
        Color(255, 236, 72, 153), Color(255, 168, 85, 247), Color(255, 234, 179, 8),
        Color(255, 239, 68, 68), Color(255, 251, 191, 36), Color(255, 45, 212, 191),
        Color(255, 244, 114, 182), Color(255, 52, 211, 153), Color(255, 253, 224, 71)
    };

    Font appFont(&fontFamily, 28, FontStyleRegular, UnitPixel);
    Font nameFont(&fontFamily, 12, FontStyleRegular, UnitPixel);
    SolidBrush textBrush(ColorFromRGB(Colors::Text));
    SolidBrush nameBrush(ColorFromRGB(Colors::TextDim));

    for (int i = 0; i < 15; i++) {
        int row = i / cols;
        int col = i % cols;
        int x = gridX + col * (cellW + spacing);
        int y = gridY + row * (cellH + spacing);

        // App icon
        FillRoundRect(g, x + 10, y, 72, 72, 16, appColors[i]);

        StringFormat centerFormat;
        centerFormat.SetAlignment(StringAlignmentCenter);
        centerFormat.SetLineAlignment(StringAlignmentCenter);

        RectF iconRect((REAL)x + 10, (REAL)y, 72, 72);
        g.DrawString(appIcons[i], -1, &appFont, iconRect, &centerFormat, &textBrush);

        // App name
        RectF nameRect((REAL)x, (REAL)y + 76, (REAL)cellW, 20);
        g.DrawString(appNames[i], -1, &nameFont, nameRect, &centerFormat, &nameBrush);
    }
}

// ============================================================================
// DESKTOP WINDOW
// ============================================================================

LRESULT CALLBACK DesktopProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        UpdateWidgets();
        
        // Start loading wallpaper from URL asynchronously
        StartWallpaperLoadAsync();
        
        // Timer for clock update
        SetTimer(hWnd, 1, 1000, nullptr);
        break;
    }

    case WM_TIMER: {
        if (wParam == 1) {
            UpdateWidgets();
            InvalidateRect(hWnd, nullptr, FALSE);
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        Graphics graphics(hdc);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        // Draw wallpaper (photo background from URL)
        DrawWallpaper(graphics, screenW, screenH);

        // Draw widgets
        for (size_t i = 0; i < g_widgets.size(); i++) {
            DrawWidget(graphics, g_widgets[i], g_hoveredWidget == (int)i);
        }

        // Draw dock
        DrawDock(graphics, screenW, screenH);

        // Draw launcher
        DrawLauncher(graphics, screenW, screenH);

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_MOUSEMOVE: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        bool needUpdate = false;

        // Check dock hover
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();
        int iconSize = 52;
        int padding = 12;
        int totalWidth = (int)g_dockApps.size() * (iconSize + 8) - 8 + padding * 2;
        int dockX = (screenW - totalWidth) / 2;
        int dockY = screenH - iconSize - padding * 2 - 20;

        for (size_t i = 0; i < g_dockApps.size(); i++) {
            bool wasHovered = g_dockApps[i].hovered;
            g_dockApps[i].hovered = PtInRect(&g_dockApps[i].bounds, { x, y });
            if (wasHovered != g_dockApps[i].hovered) needUpdate = true;
        }

        // Check widget hover
        for (size_t i = 0; i < g_widgets.size(); i++) {
            bool wasHovered = (g_hoveredWidget == (int)i);
            bool nowHovered = PtInRect(&g_widgets[i].bounds, { x, y });
            if (wasHovered != nowHovered) {
                g_hoveredWidget = nowHovered ? (int)i : -1;
                needUpdate = true;
            }
        }

        if (needUpdate) InvalidateRect(hWnd, nullptr, FALSE);
        break;
    }

    case WM_LBUTTONDOWN: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        // Check dock clicks
        for (size_t i = 0; i < g_dockApps.size(); i++) {
            if (PtInRect(&g_dockApps[i].bounds, { x, y })) {
                // Launch app
                ShellExecute(nullptr, L"open", g_dockApps[i].executable.c_str(), nullptr, nullptr, SW_SHOW);
                break;
            }
        }

        // Toggle launcher on empty space click
        if (y < GetScreenHeight() - 100 && !g_showLauncher) {
            g_showLauncher = true;
            InvalidateRect(hWnd, nullptr, FALSE);
        }
        break;
    }

    case WM_RBUTTONDOWN: {
        if (g_showLauncher) {
            g_showLauncher = false;
            InvalidateRect(hWnd, nullptr, FALSE);
        }
        break;
    }

    case WM_KEYDOWN: {
        if (wParam == VK_ESCAPE && g_showLauncher) {
            g_showLauncher = false;
            InvalidateRect(hWnd, nullptr, FALSE);
        }
        break;
    }

    case WM_DESTROY:
        KillTimer(hWnd, 1);
        // Clean up wallpaper bitmap
        if (g_wallpaperBitmap) {
            delete g_wallpaperBitmap;
            g_wallpaperBitmap = nullptr;
        }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// ============================================================================
// MAIN
// ============================================================================

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nShow) {
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, nullptr);

    // Register window class
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = DesktopProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = L"VORTEX_Desktop";

    RegisterClassExW(&wc);

    // Create fullscreen window
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    g_hDesktop = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"VORTEX_Desktop", L"VORTEX Desktop",
        WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN,
        0, 0, screenW, screenH,
        nullptr, nullptr, hInst, nullptr
    );

    // Hide taskbar
    HWND hTaskbar = FindWindow(L"Shell_TrayWnd", nullptr);
    if (hTaskbar) ShowWindow(hTaskbar, SW_HIDE);

    ShowWindow(g_hDesktop, nShow);
    UpdateWindow(g_hDesktop);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Restore taskbar
    if (hTaskbar) ShowWindow(hTaskbar, SW_SHOW);

    GdiplusShutdown(g_gdiplusToken);
    return (int)msg.wParam;
}

int main() {
    return wWinMain(GetModuleHandle(nullptr), nullptr, nullptr, SW_SHOW);
}
