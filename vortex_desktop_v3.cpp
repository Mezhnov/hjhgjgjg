/*
 * VORTEX Desktop Environment v4.0 - Windows 11 Fluent Design (Enhanced)
 * Complete redesign with real file browsing, disk info, WiFi scanning
 *
 * Enhancements over v3:
 * - High quality Windows 11 taskbar with pixel-perfect system tray
 * - Real file explorer window (browse real folders)
 * - This PC shows real disk drives with capacity
 * - WiFi panel with real network scanning
 * - Smooth cubic-ease animations everywhere
 * - Internal window system (draggable, resizable explorer windows)
 * - Proper folder/file icons
 * - Volume slider, brightness in quick settings
 *
 * Controls:
 * ESC       - close window / exit
 * SPACE     - toggle Start Menu
 * M         - toggle music widget
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
#include <set>

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
//  SMOOTH ANIMATION HELPERS
// ============================================================================
inline float EaseOutCubic(float t) {
    t = t - 1.0f;
    return t * t * t + 1.0f;
}
inline float EaseInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}
inline float SmoothLerp(float current, float target, float speed) {
    float diff = target - current;
    if (fabsf(diff) < 0.002f) return target;
    return current + diff * speed;
}

// ============================================================================
//  WINDOWS 11 COLOR PALETTE
// ============================================================================
namespace W11 {
    const Color Accent(255, 0, 103, 192);
    const Color AccentLight(255, 76, 194, 255);
    const Color AccentDark(255, 0, 84, 153);
    const Color AccentSubtle(40, 0, 103, 192);

    const Color SurfaceBase(255, 32, 32, 32);
    const Color SurfaceCard(230, 44, 44, 44);
    const Color SurfaceCardHover(240, 55, 55, 55);
    const Color SurfaceFlyout(245, 44, 44, 44);
    const Color SurfaceOverlay(180, 0, 0, 0);
    const Color SurfaceStroke(60, 255, 255, 255);
    const Color SurfaceStrokeLight(30, 255, 255, 255);

    const Color MicaBg(220, 32, 32, 32);
    const Color AcrylicBg(200, 44, 44, 44);

    const Color TextPrimary(255, 255, 255, 255);
    const Color TextSecondary(255, 200, 200, 200);
    const Color TextTertiary(255, 150, 150, 150);
    const Color TextDisabled(255, 100, 100, 100);

    const Color Success(255, 108, 203, 95);
    const Color Warning(255, 252, 225, 0);
    const Color Error(255, 255, 99, 97);
    const Color Info(255, 98, 205, 255);

    const Color TaskbarBg(245, 28, 28, 28);
    const Color TaskbarHover(255, 50, 50, 50);
    const Color TaskbarActive(255, 60, 60, 60);
    const Color TaskbarIndicator(255, 76, 194, 255);

    const Color StartBg(240, 38, 38, 38);
    const Color StartSearch(255, 55, 55, 55);
    const Color StartItemHover(255, 60, 60, 60);

    const Color CtxBg(245, 44, 44, 44);
    const Color CtxHover(255, 60, 60, 60);

    const Color SelectionRect(80, 0, 103, 192);
    const Color SelectionBorder(200, 0, 103, 192);
    const Color IconSelectedBg(60, 0, 103, 192);
    const Color IconHoverBg(40, 255, 255, 255);

    // Explorer window
    const Color WinTitleBar(255, 32, 32, 32);
    const Color WinBody(255, 30, 30, 30);
    const Color WinSidebar(255, 36, 36, 36);
    const Color WinToolbar(255, 40, 40, 40);
    const Color WinItemHover(255, 50, 50, 55);
    const Color WinItemSelected(255, 0, 80, 160);
    const Color WinScrollbar(255, 55, 55, 55);
    const Color WinScrollThumb(255, 80, 80, 85);
}

// ============================================================================
//  CONSTANTS
// ============================================================================
const wchar_t* WALLPAPER_URL = L"https://images.wallpaperscraft.com/image/single/lake_mountains_trees_1219008_1920x1080.jpg";
const wchar_t* WALLPAPER_CACHE = L"vortex_wallpaper_cache.jpg";

const int TASKBAR_HEIGHT = 48;
const int TASKBAR_ICON_SIZE = 40;

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
const int WIFI_PANEL_H = 440;

const int NOTIF_W = 360;
const int NOTIF_H = 80;

const int WIDGET_W = 320;

// ============================================================================
//  EXPLORER WINDOW - Full file browser
// ============================================================================
struct FileItem {
    std::wstring name;
    std::wstring fullPath;
    bool isDirectory;
    bool isDrive;
    ULONGLONG fileSize;
    ULONGLONG totalSpace;  // for drives
    ULONGLONG freeSpace;   // for drives
    std::wstring driveType;
    FILETIME modTime;
};

struct ExplorerWindow {
    int id;
    std::wstring title;
    std::wstring currentPath; // empty = "This PC" view
    int x, y, w, h;
    float animAlpha;
    float targetAlpha;
    bool visible;
    bool maximized;
    bool dragging;
    int dragOffX, dragOffY;
    bool resizing;
    int resizeEdge; // 1=right, 2=bottom, 3=corner
    int resizeStartX, resizeStartY, resizeStartW, resizeStartH;

    std::vector<FileItem> items;
    int scrollOffset;
    int hoveredItem;
    int selectedItem;
    std::vector<std::wstring> pathHistory;
    int historyIndex;

    // Sidebar
    int hoveredSidebarItem;

    // Animation
    float scrollAnim;
    float targetScroll;

    // Close/min/max button hover
    int hoveredTitleBtn; // 0=none, 1=min, 2=max, 3=close

    // Address bar
    bool addressBarActive;
};
std::vector<ExplorerWindow> g_explorers;
int g_nextExplorerId = 1;
int g_activeExplorerId = -1;
int g_explorerDragging = -1;
int g_explorerResizing = -1;

// ============================================================================
//  GLOBAL STATE
// ============================================================================
HWND g_hWnd = NULL;
ULONG_PTR g_gdiplusToken = 0;
DWORD g_tick = 0;

HDC g_memDC = NULL;
HBITMAP g_memBmp = NULL, g_oldBmp = NULL;
void* g_bits = NULL;
int g_bufW = 0, g_bufH = 0;

Bitmap* g_wallpaper = NULL;
bool g_wallpaperReady = false;
bool g_wallpaperLoading = false;

bool g_startMenuOpen = false;
bool g_widgetsVisible = false;
bool g_musicPlaying = false;
bool g_wifiPanelOpen = false;
bool g_contextMenuOpen = false;
bool g_quickSettingsOpen = false;

int g_hoveredTaskbarIcon = -1;
int g_hoveredStartItem = -1;
int g_hoveredDesktopIcon = -1;
int g_hoveredWifiItem = -1;
int g_contextMenuX = 0, g_contextMenuY = 0;
int g_hoveredContextItem = -1;

float g_startMenuAnim = 0.0f;
float g_widgetsAnim = 0.0f;
float g_wifiPanelAnim = 0.0f;
float g_contextMenuAnim = 0.0f;
float g_quickSettingsAnim = 0.0f;

bool g_selecting = false;
int g_selStartX = 0, g_selStartY = 0;
int g_selEndX = 0, g_selEndY = 0;

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
            if (!IsGridOccupied(c, r)) { col = c; row = r; return; }
        }
    }
    col = 0; row = 0;
}

void InitDesktopIcons() {
    g_desktopIcons.clear();
    auto add = [](const std::wstring& name, const std::wstring& act,
        int col, int row, Color c, int type) {
            DesktopIcon d;
            d.name = name; d.action = act;
            d.gridCol = col; d.gridRow = row;
            d.selected = false; d.iconColor = c; d.iconType = type;
            CalcIconPixelPos(d);
            g_desktopIcons.push_back(d);
        };

    add(L"This PC", L"__thispc__", 0, 0, Color(255, 255, 200, 50), 0);
    add(L"Recycle Bin", L"shell:RecycleBinFolder", 0, 1, Color(255, 160, 160, 160), 1);
    add(L"Documents", L"__folder__", 0, 2, Color(255, 255, 210, 76), 5);
    add(L"Downloads", L"__folder__", 0, 3, Color(255, 76, 194, 255), 5);
    add(L"Pictures", L"__folder__", 0, 4, Color(255, 160, 120, 255), 5);
    add(L"Music", L"__folder__", 0, 5, Color(255, 255, 100, 130), 5);
}

// ============================================================================
//  EXPLORER FUNCTIONS - Real file system browsing
// ============================================================================
int ScreenW() { return GetSystemMetrics(SM_CXSCREEN); }
int ScreenH() { return GetSystemMetrics(SM_CYSCREEN); }

void LoadDrives(ExplorerWindow& win) {
    win.items.clear();
    DWORD drives = GetLogicalDrives();
    for (int i = 0; i < 26; i++) {
        if (drives & (1 << i)) {
            wchar_t root[4] = { (wchar_t)('A' + i), L':', L'\\', 0 };
            FileItem fi;
            fi.name = std::wstring(1, (wchar_t)('A' + i)) + L":";
            fi.fullPath = root;
            fi.isDirectory = true;
            fi.isDrive = true;
            fi.fileSize = 0;
            fi.totalSpace = 0;
            fi.freeSpace = 0;

            UINT dtype = GetDriveTypeW(root);
            switch (dtype) {
            case DRIVE_FIXED: fi.driveType = L"Local Disk"; break;
            case DRIVE_REMOVABLE: fi.driveType = L"Removable Disk"; break;
            case DRIVE_REMOTE: fi.driveType = L"Network Drive"; break;
            case DRIVE_CDROM: fi.driveType = L"CD-ROM"; break;
            default: fi.driveType = L"Drive"; break;
            }

            ULARGE_INTEGER freeBytes, totalBytes, totalFree;
            if (GetDiskFreeSpaceExW(root, &freeBytes, &totalBytes, &totalFree)) {
                fi.totalSpace = totalBytes.QuadPart;
                fi.freeSpace = freeBytes.QuadPart;
            }

            // Get volume label
            wchar_t volName[MAX_PATH] = { 0 };
            if (GetVolumeInformationW(root, volName, MAX_PATH, NULL, NULL, NULL, NULL, 0)) {
                if (wcslen(volName) > 0) {
                    fi.name = std::wstring(volName) + L" (" + fi.name + L")";
                }
                else {
                    fi.name = fi.driveType + L" (" + std::wstring(1, (wchar_t)('A' + i)) + L":)";
                }
            }

            memset(&fi.modTime, 0, sizeof(FILETIME));
            win.items.push_back(fi);
        }
    }
}

void LoadDirectory(ExplorerWindow& win, const std::wstring& path) {
    win.items.clear();
    win.scrollOffset = 0;
    win.scrollAnim = 0;
    win.targetScroll = 0;
    win.selectedItem = -1;
    win.hoveredItem = -1;

    std::wstring searchPath = path;
    if (searchPath.back() != L'\\') searchPath += L'\\';
    searchPath += L'*';

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    // Directories first
    std::vector<FileItem> dirs, files;

    do {
        if (wcscmp(fd.cFileName, L".") == 0) continue;

        FileItem fi;
        fi.name = fd.cFileName;
        fi.isDirectory = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        fi.isDrive = false;
        fi.modTime = fd.ftLastWriteTime;

        std::wstring base = path;
        if (base.back() != L'\\') base += L'\\';
        fi.fullPath = base + fd.cFileName;
        fi.totalSpace = 0;
        fi.freeSpace = 0;

        if (fi.isDirectory) {
            fi.fileSize = 0;
            dirs.push_back(fi);
        }
        else {
            fi.fileSize = ((ULONGLONG)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
            files.push_back(fi);
        }
    } while (FindNextFileW(hFind, &fd));
    FindClose(hFind);

    // Sort: directories first, then files, alphabetical
    std::sort(dirs.begin(), dirs.end(), [](const FileItem& a, const FileItem& b) {
        return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0;
        });
    std::sort(files.begin(), files.end(), [](const FileItem& a, const FileItem& b) {
        return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0;
        });

    for (auto& d : dirs) win.items.push_back(d);
    for (auto& f : files) win.items.push_back(f);
}

void NavigateExplorer(ExplorerWindow& win, const std::wstring& path) {
    // Save to history
    if (win.historyIndex < (int)win.pathHistory.size() - 1) {
        win.pathHistory.resize(win.historyIndex + 1);
    }
    win.pathHistory.push_back(path);
    win.historyIndex = (int)win.pathHistory.size() - 1;

    win.currentPath = path;
    if (path.empty()) {
        win.title = L"This PC";
        LoadDrives(win);
    }
    else {
        // Extract folder name for title
        std::wstring title = path;
        if (title.back() == L'\\') title.pop_back();
        size_t pos = title.find_last_of(L'\\');
        if (pos != std::wstring::npos) title = title.substr(pos + 1);
        if (title.length() == 2 && title[1] == L':') title = L"Local Disk (" + title + L")";
        win.title = title;
        LoadDirectory(win, path);
    }
}

std::wstring GetSpecialFolderPath(const std::wstring& name) {
    wchar_t path[MAX_PATH];
    if (name == L"Documents") {
        SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, path);
        return path;
    }
    else if (name == L"Downloads") {
        SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path);
        return std::wstring(path) + L"\\Downloads";
    }
    else if (name == L"Pictures") {
        SHGetFolderPathW(NULL, CSIDL_MYPICTURES, NULL, 0, path);
        return path;
    }
    else if (name == L"Music") {
        SHGetFolderPathW(NULL, CSIDL_MYMUSIC, NULL, 0, path);
        return path;
    }
    else if (name == L"Desktop") {
        SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path);
        return path;
    }
    else if (name == L"Videos") {
        SHGetFolderPathW(NULL, CSIDL_MYVIDEO, NULL, 0, path);
        return path;
    }
    return L"";
}

ExplorerWindow* CreateExplorerWindow(const std::wstring& path, const std::wstring& title) {
    ExplorerWindow win;
    win.id = g_nextExplorerId++;
    win.title = title;
    win.currentPath = path;

    // Center with slight offset for multiple windows
    int offset = ((int)g_explorers.size() % 5) * 30;
    win.x = (ScreenW() - 900) / 2 + offset;
    win.y = (ScreenH() - 600) / 2 + offset;
    win.w = 900;
    win.h = 600;
    win.animAlpha = 0.0f;
    win.targetAlpha = 1.0f;
    win.visible = true;
    win.maximized = false;
    win.dragging = false;
    win.resizing = false;
    win.scrollOffset = 0;
    win.hoveredItem = -1;
    win.selectedItem = -1;
    win.historyIndex = -1;
    win.hoveredSidebarItem = -1;
    win.scrollAnim = 0;
    win.targetScroll = 0;
    win.hoveredTitleBtn = 0;
    win.addressBarActive = false;

    if (path.empty()) {
        LoadDrives(win);
        win.pathHistory.push_back(L"");
        win.historyIndex = 0;
    }
    else {
        LoadDirectory(win, path);
        win.pathHistory.push_back(path);
        win.historyIndex = 0;
    }

    g_explorers.push_back(win);
    g_activeExplorerId = win.id;
    return &g_explorers.back();
}

void CloseExplorer(int id) {
    for (auto it = g_explorers.begin(); it != g_explorers.end(); ++it) {
        if (it->id == id) {
            it->targetAlpha = 0.0f;
            // Will be removed when alpha reaches 0
            break;
        }
    }
}

ExplorerWindow* FindExplorer(int id) {
    for (auto& w : g_explorers) if (w.id == id) return &w;
    return nullptr;
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
    float hoverAnim;
};
std::vector<TaskbarApp> g_taskApps;

void InitTaskbarApps() {
    g_taskApps.clear();
    auto add = [](const std::wstring& n, const std::wstring& e, const std::wstring& ico,
        Color c, bool run, bool active) {
            TaskbarApp a;
            a.name = n; a.exec = e; a.iconLabel = ico; a.accentColor = c;
            a.pinned = true; a.running = run; a.active = active;
            a.hoverAnim = 0.0f;
            memset(&a.bounds, 0, sizeof(RECT));
            g_taskApps.push_back(a);
        };

    add(L"File Explorer", L"__thispc__", L"E", Color(255, 255, 200, 50), false, false);
    add(L"Microsoft Edge", L"msedge.exe", L"e", Color(255, 0, 150, 255), false, false);
    add(L"Terminal", L"cmd.exe", L">_", Color(255, 48, 48, 48), false, false);
    add(L"Notepad", L"notepad.exe", L"N", Color(255, 100, 180, 255), false, false);
    add(L"Settings", L"ms-settings:", L"S", Color(255, 100, 100, 120), false, false);
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
        StartApp a; a.name = n; a.iconLabel = ico; a.exec = exec; a.color = c;
        g_startApps.push_back(a);
        };
    add(L"File Explorer", L"E", L"__thispc__", Color(255, 255, 200, 50));
    add(L"Edge", L"e", L"msedge.exe", Color(255, 0, 150, 255));
    add(L"Terminal", L">_", L"cmd.exe", Color(255, 48, 48, 48));
    add(L"Notepad", L"N", L"notepad.exe", Color(255, 100, 180, 255));
    add(L"Settings", L"S", L"ms-settings:", Color(255, 100, 100, 120));
    add(L"Calculator", L"=", L"calc.exe", Color(255, 60, 60, 60));
    add(L"Paint", L"P", L"mspaint.exe", Color(255, 255, 100, 100));
    add(L"Photos", L"Ph", L"ms-photos:", Color(255, 255, 180, 50));
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
        w.ssid = L"CoffeeShop_Free"; w.signal = 35; w.secured = false; w.connected = false;
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
    int id;
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
            d.action = L"__folder__";
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
    GraphicsPath p; RoundedRectPath(p, x, y, w, h, r);
    g.FillPath(&br, &p);
}

void DrawRoundRect(Graphics& g, int x, int y, int w, int h, int r, Color c, float width = 1.0f) {
    GraphicsPath p; RoundedRectPath(p, x, y, w, h, r);
    Pen pen(c, width); g.DrawPath(&pen, &p);
}

void FillRoundRectSolid(Graphics& g, int x, int y, int w, int h, int r, Color c) {
    SolidBrush br(c); FillRoundRect(g, x, y, w, h, r, br);
}

void DrawShadow(Graphics& g, int x, int y, int w, int h, int r, int layers = 6) {
    for (int i = layers; i >= 1; i--) {
        int sp = i * 3;
        int alpha = (int)(18.0f * (float)(layers - i + 1) / (float)layers);
        FillRoundRectSolid(g, x - sp + 2, y - sp + 4, w + sp * 2, h + sp * 2, r + sp / 2,
            Color((BYTE)alpha, 0, 0, 0));
    }
}

// Format file size
std::wstring FormatSize(ULONGLONG bytes) {
    wchar_t buf[64];
    if (bytes >= (ULONGLONG)1024 * 1024 * 1024 * 1024)
        swprintf_s(buf, L"%.1f TB", (double)bytes / (1024.0 * 1024 * 1024 * 1024));
    else if (bytes >= (ULONGLONG)1024 * 1024 * 1024)
        swprintf_s(buf, L"%.1f GB", (double)bytes / (1024.0 * 1024 * 1024));
    else if (bytes >= (ULONGLONG)1024 * 1024)
        swprintf_s(buf, L"%.1f MB", (double)bytes / (1024.0 * 1024));
    else if (bytes >= 1024)
        swprintf_s(buf, L"%.1f KB", (double)bytes / 1024.0);
    else
        swprintf_s(buf, L"%llu B", bytes);
    return buf;
}

// ============================================================================
//  DRAW FILE/FOLDER ICONS
// ============================================================================
void DrawFolderIcon(Graphics& g, int x, int y, int size, Color color, float alpha = 1.0f) {
    Color c(
        (BYTE)(color.GetA() * alpha),
        color.GetR(), color.GetG(), color.GetB()
    );
    // Tab
    FillRoundRectSolid(g, x + 2, y + 2, (int)(size * 0.4f), (int)(size * 0.15f), 2, c);
    // Body
    FillRoundRectSolid(g, x + 2, y + (int)(size * 0.12f), size - 4, (int)(size * 0.75f), 3, c);
    // Front face lighter
    Color lighter(
        (BYTE)(255 * alpha),
        (BYTE)(std::min)(255, (int)color.GetR() + 35),
        (BYTE)(std::min)(255, (int)color.GetG() + 35),
        (BYTE)(std::min)(255, (int)color.GetB() + 35)
    );
    FillRoundRectSolid(g, x + 2, y + (int)(size * 0.35f), size - 4, (int)(size * 0.52f), 3, lighter);
}

void DrawFileIcon(Graphics& g, int x, int y, int size, const std::wstring& ext, float alpha = 1.0f) {
    // Page
    int pw = (int)(size * 0.7f);
    int ph = (int)(size * 0.85f);
    int px = x + (size - pw) / 2;
    int py = y + (size - ph) / 2;

    FillRoundRectSolid(g, px, py, pw, ph, 2, Color((BYTE)(240 * alpha), 220, 220, 230));

    // Corner fold
    int foldSize = (int)(pw * 0.3f);
    Point foldPts[3] = {
        Point(px + pw - foldSize, py),
        Point(px + pw, py),
        Point(px + pw, py + foldSize)
    };
    SolidBrush foldBr(Color((BYTE)(200 * alpha), 180, 180, 195));
    g.FillPolygon(&foldBr, foldPts, 3);

    // Extension label
    if (!ext.empty()) {
        Color extColor((BYTE)(200 * alpha), 80, 80, 100);
        // Choose color by extension
        if (ext == L".txt" || ext == L".log") extColor = Color((BYTE)(200 * alpha), 100, 150, 200);
        else if (ext == L".exe" || ext == L".msi") extColor = Color((BYTE)(200 * alpha), 200, 100, 80);
        else if (ext == L".jpg" || ext == L".png" || ext == L".bmp") extColor = Color((BYTE)(200 * alpha), 100, 180, 100);
        else if (ext == L".mp3" || ext == L".wav") extColor = Color((BYTE)(200 * alpha), 200, 100, 200);
        else if (ext == L".zip" || ext == L".rar" || ext == L".7z") extColor = Color((BYTE)(200 * alpha), 200, 180, 50);
        else if (ext == L".doc" || ext == L".docx") extColor = Color((BYTE)(200 * alpha), 50, 100, 200);
        else if (ext == L".pdf") extColor = Color((BYTE)(200 * alpha), 200, 50, 50);

        FillRoundRectSolid(g, px + 2, py + ph - (int)(ph * 0.35f), pw - 4, (int)(ph * 0.3f), 2, extColor);

        FontFamily ff(L"Segoe UI");
        Font extFont(&ff, 8.0f, FontStyleBold, UnitPixel);
        SolidBrush extBr(Color((BYTE)(255 * alpha), 255, 255, 255));
        StringFormat cfmt;
        cfmt.SetAlignment(StringAlignmentCenter);
        cfmt.SetLineAlignment(StringAlignmentCenter);
        std::wstring shortExt = ext.substr(0, 4);
        for (auto& ch : shortExt) ch = towupper(ch);
        RectF extRc((REAL)(px + 2), (REAL)(py + ph - (int)(ph * 0.35f)), (REAL)(pw - 4), (REAL)((int)(ph * 0.3f)));
        g.DrawString(shortExt.c_str(), -1, &extFont, extRc, &cfmt, &extBr);
    }
}

void DrawDriveIcon(Graphics& g, int x, int y, int size, float usedPercent, float alpha = 1.0f) {
    // Drive body
    FillRoundRectSolid(g, x + 2, y + (int)(size * 0.15f), size - 4, (int)(size * 0.7f), 4,
        Color((BYTE)(230 * alpha), 70, 70, 80));

    // Usage bar background
    int barX = x + 6;
    int barY = y + (int)(size * 0.55f);
    int barW = size - 12;
    int barH = 6;
    FillRoundRectSolid(g, barX, barY, barW, barH, 2, Color((BYTE)(200 * alpha), 40, 40, 45));

    // Usage bar fill
    int fillW = (int)(barW * usedPercent);
    Color barColor = usedPercent > 0.9f ? Color((BYTE)(230 * alpha), 255, 80, 80) :
        usedPercent > 0.7f ? Color((BYTE)(230 * alpha), 255, 200, 50) :
        Color((BYTE)(230 * alpha), 76, 194, 255);
    if (fillW > 0)
        FillRoundRectSolid(g, barX, barY, fillW, barH, 2, barColor);

    // Drive letter area
    FillRoundRectSolid(g, x + (int)(size * 0.3f), y + (int)(size * 0.2f), (int)(size * 0.4f), (int)(size * 0.3f), 3,
        Color((BYTE)(180 * alpha), 100, 100, 115));
}

void DrawPCIcon(Graphics& g, int x, int y, int size, float alpha = 1.0f) {
    // Monitor
    int mw = (int)(size * 0.8f);
    int mh = (int)(size * 0.55f);
    int mx = x + (size - mw) / 2;
    int my = y + 2;
    FillRoundRectSolid(g, mx, my, mw, mh, 3, Color((BYTE)(230 * alpha), 55, 120, 190));
    // Screen
    FillRoundRectSolid(g, mx + 3, my + 3, mw - 6, mh - 6, 2, Color((BYTE)(230 * alpha), 90, 160, 230));
    // Stand
    SolidBrush standBr(Color((BYTE)(200 * alpha), 110, 110, 120));
    g.FillRectangle(&standBr, x + size / 2 - 3, my + mh, 6, (int)(size * 0.12f));
    g.FillRectangle(&standBr, x + size / 2 - 8, my + mh + (int)(size * 0.1f), 16, 3);
}

void DrawRecycleBinIcon(Graphics& g, int x, int y, int size, float alpha = 1.0f) {
    int bw = (int)(size * 0.5f);
    int bh = (int)(size * 0.6f);
    int bx = x + (size - bw) / 2;
    int by = y + (int)(size * 0.2f);

    FillRoundRectSolid(g, bx, by, bw, bh, 3, Color((BYTE)(220 * alpha), 130, 130, 140));
    // Lid
    Pen lidPen(Color((BYTE)(230 * alpha), 150, 150, 160), 2.5f);
    g.DrawLine(&lidPen, bx - 2, by, bx + bw + 2, by);
    // Handle
    g.DrawLine(&lidPen, x + size / 2 - 3, by - 5, x + size / 2 + 3, by - 5);
    g.DrawLine(&lidPen, x + size / 2 - 3, by - 5, x + size / 2 - 3, by);
    g.DrawLine(&lidPen, x + size / 2 + 3, by - 5, x + size / 2 + 3, by);
    // Lines
    Pen lp(Color((BYTE)(120 * alpha), 90, 90, 100), 1.0f);
    for (int li = 0; li < 3; li++) {
        int lx = bx + bw / 4 + li * bw / 4;
        g.DrawLine(&lp, lx, by + 6, lx, by + bh - 4);
    }
}

// ============================================================================
//  DRAW DESKTOP ICON
// ============================================================================
void DrawDesktopIconItem(Graphics& g, const DesktopIcon& d, bool hovered, bool dragging_this) {
    int x = d.pixelX, y = d.pixelY;
    if (dragging_this) {
        x = g_dragCurrentX - g_dragOffsetX;
        y = g_dragCurrentY - g_dragOffsetY;
    }

    if (d.selected && hovered) {
        FillRoundRectSolid(g, x, y, DESKTOP_ICON_W, DESKTOP_ICON_H, 4,
            Color(90, 0, 103, 192));
        DrawRoundRect(g, x, y, DESKTOP_ICON_W, DESKTOP_ICON_H, 4,
            Color(120, 0, 103, 192), 1.0f);
    }
    else if (d.selected) {
        FillRoundRectSolid(g, x, y, DESKTOP_ICON_W, DESKTOP_ICON_H, 4, W11::IconSelectedBg);
        DrawRoundRect(g, x, y, DESKTOP_ICON_W, DESKTOP_ICON_H, 4,
            Color(80, 0, 103, 192), 1.0f);
    }
    else if (hovered) {
        FillRoundRectSolid(g, x, y, DESKTOP_ICON_W, DESKTOP_ICON_H, 4, W11::IconHoverBg);
    }

    int icoSize = 42;
    int icoX = x + (DESKTOP_ICON_W - icoSize) / 2;
    int icoY = y + 6;

    switch (d.iconType) {
    case 0: DrawPCIcon(g, icoX, icoY, icoSize); break;
    case 1: DrawRecycleBinIcon(g, icoX, icoY, icoSize); break;
    case 2: case 5: DrawFolderIcon(g, icoX, icoY, icoSize, d.iconColor); break;
    default: DrawFileIcon(g, icoX, icoY, icoSize, L".txt"); break;
    }

    FontFamily ff(L"Segoe UI");
    Font nameFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
    StringFormat cfmt;
    cfmt.SetAlignment(StringAlignmentCenter);
    cfmt.SetLineAlignment(StringAlignmentNear);
    cfmt.SetTrimming(StringTrimmingEllipsisCharacter);
    cfmt.SetFormatFlags(StringFormatFlagsLineLimit);

    SolidBrush shadowBr(Color(200, 0, 0, 0));
    RectF shadowRc((REAL)(x - 2), (REAL)(y + DESKTOP_ICON_H - 28 + 1), (REAL)(DESKTOP_ICON_W + 4), 28.0f);
    g.DrawString(d.name.c_str(), -1, &nameFont, shadowRc, &cfmt, &shadowBr);
    SolidBrush textBr(W11::TextPrimary);
    RectF textRc((REAL)(x - 2), (REAL)(y + DESKTOP_ICON_H - 28), (REAL)(DESKTOP_ICON_W + 4), 28.0f);
    g.DrawString(d.name.c_str(), -1, &nameFont, textRc, &cfmt, &textBr);
}

void DrawDesktopIcons(Graphics& g) {
    for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
        bool hov = (g_hoveredDesktopIcon == i);
        bool drag = (g_dragging && g_dragIconIdx == i && g_dragStarted);
        if (!drag) DrawDesktopIconItem(g, g_desktopIcons[i], hov, false);
    }
    if (g_dragging && g_dragIconIdx >= 0 && g_dragStarted)
        DrawDesktopIconItem(g, g_desktopIcons[g_dragIconIdx], true, true);
}

void DrawSelectionRect(Graphics& g) {
    if (!g_selecting) return;
    int x1 = (std::min)(g_selStartX, g_selEndX);
    int y1 = (std::min)(g_selStartY, g_selEndY);
    int w = (std::max)(g_selStartX, g_selEndX) - x1;
    int h = (std::max)(g_selStartY, g_selEndY) - y1;
    if (w < 2 && h < 2) return;
    SolidBrush fillBr(W11::SelectionRect);
    g.FillRectangle(&fillBr, x1, y1, w, h);
    Pen borderPen(W11::SelectionBorder, 1.0f);
    g.DrawRectangle(&borderPen, x1, y1, w, h);
}

// ============================================================================
//  DRAW EXPLORER WINDOW - Windows 11 File Explorer
// ============================================================================
void DrawExplorerWindow(Graphics& g, ExplorerWindow& win) {
    if (win.animAlpha <= 0.01f) return;
    float a = win.animAlpha;

    int wx = win.x, wy = win.y, ww = win.w, wh = win.h;
    bool isActive = (win.id == g_activeExplorerId);

    // Shadow (stronger for active)
    DrawShadow(g, wx, wy, ww, wh, 8, isActive ? 10 : 5);

    // Window background
    FillRoundRectSolid(g, wx, wy, ww, wh, 8, Color((BYTE)(252 * a), 30, 30, 30));
    DrawRoundRect(g, wx, wy, ww, wh, 8,
        isActive ? Color((BYTE)(60 * a), 255, 255, 255) : Color((BYTE)(30 * a), 255, 255, 255), 1.0f);

    FontFamily ff(L"Segoe UI");
    StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat cfmt; cfmt.SetAlignment(StringAlignmentCenter); cfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat rfmt; rfmt.SetAlignment(StringAlignmentFar); rfmt.SetLineAlignment(StringAlignmentCenter);

    // ---- Title bar (32px) ----
    int titleH = 32;

    // Window icon
    DrawFolderIcon(g, wx + 10, wy + 6, 20, Color(255, 255, 210, 76), a);

    // Title text
    Font titleFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    SolidBrush titleBr(Color((BYTE)(220 * a), 220, 220, 230));
    RectF titleRc((REAL)(wx + 36), (REAL)wy, 300.0f, (REAL)titleH);
    g.DrawString(win.title.c_str(), -1, &titleFont, titleRc, &lfmt, &titleBr);

    // Window buttons (minimize, maximize, close)
    int btnW = 46, btnH = titleH;
    int btnX = wx + ww - btnW * 3;

    // Minimize
    {
        bool hov = (win.hoveredTitleBtn == 1);
        if (hov) FillRoundRectSolid(g, btnX, wy, btnW, btnH, 0, Color((BYTE)(160 * a), 60, 60, 65));
        Pen p(Color((BYTE)(200 * a), 200, 200, 210), 1.2f);
        g.DrawLine(&p, btnX + btnW / 2 - 6, wy + btnH / 2, btnX + btnW / 2 + 6, wy + btnH / 2);
    }
    // Maximize
    btnX += btnW;
    {
        bool hov = (win.hoveredTitleBtn == 2);
        if (hov) FillRoundRectSolid(g, btnX, wy, btnW, btnH, 0, Color((BYTE)(160 * a), 60, 60, 65));
        Pen p(Color((BYTE)(200 * a), 200, 200, 210), 1.2f);
        g.DrawRectangle(&p, btnX + btnW / 2 - 5, wy + btnH / 2 - 5, 10, 10);
    }
    // Close
    btnX += btnW;
    {
        bool hov = (win.hoveredTitleBtn == 3);
        if (hov) FillRoundRectSolid(g, btnX, wy, btnW, btnH, 0, Color((BYTE)(200 * a), 196, 43, 28));
        Pen p(Color((BYTE)(200 * a), 200, 200, 210), 1.5f);
        g.DrawLine(&p, btnX + btnW / 2 - 5, wy + btnH / 2 - 5, btnX + btnW / 2 + 5, wy + btnH / 2 + 5);
        g.DrawLine(&p, btnX + btnW / 2 + 5, wy + btnH / 2 - 5, btnX + btnW / 2 - 5, wy + btnH / 2 + 5);
    }

    // ---- Toolbar (40px) ----
    int toolY = wy + titleH;
    int toolH = 40;
    SolidBrush toolBg(Color((BYTE)(250 * a), 36, 36, 36));
    g.FillRectangle(&toolBg, wx, toolY, ww, toolH);

    // Navigation buttons
    int navX = wx + 12;
    int navBtnSize = 28;

    // Back button
    {
        bool canBack = win.historyIndex > 0;
        Color c = canBack ? Color((BYTE)(200 * a), 200, 200, 210) : Color((BYTE)(60 * a), 100, 100, 110);
        Pen p(c, 1.5f);
        int cx2 = navX + navBtnSize / 2, cy2 = toolY + toolH / 2;
        g.DrawLine(&p, cx2 + 4, cy2 - 5, cx2 - 2, cy2);
        g.DrawLine(&p, cx2 - 2, cy2, cx2 + 4, cy2 + 5);
    }
    navX += navBtnSize + 4;

    // Forward button  
    {
        bool canFwd = win.historyIndex < (int)win.pathHistory.size() - 1;
        Color c = canFwd ? Color((BYTE)(200 * a), 200, 200, 210) : Color((BYTE)(60 * a), 100, 100, 110);
        Pen p(c, 1.5f);
        int cx2 = navX + navBtnSize / 2, cy2 = toolY + toolH / 2;
        g.DrawLine(&p, cx2 - 4, cy2 - 5, cx2 + 2, cy2);
        g.DrawLine(&p, cx2 + 2, cy2, cx2 - 4, cy2 + 5);
    }
    navX += navBtnSize + 4;

    // Up button
    {
        bool canUp = !win.currentPath.empty();
        Color c = canUp ? Color((BYTE)(200 * a), 200, 200, 210) : Color((BYTE)(60 * a), 100, 100, 110);
        Pen p(c, 1.5f);
        int cx2 = navX + navBtnSize / 2, cy2 = toolY + toolH / 2;
        g.DrawLine(&p, cx2 - 5, cy2 + 2, cx2, cy2 - 4);
        g.DrawLine(&p, cx2, cy2 - 4, cx2 + 5, cy2 + 2);
    }
    navX += navBtnSize + 12;

    // Address bar
    int addrX = navX;
    int addrW = ww - (navX - wx) - 16;
    int addrH = 28;
    int addrY = toolY + (toolH - addrH) / 2;

    FillRoundRectSolid(g, addrX, addrY, addrW, addrH, 4, Color((BYTE)(250 * a), 50, 50, 55));
    DrawRoundRect(g, addrX, addrY, addrW, addrH, 4, Color((BYTE)(40 * a), 255, 255, 255), 1.0f);

    Font addrFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    SolidBrush addrBr(Color((BYTE)(200 * a), 200, 200, 210));
    std::wstring addrText = win.currentPath.empty() ? L"This PC" : win.currentPath;
    RectF addrRc((REAL)(addrX + 10), (REAL)addrY, (REAL)(addrW - 20), (REAL)addrH);
    cfmt.SetAlignment(StringAlignmentNear);
    g.DrawString(addrText.c_str(), -1, &addrFont, addrRc, &cfmt, &addrBr);
    cfmt.SetAlignment(StringAlignmentCenter);

    // ---- Sidebar + Content area ----
    int sidebarW = 200;
    int contentX = wx + sidebarW;
    int contentY = toolY + toolH;
    int contentW = ww - sidebarW;
    int contentH = wh - titleH - toolH;

    // Sidebar background
    SolidBrush sidebarBg(Color((BYTE)(250 * a), 36, 36, 36));
    g.FillRectangle(&sidebarBg, wx, contentY, sidebarW, contentH);

    // Sidebar separator
    Pen sepPen(Color((BYTE)(25 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&sepPen, contentX, contentY, contentX, wy + wh);
    g.DrawLine(&sepPen, wx, contentY, wx + ww, contentY);

    // Sidebar items
    struct SideItem { std::wstring label; std::wstring path; int icon; };
    SideItem sideItems[] = {
        { L"Desktop", L"__desktop__", 0 },
        { L"Downloads", L"__downloads__", 1 },
        { L"Documents", L"__documents__", 2 },
        { L"Pictures", L"__pictures__", 3 },
        { L"Music", L"__music__", 4 },
        { L"Videos", L"__videos__", 5 },
        { L"This PC", L"", 6 },
    };

    Font sideFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    int sideY = contentY + 8;

    // Quick access header
    Font headerFont(&ff, 11.0f, FontStyleBold, UnitPixel);
    SolidBrush headerBr(Color((BYTE)(120 * a), 160, 160, 170));
    RectF headerRc((REAL)(wx + 14), (REAL)sideY, (REAL)(sidebarW - 20), 20.0f);
    g.DrawString(L"Quick access", -1, &headerFont, headerRc, &lfmt, &headerBr);
    sideY += 28;

    for (int i = 0; i < 7; i++) {
        bool hov = (win.hoveredSidebarItem == i);
        int itemH = 30;

        if (hov) {
            FillRoundRectSolid(g, wx + 4, sideY, sidebarW - 8, itemH, 4,
                Color((BYTE)(200 * a), 50, 50, 55));
        }

        // Small icon
        if (i < 6) {
            Color folderColor(255, 255, 210, 76);
            if (i == 1) folderColor = Color(255, 76, 194, 255);
            else if (i == 3) folderColor = Color(255, 160, 120, 255);
            else if (i == 4) folderColor = Color(255, 255, 100, 130);
            else if (i == 5) folderColor = Color(255, 100, 200, 130);
            DrawFolderIcon(g, wx + 14, sideY + 3, 22, folderColor, a);
        }
        else {
            DrawPCIcon(g, wx + 14, sideY + 3, 22, a);
        }

        SolidBrush sideBr(Color((BYTE)(220 * a), 220, 220, 230));
        RectF sideRc((REAL)(wx + 42), (REAL)sideY, (REAL)(sidebarW - 50), (REAL)itemH);
        g.DrawString(sideItems[i].label.c_str(), -1, &sideFont, sideRc, &lfmt, &sideBr);

        if (i == 5) {
            // Separator before This PC
            sideY += itemH + 4;
            Pen sp(Color((BYTE)(20 * a), 255, 255, 255), 1.0f);
            g.DrawLine(&sp, wx + 12, sideY, wx + sidebarW - 12, sideY);
            sideY += 8;

            SolidBrush hdr2(Color((BYTE)(120 * a), 160, 160, 170));
            RectF hdr2Rc((REAL)(wx + 14), (REAL)sideY, (REAL)(sidebarW - 20), 20.0f);
            g.DrawString(L"This PC", -1, &headerFont, hdr2Rc, &lfmt, &hdr2);
            sideY += 26;
        }
        else {
            sideY += itemH;
        }
    }

    // ---- Content area ----
    // Clip to content area
    g.SetClip(Rect(contentX + 1, contentY + 1, contentW - 2, contentH - 2));

    // Content background
    SolidBrush contentBg(Color((BYTE)(250 * a), 30, 30, 30));
    g.FillRectangle(&contentBg, contentX, contentY, contentW, contentH);

    int scrollOff = (int)win.scrollAnim;

    if (win.currentPath.empty()) {
        // ====== THIS PC VIEW ======
        Font secFont(&ff, 13.0f, FontStyleBold, UnitPixel);
        SolidBrush secBr(Color((BYTE)(180 * a), 200, 200, 210));

        int cy = contentY + 16 - scrollOff;
        RectF secRc((REAL)(contentX + 20), (REAL)cy, 200.0f, 24.0f);
        g.DrawString(L"Devices and drives", -1, &secFont, secRc, &lfmt, &secBr);
        cy += 36;

        // Drive tiles
        int tileW = 240;
        int tileH = 70;
        int cols = (std::max)(1, (contentW - 40) / (tileW + 10));

        for (int i = 0; i < (int)win.items.size(); i++) {
            const FileItem& fi = win.items[i];
            int col = i % cols;
            int row = i / cols;
            int tx = contentX + 20 + col * (tileW + 10);
            int ty = cy + row * (tileH + 10);

            bool hov = (win.hoveredItem == i);
            bool sel = (win.selectedItem == i);

            Color bg = sel ? Color((BYTE)(230 * a), 0, 70, 140) :
                hov ? Color((BYTE)(220 * a), 50, 50, 55) :
                Color((BYTE)(200 * a), 42, 42, 46);

            FillRoundRectSolid(g, tx, ty, tileW, tileH, 6, bg);
            DrawRoundRect(g, tx, ty, tileW, tileH, 6, Color((BYTE)(20 * a), 255, 255, 255), 1.0f);

            // Drive icon
            float usedPct = fi.totalSpace > 0 ? (float)(fi.totalSpace - fi.freeSpace) / (float)fi.totalSpace : 0;
            DrawDriveIcon(g, tx + 10, ty + 10, 44, usedPct, a);

            // Drive name
            Font driveFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
            SolidBrush driveBr(Color((BYTE)(240 * a), 240, 240, 250));
            RectF driveRc((REAL)(tx + 60), (REAL)(ty + 8), (REAL)(tileW - 70), 20.0f);
            cfmt.SetAlignment(StringAlignmentNear);
            g.DrawString(fi.name.c_str(), -1, &driveFont, driveRc, &cfmt, &driveBr);

            // Space info
            if (fi.totalSpace > 0) {
                std::wstring spaceStr = FormatSize(fi.freeSpace) + L" free of " + FormatSize(fi.totalSpace);
                Font spaceFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
                SolidBrush spaceBr(Color((BYTE)(140 * a), 160, 160, 170));
                RectF spaceRc((REAL)(tx + 60), (REAL)(ty + 28), (REAL)(tileW - 70), 16.0f);
                g.DrawString(spaceStr.c_str(), -1, &spaceFont, spaceRc, &cfmt, &spaceBr);

                // Usage bar
                int barX2 = tx + 60;
                int barY2 = ty + 48;
                int barW2 = tileW - 76;
                int barH2 = 6;
                FillRoundRectSolid(g, barX2, barY2, barW2, barH2, 3, Color((BYTE)(150 * a), 55, 55, 60));
                int fillW2 = (int)(barW2 * usedPct);
                Color barCol2 = usedPct > 0.9f ? Color((BYTE)(230 * a), 255, 80, 80) :
                    usedPct > 0.7f ? Color((BYTE)(230 * a), 255, 200, 50) :
                    Color((BYTE)(230 * a), 76, 194, 255);
                if (fillW2 > 0)
                    FillRoundRectSolid(g, barX2, barY2, fillW2, barH2, 3, barCol2);
            }

            cfmt.SetAlignment(StringAlignmentCenter);
        }

        // Item count
        int totalItems = (int)win.items.size();
        Font countFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
        SolidBrush countBr(Color((BYTE)(100 * a), 150, 150, 160));
        wchar_t countStr[64];
        swprintf_s(countStr, L"%d items", totalItems);
        RectF countRc((REAL)(contentX + 20), (REAL)(wy + wh - 28), (REAL)(contentW - 40), 20.0f);
        lfmt.SetLineAlignment(StringAlignmentCenter);
        g.DrawString(countStr, -1, &countFont, countRc, &lfmt, &countBr);

    }
    else {
        // ====== DIRECTORY VIEW ======
        int itemH = 32;

        // Column headers
        int headerY = contentY + 4;
        Font hdrFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
        SolidBrush hdrBr(Color((BYTE)(140 * a), 170, 170, 180));

        RectF nameHdr((REAL)(contentX + 48), (REAL)headerY, 300.0f, 24.0f);
        g.DrawString(L"Name", -1, &hdrFont, nameHdr, &lfmt, &hdrBr);
        RectF sizeHdr((REAL)(contentX + contentW - 250), (REAL)headerY, 100.0f, 24.0f);
        g.DrawString(L"Size", -1, &hdrFont, sizeHdr, &lfmt, &hdrBr);
        RectF typeHdr((REAL)(contentX + contentW - 140), (REAL)headerY, 120.0f, 24.0f);
        g.DrawString(L"Type", -1, &hdrFont, typeHdr, &lfmt, &hdrBr);

        Pen hdrLine(Color((BYTE)(20 * a), 255, 255, 255), 1.0f);
        g.DrawLine(&hdrLine, contentX + 12, headerY + 26, contentX + contentW - 12, headerY + 26);

        int listY = headerY + 30 - scrollOff;
        Font itemFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
        Font smallFont(&ff, 10.0f, FontStyleRegular, UnitPixel);

        for (int i = 0; i < (int)win.items.size(); i++) {
            int iy = listY + i * itemH;
            if (iy + itemH < contentY || iy > wy + wh) continue;

            const FileItem& fi = win.items[i];
            bool hov = (win.hoveredItem == i);
            bool sel = (win.selectedItem == i);

            if (sel) {
                FillRoundRectSolid(g, contentX + 4, iy, contentW - 8, itemH - 1, 3,
                    Color((BYTE)(220 * a), 0, 70, 140));
            }
            else if (hov) {
                FillRoundRectSolid(g, contentX + 4, iy, contentW - 8, itemH - 1, 3,
                    Color((BYTE)(200 * a), 50, 50, 55));
            }

            // Icon (small)
            if (fi.isDirectory) {
                if (fi.name == L"..") {
                    // Up folder arrow
                    Pen upPen(Color((BYTE)(200 * a), 200, 200, 210), 1.5f);
                    int cx2 = contentX + 28, cy2 = iy + itemH / 2;
                    g.DrawLine(&upPen, cx2, cy2 + 4, cx2, cy2 - 4);
                    g.DrawLine(&upPen, cx2 - 4, cy2, cx2, cy2 - 4);
                    g.DrawLine(&upPen, cx2 + 4, cy2, cx2, cy2 - 4);
                }
                else {
                    DrawFolderIcon(g, contentX + 16, iy + 3, 24, Color(255, 255, 210, 76), a);
                }
            }
            else {
                // Get extension
                std::wstring ext;
                size_t dotPos = fi.name.find_last_of(L'.');
                if (dotPos != std::wstring::npos) ext = fi.name.substr(dotPos);
                for (auto& ch : ext) ch = towlower(ch);
                DrawFileIcon(g, contentX + 16, iy + 3, 24, ext, a);
            }

            // Name
            SolidBrush nameBr(Color((BYTE)(240 * a), 240, 240, 250));
            RectF nameRc((REAL)(contentX + 48), (REAL)iy, (REAL)(contentW - 310), (REAL)itemH);
            g.DrawString(fi.name.c_str(), -1, &itemFont, nameRc, &lfmt, &nameBr);

            // Size
            if (!fi.isDirectory && fi.fileSize > 0) {
                SolidBrush sizeBr(Color((BYTE)(160 * a), 170, 170, 180));
                RectF sizeRc((REAL)(contentX + contentW - 250), (REAL)iy, 100.0f, (REAL)itemH);
                g.DrawString(FormatSize(fi.fileSize).c_str(), -1, &smallFont, sizeRc, &lfmt, &sizeBr);
            }

            // Type
            std::wstring typeStr;
            if (fi.isDirectory) typeStr = L"File folder";
            else {
                size_t dotPos = fi.name.find_last_of(L'.');
                if (dotPos != std::wstring::npos) {
                    std::wstring ext = fi.name.substr(dotPos);
                    for (auto& ch : ext) ch = towupper(ch);
                    typeStr = ext.substr(1) + L" File";
                }
                else {
                    typeStr = L"File";
                }
            }
            SolidBrush typeBr(Color((BYTE)(130 * a), 160, 160, 170));
            RectF typeRc((REAL)(contentX + contentW - 140), (REAL)iy, 120.0f, (REAL)itemH);
            g.DrawString(typeStr.c_str(), -1, &smallFont, typeRc, &lfmt, &typeBr);
        }

        // Status bar
        int totalItems = (int)win.items.size();
        int dirCount = 0, fileCount = 0;
        for (auto& fi : win.items) {
            if (fi.name == L"..") continue;
            if (fi.isDirectory) dirCount++; else fileCount++;
        }
        Font countFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
        SolidBrush countBr(Color((BYTE)(100 * a), 150, 150, 160));
        wchar_t countStr[128];
        swprintf_s(countStr, L"%d folders, %d files", dirCount, fileCount);
        RectF countRc((REAL)(contentX + 12), (REAL)(wy + wh - 28), (REAL)(contentW - 24), 20.0f);
        lfmt.SetLineAlignment(StringAlignmentCenter);
        g.DrawString(countStr, -1, &countFont, countRc, &lfmt, &countBr);

        // Scrollbar
        int totalHeight = (int)win.items.size() * itemH + 40;
        if (totalHeight > contentH) {
            int sbX = contentX + contentW - 8;
            int sbY = contentY + 4;
            int sbH = contentH - 8;
            float ratio = (float)contentH / (float)totalHeight;
            int thumbH = (std::max)(30, (int)(sbH * ratio));
            float scrollRatio = totalHeight > contentH ? (float)scrollOff / (float)(totalHeight - contentH) : 0;
            int thumbY = sbY + (int)((sbH - thumbH) * scrollRatio);

            FillRoundRectSolid(g, sbX, sbY, 6, sbH, 3, Color((BYTE)(40 * a), 255, 255, 255));
            FillRoundRectSolid(g, sbX, thumbY, 6, thumbH, 3, Color((BYTE)(100 * a), 150, 150, 160));
        }
    }

    g.ResetClip();
}

// ============================================================================
//  TASKBAR - Windows 11 Style (Enhanced)
// ============================================================================
void DrawTaskbar(Graphics& g, int sw, int sh) {
    int barY = sh - TASKBAR_HEIGHT;

    // Background with subtle gradient
    LinearGradientBrush barBg(Point(0, barY), Point(0, sh),
        Color(248, 28, 28, 28), Color(252, 24, 24, 24));
    g.FillRectangle(&barBg, 0, barY, sw, TASKBAR_HEIGHT);

    // Top border - subtle acrylic edge
    Pen topLine(Color(45, 255, 255, 255), 1.0f);
    g.DrawLine(&topLine, 0, barY, sw, barY);

    FontFamily ff(L"Segoe UI");

    int iconCount = (int)g_taskApps.size();
    int totalW = (iconCount + 3) * (TASKBAR_ICON_SIZE + 4);
    int startX = (sw - totalW) / 2;
    int iconY = barY + (TASKBAR_HEIGHT - TASKBAR_ICON_SIZE) / 2;

    // Start button
    {
        int bx = startX;
        int by = iconY;
        bool startHov = (g_hoveredTaskbarIcon == -10);

        if (g_startMenuOpen) {
            FillRoundRectSolid(g, bx, by, TASKBAR_ICON_SIZE, TASKBAR_ICON_SIZE, 6, W11::TaskbarActive);
        }
        else if (startHov) {
            FillRoundRectSolid(g, bx, by, TASKBAR_ICON_SIZE, TASKBAR_ICON_SIZE, 6, W11::TaskbarHover);
        }

        // Windows 11 logo
        int logoSize = 14;
        int lx = bx + (TASKBAR_ICON_SIZE - logoSize) / 2;
        int ly = by + (TASKBAR_ICON_SIZE - logoSize) / 2;
        int gap = 2, sq = (logoSize - gap) / 2;
        SolidBrush logoBr(g_startMenuOpen ? Color(255, 255, 255, 255) : W11::AccentLight);
        g.FillRectangle(&logoBr, lx, ly, sq, sq);
        g.FillRectangle(&logoBr, lx + sq + gap, ly, sq, sq);
        g.FillRectangle(&logoBr, lx, ly + sq + gap, sq, sq);
        g.FillRectangle(&logoBr, lx + sq + gap, ly + sq + gap, sq, sq);
    }

    // Search
    {
        int bx = startX + (TASKBAR_ICON_SIZE + 4);
        int by = iconY;
        bool hov = (g_hoveredTaskbarIcon == -11);
        if (hov) FillRoundRectSolid(g, bx, by, TASKBAR_ICON_SIZE, TASKBAR_ICON_SIZE, 6, W11::TaskbarHover);
        Pen sp(W11::TextSecondary, 1.8f);
        int cx2 = bx + TASKBAR_ICON_SIZE / 2 - 2, cy2 = by + TASKBAR_ICON_SIZE / 2 - 2;
        g.DrawEllipse(&sp, cx2 - 6, cy2 - 6, 12, 12);
        g.DrawLine(&sp, cx2 + 4, cy2 + 4, cx2 + 7, cy2 + 7);
    }

    // Task View
    {
        int bx = startX + 2 * (TASKBAR_ICON_SIZE + 4);
        int by = iconY;
        bool hov = (g_hoveredTaskbarIcon == -12);
        if (hov) FillRoundRectSolid(g, bx, by, TASKBAR_ICON_SIZE, TASKBAR_ICON_SIZE, 6, W11::TaskbarHover);
        Pen tp(W11::TextSecondary, 1.3f);
        g.DrawRectangle(&tp, bx + 10, by + 11, 10, 8);
        g.DrawRectangle(&tp, bx + 19, by + 18, 10, 8);
    }

    // App icons
    int appsStartX = startX + 3 * (TASKBAR_ICON_SIZE + 4) + 8;
    for (int i = 0; i < iconCount; i++) {
        int ix = appsStartX + i * (TASKBAR_ICON_SIZE + 4);
        bool hov = (g_hoveredTaskbarIcon == i);

        // Animate hover
        float targetHov = hov ? 1.0f : 0.0f;
        g_taskApps[i].hoverAnim = SmoothLerp(g_taskApps[i].hoverAnim, targetHov, 0.3f);
        float ha = g_taskApps[i].hoverAnim;

        if (ha > 0.01f || g_taskApps[i].active) {
            BYTE bgAlpha = (BYTE)(255 * (std::max)(ha * 0.4f, g_taskApps[i].active ? 0.35f : 0.0f));
            FillRoundRectSolid(g, ix, iconY, TASKBAR_ICON_SIZE, TASKBAR_ICON_SIZE, 6,
                Color(bgAlpha, 70, 70, 75));
        }

        // Icon background
        int icoInner = 24;
        int icoX = ix + (TASKBAR_ICON_SIZE - icoInner) / 2;
        int icoY = iconY + (TASKBAR_ICON_SIZE - icoInner) / 2;
        FillRoundRectSolid(g, icoX, icoY, icoInner, icoInner, 5, g_taskApps[i].accentColor);

        // Icon label
        Font icoFont(&ff, 10.0f, FontStyleBold, UnitPixel);
        SolidBrush icoTextBr(Color(255, 255, 255, 255));
        StringFormat cfmt; cfmt.SetAlignment(StringAlignmentCenter); cfmt.SetLineAlignment(StringAlignmentCenter);
        RectF icoRc((REAL)icoX, (REAL)icoY, (REAL)icoInner, (REAL)icoInner);
        g.DrawString(g_taskApps[i].iconLabel.c_str(), -1, &icoFont, icoRc, &cfmt, &icoTextBr);

        // Running indicator (pill shape)
        if (g_taskApps[i].running) {
            int dotW = g_taskApps[i].active ? 18 : 6;
            int dotX = ix + (TASKBAR_ICON_SIZE - dotW) / 2;
            int dotY = iconY + TASKBAR_ICON_SIZE - 1;
            FillRoundRectSolid(g, dotX, dotY, dotW, 3, 1, W11::TaskbarIndicator);
        }

        SetRect(&g_taskApps[i].bounds, ix, iconY, ix + TASKBAR_ICON_SIZE, iconY + TASKBAR_ICON_SIZE);
    }

    // ---- System Tray (right side) ----
    int trayX = sw - 220;

    // Hidden icons chevron
    {
        int cx2 = trayX + 6;
        int cy2 = barY + TASKBAR_HEIGHT / 2;
        Pen chev(Color(100, 200, 200, 210), 1.5f);
        g.DrawLine(&chev, cx2 - 3, cy2 - 3, cx2, cy2);
        g.DrawLine(&chev, cx2, cy2, cx2 + 3, cy2 - 3);
    }

    // WiFi icon (clickable area)
    {
        int wx2 = trayX + 30;
        int wy2 = barY + (TASKBAR_HEIGHT - 18) / 2;
        bool wifiHov = (g_hoveredTaskbarIcon == -20);
        if (wifiHov || g_wifiPanelOpen) {
            FillRoundRectSolid(g, wx2 - 4, wy2 - 4, 26, 26, 4,
                Color(80, 255, 255, 255));
        }

        // WiFi arcs
        int wcx = wx2 + 8, wcy = wy2 + 14;
        for (int arc = 0; arc < 3; arc++) {
            int r = 4 + arc * 4;
            Pen wPen(Color(220, 200, 200, 210), 1.5f);
            g.DrawArc(&wPen, wcx - r, wcy - r, r * 2, r * 2, 225, 90);
        }
        SolidBrush dotBr(Color(220, 200, 200, 210));
        g.FillEllipse(&dotBr, wcx - 2, wcy - 2, 4, 4);
    }

    // Volume icon
    {
        int vx = trayX + 66;
        int vy = barY + (TASKBAR_HEIGHT - 16) / 2;
        Pen vPen(Color(200, 200, 200, 210), 1.5f);
        g.DrawRectangle(&vPen, vx + 2, vy + 4, 4, 8);
        Point cone[4] = { Point(vx + 6,vy + 4), Point(vx + 12,vy + 1), Point(vx + 12,vy + 15), Point(vx + 6,vy + 12) };
        g.DrawPolygon(&vPen, cone, 4);
        // Sound waves
        g.DrawArc(&vPen, vx + 10, vy + 3, 8, 10, -50, 100);
    }

    // Battery
    {
        int bx = trayX + 96;
        int by = barY + (TASKBAR_HEIGHT - 12) / 2;
        Pen bPen(Color(200, 200, 200, 210), 1.3f);
        g.DrawRectangle(&bPen, bx, by, 22, 12);
        SolidBrush bFill(W11::Success);
        g.FillRectangle(&bFill, bx + 2, by + 2, 16, 8);
        SolidBrush bTip(Color(200, 200, 200, 210));
        g.FillRectangle(&bTip, bx + 22, by + 3, 3, 6);
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
    StringFormat rfmt; rfmt.SetAlignment(StringAlignmentFar); rfmt.SetLineAlignment(StringAlignmentCenter);

    RectF clockRc((REAL)(sw - 100), (REAL)barY, 88.0f, (REAL)(TASKBAR_HEIGHT / 2));
    g.DrawString(timeStr, -1, &clockFont, clockRc, &rfmt, &clockBr);
    RectF dateRc((REAL)(sw - 100), (REAL)(barY + TASKBAR_HEIGHT / 2 - 2), 88.0f, (REAL)(TASKBAR_HEIGHT / 2));
    g.DrawString(dateStr, -1, &dateFont, dateRc, &rfmt, &dateBr);

    // Notification indicator
    if (!g_notifs.empty()) {
        SolidBrush notifDot(W11::AccentLight);
        g.FillEllipse(&notifDot, sw - 16, barY + TASKBAR_HEIGHT / 2 - 3, 6, 6);
    }
}

// ============================================================================
//  START MENU
// ============================================================================
void DrawStartMenu(Graphics& g, int sw, int sh) {
    if (g_startMenuAnim <= 0.01f) return;
    float a = g_startMenuAnim;
    float ease = EaseOutCubic(a);

    SolidBrush overlay(Color((BYTE)(60 * a), 0, 0, 0));
    g.FillRectangle(&overlay, 0, 0, sw, sh);

    int menuW = START_MENU_W, menuH = START_MENU_H;
    int menuX = (sw - menuW) / 2;
    int targetY = sh - TASKBAR_HEIGHT - menuH - 12;
    int menuY = (int)(targetY + 30 * (1.0f - ease));

    DrawShadow(g, menuX, menuY, menuW, menuH, START_MENU_RADIUS, 10);
    FillRoundRectSolid(g, menuX, menuY, menuW, menuH, START_MENU_RADIUS,
        Color((BYTE)(248 * a), 38, 38, 38));
    DrawRoundRect(g, menuX, menuY, menuW, menuH, START_MENU_RADIUS,
        Color((BYTE)(45 * a), 255, 255, 255), 1.0f);

    FontFamily ff(L"Segoe UI");
    StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat cfmt; cfmt.SetAlignment(StringAlignmentCenter); cfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat rfmt; rfmt.SetAlignment(StringAlignmentFar); rfmt.SetLineAlignment(StringAlignmentCenter);

    // Search bar
    int searchX = menuX + 24, searchY = menuY + 24;
    int searchW = menuW - 48, searchH = 36;
    FillRoundRectSolid(g, searchX, searchY, searchW, searchH, 20,
        Color((BYTE)(255 * a), 55, 55, 55));
    DrawRoundRect(g, searchX, searchY, searchW, searchH, 20,
        Color((BYTE)(30 * a), 255, 255, 255), 1.0f);

    Pen sPen(Color((BYTE)(180 * a), 180, 180, 180), 1.5f);
    g.DrawEllipse(&sPen, searchX + 14, searchY + 9, 12, 12);
    g.DrawLine(&sPen, searchX + 24, searchY + 21, searchX + 28, searchY + 25);

    Font searchFont(&ff, 13.0f, FontStyleRegular, UnitPixel);
    SolidBrush searchBr(Color((BYTE)(120 * a), 180, 180, 180));
    RectF searchRc((REAL)(searchX + 36), (REAL)searchY, (REAL)(searchW - 48), (REAL)searchH);
    g.DrawString(L"Type here to search", -1, &searchFont, searchRc, &lfmt, &searchBr);

    // Pinned
    Font sectionFont(&ff, 14.0f, FontStyleBold, UnitPixel);
    SolidBrush sectionBr(Color((BYTE)(255 * a), 255, 255, 255));
    RectF pinnedTitleRc((REAL)(menuX + 30), (REAL)(menuY + 76), 100.0f, 20.0f);
    g.DrawString(L"Pinned", -1, &sectionFont, pinnedTitleRc, &lfmt, &sectionBr);

    Font smallFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    SolidBrush allBr(Color((BYTE)(200 * a), 180, 180, 180));
    RectF allRc((REAL)(menuX + menuW - 130), (REAL)(menuY + 76), 100.0f, 20.0f);
    g.DrawString(L"All apps >", -1, &smallFont, allRc, &rfmt, &allBr);

    int gridX = menuX + 28, gridY = menuY + 108;
    int cols = 6;
    int cellW = (menuW - 56) / cols, cellH = 72;

    for (int i = 0; i < (int)g_startApps.size() && i < 18; i++) {
        int row = i / cols, col = i % cols;
        int ix = gridX + col * cellW;
        int iy = gridY + row * cellH;

        // Staggered fade-in
        float itemA = a * (std::min)(1.0f, (std::max)(0.0f, (a * 18.0f - (float)i) / 6.0f));

        bool hov = (g_hoveredStartItem == i);
        if (hov) {
            FillRoundRectSolid(g, ix + 2, iy + 2, cellW - 4, cellH - 4, 6,
                Color((BYTE)(255 * itemA), 60, 60, 60));
        }

        int aicoSize = 34;
        int aicoX = ix + (cellW - aicoSize) / 2;
        int aicoY = iy + 6;
        FillRoundRectSolid(g, aicoX, aicoY, aicoSize, aicoSize, 7,
            Color((BYTE)(240 * itemA), g_startApps[i].color.GetR(),
                g_startApps[i].color.GetG(), g_startApps[i].color.GetB()));

        Font aicoFont(&ff, 12.0f, FontStyleBold, UnitPixel);
        SolidBrush aicoTextBr(Color((BYTE)(255 * itemA), 255, 255, 255));
        RectF aicoRc((REAL)aicoX, (REAL)aicoY, (REAL)aicoSize, (REAL)aicoSize);
        g.DrawString(g_startApps[i].iconLabel.c_str(), -1, &aicoFont, aicoRc, &cfmt, &aicoTextBr);

        Font nameFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
        SolidBrush nameBr(Color((BYTE)(220 * itemA), 220, 220, 220));
        cfmt.SetTrimming(StringTrimmingEllipsisCharacter);
        RectF nameRc((REAL)ix, (REAL)(iy + 44), (REAL)cellW, 18.0f);
        g.DrawString(g_startApps[i].name.c_str(), -1, &nameFont, nameRc, &cfmt, &nameBr);
    }

    // Separator
    int sepY = menuY + 108 + 3 * cellH + 12;
    Pen sepLine(Color((BYTE)(30 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&sepLine, menuX + 28, sepY, menuX + menuW - 28, sepY);

    // Recommended
    RectF recTitleRc((REAL)(menuX + 30), (REAL)(sepY + 12), 140.0f, 20.0f);
    g.DrawString(L"Recommended", -1, &sectionFont, recTitleRc, &lfmt, &sectionBr);

    const wchar_t* recNames[] = { L"Recently opened file.txt", L"Project_v2.docx", L"Screenshot_2026.png" };
    const wchar_t* recTimes[] = { L"Just now", L"Yesterday", L"2 days ago" };
    for (int i = 0; i < 3; i++) {
        int ry = sepY + 44 + i * 40;
        int rx = menuX + 30;
        FillRoundRectSolid(g, rx, ry + 4, 28, 28, 4, Color((BYTE)(180 * a), 60, 60, 70));
        Font fIcon(&ff, 10.0f, FontStyleRegular, UnitPixel);
        SolidBrush fIcoBr(Color((BYTE)(200 * a), 180, 180, 200));
        RectF fIcoRc((REAL)rx, (REAL)(ry + 4), 28.0f, 28.0f);
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

    // Bottom bar
    int bottomY = menuY + menuH - 52;
    Pen bottomSep(Color((BYTE)(25 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&bottomSep, menuX + 24, bottomY, menuX + menuW - 24, bottomY);

    FillRoundRectSolid(g, menuX + 30, bottomY + 12, 28, 28, 14,
        Color((BYTE)(200 * a), 80, 80, 90));
    Font userIcon(&ff, 14.0f, FontStyleBold, UnitPixel);
    SolidBrush userBr(Color((BYTE)(240 * a), 200, 200, 210));
    RectF userRc((REAL)(menuX + 30), (REAL)(bottomY + 12), 28.0f, 28.0f);
    g.DrawString(L"U", -1, &userIcon, userRc, &cfmt, &userBr);

    Font usernameFont(&ff, 13.0f, FontStyleRegular, UnitPixel);
    SolidBrush usernameBr(Color((BYTE)(220 * a), 230, 230, 240));
    lfmt.SetLineAlignment(StringAlignmentCenter);
    RectF usernameRc((REAL)(menuX + 66), (REAL)(bottomY + 8), 200.0f, 36.0f);
    g.DrawString(L"User", -1, &usernameFont, usernameRc, &lfmt, &usernameBr);

    int pwrX = menuX + menuW - 52, pwrY = bottomY + 12;
    FillRoundRectSolid(g, pwrX, pwrY, 28, 28, 4, Color((BYTE)(60 * a), 255, 255, 255));
    Pen pwrPen(Color((BYTE)(200 * a), 200, 200, 210), 2.0f);
    int pcx = pwrX + 14, pcy = pwrY + 14;
    g.DrawArc(&pwrPen, pcx - 6, pcy - 6, 12, 12, -60, 300);
    g.DrawLine(&pwrPen, pcx, pcy - 8, pcx, pcy - 3);
}

// ============================================================================
//  CONTEXT MENU
// ============================================================================
void DrawContextMenu(Graphics& g) {
    if (!g_contextMenuOpen || g_contextMenuAnim <= 0.01f) return;
    float a = g_contextMenuAnim;
    float ease = EaseOutCubic(a);

    int menuW = CTX_MENU_W;
    int menuH = 8;
    for (const auto& item : g_contextItems)
        menuH += (item.id == 0) ? CTX_SEP_H : CTX_ITEM_H;
    menuH += 8;

    int mx = g_contextMenuX, my = g_contextMenuY;
    if (mx + menuW > ScreenW()) mx = ScreenW() - menuW - 8;
    if (my + menuH > ScreenH()) my = ScreenH() - menuH - 8;

    // Scale animation
    int animMenuH = (int)(menuH * ease);

    DrawShadow(g, mx, my, menuW, animMenuH, CTX_RADIUS, 6);
    FillRoundRectSolid(g, mx, my, menuW, animMenuH, CTX_RADIUS,
        Color((BYTE)(252 * a), 44, 44, 44));
    DrawRoundRect(g, mx, my, menuW, animMenuH, CTX_RADIUS,
        Color((BYTE)(40 * a), 255, 255, 255), 1.0f);

    if (ease < 0.5f) return; // Don't draw items until half expanded

    FontFamily ff(L"Segoe UI");
    Font font(&ff, 12.0f, FontStyleRegular, UnitPixel);
    Font shortcutFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
    StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat rfmt2; rfmt2.SetAlignment(StringAlignmentFar); rfmt2.SetLineAlignment(StringAlignmentCenter);

    g.SetClip(Rect(mx, my, menuW, animMenuH));

    int cy = my + 8;
    for (int i = 0; i < (int)g_contextItems.size(); i++) {
        const auto& item = g_contextItems[i];
        if (item.id == 0) {
            Pen sp(Color((BYTE)(30 * a), 255, 255, 255), 1.0f);
            g.DrawLine(&sp, mx + 12, cy + CTX_SEP_H / 2, mx + menuW - 12, cy + CTX_SEP_H / 2);
            cy += CTX_SEP_H;
            continue;
        }
        bool hov = (g_hoveredContextItem == i);
        if (hov) {
            FillRoundRectSolid(g, mx + 4, cy + 1, menuW - 8, CTX_ITEM_H - 2, 4,
                Color((BYTE)(255 * a), 55, 55, 60));
        }
        SolidBrush textBr(Color((BYTE)(230 * a), 230, 230, 240));
        RectF textRc((REAL)(mx + 16), (REAL)cy, (REAL)(menuW - 32), (REAL)CTX_ITEM_H);
        g.DrawString(item.label.c_str(), -1, &font, textRc, &lfmt, &textBr);
        if (!item.shortcut.empty()) {
            SolidBrush scBr(Color((BYTE)(100 * a), 140, 140, 150));
            g.DrawString(item.shortcut.c_str(), -1, &shortcutFont, textRc, &rfmt2, &scBr);
        }
        cy += CTX_ITEM_H;
    }
    g.ResetClip();
}

// ============================================================================
//  WIFI PANEL
// ============================================================================
void DrawWifiPanel(Graphics& g, int sw, int sh) {
    if (g_wifiPanelAnim <= 0.01f) return;
    float a = g_wifiPanelAnim;
    float ease = EaseOutCubic(a);

    int panelW = WIFI_PANEL_W, panelH = WIFI_PANEL_H;
    int panelX = sw - panelW - 12;
    int panelY = (int)(sh - TASKBAR_HEIGHT - panelH - 12 + 20 * (1.0f - ease));

    DrawShadow(g, panelX, panelY, panelW, panelH, 8, 8);
    FillRoundRectSolid(g, panelX, panelY, panelW, panelH, 8,
        Color((BYTE)(250 * a), 38, 38, 38));
    DrawRoundRect(g, panelX, panelY, panelW, panelH, 8,
        Color((BYTE)(40 * a), 255, 255, 255), 1.0f);

    FontFamily ff(L"Segoe UI");
    StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat cfmt; cfmt.SetAlignment(StringAlignmentCenter); cfmt.SetLineAlignment(StringAlignmentCenter);

    // Quick toggles
    int toggleY = panelY + 16;
    int toggleH = 70;
    int toggleW = (panelW - 48) / 3;

    struct Toggle { std::wstring label; bool active; Color color; };
    Toggle toggles[] = {
        { L"WiFi", true, W11::AccentLight },
        { L"Bluetooth", true, Color(255, 0, 120, 212) },
        { L"Airplane", false, Color(255, 80, 80, 85) }
    };

    for (int i = 0; i < 3; i++) {
        int tx = panelX + 16 + i * (toggleW + 8);
        Color bgCol = toggles[i].active ?
            Color((BYTE)(255 * a), toggles[i].color.GetR(), toggles[i].color.GetG(), toggles[i].color.GetB()) :
            Color((BYTE)(200 * a), 55, 55, 60);
        FillRoundRectSolid(g, tx, toggleY, toggleW, toggleH, 8, bgCol);

        Font tFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
        Color tColor = toggles[i].active ? Color((BYTE)(255 * a), 255, 255, 255) :
            Color((BYTE)(200 * a), 180, 180, 190);
        SolidBrush tBr(tColor);
        cfmt.SetLineAlignment(StringAlignmentFar);
        RectF tRc((REAL)tx, (REAL)toggleY, (REAL)toggleW, (REAL)(toggleH - 8));
        g.DrawString(toggles[i].label.c_str(), -1, &tFont, tRc, &cfmt, &tBr);
        cfmt.SetLineAlignment(StringAlignmentCenter);
    }

    // Sliders
    int sliderY = toggleY + toggleH + 16;

    // Brightness slider
    {
        Font lbl(&ff, 11.0f, FontStyleRegular, UnitPixel);
        SolidBrush lblBr(Color((BYTE)(160 * a), 180, 180, 190));
        RectF lblRc((REAL)(panelX + 18), (REAL)sliderY, 100.0f, 18.0f);
        lfmt.SetLineAlignment(StringAlignmentNear);
        g.DrawString(L"Brightness", -1, &lbl, lblRc, &lfmt, &lblBr);

        int barX2 = panelX + 18, barY2 = sliderY + 22, barW2 = panelW - 36, barH2 = 6;
        FillRoundRectSolid(g, barX2, barY2, barW2, barH2, 3, Color((BYTE)(150 * a), 55, 55, 60));
        int fillW = (int)(barW2 * 0.7f);
        FillRoundRectSolid(g, barX2, barY2, fillW, barH2, 3, Color((BYTE)(230 * a), 76, 194, 255));
    }

    // Volume slider
    sliderY += 40;
    {
        Font lbl(&ff, 11.0f, FontStyleRegular, UnitPixel);
        SolidBrush lblBr(Color((BYTE)(160 * a), 180, 180, 190));
        RectF lblRc((REAL)(panelX + 18), (REAL)sliderY, 100.0f, 18.0f);
        g.DrawString(L"Volume", -1, &lbl, lblRc, &lfmt, &lblBr);

        int barX2 = panelX + 18, barY2 = sliderY + 22, barW2 = panelW - 36, barH2 = 6;
        FillRoundRectSolid(g, barX2, barY2, barW2, barH2, 3, Color((BYTE)(150 * a), 55, 55, 60));
        int fillW = (int)(barW2 * 0.5f);
        FillRoundRectSolid(g, barX2, barY2, fillW, barH2, 3, Color((BYTE)(230 * a), 76, 194, 255));
    }

    // Separator
    sliderY += 44;
    Pen sep(Color((BYTE)(25 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&sep, panelX + 16, sliderY, panelX + panelW - 16, sliderY);

    // WiFi networks header
    Font titleFont(&ff, 13.0f, FontStyleBold, UnitPixel);
    SolidBrush titleBr(Color((BYTE)(255 * a), 240, 240, 240));
    RectF titleRc((REAL)(panelX + 18), (REAL)(sliderY + 8), 200.0f, 24.0f);
    lfmt.SetLineAlignment(StringAlignmentCenter);
    g.DrawString(L"Available networks", -1, &titleFont, titleRc, &lfmt, &titleBr);

    // Scanning indicator
    if (g_wifiScanning) {
        SolidBrush scanBr(Color((BYTE)(120 * a), 76, 194, 255));
        Font scanFont(&ff, 10.0f, FontStyleItalic, UnitPixel);
        StringFormat rfmt2; rfmt2.SetAlignment(StringAlignmentFar); rfmt2.SetLineAlignment(StringAlignmentCenter);
        RectF scanRc((REAL)(panelX + panelW - 120), (REAL)(sliderY + 8), 100.0f, 24.0f);
        g.DrawString(L"Scanning...", -1, &scanFont, scanRc, &rfmt2, &scanBr);
    }

    Font itemFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    Font small2Font(&ff, 10.0f, FontStyleRegular, UnitPixel);
    int listY = sliderY + 40;
    int itemHH = 50;

    for (int i = 0; i < (int)g_wifiNetworks.size() && i < 6; i++) {
        const WifiNetwork& net = g_wifiNetworks[i];
        int iy = listY + i * itemHH;
        bool hov = (g_hoveredWifiItem == i);

        if (hov) {
            FillRoundRectSolid(g, panelX + 8, iy, panelW - 16, itemHH - 4, 6,
                Color((BYTE)(200 * a), 50, 50, 55));
        }

        if (net.connected) {
            DrawRoundRect(g, panelX + 8, iy, panelW - 16, itemHH - 4, 6,
                Color((BYTE)(60 * a), 76, 194, 255), 1.0f);
        }

        // Signal bars
        int sigX = panelX + 22;
        int sigBaseY = iy + itemHH / 2 + 8;
        for (int bar = 0; bar < 4; bar++) {
            int bH = 4 + bar * 4;
            bool filled = net.signal >= (bar + 1) * 25;
            Color barCol = filled ? Color((BYTE)(220 * a), 255, 255, 255) : Color((BYTE)(40 * a), 255, 255, 255);
            SolidBrush barBr(barCol);
            g.FillRectangle(&barBr, sigX + bar * 6, sigBaseY - bH, 4, bH);
        }

        // Lock
        if (net.secured) {
            SolidBrush lockBr(Color((BYTE)(100 * a), 200, 200, 210));
            g.FillRectangle(&lockBr, sigX + 28, sigBaseY - 8, 6, 5);
            Pen lockPen(Color((BYTE)(100 * a), 200, 200, 210), 1.0f);
            g.DrawArc(&lockPen, sigX + 28, sigBaseY - 13, 6, 8, 180, 180);
        }

        // SSID
        SolidBrush nameBr(Color((BYTE)(240 * a), 240, 240, 250));
        RectF nameRc((REAL)(panelX + 60), (REAL)(iy + 6), (REAL)(panelW - 130), 22.0f);
        g.DrawString(net.ssid.c_str(), -1, &itemFont, nameRc, &lfmt, &nameBr);

        // Status
        std::wstring status = net.connected ? L"Connected, secured" : (net.secured ? L"Secured" : L"Open network");
        SolidBrush statusBr(Color((BYTE)(net.connected ? 160 : 100) * a / 255.0f * 255,
            net.connected ? 108 : 160, net.connected ? 203 : 160, net.connected ? 95 : 170));
        RectF statusRc((REAL)(panelX + 60), (REAL)(iy + 28), (REAL)(panelW - 130), 16.0f);
        g.DrawString(status.c_str(), -1, &small2Font, statusRc, &lfmt, &statusBr);

        // Connected checkmark
        if (net.connected) {
            SolidBrush checkBr(Color((BYTE)(200 * a), 108, 203, 95));
            Font checkFont(&ff, 14.0f, FontStyleBold, UnitPixel);
            RectF checkRc((REAL)(panelX + panelW - 40), (REAL)iy, 24.0f, (REAL)itemHH);
            g.DrawString(L"\u2713", -1, &checkFont, checkRc, &cfmt, &checkBr);
        }
    }

    if (g_wifiNetworks.empty() && !g_wifiScanning) {
        SolidBrush emptyBr(Color((BYTE)(120 * a), 150, 150, 160));
        RectF emptyRc((REAL)panelX, (REAL)(listY + 40), (REAL)panelW, 30.0f);
        g.DrawString(L"No networks found", -1, &itemFont, emptyRc, &cfmt, &emptyBr);
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
        FillRoundRectSolid(g, nx, ny, NOTIF_W, NOTIF_H, 8,
            Color((BYTE)(250 * a), 44, 44, 48));
        DrawRoundRect(g, nx, ny, NOTIF_W, NOTIF_H, 8,
            Color((BYTE)(40 * a), 255, 255, 255), 1.0f);

        FillRoundRectSolid(g, nx, ny + 12, 3, NOTIF_H - 24, 1,
            Color((BYTE)(200 * a), 76, 194, 255));

        FillRoundRectSolid(g, nx + 14, ny + 16, 36, 36, 8,
            Color((BYTE)(180 * a), 76, 194, 255));

        FontFamily ff(L"Segoe UI");
        Font icoFont(&ff, 14.0f, FontStyleBold, UnitPixel);
        SolidBrush icoBr(Color((BYTE)(255 * a), 255, 255, 255));
        StringFormat cfmt; cfmt.SetAlignment(StringAlignmentCenter); cfmt.SetLineAlignment(StringAlignmentCenter);
        RectF icoRc((REAL)(nx + 14), (REAL)(ny + 16), 36.0f, 36.0f);
        g.DrawString(L"V", -1, &icoFont, icoRc, &cfmt, &icoBr);

        Font titleFont(&ff, 12.0f, FontStyleBold, UnitPixel);
        Font msgFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
        StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentNear);

        SolidBrush titleBr(Color((BYTE)(250 * a), 245, 245, 250));
        RectF titleRc((REAL)(nx + 60), (REAL)(ny + 16), (REAL)(NOTIF_W - 80), 18.0f);
        g.DrawString(n.title.c_str(), -1, &titleFont, titleRc, &lfmt, &titleBr);

        SolidBrush msgBr(Color((BYTE)(180 * a), 190, 190, 200));
        RectF msgRc((REAL)(nx + 60), (REAL)(ny + 38), (REAL)(NOTIF_W - 80), 28.0f);
        g.DrawString(n.message.c_str(), -1, &msgFont, msgRc, &lfmt, &msgBr);

        DWORD elapsed = (GetTickCount() - n.time) / 1000;
        wchar_t tStr[32];
        if (elapsed < 60) swprintf_s(tStr, L"%ds", elapsed);
        else swprintf_s(tStr, L"%dm", elapsed / 60);
        Font tFont(&ff, 9.0f, FontStyleRegular, UnitPixel);
        SolidBrush tBr(Color((BYTE)(80 * a), 130, 130, 140));
        StringFormat rfmt2; rfmt2.SetAlignment(StringAlignmentFar); rfmt2.SetLineAlignment(StringAlignmentNear);
        RectF tRc((REAL)(nx + NOTIF_W - 50), (REAL)(ny + 8), 40.0f, 14.0f);
        g.DrawString(tStr, -1, &tFont, tRc, &rfmt2, &tBr);
    }
}

// ============================================================================
//  WIDGETS
// ============================================================================
struct Widget {
    std::wstring title, content;
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
    w.title = L"Weather"; w.height = 100; w.accent = Color(255, 255, 200, 50);
    w.content = L"22\xB0  Partly Cloudy\nMoscow, Russia\nFeels like 19\xB0  |  Wind 3 m/s";
    g_widgets.push_back(w);

    MEMORYSTATUSEX mem; mem.dwLength = sizeof(mem); GlobalMemoryStatusEx(&mem);
    int cpuSim = 15 + (g_tick / 1000) % 25;
    swprintf_s(buf, L"CPU: %d%%  |  RAM: %d%%\nDisk: 62%%  |  GPU: 45%%", cpuSim, (int)mem.dwMemoryLoad);
    w.title = L"System Monitor"; w.content = buf; w.height = 80; w.accent = Color(255, 76, 194, 255);
    g_widgets.push_back(w);

    wcsftime(buf, 128, L"%A, %d %B %Y\n%H:%M:%S", &ti);
    w.title = L"Clock"; w.content = buf; w.height = 80; w.accent = Color(255, 200, 200, 200);
    g_widgets.push_back(w);

    w.title = L"Now Playing"; w.height = 90; w.accent = Color(255, 255, 100, 130);
    w.content = g_musicPlaying ? L"Synthwave Mix\nArtist - Track Name\n> 2:15 / 4:30" : L"No music playing\nPress M to start";
    g_widgets.push_back(w);
}

void DrawWidgets(Graphics& g, int sw, int sh) {
    if (g_widgetsAnim <= 0.01f) return;
    float a = g_widgetsAnim;
    float ease = EaseOutCubic(a);

    int panelW = WIDGET_W;
    int offsetX = (int)(-panelW * (1.0f - ease));
    int baseX = 12 + offsetX, baseY = 12;

    FontFamily ff(L"Segoe UI");
    StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentNear);

    for (size_t i = 0; i < g_widgets.size(); i++) {
        const Widget& w = g_widgets[i];
        int x = baseX, y = baseY;

        DrawShadow(g, x, y, panelW, w.height, 6, 3);
        FillRoundRectSolid(g, x, y, panelW, w.height, 8,
            Color((BYTE)(238 * a), 40, 40, 44));
        DrawRoundRect(g, x, y, panelW, w.height, 8,
            Color((BYTE)(25 * a), 255, 255, 255), 1.0f);

        FillRoundRectSolid(g, x + 1, y + 12, 3, w.height - 24, 1,
            Color((BYTE)(200 * a), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));

        Font titleFont(&ff, 11.0f, FontStyleBold, UnitPixel);
        SolidBrush titleBr(Color((BYTE)(180 * a), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));
        RectF titleRc((REAL)(x + 16), (REAL)(y + 10), (REAL)(panelW - 32), 16.0f);
        g.DrawString(w.title.c_str(), -1, &titleFont, titleRc, &lfmt, &titleBr);

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
    // Try loading from cache first
    if (LoadWallpaperFile(cache.c_str())) {
        g_wallpaperReady = true;
        if (g_hWnd) InvalidateRect(g_hWnd, NULL, FALSE);
    }
    // Download new
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
        LinearGradientBrush bg(Point(0, 0), Point(sw, sh),
            Color(255, 20, 40, 80), Color(255, 40, 20, 60));
        g.FillRectangle(&bg, 0, 0, sw, sh);
        for (int i = 0; i < 4; i++) {
            int cx2 = sw / 4 + i * sw / 4;
            int cy2 = sh / 3 + (i % 2) * sh / 3;
            int cr = 200 + i * 50;
            SolidBrush blob(Color(8, 100 + i * 30, 100, 200));
            g.FillEllipse(&blob, cx2 - cr, cy2 - cr, cr * 2, cr * 2);
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

    // Draw explorer windows
    for (auto& win : g_explorers) {
        if (win.visible && win.id != g_activeExplorerId)
            DrawExplorerWindow(g, win);
    }
    // Draw active window last (on top)
    for (auto& win : g_explorers) {
        if (win.visible && win.id == g_activeExplorerId)
            DrawExplorerWindow(g, win);
    }

    DrawWifiPanel(g, sw, sh);
    DrawNotifications(g, sw, sh);
    DrawTaskbar(g, sw, sh);
    DrawContextMenu(g);
    DrawStartMenu(g, sw, sh);

    // Hotkey hints
    if (!g_startMenuOpen && !g_contextMenuOpen && g_explorers.empty()) {
        FontFamily ff(L"Segoe UI");
        Font hintFont(&ff, 9.0f, FontStyleRegular, UnitPixel);
        SolidBrush hintBr(Color(35, 200, 200, 210));
        StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentFar);
        RectF hintRc(12.0f, (REAL)(sh - TASKBAR_HEIGHT - 18), 600.0f, 14.0f);
        g.DrawString(L"ESC close | SPACE menu | M music | W widgets | N notify | F wifi",
            -1, &hintFont, hintRc, &lfmt, &hintBr);
    }

    BitBlt(hdc, 0, 0, sw, sh, g_memDC, 0, 0, SRCCOPY);
}

// ============================================================================
//  OPEN DESKTOP ICON ACTION
// ============================================================================
void OpenDesktopIcon(int idx) {
    if (idx < 0 || idx >= (int)g_desktopIcons.size()) return;
    const DesktopIcon& d = g_desktopIcons[idx];

    if (d.action == L"__thispc__") {
        CreateExplorerWindow(L"", L"This PC");
        // Mark running in taskbar
        if (!g_taskApps.empty()) { g_taskApps[0].running = true; g_taskApps[0].active = true; }
    }
    else if (d.action == L"__folder__") {
        std::wstring path = GetSpecialFolderPath(d.name);
        if (!path.empty()) {
            CreateExplorerWindow(path, d.name);
            if (!g_taskApps.empty()) { g_taskApps[0].running = true; g_taskApps[0].active = true; }
        }
    }
    else {
        ShellExecute(NULL, L"open", d.action.c_str(), NULL, NULL, SW_SHOW);
    }
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
            // Smooth animations
            float speed = 0.18f;

            g_startMenuAnim = SmoothLerp(g_startMenuAnim, g_startMenuOpen ? 1.0f : 0.0f, speed);
            g_widgetsAnim = SmoothLerp(g_widgetsAnim, g_widgetsVisible ? 1.0f : 0.0f, 0.15f);
            g_wifiPanelAnim = SmoothLerp(g_wifiPanelAnim, g_wifiPanelOpen ? 1.0f : 0.0f, speed);
            g_contextMenuAnim = SmoothLerp(g_contextMenuAnim, g_contextMenuOpen ? 1.0f : 0.0f, 0.25f);

            // Explorer window animations
            for (auto it = g_explorers.begin(); it != g_explorers.end(); ) {
                it->animAlpha = SmoothLerp(it->animAlpha, it->targetAlpha, 0.2f);
                it->scrollAnim = SmoothLerp(it->scrollAnim, it->targetScroll, 0.2f);

                if (it->targetAlpha <= 0.0f && it->animAlpha <= 0.01f) {
                    it = g_explorers.erase(it);
                    // Update taskbar
                    if (g_explorers.empty() && !g_taskApps.empty()) {
                        g_taskApps[0].running = false;
                        g_taskApps[0].active = false;
                    }
                }
                else {
                    ++it;
                }
            }

            // Notifications
            for (auto& n : g_notifs) {
                if (!n.alive) continue;
                if (n.alpha < 1.0f) n.alpha = (std::min)(1.0f, n.alpha + 0.08f);
                if (n.offsetY < 0) n.offsetY = (std::min)(0.0f, n.offsetY + 4.0f);
                if (GetTickCount() - n.time > 5000) {
                    n.alpha -= 0.04f;
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

    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        // Scroll active explorer
        if (g_activeExplorerId >= 0) {
            ExplorerWindow* win = FindExplorer(g_activeExplorerId);
            if (win && win->visible) {
                win->targetScroll -= delta / 2;
                int maxScroll = (int)win->items.size() * 32 - (win->h - 72);
                if (win->currentPath.empty()) maxScroll = (int)win->items.size() * 80 - (win->h - 72);
                if (maxScroll < 0) maxScroll = 0;
                if (win->targetScroll < 0) win->targetScroll = 0;
                if (win->targetScroll > maxScroll) win->targetScroll = (float)maxScroll;
            }
        }
        break;
    }

    case WM_MOUSEMOVE: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);
        POINT pt = { mx, my };

        // Explorer window dragging
        if (g_explorerDragging >= 0) {
            ExplorerWindow* win = FindExplorer(g_explorerDragging);
            if (win) {
                win->x = mx - win->dragOffX;
                win->y = my - win->dragOffY;
                // Clamp
                if (win->y < 0) win->y = 0;
                if (win->y > ScreenH() - 50) win->y = ScreenH() - 50;
            }
            break;
        }

        // Explorer window resizing
        if (g_explorerResizing >= 0) {
            ExplorerWindow* win = FindExplorer(g_explorerResizing);
            if (win) {
                int dx = mx - win->resizeStartX;
                int dy = my - win->resizeStartY;
                if (win->resizeEdge & 1) win->w = (std::max)(400, win->resizeStartW + dx);
                if (win->resizeEdge & 2) win->h = (std::max)(300, win->resizeStartH + dy);
            }
            break;
        }

        // Desktop icon dragging
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
            g_selEndX = mx; g_selEndY = my;
            int x1 = (std::min)(g_selStartX, g_selEndX);
            int y1 = (std::min)(g_selStartY, g_selEndY);
            int x2 = (std::max)(g_selStartX, g_selEndX);
            int y2 = (std::max)(g_selStartY, g_selEndY);
            RECT selRect; SetRect(&selRect, x1, y1, x2, y2);
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
            int cmx = g_contextMenuX, cmy = g_contextMenuY;
            int menuH = 8;
            for (const auto& item : g_contextItems)
                menuH += (item.id == 0) ? CTX_SEP_H : CTX_ITEM_H;
            menuH += 8;
            if (cmx + menuW > ScreenW()) cmx = ScreenW() - menuW - 8;
            if (cmy + menuH > ScreenH()) cmy = ScreenH() - menuH - 8;
            int cy = cmy + 8;
            for (int i = 0; i < (int)g_contextItems.size(); i++) {
                if (g_contextItems[i].id == 0) { cy += CTX_SEP_H; continue; }
                RECT ir; SetRect(&ir, cmx + 4, cy, cmx + menuW - 4, cy + CTX_ITEM_H);
                if (PtInRect(&ir, pt)) { g_hoveredContextItem = i; break; }
                cy += CTX_ITEM_H;
            }
        }

        // Explorer window hover
        for (int ei = (int)g_explorers.size() - 1; ei >= 0; ei--) {
            ExplorerWindow& win = g_explorers[ei];
            if (!win.visible || win.animAlpha < 0.5f) continue;

            int wx2 = win.x, wy2 = win.y, ww2 = win.w, wh2 = win.h;
            RECT winRect; SetRect(&winRect, wx2, wy2, wx2 + ww2, wy2 + wh2);

            if (PtInRect(&winRect, pt) && win.id == g_activeExplorerId) {
                // Title buttons hover
                win.hoveredTitleBtn = 0;
                int btnW2 = 46;
                for (int b = 1; b <= 3; b++) {
                    int bx = wx2 + ww2 - btnW2 * (4 - b);
                    RECT br; SetRect(&br, bx, wy2, bx + btnW2, wy2 + 32);
                    if (PtInRect(&br, pt)) { win.hoveredTitleBtn = b; break; }
                }

                // Sidebar hover
                win.hoveredSidebarItem = -1;
                int sidebarW2 = 200;
                int contentY2 = wy2 + 72;
                int sideY2 = contentY2 + 36; // After header
                for (int si = 0; si < 7; si++) {
                    RECT sr; SetRect(&sr, wx2 + 4, sideY2, wx2 + sidebarW2 - 4, sideY2 + 30);
                    if (PtInRect(&sr, pt)) { win.hoveredSidebarItem = si; break; }
                    sideY2 += 30;
                    if (si == 5) sideY2 += 38; // Extra space for separator
                }

                // Content items hover
                win.hoveredItem = -1;
                int contentX2 = wx2 + sidebarW2;
                int contentW2 = ww2 - sidebarW2;
                int contentH2 = wh2 - 72;

                if (mx > contentX2 && mx < wx2 + ww2 && my > contentY2 && my < wy2 + wh2) {
                    if (win.currentPath.empty()) {
                        // This PC - tile layout
                        int tileW = 240, tileH = 70;
                        int cols = (std::max)(1, (contentW2 - 40) / (tileW + 10));
                        int startY = contentY2 + 52 - (int)win.scrollAnim;
                        for (int i = 0; i < (int)win.items.size(); i++) {
                            int col = i % cols, row = i / cols;
                            int tx = contentX2 + 20 + col * (tileW + 10);
                            int ty = startY + row * (tileH + 10);
                            RECT tr; SetRect(&tr, tx, ty, tx + tileW, ty + tileH);
                            if (PtInRect(&tr, pt)) { win.hoveredItem = i; break; }
                        }
                    }
                    else {
                        // List layout
                        int headerH = 34;
                        int itemH2 = 32;
                        int listY2 = contentY2 + headerH - (int)win.scrollAnim;
                        for (int i = 0; i < (int)win.items.size(); i++) {
                            int iy = listY2 + i * itemH2;
                            if (iy < contentY2 || iy + itemH2 > wy2 + wh2) continue;
                            RECT ir; SetRect(&ir, contentX2 + 4, iy, contentX2 + contentW2 - 4, iy + itemH2);
                            if (PtInRect(&ir, pt)) { win.hoveredItem = i; break; }
                        }
                    }
                }
                break; // Only process topmost window
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
            int iconY = barY + (TASKBAR_HEIGHT - TASKBAR_ICON_SIZE) / 2;

            // Start button
            RECT sr; SetRect(&sr, startX, iconY, startX + TASKBAR_ICON_SIZE, iconY + TASKBAR_ICON_SIZE);
            if (PtInRect(&sr, pt)) g_hoveredTaskbarIcon = -10;

            int searchBtnX = startX + (TASKBAR_ICON_SIZE + 4);
            RECT sr2; SetRect(&sr2, searchBtnX, iconY, searchBtnX + TASKBAR_ICON_SIZE, iconY + TASKBAR_ICON_SIZE);
            if (PtInRect(&sr2, pt)) g_hoveredTaskbarIcon = -11;

            int tvBtnX = startX + 2 * (TASKBAR_ICON_SIZE + 4);
            RECT sr3; SetRect(&sr3, tvBtnX, iconY, tvBtnX + TASKBAR_ICON_SIZE, iconY + TASKBAR_ICON_SIZE);
            if (PtInRect(&sr3, pt)) g_hoveredTaskbarIcon = -12;

            for (int i = 0; i < iconCount; i++) {
                if (PtInRect(&g_taskApps[i].bounds, pt)) { g_hoveredTaskbarIcon = i; break; }
            }

            // WiFi icon area in system tray
            int trayX = sw - 220;
            RECT wifiRect; SetRect(&wifiRect, trayX + 26, barY, trayX + 56, barY + TASKBAR_HEIGHT);
            if (PtInRect(&wifiRect, pt)) g_hoveredTaskbarIcon = -20;
        }

        // Desktop icon hover
        g_hoveredDesktopIcon = -1;
        if (!g_startMenuOpen && !g_contextMenuOpen) {
            bool overExplorer = false;
            for (auto& win : g_explorers) {
                if (win.visible && win.animAlpha > 0.5f) {
                    RECT wr; SetRect(&wr, win.x, win.y, win.x + win.w, win.y + win.h);
                    if (PtInRect(&wr, pt)) { overExplorer = true; break; }
                }
            }
            if (!overExplorer) {
                for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
                    RECT r;
                    SetRect(&r, g_desktopIcons[i].pixelX, g_desktopIcons[i].pixelY,
                        g_desktopIcons[i].pixelX + DESKTOP_ICON_W, g_desktopIcons[i].pixelY + DESKTOP_ICON_H);
                    if (PtInRect(&r, pt)) { g_hoveredDesktopIcon = i; break; }
                }
            }
        }

        // WiFi hover
        g_hoveredWifiItem = -1;
        if (g_wifiPanelOpen && g_wifiPanelAnim > 0.5f) {
            int panelX = sw - WIFI_PANEL_W - 12;
            int panelY = sh - TASKBAR_HEIGHT - WIFI_PANEL_H - 12;
            // Calculate list position matching draw code
            int toggleH = 70;
            int listY2 = panelY + 16 + toggleH + 16 + 40 + 44 + 25 + 40;
            int itemHH = 50;
            for (int i = 0; i < (int)g_wifiNetworks.size() && i < 6; i++) {
                int iy = listY2 + i * itemHH;
                RECT ir; SetRect(&ir, panelX + 8, iy, panelX + WIFI_PANEL_W - 8, iy + itemHH);
                if (PtInRect(&ir, pt)) { g_hoveredWifiItem = i; break; }
            }
        }

        // Start menu hover
        g_hoveredStartItem = -1;
        if (g_startMenuOpen && g_startMenuAnim > 0.5f) {
            int menuX = (sw - START_MENU_W) / 2;
            int menuY2 = sh - TASKBAR_HEIGHT - START_MENU_H - 12;
            int gridX = menuX + 28, gridY = menuY2 + 108;
            int cols = 6;
            int cellW = (START_MENU_W - 56) / cols, cellH = 72;
            for (int i = 0; i < (int)g_startApps.size() && i < 18; i++) {
                int row = i / cols, col = i % cols;
                int ix = gridX + col * cellW;
                int iy = gridY + row * cellH;
                RECT cr; SetRect(&cr, ix, iy, ix + cellW, iy + cellH);
                if (PtInRect(&cr, pt)) { g_hoveredStartItem = i; break; }
            }
        }

        // Set cursor for explorer resize
        bool onResizeEdge = false;
        for (auto& win : g_explorers) {
            if (!win.visible || win.animAlpha < 0.5f) continue;
            int wx2 = win.x, wy2 = win.y, ww2 = win.w, wh2 = win.h;
            // Bottom-right corner
            if (mx > wx2 + ww2 - 8 && mx < wx2 + ww2 + 4 && my > wy2 + wh2 - 8 && my < wy2 + wh2 + 4) {
                SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
                onResizeEdge = true;
                break;
            }
            // Right edge
            if (mx > wx2 + ww2 - 4 && mx < wx2 + ww2 + 4 && my > wy2 + 32 && my < wy2 + wh2 - 8) {
                SetCursor(LoadCursor(NULL, IDC_SIZEWE));
                onResizeEdge = true;
                break;
            }
            // Bottom edge
            if (my > wy2 + wh2 - 4 && my < wy2 + wh2 + 4 && mx > wx2 && mx < wx2 + ww2 - 8) {
                SetCursor(LoadCursor(NULL, IDC_SIZENS));
                onResizeEdge = true;
                break;
            }
        }
        if (!onResizeEdge) {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
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
                case 10:
                    for (int i = 0; i < (int)g_desktopIcons.size(); i++)
                        if (g_desktopIcons[i].selected) OpenDesktopIcon(i);
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
                std::wstring exec = g_startApps[g_hoveredStartItem].exec;
                if (exec == L"__thispc__") {
                    CreateExplorerWindow(L"", L"This PC");
                    if (!g_taskApps.empty()) { g_taskApps[0].running = true; g_taskApps[0].active = true; }
                }
                else {
                    ShellExecute(NULL, L"open", exec.c_str(), NULL, NULL, SW_SHOW);
                }
                g_startMenuOpen = false;
            }
            else {
                int sw2 = ScreenW(), sh2 = ScreenH();
                int menuX = (sw2 - START_MENU_W) / 2;
                int menuY2 = sh2 - TASKBAR_HEIGHT - START_MENU_H - 12;
                RECT mr; SetRect(&mr, menuX, menuY2, menuX + START_MENU_W, menuY2 + START_MENU_H);
                if (!PtInRect(&mr, pt)) g_startMenuOpen = false;
            }
            break;
        }

        // WiFi panel
        if (g_wifiPanelOpen && g_wifiPanelAnim > 0.5f) {
            int sw2 = ScreenW(), sh2 = ScreenH();
            int panelX = sw2 - WIFI_PANEL_W - 12;
            int panelY = sh2 - TASKBAR_HEIGHT - WIFI_PANEL_H - 12;
            RECT pr; SetRect(&pr, panelX, panelY, panelX + WIFI_PANEL_W, panelY + WIFI_PANEL_H);
            if (!PtInRect(&pr, pt) && my < ScreenH() - TASKBAR_HEIGHT)
                g_wifiPanelOpen = false;
        }

        // Explorer windows (from front to back)
        bool clickedExplorer = false;
        for (int ei = (int)g_explorers.size() - 1; ei >= 0; ei--) {
            ExplorerWindow& win = g_explorers[ei];
            if (!win.visible || win.animAlpha < 0.5f) continue;

            int wx2 = win.x, wy2 = win.y, ww2 = win.w, wh2 = win.h;
            RECT winRect; SetRect(&winRect, wx2, wy2, wx2 + ww2, wy2 + wh2);

            if (!PtInRect(&winRect, pt)) continue;

            clickedExplorer = true;
            g_activeExplorerId = win.id;

            // Check resize edges first
            if (mx > wx2 + ww2 - 8 && mx < wx2 + ww2 + 4 && my > wy2 + wh2 - 8 && my < wy2 + wh2 + 4) {
                g_explorerResizing = win.id;
                win.resizing = true;
                win.resizeEdge = 3;
                win.resizeStartX = mx; win.resizeStartY = my;
                win.resizeStartW = ww2; win.resizeStartH = wh2;
                SetCapture(hWnd);
                break;
            }
            if (mx > wx2 + ww2 - 4 && mx < wx2 + ww2 + 4 && my > wy2 + 32) {
                g_explorerResizing = win.id;
                win.resizing = true;
                win.resizeEdge = 1;
                win.resizeStartX = mx; win.resizeStartY = my;
                win.resizeStartW = ww2; win.resizeStartH = wh2;
                SetCapture(hWnd);
                break;
            }
            if (my > wy2 + wh2 - 4 && my < wy2 + wh2 + 4) {
                g_explorerResizing = win.id;
                win.resizing = true;
                win.resizeEdge = 2;
                win.resizeStartX = mx; win.resizeStartY = my;
                win.resizeStartW = ww2; win.resizeStartH = wh2;
                SetCapture(hWnd);
                break;
            }

            // Title bar buttons
            int btnW2 = 46;
            for (int b = 1; b <= 3; b++) {
                int bx = wx2 + ww2 - btnW2 * (4 - b);
                RECT br; SetRect(&br, bx, wy2, bx + btnW2, wy2 + 32);
                if (PtInRect(&br, pt)) {
                    if (b == 3) { // Close
                        CloseExplorer(win.id);
                    }
                    else if (b == 2) { // Maximize
                        if (!win.maximized) {
                            win.maximized = true;
                            win.x = 0; win.y = 0;
                            win.w = ScreenW(); win.h = ScreenH() - TASKBAR_HEIGHT;
                        }
                        else {
                            win.maximized = false;
                            win.x = (ScreenW() - 900) / 2;
                            win.y = (ScreenH() - 600) / 2;
                            win.w = 900; win.h = 600;
                        }
                    }
                    else if (b == 1) { // Minimize
                        win.targetAlpha = 0.0f;
                    }
                    clickedExplorer = true;
                    break;
                }
            }

            // Title bar drag
            if (my < wy2 + 32 && mx < wx2 + ww2 - btnW2 * 3) {
                g_explorerDragging = win.id;
                win.dragging = true;
                win.dragOffX = mx - wx2;
                win.dragOffY = my - wy2;
                SetCapture(hWnd);
                break;
            }

            // Navigation buttons
            int toolY2 = wy2 + 32;
            int navX2 = wx2 + 12;
            int navBtnSize = 28;

            // Back
            RECT backRect; SetRect(&backRect, navX2, toolY2 + 6, navX2 + navBtnSize, toolY2 + 34);
            if (PtInRect(&backRect, pt) && win.historyIndex > 0) {
                win.historyIndex--;
                win.currentPath = win.pathHistory[win.historyIndex];
                if (win.currentPath.empty()) {
                    win.title = L"This PC";
                    LoadDrives(win);
                }
                else {
                    std::wstring t = win.currentPath;
                    if (t.back() == L'\\') t.pop_back();
                    size_t pos = t.find_last_of(L'\\');
                    if (pos != std::wstring::npos) t = t.substr(pos + 1);
                    win.title = t;
                    LoadDirectory(win, win.currentPath);
                }
                win.scrollOffset = 0; win.scrollAnim = 0; win.targetScroll = 0;
                break;
            }
            navX2 += navBtnSize + 4;

            // Forward
            RECT fwdRect; SetRect(&fwdRect, navX2, toolY2 + 6, navX2 + navBtnSize, toolY2 + 34);
            if (PtInRect(&fwdRect, pt) && win.historyIndex < (int)win.pathHistory.size() - 1) {
                win.historyIndex++;
                win.currentPath = win.pathHistory[win.historyIndex];
                if (win.currentPath.empty()) {
                    win.title = L"This PC";
                    LoadDrives(win);
                }
                else {
                    std::wstring t = win.currentPath;
                    if (t.back() == L'\\') t.pop_back();
                    size_t pos = t.find_last_of(L'\\');
                    if (pos != std::wstring::npos) t = t.substr(pos + 1);
                    win.title = t;
                    LoadDirectory(win, win.currentPath);
                }
                win.scrollOffset = 0; win.scrollAnim = 0; win.targetScroll = 0;
                break;
            }
            navX2 += navBtnSize + 4;

            // Up
            RECT upRect; SetRect(&upRect, navX2, toolY2 + 6, navX2 + navBtnSize, toolY2 + 34);
            if (PtInRect(&upRect, pt) && !win.currentPath.empty()) {
                std::wstring parent = win.currentPath;
                if (parent.back() == L'\\') parent.pop_back();
                size_t pos = parent.find_last_of(L'\\');
                if (pos != std::wstring::npos && pos > 2) {
                    parent = parent.substr(0, pos);
                }
                else if (pos == 2) {
                    parent = parent.substr(0, 3); // e.g. "C:\"
                }
                else {
                    parent = L""; // Go to This PC
                }
                NavigateExplorer(win, parent);
                break;
            }

            // Sidebar clicks
            if (win.hoveredSidebarItem >= 0) {
                std::wstring paths[] = {
                    GetSpecialFolderPath(L"Desktop"),
                    GetSpecialFolderPath(L"Downloads"),
                    GetSpecialFolderPath(L"Documents"),
                    GetSpecialFolderPath(L"Pictures"),
                    GetSpecialFolderPath(L"Music"),
                    GetSpecialFolderPath(L"Videos"),
                    L""  // This PC
                };
                NavigateExplorer(win, paths[win.hoveredSidebarItem]);
                break;
            }

            // Content item click
            if (win.hoveredItem >= 0 && win.hoveredItem < (int)win.items.size()) {
                win.selectedItem = win.hoveredItem;
            }

            break;
        }

        // Taskbar clicks
        int sw2 = ScreenW(), sh2 = ScreenH();
        if (my >= sh2 - TASKBAR_HEIGHT) {
            if (g_hoveredTaskbarIcon == -10) {
                g_startMenuOpen = !g_startMenuOpen;
                g_wifiPanelOpen = false;
            }
            else if (g_hoveredTaskbarIcon == -20) {
                g_wifiPanelOpen = !g_wifiPanelOpen;
                g_startMenuOpen = false;
                if (g_wifiPanelOpen && !g_wifiScanning)
                    CreateThread(NULL, 0, WifiScanThread, NULL, 0, NULL);
            }
            else if (g_hoveredTaskbarIcon >= 0) {
                std::wstring exec = g_taskApps[g_hoveredTaskbarIcon].exec;
                if (exec == L"__thispc__") {
                    if (g_explorers.empty()) {
                        CreateExplorerWindow(L"", L"This PC");
                        g_taskApps[g_hoveredTaskbarIcon].running = true;
                        g_taskApps[g_hoveredTaskbarIcon].active = true;
                    }
                    else {
                        // Bring to front or toggle
                        for (auto& w : g_explorers) {
                            if (w.targetAlpha <= 0) w.targetAlpha = 1.0f;
                            g_activeExplorerId = w.id;
                        }
                    }
                }
                else {
                    ShellExecute(NULL, L"open", exec.c_str(), NULL, NULL, SW_SHOW);
                }
            }
            break;
        }

        if (clickedExplorer) break;

        // Desktop icon click / drag start
        bool clickedIcon = false;
        for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
            RECT r;
            SetRect(&r, g_desktopIcons[i].pixelX, g_desktopIcons[i].pixelY,
                g_desktopIcons[i].pixelX + DESKTOP_ICON_W, g_desktopIcons[i].pixelY + DESKTOP_ICON_H);
            if (PtInRect(&r, pt)) {
                clickedIcon = true;
                if (!(GetKeyState(VK_CONTROL) & 0x8000))
                    for (auto& d : g_desktopIcons) d.selected = false;
                g_desktopIcons[i].selected = true;

                g_dragging = true;
                g_dragIconIdx = i;
                g_dragOffsetX = mx - g_desktopIcons[i].pixelX;
                g_dragOffsetY = my - g_desktopIcons[i].pixelY;
                g_dragCurrentX = mx; g_dragCurrentY = my;
                g_dragStarted = false;
                SetCapture(hWnd);
                break;
            }
        }

        if (!clickedIcon && my < sh2 - TASKBAR_HEIGHT) {
            for (auto& d : g_desktopIcons) d.selected = false;
            g_activeExplorerId = -1;
            g_selecting = true;
            g_selStartX = mx; g_selStartY = my;
            g_selEndX = mx; g_selEndY = my;
            SetCapture(hWnd);
        }
        break;
    }

    case WM_LBUTTONUP: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);

        if (g_explorerDragging >= 0) {
            ExplorerWindow* win = FindExplorer(g_explorerDragging);
            if (win) win->dragging = false;
            g_explorerDragging = -1;
            ReleaseCapture();
        }

        if (g_explorerResizing >= 0) {
            ExplorerWindow* win = FindExplorer(g_explorerResizing);
            if (win) win->resizing = false;
            g_explorerResizing = -1;
            ReleaseCapture();
        }

        if (g_dragging && g_dragIconIdx >= 0) {
            if (g_dragStarted) {
                int newX = mx - g_dragOffsetX;
                int newY = my - g_dragOffsetY;
                int oldCol = g_desktopIcons[g_dragIconIdx].gridCol;
                int oldRow = g_desktopIcons[g_dragIconIdx].gridRow;
                SnapToGrid(g_desktopIcons[g_dragIconIdx], newX + DESKTOP_ICON_W / 2, newY + DESKTOP_ICON_H / 2);
                if (IsGridOccupied(g_desktopIcons[g_dragIconIdx].gridCol,
                    g_desktopIcons[g_dragIconIdx].gridRow, g_dragIconIdx)) {
                    g_desktopIcons[g_dragIconIdx].gridCol = oldCol;
                    g_desktopIcons[g_dragIconIdx].gridRow = oldRow;
                    CalcIconPixelPos(g_desktopIcons[g_dragIconIdx]);
                }
            }
            g_dragging = false; g_dragIconIdx = -1; g_dragStarted = false;
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

        // Double-click on desktop icon
        for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
            RECT r;
            SetRect(&r, g_desktopIcons[i].pixelX, g_desktopIcons[i].pixelY,
                g_desktopIcons[i].pixelX + DESKTOP_ICON_W, g_desktopIcons[i].pixelY + DESKTOP_ICON_H);
            if (PtInRect(&r, pt)) {
                OpenDesktopIcon(i);
                break;
            }
        }

        // Double-click on explorer item
        for (int ei = (int)g_explorers.size() - 1; ei >= 0; ei--) {
            ExplorerWindow& win = g_explorers[ei];
            if (!win.visible || win.animAlpha < 0.5f || win.id != g_activeExplorerId) continue;

            if (win.hoveredItem >= 0 && win.hoveredItem < (int)win.items.size()) {
                const FileItem& fi = win.items[win.hoveredItem];
                if (fi.isDirectory) {
                    if (fi.name == L"..") {
                        // Go up
                        std::wstring parent = win.currentPath;
                        if (parent.back() == L'\\') parent.pop_back();
                        size_t pos = parent.find_last_of(L'\\');
                        if (pos != std::wstring::npos && pos > 2) parent = parent.substr(0, pos);
                        else if (pos == 2) parent = parent.substr(0, 3);
                        else parent = L"";
                        NavigateExplorer(win, parent);
                    }
                    else {
                        NavigateExplorer(win, fi.fullPath);
                    }
                }
                else {
                    // Open file
                    ShellExecute(NULL, L"open", fi.fullPath.c_str(), NULL, NULL, SW_SHOW);
                }
                break;
            }
            break;
        }
        break;
    }

    case WM_RBUTTONDOWN: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);
        POINT pt = { mx, my };

        if (g_startMenuOpen) { g_startMenuOpen = false; break; }
        if (g_contextMenuOpen) { g_contextMenuOpen = false; break; }

        // Check if over an explorer window - don't show desktop context menu
        for (auto& win : g_explorers) {
            if (win.visible && win.animAlpha > 0.5f) {
                RECT wr; SetRect(&wr, win.x, win.y, win.x + win.w, win.y + win.h);
                if (PtInRect(&wr, pt)) return 0; // Consume
            }
        }

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
        g_contextMenuX = mx; g_contextMenuY = my;
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
            else if (!g_explorers.empty()) {
                // Close active explorer
                if (g_activeExplorerId >= 0) CloseExplorer(g_activeExplorerId);
                else if (!g_explorers.empty()) CloseExplorer(g_explorers.back().id);
            }
            else PostQuitMessage(0);
        }
        else if (wParam == VK_SPACE) {
            g_startMenuOpen = !g_startMenuOpen;
            g_contextMenuOpen = false; g_wifiPanelOpen = false;
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
            g_desktopIcons.erase(
                std::remove_if(g_desktopIcons.begin(), g_desktopIcons.end(),
                    [](const DesktopIcon& d) { return d.selected && d.iconType == 2; }),
                g_desktopIcons.end());
        }
        else if (wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
            for (auto& d : g_desktopIcons) d.selected = true;
        }
        else if (wParam == VK_RETURN) {
            // Enter on explorer - open selected item
            if (g_activeExplorerId >= 0) {
                ExplorerWindow* win = FindExplorer(g_activeExplorerId);
                if (win && win->selectedItem >= 0 && win->selectedItem < (int)win->items.size()) {
                    const FileItem& fi = win->items[win->selectedItem];
                    if (fi.isDirectory) {
                        if (fi.name == L"..") {
                            std::wstring parent = win->currentPath;
                            if (parent.back() == L'\\') parent.pop_back();
                            size_t pos = parent.find_last_of(L'\\');
                            if (pos != std::wstring::npos && pos > 2) parent = parent.substr(0, pos);
                            else if (pos == 2) parent = parent.substr(0, 3);
                            else parent = L"";
                            NavigateExplorer(*win, parent);
                        }
                        else {
                            NavigateExplorer(*win, fi.fullPath);
                        }
                    }
                    else {
                        ShellExecute(NULL, L"open", fi.fullPath.c_str(), NULL, NULL, SW_SHOW);
                    }
                }
            }
        }
        else if (wParam == VK_BACK) {
            // Backspace in explorer - go up
            if (g_activeExplorerId >= 0) {
                ExplorerWindow* win = FindExplorer(g_activeExplorerId);
                if (win && !win->currentPath.empty()) {
                    std::wstring parent = win->currentPath;
                    if (parent.back() == L'\\') parent.pop_back();
                    size_t pos = parent.find_last_of(L'\\');
                    if (pos != std::wstring::npos && pos > 2) parent = parent.substr(0, pos);
                    else if (pos == 2) parent = parent.substr(0, 3);
                    else parent = L"";
                    NavigateExplorer(*win, parent);
                }
            }
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
    wc.lpszClassName = L"VORTEX_Desktop_v4";
    RegisterClassExW(&wc);

    int sw = ScreenW(), sh = ScreenH();
    g_hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"VORTEX_Desktop_v4", L"VORTEX Desktop v4",
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
