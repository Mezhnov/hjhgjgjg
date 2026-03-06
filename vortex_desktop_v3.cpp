/*
 * VORTEX Desktop Environment v3.0 - Windows 11 Fluent Design
 * Complete redesign with Windows 10/11 aesthetics
 *
 * Features:
 * - Fluent Design / Mica-like effects
 * - Draggable desktop icons with grid snapping
 * - Windows 11 style taskbar (centered)
 * - Windows 11 style Start Menu
 * - Context menu (RMB) with Win11 style
 * - System tray with clock, WiFi, volume, battery
 * - Desktop icons: This PC, Recycle Bin, Documents, Downloads, etc.
 * - Folder creation via context menu
 * - WiFi panel
 * - Notification center
 * - Selection rectangle on desktop
 *
 * Controls:
 * ESC       - exit / close
 * SPACE     - toggle Start Menu
 * M         - toggle music
 * W         - toggle widgets
 * N         - new notification
 * F         - toggle WiFi panel
 */

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <urlmon.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <math.h>
#include <random>
#include <map>
#include <functional>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dwmapi.lib")

#include <gdiplus.h>
using namespace Gdiplus;

// ============================================================================
//  WINDOWS 11 COLOR PALETTE
// ============================================================================
namespace W11 {
    // Accent colors (Windows 11 default blue)
    const Color Accent(255, 0, 103, 192);
    const Color AccentLight(255, 0, 120, 212);
    const Color AccentDark(255, 0, 84, 153);
    const Color AccentSubtle(40, 0, 103, 192);

    // Surface colors (Dark mode)
    const Color SurfaceBase(255, 32, 32, 32);
    const Color SurfaceCard(230, 44, 44, 44);
    const Color SurfaceCardHover(240, 55, 55, 55);
    const Color SurfaceFlyout(245, 44, 44, 44);
    const Color SurfaceOverlay(180, 0, 0, 0);
    const Color SurfaceStroke(60, 255, 255, 255);
    const Color SurfaceStrokeLight(30, 255, 255, 255);

    // Mica / Acrylic
    const Color MicaBg(220, 32, 32, 32);
    const Color AcrylicBg(200, 44, 44, 44);
    const Color AcrylicBgLight(180, 55, 55, 55);

    // Text
    const Color TextPrimary(255, 255, 255, 255);
    const Color TextSecondary(255, 200, 200, 200);
    const Color TextTertiary(255, 150, 150, 150);
    const Color TextDisabled(255, 100, 100, 100);

    // Status
    const Color Success(255, 108, 203, 95);
    const Color Warning(255, 252, 225, 0);
    const Color Error(255, 255, 99, 97);
    const Color Info(255, 98, 205, 255);

    // Taskbar
    const Color TaskbarBg(235, 32, 32, 32);
    const Color TaskbarHover(255, 55, 55, 55);
    const Color TaskbarActive(255, 65, 65, 65);
    const Color TaskbarIndicator(255, 76, 194, 255);

    // Start Menu
    const Color StartBg(240, 38, 38, 38);
    const Color StartSearch(255, 55, 55, 55);
    const Color StartSearchHover(255, 65, 65, 65);
    const Color StartItemHover(255, 60, 60, 60);

    // Context Menu
    const Color CtxBg(245, 44, 44, 44);
    const Color CtxHover(255, 60, 60, 60);
    const Color CtxSep(40, 255, 255, 255);

    // Desktop
    const Color SelectionRect(80, 0, 103, 192);
    const Color SelectionBorder(200, 0, 103, 192);
    const Color IconSelectedBg(60, 0, 103, 192);
    const Color IconHoverBg(40, 255, 255, 255);
}

// ============================================================================
//  CONSTANTS
// ============================================================================
const wchar_t* WALLPAPER_URL = L"https://images.wallpaperscraft.com/image/single/lake_mountains_trees_1219008_1920x1080.jpg";
const wchar_t* WALLPAPER_CACHE = L"vortex_wallpaper_cache.jpg";

const int TASKBAR_HEIGHT = 48;
const int TASKBAR_ICON_SIZE = 40;
const int TASKBAR_CORNER_RADIUS = 0; // Win11 taskbar has no visible corners (full width)

const int DESKTOP_ICON_W = 76;
const int DESKTOP_ICON_H = 86;
const int DESKTOP_GRID_X = 80;
const int DESKTOP_GRID_Y = 90;
const int DESKTOP_MARGIN_X = 12;
const int DESKTOP_MARGIN_Y = 12;

const int START_MENU_W = 620;
const int START_MENU_H = 580;
const int START_MENU_RADIUS = 8;

const int CTX_MENU_W = 260;
const int CTX_ITEM_H = 36;
const int CTX_SEP_H = 9;
const int CTX_RADIUS = 8;

const int WIFI_PANEL_W = 360;
const int WIFI_PANEL_H = 420;

const int NOTIF_W = 360;
const int NOTIF_H = 80;

const int WIDGET_W = 320;

// ============================================================================
//  GLOBAL STATE
// ============================================================================
HWND g_hWnd = NULL;
ULONG_PTR g_gdiplusToken = 0;
DWORD g_tick = 0;

// Double buffer
HDC g_memDC = NULL;
HBITMAP g_memBmp = NULL, g_oldBmp = NULL;
void* g_bits = NULL;
int g_bufW = 0, g_bufH = 0;

// Wallpaper
Bitmap* g_wallpaper = NULL;
bool g_wallpaperReady = false;
bool g_wallpaperLoading = false;

// UI state
bool g_startMenuOpen = false;
bool g_widgetsVisible = false;
bool g_musicPlaying = false;
bool g_wifiPanelOpen = false;
bool g_contextMenuOpen = false;
bool g_calendarOpen = false;

int g_hoveredTaskbarIcon = -1;
int g_hoveredStartItem = -1;
int g_hoveredDesktopIcon = -1;
int g_hoveredWifiItem = -1;
int g_contextMenuX = 0, g_contextMenuY = 0;
int g_hoveredContextItem = -1;

// Animations
float g_startMenuAnim = 0.0f;
float g_widgetsAnim = 0.0f;
float g_wifiPanelAnim = 0.0f;
float g_contextMenuAnim = 0.0f;
float g_calendarAnim = 0.0f;

// Desktop selection
bool g_selecting = false;
int g_selStartX = 0, g_selStartY = 0;
int g_selEndX = 0, g_selEndY = 0;

// Desktop icon dragging
bool g_dragging = false;
int g_dragIconIdx = -1;
int g_dragOffsetX = 0, g_dragOffsetY = 0;
int g_dragCurrentX = 0, g_dragCurrentY = 0;
bool g_dragStarted = false;

// ============================================================================
//  DESKTOP ICONS
// ============================================================================
struct DesktopIcon {
    std::wstring name;
    std::wstring iconSymbol;
    std::wstring action;
    int gridCol, gridRow;
    int pixelX, pixelY;
    bool selected;
    Color iconColor;
    int iconType; // 0=thisPC, 1=recycle, 2=folder, 3=file, 4=shortcut, 5=user_folder
};
std::vector<DesktopIcon> g_desktopIcons;

void CalcIconPixelPos(DesktopIcon& d) {
    d.pixelX = DESKTOP_MARGIN_X + d.gridCol * DESKTOP_GRID_X;
    d.pixelY = DESKTOP_MARGIN_Y + d.gridRow * DESKTOP_GRID_Y;
}

void SnapToGrid(DesktopIcon& d, int px, int py) {
    d.gridCol = (std::max)(0, (px - DESKTOP_MARGIN_X + DESKTOP_GRID_X / 2) / DESKTOP_GRID_X);
    d.gridRow = (std::max)(0, (py - DESKTOP_MARGIN_Y + DESKTOP_GRID_Y / 2) / DESKTOP_GRID_Y);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    int maxRow = (sh - TASKBAR_HEIGHT - DESKTOP_MARGIN_Y * 2) / DESKTOP_GRID_Y;
    if (d.gridRow >= maxRow) d.gridRow = maxRow - 1;
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int maxCol = (sw - DESKTOP_MARGIN_X * 2) / DESKTOP_GRID_X;
    if (d.gridCol >= maxCol) d.gridCol = maxCol - 1;
    CalcIconPixelPos(d);
}

bool IsGridOccupied(int col, int row, int excludeIdx = -1) {
    for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
        if (i == excludeIdx) continue;
        if (g_desktopIcons[i].gridCol == col && g_desktopIcons[i].gridRow == row) return true;
    }
    return false;
}

void FindFreeGrid(int& col, int& row) {
    int sh = GetSystemMetrics(SM_CYSCREEN);
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int maxRow = (sh - TASKBAR_HEIGHT - DESKTOP_MARGIN_Y * 2) / DESKTOP_GRID_Y;
    int maxCol = (sw - DESKTOP_MARGIN_X * 2) / DESKTOP_GRID_X;
    for (int c = 0; c < maxCol; c++) {
        for (int r = 0; r < maxRow; r++) {
            if (!IsGridOccupied(c, r)) {
                col = c; row = r; return;
            }
        }
    }
    col = 0; row = 0;
}

void InitDesktopIcons() {
    g_desktopIcons.clear();
    auto add = [](const std::wstring& name, const std::wstring& ico, const std::wstring& act,
        int col, int row, Color c, int type) {
            DesktopIcon d;
            d.name = name; d.iconSymbol = ico; d.action = act;
            d.gridCol = col; d.gridRow = row;
            d.selected = false; d.iconColor = c; d.iconType = type;
            CalcIconPixelPos(d);
            g_desktopIcons.push_back(d);
        };

    add(L"This PC", L"PC", L"shell:::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", 0, 0,
        Color(255, 255, 200, 50), 0);
    add(L"Recycle Bin", L"BIN", L"shell:RecycleBinFolder", 0, 1,
        Color(255, 160, 160, 160), 1);
    add(L"Documents", L"DOC", L"shell:Personal", 0, 2,
        Color(255, 255, 210, 76), 5);
    add(L"Downloads", L"DL", L"shell:Downloads", 0, 3,
        Color(255, 76, 194, 255), 5);
    add(L"Pictures", L"PIC", L"shell:My Pictures", 0, 4,
        Color(255, 160, 120, 255), 5);
    add(L"Music", L"MUS", L"shell:My Music", 0, 5,
        Color(255, 255, 100, 130), 5);
    add(L"Desktop", L"DSK", L"shell:Desktop", 0, 6,
        Color(255, 100, 200, 130), 5);
}

// ============================================================================
//  TASKBAR APPS
// ============================================================================
struct TaskbarApp {
    std::wstring name;
    std::wstring exec;
    std::wstring iconLabel;
    Color accentColor;
    bool pinned;
    bool running;
    bool active;
    RECT bounds;
};
std::vector<TaskbarApp> g_taskApps;

void InitTaskbarApps() {
    g_taskApps.clear();
    auto add = [](const std::wstring& n, const std::wstring& e, const std::wstring& ico,
        Color c, bool run, bool active) {
            TaskbarApp a;
            a.name = n; a.exec = e; a.iconLabel = ico; a.accentColor = c;
            a.pinned = true; a.running = run; a.active = active;
            memset(&a.bounds, 0, sizeof(RECT));
            g_taskApps.push_back(a);
        };

    add(L"File Explorer", L"explorer.exe", L"E", Color(255, 255, 200, 50), true, false);
    add(L"Microsoft Edge", L"msedge.exe", L"e", Color(255, 0, 150, 255), true, false);
    add(L"Terminal", L"cmd.exe", L">_", Color(255, 40, 40, 40), false, false);
    add(L"Notepad", L"notepad.exe", L"N", Color(255, 100, 180, 255), true, false);
    add(L"Settings", L"ms-settings:", L"S", Color(255, 142, 142, 160), false, false);
    add(L"Store", L"ms-windows-store:", L"St", Color(255, 0, 120, 212), false, false);
}

// ============================================================================
//  START MENU APPS
// ============================================================================
struct StartApp {
    std::wstring name;
    std::wstring iconLabel;
    std::wstring exec;
    Color color;
};
std::vector<StartApp> g_startApps;

void InitStartApps() {
    g_startApps.clear();
    auto add = [](const std::wstring& n, const std::wstring& ico, const std::wstring& exec, Color c) {
        StartApp a;
        a.name = n; a.iconLabel = ico; a.exec = exec; a.color = c;
        g_startApps.push_back(a);
        };

    add(L"File Explorer", L"E", L"explorer.exe", Color(255, 255, 200, 50));
    add(L"Edge", L"e", L"msedge.exe", Color(255, 0, 150, 255));
    add(L"Terminal", L">_", L"cmd.exe", Color(255, 40, 40, 40));
    add(L"Notepad", L"N", L"notepad.exe", Color(255, 100, 180, 255));
    add(L"Settings", L"S", L"ms-settings:", Color(255, 142, 142, 160));
    add(L"Calculator", L"=", L"calc.exe", Color(255, 60, 60, 60));
    add(L"Paint", L"P", L"mspaint.exe", Color(255, 255, 100, 100));
    add(L"Photos", L"Ph", L"mspaint.exe", Color(255, 255, 180, 50));
    add(L"Mail", L"@", L"notepad.exe", Color(255, 0, 120, 212));
    add(L"Calendar", L"31", L"notepad.exe", Color(255, 0, 120, 212));
    add(L"Maps", L"M", L"notepad.exe", Color(255, 100, 180, 255));
    add(L"Weather", L"W", L"notepad.exe", Color(255, 255, 200, 50));
    add(L"Clock", L"C", L"notepad.exe", Color(255, 100, 100, 100));
    add(L"Camera", L"Ca", L"notepad.exe", Color(255, 80, 80, 80));
    add(L"Store", L"St", L"ms-windows-store:", Color(255, 0, 120, 212));
    add(L"Task Manager", L"TM", L"taskmgr.exe", Color(255, 60, 60, 60));
    add(L"Control Panel", L"CP", L"control.exe", Color(255, 0, 80, 160));
    add(L"Snip & Sketch", L"Sn", L"notepad.exe", Color(255, 178, 87, 255));
}

// ============================================================================
//  WIFI
// ============================================================================
struct WifiNetwork {
    std::wstring ssid;
    int signal;
    bool secured;
    bool connected;
};
std::vector<WifiNetwork> g_wifiNetworks;
bool g_wifiScanning = false;

DWORD WINAPI WifiScanThread(LPVOID) {
    g_wifiScanning = true;
    g_wifiNetworks.clear();

    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    CreatePipe(&hReadPipe, &hWritePipe, &sa, 0);
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi = {};
    wchar_t cmd[] = L"netsh wlan show networks mode=bssid";

    if (CreateProcessW(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(hWritePipe);
        std::string output;
        char buf[4096];
        DWORD bytesRead;
        while (ReadFile(hReadPipe, buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buf[bytesRead] = 0;
            output += buf;
        }
        CloseHandle(hReadPipe);
        WaitForSingleObject(pi.hProcess, 3000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        std::istringstream stream(output);
        std::string line;
        WifiNetwork current;
        current.signal = 0; current.secured = false; current.connected = false;
        bool hasNetwork = false;

        while (std::getline(stream, line)) {
            if (line.find("SSID") != std::string::npos && line.find("BSSID") == std::string::npos) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    if (hasNetwork && !current.ssid.empty()) g_wifiNetworks.push_back(current);
                    std::string ssid = line.substr(pos + 2);
                    while (!ssid.empty() && (ssid.back() == '\r' || ssid.back() == '\n' || ssid.back() == ' '))
                        ssid.pop_back();
                    current.ssid = std::wstring(ssid.begin(), ssid.end());
                    current.signal = 0; current.secured = false; current.connected = false;
                    hasNetwork = true;
                }
            }
            if (line.find("Signal") != std::string::npos || line.find("%") != std::string::npos) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    std::string val = line.substr(pos + 1);
                    int sig = 0;
                    for (char c : val) { if (c >= '0' && c <= '9') sig = sig * 10 + (c - '0'); }
                    if (sig > 0 && sig <= 100) current.signal = sig;
                }
            }
            if (line.find("Authentication") != std::string::npos) {
                if (line.find("Open") == std::string::npos) current.secured = true;
            }
        }
        if (hasNetwork && !current.ssid.empty()) g_wifiNetworks.push_back(current);
    }
    else {
        CloseHandle(hWritePipe);
        CloseHandle(hReadPipe);
    }

    if (g_wifiNetworks.empty()) {
        WifiNetwork w;
        w.ssid = L"HomeNetwork-5G"; w.signal = 95; w.secured = true; w.connected = true;
        g_wifiNetworks.push_back(w);
        w.ssid = L"HomeNetwork-2.4G"; w.signal = 78; w.secured = true; w.connected = false;
        g_wifiNetworks.push_back(w);
        w.ssid = L"Neighbor_WiFi"; w.signal = 52; w.secured = true; w.connected = false;
        g_wifiNetworks.push_back(w);
        w.ssid = L"CoffeeShop"; w.signal = 35; w.secured = false; w.connected = false;
        g_wifiNetworks.push_back(w);
        w.ssid = L"Guest_Network"; w.signal = 22; w.secured = false; w.connected = false;
        g_wifiNetworks.push_back(w);
    }
    else {
        if (!g_wifiNetworks.empty()) g_wifiNetworks[0].connected = true;
    }

    std::sort(g_wifiNetworks.begin(), g_wifiNetworks.end(),
        [](const WifiNetwork& a, const WifiNetwork& b) { return a.signal > b.signal; });

    g_wifiScanning = false;
    if (g_hWnd) InvalidateRect(g_hWnd, NULL, FALSE);
    return 0;
}

// ============================================================================
//  NOTIFICATIONS
// ============================================================================
struct Notification {
    std::wstring title, message;
    DWORD time;
    float alpha, offsetY;
    bool alive;
};
std::vector<Notification> g_notifs;

void PushNotification(const std::wstring& title, const std::wstring& msg) {
    Notification n;
    n.title = title; n.message = msg;
    n.time = GetTickCount();
    n.alpha = 0.0f; n.offsetY = -30.0f;
    n.alive = true;
    g_notifs.push_back(n);
}

// ============================================================================
//  CONTEXT MENU
// ============================================================================
struct ContextMenuItem {
    std::wstring label;
    std::wstring shortcut;
    int id; // 0=separator
    bool enabled;
};
std::vector<ContextMenuItem> g_contextItems;

void InitContextMenu(bool onIcon = false) {
    g_contextItems.clear();
    ContextMenuItem item;

    if (onIcon) {
        item.label = L"Open"; item.shortcut = L"Enter"; item.id = 10; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L""; item.shortcut = L""; item.id = 0; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L"Cut"; item.shortcut = L"Ctrl+X"; item.id = 11; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L"Copy"; item.shortcut = L"Ctrl+C"; item.id = 12; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L"Delete"; item.shortcut = L"Del"; item.id = 13; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L"Rename"; item.shortcut = L"F2"; item.id = 14; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L""; item.shortcut = L""; item.id = 0; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L"Properties"; item.shortcut = L""; item.id = 15; item.enabled = true;
        g_contextItems.push_back(item);
    }
    else {
        item.label = L"View"; item.shortcut = L""; item.id = 20; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L"Sort by"; item.shortcut = L""; item.id = 21; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L"Refresh"; item.shortcut = L"F5"; item.id = 2; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L""; item.shortcut = L""; item.id = 0; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L"New folder"; item.shortcut = L""; item.id = 1; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L"New shortcut"; item.shortcut = L""; item.id = 22; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L""; item.shortcut = L""; item.id = 0; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L"Open in Terminal"; item.shortcut = L""; item.id = 4; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L""; item.shortcut = L""; item.id = 0; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L"Display settings"; item.shortcut = L""; item.id = 3; item.enabled = true;
        g_contextItems.push_back(item);
        item.label = L"Personalize"; item.shortcut = L""; item.id = 23; item.enabled = true;
        g_contextItems.push_back(item);
    }
}

// ============================================================================
//  FOLDER CREATION
// ============================================================================
void CreateNewFolderOnDesktop() {
    wchar_t desktopPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath))) {
        std::wstring baseName = std::wstring(desktopPath) + L"\\New Folder";
        std::wstring folderName = baseName;
        int idx = 1;
        while (GetFileAttributesW(folderName.c_str()) != INVALID_FILE_ATTRIBUTES)
            folderName = baseName + L" (" + std::to_wstring(idx++) + L")";

        if (CreateDirectoryW(folderName.c_str(), NULL)) {
            DesktopIcon d;
            size_t pos = folderName.find_last_of(L'\\');
            d.name = (pos != std::wstring::npos) ? folderName.substr(pos + 1) : folderName;
            d.iconSymbol = L"F";
            d.action = L"explorer.exe \"" + folderName + L"\"";
            d.iconColor = Color(255, 255, 210, 76);
            d.iconType = 2;
            d.selected = false;
            int col, row;
            FindFreeGrid(col, row);
            d.gridCol = col; d.gridRow = row;
            CalcIconPixelPos(d);
            g_desktopIcons.push_back(d);
        }
    }
}

// ============================================================================
//  GDI+ UTILITIES
// ============================================================================
int ScreenW() { return GetSystemMetrics(SM_CXSCREEN); }
int ScreenH() { return GetSystemMetrics(SM_CYSCREEN); }

void RoundedRectPath(GraphicsPath& p, int x, int y, int w, int h, int r) {
    int d = r * 2;
    if (d > w) d = w;
    if (d > h) d = h;
    if (r <= 0) { p.AddRectangle(Rect(x, y, w, h)); return; }
    p.AddArc(x, y, d, d, 180, 90);
    p.AddArc(x + w - d, y, d, d, 270, 90);
    p.AddArc(x + w - d, y + h - d, d, d, 0, 90);
    p.AddArc(x, y + h - d, d, d, 90, 90);
    p.CloseFigure();
}

void FillRoundRect(Graphics& g, int x, int y, int w, int h, int r, const Brush& br) {
    GraphicsPath p;
    RoundedRectPath(p, x, y, w, h, r);
    g.FillPath(&br, &p);
}

void DrawRoundRect(Graphics& g, int x, int y, int w, int h, int r, Color c, float width = 1.0f) {
    GraphicsPath p;
    RoundedRectPath(p, x, y, w, h, r);
    Pen pen(c, width);
    g.DrawPath(&pen, &p);
}

void FillRoundRectSolid(Graphics& g, int x, int y, int w, int h, int r, Color c) {
    SolidBrush br(c);
    FillRoundRect(g, x, y, w, h, r, br);
}

// Drop shadow
void DrawShadow(Graphics& g, int x, int y, int w, int h, int r, int layers = 6) {
    for (int i = layers; i >= 1; i--) {
        int sp = i * 3;
        int alpha = 12 * (layers - i + 1) / layers;
        FillRoundRectSolid(g, x - sp + 2, y - sp + 4, w + sp * 2, h + sp * 2, r + sp / 2,
            Color((BYTE)alpha, 0, 0, 0));
    }
}

// ============================================================================
//  DRAW DESKTOP ICON - Windows 11 style
// ============================================================================
void DrawDesktopIconItem(Graphics& g, const DesktopIcon& d, bool hovered, bool dragging_this) {
    int x = d.pixelX;
    int y = d.pixelY;

    if (dragging_this) {
        x = g_dragCurrentX - g_dragOffsetX;
        y = g_dragCurrentY - g_dragOffsetY;
    }

    // Background on hover / selected
    if (d.selected && hovered) {
        FillRoundRectSolid(g, x, y, DESKTOP_ICON_W, DESKTOP_ICON_H, 4,
            Color(90, 0, 103, 192));
        DrawRoundRect(g, x, y, DESKTOP_ICON_W, DESKTOP_ICON_H, 4,
            Color(120, 0, 103, 192), 1.0f);
    }
    else if (d.selected) {
        FillRoundRectSolid(g, x, y, DESKTOP_ICON_W, DESKTOP_ICON_H, 4,
            W11::IconSelectedBg);
        DrawRoundRect(g, x, y, DESKTOP_ICON_W, DESKTOP_ICON_H, 4,
            Color(80, 0, 103, 192), 1.0f);
    }
    else if (hovered) {
        FillRoundRectSolid(g, x, y, DESKTOP_ICON_W, DESKTOP_ICON_H, 4,
            W11::IconHoverBg);
    }

    // Icon visual - Windows 11 style folder/file icon
    int icoSize = 42;
    int icoX = x + (DESKTOP_ICON_W - icoSize) / 2;
    int icoY = y + 6;

    if (d.iconType == 0) {
        // This PC - monitor icon
        // Monitor body
        FillRoundRectSolid(g, icoX + 4, icoY + 2, icoSize - 8, icoSize - 16, 3,
            Color(255, 60, 130, 200));
        // Screen
        FillRoundRectSolid(g, icoX + 7, icoY + 5, icoSize - 14, icoSize - 22, 2,
            Color(255, 100, 180, 240));
        // Stand
        SolidBrush standBr(Color(255, 120, 120, 130));
        g.FillRectangle(&standBr, icoX + icoSize / 2 - 4, icoY + icoSize - 14, 8, 6);
        g.FillRectangle(&standBr, icoX + icoSize / 2 - 8, icoY + icoSize - 9, 16, 3);
    }
    else if (d.iconType == 1) {
        // Recycle Bin
        FillRoundRectSolid(g, icoX + 10, icoY + 8, icoSize - 20, icoSize - 14, 2,
            Color(255, 140, 140, 150));
        // Lid
        Pen lidPen(Color(255, 160, 160, 170), 2.5f);
        g.DrawLine(&lidPen, icoX + 8, icoY + 8, icoX + icoSize - 8, icoY + 8);
        // Handle
        g.DrawLine(&lidPen, icoX + icoSize / 2 - 3, icoY + 4, icoX + icoSize / 2 + 3, icoY + 4);
        g.DrawLine(&lidPen, icoX + icoSize / 2 - 3, icoY + 4, icoX + icoSize / 2 - 3, icoY + 8);
        g.DrawLine(&lidPen, icoX + icoSize / 2 + 3, icoY + 4, icoX + icoSize / 2 + 3, icoY + 8);
        // Lines on bin
        Pen linePen(Color(100, 80, 80, 90), 1.0f);
        for (int li = 0; li < 3; li++) {
            int lx = icoX + 15 + li * 6;
            g.DrawLine(&linePen, lx, icoY + 14, lx, icoY + icoSize - 10);
        }
    }
    else if (d.iconType == 2 || d.iconType == 5) {
        // Folder - Windows 11 yellow folder
        // Tab
        Color folderColor = d.iconColor;
        FillRoundRectSolid(g, icoX + 4, icoY + 6, 16, 6, 2, folderColor);
        // Body
        FillRoundRectSolid(g, icoX + 4, icoY + 10, icoSize - 8, icoSize - 16, 3, folderColor);
        // Front face (lighter)
        Color lighter(255,
            (BYTE)(std::min)(255, (int)folderColor.GetR() + 30),
            (BYTE)(std::min)(255, (int)folderColor.GetG() + 30),
            (BYTE)(std::min)(255, (int)folderColor.GetB() + 30));
        FillRoundRectSolid(g, icoX + 4, icoY + 16, icoSize - 8, icoSize - 22, 3, lighter);
    }
    else if (d.iconType == 4) {
        // Shortcut
        FillRoundRectSolid(g, icoX + 6, icoY + 2, icoSize - 12, icoSize - 10, 3,
            Color(255, 70, 70, 80));
        // Arrow overlay
        Pen arrowPen(Color(255, 200, 200, 220), 2.0f);
        g.DrawLine(&arrowPen, icoX + 12, icoY + icoSize / 2, icoX + icoSize - 12, icoY + icoSize / 2);
        g.DrawLine(&arrowPen, icoX + icoSize - 16, icoY + icoSize / 2 - 4,
            icoX + icoSize - 12, icoY + icoSize / 2);
        g.DrawLine(&arrowPen, icoX + icoSize - 16, icoY + icoSize / 2 + 4,
            icoX + icoSize - 12, icoY + icoSize / 2);
    }
    else {
        // Generic file
        // Page
        FillRoundRectSolid(g, icoX + 8, icoY + 2, icoSize - 16, icoSize - 10, 2,
            Color(255, 240, 240, 245));
        // Corner fold
        SolidBrush foldBr(Color(255, 200, 200, 210));
        Point foldPts[3] = {
            Point(icoX + icoSize - 8 - 10, icoY + 2),
            Point(icoX + icoSize - 8, icoY + 2),
            Point(icoX + icoSize - 8, icoY + 12)
        };
        g.FillPolygon(&foldBr, foldPts, 3);
        // Lines
        Pen lp(Color(80, 150, 150, 160), 1.0f);
        for (int li = 0; li < 3; li++) {
            g.DrawLine(&lp, icoX + 14, icoY + 16 + li * 6, icoX + icoSize - 14, icoY + 16 + li * 6);
        }
    }

    // Name label
    FontFamily ff(L"Segoe UI");
    Font nameFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
    StringFormat cfmt;
    cfmt.SetAlignment(StringAlignmentCenter);
    cfmt.SetLineAlignment(StringAlignmentNear);
    cfmt.SetTrimming(StringTrimmingEllipsisCharacter);
    cfmt.SetFormatFlags(StringFormatFlagsLineLimit);

    // Text shadow
    SolidBrush shadowBr(Color(200, 0, 0, 0));
    RectF shadowRc((REAL)(x - 2), (REAL)(y + DESKTOP_ICON_H - 28 + 1),
        (REAL)(DESKTOP_ICON_W + 4), 28.0f);
    g.DrawString(d.name.c_str(), -1, &nameFont, shadowRc, &cfmt, &shadowBr);

    SolidBrush textBr(W11::TextPrimary);
    RectF textRc((REAL)(x - 2), (REAL)(y + DESKTOP_ICON_H - 28),
        (REAL)(DESKTOP_ICON_W + 4), 28.0f);
    g.DrawString(d.name.c_str(), -1, &nameFont, textRc, &cfmt, &textBr);
}

// ============================================================================
//  DRAW ALL DESKTOP ICONS
// ============================================================================
void DrawDesktopIcons(Graphics& g) {
    for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
        bool hov = (g_hoveredDesktopIcon == i);
        bool drag = (g_dragging && g_dragIconIdx == i && g_dragStarted);
        if (!drag) {
            DrawDesktopIconItem(g, g_desktopIcons[i], hov, false);
        }
    }
    // Draw dragged icon last (on top)
    if (g_dragging && g_dragIconIdx >= 0 && g_dragStarted) {
        DrawDesktopIconItem(g, g_desktopIcons[g_dragIconIdx], true, true);
    }
}

// ============================================================================
//  DRAW SELECTION RECTANGLE
// ============================================================================
void DrawSelectionRect(Graphics& g) {
    if (!g_selecting) return;
    int x1 = (std::min)(g_selStartX, g_selEndX);
    int y1 = (std::min)(g_selStartY, g_selEndY);
    int x2 = (std::max)(g_selStartX, g_selEndX);
    int y2 = (std::max)(g_selStartY, g_selEndY);
    int w = x2 - x1;
    int h = y2 - y1;
    if (w < 2 && h < 2) return;

    SolidBrush fillBr(W11::SelectionRect);
    g.FillRectangle(&fillBr, x1, y1, w, h);
    Pen borderPen(W11::SelectionBorder, 1.0f);
    g.DrawRectangle(&borderPen, x1, y1, w, h);
}

// ============================================================================
//  TASKBAR - Windows 11 Style
// ============================================================================
void DrawTaskbar(Graphics& g, int sw, int sh) {
    int barY = sh - TASKBAR_HEIGHT;

    // Background
    SolidBrush barBg(W11::TaskbarBg);
    g.FillRectangle(&barBg, 0, barY, sw, TASKBAR_HEIGHT);

    // Top border line
    Pen topLine(Color(50, 255, 255, 255), 1.0f);
    g.DrawLine(&topLine, 0, barY, sw, barY);

    // Center section - icons
    int iconCount = (int)g_taskApps.size();
    int totalW = (iconCount + 3) * (TASKBAR_ICON_SIZE + 4); // +3 for start, search, taskview
    int startX = (sw - totalW) / 2;

    FontFamily ff(L"Segoe UI");

    // Start button (Windows logo area)
    {
        int bx = startX;
        int by = barY + (TASKBAR_HEIGHT - TASKBAR_ICON_SIZE) / 2;
        bool startHov = (g_hoveredTaskbarIcon == -10);

        if (g_startMenuOpen || startHov) {
            FillRoundRectSolid(g, bx, by, TASKBAR_ICON_SIZE, TASKBAR_ICON_SIZE, 4,
                g_startMenuOpen ? W11::TaskbarActive : W11::TaskbarHover);
        }

        // Windows 11 logo - 4 squares
        int logoSize = 14;
        int lx = bx + (TASKBAR_ICON_SIZE - logoSize) / 2;
        int ly = by + (TASKBAR_ICON_SIZE - logoSize) / 2;
        int gap = 2;
        int sq = (logoSize - gap) / 2;

        SolidBrush logoBr(W11::AccentLight);
        g.FillRectangle(&logoBr, lx, ly, sq, sq);
        g.FillRectangle(&logoBr, lx + sq + gap, ly, sq, sq);
        g.FillRectangle(&logoBr, lx, ly + sq + gap, sq, sq);
        g.FillRectangle(&logoBr, lx + sq + gap, ly + sq + gap, sq, sq);

        // Store bounds for click detection
        RECT startBounds;
        SetRect(&startBounds, bx, by, bx + TASKBAR_ICON_SIZE, by + TASKBAR_ICON_SIZE);
    }

    // Search button
    {
        int bx = startX + (TASKBAR_ICON_SIZE + 4);
        int by = barY + (TASKBAR_HEIGHT - TASKBAR_ICON_SIZE) / 2;
        bool searchHov = (g_hoveredTaskbarIcon == -11);
        if (searchHov)
            FillRoundRectSolid(g, bx, by, TASKBAR_ICON_SIZE, TASKBAR_ICON_SIZE, 4, W11::TaskbarHover);

        // Magnifying glass
        Pen searchPen(W11::TextSecondary, 2.0f);
        int cx = bx + TASKBAR_ICON_SIZE / 2 - 2;
        int cy = by + TASKBAR_ICON_SIZE / 2 - 2;
        g.DrawEllipse(&searchPen, cx - 6, cy - 6, 12, 12);
        g.DrawLine(&searchPen, cx + 4, cy + 4, cx + 8, cy + 8);
    }

    // Task View button
    {
        int bx = startX + 2 * (TASKBAR_ICON_SIZE + 4);
        int by = barY + (TASKBAR_HEIGHT - TASKBAR_ICON_SIZE) / 2;
        bool tvHov = (g_hoveredTaskbarIcon == -12);
        if (tvHov)
            FillRoundRectSolid(g, bx, by, TASKBAR_ICON_SIZE, TASKBAR_ICON_SIZE, 4, W11::TaskbarHover);

        // Two overlapping rectangles
        Pen tvPen(W11::TextSecondary, 1.5f);
        g.DrawRectangle(&tvPen, bx + 10, by + 12, 10, 8);
        g.DrawRectangle(&tvPen, bx + 18, by + 18, 10, 8);
    }

    // App icons
    int appsStartX = startX + 3 * (TASKBAR_ICON_SIZE + 4) + 8;
    int iconY = barY + (TASKBAR_HEIGHT - TASKBAR_ICON_SIZE) / 2;

    for (int i = 0; i < iconCount; i++) {
        int ix = appsStartX + i * (TASKBAR_ICON_SIZE + 4);
        bool hov = (g_hoveredTaskbarIcon == i);

        if (hov || g_taskApps[i].active) {
            FillRoundRectSolid(g, ix, iconY, TASKBAR_ICON_SIZE, TASKBAR_ICON_SIZE, 4,
                g_taskApps[i].active ? W11::TaskbarActive : W11::TaskbarHover);
        }

        // Icon - simple colored rounded rect with letter
        int icoInner = 24;
        int icoX = ix + (TASKBAR_ICON_SIZE - icoInner) / 2;
        int icoY2 = iconY + (TASKBAR_ICON_SIZE - icoInner) / 2;

        FillRoundRectSolid(g, icoX, icoY2, icoInner, icoInner, 4, g_taskApps[i].accentColor);

        Font icoFont(&ff, 11.0f, FontStyleBold, UnitPixel);
        SolidBrush icoTextBr(Color(255, 255, 255, 255));
        StringFormat cfmt;
        cfmt.SetAlignment(StringAlignmentCenter);
        cfmt.SetLineAlignment(StringAlignmentCenter);
        RectF icoRc((REAL)icoX, (REAL)icoY2, (REAL)icoInner, (REAL)icoInner);
        g.DrawString(g_taskApps[i].iconLabel.c_str(), -1, &icoFont, icoRc, &cfmt, &icoTextBr);

        // Running indicator
        if (g_taskApps[i].running) {
            int dotW = g_taskApps[i].active ? 16 : 6;
            int dotX = ix + (TASKBAR_ICON_SIZE - dotW) / 2;
            int dotY = iconY + TASKBAR_ICON_SIZE - 2;
            FillRoundRectSolid(g, dotX, dotY, dotW, 3, 1, W11::TaskbarIndicator);
        }

        SetRect(&g_taskApps[i].bounds, ix, iconY, ix + TASKBAR_ICON_SIZE, iconY + TASKBAR_ICON_SIZE);
    }

    // System tray (right side)
    int trayX = sw - 200;
    int trayY = barY;

    // Hidden icons arrow
    Font trayFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
    SolidBrush trayBr(W11::TextSecondary);

    // WiFi icon
    {
        int wx = trayX + 20;
        int wy = trayY + (TASKBAR_HEIGHT - 16) / 2;
        Pen wPen(W11::TextSecondary, 1.5f);
        // Simple wifi arcs
        for (int arc = 0; arc < 3; arc++) {
            int r = 4 + arc * 4;
            g.DrawArc(&wPen, wx + 8 - r, wy + 12 - r, r * 2, r * 2, 225, 90);
        }
        SolidBrush dotBr(W11::TextSecondary);
        g.FillEllipse(&dotBr, wx + 6, wy + 10, 4, 4);
    }

    // Volume icon
    {
        int vx = trayX + 52;
        int vy = trayY + (TASKBAR_HEIGHT - 16) / 2;
        Pen vPen(W11::TextSecondary, 1.5f);
        // Speaker
        g.DrawRectangle(&vPen, vx + 2, vy + 4, 4, 8);
        Point cone[4] = { Point(vx + 6, vy + 4), Point(vx + 12, vy + 1), Point(vx + 12, vy + 15), Point(vx + 6, vy + 12) };
        g.DrawPolygon(&vPen, cone, 4);
    }

    // Battery
    {
        int bx = trayX + 82;
        int by = trayY + (TASKBAR_HEIGHT - 12) / 2;
        Pen bPen(W11::TextSecondary, 1.5f);
        g.DrawRectangle(&bPen, bx, by, 20, 12);
        SolidBrush bFill(W11::Success);
        g.FillRectangle(&bFill, bx + 2, by + 2, 14, 8);
        SolidBrush bTip(W11::TextSecondary);
        g.FillRectangle(&bTip, bx + 20, by + 3, 3, 6);
    }

    // Clock
    time_t now = time(NULL);
    struct tm ti;
    localtime_s(&ti, &now);
    wchar_t timeStr[32], dateStr[32];
    wcsftime(timeStr, 32, L"%H:%M", &ti);
    wcsftime(dateStr, 32, L"%d.%m.%Y", &ti);

    Font clockFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    Font dateFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
    SolidBrush clockBr(W11::TextPrimary);
    SolidBrush dateBr(W11::TextSecondary);

    StringFormat rfmt;
    rfmt.SetAlignment(StringAlignmentFar);
    rfmt.SetLineAlignment(StringAlignmentCenter);

    RectF clockRc((REAL)(sw - 90), (REAL)barY, 80.0f, (REAL)(TASKBAR_HEIGHT / 2));
    g.DrawString(timeStr, -1, &clockFont, clockRc, &rfmt, &clockBr);

    RectF dateRc((REAL)(sw - 90), (REAL)(barY + TASKBAR_HEIGHT / 2 - 2), 80.0f, (REAL)(TASKBAR_HEIGHT / 2));
    g.DrawString(dateStr, -1, &dateFont, dateRc, &rfmt, &dateBr);

    // Notification dot
    SolidBrush notifDot(W11::AccentLight);
    g.FillEllipse(&notifDot, sw - 16, barY + TASKBAR_HEIGHT / 2 - 3, 6, 6);
}

// ============================================================================
//  START MENU - Windows 11 Style
// ============================================================================
void DrawStartMenu(Graphics& g, int sw, int sh) {
    if (g_startMenuAnim <= 0.01f) return;
    float a = g_startMenuAnim;

    // Dim overlay
    SolidBrush overlay(Color((BYTE)(100 * a), 0, 0, 0));
    g.FillRectangle(&overlay, 0, 0, sw, sh);

    int menuW = START_MENU_W;
    int menuH = START_MENU_H;
    int menuX = (sw - menuW) / 2;
    int targetY = sh - TASKBAR_HEIGHT - menuH - 12;
    int menuY = (int)(targetY + 20 * (1.0f - a));

    // Shadow
    DrawShadow(g, menuX, menuY, menuW, menuH, START_MENU_RADIUS, 8);

    // Background
    FillRoundRectSolid(g, menuX, menuY, menuW, menuH, START_MENU_RADIUS,
        Color((BYTE)(245 * a), 38, 38, 38));

    // Border
    DrawRoundRect(g, menuX, menuY, menuW, menuH, START_MENU_RADIUS,
        Color((BYTE)(40 * a), 255, 255, 255), 1.0f);

    FontFamily ff(L"Segoe UI");

    // Search bar
    int searchX = menuX + 24;
    int searchY = menuY + 24;
    int searchW = menuW - 48;
    int searchH = 36;

    FillRoundRectSolid(g, searchX, searchY, searchW, searchH, 18,
        Color((BYTE)(255 * a), 55, 55, 55));
    DrawRoundRect(g, searchX, searchY, searchW, searchH, 18,
        Color((BYTE)(30 * a), 255, 255, 255), 1.0f);

    // Search icon
    Pen searchPen(Color((BYTE)(180 * a), 180, 180, 180), 1.5f);
    g.DrawEllipse(&searchPen, searchX + 14, searchY + 9, 12, 12);
    g.DrawLine(&searchPen, searchX + 24, searchY + 21, searchX + 28, searchY + 25);

    Font searchFont(&ff, 13.0f, FontStyleRegular, UnitPixel);
    SolidBrush searchBr(Color((BYTE)(120 * a), 180, 180, 180));
    StringFormat lfmt;
    lfmt.SetAlignment(StringAlignmentNear);
    lfmt.SetLineAlignment(StringAlignmentCenter);
    RectF searchRc((REAL)(searchX + 36), (REAL)searchY, (REAL)(searchW - 48), (REAL)searchH);
    g.DrawString(L"Type here to search", -1, &searchFont, searchRc, &lfmt, &searchBr);

    // Pinned section
    Font sectionFont(&ff, 14.0f, FontStyleBold, UnitPixel);
    SolidBrush sectionBr(Color((BYTE)(255 * a), 255, 255, 255));
    RectF pinnedTitleRc((REAL)(menuX + 30), (REAL)(menuY + 76), 100.0f, 20.0f);
    g.DrawString(L"Pinned", -1, &sectionFont, pinnedTitleRc, &lfmt, &sectionBr);

    // "All apps >" button
    Font smallFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    SolidBrush allBr(Color((BYTE)(200 * a), 180, 180, 180));
    StringFormat rfmt;
    rfmt.SetAlignment(StringAlignmentFar);
    rfmt.SetLineAlignment(StringAlignmentCenter);
    RectF allRc((REAL)(menuX + menuW - 130), (REAL)(menuY + 76), 100.0f, 20.0f);
    g.DrawString(L"All apps >", -1, &smallFont, allRc, &rfmt, &allBr);

    // App grid
    int gridX = menuX + 28;
    int gridY = menuY + 108;
    int cols = 6;
    int cellW = (menuW - 56) / cols;
    int cellH = 72;

    for (int i = 0; i < (int)g_startApps.size() && i < 18; i++) {
        int row = i / cols;
        int col = i % cols;
        int ix = gridX + col * cellW;
        int iy = gridY + row * cellH;

        bool hov = (g_hoveredStartItem == i);

        if (hov) {
            FillRoundRectSolid(g, ix + 2, iy + 2, cellW - 4, cellH - 4, 4,
                Color((BYTE)(255 * a), 60, 60, 60));
        }

        // App icon (rounded rect with letter)
        int aicoSize = 32;
        int aicoX = ix + (cellW - aicoSize) / 2;
        int aicoY = iy + 8;

        FillRoundRectSolid(g, aicoX, aicoY, aicoSize, aicoSize, 6,
            Color((BYTE)(240 * a), g_startApps[i].color.GetR(),
                g_startApps[i].color.GetG(), g_startApps[i].color.GetB()));

        Font aicoFont(&ff, 12.0f, FontStyleBold, UnitPixel);
        SolidBrush aicoTextBr(Color((BYTE)(255 * a), 255, 255, 255));
        StringFormat cfmt;
        cfmt.SetAlignment(StringAlignmentCenter);
        cfmt.SetLineAlignment(StringAlignmentCenter);
        RectF aicoRc((REAL)aicoX, (REAL)aicoY, (REAL)aicoSize, (REAL)aicoSize);
        g.DrawString(g_startApps[i].iconLabel.c_str(), -1, &aicoFont, aicoRc, &cfmt, &aicoTextBr);

        // Name
        Font nameFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
        SolidBrush nameBr(Color((BYTE)(220 * a), 220, 220, 220));
        RectF nameRc((REAL)(ix), (REAL)(iy + 44), (REAL)cellW, 18.0f);
        cfmt.SetTrimming(StringTrimmingEllipsisCharacter);
        g.DrawString(g_startApps[i].name.c_str(), -1, &nameFont, nameRc, &cfmt, &nameBr);
    }

    // Separator line
    Pen sepLine(Color((BYTE)(30 * a), 255, 255, 255), 1.0f);
    int sepY = menuY + 108 + 3 * cellH + 12;
    g.DrawLine(&sepLine, menuX + 28, sepY, menuX + menuW - 28, sepY);

    // Recommended section
    RectF recTitleRc((REAL)(menuX + 30), (REAL)(sepY + 12), 140.0f, 20.0f);
    g.DrawString(L"Recommended", -1, &sectionFont, recTitleRc, &lfmt, &sectionBr);

    // Recommended items (fake)
    const wchar_t* recNames[] = { L"Recently opened file.txt", L"Project_v2.docx", L"Screenshot_2026.png" };
    const wchar_t* recTimes[] = { L"Just now", L"Yesterday", L"2 days ago" };

    for (int i = 0; i < 3; i++) {
        int ry = sepY + 44 + i * 40;
        int rx = menuX + 30;

        // File icon
        FillRoundRectSolid(g, rx, ry + 4, 28, 28, 4, Color((BYTE)(180 * a), 60, 60, 70));
        Font fIcon(&ff, 10.0f, FontStyleRegular, UnitPixel);
        SolidBrush fIcoBr(Color((BYTE)(200 * a), 180, 180, 200));
        RectF fIcoRc((REAL)rx, (REAL)(ry + 4), 28.0f, 28.0f);
        StringFormat cfmt;
        cfmt.SetAlignment(StringAlignmentCenter);
        cfmt.SetLineAlignment(StringAlignmentCenter);
        g.DrawString(L"F", -1, &fIcon, fIcoRc, &cfmt, &fIcoBr);

        Font recFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
        SolidBrush recBr(Color((BYTE)(220 * a), 220, 220, 230));
        RectF recNameRc((REAL)(rx + 36), (REAL)(ry + 2), 200.0f, 18.0f);
        g.DrawString(recNames[i], -1, &recFont, recNameRc, &lfmt, &recBr);

        Font timeFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
        SolidBrush timeBr(Color((BYTE)(120 * a), 150, 150, 160));
        RectF recTimeRc((REAL)(rx + 36), (REAL)(ry + 20), 200.0f, 16.0f);
        g.DrawString(recTimes[i], -1, &timeFont, recTimeRc, &lfmt, &timeBr);
    }

    // Bottom bar - User + Power
    int bottomY = menuY + menuH - 52;
    Pen bottomSep(Color((BYTE)(25 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&bottomSep, menuX + 24, bottomY, menuX + menuW - 24, bottomY);

    // User avatar
    FillRoundRectSolid(g, menuX + 30, bottomY + 12, 28, 28, 14,
        Color((BYTE)(200 * a), 80, 80, 90));
    Font userIcon(&ff, 14.0f, FontStyleBold, UnitPixel);
    SolidBrush userBr(Color((BYTE)(240 * a), 200, 200, 210));
    StringFormat cfmt;
    cfmt.SetAlignment(StringAlignmentCenter);
    cfmt.SetLineAlignment(StringAlignmentCenter);
    RectF userRc((REAL)(menuX + 30), (REAL)(bottomY + 12), 28.0f, 28.0f);
    g.DrawString(L"U", -1, &userIcon, userRc, &cfmt, &userBr);

    // Username
    Font usernameFont(&ff, 13.0f, FontStyleRegular, UnitPixel);
    SolidBrush usernameBr(Color((BYTE)(220 * a), 230, 230, 240));
    RectF usernameRc((REAL)(menuX + 66), (REAL)(bottomY + 8), 200.0f, 36.0f);
    lfmt.SetLineAlignment(StringAlignmentCenter);
    g.DrawString(L"User", -1, &usernameFont, usernameRc, &lfmt, &usernameBr);

    // Power button
    int pwrX = menuX + menuW - 52;
    int pwrY = bottomY + 12;
    FillRoundRectSolid(g, pwrX, pwrY, 28, 28, 4,
        Color((BYTE)(60 * a), 255, 255, 255));

    Pen pwrPen(Color((BYTE)(200 * a), 200, 200, 210), 2.0f);
    int pcx = pwrX + 14, pcy = pwrY + 14;
    g.DrawArc(&pwrPen, pcx - 6, pcy - 6, 12, 12, -60, 300);
    g.DrawLine(&pwrPen, pcx, pcy - 8, pcx, pcy - 3);
}

// ============================================================================
//  CONTEXT MENU - Windows 11 Style
// ============================================================================
void DrawContextMenu(Graphics& g) {
    if (!g_contextMenuOpen || g_contextMenuAnim <= 0.01f) return;
    float a = g_contextMenuAnim;

    int menuW = CTX_MENU_W;
    int menuH = 8;
    for (const auto& item : g_contextItems)
        menuH += (item.id == 0) ? CTX_SEP_H : CTX_ITEM_H;
    menuH += 8;

    int mx = g_contextMenuX;
    int my = g_contextMenuY;
    if (mx + menuW > ScreenW()) mx = ScreenW() - menuW - 8;
    if (my + menuH > ScreenH()) my = ScreenH() - menuH - 8;

    // Shadow
    DrawShadow(g, mx, my, menuW, menuH, CTX_RADIUS, 6);

    // Background
    FillRoundRectSolid(g, mx, my, menuW, menuH, CTX_RADIUS,
        Color((BYTE)(250 * a), 44, 44, 44));

    // Border
    DrawRoundRect(g, mx, my, menuW, menuH, CTX_RADIUS,
        Color((BYTE)(40 * a), 255, 255, 255), 1.0f);

    FontFamily ff(L"Segoe UI");
    Font font(&ff, 12.0f, FontStyleRegular, UnitPixel);
    Font shortcutFont(&ff, 11.0f, FontStyleRegular, UnitPixel);

    StringFormat lfmt;
    lfmt.SetAlignment(StringAlignmentNear);
    lfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat rfmt;
    rfmt.SetAlignment(StringAlignmentFar);
    rfmt.SetLineAlignment(StringAlignmentCenter);

    int cy = my + 8;
    for (int i = 0; i < (int)g_contextItems.size(); i++) {
        const auto& item = g_contextItems[i];

        if (item.id == 0) {
            Pen sepPen(Color((BYTE)(30 * a), 255, 255, 255), 1.0f);
            g.DrawLine(&sepPen, mx + 12, cy + CTX_SEP_H / 2, mx + menuW - 12, cy + CTX_SEP_H / 2);
            cy += CTX_SEP_H;
            continue;
        }

        bool hov = (g_hoveredContextItem == i);

        if (hov) {
            FillRoundRectSolid(g, mx + 4, cy + 1, menuW - 8, CTX_ITEM_H - 2, 4,
                Color((BYTE)(255 * a), 60, 60, 60));
        }

        SolidBrush textBr(Color((BYTE)(230 * a), 230, 230, 240));
        RectF textRc((REAL)(mx + 16), (REAL)cy, (REAL)(menuW - 32), (REAL)CTX_ITEM_H);
        g.DrawString(item.label.c_str(), -1, &font, textRc, &lfmt, &textBr);

        if (!item.shortcut.empty()) {
            SolidBrush scBr(Color((BYTE)(100 * a), 140, 140, 150));
            g.DrawString(item.shortcut.c_str(), -1, &shortcutFont, textRc, &rfmt, &scBr);
        }

        cy += CTX_ITEM_H;
    }
}

// ============================================================================
//  WIFI PANEL - Windows 11 Quick Settings Style
// ============================================================================
void DrawWifiPanel(Graphics& g, int sw, int sh) {
    if (g_wifiPanelAnim <= 0.01f) return;
    float a = g_wifiPanelAnim;

    int panelW = WIFI_PANEL_W;
    int panelH = WIFI_PANEL_H;
    int panelX = sw - panelW - 12;
    int panelY = (int)(sh - TASKBAR_HEIGHT - panelH - 12 + 15 * (1.0f - a));

    DrawShadow(g, panelX, panelY, panelW, panelH, 8, 6);
    FillRoundRectSolid(g, panelX, panelY, panelW, panelH, 8,
        Color((BYTE)(248 * a), 38, 38, 38));
    DrawRoundRect(g, panelX, panelY, panelW, panelH, 8,
        Color((BYTE)(40 * a), 255, 255, 255), 1.0f);

    FontFamily ff(L"Segoe UI");

    // Quick toggles row
    int toggleY = panelY + 16;
    int toggleH = 70;
    int toggleW = (panelW - 48) / 3;

    struct Toggle {
        std::wstring label;
        bool active;
        Color color;
    };
    Toggle toggles[] = {
        { L"WiFi", true, W11::AccentLight },
        { L"Bluetooth", true, Color(255, 0, 120, 212) },
        { L"Airplane", false, Color(255, 100, 100, 100) }
    };

    for (int i = 0; i < 3; i++) {
        int tx = panelX + 16 + i * (toggleW + 8);
        Color bgCol = toggles[i].active ?
            Color((BYTE)(255 * a), toggles[i].color.GetR(), toggles[i].color.GetG(), toggles[i].color.GetB()) :
            Color((BYTE)(200 * a), 60, 60, 65);

        FillRoundRectSolid(g, tx, toggleY, toggleW, toggleH, 6, bgCol);

        Font tFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
        Color tColor = toggles[i].active ? Color((BYTE)(255 * a), 255, 255, 255) :
            Color((BYTE)(200 * a), 180, 180, 190);
        SolidBrush tBr(tColor);
        StringFormat cfmt;
        cfmt.SetAlignment(StringAlignmentCenter);
        cfmt.SetLineAlignment(StringAlignmentFar);
        RectF tRc((REAL)tx, (REAL)toggleY, (REAL)toggleW, (REAL)(toggleH - 8));
        g.DrawString(toggles[i].label.c_str(), -1, &tFont, tRc, &cfmt, &tBr);
    }

    // Separator
    Pen sep(Color((BYTE)(30 * a), 255, 255, 255), 1.0f);
    int sepY = toggleY + toggleH + 12;
    g.DrawLine(&sep, panelX + 16, sepY, panelX + panelW - 16, sepY);

    // WiFi networks header
    Font titleFont(&ff, 13.0f, FontStyleBold, UnitPixel);
    SolidBrush titleBr(Color((BYTE)(255 * a), 240, 240, 240));
    StringFormat lfmt;
    lfmt.SetAlignment(StringAlignmentNear);
    lfmt.SetLineAlignment(StringAlignmentCenter);
    RectF titleRc((REAL)(panelX + 18), (REAL)(sepY + 8), 200.0f, 24.0f);
    g.DrawString(L"Wi-Fi networks", -1, &titleFont, titleRc, &lfmt, &titleBr);

    // Network list
    Font itemFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    Font smallFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
    int listY = sepY + 40;
    int itemH = 48;

    for (int i = 0; i < (int)g_wifiNetworks.size() && i < 6; i++) {
        const WifiNetwork& net = g_wifiNetworks[i];
        int iy = listY + i * itemH;
        bool hov = (g_hoveredWifiItem == i);

        if (hov) {
            FillRoundRectSolid(g, panelX + 8, iy, panelW - 16, itemH - 4, 4,
                Color((BYTE)(200 * a), 55, 55, 60));
        }

        // WiFi strength bars
        int sigX = panelX + 22;
        int sigBaseY = iy + itemH / 2 + 8;
        for (int bar = 0; bar < 4; bar++) {
            int barH2 = 4 + bar * 4;
            bool filled = net.signal >= (bar + 1) * 25;
            Color barCol = filled ?
                Color((BYTE)(220 * a), 255, 255, 255) :
                Color((BYTE)(40 * a), 255, 255, 255);
            SolidBrush barBr(barCol);
            g.FillRectangle(&barBr, sigX + bar * 5, sigBaseY - barH2, 3, barH2);
        }

        // Lock icon for secured
        if (net.secured) {
            SolidBrush lockBr(Color((BYTE)(120 * a), 200, 200, 210));
            g.FillRectangle(&lockBr, sigX + 24, sigBaseY - 8, 6, 5);
            Pen lockPen(Color((BYTE)(120 * a), 200, 200, 210), 1.0f);
            g.DrawArc(&lockPen, sigX + 24, sigBaseY - 13, 6, 8, 180, 180);
        }

        // SSID
        SolidBrush nameBr(Color((BYTE)(240 * a), 240, 240, 250));
        RectF nameRc((REAL)(panelX + 58), (REAL)(iy + 6), (REAL)(panelW - 120), 20.0f);
        g.DrawString(net.ssid.c_str(), -1, &itemFont, nameRc, &lfmt, &nameBr);

        // Status
        std::wstring status = net.connected ? L"Connected" : (net.secured ? L"Secured" : L"Open");
        SolidBrush statusBr(Color((BYTE)(120 * a), 160, 160, 170));
        RectF statusRc((REAL)(panelX + 58), (REAL)(iy + 26), (REAL)(panelW - 120), 16.0f);
        g.DrawString(status.c_str(), -1, &smallFont, statusRc, &lfmt, &statusBr);

        // Connected checkmark
        if (net.connected) {
            SolidBrush checkBr(Color((BYTE)(200 * a), 108, 203, 95));
            Font checkFont(&ff, 12.0f, FontStyleBold, UnitPixel);
            StringFormat cfmt;
            cfmt.SetAlignment(StringAlignmentCenter);
            cfmt.SetLineAlignment(StringAlignmentCenter);
            RectF checkRc((REAL)(panelX + panelW - 40), (REAL)iy, 24.0f, (REAL)itemH);
            g.DrawString(L"\u2713", -1, &checkFont, checkRc, &cfmt, &checkBr);
        }
    }

    if (g_wifiNetworks.empty()) {
        SolidBrush emptyBr(Color((BYTE)(120 * a), 150, 150, 160));
        StringFormat cfmt;
        cfmt.SetAlignment(StringAlignmentCenter);
        cfmt.SetLineAlignment(StringAlignmentCenter);
        RectF emptyRc((REAL)panelX, (REAL)(listY + 40), (REAL)panelW, 30.0f);
        g.DrawString(g_wifiScanning ? L"Scanning..." : L"No networks found", -1, &itemFont, emptyRc, &cfmt, &emptyBr);
    }
}

// ============================================================================
//  NOTIFICATIONS
// ============================================================================
void DrawNotifications(Graphics& g, int sw, int sh) {
    int baseY = sh - TASKBAR_HEIGHT - NOTIF_H - 16;

    for (int i = (int)g_notifs.size() - 1; i >= 0 && i >= (int)g_notifs.size() - 3; i--) {
        Notification& n = g_notifs[i];
        if (!n.alive || n.alpha <= 0.01f) continue;

        float a = n.alpha;
        int idx = (int)g_notifs.size() - 1 - i;
        int nx = sw - NOTIF_W - 12;
        int ny = (int)(baseY - idx * (NOTIF_H + 8) + n.offsetY);

        DrawShadow(g, nx, ny, NOTIF_W, NOTIF_H, 6, 4);

        FillRoundRectSolid(g, nx, ny, NOTIF_W, NOTIF_H, 6,
            Color((BYTE)(248 * a), 48, 48, 52));
        DrawRoundRect(g, nx, ny, NOTIF_W, NOTIF_H, 6,
            Color((BYTE)(40 * a), 255, 255, 255), 1.0f);

        // Accent line on left
        FillRoundRectSolid(g, nx, ny + 12, 3, NOTIF_H - 24, 1,
            Color((BYTE)(200 * a), W11::AccentLight.GetR(), W11::AccentLight.GetG(), W11::AccentLight.GetB()));

        // App icon
        FillRoundRectSolid(g, nx + 14, ny + 16, 36, 36, 6,
            Color((BYTE)(180 * a), W11::AccentLight.GetR(), W11::AccentLight.GetG(), W11::AccentLight.GetB()));

        FontFamily ff(L"Segoe UI");
        Font icoFont(&ff, 14.0f, FontStyleBold, UnitPixel);
        SolidBrush icoBr(Color((BYTE)(255 * a), 255, 255, 255));
        StringFormat cfmt;
        cfmt.SetAlignment(StringAlignmentCenter);
        cfmt.SetLineAlignment(StringAlignmentCenter);
        RectF icoRc((REAL)(nx + 14), (REAL)(ny + 16), 36.0f, 36.0f);
        g.DrawString(L"V", -1, &icoFont, icoRc, &cfmt, &icoBr);

        Font titleFont(&ff, 12.0f, FontStyleBold, UnitPixel);
        Font msgFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
        StringFormat lfmt;
        lfmt.SetAlignment(StringAlignmentNear);
        lfmt.SetLineAlignment(StringAlignmentNear);

        SolidBrush titleBr(Color((BYTE)(250 * a), 245, 245, 250));
        RectF titleRc((REAL)(nx + 60), (REAL)(ny + 16), (REAL)(NOTIF_W - 80), 18.0f);
        g.DrawString(n.title.c_str(), -1, &titleFont, titleRc, &lfmt, &titleBr);

        SolidBrush msgBr(Color((BYTE)(180 * a), 190, 190, 200));
        RectF msgRc((REAL)(nx + 60), (REAL)(ny + 38), (REAL)(NOTIF_W - 80), 28.0f);
        g.DrawString(n.message.c_str(), -1, &msgFont, msgRc, &lfmt, &msgBr);

        // Time
        DWORD elapsed = (GetTickCount() - n.time) / 1000;
        wchar_t timeStr[32];
        if (elapsed < 60) swprintf_s(timeStr, L"%ds", elapsed);
        else swprintf_s(timeStr, L"%dm", elapsed / 60);
        Font timeFont(&ff, 9.0f, FontStyleRegular, UnitPixel);
        SolidBrush timeBr(Color((BYTE)(80 * a), 130, 130, 140));
        StringFormat rfmt;
        rfmt.SetAlignment(StringAlignmentFar);
        rfmt.SetLineAlignment(StringAlignmentNear);
        RectF timeRc((REAL)(nx + NOTIF_W - 50), (REAL)(ny + 8), 40.0f, 14.0f);
        g.DrawString(timeStr, -1, &timeFont, timeRc, &rfmt, &timeBr);
    }
}

// ============================================================================
//  WIDGETS - Windows 11 Style
// ============================================================================
struct Widget {
    std::wstring title;
    std::wstring content;
    int height;
    Color accent;
};
std::vector<Widget> g_widgets;

void UpdateWidgets() {
    g_widgets.clear();
    time_t now = time(NULL);
    struct tm ti;
    localtime_s(&ti, &now);
    wchar_t buf[128];

    Widget w;

    w.title = L"Weather";
    w.content = L"22\xB0  Partly Cloudy\nMoscow, Russia\nFeels like 19\xB0  |  Wind 3 m/s";
    w.height = 100;
    w.accent = Color(255, 255, 200, 50);
    g_widgets.push_back(w);

    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    int cpuSim = 15 + (g_tick / 1000) % 25;
    swprintf_s(buf, L"CPU: %d%%  |  RAM: %d%%\nDisk: 62%%  |  GPU: 45%%", cpuSim, (int)mem.dwMemoryLoad);
    w.title = L"System Monitor";
    w.content = buf;
    w.height = 80;
    w.accent = Color(255, 76, 194, 255);
    g_widgets.push_back(w);

    wcsftime(buf, 128, L"%A, %d %B %Y\n%H:%M:%S", &ti);
    w.title = L"Clock";
    w.content = buf;
    w.height = 80;
    w.accent = Color(255, 200, 200, 200);
    g_widgets.push_back(w);

    w.title = L"Now Playing";
    w.content = g_musicPlaying ? L"Synthwave Mix\nArtist - Track Name\n> 2:15 / 4:30" :
        L"No music playing\nPress M to start";
    w.height = 90;
    w.accent = Color(255, 255, 100, 130);
    g_widgets.push_back(w);
}

void DrawWidgets(Graphics& g, int sw, int sh) {
    if (g_widgetsAnim <= 0.01f) return;
    float a = g_widgetsAnim;

    int panelW = WIDGET_W;
    int offsetX = (int)(-panelW * (1.0f - a));
    int baseX = 12 + offsetX;
    int baseY = 12;

    FontFamily ff(L"Segoe UI");

    for (size_t i = 0; i < g_widgets.size(); i++) {
        const Widget& w = g_widgets[i];
        int x = baseX;
        int y = baseY;

        // Card background
        DrawShadow(g, x, y, panelW, w.height, 6, 3);
        FillRoundRectSolid(g, x, y, panelW, w.height, 6,
            Color((BYTE)(235 * a), 44, 44, 48));
        DrawRoundRect(g, x, y, panelW, w.height, 6,
            Color((BYTE)(30 * a), 255, 255, 255), 1.0f);

        // Accent bar
        FillRoundRectSolid(g, x + 1, y + 12, 3, w.height - 24, 1,
            Color((BYTE)(200 * a), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));

        // Title
        Font titleFont(&ff, 11.0f, FontStyleBold, UnitPixel);
        SolidBrush titleBr(Color((BYTE)(180 * a), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));
        StringFormat lfmt;
        lfmt.SetAlignment(StringAlignmentNear);
        lfmt.SetLineAlignment(StringAlignmentNear);
        RectF titleRc((REAL)(x + 16), (REAL)(y + 10), (REAL)(panelW - 32), 16.0f);
        g.DrawString(w.title.c_str(), -1, &titleFont, titleRc, &lfmt, &titleBr);

        // Content
        Font contentFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
        SolidBrush contentBr(Color((BYTE)(220 * a), 230, 230, 240));

        std::wistringstream ss(w.content);
        std::wstring line;
        REAL ly = (REAL)(y + 30);
        while (std::getline(ss, line)) {
            RectF lineRc((REAL)(x + 16), ly, (REAL)(panelW - 32), 18.0f);
            g.DrawString(line.c_str(), -1, &contentFont, lineRc, &lfmt, &contentBr);
            ly += 18.0f;
        }

        baseY += w.height + 8;
    }
}

// ============================================================================
//  WALLPAPER
// ============================================================================
std::wstring GetCachePath() {
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(NULL, buf, MAX_PATH);
    std::wstring s = buf;
    size_t pos = s.find_last_of(L"\\");
    if (pos != std::wstring::npos) s = s.substr(0, pos);
    return s + L"\\" + WALLPAPER_CACHE;
}

bool LoadWallpaperFile(const wchar_t* path) {
    if (g_wallpaper) { delete g_wallpaper; g_wallpaper = NULL; }
    if (GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES) return false;
    Bitmap* bmp = new Bitmap(path);
    if (bmp->GetLastStatus() != Ok) { delete bmp; return false; }
    int sw = ScreenW(), sh = ScreenH();
    REAL iw = (REAL)bmp->GetWidth(), ih = (REAL)bmp->GetHeight();
    REAL scale = (std::max)((REAL)sw / iw, (REAL)sh / ih);
    int nw = (int)(iw * scale), nh = (int)(ih * scale);
    g_wallpaper = new Bitmap(sw, sh, PixelFormat32bppARGB);
    Graphics gfx(g_wallpaper);
    gfx.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    gfx.DrawImage(bmp, (sw - nw) / 2, (sh - nh) / 2, nw, nh);
    delete bmp;
    return true;
}

DWORD WINAPI WallpaperThread(LPVOID) {
    g_wallpaperLoading = true;
    std::wstring cache = GetCachePath();
    HRESULT hr = URLDownloadToFileW(NULL, WALLPAPER_URL, cache.c_str(), 0, NULL);
    if (SUCCEEDED(hr) && LoadWallpaperFile(cache.c_str())) {
        g_wallpaperReady = true;
        if (g_hWnd) InvalidateRect(g_hWnd, NULL, FALSE);
    }
    g_wallpaperLoading = false;
    return 0;
}

void DrawBackground(Graphics& g, int sw, int sh) {
    if (g_wallpaper && g_wallpaperReady) {
        g.SetInterpolationMode(InterpolationModeNearestNeighbor);
        g.DrawImage(g_wallpaper, 0, 0);
    }
    else {
        // Windows 11 default-like gradient
        LinearGradientBrush bg(Point(0, 0), Point(sw, sh),
            Color(255, 20, 40, 80), Color(255, 40, 20, 60));
        g.FillRectangle(&bg, 0, 0, sw, sh);

        // Soft light blobs
        for (int i = 0; i < 4; i++) {
            int cx = sw / 4 + i * sw / 4;
            int cy = sh / 3 + (i % 2) * sh / 3;
            int cr = 200 + i * 50;
            SolidBrush blob(Color(8, 100 + i * 30, 100, 200));
            g.FillEllipse(&blob, cx - cr, cy - cr, cr * 2, cr * 2);
        }
    }
}

// ============================================================================
//  DOUBLE BUFFERING
// ============================================================================
void EnsureBuffer(HWND hWnd, int w, int h) {
    if (g_memDC && g_bufW == w && g_bufH == h) return;
    HDC hdc = GetDC(hWnd);
    if (g_memDC) {
        SelectObject(g_memDC, g_oldBmp);
        DeleteObject(g_memBmp);
        DeleteDC(g_memDC);
    }
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    g_memDC = CreateCompatibleDC(hdc);
    g_memBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &g_bits, NULL, 0);
    g_oldBmp = (HBITMAP)SelectObject(g_memDC, g_memBmp);
    g_bufW = w; g_bufH = h;
    ReleaseDC(hWnd, hdc);
}

// ============================================================================
//  MAIN RENDER
// ============================================================================
void Render(HWND hWnd, HDC hdc, int sw, int sh) {
    EnsureBuffer(hWnd, sw, sh);

    Graphics g(g_memDC);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    g.SetCompositingQuality(CompositingQualityHighQuality);

    DrawBackground(g, sw, sh);
    DrawDesktopIcons(g);
    DrawSelectionRect(g);
    DrawWidgets(g, sw, sh);
    DrawWifiPanel(g, sw, sh);
    DrawNotifications(g, sw, sh);
    DrawTaskbar(g, sw, sh);
    DrawContextMenu(g);
    DrawStartMenu(g, sw, sh);

    // Hotkey hints (subtle)
    if (!g_startMenuOpen && !g_contextMenuOpen) {
        FontFamily ff(L"Segoe UI");
        Font hintFont(&ff, 9.0f, FontStyleRegular, UnitPixel);
        SolidBrush hintBr(Color(40, 200, 200, 210));
        StringFormat lfmt;
        lfmt.SetAlignment(StringAlignmentNear);
        lfmt.SetLineAlignment(StringAlignmentFar);
        RectF hintRc(12.0f, (REAL)(sh - TASKBAR_HEIGHT - 18), 600.0f, 14.0f);
        g.DrawString(L"ESC close | SPACE menu | M music | W widgets | N notify | F wifi",
            -1, &hintFont, hintRc, &lfmt, &hintBr);
    }

    BitBlt(hdc, 0, 0, sw, sh, g_memDC, 0, 0, SRCCOPY);
}

// ============================================================================
//  WINDOW PROCEDURE
// ============================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE:
        InitTaskbarApps();
        InitStartApps();
        InitDesktopIcons();
        InitContextMenu();
        UpdateWidgets();
        CreateThread(NULL, 0, WallpaperThread, NULL, 0, NULL);
        CreateThread(NULL, 0, WifiScanThread, NULL, 0, NULL);
        SetTimer(hWnd, 1, 1000, NULL);
        SetTimer(hWnd, 2, 16, NULL);
        PushNotification(L"VORTEX Desktop", L"Welcome! Your desktop is ready.");
        break;

    case WM_TIMER:
        g_tick = GetTickCount();
        if (wParam == 1) {
            UpdateWidgets();
        }
        else if (wParam == 2) {
            // Animations
            float targetStart = g_startMenuOpen ? 1.0f : 0.0f;
            g_startMenuAnim += (targetStart - g_startMenuAnim) * 0.35f;
            if (fabs(g_startMenuAnim - targetStart) < 0.01f) g_startMenuAnim = targetStart;

            float targetW = g_widgetsVisible ? 1.0f : 0.0f;
            g_widgetsAnim += (targetW - g_widgetsAnim) * 0.30f;
            if (fabs(g_widgetsAnim - targetW) < 0.01f) g_widgetsAnim = targetW;

            float targetWifi = g_wifiPanelOpen ? 1.0f : 0.0f;
            g_wifiPanelAnim += (targetWifi - g_wifiPanelAnim) * 0.35f;
            if (fabs(g_wifiPanelAnim - targetWifi) < 0.01f) g_wifiPanelAnim = targetWifi;

            float targetCtx = g_contextMenuOpen ? 1.0f : 0.0f;
            g_contextMenuAnim += (targetCtx - g_contextMenuAnim) * 0.45f;
            if (fabs(g_contextMenuAnim - targetCtx) < 0.01f) g_contextMenuAnim = targetCtx;

            // Notifications
            for (auto& n : g_notifs) {
                if (!n.alive) continue;
                if (n.alpha < 1.0f) n.alpha = (std::min)(1.0f, n.alpha + 0.12f);
                if (n.offsetY < 0) n.offsetY = (std::min)(0.0f, n.offsetY + 6.0f);
                if (GetTickCount() - n.time > 5000) {
                    n.alpha -= 0.06f;
                    if (n.alpha <= 0) n.alive = false;
                }
            }
            g_notifs.erase(std::remove_if(g_notifs.begin(), g_notifs.end(),
                [](const Notification& n) { return !n.alive; }), g_notifs.end());

            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        Render(hWnd, hdc, ScreenW(), ScreenH());
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_MOUSEMOVE: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);
        POINT pt = { mx, my };

        // Dragging desktop icon
        if (g_dragging && g_dragIconIdx >= 0) {
            g_dragCurrentX = mx;
            g_dragCurrentY = my;
            if (!g_dragStarted) {
                int dx = mx - (g_desktopIcons[g_dragIconIdx].pixelX + g_dragOffsetX);
                int dy = my - (g_desktopIcons[g_dragIconIdx].pixelY + g_dragOffsetY);
                if (abs(dx) > 4 || abs(dy) > 4) g_dragStarted = true;
            }
            break;
        }

        // Selection rectangle
        if (g_selecting) {
            g_selEndX = mx;
            g_selEndY = my;

            // Update selection state for icons
            int x1 = (std::min)(g_selStartX, g_selEndX);
            int y1 = (std::min)(g_selStartY, g_selEndY);
            int x2 = (std::max)(g_selStartX, g_selEndX);
            int y2 = (std::max)(g_selStartY, g_selEndY);
            RECT selRect;
            SetRect(&selRect, x1, y1, x2, y2);

            for (auto& d : g_desktopIcons) {
                RECT icoRect;
                SetRect(&icoRect, d.pixelX, d.pixelY, d.pixelX + DESKTOP_ICON_W, d.pixelY + DESKTOP_ICON_H);
                RECT intersect;
                d.selected = IntersectRect(&intersect, &selRect, &icoRect) != 0;
            }
            break;
        }

        // Context menu hover
        if (g_contextMenuOpen) {
            g_hoveredContextItem = -1;
            int menuW = CTX_MENU_W;
            int cmx = g_contextMenuX;
            int cmy = g_contextMenuY;
            int menuH = 8;
            for (const auto& item : g_contextItems)
                menuH += (item.id == 0) ? CTX_SEP_H : CTX_ITEM_H;
            menuH += 8;
            if (cmx + menuW > ScreenW()) cmx = ScreenW() - menuW - 8;
            if (cmy + menuH > ScreenH()) cmy = ScreenH() - menuH - 8;

            int cy = cmy + 8;
            for (int i = 0; i < (int)g_contextItems.size(); i++) {
                if (g_contextItems[i].id == 0) { cy += CTX_SEP_H; continue; }
                RECT ir;
                SetRect(&ir, cmx + 4, cy, cmx + menuW - 4, cy + CTX_ITEM_H);
                if (PtInRect(&ir, pt)) { g_hoveredContextItem = i; break; }
                cy += CTX_ITEM_H;
            }
        }

        // Taskbar hover
        g_hoveredTaskbarIcon = -1;
        int sw = ScreenW(), sh = ScreenH();
        int barY = sh - TASKBAR_HEIGHT;
        if (my >= barY) {
            int iconCount = (int)g_taskApps.size();
            int totalW = (iconCount + 3) * (TASKBAR_ICON_SIZE + 4);
            int startX = (sw - totalW) / 2;

            // Start button
            RECT startBtnRect;
            int btnY = barY + (TASKBAR_HEIGHT - TASKBAR_ICON_SIZE) / 2;
            SetRect(&startBtnRect, startX, btnY, startX + TASKBAR_ICON_SIZE, btnY + TASKBAR_ICON_SIZE);
            if (PtInRect(&startBtnRect, pt)) g_hoveredTaskbarIcon = -10;

            // Search
            int searchBtnX = startX + (TASKBAR_ICON_SIZE + 4);
            RECT searchRect;
            SetRect(&searchRect, searchBtnX, btnY, searchBtnX + TASKBAR_ICON_SIZE, btnY + TASKBAR_ICON_SIZE);
            if (PtInRect(&searchRect, pt)) g_hoveredTaskbarIcon = -11;

            // Task view
            int tvBtnX = startX + 2 * (TASKBAR_ICON_SIZE + 4);
            RECT tvRect;
            SetRect(&tvRect, tvBtnX, btnY, tvBtnX + TASKBAR_ICON_SIZE, btnY + TASKBAR_ICON_SIZE);
            if (PtInRect(&tvRect, pt)) g_hoveredTaskbarIcon = -12;

            // App icons
            for (int i = 0; i < iconCount; i++) {
                if (PtInRect(&g_taskApps[i].bounds, pt)) {
                    g_hoveredTaskbarIcon = i;
                    break;
                }
            }
        }

        // Desktop icon hover
        g_hoveredDesktopIcon = -1;
        if (!g_startMenuOpen && !g_contextMenuOpen) {
            for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
                RECT r;
                SetRect(&r, g_desktopIcons[i].pixelX, g_desktopIcons[i].pixelY,
                    g_desktopIcons[i].pixelX + DESKTOP_ICON_W, g_desktopIcons[i].pixelY + DESKTOP_ICON_H);
                if (PtInRect(&r, pt)) {
                    g_hoveredDesktopIcon = i;
                    break;
                }
            }
        }

        // WiFi hover
        g_hoveredWifiItem = -1;
        if (g_wifiPanelOpen && g_wifiPanelAnim > 0.5f) {
            int panelX = sw - WIFI_PANEL_W - 12;
            int panelY = sh - TASKBAR_HEIGHT - WIFI_PANEL_H - 12;
            int toggleH = 70;
            int listY = panelY + 16 + toggleH + 12 + 10 + 40;
            int itemH = 48;
            for (int i = 0; i < (int)g_wifiNetworks.size() && i < 6; i++) {
                int iy = listY + i * itemH;
                RECT ir;
                SetRect(&ir, panelX + 8, iy, panelX + WIFI_PANEL_W - 8, iy + itemH);
                if (PtInRect(&ir, pt)) { g_hoveredWifiItem = i; break; }
            }
        }

        // Start menu hover
        g_hoveredStartItem = -1;
        if (g_startMenuOpen && g_startMenuAnim > 0.5f) {
            int menuX = (sw - START_MENU_W) / 2;
            int menuY2 = sh - TASKBAR_HEIGHT - START_MENU_H - 12;
            int gridX = menuX + 28;
            int gridY = menuY2 + 108;
            int cols = 6;
            int cellW = (START_MENU_W - 56) / cols;
            int cellH = 72;
            for (int i = 0; i < (int)g_startApps.size() && i < 18; i++) {
                int row = i / cols;
                int col = i % cols;
                int ix = gridX + col * cellW;
                int iy = gridY + row * cellH;
                RECT cr;
                SetRect(&cr, ix, iy, ix + cellW, iy + cellH);
                if (PtInRect(&cr, pt)) { g_hoveredStartItem = i; break; }
            }
        }
        break;
    }

    case WM_LBUTTONDOWN: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);
        POINT pt = { mx, my };

        // Close context menu
        if (g_contextMenuOpen) {
            if (g_hoveredContextItem >= 0) {
                int id = g_contextItems[g_hoveredContextItem].id;
                switch (id) {
                case 1: CreateNewFolderOnDesktop();
                    PushNotification(L"New Folder", L"Folder created on Desktop"); break;
                case 2: InvalidateRect(hWnd, NULL, FALSE); break;
                case 3: ShellExecute(NULL, L"open", L"ms-settings:display", NULL, NULL, SW_SHOW); break;
                case 4: ShellExecute(NULL, L"open", L"cmd.exe", NULL, NULL, SW_SHOW); break;
                case 10: // Open
                    for (auto& d : g_desktopIcons)
                        if (d.selected) ShellExecute(NULL, L"open", d.action.c_str(), NULL, NULL, SW_SHOW);
                    break;
                case 23: ShellExecute(NULL, L"open", L"ms-settings:personalization", NULL, NULL, SW_SHOW); break;
                }
            }
            g_contextMenuOpen = false;
            break;
        }

        // Start menu
        if (g_startMenuOpen && g_startMenuAnim > 0.5f) {
            if (g_hoveredStartItem >= 0 && g_hoveredStartItem < (int)g_startApps.size()) {
                ShellExecute(NULL, L"open", g_startApps[g_hoveredStartItem].exec.c_str(), NULL, NULL, SW_SHOW);
                g_startMenuOpen = false;
            }
            else {
                int sw = ScreenW(), sh = ScreenH();
                int menuX = (sw - START_MENU_W) / 2;
                int menuY2 = sh - TASKBAR_HEIGHT - START_MENU_H - 12;
                RECT mr;
                SetRect(&mr, menuX, menuY2, menuX + START_MENU_W, menuY2 + START_MENU_H);
                if (!PtInRect(&mr, pt)) g_startMenuOpen = false;
            }
            break;
        }

        // WiFi panel close on outside click
        if (g_wifiPanelOpen && g_wifiPanelAnim > 0.5f) {
            int sw = ScreenW(), sh = ScreenH();
            int panelX = sw - WIFI_PANEL_W - 12;
            int panelY = sh - TASKBAR_HEIGHT - WIFI_PANEL_H - 12;
            RECT pr;
            SetRect(&pr, panelX, panelY, panelX + WIFI_PANEL_W, panelY + WIFI_PANEL_H);
            if (!PtInRect(&pr, pt) && my < ScreenH() - TASKBAR_HEIGHT) {
                g_wifiPanelOpen = false;
            }
        }

        // Taskbar clicks
        int sw = ScreenW(), sh = ScreenH();
        if (my >= sh - TASKBAR_HEIGHT) {
            if (g_hoveredTaskbarIcon == -10) {
                g_startMenuOpen = !g_startMenuOpen;
                g_wifiPanelOpen = false;
            }
            else if (g_hoveredTaskbarIcon >= 0) {
                ShellExecute(NULL, L"open", g_taskApps[g_hoveredTaskbarIcon].exec.c_str(), NULL, NULL, SW_SHOW);
            }
            break;
        }

        // Desktop icon click / drag start
        bool clickedIcon = false;
        for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
            RECT r;
            SetRect(&r, g_desktopIcons[i].pixelX, g_desktopIcons[i].pixelY,
                g_desktopIcons[i].pixelX + DESKTOP_ICON_W, g_desktopIcons[i].pixelY + DESKTOP_ICON_H);
            if (PtInRect(&r, pt)) {
                clickedIcon = true;

                // Select / deselect logic
                if (!(GetKeyState(VK_CONTROL) & 0x8000)) {
                    for (auto& d : g_desktopIcons) d.selected = false;
                }
                g_desktopIcons[i].selected = true;

                // Start drag
                g_dragging = true;
                g_dragIconIdx = i;
                g_dragOffsetX = mx - g_desktopIcons[i].pixelX;
                g_dragOffsetY = my - g_desktopIcons[i].pixelY;
                g_dragCurrentX = mx;
                g_dragCurrentY = my;
                g_dragStarted = false;
                SetCapture(hWnd);
                break;
            }
        }

        if (!clickedIcon && my < sh - TASKBAR_HEIGHT) {
            // Deselect all
            for (auto& d : g_desktopIcons) d.selected = false;

            // Start selection rectangle
            g_selecting = true;
            g_selStartX = mx;
            g_selStartY = my;
            g_selEndX = mx;
            g_selEndY = my;
            SetCapture(hWnd);
        }
        break;
    }

    case WM_LBUTTONUP: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);

        if (g_dragging && g_dragIconIdx >= 0) {
            if (g_dragStarted) {
                // Snap to grid
                int newX = mx - g_dragOffsetX;
                int newY = my - g_dragOffsetY;
                int oldCol = g_desktopIcons[g_dragIconIdx].gridCol;
                int oldRow = g_desktopIcons[g_dragIconIdx].gridRow;

                SnapToGrid(g_desktopIcons[g_dragIconIdx], newX + DESKTOP_ICON_W / 2, newY + DESKTOP_ICON_H / 2);

                // Check collision
                if (IsGridOccupied(g_desktopIcons[g_dragIconIdx].gridCol,
                    g_desktopIcons[g_dragIconIdx].gridRow, g_dragIconIdx)) {
                    g_desktopIcons[g_dragIconIdx].gridCol = oldCol;
                    g_desktopIcons[g_dragIconIdx].gridRow = oldRow;
                    CalcIconPixelPos(g_desktopIcons[g_dragIconIdx]);
                }
            }
            g_dragging = false;
            g_dragIconIdx = -1;
            g_dragStarted = false;
            ReleaseCapture();
        }

        if (g_selecting) {
            g_selecting = false;
            ReleaseCapture();
        }
        break;
    }

    case WM_LBUTTONDBLCLK: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);
        POINT pt = { mx, my };

        for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
            RECT r;
            SetRect(&r, g_desktopIcons[i].pixelX, g_desktopIcons[i].pixelY,
                g_desktopIcons[i].pixelX + DESKTOP_ICON_W, g_desktopIcons[i].pixelY + DESKTOP_ICON_H);
            if (PtInRect(&r, pt)) {
                ShellExecute(NULL, L"open", g_desktopIcons[i].action.c_str(), NULL, NULL, SW_SHOW);
                break;
            }
        }
        break;
    }

    case WM_RBUTTONDOWN: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);
        POINT pt = { mx, my };

        if (g_startMenuOpen) { g_startMenuOpen = false; break; }
        if (g_contextMenuOpen) { g_contextMenuOpen = false; break; }

        // Check if right-clicked on an icon
        bool onIcon = false;
        for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
            RECT r;
            SetRect(&r, g_desktopIcons[i].pixelX, g_desktopIcons[i].pixelY,
                g_desktopIcons[i].pixelX + DESKTOP_ICON_W, g_desktopIcons[i].pixelY + DESKTOP_ICON_H);
            if (PtInRect(&r, pt)) {
                onIcon = true;
                if (!g_desktopIcons[i].selected) {
                    for (auto& d : g_desktopIcons) d.selected = false;
                    g_desktopIcons[i].selected = true;
                }
                break;
            }
        }

        InitContextMenu(onIcon);
        g_contextMenuX = mx;
        g_contextMenuY = my;
        g_contextMenuOpen = true;
        g_contextMenuAnim = 0.0f;
        g_hoveredContextItem = -1;
        break;
    }

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            if (g_contextMenuOpen) g_contextMenuOpen = false;
            else if (g_wifiPanelOpen) g_wifiPanelOpen = false;
            else if (g_startMenuOpen) g_startMenuOpen = false;
            else PostQuitMessage(0);
        }
        else if (wParam == VK_SPACE) {
            g_startMenuOpen = !g_startMenuOpen;
            g_contextMenuOpen = false;
            g_wifiPanelOpen = false;
        }
        else if (wParam == 'M') {
            g_musicPlaying = !g_musicPlaying;
            PushNotification(L"Music", g_musicPlaying ? L"Now playing" : L"Paused");
        }
        else if (wParam == 'W') {
            g_widgetsVisible = !g_widgetsVisible;
        }
        else if (wParam == 'N') {
            PushNotification(L"Notification", L"This is a test notification");
        }
        else if (wParam == 'F') {
            g_wifiPanelOpen = !g_wifiPanelOpen;
            g_startMenuOpen = false;
            if (g_wifiPanelOpen && !g_wifiScanning)
                CreateThread(NULL, 0, WifiScanThread, NULL, 0, NULL);
        }
        else if (wParam == VK_DELETE) {
            // Delete selected desktop icons (except system ones)
            g_desktopIcons.erase(
                std::remove_if(g_desktopIcons.begin(), g_desktopIcons.end(),
                    [](const DesktopIcon& d) {
                        return d.selected && d.iconType == 2; // Only user-created folders
                    }), g_desktopIcons.end());
        }
        else if (wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
            // Ctrl+A - select all
            for (auto& d : g_desktopIcons) d.selected = true;
        }
        break;

    case WM_DESTROY:
        KillTimer(hWnd, 1);
        KillTimer(hWnd, 2);
        if (g_wallpaper) { delete g_wallpaper; g_wallpaper = NULL; }
        if (g_memDC) {
            SelectObject(g_memDC, g_oldBmp);
            DeleteObject(g_memBmp);
            DeleteDC(g_memDC);
        }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// ============================================================================
//  ENTRY POINT
// ============================================================================
int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nShow) {
    GdiplusStartupInput gdipIn;
    GdiplusStartup(&g_gdiplusToken, &gdipIn, NULL);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_DBLCLKS;
    wc.lpszClassName = L"VORTEX_Desktop_v3";
    RegisterClassExW(&wc);

    int sw = ScreenW(), sh = ScreenH();

    g_hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"VORTEX_Desktop_v3", L"VORTEX Desktop v3",
        WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN,
        0, 0, sw, sh,
        NULL, NULL, hInst, NULL);

    HWND hTaskbar = FindWindow(L"Shell_TrayWnd", NULL);
    if (hTaskbar) ShowWindow(hTaskbar, SW_HIDE);

    ShowWindow(g_hWnd, nShow);
    UpdateWindow(g_hWnd);

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
