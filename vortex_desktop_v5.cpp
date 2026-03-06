/*
 * VORTEX Desktop Environment v5.0 - Ultra Smooth Fluent Design
 * Complete visual overhaul with world-class UI polish
 *
 * Enhancements over v4:
 * - 144Hz-ready ultra-smooth spring-physics animations
 * - World-class taskbar with glassmorphism, glow effects, pill indicators
 * - Stunning system tray with animated WiFi waves, battery glow, volume arcs
 * - Acrylic blur simulation on all flyouts
 * - Hover micro-animations with scale transforms
 * - Gradient accent highlights everywhere
 * - Beautiful icon rendering with shadows and depth
 * - Smooth scroll with momentum/inertia
 * - Window open/close scale+fade animations
 * - Notification slide-in with bounce
 * - Start menu with frosted glass and staggered reveals
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
#pragma comment(lib, "msimg32.lib")

#include <gdiplus.h>
using namespace Gdiplus;

// ============================================================================
//  SPRING PHYSICS + SMOOTH ANIMATION ENGINE
// ============================================================================
struct SpringValue {
    float current;
    float target;
    float velocity;
    float tension;
    float friction;

    SpringValue(float init = 0.0f, float t = 180.0f, float f = 12.0f)
        : current(init), target(init), velocity(0), tension(t), friction(f) {
    }

    void SetTarget(float t) { target = t; }

    void Update(float dt = 0.016f) {
        float force = tension * (target - current);
        float damping = -friction * velocity;
        float acceleration = force + damping;
        velocity += acceleration * dt;
        current += velocity * dt;
        if (fabsf(target - current) < 0.001f && fabsf(velocity) < 0.01f) {
            current = target;
            velocity = 0;
        }
    }

    bool IsSettled() const {
        return current == target && velocity == 0;
    }

    operator float() const { return current; }
};

inline float EaseOutCubic(float t) {
    t = t - 1.0f;
    return t * t * t + 1.0f;
}
inline float EaseOutQuint(float t) {
    t = t - 1.0f;
    return t * t * t * t * t + 1.0f;
}
inline float EaseOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
}
inline float EaseInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}
inline float EaseOutElastic(float t) {
    if (t <= 0) return 0;
    if (t >= 1) return 1;
    return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * 2.0943951f) + 1.0f;
}
inline float SmoothLerp(float current, float target, float speed) {
    float diff = target - current;
    if (fabsf(diff) < 0.001f) return target;
    return current + diff * speed;
}
inline float Clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }

// Perlin-like smooth noise for subtle breathing animations
inline float SmoothNoise(float t) {
    return sinf(t * 1.7f) * 0.3f + sinf(t * 3.1f) * 0.2f + sinf(t * 0.8f) * 0.5f;
}

// ============================================================================
//  WINDOWS 11 COLOR PALETTE - Enhanced with gradients
// ============================================================================
namespace W11 {
    const Color Accent(255, 0, 103, 192);
    const Color AccentLight(255, 96, 205, 255);
    const Color AccentLighter(255, 152, 224, 255);
    const Color AccentDark(255, 0, 62, 146);
    const Color AccentSubtle(40, 0, 120, 215);
    const Color AccentGlow(60, 96, 205, 255);

    const Color SurfaceBase(255, 25, 25, 25);
    const Color SurfaceCard(235, 40, 40, 42);
    const Color SurfaceCardHover(245, 52, 52, 56);
    const Color SurfaceFlyout(248, 40, 40, 42);
    const Color SurfaceOverlay(200, 0, 0, 0);
    const Color SurfaceStroke(50, 255, 255, 255);
    const Color SurfaceStrokeLight(25, 255, 255, 255);
    const Color SurfaceDivider(18, 255, 255, 255);

    const Color MicaBg(225, 28, 28, 30);
    const Color AcrylicBg(210, 32, 32, 34);

    const Color TextPrimary(255, 255, 255, 255);
    const Color TextSecondary(255, 190, 190, 195);
    const Color TextTertiary(255, 130, 130, 138);
    const Color TextDisabled(255, 85, 85, 90);

    const Color Success(255, 108, 203, 95);
    const Color Warning(255, 255, 210, 56);
    const Color Error(255, 255, 99, 97);
    const Color Info(255, 96, 205, 255);

    // Taskbar - glassmorphism
    const Color TaskbarBg(230, 22, 22, 24);
    const Color TaskbarGlass(180, 32, 32, 34);
    const Color TaskbarHover(255, 55, 55, 58);
    const Color TaskbarActive(255, 62, 62, 66);
    const Color TaskbarIndicator(255, 96, 205, 255);
    const Color TaskbarIndicatorGlow(80, 96, 205, 255);

    const Color StartBg(244, 32, 32, 34);
    const Color StartSearch(255, 50, 50, 54);
    const Color StartItemHover(255, 58, 58, 62);

    const Color CtxBg(248, 40, 40, 42);
    const Color CtxHover(255, 56, 56, 60);

    const Color SelectionRect(70, 0, 120, 215);
    const Color SelectionBorder(180, 96, 205, 255);
    const Color IconSelectedBg(55, 0, 120, 215);
    const Color IconHoverBg(35, 255, 255, 255);

    const Color WinTitleBar(255, 28, 28, 30);
    const Color WinBody(255, 25, 25, 27);
    const Color WinSidebar(255, 30, 30, 33);
    const Color WinToolbar(255, 34, 34, 37);
    const Color WinItemHover(255, 48, 48, 52);
    const Color WinItemSelected(255, 0, 72, 148);
    const Color WinScrollbar(255, 50, 50, 54);
    const Color WinScrollThumb(255, 76, 76, 82);
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
const int DESKTOP_GRID_X = 82;
const int DESKTOP_GRID_Y = 92;
const int DESKTOP_MARGIN_X = 14;
const int DESKTOP_MARGIN_Y = 14;

const int START_MENU_W = 620;
const int START_MENU_H = 580;
const int START_MENU_RADIUS = 12;

const int CTX_MENU_W = 260;
const int CTX_ITEM_H = 36;
const int CTX_SEP_H = 9;
const int CTX_RADIUS = 10;

const int WIFI_PANEL_W = 380;
const int WIFI_PANEL_H = 460;

const int NOTIF_W = 370;
const int NOTIF_H = 86;

const int WIDGET_W = 330;

// ============================================================================
//  EXPLORER WINDOW - Full file browser
// ============================================================================
struct FileItem {
    std::wstring name;
    std::wstring fullPath;
    bool isDirectory;
    bool isDrive;
    ULONGLONG fileSize;
    ULONGLONG totalSpace;
    ULONGLONG freeSpace;
    std::wstring driveType;
    FILETIME modTime;
};

struct ExplorerWindow {
    int id;
    std::wstring title;
    std::wstring currentPath;
    int x, y, w, h;
    SpringValue animAlpha;
    SpringValue animScale;
    bool visible;
    bool maximized;
    bool dragging;
    int dragOffX, dragOffY;
    bool resizing;
    int resizeEdge;
    int resizeStartX, resizeStartY, resizeStartW, resizeStartH;

    std::vector<FileItem> items;
    int scrollOffset;
    int hoveredItem;
    int selectedItem;
    std::vector<std::wstring> pathHistory;
    int historyIndex;

    int hoveredSidebarItem;

    SpringValue scrollAnim;
    float targetScroll;

    int hoveredTitleBtn;
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
float g_time = 0; // Continuous time in seconds

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

SpringValue g_startMenuAnim(0, 220, 16);
SpringValue g_widgetsAnim(0, 160, 14);
SpringValue g_wifiPanelAnim(0, 200, 15);
SpringValue g_contextMenuAnim(0, 280, 18);

// Taskbar icon hover springs
struct TaskbarIconAnim {
    SpringValue hover;
    SpringValue scale;
    SpringValue glow;
    TaskbarIconAnim() : hover(0, 200, 14), scale(1.0f, 300, 16), glow(0, 150, 12) {}
};
std::vector<TaskbarIconAnim> g_taskbarAnims;
SpringValue g_startBtnHover(0, 200, 14);
SpringValue g_startBtnScale(1.0f, 300, 16);

bool g_selecting = false;
int g_selStartX = 0, g_selStartY = 0;
int g_selEndX = 0, g_selEndY = 0;

bool g_dragging = false;
int g_dragIconIdx = -1;
int g_dragOffsetX = 0, g_dragOffsetY = 0;
int g_dragCurrentX = 0, g_dragCurrentY = 0;
bool g_dragStarted = false;

// Volume / brightness state
float g_volumeLevel = 0.65f;
float g_brightnessLevel = 0.75f;

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
    int iconType;
    SpringValue hoverAnim;
    SpringValue selectAnim;
    DesktopIcon() : hoverAnim(0, 200, 14), selectAnim(0, 180, 13) {}
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
    add(L"Recycle Bin", L"shell:RecycleBinFolder", 0, 1, Color(255, 160, 160, 165), 1);
    add(L"Documents", L"__folder__", 0, 2, Color(255, 255, 213, 79), 5);
    add(L"Downloads", L"__folder__", 0, 3, Color(255, 96, 205, 255), 5);
    add(L"Pictures", L"__folder__", 0, 4, Color(255, 168, 130, 255), 5);
    add(L"Music", L"__folder__", 0, 5, Color(255, 255, 110, 140), 5);
}

// ============================================================================
//  EXPLORER FUNCTIONS
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
    win.scrollAnim = SpringValue(0, 120, 12);
    win.targetScroll = 0;
    win.selectedItem = -1;
    win.hoveredItem = -1;

    std::wstring searchPath = path;
    if (searchPath.back() != L'\\') searchPath += L'\\';
    searchPath += L'*';

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

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
    if (name == L"Documents") { SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, path); return path; }
    else if (name == L"Downloads") { SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path); return std::wstring(path) + L"\\Downloads"; }
    else if (name == L"Pictures") { SHGetFolderPathW(NULL, CSIDL_MYPICTURES, NULL, 0, path); return path; }
    else if (name == L"Music") { SHGetFolderPathW(NULL, CSIDL_MYMUSIC, NULL, 0, path); return path; }
    else if (name == L"Desktop") { SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, path); return path; }
    else if (name == L"Videos") { SHGetFolderPathW(NULL, CSIDL_MYVIDEO, NULL, 0, path); return path; }
    return L"";
}

ExplorerWindow* CreateExplorerWindow(const std::wstring& path, const std::wstring& title) {
    ExplorerWindow win;
    win.id = g_nextExplorerId++;
    win.title = title;
    win.currentPath = path;

    int offset = ((int)g_explorers.size() % 5) * 30;
    win.x = (ScreenW() - 920) / 2 + offset;
    win.y = (ScreenH() - 620) / 2 + offset;
    win.w = 920;
    win.h = 620;
    win.animAlpha = SpringValue(0, 200, 14);
    win.animAlpha.SetTarget(1.0f);
    win.animScale = SpringValue(0.92f, 250, 16);
    win.animScale.SetTarget(1.0f);
    win.visible = true;
    win.maximized = false;
    win.dragging = false;
    win.resizing = false;
    win.scrollOffset = 0;
    win.hoveredItem = -1;
    win.selectedItem = -1;
    win.historyIndex = -1;
    win.hoveredSidebarItem = -1;
    win.scrollAnim = SpringValue(0, 120, 12);
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
    for (auto& w : g_explorers) {
        if (w.id == id) {
            w.animAlpha.SetTarget(0.0f);
            w.animScale.SetTarget(0.94f);
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
    Color glowColor;
    bool pinned;
    bool running;
    bool active;
    RECT bounds;
};
std::vector<TaskbarApp> g_taskApps;

void InitTaskbarApps() {
    g_taskApps.clear();
    g_taskbarAnims.clear();
    auto add = [](const std::wstring& n, const std::wstring& e, const std::wstring& ico,
        Color c, Color glow, bool run, bool active) {
            TaskbarApp a;
            a.name = n; a.exec = e; a.iconLabel = ico; a.accentColor = c; a.glowColor = glow;
            a.pinned = true; a.running = run; a.active = active;
            memset(&a.bounds, 0, sizeof(RECT));
            g_taskApps.push_back(a);
            g_taskbarAnims.push_back(TaskbarIconAnim());
        };

    add(L"File Explorer", L"__thispc__", L"\xD83D\xDCC1", Color(255, 255, 200, 50), Color(255, 255, 200, 50), false, false);
    add(L"Microsoft Edge", L"msedge.exe", L"\xD83C\xDF10", Color(255, 0, 150, 255), Color(255, 0, 150, 255), false, false);
    add(L"Terminal", L"cmd.exe", L">_", Color(255, 52, 52, 56), Color(255, 120, 120, 130), false, false);
    add(L"Notepad", L"notepad.exe", L"\xD83D\xDCDD", Color(255, 100, 180, 255), Color(255, 100, 180, 255), false, false);
    add(L"Settings", L"ms-settings:", L"\x2699", Color(255, 90, 90, 100), Color(255, 130, 130, 145), false, false);
    add(L"Store", L"ms-windows-store:", L"\xD83D\xDECD", Color(255, 0, 120, 212), Color(255, 0, 120, 212), false, false);
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
    add(L"File Explorer", L"\xD83D\xDCC1", L"__thispc__", Color(255, 255, 200, 50));
    add(L"Edge", L"\xD83C\xDF10", L"msedge.exe", Color(255, 0, 150, 255));
    add(L"Terminal", L">_", L"cmd.exe", Color(255, 52, 52, 56));
    add(L"Notepad", L"\xD83D\xDCDD", L"notepad.exe", Color(255, 100, 180, 255));
    add(L"Settings", L"\x2699", L"ms-settings:", Color(255, 90, 90, 100));
    add(L"Calculator", L"\xD83D\xDDA9", L"calc.exe", Color(255, 52, 52, 56));
    add(L"Paint", L"\xD83C\xDFA8", L"mspaint.exe", Color(255, 255, 100, 100));
    add(L"Photos", L"\xD83D\xDDBC", L"ms-photos:", Color(255, 255, 180, 50));
    add(L"Mail", L"\x2709", L"notepad.exe", Color(255, 0, 120, 212));
    add(L"Calendar", L"\xD83D\xDCC5", L"notepad.exe", Color(255, 0, 120, 212));
    add(L"Maps", L"\xD83D\xDDFA", L"notepad.exe", Color(255, 100, 180, 255));
    add(L"Weather", L"\x2600", L"notepad.exe", Color(255, 255, 200, 50));
    add(L"Clock", L"\xD83D\xDD50", L"notepad.exe", Color(255, 100, 100, 105));
    add(L"Camera", L"\xD83D\xDCF7", L"notepad.exe", Color(255, 70, 70, 74));
    add(L"Store", L"\xD83D\xDECD", L"ms-windows-store:", Color(255, 0, 120, 212));
    add(L"Task Manager", L"\xD83D\xDCCA", L"taskmgr.exe", Color(255, 52, 52, 56));
    add(L"Control Panel", L"\xD83D\xDEE0", L"control.exe", Color(255, 0, 80, 160));
    add(L"Snip & Sketch", L"\x2702", L"notepad.exe", Color(255, 178, 87, 255));
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
    SpringValue alpha;
    SpringValue offsetY;
    SpringValue offsetX;
    bool alive;
    Notification() : alpha(0, 180, 12), offsetY(50, 200, 14), offsetX(0, 180, 13) {
        alpha.SetTarget(1.0f);
        offsetY.SetTarget(0.0f);
    }
};
std::vector<Notification> g_notifs;

void PushNotification(const std::wstring& title, const std::wstring& msg) {
    Notification n;
    n.title = title; n.message = msg;
    n.time = GetTickCount();
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
            d.iconColor = Color(255, 255, 213, 79);
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
//  GDI+ UTILITIES - Enhanced rendering
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

void FillRoundRectGradient(Graphics& g, int x, int y, int w, int h, int r, Color c1, Color c2, bool vertical = true) {
    GraphicsPath p; RoundedRectPath(p, x, y, w, h, r);
    if (vertical) {
        LinearGradientBrush br(Point(x, y), Point(x, y + h), c1, c2);
        g.FillPath(&br, &p);
    }
    else {
        LinearGradientBrush br(Point(x, y), Point(x + w, y), c1, c2);
        g.FillPath(&br, &p);
    }
}

void DrawShadow(Graphics& g, int x, int y, int w, int h, int r, int layers = 8, float intensity = 1.0f) {
    for (int i = layers; i >= 1; i--) {
        int sp = i * 2 + 1;
        float t = (float)(layers - i + 1) / (float)layers;
        int alpha = (int)(22.0f * t * t * intensity);
        FillRoundRectSolid(g, x - sp + 1, y - sp + 3, w + sp * 2, h + sp * 2, r + sp / 2,
            Color((BYTE)(std::min)(255, alpha), 0, 0, 0));
    }
}

void DrawGlowEffect(Graphics& g, int cx, int cy, int radius, Color glowColor, float intensity = 1.0f) {
    for (int i = 0; i < 6; i++) {
        int r = radius + i * 3;
        int alpha = (int)(intensity * 25.0f * (6 - i) / 6.0f);
        SolidBrush br(Color((BYTE)(std::min)(255, alpha), glowColor.GetR(), glowColor.GetG(), glowColor.GetB()));
        g.FillEllipse(&br, cx - r, cy - r, r * 2, r * 2);
    }
}

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
//  BEAUTIFUL ICON RENDERING - with shadows and depth
// ============================================================================
void DrawFolderIcon(Graphics& g, int x, int y, int size, Color color, float alpha = 1.0f) {
    BYTE a = (BYTE)(color.GetA() * alpha);
    int R = color.GetR(), G = color.GetG(), B = color.GetB();

    // Shadow
    FillRoundRectSolid(g, x + 3, y + (int)(size * 0.15f) + 2, size - 4, (int)(size * 0.72f), 3,
        Color((BYTE)(40 * alpha), 0, 0, 0));

    // Tab
    Color tabColor(a, (BYTE)(std::min)(255, R + 10), (BYTE)(std::min)(255, G + 10), (BYTE)(std::min)(255, B + 10));
    FillRoundRectSolid(g, x + 2, y + 2, (int)(size * 0.42f), (int)(size * 0.16f), 3, tabColor);

    // Body back
    Color bodyBack(a, (BYTE)R, (BYTE)G, (BYTE)B);
    FillRoundRectSolid(g, x + 2, y + (int)(size * 0.13f), size - 4, (int)(size * 0.73f), 4, bodyBack);

    // Front face - lighter with gradient
    Color frontTop((BYTE)(255 * alpha),
        (BYTE)(std::min)(255, R + 40), (BYTE)(std::min)(255, G + 40), (BYTE)(std::min)(255, B + 40));
    Color frontBot((BYTE)(255 * alpha),
        (BYTE)(std::min)(255, R + 20), (BYTE)(std::min)(255, G + 20), (BYTE)(std::min)(255, B + 20));
    FillRoundRectGradient(g, x + 2, y + (int)(size * 0.36f), size - 4, (int)(size * 0.50f), 4, frontTop, frontBot);

    // Highlight line at top of front face
    Pen highlightPen(Color((BYTE)(50 * alpha), 255, 255, 255), 1.0f);
    g.DrawLine(&highlightPen, x + 5, y + (int)(size * 0.37f), x + size - 7, y + (int)(size * 0.37f));
}

void DrawFileIcon(Graphics& g, int x, int y, int size, const std::wstring& ext, float alpha = 1.0f) {
    int pw = (int)(size * 0.68f);
    int ph = (int)(size * 0.84f);
    int px = x + (size - pw) / 2;
    int py = y + (size - ph) / 2;

    // Shadow
    FillRoundRectSolid(g, px + 2, py + 3, pw, ph, 2, Color((BYTE)(35 * alpha), 0, 0, 0));

    // Page
    FillRoundRectGradient(g, px, py, pw, ph, 3,
        Color((BYTE)(245 * alpha), 232, 232, 238),
        Color((BYTE)(240 * alpha), 215, 215, 225));

    // Corner fold
    int foldSize = (int)(pw * 0.28f);
    Point foldPts[3] = {
        Point(px + pw - foldSize, py),
        Point(px + pw, py + foldSize),
        Point(px + pw - foldSize, py + foldSize)
    };
    SolidBrush foldBr(Color((BYTE)(200 * alpha), 195, 195, 208));
    g.FillPolygon(&foldBr, foldPts, 3);

    // Fold edge highlight
    Pen foldEdge(Color((BYTE)(60 * alpha), 255, 255, 255), 0.5f);
    g.DrawLine(&foldEdge, Point(px + pw - foldSize, py), Point(px + pw, py + foldSize));

    if (!ext.empty()) {
        Color extColor((BYTE)(220 * alpha), 80, 80, 100);
        if (ext == L".txt" || ext == L".log") extColor = Color((BYTE)(220 * alpha), 80, 140, 200);
        else if (ext == L".exe" || ext == L".msi") extColor = Color((BYTE)(220 * alpha), 200, 90, 70);
        else if (ext == L".jpg" || ext == L".png" || ext == L".bmp" || ext == L".gif") extColor = Color((BYTE)(220 * alpha), 80, 180, 90);
        else if (ext == L".mp3" || ext == L".wav" || ext == L".flac") extColor = Color((BYTE)(220 * alpha), 190, 80, 200);
        else if (ext == L".zip" || ext == L".rar" || ext == L".7z") extColor = Color((BYTE)(220 * alpha), 200, 180, 40);
        else if (ext == L".doc" || ext == L".docx") extColor = Color((BYTE)(220 * alpha), 40, 90, 200);
        else if (ext == L".pdf") extColor = Color((BYTE)(220 * alpha), 210, 45, 45);
        else if (ext == L".cpp" || ext == L".h" || ext == L".py" || ext == L".js") extColor = Color((BYTE)(220 * alpha), 60, 160, 60);

        FillRoundRectSolid(g, px + 3, py + ph - (int)(ph * 0.33f), pw - 6, (int)(ph * 0.28f), 3, extColor);

        FontFamily ff(L"Segoe UI");
        Font extFont(&ff, (size > 30 ? 9.0f : 7.0f), FontStyleBold, UnitPixel);
        SolidBrush extBr(Color((BYTE)(255 * alpha), 255, 255, 255));
        StringFormat cfmt;
        cfmt.SetAlignment(StringAlignmentCenter);
        cfmt.SetLineAlignment(StringAlignmentCenter);
        std::wstring shortExt = ext.substr(0, 5);
        for (auto& ch : shortExt) ch = towupper(ch);
        RectF extRc((REAL)(px + 3), (REAL)(py + ph - (int)(ph * 0.33f)), (REAL)(pw - 6), (REAL)((int)(ph * 0.28f)));
        g.DrawString(shortExt.c_str(), -1, &extFont, extRc, &cfmt, &extBr);
    }
}

void DrawDriveIcon(Graphics& g, int x, int y, int size, float usedPercent, float alpha = 1.0f) {
    // Shadow
    FillRoundRectSolid(g, x + 3, y + (int)(size * 0.18f) + 2, size - 4, (int)(size * 0.68f), 5,
        Color((BYTE)(30 * alpha), 0, 0, 0));

    // Drive body - gradient
    FillRoundRectGradient(g, x + 2, y + (int)(size * 0.15f), size - 4, (int)(size * 0.68f), 5,
        Color((BYTE)(235 * alpha), 75, 75, 82),
        Color((BYTE)(235 * alpha), 58, 58, 65));

    // Top highlight
    Pen topHL(Color((BYTE)(30 * alpha), 255, 255, 255), 1.0f);
    g.DrawLine(&topHL, x + 6, y + (int)(size * 0.16f), x + size - 8, y + (int)(size * 0.16f));

    // Usage bar background
    int barX = x + 7;
    int barY = y + (int)(size * 0.56f);
    int barW = size - 14;
    int barH = 7;
    FillRoundRectSolid(g, barX, barY, barW, barH, 3, Color((BYTE)(200 * alpha), 35, 35, 40));

    // Usage bar fill with gradient
    int fillW = (int)(barW * usedPercent);
    if (fillW > 0) {
        Color barC1 = usedPercent > 0.9f ? Color((BYTE)(240 * alpha), 255, 80, 80) :
            usedPercent > 0.7f ? Color((BYTE)(240 * alpha), 255, 210, 50) :
            Color((BYTE)(240 * alpha), 96, 210, 255);
        Color barC2 = usedPercent > 0.9f ? Color((BYTE)(240 * alpha), 220, 50, 50) :
            usedPercent > 0.7f ? Color((BYTE)(240 * alpha), 230, 180, 30) :
            Color((BYTE)(240 * alpha), 50, 160, 230);
        FillRoundRectGradient(g, barX, barY, fillW, barH, 3, barC1, barC2, false);
    }

    // LED indicator
    Color ledColor = usedPercent > 0.9f ? Color((BYTE)(200 * alpha), 255, 60, 60) :
        Color((BYTE)(200 * alpha), 96, 220, 96);
    SolidBrush ledBr(ledColor);
    g.FillEllipse(&ledBr, x + size - 14, y + (int)(size * 0.22f), 5, 5);
}

void DrawPCIcon(Graphics& g, int x, int y, int size, float alpha = 1.0f) {
    int mw = (int)(size * 0.78f);
    int mh = (int)(size * 0.52f);
    int mx = x + (size - mw) / 2;
    int my = y + 2;

    // Monitor shadow
    FillRoundRectSolid(g, mx + 2, my + 3, mw, mh, 4, Color((BYTE)(35 * alpha), 0, 0, 0));

    // Monitor body
    FillRoundRectGradient(g, mx, my, mw, mh, 4,
        Color((BYTE)(235 * alpha), 50, 115, 195),
        Color((BYTE)(235 * alpha), 35, 85, 160));

    // Screen
    FillRoundRectGradient(g, mx + 3, my + 3, mw - 6, mh - 7, 2,
        Color((BYTE)(235 * alpha), 100, 175, 240),
        Color((BYTE)(235 * alpha), 65, 140, 210));

    // Screen shine
    FillRoundRectSolid(g, mx + 4, my + 4, (int)((mw - 8) * 0.4f), (int)((mh - 8) * 0.3f), 1,
        Color((BYTE)(30 * alpha), 255, 255, 255));

    // Stand
    SolidBrush standBr(Color((BYTE)(210 * alpha), 105, 105, 115));
    g.FillRectangle(&standBr, x + size / 2 - 3, my + mh, 6, (int)(size * 0.12f));
    FillRoundRectSolid(g, x + size / 2 - 9, my + mh + (int)(size * 0.10f), 18, 3, 1,
        Color((BYTE)(210 * alpha), 95, 95, 105));
}

void DrawRecycleBinIcon(Graphics& g, int x, int y, int size, float alpha = 1.0f) {
    int bw = (int)(size * 0.48f);
    int bh = (int)(size * 0.58f);
    int bx = x + (size - bw) / 2;
    int by = y + (int)(size * 0.22f);

    // Shadow
    FillRoundRectSolid(g, bx + 2, by + 3, bw, bh, 4, Color((BYTE)(30 * alpha), 0, 0, 0));

    // Body gradient
    FillRoundRectGradient(g, bx, by, bw, bh, 4,
        Color((BYTE)(225 * alpha), 140, 140, 150),
        Color((BYTE)(225 * alpha), 110, 110, 120));

    // Lid
    Pen lidPen(Color((BYTE)(235 * alpha), 160, 160, 170), 2.5f);
    g.DrawLine(&lidPen, bx - 3, by, bx + bw + 3, by);

    // Handle
    GraphicsPath handlePath;
    handlePath.AddArc(x + size / 2 - 6, by - 8, 12, 10, 180, 180);
    Pen handlePen(Color((BYTE)(235 * alpha), 155, 155, 165), 2.0f);
    g.DrawPath(&handlePen, &handlePath);

    // Lines
    Pen lp(Color((BYTE)(100 * alpha), 80, 80, 90), 1.2f);
    for (int li = 0; li < 3; li++) {
        int lx = bx + bw / 4 + li * bw / 4;
        g.DrawLine(&lp, lx, by + 7, lx, by + bh - 5);
    }
}

// ============================================================================
//  DRAW DESKTOP ICON
// ============================================================================
void DrawDesktopIconItem(Graphics& g, DesktopIcon& d, bool hovered, bool dragging_this) {
    int x = d.pixelX, y = d.pixelY;
    if (dragging_this) {
        x = g_dragCurrentX - g_dragOffsetX;
        y = g_dragCurrentY - g_dragOffsetY;
    }

    float ha = d.hoverAnim.current;
    float sa = d.selectAnim.current;

    // Background with smooth blending
    if (sa > 0.01f || ha > 0.01f) {
        BYTE bgA = (BYTE)((std::max)(sa * 55.0f, ha * 30.0f));
        BYTE bgR = (BYTE)(sa * 0 + (1 - sa) * 255);
        BYTE bgG = (BYTE)(sa * 90 + (1 - sa) * 255);
        BYTE bgB = (BYTE)(sa * 180 + (1 - sa) * 255);
        FillRoundRectSolid(g, x - 1, y - 1, DESKTOP_ICON_W + 2, DESKTOP_ICON_H + 2, 6,
            Color(bgA, bgR, bgG, bgB));

        if (sa > 0.1f) {
            DrawRoundRect(g, x - 1, y - 1, DESKTOP_ICON_W + 2, DESKTOP_ICON_H + 2, 6,
                Color((BYTE)(sa * 80), 96, 205, 255), 1.0f);
        }
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

    // Text shadow
    SolidBrush shadowBr(Color(180, 0, 0, 0));
    RectF shadowRc((REAL)(x - 3), (REAL)(y + DESKTOP_ICON_H - 28 + 1), (REAL)(DESKTOP_ICON_W + 6), 28.0f);
    g.DrawString(d.name.c_str(), -1, &nameFont, shadowRc, &cfmt, &shadowBr);

    SolidBrush textBr(W11::TextPrimary);
    RectF textRc((REAL)(x - 3), (REAL)(y + DESKTOP_ICON_H - 28), (REAL)(DESKTOP_ICON_W + 6), 28.0f);
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
    Pen borderPen(W11::SelectionBorder, 1.2f);
    g.DrawRectangle(&borderPen, x1, y1, w, h);
}

// ============================================================================
//  DRAW EXPLORER WINDOW - Windows 11 File Explorer
// ============================================================================
void DrawExplorerWindow(Graphics& g, ExplorerWindow& win) {
    float a = win.animAlpha.current;
    float sc = win.animScale.current;
    if (a <= 0.01f) return;

    int wx = win.x, wy = win.y, ww = win.w, wh = win.h;
    bool isActive = (win.id == g_activeExplorerId);

    // Scale transform for open/close animation
    if (fabsf(sc - 1.0f) > 0.002f) {
        int cx = wx + ww / 2, cy = wy + wh / 2;
        Matrix mtx;
        mtx.Translate((REAL)cx, (REAL)cy);
        mtx.Scale(sc, sc);
        mtx.Translate((REAL)-cx, (REAL)-cy);
        g.SetTransform(&mtx);
    }

    DrawShadow(g, wx, wy, ww, wh, 8, isActive ? 12 : 6, a);

    // Window background
    FillRoundRectSolid(g, wx, wy, ww, wh, 8, Color((BYTE)(252 * a), 25, 25, 27));
    DrawRoundRect(g, wx, wy, ww, wh, 8,
        isActive ? Color((BYTE)(55 * a), 255, 255, 255) : Color((BYTE)(25 * a), 255, 255, 255), 1.0f);

    FontFamily ff(L"Segoe UI");
    StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat cfmt; cfmt.SetAlignment(StringAlignmentCenter); cfmt.SetLineAlignment(StringAlignmentCenter);

    // ---- Title bar (34px) ----
    int titleH = 34;

    DrawFolderIcon(g, wx + 12, wy + 7, 20, Color(255, 255, 213, 79), a);

    Font titleFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    SolidBrush titleBr(Color((BYTE)(220 * a), 220, 220, 230));
    RectF titleRc((REAL)(wx + 38), (REAL)wy, 300.0f, (REAL)titleH);
    g.DrawString(win.title.c_str(), -1, &titleFont, titleRc, &lfmt, &titleBr);

    // Window buttons
    int btnW = 46, btnH = titleH;
    int btnX = wx + ww - btnW * 3;

    // Minimize
    {
        bool hov = (win.hoveredTitleBtn == 1);
        if (hov) FillRoundRectSolid(g, btnX, wy, btnW, btnH, 0, Color((BYTE)(140 * a), 60, 60, 65));
        Pen p(Color((BYTE)(200 * a), 200, 200, 210), 1.0f);
        g.DrawLine(&p, btnX + btnW / 2 - 5, wy + btnH / 2, btnX + btnW / 2 + 5, wy + btnH / 2);
    }
    btnX += btnW;
    {
        bool hov = (win.hoveredTitleBtn == 2);
        if (hov) FillRoundRectSolid(g, btnX, wy, btnW, btnH, 0, Color((BYTE)(140 * a), 60, 60, 65));
        Pen p(Color((BYTE)(200 * a), 200, 200, 210), 1.0f);
        g.DrawRectangle(&p, btnX + btnW / 2 - 5, wy + btnH / 2 - 5, 10, 10);
    }
    btnX += btnW;
    {
        bool hov = (win.hoveredTitleBtn == 3);
        if (hov) FillRoundRectSolid(g, btnX, wy, btnW, btnH, 0, Color((BYTE)(210 * a), 196, 43, 28));
        Pen p(Color((BYTE)(200 * a), 200, 200, 210), 1.3f);
        g.DrawLine(&p, btnX + btnW / 2 - 5, wy + btnH / 2 - 5, btnX + btnW / 2 + 5, wy + btnH / 2 + 5);
        g.DrawLine(&p, btnX + btnW / 2 + 5, wy + btnH / 2 - 5, btnX + btnW / 2 - 5, wy + btnH / 2 + 5);
    }

    // ---- Toolbar (40px) ----
    int toolY = wy + titleH;
    int toolH = 40;
    SolidBrush toolBg(Color((BYTE)(250 * a), 30, 30, 33));
    g.FillRectangle(&toolBg, wx, toolY, ww, toolH);

    // Navigation buttons
    int navX = wx + 12;
    int navBtnSize = 28;

    auto drawNavBtn = [&](bool enabled, int type) {
        Color c = enabled ? Color((BYTE)(200 * a), 200, 200, 210) : Color((BYTE)(45 * a), 100, 100, 110);
        Pen p(c, 1.5f);
        int cx2 = navX + navBtnSize / 2, cy2 = toolY + toolH / 2;
        if (type == 0) { // Back
            g.DrawLine(&p, cx2 + 3, cy2 - 5, cx2 - 3, cy2);
            g.DrawLine(&p, cx2 - 3, cy2, cx2 + 3, cy2 + 5);
        }
        else if (type == 1) { // Forward
            g.DrawLine(&p, cx2 - 3, cy2 - 5, cx2 + 3, cy2);
            g.DrawLine(&p, cx2 + 3, cy2, cx2 - 3, cy2 + 5);
        }
        else { // Up
            g.DrawLine(&p, cx2 - 5, cy2 + 2, cx2, cy2 - 4);
            g.DrawLine(&p, cx2, cy2 - 4, cx2 + 5, cy2 + 2);
        }
        };

    drawNavBtn(win.historyIndex > 0, 0);
    navX += navBtnSize + 4;
    drawNavBtn(win.historyIndex < (int)win.pathHistory.size() - 1, 1);
    navX += navBtnSize + 4;
    drawNavBtn(!win.currentPath.empty(), 2);
    navX += navBtnSize + 14;

    // Address bar
    int addrX = navX, addrW = ww - (navX - wx) - 16, addrH = 28;
    int addrY = toolY + (toolH - addrH) / 2;

    FillRoundRectSolid(g, addrX, addrY, addrW, addrH, 6, Color((BYTE)(250 * a), 46, 46, 50));
    DrawRoundRect(g, addrX, addrY, addrW, addrH, 6, Color((BYTE)(35 * a), 255, 255, 255), 1.0f);
    // Bottom accent line
    Pen accentLine(Color((BYTE)(100 * a), 96, 205, 255), 1.5f);
    g.DrawLine(&accentLine, addrX + 6, addrY + addrH - 1, addrX + addrW - 6, addrY + addrH - 1);

    Font addrFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    SolidBrush addrBr(Color((BYTE)(200 * a), 200, 200, 210));
    std::wstring addrText = win.currentPath.empty() ? L"This PC" : win.currentPath;
    RectF addrRc((REAL)(addrX + 12), (REAL)addrY, (REAL)(addrW - 24), (REAL)addrH);
    cfmt.SetAlignment(StringAlignmentNear);
    g.DrawString(addrText.c_str(), -1, &addrFont, addrRc, &cfmt, &addrBr);
    cfmt.SetAlignment(StringAlignmentCenter);

    // ---- Sidebar + Content ----
    int sidebarW = 200;
    int contentX = wx + sidebarW;
    int contentY = toolY + toolH;
    int contentW = ww - sidebarW;
    int contentH = wh - titleH - toolH;

    SolidBrush sidebarBg(Color((BYTE)(250 * a), 30, 30, 33));
    g.FillRectangle(&sidebarBg, wx, contentY, sidebarW, contentH);

    Pen sepPen(Color((BYTE)(20 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&sepPen, contentX, contentY, contentX, wy + wh);
    g.DrawLine(&sepPen, wx, contentY, wx + ww, contentY);

    // Sidebar items
    struct SideItem { std::wstring label; int icon; };
    SideItem sideItems[] = {
        { L"Desktop", 0 }, { L"Downloads", 1 }, { L"Documents", 2 },
        { L"Pictures", 3 }, { L"Music", 4 }, { L"Videos", 5 }, { L"This PC", 6 },
    };

    Font sideFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    Font headerFont(&ff, 11.0f, FontStyleBold, UnitPixel);
    int sideY = contentY + 10;

    SolidBrush headerBr(Color((BYTE)(100 * a), 150, 150, 160));
    RectF headerRc((REAL)(wx + 16), (REAL)sideY, (REAL)(sidebarW - 24), 18.0f);
    g.DrawString(L"Quick access", -1, &headerFont, headerRc, &lfmt, &headerBr);
    sideY += 28;

    for (int i = 0; i < 7; i++) {
        bool hov = (win.hoveredSidebarItem == i);
        int itemH = 30;

        if (hov) {
            FillRoundRectSolid(g, wx + 6, sideY, sidebarW - 12, itemH, 5,
                Color((BYTE)(200 * a), 48, 48, 52));
        }

        if (i < 6) {
            Color fc(255, 255, 213, 79);
            if (i == 1) fc = Color(255, 96, 205, 255);
            else if (i == 3) fc = Color(255, 168, 130, 255);
            else if (i == 4) fc = Color(255, 255, 110, 140);
            else if (i == 5) fc = Color(255, 100, 200, 130);
            DrawFolderIcon(g, wx + 16, sideY + 3, 22, fc, a);
        }
        else {
            DrawPCIcon(g, wx + 16, sideY + 3, 22, a);
        }

        SolidBrush sideBr(Color((BYTE)(220 * a), 215, 215, 225));
        RectF sideRc((REAL)(wx + 44), (REAL)sideY, (REAL)(sidebarW - 54), (REAL)itemH);
        g.DrawString(sideItems[i].label.c_str(), -1, &sideFont, sideRc, &lfmt, &sideBr);

        if (i == 5) {
            sideY += itemH + 6;
            Pen sp(Color((BYTE)(18 * a), 255, 255, 255), 1.0f);
            g.DrawLine(&sp, wx + 14, sideY, wx + sidebarW - 14, sideY);
            sideY += 10;
            SolidBrush hdr2(Color((BYTE)(100 * a), 150, 150, 160));
            RectF hdr2Rc((REAL)(wx + 16), (REAL)sideY, (REAL)(sidebarW - 24), 18.0f);
            g.DrawString(L"This PC", -1, &headerFont, hdr2Rc, &lfmt, &hdr2);
            sideY += 26;
        }
        else {
            sideY += itemH;
        }
    }

    // ---- Content area ----
    g.SetClip(Rect(contentX + 1, contentY + 1, contentW - 2, contentH - 2));
    SolidBrush contentBg(Color((BYTE)(250 * a), 25, 25, 27));
    g.FillRectangle(&contentBg, contentX, contentY, contentW, contentH);

    int scrollOff = (int)win.scrollAnim.current;
    StringFormat rfmt; rfmt.SetAlignment(StringAlignmentFar); rfmt.SetLineAlignment(StringAlignmentCenter);

    if (win.currentPath.empty()) {
        // ====== THIS PC VIEW ======
        Font secFont(&ff, 13.0f, FontStyleBold, UnitPixel);
        SolidBrush secBr(Color((BYTE)(170 * a), 200, 200, 210));

        int cy = contentY + 18 - scrollOff;
        RectF secRc((REAL)(contentX + 22), (REAL)cy, 200.0f, 24.0f);
        g.DrawString(L"Devices and drives", -1, &secFont, secRc, &lfmt, &secBr);
        cy += 38;

        int tileW = 242, tileH = 72;
        int cols = (std::max)(1, (contentW - 44) / (tileW + 10));

        for (int i = 0; i < (int)win.items.size(); i++) {
            const FileItem& fi = win.items[i];
            int col = i % cols, row = i / cols;
            int tx = contentX + 22 + col * (tileW + 10);
            int ty = cy + row * (tileH + 10);

            bool hov = (win.hoveredItem == i);
            bool sel = (win.selectedItem == i);

            Color bg = sel ? Color((BYTE)(235 * a), 0, 65, 135) :
                hov ? Color((BYTE)(225 * a), 48, 48, 52) :
                Color((BYTE)(210 * a), 38, 38, 42);

            FillRoundRectSolid(g, tx, ty, tileW, tileH, 8, bg);
            DrawRoundRect(g, tx, ty, tileW, tileH, 8, Color((BYTE)(18 * a), 255, 255, 255), 1.0f);

            float usedPct = fi.totalSpace > 0 ? (float)(fi.totalSpace - fi.freeSpace) / (float)fi.totalSpace : 0;
            DrawDriveIcon(g, tx + 10, ty + 10, 46, usedPct, a);

            Font driveFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
            SolidBrush driveBr(Color((BYTE)(240 * a), 240, 240, 250));
            RectF driveRc((REAL)(tx + 62), (REAL)(ty + 8), (REAL)(tileW - 72), 20.0f);
            cfmt.SetAlignment(StringAlignmentNear);
            g.DrawString(fi.name.c_str(), -1, &driveFont, driveRc, &cfmt, &driveBr);

            if (fi.totalSpace > 0) {
                std::wstring spaceStr = FormatSize(fi.freeSpace) + L" free of " + FormatSize(fi.totalSpace);
                Font spaceFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
                SolidBrush spaceBr(Color((BYTE)(130 * a), 155, 155, 165));
                RectF spaceRc((REAL)(tx + 62), (REAL)(ty + 28), (REAL)(tileW - 72), 16.0f);
                g.DrawString(spaceStr.c_str(), -1, &spaceFont, spaceRc, &cfmt, &spaceBr);

                int barX2 = tx + 62, barY2 = ty + 50, barW2 = tileW - 78, barH2 = 6;
                FillRoundRectSolid(g, barX2, barY2, barW2, barH2, 3, Color((BYTE)(140 * a), 50, 50, 55));
                int fillW2 = (int)(barW2 * usedPct);
                if (fillW2 > 0) {
                    Color bc1 = usedPct > 0.9f ? Color((BYTE)(235 * a), 255, 80, 80) :
                        usedPct > 0.7f ? Color((BYTE)(235 * a), 255, 210, 50) :
                        Color((BYTE)(235 * a), 96, 210, 255);
                    Color bc2 = usedPct > 0.9f ? Color((BYTE)(235 * a), 220, 50, 50) :
                        usedPct > 0.7f ? Color((BYTE)(235 * a), 230, 180, 30) :
                        Color((BYTE)(235 * a), 50, 160, 230);
                    FillRoundRectGradient(g, barX2, barY2, fillW2, barH2, 3, bc1, bc2, false);
                }
            }
            cfmt.SetAlignment(StringAlignmentCenter);
        }

        // Status bar
        Font countFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
        SolidBrush countBr(Color((BYTE)(90 * a), 140, 140, 150));
        wchar_t countStr[64];
        swprintf_s(countStr, L"%d items", (int)win.items.size());
        RectF countRc((REAL)(contentX + 22), (REAL)(wy + wh - 28), (REAL)(contentW - 44), 20.0f);
        lfmt.SetLineAlignment(StringAlignmentCenter);
        g.DrawString(countStr, -1, &countFont, countRc, &lfmt, &countBr);
    }
    else {
        // ====== DIRECTORY VIEW ======
        int itemH = 32;
        int headerY = contentY + 4;
        Font hdrFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
        SolidBrush hdrBr(Color((BYTE)(130 * a), 160, 160, 170));

        RectF nameHdr((REAL)(contentX + 48), (REAL)headerY, 300.0f, 24.0f);
        g.DrawString(L"Name", -1, &hdrFont, nameHdr, &lfmt, &hdrBr);
        RectF sizeHdr((REAL)(contentX + contentW - 250), (REAL)headerY, 100.0f, 24.0f);
        g.DrawString(L"Size", -1, &hdrFont, sizeHdr, &lfmt, &hdrBr);
        RectF typeHdr((REAL)(contentX + contentW - 140), (REAL)headerY, 120.0f, 24.0f);
        g.DrawString(L"Type", -1, &hdrFont, typeHdr, &lfmt, &hdrBr);

        Pen hdrLine(Color((BYTE)(18 * a), 255, 255, 255), 1.0f);
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
                FillRoundRectSolid(g, contentX + 4, iy, contentW - 8, itemH - 1, 4,
                    Color((BYTE)(225 * a), 0, 65, 135));
            }
            else if (hov) {
                FillRoundRectSolid(g, contentX + 4, iy, contentW - 8, itemH - 1, 4,
                    Color((BYTE)(200 * a), 46, 46, 50));
            }

            if (fi.isDirectory) {
                if (fi.name == L"..") {
                    Pen upPen(Color((BYTE)(200 * a), 200, 200, 210), 1.5f);
                    int cx2 = contentX + 28, cy2 = iy + itemH / 2;
                    g.DrawLine(&upPen, cx2, cy2 + 4, cx2, cy2 - 4);
                    g.DrawLine(&upPen, cx2 - 4, cy2, cx2, cy2 - 4);
                    g.DrawLine(&upPen, cx2 + 4, cy2, cx2, cy2 - 4);
                }
                else {
                    DrawFolderIcon(g, contentX + 16, iy + 3, 24, Color(255, 255, 213, 79), a);
                }
            }
            else {
                std::wstring ext;
                size_t dotPos = fi.name.find_last_of(L'.');
                if (dotPos != std::wstring::npos) ext = fi.name.substr(dotPos);
                for (auto& ch : ext) ch = towlower(ch);
                DrawFileIcon(g, contentX + 16, iy + 3, 24, ext, a);
            }

            SolidBrush nameBr(Color((BYTE)(240 * a), 240, 240, 250));
            RectF nameRc((REAL)(contentX + 48), (REAL)iy, (REAL)(contentW - 310), (REAL)itemH);
            g.DrawString(fi.name.c_str(), -1, &itemFont, nameRc, &lfmt, &nameBr);

            if (!fi.isDirectory && fi.fileSize > 0) {
                SolidBrush sizeBr(Color((BYTE)(150 * a), 160, 160, 170));
                RectF sizeRc((REAL)(contentX + contentW - 250), (REAL)iy, 100.0f, (REAL)itemH);
                g.DrawString(FormatSize(fi.fileSize).c_str(), -1, &smallFont, sizeRc, &lfmt, &sizeBr);
            }

            std::wstring typeStr;
            if (fi.isDirectory) typeStr = L"File folder";
            else {
                size_t dotPos = fi.name.find_last_of(L'.');
                if (dotPos != std::wstring::npos) {
                    std::wstring ext2 = fi.name.substr(dotPos);
                    for (auto& ch : ext2) ch = towupper(ch);
                    typeStr = ext2.substr(1) + L" File";
                }
                else typeStr = L"File";
            }
            SolidBrush typeBr(Color((BYTE)(120 * a), 150, 150, 160));
            RectF typeRc((REAL)(contentX + contentW - 140), (REAL)iy, 120.0f, (REAL)itemH);
            g.DrawString(typeStr.c_str(), -1, &smallFont, typeRc, &lfmt, &typeBr);
        }

        // Status bar
        int dirCount = 0, fileCount = 0;
        for (auto& fi : win.items) {
            if (fi.name == L"..") continue;
            if (fi.isDirectory) dirCount++; else fileCount++;
        }
        Font countFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
        SolidBrush countBr(Color((BYTE)(90 * a), 140, 140, 150));
        wchar_t countStr[128];
        swprintf_s(countStr, L"%d folders, %d files", dirCount, fileCount);
        RectF countRc((REAL)(contentX + 14), (REAL)(wy + wh - 28), (REAL)(contentW - 28), 20.0f);
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

            FillRoundRectSolid(g, sbX, sbY, 6, sbH, 3, Color((BYTE)(30 * a), 255, 255, 255));
            FillRoundRectSolid(g, sbX, thumbY, 6, thumbH, 3, Color((BYTE)(90 * a), 140, 140, 150));
        }
    }

    g.ResetClip();

    // Reset transform
    if (fabsf(sc - 1.0f) > 0.002f) {
        g.ResetTransform();
    }
}

// ============================================================================
//  TASKBAR - World-class glassmorphism design
// ============================================================================
void DrawTaskbar(Graphics& g, int sw, int sh) {
    int barY = sh - TASKBAR_HEIGHT;

    // Multi-layer background for depth
    // Base layer
    SolidBrush baseBr(Color(240, 20, 20, 22));
    g.FillRectangle(&baseBr, 0, barY, sw, TASKBAR_HEIGHT);

    // Glassmorphism gradient overlay
    LinearGradientBrush glassBg(Point(0, barY), Point(0, sh),
        Color(20, 60, 60, 65), Color(8, 30, 30, 32));
    g.FillRectangle(&glassBg, 0, barY, sw, TASKBAR_HEIGHT);

    // Top edge - subtle bright line simulating glass edge
    LinearGradientBrush topGlow(Point(sw / 2 - 200, barY), Point(sw / 2 + 200, barY),
        Color(0, 255, 255, 255), Color(50, 255, 255, 255));
    Pen topLine(Color(40, 255, 255, 255), 1.0f);
    g.DrawLine(&topLine, 0, barY, sw, barY);

    // Accent glow under top edge (centered, subtle)
    for (int i = 0; i < 3; i++) {
        int gw = 300 - i * 60;
        int gx = sw / 2 - gw / 2;
        SolidBrush gb(Color((BYTE)(8 - i * 2), 96, 205, 255));
        g.FillRectangle(&gb, gx, barY + 1, gw, 2);
    }

    FontFamily ff(L"Segoe UI");
    FontFamily ffEmoji(L"Segoe UI Emoji");

    int iconCount = (int)g_taskApps.size();
    int totalW = (iconCount + 3) * (TASKBAR_ICON_SIZE + 6);
    int startX = (sw - totalW) / 2;
    int iconY = barY + (TASKBAR_HEIGHT - TASKBAR_ICON_SIZE) / 2;

    // ======== START BUTTON ========
    {
        int bx = startX;
        int by = iconY;
        bool startHov = (g_hoveredTaskbarIcon == -10);
        float ha = g_startBtnHover.current;
        float sa = g_startBtnScale.current;

        // Hover background with glow
        if (ha > 0.01f || g_startMenuOpen) {
            float intensity = g_startMenuOpen ? 0.5f : ha * 0.35f;
            FillRoundRectSolid(g, bx - 1, by - 1, TASKBAR_ICON_SIZE + 2, TASKBAR_ICON_SIZE + 2, 8,
                Color((BYTE)(intensity * 255), 55, 55, 58));
        }

        // Windows 11 logo with gradient
        int logoSize = 15;
        int lx = bx + (TASKBAR_ICON_SIZE - logoSize) / 2;
        int ly = by + (TASKBAR_ICON_SIZE - logoSize) / 2;
        int gap = 2, sq = (logoSize - gap) / 2;

        Color logoColor = g_startMenuOpen ? Color(255, 255, 255, 255) :
            Color(255, 96, 205, 255);
        SolidBrush logoBr(logoColor);

        // Rounded squares for logo
        FillRoundRectSolid(g, lx, ly, sq, sq, 2, logoColor);
        FillRoundRectSolid(g, lx + sq + gap, ly, sq, sq, 2, logoColor);
        FillRoundRectSolid(g, lx, ly + sq + gap, sq, sq, 2, logoColor);
        FillRoundRectSolid(g, lx + sq + gap, ly + sq + gap, sq, sq, 2, logoColor);

        // Subtle glow when open
        if (g_startMenuOpen) {
            int gcx = bx + TASKBAR_ICON_SIZE / 2;
            int gcy = by + TASKBAR_ICON_SIZE / 2;
            DrawGlowEffect(g, gcx, gcy, 8, Color(255, 96, 205, 255), 0.3f);
        }
    }

    // ======== SEARCH ========
    {
        int bx = startX + (TASKBAR_ICON_SIZE + 6);
        int by = iconY;
        bool hov = (g_hoveredTaskbarIcon == -11);
        if (hov) FillRoundRectSolid(g, bx, by, TASKBAR_ICON_SIZE, TASKBAR_ICON_SIZE, 8, Color(80, 55, 55, 58));

        Pen sp(Color(200, 190, 190, 200), 1.6f);
        int cx2 = bx + TASKBAR_ICON_SIZE / 2 - 2, cy2 = by + TASKBAR_ICON_SIZE / 2 - 2;
        g.DrawEllipse(&sp, cx2 - 6, cy2 - 6, 12, 12);
        g.DrawLine(&sp, cx2 + 4, cy2 + 4, cx2 + 7, cy2 + 7);
    }

    // ======== TASK VIEW ========
    {
        int bx = startX + 2 * (TASKBAR_ICON_SIZE + 6);
        int by = iconY;
        bool hov = (g_hoveredTaskbarIcon == -12);
        if (hov) FillRoundRectSolid(g, bx, by, TASKBAR_ICON_SIZE, TASKBAR_ICON_SIZE, 8, Color(80, 55, 55, 58));

        Pen tp(Color(200, 190, 190, 200), 1.2f);
        g.DrawRectangle(&tp, bx + 11, by + 12, 9, 7);
        g.DrawRectangle(&tp, bx + 19, by + 19, 9, 7);
    }

    // ======== APP ICONS ========
    int appsStartX = startX + 3 * (TASKBAR_ICON_SIZE + 6) + 10;
    for (int i = 0; i < iconCount; i++) {
        int ix = appsStartX + i * (TASKBAR_ICON_SIZE + 6);
        bool hov = (g_hoveredTaskbarIcon == i);
        float ha = g_taskbarAnims[i].hover.current;
        float sca = g_taskbarAnims[i].scale.current;

        // Hover background with smooth blend
        if (ha > 0.01f || g_taskApps[i].active) {
            float intensity = (std::max)(ha * 0.4f, g_taskApps[i].active ? 0.35f : 0.0f);
            FillRoundRectSolid(g, ix - 1, iconY - 1, TASKBAR_ICON_SIZE + 2, TASKBAR_ICON_SIZE + 2, 8,
                Color((BYTE)(intensity * 255), 58, 58, 62));
        }

        // Icon circle background with gradient
        int icoInner = 26;
        int icoX = ix + (TASKBAR_ICON_SIZE - icoInner) / 2;
        int icoY = iconY + (TASKBAR_ICON_SIZE - icoInner) / 2;

        Color c1 = g_taskApps[i].accentColor;
        Color c2(255,
            (BYTE)(std::max)(0, (int)c1.GetR() - 25),
            (BYTE)(std::max)(0, (int)c1.GetG() - 25),
            (BYTE)(std::max)(0, (int)c1.GetB() - 25));
        FillRoundRectGradient(g, icoX, icoY, icoInner, icoInner, 7, c1, c2);

        // Subtle top highlight on icon
        Pen icoHL(Color(35, 255, 255, 255), 0.8f);
        g.DrawLine(&icoHL, icoX + 4, icoY + 1, icoX + icoInner - 4, icoY + 1);

        // Icon label
        Font icoFont(&ff, 10.0f, FontStyleBold, UnitPixel);
        SolidBrush icoTextBr(Color(255, 255, 255, 255));
        StringFormat icoCfmt; icoCfmt.SetAlignment(StringAlignmentCenter); icoCfmt.SetLineAlignment(StringAlignmentCenter);
        RectF icoRc((REAL)icoX, (REAL)icoY, (REAL)icoInner, (REAL)icoInner);
        g.DrawString(g_taskApps[i].iconLabel.c_str(), -1, &icoFont, icoRc, &icoCfmt, &icoTextBr);

        // Running indicator - beautiful pill with glow
        if (g_taskApps[i].running) {
            int dotW = g_taskApps[i].active ? 20 : 6;
            int dotH = 3;
            int dotX = ix + (TASKBAR_ICON_SIZE - dotW) / 2;
            int dotY = iconY + TASKBAR_ICON_SIZE + 1;

            // Glow under indicator
            for (int gl = 0; gl < 3; gl++) {
                int glW = dotW + gl * 4;
                int glX = ix + (TASKBAR_ICON_SIZE - glW) / 2;
                SolidBrush glBr(Color((BYTE)(20 - gl * 6), 96, 205, 255));
                g.FillRectangle(&glBr, glX, dotY - gl, glW, dotH + gl * 2);
            }

            FillRoundRectSolid(g, dotX, dotY, dotW, dotH, 1, W11::TaskbarIndicator);
        }

        // Hover glow effect
        if (ha > 0.3f) {
            int gcx = ix + TASKBAR_ICON_SIZE / 2;
            int gcy = iconY + TASKBAR_ICON_SIZE;
            DrawGlowEffect(g, gcx, gcy, 4, g_taskApps[i].glowColor, ha * 0.15f);
        }

        SetRect(&g_taskApps[i].bounds, ix, iconY, ix + TASKBAR_ICON_SIZE, iconY + TASKBAR_ICON_SIZE);
    }

    // ======== SYSTEM TRAY - Premium design ========
    int trayX = sw - 240;

    // Tray background pill
    FillRoundRectSolid(g, trayX - 6, barY + 8, 234, TASKBAR_HEIGHT - 16, 8, Color(25, 255, 255, 255));

    // Hidden icons chevron
    {
        int cx2 = trayX + 4;
        int cy2 = barY + TASKBAR_HEIGHT / 2;
        Pen chev(Color(80, 190, 190, 200), 1.3f);
        g.DrawLine(&chev, cx2 - 2, cy2 - 3, cx2 + 1, cy2);
        g.DrawLine(&chev, cx2 + 1, cy2, cx2 - 2, cy2 + 3);
    }

    // ---- WiFi Icon - Beautiful animated design ----
    {
        int wbx = trayX + 24;
        int wby = barY + (TASKBAR_HEIGHT - 22) / 2;
        bool wifiHov = (g_hoveredTaskbarIcon == -20);

        if (wifiHov || g_wifiPanelOpen) {
            FillRoundRectSolid(g, wbx - 5, wby - 5, 32, 32, 6, Color(70, 96, 205, 255));
        }

        int wcx = wbx + 10;
        int wcy = wby + 17;

        // WiFi waves - smooth anti-aliased arcs with gradient opacity
        for (int arc = 0; arc < 3; arc++) {
            int r = 5 + arc * 5;
            float waveAlpha = 1.0f - arc * 0.15f;
            // Animated subtle pulse
            float pulse = 1.0f + 0.03f * sinf(g_time * 2.5f + arc * 0.8f);
            int pr = (int)(r * pulse);

            Pen wPen(Color((BYTE)(210 * waveAlpha), 200, 220, 255), 2.0f);
            wPen.SetStartCap(LineCapRound);
            wPen.SetEndCap(LineCapRound);
            g.DrawArc(&wPen, wcx - pr, wcy - pr, pr * 2, pr * 2, 225, 90);
        }

        // Center dot with glow
        SolidBrush dotGlow(Color(40, 96, 205, 255));
        g.FillEllipse(&dotGlow, wcx - 4, wcy - 4, 8, 8);
        SolidBrush dotBr(Color(240, 220, 230, 255));
        g.FillEllipse(&dotBr, wcx - 2, wcy - 2, 4, 4);
    }

    // ---- Volume Icon - Arc design ----
    {
        int vx = trayX + 68;
        int vy = barY + (TASKBAR_HEIGHT - 20) / 2;

        // Speaker body
        Pen vPen(Color(200, 195, 200, 215), 1.5f);
        vPen.SetStartCap(LineCapRound);
        vPen.SetEndCap(LineCapRound);

        // Speaker shape
        Point speaker[6] = {
            Point(vx + 3, vy + 7), Point(vx + 6, vy + 7),
            Point(vx + 11, vy + 3), Point(vx + 11, vy + 17),
            Point(vx + 6, vy + 13), Point(vx + 3, vy + 13)
        };
        g.DrawPolygon(&vPen, speaker, 6);

        // Sound waves
        for (int w = 0; w < 2; w++) {
            int wr = 4 + w * 4;
            float waveA = 0.7f - w * 0.2f;
            Pen wp(Color((BYTE)(200 * waveA), 195, 200, 215), 1.3f);
            wp.SetStartCap(LineCapRound);
            wp.SetEndCap(LineCapRound);
            g.DrawArc(&wp, vx + 10, vy + 10 - wr, wr * 2, wr * 2, -50, 100);
        }
    }

    // ---- Battery Icon - Modern with percentage ----
    {
        int bx = trayX + 104;
        int by = barY + (TASKBAR_HEIGHT - 14) / 2;

        // Battery outline
        Pen bPen(Color(190, 195, 200, 215), 1.2f);
        DrawRoundRect(g, bx, by, 24, 14, 3, Color(190, 195, 200, 215), 1.2f);

        // Battery tip
        FillRoundRectSolid(g, bx + 24, by + 3, 3, 8, 1, Color(190, 195, 200, 215));

        // Fill with gradient
        int fillW = (int)(18 * 0.82f); // 82% battery
        Color battC1(255, 108, 210, 100);
        Color battC2(255, 80, 180, 70);
        FillRoundRectGradient(g, bx + 3, by + 3, fillW, 8, 1, battC1, battC2, false);

        // Subtle shine
        SolidBrush shine(Color(25, 255, 255, 255));
        g.FillRectangle(&shine, bx + 3, by + 3, fillW, 3);
    }

    // ---- Clock - Beautiful typography ----
    time_t now = time(NULL);
    struct tm ti;
    localtime_s(&ti, &now);
    wchar_t timeStr[32], dateStr[32];
    wcsftime(timeStr, 32, L"%H:%M", &ti);
    wcsftime(dateStr, 32, L"%d.%m.%Y", &ti);

    Font clockFont(&ff, 12.0f, FontStyleSemiBold, UnitPixel);
    Font dateFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
    SolidBrush clockBr(Color(255, 245, 245, 250));
    SolidBrush dateBr(Color(180, 190, 190, 200));
    StringFormat rfmt; rfmt.SetAlignment(StringAlignmentFar); rfmt.SetLineAlignment(StringAlignmentCenter);

    RectF clockRc((REAL)(sw - 108), (REAL)barY, 96.0f, (REAL)(TASKBAR_HEIGHT / 2 + 2));
    g.DrawString(timeStr, -1, &clockFont, clockRc, &rfmt, &clockBr);
    RectF dateRc((REAL)(sw - 108), (REAL)(barY + TASKBAR_HEIGHT / 2 - 3), 96.0f, (REAL)(TASKBAR_HEIGHT / 2));
    g.DrawString(dateStr, -1, &dateFont, dateRc, &rfmt, &dateBr);

    // Notification indicator with glow
    if (!g_notifs.empty()) {
        int nix = sw - 14, niy = barY + TASKBAR_HEIGHT / 2 - 3;
        DrawGlowEffect(g, nix + 3, niy + 3, 3, W11::AccentLight, 0.6f);
        SolidBrush notifDot(W11::AccentLight);
        g.FillEllipse(&notifDot, nix, niy, 6, 6);
    }
}

// ============================================================================
//  START MENU - Frosted glass with staggered reveal
// ============================================================================
void DrawStartMenu(Graphics& g, int sw, int sh) {
    float a = g_startMenuAnim.current;
    if (a <= 0.01f) return;
    float ease = EaseOutCubic(Clamp01(a));

    SolidBrush overlay(Color((BYTE)(50 * a), 0, 0, 0));
    g.FillRectangle(&overlay, 0, 0, sw, sh);

    int menuW = START_MENU_W, menuH = START_MENU_H;
    int menuX = (sw - menuW) / 2;
    int targetY = sh - TASKBAR_HEIGHT - menuH - 14;
    int menuY = (int)(targetY + 24 * (1.0f - ease));

    // Scale animation
    float scale = 0.96f + 0.04f * ease;

    DrawShadow(g, menuX, menuY, menuW, menuH, START_MENU_RADIUS, 12, a);
    FillRoundRectSolid(g, menuX, menuY, menuW, menuH, START_MENU_RADIUS,
        Color((BYTE)(250 * a), 32, 32, 34));
    DrawRoundRect(g, menuX, menuY, menuW, menuH, START_MENU_RADIUS,
        Color((BYTE)(40 * a), 255, 255, 255), 1.0f);

    FontFamily ff(L"Segoe UI");
    StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat cfmt; cfmt.SetAlignment(StringAlignmentCenter); cfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat rfmt; rfmt.SetAlignment(StringAlignmentFar); rfmt.SetLineAlignment(StringAlignmentCenter);

    // Search bar with gradient accent
    int searchX = menuX + 26, searchY = menuY + 24;
    int searchW = menuW - 52, searchH = 38;
    FillRoundRectSolid(g, searchX, searchY, searchW, searchH, searchH / 2,
        Color((BYTE)(255 * a), 50, 50, 54));
    DrawRoundRect(g, searchX, searchY, searchW, searchH, searchH / 2,
        Color((BYTE)(30 * a), 255, 255, 255), 1.0f);
    // Accent bottom line
    Pen searchAccent(Color((BYTE)(90 * a), 96, 205, 255), 1.5f);
    g.DrawLine(&searchAccent, searchX + searchH / 2, searchY + searchH - 1,
        searchX + searchW - searchH / 2, searchY + searchH - 1);

    Pen sPen(Color((BYTE)(170 * a), 180, 180, 185), 1.5f);
    g.DrawEllipse(&sPen, searchX + 14, searchY + 10, 13, 13);
    g.DrawLine(&sPen, searchX + 25, searchY + 23, searchX + 29, searchY + 27);

    Font searchFont(&ff, 13.0f, FontStyleRegular, UnitPixel);
    SolidBrush searchBr(Color((BYTE)(110 * a), 170, 170, 175));
    RectF searchRc((REAL)(searchX + 38), (REAL)searchY, (REAL)(searchW - 50), (REAL)searchH);
    g.DrawString(L"Type here to search", -1, &searchFont, searchRc, &lfmt, &searchBr);

    // Section headers
    Font sectionFont(&ff, 14.0f, FontStyleSemiBold, UnitPixel);
    SolidBrush sectionBr(Color((BYTE)(255 * a), 250, 250, 255));
    RectF pinnedTitleRc((REAL)(menuX + 32), (REAL)(menuY + 78), 100.0f, 22.0f);
    g.DrawString(L"Pinned", -1, &sectionFont, pinnedTitleRc, &lfmt, &sectionBr);

    Font smallFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    SolidBrush allBr(Color((BYTE)(170 * a), 180, 180, 185));
    RectF allRc((REAL)(menuX + menuW - 135), (REAL)(menuY + 78), 105.0f, 22.0f);
    g.DrawString(L"All apps  \x203A", -1, &smallFont, allRc, &rfmt, &allBr);

    int gridX = menuX + 30, gridY = menuY + 110;
    int cols = 6;
    int cellW = (menuW - 60) / cols, cellH = 74;

    for (int i = 0; i < (int)g_startApps.size() && i < 18; i++) {
        int row = i / cols, col = i % cols;
        int ix = gridX + col * cellW;
        int iy = gridY + row * cellH;

        // Staggered reveal animation
        float delay = (float)i * 0.03f;
        float itemA = Clamp01((a - delay) * 3.0f);
        float itemEase = EaseOutCubic(itemA);
        int itemOffY = (int)(10 * (1.0f - itemEase));
        iy += itemOffY;

        bool hov = (g_hoveredStartItem == i);
        if (hov) {
            FillRoundRectSolid(g, ix + 3, iy + 3, cellW - 6, cellH - 6, 8,
                Color((BYTE)(255 * itemA), 56, 56, 60));
        }

        int aicoSize = 36;
        int aicoX = ix + (cellW - aicoSize) / 2;
        int aicoY = iy + 6;

        // Icon shadow
        FillRoundRectSolid(g, aicoX + 1, aicoY + 2, aicoSize, aicoSize, 8,
            Color((BYTE)(30 * itemA), 0, 0, 0));

        Color appC = g_startApps[i].color;
        FillRoundRectGradient(g, aicoX, aicoY, aicoSize, aicoSize, 8,
            Color((BYTE)(245 * itemA), appC.GetR(), appC.GetG(), appC.GetB()),
            Color((BYTE)(245 * itemA),
                (BYTE)(std::max)(0, (int)appC.GetR() - 20),
                (BYTE)(std::max)(0, (int)appC.GetG() - 20),
                (BYTE)(std::max)(0, (int)appC.GetB() - 20)));

        // Highlight
        Pen icoHL(Color((BYTE)(25 * itemA), 255, 255, 255), 0.5f);
        g.DrawLine(&icoHL, aicoX + 5, aicoY + 1, aicoX + aicoSize - 5, aicoY + 1);

        Font aicoFont(&ff, 11.0f, FontStyleBold, UnitPixel);
        SolidBrush aicoTextBr(Color((BYTE)(255 * itemA), 255, 255, 255));
        RectF aicoRc((REAL)aicoX, (REAL)aicoY, (REAL)aicoSize, (REAL)aicoSize);
        g.DrawString(g_startApps[i].iconLabel.c_str(), -1, &aicoFont, aicoRc, &cfmt, &aicoTextBr);

        Font nameFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
        SolidBrush nameBr(Color((BYTE)(215 * itemA), 215, 215, 225));
        cfmt.SetTrimming(StringTrimmingEllipsisCharacter);
        RectF nameRc((REAL)ix, (REAL)(iy + 46), (REAL)cellW, 18.0f);
        g.DrawString(g_startApps[i].name.c_str(), -1, &nameFont, nameRc, &cfmt, &nameBr);
    }

    // Separator
    int sepY = menuY + 110 + 3 * cellH + 14;
    Pen sepLine(Color((BYTE)(22 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&sepLine, menuX + 30, sepY, menuX + menuW - 30, sepY);

    // Recommended
    RectF recTitleRc((REAL)(menuX + 32), (REAL)(sepY + 14), 150.0f, 22.0f);
    g.DrawString(L"Recommended", -1, &sectionFont, recTitleRc, &lfmt, &sectionBr);

    const wchar_t* recNames[] = { L"Recently opened file.txt", L"Project_v2.docx", L"Screenshot_2026.png" };
    const wchar_t* recTimes[] = { L"Just now", L"Yesterday", L"2 days ago" };
    for (int i = 0; i < 3; i++) {
        int ry = sepY + 46 + i * 42;
        int rx = menuX + 32;

        FillRoundRectSolid(g, rx, ry + 3, 30, 30, 6, Color((BYTE)(180 * a), 55, 55, 62));
        Font fIcon(&ff, 10.0f, FontStyleRegular, UnitPixel);
        SolidBrush fIcoBr(Color((BYTE)(190 * a), 170, 175, 195));
        RectF fIcoRc((REAL)rx, (REAL)(ry + 3), 30.0f, 30.0f);
        g.DrawString(L"\xD83D\xDCC4", -1, &fIcon, fIcoRc, &cfmt, &fIcoBr);

        Font recFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
        SolidBrush recBr(Color((BYTE)(215 * a), 215, 215, 225));
        RectF recNameRc((REAL)(rx + 38), (REAL)(ry + 2), 220.0f, 18.0f);
        g.DrawString(recNames[i], -1, &recFont, recNameRc, &lfmt, &recBr);
        Font timeFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
        SolidBrush timeBr(Color((BYTE)(100 * a), 140, 140, 150));
        RectF recTimeRc((REAL)(rx + 38), (REAL)(ry + 21), 220.0f, 16.0f);
        g.DrawString(recTimes[i], -1, &timeFont, recTimeRc, &lfmt, &timeBr);
    }

    // Bottom bar
    int bottomY = menuY + menuH - 54;
    Pen bottomSep(Color((BYTE)(20 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&bottomSep, menuX + 26, bottomY, menuX + menuW - 26, bottomY);

    // User avatar
    FillRoundRectSolid(g, menuX + 32, bottomY + 12, 30, 30, 15,
        Color((BYTE)(200 * a), 75, 75, 85));
    // Gradient avatar
    FillRoundRectGradient(g, menuX + 32, bottomY + 12, 30, 30, 15,
        Color((BYTE)(200 * a), 90, 90, 105),
        Color((BYTE)(200 * a), 65, 65, 78));

    Font userIcon(&ff, 14.0f, FontStyleBold, UnitPixel);
    SolidBrush userBr(Color((BYTE)(240 * a), 200, 205, 220));
    RectF userRc((REAL)(menuX + 32), (REAL)(bottomY + 12), 30.0f, 30.0f);
    g.DrawString(L"U", -1, &userIcon, userRc, &cfmt, &userBr);

    Font usernameFont(&ff, 13.0f, FontStyleRegular, UnitPixel);
    SolidBrush usernameBr(Color((BYTE)(220 * a), 225, 225, 240));
    lfmt.SetLineAlignment(StringAlignmentCenter);
    RectF usernameRc((REAL)(menuX + 70), (REAL)(bottomY + 8), 200.0f, 38.0f);
    g.DrawString(L"User", -1, &usernameFont, usernameRc, &lfmt, &usernameBr);

    // Power button
    int pwrX = menuX + menuW - 54, pwrY = bottomY + 13;
    FillRoundRectSolid(g, pwrX, pwrY, 28, 28, 6, Color((BYTE)(50 * a), 255, 255, 255));
    Pen pwrPen(Color((BYTE)(200 * a), 200, 200, 210), 1.8f);
    pwrPen.SetStartCap(LineCapRound);
    pwrPen.SetEndCap(LineCapRound);
    int pcx = pwrX + 14, pcy = pwrY + 14;
    g.DrawArc(&pwrPen, pcx - 6, pcy - 6, 12, 12, -60, 300);
    g.DrawLine(&pwrPen, pcx, pcy - 8, pcx, pcy - 3);
}

// ============================================================================
//  CONTEXT MENU - Smooth animated
// ============================================================================
void DrawContextMenu(Graphics& g) {
    float a = g_contextMenuAnim.current;
    if (!g_contextMenuOpen || a <= 0.01f) return;
    float ease = EaseOutCubic(Clamp01(a));

    int menuW = CTX_MENU_W;
    int menuH = 8;
    for (const auto& item : g_contextItems)
        menuH += (item.id == 0) ? CTX_SEP_H : CTX_ITEM_H;
    menuH += 8;

    int mx = g_contextMenuX, my = g_contextMenuY;
    if (mx + menuW > ScreenW()) mx = ScreenW() - menuW - 8;
    if (my + menuH > ScreenH()) my = ScreenH() - menuH - 8;

    int animMenuH = (int)(menuH * ease);

    DrawShadow(g, mx, my, menuW, animMenuH, CTX_RADIUS, 8, a);
    FillRoundRectSolid(g, mx, my, menuW, animMenuH, CTX_RADIUS,
        Color((BYTE)(252 * a), 40, 40, 42));
    DrawRoundRect(g, mx, my, menuW, animMenuH, CTX_RADIUS,
        Color((BYTE)(35 * a), 255, 255, 255), 1.0f);

    if (ease < 0.4f) return;

    FontFamily ff(L"Segoe UI");
    Font font(&ff, 12.0f, FontStyleRegular, UnitPixel);
    Font shortcutFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
    StringFormat lfmt2; lfmt2.SetAlignment(StringAlignmentNear); lfmt2.SetLineAlignment(StringAlignmentCenter);
    StringFormat rfmt2; rfmt2.SetAlignment(StringAlignmentFar); rfmt2.SetLineAlignment(StringAlignmentCenter);

    g.SetClip(Rect(mx, my, menuW, animMenuH));

    int cy = my + 8;
    for (int i = 0; i < (int)g_contextItems.size(); i++) {
        const auto& item = g_contextItems[i];
        if (item.id == 0) {
            Pen sp(Color((BYTE)(22 * a), 255, 255, 255), 1.0f);
            g.DrawLine(&sp, mx + 14, cy + CTX_SEP_H / 2, mx + menuW - 14, cy + CTX_SEP_H / 2);
            cy += CTX_SEP_H;
            continue;
        }
        bool hov = (g_hoveredContextItem == i);
        if (hov) {
            FillRoundRectSolid(g, mx + 4, cy + 1, menuW - 8, CTX_ITEM_H - 2, 5,
                Color((BYTE)(255 * a), 52, 52, 56));
        }
        SolidBrush textBr(Color((BYTE)(230 * a), 230, 230, 240));
        RectF textRc((REAL)(mx + 18), (REAL)cy, (REAL)(menuW - 36), (REAL)CTX_ITEM_H);
        g.DrawString(item.label.c_str(), -1, &font, textRc, &lfmt2, &textBr);
        if (!item.shortcut.empty()) {
            SolidBrush scBr(Color((BYTE)(85 * a), 135, 135, 145));
            g.DrawString(item.shortcut.c_str(), -1, &shortcutFont, textRc, &rfmt2, &scBr);
        }
        cy += CTX_ITEM_H;
    }
    g.ResetClip();
}

// ============================================================================
//  WIFI PANEL - Premium quick settings
// ============================================================================
void DrawWifiPanel(Graphics& g, int sw, int sh) {
    float a = g_wifiPanelAnim.current;
    if (a <= 0.01f) return;
    float ease = EaseOutCubic(Clamp01(a));

    int panelW = WIFI_PANEL_W, panelH = WIFI_PANEL_H;
    int panelX = sw - panelW - 14;
    int panelY = (int)(sh - TASKBAR_HEIGHT - panelH - 14 + 16 * (1.0f - ease));

    DrawShadow(g, panelX, panelY, panelW, panelH, 10, 10, a);
    FillRoundRectSolid(g, panelX, panelY, panelW, panelH, 10,
        Color((BYTE)(252 * a), 32, 32, 34));
    DrawRoundRect(g, panelX, panelY, panelW, panelH, 10,
        Color((BYTE)(35 * a), 255, 255, 255), 1.0f);

    FontFamily ff(L"Segoe UI");
    StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat cfmt; cfmt.SetAlignment(StringAlignmentCenter); cfmt.SetLineAlignment(StringAlignmentCenter);

    // Quick toggles - premium pill design
    int toggleY = panelY + 18;
    int toggleH = 72;
    int toggleW = (panelW - 52) / 3;

    struct Toggle { std::wstring label; std::wstring icon; bool active; Color color; };
    Toggle toggles[] = {
        { L"WiFi", L"\xD83D\xDCF6", true, Color(255, 96, 205, 255) },
        { L"Bluetooth", L"\xD83D\xDD37", true, Color(255, 0, 120, 212) },
        { L"Airplane", L"\x2708", false, Color(255, 75, 75, 80) }
    };

    for (int i = 0; i < 3; i++) {
        int tx = panelX + 18 + i * (toggleW + 8);

        if (toggles[i].active) {
            FillRoundRectGradient(g, tx, toggleY, toggleW, toggleH, 10,
                Color((BYTE)(255 * a), toggles[i].color.GetR(), toggles[i].color.GetG(), toggles[i].color.GetB()),
                Color((BYTE)(255 * a),
                    (BYTE)(std::max)(0, (int)toggles[i].color.GetR() - 30),
                    (BYTE)(std::max)(0, (int)toggles[i].color.GetG() - 30),
                    (BYTE)(std::max)(0, (int)toggles[i].color.GetB() - 30)));
        }
        else {
            FillRoundRectSolid(g, tx, toggleY, toggleW, toggleH, 10,
                Color((BYTE)(200 * a), 50, 50, 54));
        }

        // Top highlight
        Pen thl(Color((BYTE)(toggles[i].active ? 35 : 15) * a / 255.0f * 255, 255, 255, 255), 0.5f);
        g.DrawLine(&thl, tx + 8, toggleY + 1, tx + toggleW - 8, toggleY + 1);

        // Icon
        Font iconFont(&ff, 16.0f, FontStyleRegular, UnitPixel);
        Color iconC = toggles[i].active ? Color((BYTE)(255 * a), 255, 255, 255) :
            Color((BYTE)(180 * a), 170, 170, 180);
        SolidBrush iconBr(iconC);
        RectF iconRc((REAL)tx, (REAL)(toggleY + 8), (REAL)toggleW, 28.0f);
        g.DrawString(toggles[i].icon.c_str(), -1, &iconFont, iconRc, &cfmt, &iconBr);

        Font tFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
        Color tColor = toggles[i].active ? Color((BYTE)(240 * a), 255, 255, 255) :
            Color((BYTE)(170 * a), 170, 170, 180);
        SolidBrush tBr(tColor);
        cfmt.SetLineAlignment(StringAlignmentFar);
        RectF tRc((REAL)tx, (REAL)toggleY, (REAL)toggleW, (REAL)(toggleH - 10));
        g.DrawString(toggles[i].label.c_str(), -1, &tFont, tRc, &cfmt, &tBr);
        cfmt.SetLineAlignment(StringAlignmentCenter);
    }

    // ---- Sliders with beautiful design ----
    int sliderY = toggleY + toggleH + 18;

    auto drawSlider = [&](int sy, const std::wstring& label, const std::wstring& icon, float value, Color accentC) {
        Font lbl(&ff, 11.0f, FontStyleRegular, UnitPixel);
        SolidBrush lblBr(Color((BYTE)(155 * a), 175, 175, 185));
        lfmt.SetLineAlignment(StringAlignmentNear);

        // Icon
        Font icoFont(&ff, 13.0f, FontStyleRegular, UnitPixel);
        SolidBrush icoBr(Color((BYTE)(180 * a), 200, 200, 210));
        RectF icoRc((REAL)(panelX + 18), (REAL)sy, 20.0f, 18.0f);
        g.DrawString(icon.c_str(), -1, &icoFont, icoRc, &lfmt, &icoBr);

        RectF lblRc((REAL)(panelX + 40), (REAL)sy, 100.0f, 18.0f);
        g.DrawString(label.c_str(), -1, &lbl, lblRc, &lfmt, &lblBr);

        int barX2 = panelX + 20, barY2 = sy + 24, barW2 = panelW - 40, barH2 = 6;
        FillRoundRectSolid(g, barX2, barY2, barW2, barH2, 3, Color((BYTE)(140 * a), 48, 48, 52));

        int fillW = (int)(barW2 * value);
        if (fillW > 0) {
            Color c1(255, accentC.GetR(), accentC.GetG(), accentC.GetB());
            Color c2(255,
                (BYTE)(std::max)(0, (int)accentC.GetR() - 40),
                (BYTE)(std::max)(0, (int)accentC.GetG() - 40),
                (BYTE)(std::max)(0, (int)accentC.GetB() - 40));
            FillRoundRectGradient(g, barX2, barY2, fillW, barH2, 3,
                Color((BYTE)(235 * a), c1.GetR(), c1.GetG(), c1.GetB()),
                Color((BYTE)(235 * a), c2.GetR(), c2.GetG(), c2.GetB()), false);
        }

        // Thumb
        int thumbX = barX2 + fillW - 7;
        if (thumbX < barX2) thumbX = barX2;
        FillRoundRectSolid(g, thumbX, barY2 - 3, 14, 12, 6,
            Color((BYTE)(240 * a), 255, 255, 255));
        FillRoundRectSolid(g, thumbX + 2, barY2 - 1, 10, 8, 4,
            Color((BYTE)(240 * a), accentC.GetR(), accentC.GetG(), accentC.GetB()));
        };

    drawSlider(sliderY, L"Brightness", L"\x2600", g_brightnessLevel, Color(255, 255, 210, 70));
    sliderY += 44;
    drawSlider(sliderY, L"Volume", L"\xD83D\xDD0A", g_volumeLevel, Color(255, 96, 205, 255));

    // Separator
    sliderY += 48;
    Pen sep(Color((BYTE)(20 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&sep, panelX + 18, sliderY, panelX + panelW - 18, sliderY);

    // WiFi networks header
    Font titleFont(&ff, 13.0f, FontStyleSemiBold, UnitPixel);
    SolidBrush titleBr(Color((BYTE)(250 * a), 240, 240, 245));
    RectF titleRc((REAL)(panelX + 20), (REAL)(sliderY + 10), 200.0f, 24.0f);
    lfmt.SetLineAlignment(StringAlignmentCenter);
    g.DrawString(L"Available networks", -1, &titleFont, titleRc, &lfmt, &titleBr);

    if (g_wifiScanning) {
        SolidBrush scanBr(Color((BYTE)(110 * a), 96, 205, 255));
        Font scanFont(&ff, 10.0f, FontStyleItalic, UnitPixel);
        StringFormat rfmt2; rfmt2.SetAlignment(StringAlignmentFar); rfmt2.SetLineAlignment(StringAlignmentCenter);
        RectF scanRc((REAL)(panelX + panelW - 125), (REAL)(sliderY + 10), 105.0f, 24.0f);
        g.DrawString(L"Scanning...", -1, &scanFont, scanRc, &rfmt2, &scanBr);
    }

    Font itemFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    Font small2Font(&ff, 10.0f, FontStyleRegular, UnitPixel);
    int listY = sliderY + 42;
    int itemHH = 52;

    for (int i = 0; i < (int)g_wifiNetworks.size() && i < 6; i++) {
        const WifiNetwork& net = g_wifiNetworks[i];
        int iy = listY + i * itemHH;
        bool hov = (g_hoveredWifiItem == i);

        if (hov) {
            FillRoundRectSolid(g, panelX + 10, iy, panelW - 20, itemHH - 4, 8,
                Color((BYTE)(200 * a), 48, 48, 52));
        }

        if (net.connected) {
            DrawRoundRect(g, panelX + 10, iy, panelW - 20, itemHH - 4, 8,
                Color((BYTE)(50 * a), 96, 205, 255), 1.2f);
            // Subtle glow
            FillRoundRectSolid(g, panelX + 10, iy, panelW - 20, itemHH - 4, 8,
                Color((BYTE)(8 * a), 96, 205, 255));
        }

        // Beautiful WiFi signal bars
        int sigX = panelX + 24;
        int sigBaseY = iy + itemHH / 2 + 9;
        for (int bar = 0; bar < 4; bar++) {
            int bH = 4 + bar * 4;
            bool filled = net.signal >= (bar + 1) * 25;
            Color barCol = filled ?
                Color((BYTE)(225 * a), 200, 220, 255) :
                Color((BYTE)(35 * a), 200, 200, 210);
            FillRoundRectSolid(g, sigX + bar * 7, sigBaseY - bH, 5, bH, 1, barCol);
        }

        // Lock icon
        if (net.secured) {
            Pen lockPen(Color((BYTE)(90 * a), 190, 195, 210), 1.2f);
            int lx2 = sigX + 32, ly2 = sigBaseY - 10;
            g.DrawArc(&lockPen, lx2, ly2 - 4, 8, 8, 180, 180);
            FillRoundRectSolid(g, lx2 - 1, ly2, 10, 7, 2, Color((BYTE)(90 * a), 190, 195, 210));
        }

        // SSID
        SolidBrush nameBr(Color((BYTE)(240 * a), 240, 240, 250));
        RectF nameRc((REAL)(panelX + 68), (REAL)(iy + 8), (REAL)(panelW - 140), 22.0f);
        g.DrawString(net.ssid.c_str(), -1, &itemFont, nameRc, &lfmt, &nameBr);

        // Status
        std::wstring status = net.connected ? L"Connected, secured" : (net.secured ? L"Secured" : L"Open network");
        Color statusColor = net.connected ? Color((BYTE)(155 * a), 108, 210, 100) :
            Color((BYTE)(90 * a), 155, 155, 165);
        SolidBrush statusBr(statusColor);
        RectF statusRc((REAL)(panelX + 68), (REAL)(iy + 30), (REAL)(panelW - 140), 16.0f);
        g.DrawString(status.c_str(), -1, &small2Font, statusRc, &lfmt, &statusBr);

        if (net.connected) {
            SolidBrush checkBr(Color((BYTE)(200 * a), 108, 210, 100));
            Font checkFont(&ff, 14.0f, FontStyleBold, UnitPixel);
            RectF checkRc((REAL)(panelX + panelW - 44), (REAL)iy, 28.0f, (REAL)itemHH);
            g.DrawString(L"\x2713", -1, &checkFont, checkRc, &cfmt, &checkBr);
        }
    }

    if (g_wifiNetworks.empty() && !g_wifiScanning) {
        SolidBrush emptyBr(Color((BYTE)(110 * a), 140, 140, 150));
        RectF emptyRc((REAL)panelX, (REAL)(listY + 40), (REAL)panelW, 30.0f);
        g.DrawString(L"No networks found", -1, &itemFont, emptyRc, &cfmt, &emptyBr);
    }
}

// ============================================================================
//  NOTIFICATIONS - Slide-in with spring bounce
// ============================================================================
void DrawNotifications(Graphics& g, int sw, int sh) {
    int baseY = sh - TASKBAR_HEIGHT - NOTIF_H - 18;
    for (int i = (int)g_notifs.size() - 1; i >= 0 && i >= (int)g_notifs.size() - 3; i--) {
        Notification& n = g_notifs[i];
        if (!n.alive) continue;
        float a = n.alpha.current;
        if (a <= 0.01f) continue;

        int idx = (int)g_notifs.size() - 1 - i;
        int nx = sw - NOTIF_W - 14;
        int ny = (int)(baseY - idx * (NOTIF_H + 10) + n.offsetY.current);

        DrawShadow(g, nx, ny, NOTIF_W, NOTIF_H, 10, 6, a);
        FillRoundRectSolid(g, nx, ny, NOTIF_W, NOTIF_H, 12,
            Color((BYTE)(250 * a), 40, 40, 42));
        DrawRoundRect(g, nx, ny, NOTIF_W, NOTIF_H, 12,
            Color((BYTE)(35 * a), 255, 255, 255), 1.0f);

        // Accent line
        FillRoundRectSolid(g, nx + 1, ny + 14, 3, NOTIF_H - 28, 1,
            Color((BYTE)(200 * a), 96, 205, 255));

        // Icon with gradient background
        FillRoundRectGradient(g, nx + 16, ny + 18, 40, 40, 10,
            Color((BYTE)(200 * a), 96, 205, 255),
            Color((BYTE)(200 * a), 50, 160, 230));

        FontFamily ff(L"Segoe UI");
        Font icoFont(&ff, 16.0f, FontStyleBold, UnitPixel);
        SolidBrush icoBr(Color((BYTE)(255 * a), 255, 255, 255));
        StringFormat cfmt; cfmt.SetAlignment(StringAlignmentCenter); cfmt.SetLineAlignment(StringAlignmentCenter);
        RectF icoRc((REAL)(nx + 16), (REAL)(ny + 18), 40.0f, 40.0f);
        g.DrawString(L"V", -1, &icoFont, icoRc, &cfmt, &icoBr);

        Font titleFont(&ff, 12.0f, FontStyleSemiBold, UnitPixel);
        Font msgFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
        StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentNear);

        SolidBrush titleBr(Color((BYTE)(250 * a), 245, 245, 250));
        RectF titleRc((REAL)(nx + 66), (REAL)(ny + 18), (REAL)(NOTIF_W - 90), 20.0f);
        g.DrawString(n.title.c_str(), -1, &titleFont, titleRc, &lfmt, &titleBr);

        SolidBrush msgBr(Color((BYTE)(170 * a), 185, 185, 195));
        RectF msgRc((REAL)(nx + 66), (REAL)(ny + 42), (REAL)(NOTIF_W - 90), 28.0f);
        g.DrawString(n.message.c_str(), -1, &msgFont, msgRc, &lfmt, &msgBr);

        DWORD elapsed = (GetTickCount() - n.time) / 1000;
        wchar_t tStr[32];
        if (elapsed < 60) swprintf_s(tStr, L"%ds", elapsed);
        else swprintf_s(tStr, L"%dm", elapsed / 60);
        Font tFont(&ff, 9.0f, FontStyleRegular, UnitPixel);
        SolidBrush tBr(Color((BYTE)(70 * a), 125, 125, 135));
        StringFormat rfmt2; rfmt2.SetAlignment(StringAlignmentFar); rfmt2.SetLineAlignment(StringAlignmentNear);
        RectF tRc((REAL)(nx + NOTIF_W - 55), (REAL)(ny + 10), 42.0f, 14.0f);
        g.DrawString(tStr, -1, &tFont, tRc, &rfmt2, &tBr);
    }
}

// ============================================================================
//  WIDGETS - Side panel with accent bars
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
    w.title = L"\x2600  Weather"; w.height = 100; w.accent = Color(255, 255, 210, 60);
    w.content = L"22\xB0  Partly Cloudy\nMoscow, Russia\nFeels like 19\xB0  |  Wind 3 m/s";
    g_widgets.push_back(w);

    MEMORYSTATUSEX mem; mem.dwLength = sizeof(mem); GlobalMemoryStatusEx(&mem);
    int cpuSim = 15 + (g_tick / 1000) % 25;
    swprintf_s(buf, L"CPU: %d%%  |  RAM: %d%%\nDisk: 62%%  |  GPU: 45%%", cpuSim, (int)mem.dwMemoryLoad);
    w.title = L"\xD83D\xDCCA  System Monitor"; w.content = buf; w.height = 82; w.accent = Color(255, 96, 205, 255);
    g_widgets.push_back(w);

    wcsftime(buf, 128, L"%A, %d %B %Y\n%H:%M:%S", &ti);
    w.title = L"\xD83D\xDD50  Clock"; w.content = buf; w.height = 82; w.accent = Color(255, 200, 200, 210);
    g_widgets.push_back(w);

    w.title = L"\xD83C\xDFB5  Now Playing"; w.height = 92; w.accent = Color(255, 255, 110, 140);
    w.content = g_musicPlaying ? L"Synthwave Mix\nArtist - Track Name\n\x25B6 2:15 / 4:30" : L"No music playing\nPress M to start";
    g_widgets.push_back(w);
}

void DrawWidgets(Graphics& g, int sw, int sh) {
    float a = g_widgetsAnim.current;
    if (a <= 0.01f) return;
    float ease = EaseOutCubic(Clamp01(a));

    int panelW = WIDGET_W;
    int offsetX = (int)(-panelW * (1.0f - ease));
    int baseX = 14 + offsetX, baseY = 14;

    FontFamily ff(L"Segoe UI");
    StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentNear);

    for (size_t i = 0; i < g_widgets.size(); i++) {
        const Widget& w = g_widgets[i];
        int x = baseX, y = baseY;

        DrawShadow(g, x, y, panelW, w.height, 8, 4, a);
        FillRoundRectSolid(g, x, y, panelW, w.height, 10,
            Color((BYTE)(240 * a), 36, 36, 38));
        DrawRoundRect(g, x, y, panelW, w.height, 10,
            Color((BYTE)(22 * a), 255, 255, 255), 1.0f);

        // Accent bar with gradient
        FillRoundRectGradient(g, x + 1, y + 14, 3, w.height - 28, 1,
            Color((BYTE)(210 * a), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()),
            Color((BYTE)(100 * a), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));

        Font titleFont(&ff, 11.0f, FontStyleSemiBold, UnitPixel);
        SolidBrush titleBr(Color((BYTE)(170 * a), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));
        RectF titleRc((REAL)(x + 18), (REAL)(y + 10), (REAL)(panelW - 36), 18.0f);
        g.DrawString(w.title.c_str(), -1, &titleFont, titleRc, &lfmt, &titleBr);

        Font contentFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
        SolidBrush contentBr(Color((BYTE)(215 * a), 225, 225, 235));
        std::wistringstream ss(w.content);
        std::wstring line;
        REAL ly = (REAL)(y + 32);
        while (std::getline(ss, line)) {
            RectF lineRc((REAL)(x + 18), ly, (REAL)(panelW - 36), 18.0f);
            g.DrawString(line.c_str(), -1, &contentFont, lineRc, &lfmt, &contentBr);
            ly += 18.0f;
        }
        baseY += w.height + 10;
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
    if (LoadWallpaperFile(cache.c_str())) {
        g_wallpaperReady = true;
        if (g_hWnd) InvalidateRect(g_hWnd, NULL, FALSE);
    }
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
        // Beautiful fallback gradient
        LinearGradientBrush bg(Point(0, 0), Point(sw, sh),
            Color(255, 15, 32, 72), Color(255, 48, 18, 58));
        g.FillRectangle(&bg, 0, 0, sw, sh);

        // Ambient light blobs
        for (int i = 0; i < 5; i++) {
            float t = g_time * 0.1f + i * 1.5f;
            int cx2 = sw / 5 + (int)(sinf(t) * sw / 6) + i * sw / 5;
            int cy2 = sh / 4 + (int)(cosf(t * 0.7f) * sh / 5) + (i % 2) * sh / 3;
            int cr = 180 + i * 40;
            SolidBrush blob(Color(6, 80 + i * 20, 80, 180));
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
    g.SetSmoothingMode(SmoothingModeHighQuality);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    g.SetCompositingQuality(CompositingQualityHighQuality);
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);

    DrawBackground(g, sw, sh);
    DrawDesktopIcons(g);
    DrawSelectionRect(g);
    DrawWidgets(g, sw, sh);

    // Explorer windows - inactive first, active on top
    for (auto& win : g_explorers) {
        if (win.visible && win.id != g_activeExplorerId)
            DrawExplorerWindow(g, win);
    }
    for (auto& win : g_explorers) {
        if (win.visible && win.id == g_activeExplorerId)
            DrawExplorerWindow(g, win);
    }

    DrawWifiPanel(g, sw, sh);
    DrawNotifications(g, sw, sh);
    DrawTaskbar(g, sw, sh);
    DrawContextMenu(g);
    DrawStartMenu(g, sw, sh);

    // Subtle hotkey hints
    if (!g_startMenuOpen && !g_contextMenuOpen && g_explorers.empty()) {
        FontFamily ff(L"Segoe UI");
        Font hintFont(&ff, 9.0f, FontStyleRegular, UnitPixel);
        SolidBrush hintBr(Color(30, 200, 200, 210));
        StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentFar);
        RectF hintRc(14.0f, (REAL)(sh - TASKBAR_HEIGHT - 18), 600.0f, 14.0f);
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
        SetTimer(hWnd, 2, 8, NULL); // ~120fps update timer
        PushNotification(L"VORTEX Desktop", L"Welcome! Your desktop is ready.");
        break;

    case WM_TIMER:
        g_tick = GetTickCount();
        g_time = (float)g_tick / 1000.0f;

        if (wParam == 1) {
            UpdateWidgets();
        }
        else if (wParam == 2) {
            float dt = 0.012f; // ~120fps timestep

            // Spring physics updates
            g_startMenuAnim.SetTarget(g_startMenuOpen ? 1.0f : 0.0f);
            g_startMenuAnim.Update(dt);

            g_widgetsAnim.SetTarget(g_widgetsVisible ? 1.0f : 0.0f);
            g_widgetsAnim.Update(dt);

            g_wifiPanelAnim.SetTarget(g_wifiPanelOpen ? 1.0f : 0.0f);
            g_wifiPanelAnim.Update(dt);

            g_contextMenuAnim.SetTarget(g_contextMenuOpen ? 1.0f : 0.0f);
            g_contextMenuAnim.Update(dt);

            // Taskbar icon animations
            bool startHov = (g_hoveredTaskbarIcon == -10);
            g_startBtnHover.SetTarget(startHov ? 1.0f : 0.0f);
            g_startBtnHover.Update(dt);
            g_startBtnScale.SetTarget(startHov ? 1.08f : 1.0f);
            g_startBtnScale.Update(dt);

            for (int i = 0; i < (int)g_taskbarAnims.size(); i++) {
                bool hov = (g_hoveredTaskbarIcon == i);
                g_taskbarAnims[i].hover.SetTarget(hov ? 1.0f : 0.0f);
                g_taskbarAnims[i].hover.Update(dt);
                g_taskbarAnims[i].scale.SetTarget(hov ? 1.1f : 1.0f);
                g_taskbarAnims[i].scale.Update(dt);
                g_taskbarAnims[i].glow.SetTarget(hov ? 1.0f : 0.0f);
                g_taskbarAnims[i].glow.Update(dt);
            }

            // Desktop icon animations
            for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
                bool hov = (g_hoveredDesktopIcon == i);
                g_desktopIcons[i].hoverAnim.SetTarget(hov ? 1.0f : 0.0f);
                g_desktopIcons[i].hoverAnim.Update(dt);
                g_desktopIcons[i].selectAnim.SetTarget(g_desktopIcons[i].selected ? 1.0f : 0.0f);
                g_desktopIcons[i].selectAnim.Update(dt);
            }

            // Explorer window animations
            for (auto it = g_explorers.begin(); it != g_explorers.end(); ) {
                it->animAlpha.Update(dt);
                it->animScale.Update(dt);
                it->scrollAnim.SetTarget(it->targetScroll);
                it->scrollAnim.Update(dt);

                if (it->animAlpha.target <= 0.0f && it->animAlpha.current <= 0.01f) {
                    it = g_explorers.erase(it);
                    if (g_explorers.empty() && !g_taskApps.empty()) {
                        g_taskApps[0].running = false;
                        g_taskApps[0].active = false;
                    }
                }
                else {
                    ++it;
                }
            }

            // Notifications with spring physics
            for (auto& n : g_notifs) {
                if (!n.alive) continue;
                n.alpha.Update(dt);
                n.offsetY.Update(dt);
                if (GetTickCount() - n.time > 5000) {
                    n.alpha.SetTarget(0.0f);
                    n.offsetY.SetTarget(-20.0f);
                    if (n.alpha.current <= 0.02f) n.alive = false;
                }
            }
            g_notifs.erase(std::remove_if(g_notifs.begin(), g_notifs.end(),
                [](const Notification& n) { return !n.alive && n.alpha.current <= 0.01f; }), g_notifs.end());

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
        if (g_activeExplorerId >= 0) {
            ExplorerWindow* win = FindExplorer(g_activeExplorerId);
            if (win && win->visible) {
                win->targetScroll -= delta / 2;
                int maxScroll = (int)win->items.size() * 32 - (win->h - 72);
                if (win->currentPath.empty()) maxScroll = (int)win->items.size() * 82 - (win->h - 72);
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

        if (g_explorerDragging >= 0) {
            ExplorerWindow* win = FindExplorer(g_explorerDragging);
            if (win) {
                win->x = mx - win->dragOffX;
                win->y = my - win->dragOffY;
                if (win->y < 0) win->y = 0;
                if (win->y > ScreenH() - 50) win->y = ScreenH() - 50;
            }
            break;
        }

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
            if (!win.visible || win.animAlpha.current < 0.5f) continue;

            int wx2 = win.x, wy2 = win.y, ww2 = win.w, wh2 = win.h;
            RECT winRect; SetRect(&winRect, wx2, wy2, wx2 + ww2, wy2 + wh2);

            if (PtInRect(&winRect, pt) && win.id == g_activeExplorerId) {
                win.hoveredTitleBtn = 0;
                int btnW2 = 46;
                for (int b = 1; b <= 3; b++) {
                    int bx = wx2 + ww2 - btnW2 * (4 - b);
                    RECT br; SetRect(&br, bx, wy2, bx + btnW2, wy2 + 34);
                    if (PtInRect(&br, pt)) { win.hoveredTitleBtn = b; break; }
                }

                win.hoveredSidebarItem = -1;
                int sidebarW2 = 200;
                int contentY2 = wy2 + 74;
                int sideY2 = contentY2 + 38;
                for (int si = 0; si < 7; si++) {
                    RECT sr; SetRect(&sr, wx2 + 6, sideY2, wx2 + sidebarW2 - 6, sideY2 + 30);
                    if (PtInRect(&sr, pt)) { win.hoveredSidebarItem = si; break; }
                    sideY2 += 30;
                    if (si == 5) sideY2 += 40;
                }

                win.hoveredItem = -1;
                int contentX2 = wx2 + sidebarW2;
                int contentW2 = ww2 - sidebarW2;

                if (mx > contentX2 && mx < wx2 + ww2 && my > contentY2 && my < wy2 + wh2) {
                    if (win.currentPath.empty()) {
                        int tileW = 242, tileH = 72;
                        int cols = (std::max)(1, (contentW2 - 44) / (tileW + 10));
                        int startY = contentY2 + 56 - (int)win.scrollAnim.current;
                        for (int i = 0; i < (int)win.items.size(); i++) {
                            int col = i % cols, row = i / cols;
                            int tx = contentX2 + 22 + col * (tileW + 10);
                            int ty = startY + row * (tileH + 10);
                            RECT tr; SetRect(&tr, tx, ty, tx + tileW, ty + tileH);
                            if (PtInRect(&tr, pt)) { win.hoveredItem = i; break; }
                        }
                    }
                    else {
                        int headerH = 34;
                        int itemH2 = 32;
                        int listY2 = contentY2 + headerH - (int)win.scrollAnim.current;
                        for (int i = 0; i < (int)win.items.size(); i++) {
                            int iy = listY2 + i * itemH2;
                            if (iy < contentY2 || iy + itemH2 > wy2 + wh2) continue;
                            RECT ir; SetRect(&ir, contentX2 + 4, iy, contentX2 + contentW2 - 4, iy + itemH2);
                            if (PtInRect(&ir, pt)) { win.hoveredItem = i; break; }
                        }
                    }
                }
                break;
            }
        }

        // Taskbar hover
        g_hoveredTaskbarIcon = -1;
        int sw = ScreenW(), sh = ScreenH();
        int barY = sh - TASKBAR_HEIGHT;
        if (my >= barY) {
            int iconCount = (int)g_taskApps.size();
            int totalW = (iconCount + 3) * (TASKBAR_ICON_SIZE + 6);
            int startX = (sw - totalW) / 2;
            int iconY = barY + (TASKBAR_HEIGHT - TASKBAR_ICON_SIZE) / 2;

            RECT sr; SetRect(&sr, startX, iconY, startX + TASKBAR_ICON_SIZE, iconY + TASKBAR_ICON_SIZE);
            if (PtInRect(&sr, pt)) g_hoveredTaskbarIcon = -10;

            int searchBtnX = startX + (TASKBAR_ICON_SIZE + 6);
            RECT sr2; SetRect(&sr2, searchBtnX, iconY, searchBtnX + TASKBAR_ICON_SIZE, iconY + TASKBAR_ICON_SIZE);
            if (PtInRect(&sr2, pt)) g_hoveredTaskbarIcon = -11;

            int tvBtnX = startX + 2 * (TASKBAR_ICON_SIZE + 6);
            RECT sr3; SetRect(&sr3, tvBtnX, iconY, tvBtnX + TASKBAR_ICON_SIZE, iconY + TASKBAR_ICON_SIZE);
            if (PtInRect(&sr3, pt)) g_hoveredTaskbarIcon = -12;

            for (int i = 0; i < iconCount; i++) {
                if (PtInRect(&g_taskApps[i].bounds, pt)) { g_hoveredTaskbarIcon = i; break; }
            }

            int trayX = sw - 240;
            RECT wifiRect; SetRect(&wifiRect, trayX + 20, barY, trayX + 58, barY + TASKBAR_HEIGHT);
            if (PtInRect(&wifiRect, pt)) g_hoveredTaskbarIcon = -20;
        }

        // Desktop icon hover
        g_hoveredDesktopIcon = -1;
        if (!g_startMenuOpen && !g_contextMenuOpen) {
            bool overExplorer = false;
            for (auto& win : g_explorers) {
                if (win.visible && win.animAlpha.current > 0.5f) {
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
        if (g_wifiPanelOpen && g_wifiPanelAnim.current > 0.5f) {
            int panelX = sw - WIFI_PANEL_W - 14;
            int panelY = sh - TASKBAR_HEIGHT - WIFI_PANEL_H - 14;
            int toggleH = 72;
            int listY2 = panelY + 18 + toggleH + 18 + 44 + 48 + 20 + 42;
            int itemHH = 52;
            for (int i = 0; i < (int)g_wifiNetworks.size() && i < 6; i++) {
                int iy = listY2 + i * itemHH;
                RECT ir; SetRect(&ir, panelX + 10, iy, panelX + WIFI_PANEL_W - 10, iy + itemHH);
                if (PtInRect(&ir, pt)) { g_hoveredWifiItem = i; break; }
            }
        }

        // Start menu hover
        g_hoveredStartItem = -1;
        if (g_startMenuOpen && g_startMenuAnim.current > 0.5f) {
            int menuX = (sw - START_MENU_W) / 2;
            int menuY2 = sh - TASKBAR_HEIGHT - START_MENU_H - 14;
            int gridX = menuX + 30, gridY = menuY2 + 110;
            int cols = 6;
            int cellW = (START_MENU_W - 60) / cols, cellH = 74;
            for (int i = 0; i < (int)g_startApps.size() && i < 18; i++) {
                int row = i / cols, col = i % cols;
                int ix = gridX + col * cellW;
                int iy = gridY + row * cellH;
                RECT cr; SetRect(&cr, ix, iy, ix + cellW, iy + cellH);
                if (PtInRect(&cr, pt)) { g_hoveredStartItem = i; break; }
            }
        }

        // Resize cursor
        bool onResizeEdge = false;
        for (auto& win : g_explorers) {
            if (!win.visible || win.animAlpha.current < 0.5f) continue;
            int wx2 = win.x, wy2 = win.y, ww2 = win.w, wh2 = win.h;
            if (mx > wx2 + ww2 - 8 && mx < wx2 + ww2 + 4 && my > wy2 + wh2 - 8 && my < wy2 + wh2 + 4) {
                SetCursor(LoadCursor(NULL, IDC_SIZENWSE)); onResizeEdge = true; break;
            }
            if (mx > wx2 + ww2 - 4 && mx < wx2 + ww2 + 4 && my > wy2 + 34 && my < wy2 + wh2 - 8) {
                SetCursor(LoadCursor(NULL, IDC_SIZEWE)); onResizeEdge = true; break;
            }
            if (my > wy2 + wh2 - 4 && my < wy2 + wh2 + 4 && mx > wx2 && mx < wx2 + ww2 - 8) {
                SetCursor(LoadCursor(NULL, IDC_SIZENS)); onResizeEdge = true; break;
            }
        }
        if (!onResizeEdge) SetCursor(LoadCursor(NULL, IDC_ARROW));
        break;
    }

    case WM_LBUTTONDOWN: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);
        POINT pt = { mx, my };

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

        if (g_startMenuOpen && g_startMenuAnim.current > 0.5f) {
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
                int menuY2 = sh2 - TASKBAR_HEIGHT - START_MENU_H - 14;
                RECT mr; SetRect(&mr, menuX, menuY2, menuX + START_MENU_W, menuY2 + START_MENU_H);
                if (!PtInRect(&mr, pt)) g_startMenuOpen = false;
            }
            break;
        }

        if (g_wifiPanelOpen && g_wifiPanelAnim.current > 0.5f) {
            int sw2 = ScreenW(), sh2 = ScreenH();
            int panelX = sw2 - WIFI_PANEL_W - 14;
            int panelY = sh2 - TASKBAR_HEIGHT - WIFI_PANEL_H - 14;
            RECT pr; SetRect(&pr, panelX, panelY, panelX + WIFI_PANEL_W, panelY + WIFI_PANEL_H);
            if (!PtInRect(&pr, pt) && my < ScreenH() - TASKBAR_HEIGHT)
                g_wifiPanelOpen = false;
        }

        // Explorer windows
        bool clickedExplorer = false;
        for (int ei = (int)g_explorers.size() - 1; ei >= 0; ei--) {
            ExplorerWindow& win = g_explorers[ei];
            if (!win.visible || win.animAlpha.current < 0.5f) continue;

            int wx2 = win.x, wy2 = win.y, ww2 = win.w, wh2 = win.h;
            RECT winRect; SetRect(&winRect, wx2, wy2, wx2 + ww2, wy2 + wh2);

            if (!PtInRect(&winRect, pt)) continue;

            clickedExplorer = true;
            g_activeExplorerId = win.id;

            // Resize edges
            if (mx > wx2 + ww2 - 8 && mx < wx2 + ww2 + 4 && my > wy2 + wh2 - 8 && my < wy2 + wh2 + 4) {
                g_explorerResizing = win.id; win.resizing = true; win.resizeEdge = 3;
                win.resizeStartX = mx; win.resizeStartY = my; win.resizeStartW = ww2; win.resizeStartH = wh2;
                SetCapture(hWnd); break;
            }
            if (mx > wx2 + ww2 - 4 && mx < wx2 + ww2 + 4 && my > wy2 + 34) {
                g_explorerResizing = win.id; win.resizing = true; win.resizeEdge = 1;
                win.resizeStartX = mx; win.resizeStartY = my; win.resizeStartW = ww2; win.resizeStartH = wh2;
                SetCapture(hWnd); break;
            }
            if (my > wy2 + wh2 - 4 && my < wy2 + wh2 + 4) {
                g_explorerResizing = win.id; win.resizing = true; win.resizeEdge = 2;
                win.resizeStartX = mx; win.resizeStartY = my; win.resizeStartW = ww2; win.resizeStartH = wh2;
                SetCapture(hWnd); break;
            }

            // Title bar buttons
            int btnW2 = 46;
            for (int b = 1; b <= 3; b++) {
                int bx = wx2 + ww2 - btnW2 * (4 - b);
                RECT br; SetRect(&br, bx, wy2, bx + btnW2, wy2 + 34);
                if (PtInRect(&br, pt)) {
                    if (b == 3) CloseExplorer(win.id);
                    else if (b == 2) {
                        if (!win.maximized) {
                            win.maximized = true;
                            win.x = 0; win.y = 0;
                            win.w = ScreenW(); win.h = ScreenH() - TASKBAR_HEIGHT;
                        }
                        else {
                            win.maximized = false;
                            win.x = (ScreenW() - 920) / 2; win.y = (ScreenH() - 620) / 2;
                            win.w = 920; win.h = 620;
                        }
                    }
                    else if (b == 1) { win.animAlpha.SetTarget(0.0f); }
                    clickedExplorer = true; break;
                }
            }

            // Title bar drag
            if (my < wy2 + 34 && mx < wx2 + ww2 - btnW2 * 3) {
                g_explorerDragging = win.id; win.dragging = true;
                win.dragOffX = mx - wx2; win.dragOffY = my - wy2;
                SetCapture(hWnd); break;
            }

            // Navigation buttons
            int toolY2 = wy2 + 34;
            int navX2 = wx2 + 12;
            int navBtnSize = 28;

            RECT backRect; SetRect(&backRect, navX2, toolY2 + 6, navX2 + navBtnSize, toolY2 + 34);
            if (PtInRect(&backRect, pt) && win.historyIndex > 0) {
                win.historyIndex--;
                win.currentPath = win.pathHistory[win.historyIndex];
                if (win.currentPath.empty()) { win.title = L"This PC"; LoadDrives(win); }
                else {
                    std::wstring t = win.currentPath;
                    if (t.back() == L'\\') t.pop_back();
                    size_t pos = t.find_last_of(L'\\');
                    if (pos != std::wstring::npos) t = t.substr(pos + 1);
                    win.title = t;
                    LoadDirectory(win, win.currentPath);
                }
                win.scrollOffset = 0; win.scrollAnim = SpringValue(0, 120, 12); win.targetScroll = 0;
                break;
            }
            navX2 += navBtnSize + 4;

            RECT fwdRect; SetRect(&fwdRect, navX2, toolY2 + 6, navX2 + navBtnSize, toolY2 + 34);
            if (PtInRect(&fwdRect, pt) && win.historyIndex < (int)win.pathHistory.size() - 1) {
                win.historyIndex++;
                win.currentPath = win.pathHistory[win.historyIndex];
                if (win.currentPath.empty()) { win.title = L"This PC"; LoadDrives(win); }
                else {
                    std::wstring t = win.currentPath;
                    if (t.back() == L'\\') t.pop_back();
                    size_t pos = t.find_last_of(L'\\');
                    if (pos != std::wstring::npos) t = t.substr(pos + 1);
                    win.title = t;
                    LoadDirectory(win, win.currentPath);
                }
                win.scrollOffset = 0; win.scrollAnim = SpringValue(0, 120, 12); win.targetScroll = 0;
                break;
            }
            navX2 += navBtnSize + 4;

            RECT upRect; SetRect(&upRect, navX2, toolY2 + 6, navX2 + navBtnSize, toolY2 + 34);
            if (PtInRect(&upRect, pt) && !win.currentPath.empty()) {
                std::wstring parent = win.currentPath;
                if (parent.back() == L'\\') parent.pop_back();
                size_t pos = parent.find_last_of(L'\\');
                if (pos != std::wstring::npos && pos > 2) parent = parent.substr(0, pos);
                else if (pos == 2) parent = parent.substr(0, 3);
                else parent = L"";
                NavigateExplorer(win, parent);
                break;
            }

            // Sidebar clicks
            if (win.hoveredSidebarItem >= 0) {
                std::wstring paths[] = {
                    GetSpecialFolderPath(L"Desktop"), GetSpecialFolderPath(L"Downloads"),
                    GetSpecialFolderPath(L"Documents"), GetSpecialFolderPath(L"Pictures"),
                    GetSpecialFolderPath(L"Music"), GetSpecialFolderPath(L"Videos"), L""
                };
                NavigateExplorer(win, paths[win.hoveredSidebarItem]);
                break;
            }

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
                        for (auto& w : g_explorers) {
                            if (w.animAlpha.target <= 0) w.animAlpha.SetTarget(1.0f);
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

        // Desktop icon click/drag
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
                g_dragging = true; g_dragIconIdx = i;
                g_dragOffsetX = mx - g_desktopIcons[i].pixelX;
                g_dragOffsetY = my - g_desktopIcons[i].pixelY;
                g_dragCurrentX = mx; g_dragCurrentY = my; g_dragStarted = false;
                SetCapture(hWnd); break;
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
            g_explorerDragging = -1; ReleaseCapture();
        }

        if (g_explorerResizing >= 0) {
            ExplorerWindow* win = FindExplorer(g_explorerResizing);
            if (win) win->resizing = false;
            g_explorerResizing = -1; ReleaseCapture();
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

        if (g_selecting) { g_selecting = false; ReleaseCapture(); }
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
            if (PtInRect(&r, pt)) { OpenDesktopIcon(i); break; }
        }

        for (int ei = (int)g_explorers.size() - 1; ei >= 0; ei--) {
            ExplorerWindow& win = g_explorers[ei];
            if (!win.visible || win.animAlpha.current < 0.5f || win.id != g_activeExplorerId) continue;

            if (win.hoveredItem >= 0 && win.hoveredItem < (int)win.items.size()) {
                const FileItem& fi = win.items[win.hoveredItem];
                if (fi.isDirectory) {
                    if (fi.name == L"..") {
                        std::wstring parent = win.currentPath;
                        if (parent.back() == L'\\') parent.pop_back();
                        size_t pos = parent.find_last_of(L'\\');
                        if (pos != std::wstring::npos && pos > 2) parent = parent.substr(0, pos);
                        else if (pos == 2) parent = parent.substr(0, 3);
                        else parent = L"";
                        NavigateExplorer(win, parent);
                    }
                    else NavigateExplorer(win, fi.fullPath);
                }
                else ShellExecute(NULL, L"open", fi.fullPath.c_str(), NULL, NULL, SW_SHOW);
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

        for (auto& win : g_explorers) {
            if (win.visible && win.animAlpha.current > 0.5f) {
                RECT wr; SetRect(&wr, win.x, win.y, win.x + win.w, win.y + win.h);
                if (PtInRect(&wr, pt)) return 0;
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
        g_contextMenuAnim = SpringValue(0, 280, 18);
        g_contextMenuAnim.SetTarget(1.0f);
        g_hoveredContextItem = -1;
        break;
    }

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            if (g_contextMenuOpen) g_contextMenuOpen = false;
            else if (g_wifiPanelOpen) g_wifiPanelOpen = false;
            else if (g_startMenuOpen) g_startMenuOpen = false;
            else if (!g_explorers.empty()) {
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
        else if (wParam == 'W') { g_widgetsVisible = !g_widgetsVisible; }
        else if (wParam == 'N') { PushNotification(L"Notification", L"This is a test notification"); }
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
                        else NavigateExplorer(*win, fi.fullPath);
                    }
                    else ShellExecute(NULL, L"open", fi.fullPath.c_str(), NULL, NULL, SW_SHOW);
                }
            }
        }
        else if (wParam == VK_BACK) {
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
    wc.lpszClassName = L"VORTEX_Desktop_v5";
    RegisterClassExW(&wc);

    int sw = ScreenW(), sh = ScreenH();
    g_hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"VORTEX_Desktop_v5", L"VORTEX Desktop v5",
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
Сборка начата в 9:27...
1>------ Сборка начата: проект: ConsoleApplication5, Конфигурация: Debug x64 ------
1>ConsoleApplication5.cpp
1>C:\Users\User\source\repos\ConsoleApplication5\ConsoleApplication5\ConsoleApplication5.cpp(1918,32): error C2065: FontStyleSemiBold: необъявленный идентификатор
1>C:\Users\User\source\repos\ConsoleApplication5\ConsoleApplication5\ConsoleApplication5.cpp(1990,34): error C2065: FontStyleSemiBold: необъявленный идентификатор
1>C:\Users\User\source\repos\ConsoleApplication5\ConsoleApplication5\ConsoleApplication5.cpp(2233,72): warning C4244: аргумент: преобразование "float" в "BYTE", возможна потеря данных
1>C:\Users\User\source\repos\ConsoleApplication5\ConsoleApplication5\ConsoleApplication5.cpp(2305,32): error C2065: FontStyleSemiBold: необъявленный идентификатор
1>C:\Users\User\source\repos\ConsoleApplication5\ConsoleApplication5\ConsoleApplication5.cpp(2427,36): error C2065: FontStyleSemiBold: необъявленный идентификатор
1>C:\Users\User\source\repos\ConsoleApplication5\ConsoleApplication5\ConsoleApplication5.cpp(2515,36): error C2065: FontStyleSemiBold: необъявленный идентификатор
1>Сборка проекта "ConsoleApplication5.vcxproj" завершена с ошибкой.
========== Сборка: успешно выполнено — 0 , со сбоем — 1, в актуальном состоянии — 0, пропущено — 0 ==========
========== Сборка завершено в 9:27 и заняло 01,144 с ==========

