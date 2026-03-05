/*
 * VORTEX Desktop Environment - Fixed for MSVC
 */

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <urlmon.h>
#include <string>
#include <vector>
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
#pragma comment(lib, "urlmon.lib")

#include <gdiplus.h>
using namespace Gdiplus;

const wchar_t* WALLPAPER_URL = L"https://images.unsplash.com/photo-1534796636912-3b95b3ab5986?w=1920&q=80";
const wchar_t* WALLPAPER_CACHE_PATH = L"vortex_wallpaper_cache.jpg";

namespace Colors {
    const COLORREF SurfaceLight = 0x2a2a3a;
    const COLORREF Text = 0xffffff;
    const COLORREF TextDim = 0x9ca3af;
    const COLORREF Border = 0x2a2a3a;
}

Color ColorFromRGB(COLORREF clr, BYTE alpha = 255) {
    return Color(alpha, GetRValue(clr), GetGValue(clr), GetBValue(clr));
}

int GetScreenWidth() { return GetSystemMetrics(SM_CXSCREEN); }
int GetScreenHeight() { return GetSystemMetrics(SM_CYSCREEN); }

void FillRoundRect(Graphics& g, int x, int y, int w, int h, int radius, Color color) {
    SolidBrush brush(color);
    GraphicsPath path;
    int r2 = radius * 2;
    path.AddArc(x, y, r2, r2, 180, 90);
    path.AddArc(x + w - r2, y, r2, r2, 270, 90);
    path.AddArc(x + w - r2, y + h - r2, r2, r2, 0, 90);
    path.AddArc(x, y + h - r2, r2, r2, 90, 90);
    path.CloseFigure();
    g.FillPath(&brush, &path);
}

void DrawRoundRect(Graphics& g, int x, int y, int w, int h, int radius, Color color, float width = 1.0f) {
    Pen pen(color, width);
    GraphicsPath path;
    int r2 = radius * 2;
    path.AddArc(x, y, r2, r2, 180, 90);
    path.AddArc(x + w - r2, y, r2, r2, 270, 90);
    path.AddArc(x + w - r2, y + h - r2, r2, r2, 0, 90);
    path.AddArc(x, y + h - r2, r2, r2, 90, 90);
    path.CloseFigure();
    g.DrawPath(&pen, &path);
}

struct AppIcon {
    std::wstring name;
    std::wstring executable;
    Color color;
    bool hovered;
    RECT bounds;
};

std::vector<AppIcon> g_dockApps;

void InitDockApps() {
    g_dockApps.clear();
    AppIcon app;
    
    app.hovered = false;
    memset(&app.bounds, 0, sizeof(RECT));
    
    app.name = L"Files"; app.executable = L"explorer.exe"; app.color = Color(255, 59, 130, 246);
    g_dockApps.push_back(app);
    
    app.name = L"Browser"; app.executable = L"msedge.exe"; app.color = Color(255, 34, 197, 94);
    g_dockApps.push_back(app);
    
    app.name = L"Terminal"; app.executable = L"cmd.exe"; app.color = Color(255, 139, 92, 246);
    g_dockApps.push_back(app);
    
    app.name = L"NEXUS IDE"; app.executable = L"notepad.exe"; app.color = Color(255, 249, 115, 22);
    g_dockApps.push_back(app);
    
    app.name = L"Settings"; app.executable = L"ms-settings:"; app.color = Color(255, 107, 114, 128);
    g_dockApps.push_back(app);
    
    app.name = L"Store"; app.executable = L"ms-windows-store:"; app.color = Color(255, 6, 182, 212);
    g_dockApps.push_back(app);
    
    app.name = L"Photos"; app.executable = L"ms-photos:"; app.color = Color(255, 236, 72, 153);
    g_dockApps.push_back(app);
    
    app.name = L"Music"; app.executable = L"wmplayer.exe"; app.color = Color(255, 168, 85, 247);
    g_dockApps.push_back(app);
}

struct Widget {
    std::wstring title;
    std::wstring content;
    RECT bounds;
};

std::vector<Widget> g_widgets;

void UpdateWidgets() {
    g_widgets.clear();
    int screenW = GetScreenWidth();

    Widget timeWidget;
    timeWidget.title = L"TIME";
    SetRect(&timeWidget.bounds, screenW - 320, 40, screenW - 20, 180);

    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);

    wchar_t timeStr[64], dateStr[64];
    wcsftime(timeStr, 64, L"%H:%M", &timeinfo);
    wcsftime(dateStr, 64, L"%A, %B %d", &timeinfo);
    timeWidget.content = std::wstring(timeStr) + L"\n" + dateStr;
    g_widgets.push_back(timeWidget);

    Widget sysWidget;
    sysWidget.title = L"SYSTEM";
    SetRect(&sysWidget.bounds, screenW - 320, 200, screenW - 20, 340);

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);

    wchar_t sysStr[256];
    swprintf_s(sysStr, L"CPU: %d%%\nRAM: %d%%\nDisk: 45%%", 35, (int)memStatus.dwMemoryLoad);
    sysWidget.content = sysStr;
    g_widgets.push_back(sysWidget);

    Widget weatherWidget;
    weatherWidget.title = L"WEATHER";
    SetRect(&weatherWidget.bounds, screenW - 320, 360, screenW - 20, 480);
    weatherWidget.content = L"22 C\nSunny\nMoscow, RU";
    g_widgets.push_back(weatherWidget);
}

HWND g_hDesktop = NULL;
bool g_showLauncher = false;
int g_hoveredWidget = -1;
ULONG_PTR g_gdiplusToken = 0;
Bitmap* g_wallpaperBitmap = NULL;
bool g_wallpaperLoaded = false;
bool g_wallpaperLoading = false;

std::wstring GetWallpaperCachePath() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring exeDir = exePath;
    size_t lastSlash = exeDir.find_last_of(L"\\");
    if (lastSlash != std::wstring::npos) exeDir = exeDir.substr(0, lastSlash);
    return exeDir + L"\\" + WALLPAPER_CACHE_PATH;
}

bool DownloadWallpaperFromURL(const wchar_t* url, const wchar_t* localPath) {
    HRESULT hr = URLDownloadToFileW(NULL, url, localPath, 0, NULL);
    return SUCCEEDED(hr);
}

bool LoadWallpaperFromFile(const wchar_t* filePath) {
    if (g_wallpaperBitmap) { delete g_wallpaperBitmap; g_wallpaperBitmap = NULL; }
    if (GetFileAttributesW(filePath) == INVALID_FILE_ATTRIBUTES) return false;
    g_wallpaperBitmap = new Bitmap(filePath);
    if (g_wallpaperBitmap->GetLastStatus() != Ok) {
        delete g_wallpaperBitmap;
        g_wallpaperBitmap = NULL;
        return false;
    }
    return true;
}

bool LoadWallpaperFromURL(const wchar_t* url) {
    std::wstring cachePath = GetWallpaperCachePath();
    if (DownloadWallpaperFromURL(url, cachePath.c_str())) {
        return LoadWallpaperFromFile(cachePath.c_str());
    }
    return false;
}

DWORD WINAPI LoadWallpaperThread(LPVOID param) {
    g_wallpaperLoading = true;
    if (LoadWallpaperFromURL(WALLPAPER_URL)) {
        g_wallpaperLoaded = true;
        if (g_hDesktop) InvalidateRect(g_hDesktop, NULL, FALSE);
    }
    g_wallpaperLoading = false;
    return 0;
}

void StartWallpaperLoadAsync() {
    CreateThread(NULL, 0, LoadWallpaperThread, NULL, 0, NULL);
}

void DrawWallpaper(Graphics& g, int screenW, int screenH) {
    if (g_wallpaperBitmap && g_wallpaperLoaded) {
        REAL imgW = (REAL)g_wallpaperBitmap->GetWidth();
        REAL imgH = (REAL)g_wallpaperBitmap->GetHeight();
        REAL scaleX = (REAL)screenW / imgW;
        REAL scaleY = (REAL)screenH / imgH;
        REAL scale = (scaleX > scaleY) ? scaleX : scaleY;
        REAL newW = imgW * scale;
        REAL newH = imgH * scale;
        REAL offsetX = ((REAL)screenW - newW) / 2.0f;
        REAL offsetY = ((REAL)screenH - newH) / 2.0f;
        g.DrawImage(g_wallpaperBitmap, offsetX, offsetY, newW, newH);
    }
    else {
        LinearGradientBrush wallBrush(Point(0, 0), Point(screenW, screenH),
            Color(255, 15, 23, 42), Color(255, 30, 41, 59));
        g.FillRectangle(&wallBrush, 0, 0, screenW, screenH);

        for (int i = 0; i < 5; i++) {
            int cx = 200 + i * 300;
            int cy = 200 + (i % 3) * 150;
            int cr = 100 + i * 50;
            SolidBrush circleBrush(Color(10, 99, 102, 241));
            g.FillEllipse(&circleBrush, cx - cr, cy - cr, cr * 2, cr * 2);
        }

        if (g_wallpaperLoading) {
            FontFamily fontFamily(L"Segoe UI");
            Font loadingFont(&fontFamily, 14.0f, FontStyleRegular, UnitPixel);
            SolidBrush loadingBrush(Color(200, 255, 255, 255));
            StringFormat format;
            format.SetAlignment(StringAlignmentCenter);
            format.SetLineAlignment(StringAlignmentCenter);
            RectF loadingRect;
            loadingRect.X = (REAL)(screenW / 2 - 100);
            loadingRect.Y = (REAL)(screenH / 2 - 20);
            loadingRect.Width = 200.0f;
            loadingRect.Height = 40.0f;
            g.DrawString(L"Loading wallpaper...", -1, &loadingFont, loadingRect, &format, &loadingBrush);
        }
    }
}

void DrawWidget(Graphics& g, const Widget& widget, bool hovered) {
    int x = widget.bounds.left;
    int y = widget.bounds.top;
    int w = widget.bounds.right - widget.bounds.left;
    int h = widget.bounds.bottom - widget.bounds.top;

    Color bgColor = hovered ? Color(40, 26, 26, 36) : Color(30, 26, 26, 36);
    FillRoundRect(g, x, y, w, h, 16, bgColor);
    DrawRoundRect(g, x, y, w, h, 16, ColorFromRGB(Colors::Border), 1.0f);

    FontFamily fontFamily(L"Segoe UI");
    Font titleFont(&fontFamily, 11.0f, FontStyleRegular, UnitPixel);
    Font contentFont(&fontFamily, 24.0f, FontStyleRegular, UnitPixel);
    Font smallFont(&fontFamily, 14.0f, FontStyleRegular, UnitPixel);

    SolidBrush titleBrush(ColorFromRGB(Colors::TextDim));
    SolidBrush textBrush(ColorFromRGB(Colors::Text));

    StringFormat format;
    format.SetAlignment(StringAlignmentNear);
    format.SetLineAlignment(StringAlignmentNear);

    RectF titleRect;
    titleRect.X = (REAL)(x + 16);
    titleRect.Y = (REAL)(y + 12);
    titleRect.Width = (REAL)(w - 32);
    titleRect.Height = 20.0f;
    g.DrawString(widget.title.c_str(), -1, &titleFont, titleRect, &format, &titleBrush);

    std::vector<std::wstring> lines;
    std::wistringstream stream(widget.content);
    std::wstring line;
    while (std::getline(stream, line)) lines.push_back(line);

    if (widget.title == L"TIME" && lines.size() > 0) {
        RectF timeRect;
        timeRect.X = (REAL)(x + 16);
        timeRect.Y = (REAL)(y + 40);
        timeRect.Width = (REAL)(w - 32);
        timeRect.Height = 50.0f;
        g.DrawString(lines[0].c_str(), -1, &contentFont, timeRect, &format, &textBrush);

        if (lines.size() > 1) {
            RectF dateRect;
            dateRect.X = (REAL)(x + 16);
            dateRect.Y = (REAL)(y + 90);
            dateRect.Width = (REAL)(w - 32);
            dateRect.Height = 24.0f;
            g.DrawString(lines[1].c_str(), -1, &smallFont, dateRect, &format, &titleBrush);
        }
    }
    else {
        REAL lineY = (REAL)(y + 40);
        for (size_t j = 0; j < lines.size(); j++) {
            RectF lineRect;
            lineRect.X = (REAL)(x + 16);
            lineRect.Y = lineY;
            lineRect.Width = (REAL)(w - 32);
            lineRect.Height = 24.0f;
            g.DrawString(lines[j].c_str(), -1, &smallFont, lineRect, &format, &textBrush);
            lineY += 26.0f;
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

    FillRoundRect(g, dockX, dockY, totalWidth, dockHeight, 20, Color(180, 26, 26, 36));
    DrawRoundRect(g, dockX, dockY, totalWidth, dockHeight, 20, ColorFromRGB(Colors::Border), 1.5f);

    int iconX = dockX + padding;
    const wchar_t* letters[] = { L"F", L"B", L"T", L"N", L"S", L"St", L"P", L"M" };

    FontFamily fontFamily(L"Segoe UI");

    for (size_t i = 0; i < g_dockApps.size(); i++) {
        AppIcon& app = g_dockApps[i];
        float scale = app.hovered ? 1.25f : 1.0f;
        int scaledSize = (int)(iconSize * scale);
        int offsetX = (scaledSize - iconSize) / 2;
        int offsetY = (scaledSize - iconSize) / 2;

        int px = iconX - offsetX;
        int py = dockY + padding - offsetY;

        SetRect(&app.bounds, px, py, px + scaledSize, py + scaledSize);

        // Draw colored background
        SolidBrush bgBrush(app.color);
        g.FillRectangle(&bgBrush, px, py, scaledSize, scaledSize);

        // Draw icon text
        Font iconFont(&fontFamily, app.hovered ? 20.0f : 16.0f, FontStyleBold, UnitPixel);
        SolidBrush textBrush(Color(255, 255, 255, 255));

        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);

        RectF textRect;
        textRect.X = (REAL)px;
        textRect.Y = (REAL)py;
        textRect.Width = (REAL)scaledSize;
        textRect.Height = (REAL)scaledSize;
        g.DrawString(letters[i], -1, &iconFont, textRect, &format, &textBrush);

        // Draw indicator
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

    SolidBrush overlayBrush(Color(200, 10, 10, 15));
    g.FillRectangle(&overlayBrush, 0, 0, screenW, screenH);

    int launcherW = 700;
    int launcherH = 500;
    int launcherX = (screenW - launcherW) / 2;
    int launcherY = (screenH - launcherH) / 2 - 50;

    FillRoundRect(g, launcherX, launcherY, launcherW, launcherH, 24, Color(240, 26, 26, 36));
    DrawRoundRect(g, launcherX, launcherY, launcherW, launcherH, 24, ColorFromRGB(Colors::Border), 2.0f);

    int searchBarX = launcherX + 24;
    int searchBarY = launcherY + 24;
    int searchBarW = launcherW - 48;
    int searchBarH = 48;

    FillRoundRect(g, searchBarX, searchBarY, searchBarW, searchBarH, 12, ColorFromRGB(Colors::SurfaceLight));
    DrawRoundRect(g, searchBarX, searchBarY, searchBarW, searchBarH, 12, ColorFromRGB(Colors::Border), 1.0f);

    FontFamily fontFamily(L"Segoe UI");
    Font searchFont(&fontFamily, 16.0f, FontStyleRegular, UnitPixel);
    SolidBrush placeholderBrush(ColorFromRGB(Colors::TextDim));

    StringFormat format;
    format.SetAlignment(StringAlignmentNear);
    format.SetLineAlignment(StringAlignmentCenter);

    RectF searchRect;
    searchRect.X = (REAL)(searchBarX + 48);
    searchRect.Y = (REAL)searchBarY;
    searchRect.Width = (REAL)(searchBarW - 64);
    searchRect.Height = (REAL)searchBarH;
    g.DrawString(L"Search applications...", -1, &searchFont, searchRect, &format, &placeholderBrush);

    int gridX = launcherX + 24;
    int gridY = launcherY + 100;
    int cols = 5;
    int cellW = 120;
    int cellH = 100;
    int spacing = 16;

    const wchar_t* appNames[] = {
        L"Files", L"Browser", L"Terminal", L"IDE", L"Settings",
        L"Store", L"Photos", L"Music", L"Mail", L"Calendar",
        L"Notes", L"Calc", L"Camera", L"Maps", L"Weather"
    };

    Color appColors[] = {
        Color(255, 59, 130, 246), Color(255, 34, 197, 94), Color(255, 139, 92, 246),
        Color(255, 249, 115, 22), Color(255, 107, 114, 128), Color(255, 6, 182, 212),
        Color(255, 236, 72, 153), Color(255, 168, 85, 247), Color(255, 234, 179, 8),
        Color(255, 239, 68, 68), Color(255, 251, 191, 36), Color(255, 45, 212, 191),
        Color(255, 244, 114, 182), Color(255, 52, 211, 153), Color(255, 253, 224, 71)
    };

    Font appFont(&fontFamily, 18.0f, FontStyleBold, UnitPixel);
    Font nameFont(&fontFamily, 12.0f, FontStyleRegular, UnitPixel);
    SolidBrush textBrush(ColorFromRGB(Colors::Text));
    SolidBrush nameBrush(ColorFromRGB(Colors::TextDim));

    for (int i = 0; i < 15; i++) {
        int row = i / cols;
        int col = i % cols;
        int px = gridX + col * (cellW + spacing);
        int py = gridY + row * (cellH + spacing);

        SolidBrush bgBrush(appColors[i]);
        g.FillRectangle(&bgBrush, px + 10, py, 72, 72);

        StringFormat centerFormat;
        centerFormat.SetAlignment(StringAlignmentCenter);
        centerFormat.SetLineAlignment(StringAlignmentCenter);

        RectF iconRect;
        iconRect.X = (REAL)(px + 10);
        iconRect.Y = (REAL)py;
        iconRect.Width = 72.0f;
        iconRect.Height = 72.0f;
        g.DrawString(appNames[i], -1, &appFont, iconRect, &centerFormat, &textBrush);

        RectF nameRect;
        nameRect.X = (REAL)px;
        nameRect.Y = (REAL)(py + 76);
        nameRect.Width = (REAL)cellW;
        nameRect.Height = 20.0f;
        g.DrawString(appNames[i], -1, &nameFont, nameRect, &centerFormat, &nameBrush);
    }
}

LRESULT CALLBACK DesktopProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        InitDockApps();
        UpdateWidgets();
        StartWallpaperLoadAsync();
        SetTimer(hWnd, 1, 1000, NULL);
        break;

    case WM_TIMER:
        if (wParam == 1) {
            UpdateWidgets();
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        Graphics graphics(hdc);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        DrawWallpaper(graphics, screenW, screenH);
        for (size_t i = 0; i < g_widgets.size(); i++) {
            DrawWidget(graphics, g_widgets[i], g_hoveredWidget == (int)i);
        }
        DrawDock(graphics, screenW, screenH);
        DrawLauncher(graphics, screenW, screenH);
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_MOUSEMOVE: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        bool needUpdate = false;

        POINT pt;
        pt.x = x;
        pt.y = y;

        for (size_t i = 0; i < g_dockApps.size(); i++) {
            bool wasHovered = g_dockApps[i].hovered;
            g_dockApps[i].hovered = (PtInRect(&g_dockApps[i].bounds, pt) != FALSE);
            if (wasHovered != g_dockApps[i].hovered) needUpdate = true;
        }

        for (size_t i = 0; i < g_widgets.size(); i++) {
            bool wasHovered = (g_hoveredWidget == (int)i);
            bool nowHovered = (PtInRect(&g_widgets[i].bounds, pt) != FALSE);
            if (wasHovered != nowHovered) {
                g_hoveredWidget = nowHovered ? (int)i : -1;
                needUpdate = true;
            }
        }

        if (needUpdate) InvalidateRect(hWnd, NULL, FALSE);
        break;
    }

    case WM_LBUTTONDOWN: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        POINT pt;
        pt.x = x;
        pt.y = y;

        for (size_t i = 0; i < g_dockApps.size(); i++) {
            if (PtInRect(&g_dockApps[i].bounds, pt)) {
                ShellExecute(NULL, L"open", g_dockApps[i].executable.c_str(), NULL, NULL, SW_SHOW);
                break;
            }
        }

        if (y < GetScreenHeight() - 100 && !g_showLauncher) {
            g_showLauncher = true;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    }

    case WM_RBUTTONDOWN:
        if (g_showLauncher) {
            g_showLauncher = false;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE && g_showLauncher) {
            g_showLauncher = false;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;

    case WM_DESTROY:
        KillTimer(hWnd, 1);
        if (g_wallpaperBitmap) { delete g_wallpaperBitmap; g_wallpaperBitmap = NULL; }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nShow) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASSEXW wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = DesktopProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = L"VORTEX_Desktop";

    RegisterClassExW(&wc);

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    g_hDesktop = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"VORTEX_Desktop", L"VORTEX Desktop",
        WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN,
        0, 0, screenW, screenH,
        NULL, NULL, hInst, NULL);

    HWND hTaskbar = FindWindow(L"Shell_TrayWnd", NULL);
    if (hTaskbar) ShowWindow(hTaskbar, SW_HIDE);

    ShowWindow(g_hDesktop, nShow);
    UpdateWindow(g_hDesktop);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (hTaskbar) ShowWindow(hTaskbar, SW_SHOW);

    GdiplusShutdown(g_gdiplusToken);
    return (int)msg.wParam;
}

int main() {
    return wWinMain(GetModuleHandle(NULL), NULL, NULL, SW_SHOW);
}
