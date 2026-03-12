/*
 * VORTEX Desktop Environment v6.0 - Windows 12 Premium Edition
 * With full boot sequence animation
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

#define FONT_SEMIBOLD FontStyleBold

// ============================================================================
//  BOOT SEQUENCE STATE
// ============================================================================
enum BootPhase {
    BOOT_BLACK = 0,
    BOOT_LOGO_FADE_IN,
    BOOT_SPINNER,
    BOOT_PREPARING,
    BOOT_WELCOME,
    BOOT_FADE_TO_DESKTOP,
    BOOT_COMPLETE
};

struct BootState {
    BootPhase phase;
    float phaseTime;
    float totalTime;
    float logoAlpha;
    float spinnerAngle;
    float spinnerAlpha;
    float textAlpha;
    float welcomeAlpha;
    float fadeOutAlpha;
    float dotPulse[12];
    int currentTextIdx;
    float progressValue;
    bool complete;

    BootState() : phase(BOOT_BLACK), phaseTime(0), totalTime(0),
        logoAlpha(0), spinnerAngle(0), spinnerAlpha(0),
        textAlpha(0), welcomeAlpha(0), fadeOutAlpha(1.0f),
        currentTextIdx(0), progressValue(0), complete(false) {
        for (int i = 0; i < 12; i++) dotPulse[i] = 0;
    }
};
BootState g_boot;

// Phase durations in seconds
const float BOOT_BLACK_DUR = 0.8f;
const float BOOT_LOGO_FADE_DUR = 1.5f;
const float BOOT_SPINNER_DUR = 4.5f;
const float BOOT_PREPARING_DUR = 2.5f;
const float BOOT_WELCOME_DUR = 2.0f;
const float BOOT_FADE_DUR = 1.2f;

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

// ============================================================================
//  EASING FUNCTIONS
// ============================================================================
inline float pOutCubic(float t) {
    t = t - 1.0f;
    return t * t * t + 1.0f;
}
inline float pOutQuint(float t) {
    t = t - 1.0f;
    return t * t * t * t * t + 1.0f;
}
inline float pOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
}
inline float pInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}
inline float pOutElastic(float t) {
    if (t <= 0) return 0;
    if (t >= 1) return 1;
    return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * 2.0943951f) + 1.0f;
}
inline float pOutExpo(float t) {
    return t >= 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
}
inline float pOutCirc(float t) {
    return sqrtf(1.0f - powf(t - 1.0f, 2.0f));
}
inline float pOutQuart(float t) {
    t = t - 1.0f;
    return 1.0f - t * t * t * t;
}
inline float pInOutQuad(float t) {
    return t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}
inline float pInOutSine(float t) {
    return -(cosf(3.14159265f * t) - 1.0f) / 2.0f;
}
inline float SmoothLerp(float current, float target, float speed) {
    float diff = target - current;
    if (fabsf(diff) < 0.001f) return target;
    return current + diff * speed;
}
inline float Clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }
inline float Clamp(float v, float mn, float mx) { return v < mn ? mn : (v > mx ? mx : v); }
inline float SmoothNoise(float t) {
    return sinf(t * 1.7f) * 0.3f + sinf(t * 3.1f) * 0.2f + sinf(t * 0.8f) * 0.5f;
}
inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }

inline Color BlendColors(Color a, Color b, float t) {
    return Color(
        (BYTE)((1 - t) * a.GetA() + t * b.GetA()),
        (BYTE)((1 - t) * a.GetR() + t * b.GetR()),
        (BYTE)((1 - t) * a.GetG() + t * b.GetG()),
        (BYTE)((1 - t) * a.GetB() + t * b.GetB()));
}

// ============================================================================
//  WINDOWS 12 COLOR PALETTE — Ultra-premium design system
// ============================================================================
namespace W12 {
    // Primary accent — electric blue with cyan shift
    const Color Accent(255, 0, 120, 255);
    const Color AccentLight(255, 88, 186, 255);
    const Color AccentLighter(255, 140, 210, 255);
    const Color AccentDark(255, 0, 72, 180);
    const Color AccentSubtle(35, 0, 140, 255);
    const Color AccentGlow(55, 88, 186, 255);

    // Surfaces — refined with subtle warm undertones
    const Color SurfaceBase(255, 18, 18, 22);
    const Color SurfaceCard(240, 28, 28, 33);
    const Color SurfaceCardHover(248, 38, 38, 45);
    const Color SurfaceFlyout(250, 30, 30, 35);
    const Color SurfaceOverlay(180, 0, 0, 0);
    const Color SurfaceStroke(40, 255, 255, 255);
    const Color SurfaceStrokeLight(20, 255, 255, 255);
    const Color SurfaceDivider(14, 255, 255, 255);

    // Mica & Acrylic
    const Color MicaBg(235, 20, 20, 24);
    const Color AcrylicBg(220, 24, 24, 28);

    // Text hierarchy
    const Color TextPrimary(255, 255, 255, 255);
    const Color TextSecondary(255, 195, 195, 200);
    const Color TextTertiary(255, 135, 135, 142);
    const Color TextDisabled(255, 80, 80, 86);

    // Semantic
    const Color Success(255, 90, 215, 80);
    const Color Warning(255, 255, 200, 40);
    const Color Error(255, 255, 85, 85);
    const Color Info(255, 88, 186, 255);

    // Taskbar — premium dark glass
    const Color TaskbarBg(240, 14, 14, 18);
    const Color TaskbarGlass(200, 22, 22, 26);
    const Color TaskbarHover(255, 48, 48, 55);
    const Color TaskbarActive(255, 55, 55, 62);
    const Color TaskbarIndicator(255, 88, 186, 255);
    const Color TaskbarIndicatorGlow(70, 88, 186, 255);

    // Start Menu
    const Color StartBg(248, 24, 24, 28);
    const Color StartSearch(255, 40, 40, 46);
    const Color StartItemHover(255, 50, 50, 58);

    // Context Menu
    const Color CtxBg(250, 30, 30, 35);
    const Color CtxHover(255, 48, 48, 55);

    // Selection
    const Color SelectionRect(60, 0, 120, 255);
    const Color SelectionBorder(160, 88, 186, 255);
    const Color IconSelectedBg(50, 0, 120, 255);
    const Color IconHoverBg(30, 255, 255, 255);

    // Window chrome — W12 frosted
    const Color WinTitleBar(255, 22, 22, 26);
    const Color WinBody(255, 18, 18, 22);
    const Color WinSidebar(255, 22, 22, 27);
    const Color WinToolbar(255, 26, 26, 31);
    const Color WinItemHover(255, 40, 40, 48);
    const Color WinItemSelected(255, 0, 65, 165);
    const Color WinScrollbar(255, 40, 40, 46);
    const Color WinScrollThumb(255, 68, 68, 76);

    // Boot screen
    const Color BootBg(255, 0, 0, 0);
    const Color BootLogoGlow(255, 88, 186, 255);
    const Color BootSpinner(255, 88, 186, 255);
    const Color BootText(255, 200, 200, 210);
}

// ============================================================================
//  CONSTANTS
// ============================================================================
const wchar_t* WALLPAPER_URL = L"https://images.wallpaperscraft.com/image/single/lake_mountains_trees_1219008_1920x1080.jpg";
const wchar_t* WALLPAPER_CACHE = L"vortex_wallpaper_cache.jpg";

const int TASKBAR_HEIGHT = 54;
const int TASKBAR_ICON_SIZE = 46;

const int DESKTOP_ICON_W = 78;
const int DESKTOP_ICON_H = 88;
const int DESKTOP_GRID_X = 84;
const int DESKTOP_GRID_Y = 94;
const int DESKTOP_MARGIN_X = 16;
const int DESKTOP_MARGIN_Y = 16;

const int START_MENU_W = 640;
const int START_MENU_H = 600;
const int START_MENU_RADIUS = 14;

const int CTX_MENU_W = 268;
const int CTX_ITEM_H = 38;
const int CTX_SEP_H = 9;
const int CTX_RADIUS = 12;

const int WIFI_PANEL_W = 390;
const int WIFI_PANEL_H = 470;

const int NOTIF_W = 380;
const int NOTIF_H = 90;

const int WIDGET_W = 340;

// ============================================================================
//  EXPLORER WINDOW
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
float g_time = 0;
LARGE_INTEGER g_perfFreq, g_lastFrame;

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

int g_hoveredTaskbarIcon = -1;
int g_hoveredStartItem = -1;
int g_hoveredDesktopIcon = -1;
int g_hoveredWifiItem = -1;
int g_contextMenuX = 0, g_contextMenuY = 0;
int g_hoveredContextItem = -1;

SpringValue g_startMenuAnim(0, 350, 22);
SpringValue g_widgetsAnim(0, 300, 20);
SpringValue g_wifiPanelAnim(0, 320, 21);
SpringValue g_contextMenuAnim(0, 400, 24);
SpringValue g_desktopFadeIn(0, 120, 10);

struct TaskbarIconAnim {
    SpringValue hover;
    SpringValue scale;
    SpringValue glow;
    SpringValue bounce;
    SpringValue labelAlpha;
    TaskbarIconAnim() : hover(0, 320, 18), scale(1.0f, 400, 20),
        glow(0, 250, 16), bounce(0, 300, 18), labelAlpha(0, 220, 14) {
    }
};
std::vector<TaskbarIconAnim> g_taskbarAnims;
SpringValue g_startBtnHover(0, 300, 18);
SpringValue g_startBtnScale(1.0f, 400, 20);

bool g_selecting = false;
int g_selStartX = 0, g_selStartY = 0;
int g_selEndX = 0, g_selEndY = 0;

bool g_dragging = false;
int g_dragIconIdx = -1;
int g_dragOffsetX = 0, g_dragOffsetY = 0;
int g_dragCurrentX = 0, g_dragCurrentY = 0;
bool g_dragStarted = false;

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
    DesktopIcon() : hoverAnim(0, 300, 18), selectAnim(0, 280, 17) {}
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
    add(L"Recycle Bin", L"shell:RecycleBinFolder", 0, 1, Color(255, 165, 165, 170), 1);
    add(L"Documents", L"__folder__", 0, 2, Color(255, 255, 213, 79), 5);
    add(L"Downloads", L"__folder__", 0, 3, Color(255, 88, 186, 255), 5);
    add(L"Pictures", L"__folder__", 0, 4, Color(255, 172, 135, 255), 5);
    add(L"Music", L"__folder__", 0, 5, Color(255, 255, 105, 135), 5);
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
            case DRIVE_REMOVABLE: fi.driveType = L"Removable"; break;
            case DRIVE_REMOTE: fi.driveType = L"Network"; break;
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
                if (wcslen(volName) > 0)
                    fi.name = std::wstring(volName) + L" (" + fi.name + L")";
                else
                    fi.name = fi.driveType + L" (" + std::wstring(1, (wchar_t)('A' + i)) + L":)";
            }

            memset(&fi.modTime, 0, sizeof(FILETIME));
            win.items.push_back(fi);
        }
    }
}

void LoadDirectory(ExplorerWindow& win, const std::wstring& path) {
    win.items.clear();
    win.scrollOffset = 0;
    win.scrollAnim = SpringValue(0, 180, 15);
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
    if (win.historyIndex < (int)win.pathHistory.size() - 1)
        win.pathHistory.resize(win.historyIndex + 1);
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
    win.x = (ScreenW() - 940) / 2 + offset;
    win.y = (ScreenH() - 640) / 2 + offset;
    win.w = 940;
    win.h = 640;
    win.animAlpha = SpringValue(0, 400, 22);
    win.animAlpha.SetTarget(1.0f);
    win.animScale = SpringValue(0.94f, 500, 24);
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
    win.scrollAnim = SpringValue(0, 180, 15);
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
            w.animScale.SetTarget(0.93f);
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
    add(L"Terminal", L"cmd.exe", L">_", Color(255, 45, 45, 52), Color(255, 120, 120, 130), false, false);
    add(L"Notepad", L"notepad.exe", L"\xD83D\xDCDD", Color(255, 100, 170, 255), Color(255, 100, 170, 255), false, false);
    add(L"Settings", L"ms-settings:", L"\x2699", Color(255, 82, 82, 95), Color(255, 130, 130, 145), false, false);
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
    add(L"Terminal", L">_", L"cmd.exe", Color(255, 45, 45, 52));
    add(L"Notepad", L"\xD83D\xDCDD", L"notepad.exe", Color(255, 100, 170, 255));
    add(L"Settings", L"\x2699", L"ms-settings:", Color(255, 82, 82, 95));
    add(L"Calculator", L"\xD83D\xDDA9", L"calc.exe", Color(255, 45, 45, 52));
    add(L"Paint", L"\xD83C\xDFA8", L"mspaint.exe", Color(255, 255, 95, 95));
    add(L"Photos", L"\xD83D\xDDBC", L"ms-photos:", Color(255, 255, 175, 50));
    add(L"Mail", L"\x2709", L"notepad.exe", Color(255, 0, 120, 212));
    add(L"Calendar", L"\xD83D\xDCC5", L"notepad.exe", Color(255, 0, 120, 212));
    add(L"Maps", L"\xD83D\xDDFA", L"notepad.exe", Color(255, 100, 170, 255));
    add(L"Weather", L"\x2600", L"notepad.exe", Color(255, 255, 200, 50));
    add(L"Clock", L"\xD83D\xDD50", L"notepad.exe", Color(255, 95, 95, 102));
    add(L"Camera", L"\xD83D\xDCF7", L"notepad.exe", Color(255, 65, 65, 72));
    add(L"Store", L"\xD83D\xDECD", L"ms-windows-store:", Color(255, 0, 120, 212));
    add(L"Task Manager", L"\xD83D\xDCCA", L"taskmgr.exe", Color(255, 45, 45, 52));
    add(L"Control Panel", L"\xD83D\xDEE0", L"control.exe", Color(255, 0, 80, 160));
    add(L"Snip & Sketch", L"\x2702", L"notepad.exe", Color(255, 170, 85, 255));
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
    Notification() : alpha(0, 280, 16), offsetY(50, 350, 18), offsetX(0, 280, 16) {
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

void RoundedRectPathF(GraphicsPath& p, float x, float y, float w, float h, float r) {
    float d = r * 2.0f;
    if (d > w) d = w;
    if (d > h) d = h;
    if (r <= 0) { p.AddRectangle(RectF(x, y, w, h)); return; }
    p.AddArc(x, y, d, d, 180.0f, 90.0f);
    p.AddArc(x + w - d, y, d, d, 270.0f, 90.0f);
    p.AddArc(x + w - d, y + h - d, d, d, 0.0f, 90.0f);
    p.AddArc(x, y + h - d, d, d, 90.0f, 90.0f);
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

void FillRoundRectF(Graphics& g, float x, float y, float w, float h, float r, Color c) {
    GraphicsPath p; RoundedRectPathF(p, x, y, w, h, r);
    SolidBrush br(c); g.FillPath(&br, &p);
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

void FillRoundRectGradientF(Graphics& g, float x, float y, float w, float h, float r, Color c1, Color c2, bool vertical = true) {
    GraphicsPath p; RoundedRectPathF(p, x, y, w, h, r);
    if (vertical) {
        LinearGradientBrush br(PointF(x, y), PointF(x, y + h), c1, c2);
        g.FillPath(&br, &p);
    }
    else {
        LinearGradientBrush br(PointF(x, y), PointF(x + w, y), c1, c2);
        g.FillPath(&br, &p);
    }
}

void DrawShadow(Graphics& g, int x, int y, int w, int h, int r, int layers = 8, float intensity = 1.0f) {
    for (int i = layers; i >= 1; i--) {
        int sp = i * 2 + 1;
        float t = (float)(layers - i + 1) / (float)layers;
        int alpha = (int)(24.0f * t * t * intensity);
        FillRoundRectSolid(g, x - sp + 1, y - sp + 3, w + sp * 2, h + sp * 2, r + sp / 2,
            Color((BYTE)(std::min)(255, alpha), 0, 0, 0));
    }
}

void DrawPremiumShadow(Graphics& g, int x, int y, int w, int h, int r, float intensity = 1.0f) {
    for (int i = 18; i >= 1; i--) {
        int sp = i * 2;
        float t = (float)i / 18.0f;
        int alpha = (int)(10.0f * (1.0f - t) * intensity);
        FillRoundRectSolid(g, x - sp, y - sp / 2 + sp, w + sp * 2, h + sp * 2, r + sp,
            Color((BYTE)(std::min)(255, alpha), 0, 0, 0));
    }
    for (int i = 5; i >= 1; i--) {
        int sp = i;
        float t = (float)i / 5.0f;
        int alpha = (int)(20.0f * (1.0f - t) * intensity);
        FillRoundRectSolid(g, x - sp, y + sp, w + sp * 2, h + sp * 2, r + sp / 2,
            Color((BYTE)(std::min)(255, alpha), 0, 0, 0));
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
//  ICON RENDERING
// ============================================================================
void DrawFolderIcon(Graphics& g, int x, int y, int size, Color color, float alpha = 1.0f) {
    BYTE a = (BYTE)(color.GetA() * alpha);
    int R = color.GetR(), G = color.GetG(), B = color.GetB();
    FillRoundRectSolid(g, x + 3, y + (int)(size * 0.15f) + 2, size - 4, (int)(size * 0.72f), 4,
        Color((BYTE)(40 * alpha), 0, 0, 0));
    Color tabColor(a, (BYTE)(std::min)(255, R + 10), (BYTE)(std::min)(255, G + 10), (BYTE)(std::min)(255, B + 10));
    FillRoundRectSolid(g, x + 2, y + 2, (int)(size * 0.42f), (int)(size * 0.16f), 4, tabColor);
    Color bodyBack(a, (BYTE)R, (BYTE)G, (BYTE)B);
    FillRoundRectSolid(g, x + 2, y + (int)(size * 0.13f), size - 4, (int)(size * 0.73f), 5, bodyBack);
    Color frontTop((BYTE)(255 * alpha),
        (BYTE)(std::min)(255, R + 40), (BYTE)(std::min)(255, G + 40), (BYTE)(std::min)(255, B + 40));
    Color frontBot((BYTE)(255 * alpha),
        (BYTE)(std::min)(255, R + 20), (BYTE)(std::min)(255, G + 20), (BYTE)(std::min)(255, B + 20));
    FillRoundRectGradient(g, x + 2, y + (int)(size * 0.36f), size - 4, (int)(size * 0.50f), 5, frontTop, frontBot);
    Pen highlightPen(Color((BYTE)(45 * alpha), 255, 255, 255), 1.0f);
    g.DrawLine(&highlightPen, x + 5, y + (int)(size * 0.37f), x + size - 7, y + (int)(size * 0.37f));
}

void DrawFileIcon(Graphics& g, int x, int y, int size, const std::wstring& ext, float alpha = 1.0f) {
    int pw = (int)(size * 0.68f);
    int ph = (int)(size * 0.84f);
    int px = x + (size - pw) / 2;
    int py = y + (size - ph) / 2;
    FillRoundRectSolid(g, px + 2, py + 3, pw, ph, 3, Color((BYTE)(35 * alpha), 0, 0, 0));
    FillRoundRectGradient(g, px, py, pw, ph, 4,
        Color((BYTE)(245 * alpha), 232, 232, 238),
        Color((BYTE)(240 * alpha), 215, 215, 225));
    int foldSize = (int)(pw * 0.28f);
    Point foldPts[3] = {
        Point(px + pw - foldSize, py),
        Point(px + pw, py + foldSize),
        Point(px + pw - foldSize, py + foldSize)
    };
    SolidBrush foldBr(Color((BYTE)(200 * alpha), 195, 195, 208));
    g.FillPolygon(&foldBr, foldPts, 3);

    if (!ext.empty()) {
        Color extColor((BYTE)(220 * alpha), 80, 80, 100);
        if (ext == L".txt" || ext == L".log") extColor = Color((BYTE)(220 * alpha), 80, 140, 200);
        else if (ext == L".exe" || ext == L".msi") extColor = Color((BYTE)(220 * alpha), 200, 90, 70);
        else if (ext == L".jpg" || ext == L".png" || ext == L".bmp") extColor = Color((BYTE)(220 * alpha), 80, 180, 90);
        else if (ext == L".mp3" || ext == L".wav") extColor = Color((BYTE)(220 * alpha), 190, 80, 200);
        else if (ext == L".zip" || ext == L".rar") extColor = Color((BYTE)(220 * alpha), 200, 180, 40);
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
    FillRoundRectSolid(g, x + 3, y + (int)(size * 0.18f) + 2, size - 4, (int)(size * 0.68f), 6,
        Color((BYTE)(30 * alpha), 0, 0, 0));
    FillRoundRectGradient(g, x + 2, y + (int)(size * 0.15f), size - 4, (int)(size * 0.68f), 6,
        Color((BYTE)(235 * alpha), 68, 68, 78),
        Color((BYTE)(235 * alpha), 52, 52, 60));
    int barX = x + 7;
    int barY = y + (int)(size * 0.56f);
    int barW = size - 14;
    int barH = 7;
    FillRoundRectSolid(g, barX, barY, barW, barH, 3, Color((BYTE)(200 * alpha), 35, 35, 40));
    int fillW = (int)(barW * usedPercent);
    if (fillW > 0) {
        Color barC1 = usedPercent > 0.9f ? Color((BYTE)(240 * alpha), 255, 80, 80) :
            usedPercent > 0.7f ? Color((BYTE)(240 * alpha), 255, 210, 50) :
            Color((BYTE)(240 * alpha), 88, 200, 255);
        Color barC2 = usedPercent > 0.9f ? Color((BYTE)(240 * alpha), 220, 50, 50) :
            usedPercent > 0.7f ? Color((BYTE)(240 * alpha), 230, 180, 30) :
            Color((BYTE)(240 * alpha), 45, 155, 230);
        FillRoundRectGradient(g, barX, barY, fillW, barH, 3, barC1, barC2, false);
    }
    Color ledColor = usedPercent > 0.9f ? Color((BYTE)(200 * alpha), 255, 60, 60) :
        Color((BYTE)(200 * alpha), 88, 220, 88);
    SolidBrush ledBr(ledColor);
    g.FillEllipse(&ledBr, x + size - 14, y + (int)(size * 0.22f), 5, 5);
}

void DrawPCIcon(Graphics& g, int x, int y, int size, float alpha = 1.0f) {
    int mw = (int)(size * 0.78f);
    int mh = (int)(size * 0.52f);
    int mx = x + (size - mw) / 2;
    int my = y + 2;
    FillRoundRectSolid(g, mx + 2, my + 3, mw, mh, 5, Color((BYTE)(35 * alpha), 0, 0, 0));
    FillRoundRectGradient(g, mx, my, mw, mh, 5,
        Color((BYTE)(235 * alpha), 45, 110, 195),
        Color((BYTE)(235 * alpha), 30, 80, 160));
    FillRoundRectGradient(g, mx + 3, my + 3, mw - 6, mh - 7, 3,
        Color((BYTE)(235 * alpha), 95, 170, 240),
        Color((BYTE)(235 * alpha), 60, 135, 210));
    FillRoundRectSolid(g, mx + 4, my + 4, (int)((mw - 8) * 0.4f), (int)((mh - 8) * 0.3f), 1,
        Color((BYTE)(30 * alpha), 255, 255, 255));
    SolidBrush standBr(Color((BYTE)(210 * alpha), 100, 100, 112));
    g.FillRectangle(&standBr, x + size / 2 - 3, my + mh, 6, (int)(size * 0.12f));
    FillRoundRectSolid(g, x + size / 2 - 9, my + mh + (int)(size * 0.10f), 18, 3, 1,
        Color((BYTE)(210 * alpha), 90, 90, 102));
}

void DrawRecycleBinIcon(Graphics& g, int x, int y, int size, float alpha = 1.0f) {
    int bw = (int)(size * 0.48f);
    int bh = (int)(size * 0.58f);
    int bx = x + (size - bw) / 2;
    int by = y + (int)(size * 0.22f);
    FillRoundRectSolid(g, bx + 2, by + 3, bw, bh, 4, Color((BYTE)(30 * alpha), 0, 0, 0));
    FillRoundRectGradient(g, bx, by, bw, bh, 4,
        Color((BYTE)(225 * alpha), 135, 135, 148),
        Color((BYTE)(225 * alpha), 108, 108, 118));
    Pen lidPen(Color((BYTE)(235 * alpha), 155, 155, 168), 2.5f);
    g.DrawLine(&lidPen, bx - 3, by, bx + bw + 3, by);
    GraphicsPath handlePath;
    handlePath.AddArc(x + size / 2 - 6, by - 8, 12, 10, 180, 180);
    Pen handlePen(Color((BYTE)(235 * alpha), 150, 150, 162), 2.0f);
    g.DrawPath(&handlePen, &handlePath);
    Pen lp(Color((BYTE)(100 * alpha), 75, 75, 88), 1.2f);
    for (int li = 0; li < 3; li++) {
        int lx = bx + bw / 4 + li * bw / 4;
        g.DrawLine(&lp, lx, by + 7, lx, by + bh - 5);
    }
}

// ============================================================================
//  DRAW BOOT SEQUENCE — Windows-style loading
// ============================================================================
void UpdateBoot(float dt) {
    if (g_boot.complete) return;

    g_boot.totalTime += dt;
    g_boot.phaseTime += dt;

    switch (g_boot.phase) {
    case BOOT_BLACK:
        if (g_boot.phaseTime >= BOOT_BLACK_DUR) {
            g_boot.phase = BOOT_LOGO_FADE_IN;
            g_boot.phaseTime = 0;
        }
        break;

    case BOOT_LOGO_FADE_IN:
        g_boot.logoAlpha = Clamp01(g_boot.phaseTime / BOOT_LOGO_FADE_DUR);
        if (g_boot.phaseTime >= BOOT_LOGO_FADE_DUR) {
            g_boot.phase = BOOT_SPINNER;
            g_boot.phaseTime = 0;
        }
        break;

    case BOOT_SPINNER:
        g_boot.logoAlpha = 1.0f;
        g_boot.spinnerAlpha = Clamp01(g_boot.phaseTime / 0.5f);
        g_boot.spinnerAngle += dt * 320.0f;
        g_boot.progressValue = Clamp01(g_boot.phaseTime / BOOT_SPINNER_DUR);

        // Animate individual dots
        for (int i = 0; i < 12; i++) {
            float phase = g_boot.phaseTime * 3.0f - i * 0.15f;
            g_boot.dotPulse[i] = Clamp01(sinf(phase) * 0.5f + 0.5f);
        }

        if (g_boot.phaseTime >= BOOT_SPINNER_DUR) {
            g_boot.phase = BOOT_PREPARING;
            g_boot.phaseTime = 0;
            g_boot.currentTextIdx = 1;
        }
        break;

    case BOOT_PREPARING:
        g_boot.spinnerAngle += dt * 320.0f;
        g_boot.textAlpha = Clamp01(g_boot.phaseTime / 0.6f);
        for (int i = 0; i < 12; i++) {
            float phase = g_boot.phaseTime * 3.0f - i * 0.15f;
            g_boot.dotPulse[i] = Clamp01(sinf(phase) * 0.5f + 0.5f);
        }
        if (g_boot.phaseTime >= BOOT_PREPARING_DUR) {
            g_boot.phase = BOOT_WELCOME;
            g_boot.phaseTime = 0;
        }
        break;

    case BOOT_WELCOME:
        g_boot.welcomeAlpha = Clamp01(g_boot.phaseTime / 0.8f);
        g_boot.spinnerAlpha = 1.0f - Clamp01(g_boot.phaseTime / 0.5f);
        g_boot.textAlpha = 1.0f - Clamp01(g_boot.phaseTime / 0.4f);
        if (g_boot.phaseTime >= BOOT_WELCOME_DUR) {
            g_boot.phase = BOOT_FADE_TO_DESKTOP;
            g_boot.phaseTime = 0;
            g_desktopFadeIn.SetTarget(1.0f);
        }
        break;

    case BOOT_FADE_TO_DESKTOP:
        g_boot.fadeOutAlpha = 1.0f - Clamp01(g_boot.phaseTime / BOOT_FADE_DUR);
        if (g_boot.phaseTime >= BOOT_FADE_DUR) {
            g_boot.phase = BOOT_COMPLETE;
            g_boot.complete = true;
        }
        break;

    case BOOT_COMPLETE:
        break;
    }
}

void DrawBootScreen(Graphics& g, int sw, int sh) {
    if (g_boot.phase == BOOT_COMPLETE && g_boot.fadeOutAlpha <= 0.01f) return;

    float masterAlpha = (g_boot.phase == BOOT_FADE_TO_DESKTOP) ? g_boot.fadeOutAlpha : 1.0f;

    // Black background
    SolidBrush bgBr(Color((BYTE)(255 * masterAlpha), 0, 0, 0));
    g.FillRectangle(&bgBr, 0, 0, sw, sh);

    // Subtle gradient overlay for depth
    if (masterAlpha > 0.5f) {
        for (int i = 0; i < 3; i++) {
            float t2 = g_boot.totalTime * 0.05f + i * 2.0f;
            int cx = sw / 2 + (int)(sinf(t2) * sw / 8);
            int cy = sh / 2 + (int)(cosf(t2 * 0.7f) * sh / 8);
            int cr = 400 + i * 100;
            BYTE ba = (BYTE)(4 * masterAlpha);
            SolidBrush blob(Color(ba, 20, 60, 120));
            g.FillEllipse(&blob, cx - cr, cy - cr, cr * 2, cr * 2);
        }
    }

    FontFamily ff(L"Segoe UI");
    StringFormat cfmt;
    cfmt.SetAlignment(StringAlignmentCenter);
    cfmt.SetLineAlignment(StringAlignmentCenter);

    int centerX = sw / 2;
    int centerY = sh / 2 - 40;

    // ========= LOGO =========
    if (g_boot.logoAlpha > 0.01f) {
        float la = g_boot.logoAlpha * masterAlpha;

        // Logo glow behind
        float glowPulse = 0.6f + 0.4f * sinf(g_boot.totalTime * 1.5f);
        for (int i = 0; i < 8; i++) {
            int pad = 10 + i * 8;
            BYTE ga = (BYTE)(la * glowPulse * (12.0f - i * 1.4f));
            SolidBrush glowBr(Color(ga, 88, 186, 255));
            g.FillEllipse(&glowBr, centerX - pad, centerY - pad, pad * 2, pad * 2);
        }

        // Windows 12 logo — 4 squares with rounded corners and gradient
        int logoSize = 52;
        int lx = centerX - logoSize / 2;
        int ly = centerY - logoSize / 2;
        int gap = 4;
        int sq = (logoSize - gap) / 2;

        // Square shadows
        for (int s = 0; s < 2; s++) {
            int off = 2 + s;
            FillRoundRectSolid(g, lx + off, ly + off, sq, sq, 4, Color((BYTE)(20 * la), 0, 0, 0));
            FillRoundRectSolid(g, lx + sq + gap + off, ly + off, sq, sq, 4, Color((BYTE)(20 * la), 0, 0, 0));
            FillRoundRectSolid(g, lx + off, ly + sq + gap + off, sq, sq, 4, Color((BYTE)(20 * la), 0, 0, 0));
            FillRoundRectSolid(g, lx + sq + gap + off, ly + sq + gap + off, sq, sq, 4, Color((BYTE)(20 * la), 0, 0, 0));
        }

        // The four colored squares with gradient
        Color sqColors[4] = {
            Color((BYTE)(255 * la), 88, 186, 255),
            Color((BYTE)(255 * la), 72, 172, 255),
            Color((BYTE)(255 * la), 100, 178, 255),
            Color((BYTE)(255 * la), 80, 180, 255)
        };

        FillRoundRectGradient(g, lx, ly, sq, sq, 4, sqColors[0],
            Color((BYTE)(255 * la), 60, 150, 230));
        FillRoundRectGradient(g, lx + sq + gap, ly, sq, sq, 4, sqColors[1],
            Color((BYTE)(255 * la), 50, 140, 220));
        FillRoundRectGradient(g, lx, ly + sq + gap, sq, sq, 4, sqColors[2],
            Color((BYTE)(255 * la), 70, 145, 225));
        FillRoundRectGradient(g, lx + sq + gap, ly + sq + gap, sq, sq, 4, sqColors[3],
            Color((BYTE)(255 * la), 55, 148, 218));

        // Specular on each
        FillRoundRectSolid(g, lx + 2, ly + 1, sq - 4, sq / 3, 3, Color((BYTE)(35 * la), 255, 255, 255));
        FillRoundRectSolid(g, lx + sq + gap + 2, ly + 1, sq - 4, sq / 3, 3, Color((BYTE)(30 * la), 255, 255, 255));
        FillRoundRectSolid(g, lx + 2, ly + sq + gap + 1, sq - 4, sq / 3, 3, Color((BYTE)(28 * la), 255, 255, 255));
        FillRoundRectSolid(g, lx + sq + gap + 2, ly + sq + gap + 1, sq - 4, sq / 3, 3, Color((BYTE)(25 * la), 255, 255, 255));

        // Brand text
        if (g_boot.phase >= BOOT_SPINNER) {
            float textFade = Clamp01((g_boot.phase == BOOT_SPINNER ? g_boot.phaseTime : 999) / 1.0f);
            Font brandFont(&ff, 28.0f, FontStyleBold, UnitPixel);
            SolidBrush brandBr(Color((BYTE)(255 * la * textFade * masterAlpha), 255, 255, 255));
            RectF brandRc((REAL)(centerX - 200), (REAL)(centerY + logoSize / 2 + 20), 400.0f, 40.0f);
            g.DrawString(L"VORTEX", -1, &brandFont, brandRc, &cfmt, &brandBr);

            Font subFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
            SolidBrush subBr(Color((BYTE)(140 * la * textFade * masterAlpha), 170, 185, 210));
            RectF subRc((REAL)(centerX - 200), (REAL)(centerY + logoSize / 2 + 55), 400.0f, 20.0f);
            g.DrawString(L"Desktop Environment", -1, &subFont, subRc, &cfmt, &subBr);
        }
    }

    // ========= SPINNING DOTS (Windows-style) =========
    if (g_boot.spinnerAlpha > 0.01f && (g_boot.phase == BOOT_SPINNER || g_boot.phase == BOOT_PREPARING || g_boot.phase == BOOT_WELCOME)) {
        float sa = g_boot.spinnerAlpha * masterAlpha;
        int spinCX = centerX;
        int spinCY = centerY + 120;
        int numDots = 5;
        float dotRadius = 18.0f;
        float dotSize = 4.0f;

        for (int i = 0; i < numDots; i++) {
            // Each dot has a different phase offset, creating the chasing effect
            float angleOffset = g_boot.spinnerAngle - i * 45.0f;
            float rad = angleOffset * 3.14159265f / 180.0f;
            float dx = cosf(rad) * dotRadius;
            float dy = sinf(rad) * dotRadius;

            // Trail effect — dots closer to lead are brighter
            float brightness = 1.0f - (float)i / (float)numDots * 0.7f;
            float scale = 1.0f - (float)i / (float)numDots * 0.4f;

            BYTE alpha = (BYTE)(255 * sa * brightness);
            float sz = dotSize * scale;

            // Glow
            for (int gl = 0; gl < 3; gl++) {
                float glsz = sz + gl * 3.0f;
                BYTE ga = (BYTE)(sa * brightness * (20.0f - gl * 6.0f));
                SolidBrush glBr(Color(ga, 88, 186, 255));
                g.FillEllipse(&glBr, (REAL)(spinCX + dx - glsz), (REAL)(spinCY + dy - glsz),
                    glsz * 2.0f, glsz * 2.0f);
            }

            SolidBrush dotBr(Color(alpha, 200, 230, 255));
            g.FillEllipse(&dotBr, (REAL)(spinCX + dx - sz), (REAL)(spinCY + dy - sz),
                sz * 2.0f, sz * 2.0f);
        }
    }

    // ========= STATUS TEXT =========
    if (g_boot.phase == BOOT_PREPARING && g_boot.textAlpha > 0.01f) {
        float ta = g_boot.textAlpha * masterAlpha;
        Font statusFont(&ff, 14.0f, FontStyleRegular, UnitPixel);
        SolidBrush statusBr(Color((BYTE)(200 * ta), 180, 190, 210));
        RectF statusRc((REAL)(centerX - 200), (REAL)(centerY + 155), 400.0f, 24.0f);
        g.DrawString(L"Getting things ready...", -1, &statusFont, statusRc, &cfmt, &statusBr);
    }

    // ========= WELCOME =========
    if (g_boot.phase == BOOT_WELCOME && g_boot.welcomeAlpha > 0.01f) {
        float wa = g_boot.welcomeAlpha * masterAlpha;
        float scaleEff = 0.9f + 0.1f * pOutCubic(Clamp01(g_boot.welcomeAlpha));

        Matrix mtx;
        mtx.Translate((REAL)centerX, (REAL)(centerY + 60));
        mtx.Scale(scaleEff, scaleEff);
        mtx.Translate((REAL)-centerX, (REAL)-(centerY + 60));
        g.SetTransform(&mtx);

        Font welcomeFont(&ff, 32.0f, FontStyleBold, UnitPixel);
        SolidBrush welcomeBr(Color((BYTE)(255 * wa), 255, 255, 255));
        RectF welcomeRc((REAL)(centerX - 250), (REAL)(centerY + 40), 500.0f, 50.0f);
        g.DrawString(L"Welcome", -1, &welcomeFont, welcomeRc, &cfmt, &welcomeBr);

        g.ResetTransform();
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

    if (sa > 0.01f || ha > 0.01f) {
        BYTE bgA = (BYTE)((std::max)(sa * 55.0f, ha * 28.0f));
        BYTE bgR = (BYTE)(sa * 0 + (1 - sa) * 255);
        BYTE bgG = (BYTE)(sa * 85 + (1 - sa) * 255);
        BYTE bgB = (BYTE)(sa * 185 + (1 - sa) * 255);
        FillRoundRectSolid(g, x - 1, y - 1, DESKTOP_ICON_W + 2, DESKTOP_ICON_H + 2, 8,
            Color(bgA, bgR, bgG, bgB));
        if (sa > 0.1f)
            DrawRoundRect(g, x - 1, y - 1, DESKTOP_ICON_W + 2, DESKTOP_ICON_H + 2, 8,
                Color((BYTE)(sa * 70), 88, 186, 255), 1.0f);
    }

    int icoSize = 44;
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

    SolidBrush shadowBr(Color(180, 0, 0, 0));
    RectF shadowRc((REAL)(x - 3), (REAL)(y + DESKTOP_ICON_H - 28 + 1), (REAL)(DESKTOP_ICON_W + 6), 28.0f);
    g.DrawString(d.name.c_str(), -1, &nameFont, shadowRc, &cfmt, &shadowBr);

    SolidBrush textBr(W12::TextPrimary);
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
    SolidBrush fillBr(W12::SelectionRect);
    g.FillRectangle(&fillBr, x1, y1, w, h);
    Pen borderPen(W12::SelectionBorder, 1.2f);
    g.DrawRectangle(&borderPen, x1, y1, w, h);
}

// ============================================================================
//  DRAW EXPLORER WINDOW (W12 style)
// ============================================================================
void DrawExplorerWindow(Graphics& g, ExplorerWindow& win) {
    float a = win.animAlpha.current;
    float sc = win.animScale.current;
    if (a <= 0.01f) return;

    int wx = win.x, wy = win.y, ww = win.w, wh = win.h;
    bool isActive = (win.id == g_activeExplorerId);

    if (fabsf(sc - 1.0f) > 0.002f) {
        int cx = wx + ww / 2, cy = wy + wh / 2;
        Matrix mtx;
        mtx.Translate((REAL)cx, (REAL)cy);
        mtx.Scale(sc, sc);
        mtx.Translate((REAL)-cx, (REAL)-cy);
        g.SetTransform(&mtx);
    }

    DrawPremiumShadow(g, wx, wy, ww, wh, 10, a);

    // Window body with subtle gradient
    FillRoundRectGradient(g, wx, wy, ww, wh, 10,
        Color((BYTE)(254 * a), 22, 22, 26),
        Color((BYTE)(254 * a), 18, 18, 22));
    DrawRoundRect(g, wx, wy, ww, wh, 10,
        isActive ? Color((BYTE)(50 * a), 255, 255, 255) : Color((BYTE)(20 * a), 255, 255, 255), 1.0f);

    // Accent top line for active window
    if (isActive) {
        Pen accentTop(Color((BYTE)(80 * a), 88, 186, 255), 1.5f);
        GraphicsPath topLine;
        topLine.AddArc(wx + 10, wy, 20, 2, 180, 90);
        topLine.AddLine(wx + 20, wy, wx + ww - 20, wy);
        topLine.AddArc(wx + ww - 30, wy, 20, 2, 270, 90);
        g.DrawPath(&accentTop, &topLine);
    }

    FontFamily ff(L"Segoe UI");
    StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat cfmt; cfmt.SetAlignment(StringAlignmentCenter); cfmt.SetLineAlignment(StringAlignmentCenter);

    int titleH = 36;
    DrawFolderIcon(g, wx + 14, wy + 8, 20, Color(255, 255, 213, 79), a);
    Font titleFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    SolidBrush titleBr(Color((BYTE)(215 * a), 215, 215, 228));
    RectF titleRc((REAL)(wx + 40), (REAL)wy, 300.0f, (REAL)titleH);
    g.DrawString(win.title.c_str(), -1, &titleFont, titleRc, &lfmt, &titleBr);

    // Window control buttons — W12 pill style
    int btnW = 46, btnH = titleH;
    int btnX = wx + ww - btnW * 3;

    // Minimize
    {
        bool hov = (win.hoveredTitleBtn == 1);
        if (hov) FillRoundRectSolid(g, btnX + 4, wy + 4, btnW - 8, btnH - 8, 6,
            Color((BYTE)(140 * a), 55, 55, 62));
        Pen p(Color((BYTE)(190 * a), 195, 200, 215), 1.0f);
        g.DrawLine(&p, btnX + btnW / 2 - 5, wy + btnH / 2, btnX + btnW / 2 + 5, wy + btnH / 2);
    }
    btnX += btnW;
    // Maximize
    {
        bool hov = (win.hoveredTitleBtn == 2);
        if (hov) FillRoundRectSolid(g, btnX + 4, wy + 4, btnW - 8, btnH - 8, 6,
            Color((BYTE)(140 * a), 55, 55, 62));
        Pen p(Color((BYTE)(190 * a), 195, 200, 215), 1.0f);
        g.DrawRectangle(&p, btnX + btnW / 2 - 5, wy + btnH / 2 - 5, 10, 10);
    }
    btnX += btnW;
    // Close
    {
        bool hov = (win.hoveredTitleBtn == 3);
        if (hov) FillRoundRectSolid(g, btnX + 4, wy + 4, btnW - 8, btnH - 8, 6,
            Color((BYTE)(210 * a), 196, 43, 28));
        Pen p(Color((BYTE)(190 * a), 195, 200, 215), 1.3f);
        g.DrawLine(&p, btnX + btnW / 2 - 5, wy + btnH / 2 - 5, btnX + btnW / 2 + 5, wy + btnH / 2 + 5);
        g.DrawLine(&p, btnX + btnW / 2 + 5, wy + btnH / 2 - 5, btnX + btnW / 2 - 5, wy + btnH / 2 + 5);
    }

    // Toolbar
    int toolY = wy + titleH;
    int toolH = 42;
    FillRoundRectSolid(g, wx, toolY, ww, toolH, 0, Color((BYTE)(250 * a), 24, 24, 28));

    int navX = wx + 14;
    int navBtnSize = 28;

    auto drawNavBtn = [&](bool enabled, int type) {
        Color c = enabled ? Color((BYTE)(200 * a), 195, 200, 215) : Color((BYTE)(40 * a), 100, 100, 110);
        Pen p(c, 1.5f);
        p.SetStartCap(LineCapRound);
        p.SetEndCap(LineCapRound);
        int cx2 = navX + navBtnSize / 2, cy2 = toolY + toolH / 2;
        if (type == 0) {
            g.DrawLine(&p, cx2 + 3, cy2 - 5, cx2 - 3, cy2);
            g.DrawLine(&p, cx2 - 3, cy2, cx2 + 3, cy2 + 5);
        }
        else if (type == 1) {
            g.DrawLine(&p, cx2 - 3, cy2 - 5, cx2 + 3, cy2);
            g.DrawLine(&p, cx2 + 3, cy2, cx2 - 3, cy2 + 5);
        }
        else {
            g.DrawLine(&p, cx2 - 5, cy2 + 2, cx2, cy2 - 4);
            g.DrawLine(&p, cx2, cy2 - 4, cx2 + 5, cy2 + 2);
        }
        };

    drawNavBtn(win.historyIndex > 0, 0);
    navX += navBtnSize + 4;
    drawNavBtn(win.historyIndex < (int)win.pathHistory.size() - 1, 1);
    navX += navBtnSize + 4;
    drawNavBtn(!win.currentPath.empty(), 2);
    navX += navBtnSize + 16;

    // Address bar — W12 frosted pill
    int addrX = navX, addrW = ww - (navX - wx) - 18, addrH = 30;
    int addrY = toolY + (toolH - addrH) / 2;

    FillRoundRectSolid(g, addrX, addrY, addrW, addrH, addrH / 2,
        Color((BYTE)(250 * a), 38, 38, 44));
    DrawRoundRect(g, addrX, addrY, addrW, addrH, addrH / 2,
        Color((BYTE)(25 * a), 255, 255, 255), 0.8f);

    // Bottom accent line
    Pen accentLine(Color((BYTE)(80 * a), 88, 186, 255), 1.5f);
    g.DrawLine(&accentLine, addrX + addrH / 2, addrY + addrH - 1,
        addrX + addrW - addrH / 2, addrY + addrH - 1);

    Font addrFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    SolidBrush addrBr(Color((BYTE)(195 * a), 195, 200, 215));
    std::wstring addrText = win.currentPath.empty() ? L"This PC" : win.currentPath;
    RectF addrRc((REAL)(addrX + 14), (REAL)addrY, (REAL)(addrW - 28), (REAL)addrH);
    cfmt.SetAlignment(StringAlignmentNear);
    g.DrawString(addrText.c_str(), -1, &addrFont, addrRc, &cfmt, &addrBr);
    cfmt.SetAlignment(StringAlignmentCenter);

    // Sidebar
    int sidebarW = 205;
    int contentX = wx + sidebarW;
    int contentY = toolY + toolH;
    int contentW = ww - sidebarW;
    int contentH = wh - titleH - toolH;

    SolidBrush sidebarBg(Color((BYTE)(250 * a), 22, 22, 27));
    g.FillRectangle(&sidebarBg, wx, contentY, sidebarW, contentH);

    Pen sepPen(Color((BYTE)(16 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&sepPen, contentX, contentY, contentX, wy + wh);
    g.DrawLine(&sepPen, wx, contentY, wx + ww, contentY);

    struct SideItem { std::wstring label; int icon; };
    SideItem sideItems[] = {
        { L"Desktop", 0 }, { L"Downloads", 1 }, { L"Documents", 2 },
        { L"Pictures", 3 }, { L"Music", 4 }, { L"Videos", 5 }, { L"This PC", 6 },
    };

    Font sideFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    Font headerFont(&ff, 10.5f, FontStyleBold, UnitPixel);
    int sideY = contentY + 12;

    SolidBrush headerBr(Color((BYTE)(90 * a), 145, 148, 158));
    RectF headerRc((REAL)(wx + 18), (REAL)sideY, (REAL)(sidebarW - 28), 18.0f);
    g.DrawString(L"QUICK ACCESS", -1, &headerFont, headerRc, &lfmt, &headerBr);
    sideY += 28;

    for (int i = 0; i < 7; i++) {
        bool hov = (win.hoveredSidebarItem == i);
        int itemH = 32;

        if (hov) {
            FillRoundRectSolid(g, wx + 6, sideY, sidebarW - 12, itemH, 6,
                Color((BYTE)(200 * a), 42, 42, 48));
        }

        if (i < 6) {
            Color fc(255, 255, 213, 79);
            if (i == 1) fc = Color(255, 88, 186, 255);
            else if (i == 3) fc = Color(255, 172, 135, 255);
            else if (i == 4) fc = Color(255, 255, 105, 135);
            else if (i == 5) fc = Color(255, 100, 200, 130);
            DrawFolderIcon(g, wx + 18, sideY + 4, 22, fc, a);
        }
        else {
            DrawPCIcon(g, wx + 18, sideY + 4, 22, a);
        }

        SolidBrush sideBr(Color((BYTE)(215 * a), 210, 212, 222));
        RectF sideRc((REAL)(wx + 46), (REAL)sideY, (REAL)(sidebarW - 58), (REAL)itemH);
        g.DrawString(sideItems[i].label.c_str(), -1, &sideFont, sideRc, &lfmt, &sideBr);

        if (i == 5) {
            sideY += itemH + 8;
            Pen sp(Color((BYTE)(14 * a), 255, 255, 255), 1.0f);
            g.DrawLine(&sp, wx + 16, sideY, wx + sidebarW - 16, sideY);
            sideY += 12;
            SolidBrush hdr2(Color((BYTE)(90 * a), 145, 148, 158));
            RectF hdr2Rc((REAL)(wx + 18), (REAL)sideY, (REAL)(sidebarW - 28), 18.0f);
            g.DrawString(L"THIS PC", -1, &headerFont, hdr2Rc, &lfmt, &hdr2);
            sideY += 28;
        }
        else {
            sideY += itemH;
        }
    }

    // Content area
    g.SetClip(Rect(contentX + 1, contentY + 1, contentW - 2, contentH - 2));
    SolidBrush contentBg(Color((BYTE)(250 * a), 18, 18, 22));
    g.FillRectangle(&contentBg, contentX, contentY, contentW, contentH);

    int scrollOff = (int)win.scrollAnim.current;
    StringFormat rfmt; rfmt.SetAlignment(StringAlignmentFar); rfmt.SetLineAlignment(StringAlignmentCenter);

    if (win.currentPath.empty()) {
        Font secFont(&ff, 13.0f, FontStyleBold, UnitPixel);
        SolidBrush secBr(Color((BYTE)(165 * a), 195, 200, 215));
        int cy = contentY + 20 - scrollOff;
        RectF secRc((REAL)(contentX + 24), (REAL)cy, 200.0f, 24.0f);
        g.DrawString(L"Devices and drives", -1, &secFont, secRc, &lfmt, &secBr);
        cy += 40;

        int tileW = 248, tileH = 76;
        int cols = (std::max)(1, (contentW - 48) / (tileW + 12));

        for (int i = 0; i < (int)win.items.size(); i++) {
            const FileItem& fi = win.items[i];
            int col = i % cols, row = i / cols;
            int tx = contentX + 24 + col * (tileW + 12);
            int ty = cy + row * (tileH + 12);

            bool hov = (win.hoveredItem == i);
            bool sel = (win.selectedItem == i);

            Color bg = sel ? Color((BYTE)(235 * a), 0, 60, 150) :
                hov ? Color((BYTE)(225 * a), 42, 42, 48) :
                Color((BYTE)(210 * a), 32, 32, 38);

            FillRoundRectSolid(g, tx, ty, tileW, tileH, 10, bg);
            DrawRoundRect(g, tx, ty, tileW, tileH, 10, Color((BYTE)(14 * a), 255, 255, 255), 0.8f);

            float usedPct = fi.totalSpace > 0 ? (float)(fi.totalSpace - fi.freeSpace) / (float)fi.totalSpace : 0;
            DrawDriveIcon(g, tx + 12, ty + 12, 46, usedPct, a);

            Font driveFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
            SolidBrush driveBr(Color((BYTE)(238 * a), 238, 240, 248));
            RectF driveRc((REAL)(tx + 66), (REAL)(ty + 10), (REAL)(tileW - 78), 20.0f);
            cfmt.SetAlignment(StringAlignmentNear);
            g.DrawString(fi.name.c_str(), -1, &driveFont, driveRc, &cfmt, &driveBr);

            if (fi.totalSpace > 0) {
                std::wstring spaceStr = FormatSize(fi.freeSpace) + L" free of " + FormatSize(fi.totalSpace);
                Font spaceFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
                SolidBrush spaceBr(Color((BYTE)(125 * a), 150, 155, 165));
                RectF spaceRc((REAL)(tx + 66), (REAL)(ty + 30), (REAL)(tileW - 78), 16.0f);
                g.DrawString(spaceStr.c_str(), -1, &spaceFont, spaceRc, &cfmt, &spaceBr);

                int barX2 = tx + 66, barY2 = ty + 52, barW2 = tileW - 82, barH2 = 6;
                FillRoundRectSolid(g, barX2, barY2, barW2, barH2, 3, Color((BYTE)(135 * a), 44, 44, 50));
                int fillW2 = (int)(barW2 * usedPct);
                if (fillW2 > 0) {
                    Color bc1 = usedPct > 0.9f ? Color((BYTE)(235 * a), 255, 80, 80) :
                        usedPct > 0.7f ? Color((BYTE)(235 * a), 255, 210, 50) :
                        Color((BYTE)(235 * a), 88, 200, 255);
                    Color bc2 = usedPct > 0.9f ? Color((BYTE)(235 * a), 220, 50, 50) :
                        usedPct > 0.7f ? Color((BYTE)(235 * a), 230, 180, 30) :
                        Color((BYTE)(235 * a), 45, 155, 230);
                    FillRoundRectGradient(g, barX2, barY2, fillW2, barH2, 3, bc1, bc2, false);
                }
            }
            cfmt.SetAlignment(StringAlignmentCenter);
        }
    }
    else {
        int itemH = 34;
        int headerY = contentY + 4;
        Font hdrFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
        SolidBrush hdrBr(Color((BYTE)(120 * a), 155, 158, 168));

        RectF nameHdr((REAL)(contentX + 50), (REAL)headerY, 300.0f, 24.0f);
        g.DrawString(L"Name", -1, &hdrFont, nameHdr, &lfmt, &hdrBr);
        RectF sizeHdr((REAL)(contentX + contentW - 255), (REAL)headerY, 100.0f, 24.0f);
        g.DrawString(L"Size", -1, &hdrFont, sizeHdr, &lfmt, &hdrBr);
        RectF typeHdr((REAL)(contentX + contentW - 145), (REAL)headerY, 120.0f, 24.0f);
        g.DrawString(L"Type", -1, &hdrFont, typeHdr, &lfmt, &hdrBr);

        Pen hdrLine(Color((BYTE)(14 * a), 255, 255, 255), 1.0f);
        g.DrawLine(&hdrLine, contentX + 14, headerY + 28, contentX + contentW - 14, headerY + 28);

        int listY = headerY + 32 - scrollOff;
        Font itemFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
        Font smallFont(&ff, 10.0f, FontStyleRegular, UnitPixel);

        for (int i = 0; i < (int)win.items.size(); i++) {
            int iy = listY + i * itemH;
            if (iy + itemH < contentY || iy > wy + wh) continue;

            const FileItem& fi = win.items[i];
            bool hov = (win.hoveredItem == i);
            bool sel = (win.selectedItem == i);

            if (sel) {
                FillRoundRectSolid(g, contentX + 4, iy, contentW - 8, itemH - 1, 6,
                    Color((BYTE)(225 * a), 0, 60, 150));
            }
            else if (hov) {
                FillRoundRectSolid(g, contentX + 4, iy, contentW - 8, itemH - 1, 6,
                    Color((BYTE)(200 * a), 40, 40, 46));
            }

            if (fi.isDirectory) {
                if (fi.name == L"..") {
                    Pen upPen(Color((BYTE)(200 * a), 195, 200, 215), 1.5f);
                    int cx2 = contentX + 30, cy2 = iy + itemH / 2;
                    g.DrawLine(&upPen, cx2, cy2 + 4, cx2, cy2 - 4);
                    g.DrawLine(&upPen, cx2 - 4, cy2, cx2, cy2 - 4);
                    g.DrawLine(&upPen, cx2 + 4, cy2, cx2, cy2 - 4);
                }
                else {
                    DrawFolderIcon(g, contentX + 18, iy + 4, 24, Color(255, 255, 213, 79), a);
                }
            }
            else {
                std::wstring ext;
                size_t dotPos = fi.name.find_last_of(L'.');
                if (dotPos != std::wstring::npos) ext = fi.name.substr(dotPos);
                for (auto& ch : ext) ch = towlower(ch);
                DrawFileIcon(g, contentX + 18, iy + 4, 24, ext, a);
            }

            SolidBrush nameBr(Color((BYTE)(238 * a), 238, 240, 248));
            RectF nameRc((REAL)(contentX + 50), (REAL)iy, (REAL)(contentW - 320), (REAL)itemH);
            g.DrawString(fi.name.c_str(), -1, &itemFont, nameRc, &lfmt, &nameBr);

            if (!fi.isDirectory && fi.fileSize > 0) {
                SolidBrush sizeBr(Color((BYTE)(145 * a), 155, 158, 168));
                RectF sizeRc((REAL)(contentX + contentW - 255), (REAL)iy, 100.0f, (REAL)itemH);
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
            SolidBrush typeBr(Color((BYTE)(115 * a), 148, 150, 158));
            RectF typeRc((REAL)(contentX + contentW - 145), (REAL)iy, 120.0f, (REAL)itemH);
            g.DrawString(typeStr.c_str(), -1, &smallFont, typeRc, &lfmt, &typeBr);
        }

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

            FillRoundRectSolid(g, sbX, sbY, 6, sbH, 3, Color((BYTE)(25 * a), 255, 255, 255));
            FillRoundRectSolid(g, sbX, thumbY, 6, thumbH, 3, Color((BYTE)(85 * a), 135, 138, 148));
        }
    }

    // Status bar
    Font countFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
    SolidBrush countBr(Color((BYTE)(85 * a), 135, 138, 148));
    wchar_t countStr[128];
    int dirCount = 0, fileCount = 0;
    for (auto& fi : win.items) {
        if (fi.name == L"..") continue;
        if (fi.isDirectory) dirCount++; else fileCount++;
    }
    if (win.currentPath.empty())
        swprintf_s(countStr, L"%d items", (int)win.items.size());
    else
        swprintf_s(countStr, L"%d folders, %d files", dirCount, fileCount);
    RectF countRc((REAL)(contentX + 14), (REAL)(wy + wh - 28), (REAL)(contentW - 28), 20.0f);
    lfmt.SetLineAlignment(StringAlignmentCenter);
    g.DrawString(countStr, -1, &countFont, countRc, &lfmt, &countBr);

    g.ResetClip();
    if (fabsf(sc - 1.0f) > 0.002f) g.ResetTransform();
}

// ============================================================================
//  TASKBAR — W12 Ultra-premium floating glass design
// ============================================================================
void DrawTaskbar(Graphics& g, int sw, int sh) {
    int barY = sh - TASKBAR_HEIGHT;
    int barH = TASKBAR_HEIGHT;
    float t = g_time;

    // === W12 FLOATING TASKBAR with pill shape ===
    int taskbarMargin = 6;
    int taskbarX = taskbarMargin;
    int taskbarW = sw - taskbarMargin * 2;
    int taskbarR = 14;

    // Multi-layer shadow for floating effect
    for (int i = 12; i >= 1; i--) {
        int sp = i * 2;
        float st = (float)i / 12.0f;
        int alpha = (int)(12.0f * (1.0f - st));
        FillRoundRectSolid(g, taskbarX - sp, barY - sp / 2 + sp, taskbarW + sp * 2, barH + sp,
            taskbarR + sp / 2, Color((BYTE)alpha, 0, 0, 0));
    }

    // Base solid dark
    FillRoundRectSolid(g, taskbarX, barY, taskbarW, barH, taskbarR,
        Color(255, 14, 14, 18));

    // Gradient overlay
    FillRoundRectGradient(g, taskbarX, barY, taskbarW, barH, taskbarR,
        Color(255, 22, 22, 28), Color(255, 16, 16, 20));

    // Glass membrane top edge
    GraphicsPath glassEdge;
    glassEdge.AddArc(taskbarX + taskbarR, barY - 1, taskbarR * 2, 4, 180, 180);
    glassEdge.AddLine(taskbarX + taskbarR * 2, barY + 1, taskbarX + taskbarW - taskbarR * 2, barY + 1);
    glassEdge.AddArc(taskbarX + taskbarW - taskbarR * 3, barY - 1, taskbarR * 2, 4, 0, 180);
    // Simplified — just a line
    Pen rimPen(Color(50, 180, 190, 220), 0.8f);
    g.DrawLine(&rimPen, taskbarX + taskbarR, barY, taskbarX + taskbarW - taskbarR, barY);

    // Border
    DrawRoundRect(g, taskbarX, barY, taskbarW, barH, taskbarR,
        Color(14, 255, 255, 255), 0.6f);

    FontFamily ff(L"Segoe UI");

    int iconCount = (int)g_taskApps.size();
    int totalW = (iconCount + 3) * (TASKBAR_ICON_SIZE + 8);
    int startX = (sw - totalW) / 2;
    int iconY = barY + (barH - TASKBAR_ICON_SIZE) / 2;

    // === START BUTTON ===
    {
        int bx = startX;
        int by = iconY;
        float ha = g_startBtnHover.current;
        bool isOpen = g_startMenuOpen;

        float bgI = isOpen ? 0.65f : ha * 0.5f;
        if (bgI > 0.005f) {
            for (int i = 0; i < 3; i++) {
                int pad = 5 + i * 3;
                BYTE ga = (BYTE)(bgI * (9.0f - i * 2.8f));
                FillRoundRectSolid(g, bx - pad, by - pad, TASKBAR_ICON_SIZE + pad * 2,
                    TASKBAR_ICON_SIZE + pad * 2, 14 + i * 2, Color(ga, 88, 186, 255));
            }
            FillRoundRectGradient(g, bx - 2, by - 2, TASKBAR_ICON_SIZE + 4, TASKBAR_ICON_SIZE + 4, 13,
                Color((BYTE)(bgI * 150), 48, 52, 62),
                Color((BYTE)(bgI * 110), 34, 36, 44));
        }

        int logoSize = 16;
        int lx = bx + (TASKBAR_ICON_SIZE - logoSize) / 2;
        int ly = by + (TASKBAR_ICON_SIZE - logoSize) / 2;
        int gap = 2;
        int sq = (logoSize - gap) / 2;

        float logoGlow = isOpen ? 1.0f : ha;
        auto makeLogoColor = [&](int baseR, int baseG, int baseB) -> Color {
            float gl = logoGlow * 0.35f;
            return Color(255,
                (BYTE)(std::min)(255, (int)(baseR + gl * (255 - baseR))),
                (BYTE)(std::min)(255, (int)(baseG + gl * (255 - baseG))),
                (BYTE)(std::min)(255, (int)(baseB + gl * (255 - baseB))));
            };

        Color c1 = makeLogoColor(88, 186, 255);
        Color c2 = makeLogoColor(72, 172, 255);
        Color c3 = makeLogoColor(100, 180, 255);
        Color c4 = makeLogoColor(82, 178, 255);

        FillRoundRectSolid(g, lx, ly, sq, sq, 3, c1);
        FillRoundRectSolid(g, lx + sq + gap, ly, sq, sq, 3, c2);
        FillRoundRectSolid(g, lx, ly + sq + gap, sq, sq, 3, c3);
        FillRoundRectSolid(g, lx + sq + gap, ly + sq + gap, sq, sq, 3, c4);

        // Specular
        FillRoundRectSolid(g, lx, ly, sq, sq / 3, 2, Color(30, 255, 255, 255));
        FillRoundRectSolid(g, lx + sq + gap, ly, sq, sq / 3, 2, Color(25, 255, 255, 255));
    }

    // === SEARCH ===
    {
        int bx = startX + (TASKBAR_ICON_SIZE + 8);
        int by = iconY;
        bool hov = (g_hoveredTaskbarIcon == -11);
        if (hov) {
            FillRoundRectGradient(g, bx - 1, by - 1, TASKBAR_ICON_SIZE + 2, TASKBAR_ICON_SIZE + 2, 11,
                Color(70, 48, 52, 62), Color(50, 36, 38, 46));
        }
        int cx = bx + TASKBAR_ICON_SIZE / 2 - 1;
        int cy = by + TASKBAR_ICON_SIZE / 2 - 1;
        Pen glassPen(Color(210, 195, 200, 218), 1.8f);
        glassPen.SetStartCap(LineCapRound);
        glassPen.SetEndCap(LineCapRound);
        g.DrawEllipse(&glassPen, cx - 6, cy - 6, 12, 12);
        g.DrawLine(&glassPen, cx + 4, cy + 4, cx + 8, cy + 8);
    }

    // === TASK VIEW ===
    {
        int bx = startX + 2 * (TASKBAR_ICON_SIZE + 8);
        int by = iconY;
        bool hov = (g_hoveredTaskbarIcon == -12);
        if (hov) {
            FillRoundRectGradient(g, bx - 1, by - 1, TASKBAR_ICON_SIZE + 2, TASKBAR_ICON_SIZE + 2, 11,
                Color(70, 48, 52, 62), Color(50, 36, 38, 46));
        }
        DrawRoundRect(g, bx + 17, by + 17, 13, 11, 2, Color(95, 180, 185, 200), 1.2f);
        DrawRoundRect(g, bx + 12, by + 13, 13, 11, 2, Color(195, 195, 200, 218), 1.3f);
    }

    // === APP ICONS ===
    int appsStartX = startX + 3 * (TASKBAR_ICON_SIZE + 8) + 14;
    for (int i = 0; i < iconCount; i++) {
        int ix = appsStartX + i * (TASKBAR_ICON_SIZE + 8);
        float ha = g_taskbarAnims[i].hover.current;
        bool isRunning = g_taskApps[i].running;
        bool isActive = g_taskApps[i].active;

        float bgIntensity = (std::max)(ha * 0.6f, isActive ? 0.5f : 0.0f);
        if (bgIntensity > 0.005f) {
            Color gc = g_taskApps[i].glowColor;
            for (int j = 0; j < 3; j++) {
                int pad = 4 + j * 3;
                BYTE ga = (BYTE)(bgIntensity * (7.0f - j * 2.2f));
                FillRoundRectSolid(g, ix - pad, iconY - pad,
                    TASKBAR_ICON_SIZE + pad * 2, TASKBAR_ICON_SIZE + pad * 2,
                    13 + j * 2, Color(ga, gc.GetR(), gc.GetG(), gc.GetB()));
            }
            FillRoundRectGradient(g, ix - 2, iconY - 2,
                TASKBAR_ICON_SIZE + 4, TASKBAR_ICON_SIZE + 4, 12,
                Color((BYTE)(bgIntensity * 130), 56, 60, 72),
                Color((BYTE)(bgIntensity * 90), 40, 42, 50));
            DrawRoundRect(g, ix - 2, iconY - 2,
                TASKBAR_ICON_SIZE + 4, TASKBAR_ICON_SIZE + 4, 12,
                Color((BYTE)(bgIntensity * 16), 255, 255, 255), 0.5f);
        }

        int icoInner = 32;
        int icoX = ix + (TASKBAR_ICON_SIZE - icoInner) / 2;
        int icoY = iconY + (TASKBAR_ICON_SIZE - icoInner) / 2;

        FillRoundRectSolid(g, icoX + 1, icoY + 2, icoInner, icoInner, 10,
            Color(35, 0, 0, 0));

        Color c1x = g_taskApps[i].accentColor;
        Color c2x(255,
            (BYTE)(std::max)(0, (int)c1x.GetR() - 30),
            (BYTE)(std::max)(0, (int)c1x.GetG() - 30),
            (BYTE)(std::max)(0, (int)c1x.GetB() - 30));
        FillRoundRectGradient(g, icoX, icoY, icoInner, icoInner, 10, c1x, c2x);

        // Specular
        FillRoundRectSolid(g, icoX + 3, icoY + 1, icoInner - 6, icoInner / 3, 5,
            Color(28, 255, 255, 255));

        DrawRoundRect(g, icoX, icoY, icoInner, icoInner, 10,
            Color(16, 255, 255, 255), 0.5f);

        Font icoFont(&ff, 10.5f, FontStyleBold, UnitPixel);
        SolidBrush icoTextBr(Color(255, 255, 255, 255));
        StringFormat icoCfmt;
        icoCfmt.SetAlignment(StringAlignmentCenter);
        icoCfmt.SetLineAlignment(StringAlignmentCenter);
        RectF icoRc((REAL)icoX, (REAL)icoY, (REAL)icoInner, (REAL)icoInner);
        g.DrawString(g_taskApps[i].iconLabel.c_str(), -1, &icoFont, icoRc, &icoCfmt, &icoTextBr);

        // Running indicator
        if (isRunning) {
            int dotY = iconY + TASKBAR_ICON_SIZE + 3;
            int dotCX = ix + TASKBAR_ICON_SIZE / 2;
            float targetW = isActive ? 22.0f : 6.0f;
            float dotH = 3.0f;

            float glowPulse = 0.6f + 0.4f * sinf(t * 2.5f + (float)i * 0.7f);
            for (int gl = 0; gl < 6; gl++) {
                float glW = targetW + gl * 6.0f;
                float glH = dotH + gl * 2.0f;
                float glA = glowPulse * (16.0f - gl * 2.5f);
                if (glA < 0) glA = 0;
                FillRoundRectF(g,
                    (float)dotCX - glW / 2.0f, (float)dotY - glH / 2.0f + 1.0f,
                    glW, glH, glH / 2.0f,
                    Color((BYTE)glA, 88, 186, 255));
            }
            FillRoundRectGradientF(g,
                (float)dotCX - targetW / 2.0f, (float)dotY - dotH / 2.0f,
                targetW, dotH, dotH / 2.0f,
                Color(255, 140, 218, 255), Color(255, 55, 165, 255), false);
        }

        SetRect(&g_taskApps[i].bounds, ix, iconY, ix + TASKBAR_ICON_SIZE, iconY + TASKBAR_ICON_SIZE);
    }

    // === SYSTEM TRAY ===
    int trayX = sw - 268;

    // Tray capsule
    FillRoundRectGradient(g, trayX - 8, barY + 8, 258, barH - 16, 14,
        Color(24, 70, 78, 95), Color(14, 32, 35, 44));
    DrawRoundRect(g, trayX - 8, barY + 8, 258, barH - 16, 14,
        Color(10, 255, 255, 255), 0.5f);

    // WiFi
    {
        int wbx = trayX + 24;
        int wby = barY + (barH - 26) / 2;
        bool wifiHov = (g_hoveredTaskbarIcon == -20);
        if (wifiHov || g_wifiPanelOpen) {
            FillRoundRectGradient(g, wbx - 6, wby - 5, 38, 36, 10,
                Color(45, 88, 186, 255), Color(25, 44, 120, 195));
        }
        int wcx = wbx + 13;
        int wcy = wby + 21;
        for (int arc = 0; arc < 3; arc++) {
            int r = 5 + arc * 6;
            float waveAlpha = 0.88f - arc * 0.15f;
            Pen wPen(Color((BYTE)(230 * waveAlpha), 185, 210, 255), 2.0f);
            wPen.SetStartCap(LineCapRound);
            wPen.SetEndCap(LineCapRound);
            g.DrawArc(&wPen, wcx - r, wcy - r, r * 2, r * 2, 225, 90);
        }
        SolidBrush dotBr(Color(248, 232, 240, 255));
        g.FillEllipse(&dotBr, wcx - 2, wcy - 2, 5, 5);
    }

    // Volume
    {
        int vx = trayX + 76;
        int vy = barY + (barH - 22) / 2;
        Pen vPen(Color(210, 200, 208, 228), 1.5f);
        vPen.SetStartCap(LineCapRound);
        vPen.SetEndCap(LineCapRound);
        Point speaker[6] = {
            Point(vx + 3, vy + 8), Point(vx + 7, vy + 8),
            Point(vx + 12, vy + 3), Point(vx + 12, vy + 19),
            Point(vx + 7, vy + 14), Point(vx + 3, vy + 14)
        };
        g.DrawPolygon(&vPen, speaker, 6);
        for (int w = 0; w < 2; w++) {
            int wr = 5 + w * 5;
            Pen wp(Color((BYTE)(210 * (0.8f - w * 0.25f)), 200, 208, 228), 1.3f);
            wp.SetStartCap(LineCapRound);
            wp.SetEndCap(LineCapRound);
            g.DrawArc(&wp, vx + 12, vy + 11 - wr, wr * 2, wr * 2, -55, 110);
        }
    }

    // Battery
    {
        int bx = trayX + 122;
        int by = barY + (barH - 16) / 2;
        DrawRoundRect(g, bx, by, 26, 16, 4, Color(200, 200, 208, 228), 1.2f);
        FillRoundRectSolid(g, bx + 26, by + 4, 3, 8, 1, Color(175, 200, 208, 228));
        int fillW = (int)(20 * 0.82f);
        FillRoundRectGradient(g, bx + 3, by + 3, fillW, 10, 2,
            Color(255, 110, 220, 100), Color(255, 72, 188, 65), false);
        FillRoundRectSolid(g, bx + 3, by + 3, fillW, 4, 1, Color(20, 255, 255, 255));
    }

    // Clock
    {
        time_t now = time(NULL);
        struct tm ti;
        localtime_s(&ti, &now);
        wchar_t timeStr[32], dateStr[32];
        wcsftime(timeStr, 32, L"%H:%M", &ti);
        wcsftime(dateStr, 32, L"%d.%m.%Y", &ti);

        int clockX = sw - 118;
        int clockRegionW = 100;

        Font clockFont(&ff, 12.5f, FontStyleBold, UnitPixel);
        Font dateFont(&ff, 10.5f, FontStyleRegular, UnitPixel);

        StringFormat rfmt;
        rfmt.SetAlignment(StringAlignmentFar);
        rfmt.SetLineAlignment(StringAlignmentCenter);

        SolidBrush clockBr(Color(255, 250, 252, 255));
        RectF clockRc((REAL)clockX, (REAL)barY, (REAL)clockRegionW, (REAL)(barH / 2 + 2));
        g.DrawString(timeStr, -1, &clockFont, clockRc, &rfmt, &clockBr);

        SolidBrush dateBr(Color(170, 190, 195, 215));
        RectF dateRc((REAL)clockX, (REAL)(barY + barH / 2 - 2), (REAL)clockRegionW, (REAL)(barH / 2));
        g.DrawString(dateStr, -1, &dateFont, dateRc, &rfmt, &dateBr);
    }

    // Notification dot
    if (!g_notifs.empty()) {
        int nix = sw - 18, niy = barY + barH / 2 - 4;
        float pulse = 0.5f + 0.5f * sinf(t * 3.5f);
        for (int i = 0; i < 4; i++) {
            int r = 4 + i * 2;
            BYTE ga = (BYTE)((0.3f + pulse * 0.15f) * (22.0f - i * 4.5f));
            SolidBrush nGlow(Color(ga, 88, 186, 255));
            g.FillEllipse(&nGlow, nix + 4 - r, niy + 4 - r, r * 2, r * 2);
        }
        SolidBrush notifDot(Color(255, 130, 210, 255));
        g.FillEllipse(&notifDot, nix, niy, 8, 8);
    }
}

// ============================================================================
//  START MENU (W12 style)
// ============================================================================
void DrawStartMenu(Graphics& g, int sw, int sh) {
    float a = g_startMenuAnim.current;
    if (a <= 0.01f) return;
    float ease = pOutCubic(Clamp01(a));

    SolidBrush overlay(Color((BYTE)(45 * a), 0, 0, 0));
    g.FillRectangle(&overlay, 0, 0, sw, sh);

    int menuW = START_MENU_W, menuH = START_MENU_H;
    int menuX = (sw - menuW) / 2;
    int targetY = sh - TASKBAR_HEIGHT - menuH - 16;
    int menuY = (int)(targetY + 24 * (1.0f - ease));

    DrawPremiumShadow(g, menuX, menuY, menuW, menuH, START_MENU_RADIUS, a);
    FillRoundRectGradient(g, menuX, menuY, menuW, menuH, START_MENU_RADIUS,
        Color((BYTE)(252 * a), 26, 26, 30),
        Color((BYTE)(252 * a), 22, 22, 26));
    DrawRoundRect(g, menuX, menuY, menuW, menuH, START_MENU_RADIUS,
        Color((BYTE)(35 * a), 255, 255, 255), 1.0f);

    // Accent top line
    Pen accentLine(Color((BYTE)(60 * a), 88, 186, 255), 1.5f);
    g.DrawLine(&accentLine, menuX + START_MENU_RADIUS, menuY,
        menuX + menuW - START_MENU_RADIUS, menuY);

    FontFamily ff(L"Segoe UI");
    StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat cfmt; cfmt.SetAlignment(StringAlignmentCenter); cfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat rfmt; rfmt.SetAlignment(StringAlignmentFar); rfmt.SetLineAlignment(StringAlignmentCenter);

    // Search bar — pill shape
    int searchX = menuX + 28, searchY = menuY + 26;
    int searchW = menuW - 56, searchH = 40;
    FillRoundRectSolid(g, searchX, searchY, searchW, searchH, searchH / 2,
        Color((BYTE)(255 * a), 42, 42, 48));
    DrawRoundRect(g, searchX, searchY, searchW, searchH, searchH / 2,
        Color((BYTE)(25 * a), 255, 255, 255), 0.8f);
    Pen searchAccent(Color((BYTE)(80 * a), 88, 186, 255), 1.5f);
    g.DrawLine(&searchAccent, searchX + searchH / 2, searchY + searchH - 1,
        searchX + searchW - searchH / 2, searchY + searchH - 1);

    Pen sPen(Color((BYTE)(165 * a), 175, 180, 190), 1.5f);
    sPen.SetStartCap(LineCapRound);
    sPen.SetEndCap(LineCapRound);
    g.DrawEllipse(&sPen, searchX + 16, searchY + 12, 13, 13);
    g.DrawLine(&sPen, searchX + 27, searchY + 25, searchX + 31, searchY + 29);

    Font searchFont(&ff, 13.0f, FontStyleRegular, UnitPixel);
    SolidBrush searchBr(Color((BYTE)(105 * a), 165, 168, 178));
    RectF searchRc((REAL)(searchX + 40), (REAL)searchY, (REAL)(searchW - 52), (REAL)searchH);
    g.DrawString(L"Search apps, settings, files...", -1, &searchFont, searchRc, &lfmt, &searchBr);

    // Pinned section
    Font sectionFont(&ff, 14.0f, FONT_SEMIBOLD, UnitPixel);
    SolidBrush sectionBr(Color((BYTE)(255 * a), 248, 248, 252));
    RectF pinnedTitleRc((REAL)(menuX + 34), (REAL)(menuY + 82), 100.0f, 22.0f);
    g.DrawString(L"Pinned", -1, &sectionFont, pinnedTitleRc, &lfmt, &sectionBr);

    Font smallFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    SolidBrush allBr(Color((BYTE)(165 * a), 175, 178, 188));
    RectF allRc((REAL)(menuX + menuW - 140), (REAL)(menuY + 82), 110.0f, 22.0f);
    g.DrawString(L"All apps  \x203A", -1, &smallFont, allRc, &rfmt, &allBr);

    int gridX = menuX + 32, gridY = menuY + 114;
    int cols = 6;
    int cellW = (menuW - 64) / cols, cellH = 76;

    for (int i = 0; i < (int)g_startApps.size() && i < 18; i++) {
        int row = i / cols, col = i % cols;
        int ix = gridX + col * cellW;
        int iy = gridY + row * cellH;

        float delay = (float)i * 0.018f;
        float itemA = Clamp01((a - delay) * 4.0f);
        float itemEase = pOutCubic(itemA);
        int itemOffY = (int)(8 * (1.0f - itemEase));
        iy += itemOffY;

        bool hov = (g_hoveredStartItem == i);
        if (hov) {
            FillRoundRectSolid(g, ix + 3, iy + 3, cellW - 6, cellH - 6, 10,
                Color((BYTE)(255 * itemA), 50, 50, 58));
        }

        int aicoSize = 38;
        int aicoX = ix + (cellW - aicoSize) / 2;
        int aicoY = iy + 6;

        FillRoundRectSolid(g, aicoX + 1, aicoY + 2, aicoSize, aicoSize, 10,
            Color((BYTE)(28 * itemA), 0, 0, 0));

        Color appC = g_startApps[i].color;
        FillRoundRectGradient(g, aicoX, aicoY, aicoSize, aicoSize, 10,
            Color((BYTE)(245 * itemA), appC.GetR(), appC.GetG(), appC.GetB()),
            Color((BYTE)(245 * itemA),
                (BYTE)(std::max)(0, (int)appC.GetR() - 22),
                (BYTE)(std::max)(0, (int)appC.GetG() - 22),
                (BYTE)(std::max)(0, (int)appC.GetB() - 22)));

        FillRoundRectSolid(g, aicoX + 3, aicoY + 1, aicoSize - 6, aicoSize / 3, 5,
            Color((BYTE)(22 * itemA), 255, 255, 255));

        Font aicoFont(&ff, 11.0f, FontStyleBold, UnitPixel);
        SolidBrush aicoTextBr(Color((BYTE)(255 * itemA), 255, 255, 255));
        RectF aicoRc((REAL)aicoX, (REAL)aicoY, (REAL)aicoSize, (REAL)aicoSize);
        g.DrawString(g_startApps[i].iconLabel.c_str(), -1, &aicoFont, aicoRc, &cfmt, &aicoTextBr);

        Font nameFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
        SolidBrush nameBr(Color((BYTE)(210 * itemA), 210, 212, 222));
        cfmt.SetTrimming(StringTrimmingEllipsisCharacter);
        RectF nameRc((REAL)ix, (REAL)(iy + 48), (REAL)cellW, 18.0f);
        g.DrawString(g_startApps[i].name.c_str(), -1, &nameFont, nameRc, &cfmt, &nameBr);
    }

    // Recommended section
    int sepY = menuY + 114 + 3 * cellH + 16;
    Pen sepLine(Color((BYTE)(18 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&sepLine, menuX + 32, sepY, menuX + menuW - 32, sepY);

    RectF recTitleRc((REAL)(menuX + 34), (REAL)(sepY + 16), 150.0f, 22.0f);
    g.DrawString(L"Recommended", -1, &sectionFont, recTitleRc, &lfmt, &sectionBr);

    const wchar_t* recNames[] = { L"Recently opened file.txt", L"Project_v2.docx", L"Screenshot_2026.png" };
    const wchar_t* recTimes[] = { L"Just now", L"Yesterday", L"2 days ago" };
    for (int i = 0; i < 3; i++) {
        int ry = sepY + 48 + i * 44;
        int rx = menuX + 34;

        FillRoundRectSolid(g, rx, ry + 3, 32, 32, 8, Color((BYTE)(180 * a), 48, 48, 55));
        Font fIcon(&ff, 10.0f, FontStyleRegular, UnitPixel);
        SolidBrush fIcoBr(Color((BYTE)(185 * a), 168, 172, 192));
        RectF fIcoRc((REAL)rx, (REAL)(ry + 3), 32.0f, 32.0f);
        g.DrawString(L"\xD83D\xDCC4", -1, &fIcon, fIcoRc, &cfmt, &fIcoBr);

        Font recFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
        SolidBrush recBr(Color((BYTE)(212 * a), 212, 215, 225));
        RectF recNameRc((REAL)(rx + 40), (REAL)(ry + 2), 220.0f, 18.0f);
        g.DrawString(recNames[i], -1, &recFont, recNameRc, &lfmt, &recBr);
        Font timeFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
        SolidBrush timeBr(Color((BYTE)(95 * a), 138, 140, 150));
        RectF recTimeRc((REAL)(rx + 40), (REAL)(ry + 22), 220.0f, 16.0f);
        g.DrawString(recTimes[i], -1, &timeFont, recTimeRc, &lfmt, &timeBr);
    }

    // Bottom bar
    int bottomY = menuY + menuH - 56;
    Pen bottomSep(Color((BYTE)(16 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&bottomSep, menuX + 28, bottomY, menuX + menuW - 28, bottomY);

    FillRoundRectGradient(g, menuX + 34, bottomY + 14, 32, 32, 16,
        Color((BYTE)(200 * a), 85, 88, 100),
        Color((BYTE)(200 * a), 60, 62, 74));
    Font userIcon(&ff, 14.0f, FontStyleBold, UnitPixel);
    SolidBrush userBr(Color((BYTE)(238 * a), 198, 202, 218));
    RectF userRc((REAL)(menuX + 34), (REAL)(bottomY + 14), 32.0f, 32.0f);
    g.DrawString(L"U", -1, &userIcon, userRc, &cfmt, &userBr);

    Font usernameFont(&ff, 13.0f, FontStyleRegular, UnitPixel);
    SolidBrush usernameBr(Color((BYTE)(218 * a), 222, 225, 238));
    lfmt.SetLineAlignment(StringAlignmentCenter);
    RectF usernameRc((REAL)(menuX + 74), (REAL)(bottomY + 10), 200.0f, 38.0f);
    g.DrawString(L"User", -1, &usernameFont, usernameRc, &lfmt, &usernameBr);

    // Power button
    int pwrX = menuX + menuW - 56, pwrY = bottomY + 15;
    FillRoundRectSolid(g, pwrX, pwrY, 30, 30, 8, Color((BYTE)(40 * a), 255, 255, 255));
    Pen pwrPen(Color((BYTE)(195 * a), 195, 200, 210), 1.8f);
    pwrPen.SetStartCap(LineCapRound);
    pwrPen.SetEndCap(LineCapRound);
    int pcx = pwrX + 15, pcy = pwrY + 15;
    g.DrawArc(&pwrPen, pcx - 6, pcy - 6, 12, 12, -60, 300);
    g.DrawLine(&pwrPen, pcx, pcy - 8, pcx, pcy - 3);
}

// ============================================================================
//  CONTEXT MENU
// ============================================================================
void DrawContextMenu(Graphics& g) {
    float a = g_contextMenuAnim.current;
    if (!g_contextMenuOpen || a <= 0.01f) return;
    float ease = pOutCubic(Clamp01(a));

    int menuW = CTX_MENU_W;
    int menuH = 8;
    for (const auto& item : g_contextItems)
        menuH += (item.id == 0) ? CTX_SEP_H : CTX_ITEM_H;
    menuH += 8;

    int mx = g_contextMenuX, my = g_contextMenuY;
    if (mx + menuW > ScreenW()) mx = ScreenW() - menuW - 8;
    if (my + menuH > ScreenH()) my = ScreenH() - menuH - 8;
    int animMenuH = (int)(menuH * ease);

    DrawShadow(g, mx, my, menuW, animMenuH, CTX_RADIUS, 10, a);
    FillRoundRectGradient(g, mx, my, menuW, animMenuH, CTX_RADIUS,
        Color((BYTE)(252 * a), 32, 32, 36),
        Color((BYTE)(252 * a), 28, 28, 32));
    DrawRoundRect(g, mx, my, menuW, animMenuH, CTX_RADIUS,
        Color((BYTE)(30 * a), 255, 255, 255), 0.8f);

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
            Pen sp(Color((BYTE)(18 * a), 255, 255, 255), 1.0f);
            g.DrawLine(&sp, mx + 16, cy + CTX_SEP_H / 2, mx + menuW - 16, cy + CTX_SEP_H / 2);
            cy += CTX_SEP_H;
            continue;
        }
        bool hov = (g_hoveredContextItem == i);
        if (hov) {
            FillRoundRectSolid(g, mx + 4, cy + 1, menuW - 8, CTX_ITEM_H - 2, 7,
                Color((BYTE)(255 * a), 48, 48, 55));
        }
        SolidBrush textBr(Color((BYTE)(228 * a), 228, 230, 238));
        RectF textRc((REAL)(mx + 20), (REAL)cy, (REAL)(menuW - 40), (REAL)CTX_ITEM_H);
        g.DrawString(item.label.c_str(), -1, &font, textRc, &lfmt2, &textBr);
        if (!item.shortcut.empty()) {
            SolidBrush scBr(Color((BYTE)(80 * a), 130, 132, 142));
            g.DrawString(item.shortcut.c_str(), -1, &shortcutFont, textRc, &rfmt2, &scBr);
        }
        cy += CTX_ITEM_H;
    }
    g.ResetClip();
}

// ============================================================================
//  WIFI PANEL (W12)
// ============================================================================
void DrawWifiPanel(Graphics& g, int sw, int sh) {
    float a = g_wifiPanelAnim.current;
    if (a <= 0.01f) return;
    float ease = pOutCubic(Clamp01(a));

    int panelW = WIFI_PANEL_W, panelH = WIFI_PANEL_H;
    int panelX = sw - panelW - 16;
    int panelY = (int)(sh - TASKBAR_HEIGHT - panelH - 16 + 16 * (1.0f - ease));

    DrawPremiumShadow(g, panelX, panelY, panelW, panelH, 12, a);
    FillRoundRectGradient(g, panelX, panelY, panelW, panelH, 12,
        Color((BYTE)(252 * a), 26, 26, 30),
        Color((BYTE)(252 * a), 22, 22, 26));
    DrawRoundRect(g, panelX, panelY, panelW, panelH, 12,
        Color((BYTE)(30 * a), 255, 255, 255), 0.8f);

    Pen accentTop(Color((BYTE)(50 * a), 88, 186, 255), 1.2f);
    g.DrawLine(&accentTop, panelX + 12, panelY, panelX + panelW - 12, panelY);

    FontFamily ff(L"Segoe UI");
    StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentCenter);
    StringFormat cfmt; cfmt.SetAlignment(StringAlignmentCenter); cfmt.SetLineAlignment(StringAlignmentCenter);

    // Quick toggles
    int toggleY = panelY + 20;
    int toggleH = 74;
    int toggleW = (panelW - 56) / 3;

    struct Toggle { std::wstring label; std::wstring icon; bool active; Color color; };
    Toggle toggles[] = {
        { L"WiFi", L"\xD83D\xDCF6", true, Color(255, 88, 186, 255) },
        { L"Bluetooth", L"\xD83D\xDD37", true, Color(255, 0, 120, 212) },
        { L"Airplane", L"\x2708", false, Color(255, 70, 70, 78) }
    };

    for (int i = 0; i < 3; i++) {
        int tx = panelX + 20 + i * (toggleW + 8);
        if (toggles[i].active) {
            FillRoundRectGradient(g, tx, toggleY, toggleW, toggleH, 12,
                Color((BYTE)(255 * a), toggles[i].color.GetR(), toggles[i].color.GetG(), toggles[i].color.GetB()),
                Color((BYTE)(255 * a),
                    (BYTE)(std::max)(0, (int)toggles[i].color.GetR() - 30),
                    (BYTE)(std::max)(0, (int)toggles[i].color.GetG() - 30),
                    (BYTE)(std::max)(0, (int)toggles[i].color.GetB() - 30)));
        }
        else {
            FillRoundRectSolid(g, tx, toggleY, toggleW, toggleH, 12,
                Color((BYTE)(200 * a), 44, 44, 50));
        }

        Font iconFont(&ff, 16.0f, FontStyleRegular, UnitPixel);
        Color iconC = toggles[i].active ? Color((BYTE)(255 * a), 255, 255, 255) :
            Color((BYTE)(175 * a), 168, 170, 178);
        SolidBrush iconBr(iconC);
        RectF iconRc((REAL)tx, (REAL)(toggleY + 10), (REAL)toggleW, 28.0f);
        g.DrawString(toggles[i].icon.c_str(), -1, &iconFont, iconRc, &cfmt, &iconBr);

        Font tFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
        Color tColor = toggles[i].active ? Color((BYTE)(238 * a), 255, 255, 255) :
            Color((BYTE)(165 * a), 168, 170, 178);
        SolidBrush tBr(tColor);
        cfmt.SetLineAlignment(StringAlignmentFar);
        RectF tRc((REAL)tx, (REAL)toggleY, (REAL)toggleW, (REAL)(toggleH - 10));
        g.DrawString(toggles[i].label.c_str(), -1, &tFont, tRc, &cfmt, &tBr);
        cfmt.SetLineAlignment(StringAlignmentCenter);
    }

    // Sliders
    int sliderY = toggleY + toggleH + 20;
    auto drawSlider = [&](int sy, const std::wstring& label, const std::wstring& icon, float value, Color accentC) {
        Font lbl(&ff, 11.0f, FontStyleRegular, UnitPixel);
        SolidBrush lblBr(Color((BYTE)(150 * a), 170, 175, 185));
        lfmt.SetLineAlignment(StringAlignmentNear);

        Font icoFont(&ff, 13.0f, FontStyleRegular, UnitPixel);
        SolidBrush icoBr(Color((BYTE)(175 * a), 195, 200, 215));
        RectF icoRc((REAL)(panelX + 20), (REAL)sy, 20.0f, 18.0f);
        g.DrawString(icon.c_str(), -1, &icoFont, icoRc, &lfmt, &icoBr);

        RectF lblRc((REAL)(panelX + 42), (REAL)sy, 100.0f, 18.0f);
        g.DrawString(label.c_str(), -1, &lbl, lblRc, &lfmt, &lblBr);

        int barX2 = panelX + 22, barY2 = sy + 26, barW2 = panelW - 44, barH2 = 6;
        FillRoundRectSolid(g, barX2, barY2, barW2, barH2, 3, Color((BYTE)(135 * a), 42, 42, 48));
        int fillW = (int)(barW2 * value);
        if (fillW > 0)
            FillRoundRectGradient(g, barX2, barY2, fillW, barH2, 3,
                Color((BYTE)(232 * a), accentC.GetR(), accentC.GetG(), accentC.GetB()),
                Color((BYTE)(232 * a),
                    (BYTE)(std::max)(0, (int)accentC.GetR() - 35),
                    (BYTE)(std::max)(0, (int)accentC.GetG() - 35),
                    (BYTE)(std::max)(0, (int)accentC.GetB() - 35)), false);

        int thumbX = barX2 + fillW - 7;
        if (thumbX < barX2) thumbX = barX2;
        FillRoundRectSolid(g, thumbX, barY2 - 3, 14, 12, 6,
            Color((BYTE)(238 * a), 255, 255, 255));
        FillRoundRectSolid(g, thumbX + 2, barY2 - 1, 10, 8, 4,
            Color((BYTE)(238 * a), accentC.GetR(), accentC.GetG(), accentC.GetB()));
        };

    drawSlider(sliderY, L"Brightness", L"\x2600", g_brightnessLevel, Color(255, 255, 208, 65));
    sliderY += 46;
    drawSlider(sliderY, L"Volume", L"\xD83D\xDD0A", g_volumeLevel, Color(255, 88, 186, 255));

    // Networks list
    sliderY += 50;
    Pen sep(Color((BYTE)(16 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&sep, panelX + 20, sliderY, panelX + panelW - 20, sliderY);

    Font titleFont(&ff, 13.0f, FONT_SEMIBOLD, UnitPixel);
    SolidBrush titleBr(Color((BYTE)(248 * a), 238, 240, 245));
    RectF titleRc((REAL)(panelX + 22), (REAL)(sliderY + 12), 200.0f, 24.0f);
    lfmt.SetLineAlignment(StringAlignmentCenter);
    g.DrawString(L"Available networks", -1, &titleFont, titleRc, &lfmt, &titleBr);

    Font itemFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    Font small2Font(&ff, 10.0f, FontStyleRegular, UnitPixel);
    int listY = sliderY + 44;
    int itemHH = 54;

    for (int i = 0; i < (int)g_wifiNetworks.size() && i < 6; i++) {
        const WifiNetwork& net = g_wifiNetworks[i];
        int iy = listY + i * itemHH;
        bool hov = (g_hoveredWifiItem == i);

        if (hov) {
            FillRoundRectSolid(g, panelX + 10, iy, panelW - 20, itemHH - 4, 10,
                Color((BYTE)(200 * a), 42, 42, 48));
        }

        if (net.connected) {
            DrawRoundRect(g, panelX + 10, iy, panelW - 20, itemHH - 4, 10,
                Color((BYTE)(45 * a), 88, 186, 255), 1.0f);
            FillRoundRectSolid(g, panelX + 10, iy, panelW - 20, itemHH - 4, 10,
                Color((BYTE)(7 * a), 88, 186, 255));
        }

        int sigX = panelX + 26;
        int sigBaseY = iy + itemHH / 2 + 9;
        for (int bar = 0; bar < 4; bar++) {
            int bH = 4 + bar * 4;
            bool filled = net.signal >= (bar + 1) * 25;
            Color barCol = filled ?
                Color((BYTE)(222 * a), 195, 215, 255) :
                Color((BYTE)(32 * a), 195, 200, 210);
            FillRoundRectSolid(g, sigX + bar * 7, sigBaseY - bH, 5, bH, 1, barCol);
        }

        if (net.secured) {
            Pen lockPen(Color((BYTE)(85 * a), 185, 190, 208), 1.2f);
            int lx2 = sigX + 34, ly2 = sigBaseY - 10;
            g.DrawArc(&lockPen, lx2, ly2 - 4, 8, 8, 180, 180);
            FillRoundRectSolid(g, lx2 - 1, ly2, 10, 7, 2, Color((BYTE)(85 * a), 185, 190, 208));
        }

        SolidBrush nameBr(Color((BYTE)(238 * a), 238, 240, 248));
        RectF nameRc((REAL)(panelX + 70), (REAL)(iy + 10), (REAL)(panelW - 145), 22.0f);
        g.DrawString(net.ssid.c_str(), -1, &itemFont, nameRc, &lfmt, &nameBr);

        std::wstring status = net.connected ? L"Connected, secured" : (net.secured ? L"Secured" : L"Open");
        Color statusColor = net.connected ? Color((BYTE)(150 * a), 100, 208, 95) :
            Color((BYTE)(85 * a), 150, 152, 162);
        SolidBrush statusBr(statusColor);
        RectF statusRc((REAL)(panelX + 70), (REAL)(iy + 32), (REAL)(panelW - 145), 16.0f);
        g.DrawString(status.c_str(), -1, &small2Font, statusRc, &lfmt, &statusBr);

        if (net.connected) {
            SolidBrush checkBr(Color((BYTE)(195 * a), 100, 208, 95));
            Font checkFont(&ff, 14.0f, FontStyleBold, UnitPixel);
            RectF checkRc((REAL)(panelX + panelW - 46), (REAL)iy, 28.0f, (REAL)itemHH);
            g.DrawString(L"\x2713", -1, &checkFont, checkRc, &cfmt, &checkBr);
        }
    }
}

// ============================================================================
//  NOTIFICATIONS (W12)
// ============================================================================
void DrawNotifications(Graphics& g, int sw, int sh) {
    int baseY = sh - TASKBAR_HEIGHT - NOTIF_H - 20;
    for (int i = (int)g_notifs.size() - 1; i >= 0 && i >= (int)g_notifs.size() - 3; i--) {
        Notification& n = g_notifs[i];
        if (!n.alive) continue;
        float a = n.alpha.current;
        if (a <= 0.01f) continue;

        int idx = (int)g_notifs.size() - 1 - i;
        int nx = sw - NOTIF_W - 16;
        int ny = (int)(baseY - idx * (NOTIF_H + 12) + n.offsetY.current);

        DrawShadow(g, nx, ny, NOTIF_W, NOTIF_H, 12, 8, a);
        FillRoundRectGradient(g, nx, ny, NOTIF_W, NOTIF_H, 14,
            Color((BYTE)(250 * a), 32, 32, 36),
            Color((BYTE)(250 * a), 28, 28, 32));
        DrawRoundRect(g, nx, ny, NOTIF_W, NOTIF_H, 14,
            Color((BYTE)(30 * a), 255, 255, 255), 0.8f);

        // Accent line left
        FillRoundRectSolid(g, nx + 1, ny + 16, 3, NOTIF_H - 32, 2,
            Color((BYTE)(195 * a), 88, 186, 255));

        // Icon
        FillRoundRectGradient(g, nx + 18, ny + 20, 42, 42, 12,
            Color((BYTE)(200 * a), 88, 186, 255),
            Color((BYTE)(200 * a), 45, 155, 230));

        FontFamily ff(L"Segoe UI");
        Font icoFont(&ff, 16.0f, FontStyleBold, UnitPixel);
        SolidBrush icoBr(Color((BYTE)(255 * a), 255, 255, 255));
        StringFormat cfmt; cfmt.SetAlignment(StringAlignmentCenter); cfmt.SetLineAlignment(StringAlignmentCenter);
        RectF icoRc((REAL)(nx + 18), (REAL)(ny + 20), 42.0f, 42.0f);
        g.DrawString(L"V", -1, &icoFont, icoRc, &cfmt, &icoBr);

        Font titleFont(&ff, 12.0f, FONT_SEMIBOLD, UnitPixel);
        Font msgFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
        StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentNear);

        SolidBrush titleBr(Color((BYTE)(248 * a), 242, 245, 250));
        RectF titleRc2((REAL)(nx + 70), (REAL)(ny + 20), (REAL)(NOTIF_W - 95), 20.0f);
        g.DrawString(n.title.c_str(), -1, &titleFont, titleRc2, &lfmt, &titleBr);

        SolidBrush msgBr(Color((BYTE)(165 * a), 182, 185, 195));
        RectF msgRc((REAL)(nx + 70), (REAL)(ny + 44), (REAL)(NOTIF_W - 95), 28.0f);
        g.DrawString(n.message.c_str(), -1, &msgFont, msgRc, &lfmt, &msgBr);
    }
}

// ============================================================================
//  WIDGETS (W12)
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
    w.title = L"\x2600  Weather"; w.height = 102; w.accent = Color(255, 255, 208, 55);
    w.content = L"22\xB0  Partly Cloudy\nMoscow, Russia\nFeels like 19\xB0  |  Wind 3 m/s";
    g_widgets.push_back(w);

    MEMORYSTATUSEX mem; mem.dwLength = sizeof(mem); GlobalMemoryStatusEx(&mem);
    int cpuSim = 15 + (g_tick / 1000) % 25;
    swprintf_s(buf, L"CPU: %d%%  |  RAM: %d%%\nDisk: 62%%  |  GPU: 45%%", cpuSim, (int)mem.dwMemoryLoad);
    w.title = L"\xD83D\xDCCA  System"; w.content = buf; w.height = 84; w.accent = Color(255, 88, 186, 255);
    g_widgets.push_back(w);

    wcsftime(buf, 128, L"%A, %d %B %Y\n%H:%M:%S", &ti);
    w.title = L"\xD83D\xDD50  Clock"; w.content = buf; w.height = 84; w.accent = Color(255, 195, 200, 215);
    g_widgets.push_back(w);

    w.title = L"\xD83C\xDFB5  Now Playing"; w.height = 94; w.accent = Color(255, 255, 105, 135);
    w.content = g_musicPlaying ? L"Synthwave Mix\nArtist - Track\n\x25B6 2:15 / 4:30" : L"No music playing\nPress M to start";
    g_widgets.push_back(w);
}

void DrawWidgets(Graphics& g, int sw, int sh) {
    float a = g_widgetsAnim.current;
    if (a <= 0.01f) return;
    float ease = pOutCubic(Clamp01(a));

    int panelW = WIDGET_W;
    int offsetX = (int)(-panelW * (1.0f - ease));
    int baseX = 16 + offsetX, baseY = 16;

    FontFamily ff(L"Segoe UI");
    StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentNear);

    for (size_t i = 0; i < g_widgets.size(); i++) {
        const Widget& w = g_widgets[i];
        int x = baseX, y = baseY;

        DrawShadow(g, x, y, panelW, w.height, 10, 5, a);
        FillRoundRectGradient(g, x, y, panelW, w.height, 12,
            Color((BYTE)(240 * a), 30, 30, 34),
            Color((BYTE)(240 * a), 26, 26, 30));
        DrawRoundRect(g, x, y, panelW, w.height, 12,
            Color((BYTE)(18 * a), 255, 255, 255), 0.8f);

        FillRoundRectGradient(g, x + 1, y + 16, 3, w.height - 32, 1,
            Color((BYTE)(205 * a), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()),
            Color((BYTE)(95 * a), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));

        Font titleFont(&ff, 11.0f, FONT_SEMIBOLD, UnitPixel);
        SolidBrush titleBr(Color((BYTE)(165 * a), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));
        RectF titleRc((REAL)(x + 20), (REAL)(y + 12), (REAL)(panelW - 40), 18.0f);
        g.DrawString(w.title.c_str(), -1, &titleFont, titleRc, &lfmt, &titleBr);

        Font contentFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
        SolidBrush contentBr(Color((BYTE)(212 * a), 222, 225, 235));
        std::wistringstream ss(w.content);
        std::wstring line;
        REAL ly = (REAL)(y + 34);
        while (std::getline(ss, line)) {
            RectF lineRc((REAL)(x + 20), ly, (REAL)(panelW - 40), 18.0f);
            g.DrawString(line.c_str(), -1, &contentFont, lineRc, &lfmt, &contentBr);
            ly += 18.0f;
        }
        baseY += w.height + 12;
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
        LinearGradientBrush bg(Point(0, 0), Point(sw, sh),
            Color(255, 10, 28, 65), Color(255, 42, 15, 52));
        g.FillRectangle(&bg, 0, 0, sw, sh);
        for (int i = 0; i < 5; i++) {
            float t2 = g_time * 0.08f + i * 1.5f;
            int cx = sw / 5 + (int)(sinf(t2) * sw / 6) + i * sw / 5;
            int cy = sh / 4 + (int)(cosf(t2 * 0.7f) * sh / 5) + (i % 2) * sh / 3;
            int cr = 200 + i * 45;
            SolidBrush blob(Color(5, 70 + i * 18, 75, 175));
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
    g.SetSmoothingMode(SmoothingModeHighQuality);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    g.SetCompositingQuality(CompositingQualityHighQuality);
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);

    if (!g_boot.complete) {
        // During boot, draw desktop behind for the fade transition
        if (g_boot.phase >= BOOT_FADE_TO_DESKTOP) {
            float desktopAlpha = g_desktopFadeIn.current;
            DrawBackground(g, sw, sh);
            DrawDesktopIcons(g);
            DrawTaskbar(g, sw, sh);
        }
        DrawBootScreen(g, sw, sh);
    }
    else {
        DrawBackground(g, sw, sh);
        DrawDesktopIcons(g);
        DrawSelectionRect(g);
        DrawWidgets(g, sw, sh);

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

        if (!g_startMenuOpen && !g_contextMenuOpen && g_explorers.empty()) {
            FontFamily ff(L"Segoe UI");
            Font hintFont(&ff, 9.0f, FontStyleRegular, UnitPixel);
            SolidBrush hintBr(Color(28, 195, 200, 210));
            StringFormat lfmt; lfmt.SetAlignment(StringAlignmentNear); lfmt.SetLineAlignment(StringAlignmentFar);
            RectF hintRc(16.0f, (REAL)(sh - TASKBAR_HEIGHT - 20), 600.0f, 14.0f);
            g.DrawString(L"ESC close | SPACE menu | M music | W widgets | N notify | F wifi",
                -1, &hintFont, hintRc, &lfmt, &hintBr);
        }
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
        QueryPerformanceFrequency(&g_perfFreq);
        QueryPerformanceCounter(&g_lastFrame);
        InitTaskbarApps();
        InitStartApps();
        InitDesktopIcons();
        InitContextMenu();
        UpdateWidgets();
        CreateThread(NULL, 0, WallpaperThread, NULL, 0, NULL);
        CreateThread(NULL, 0, WifiScanThread, NULL, 0, NULL);
        SetTimer(hWnd, 1, 1000, NULL);
        SetTimer(hWnd, 2, 7, NULL);
        // Boot starts automatically
        g_boot = BootState();
        break;

    case WM_TIMER:
        g_tick = GetTickCount();
        g_time = (float)g_tick / 1000.0f;

        if (wParam == 1) {
            UpdateWidgets();
        }
        else if (wParam == 2) {
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            float dt = (float)(now.QuadPart - g_lastFrame.QuadPart) / (float)g_perfFreq.QuadPart;
            g_lastFrame = now;
            if (dt > 0.05f) dt = 0.016f;
            if (dt < 0.001f) dt = 0.007f;

            // Update boot
            UpdateBoot(dt);
            g_desktopFadeIn.Update(dt);

            // Only update UI animations after boot
            if (g_boot.complete) {
                g_startMenuAnim.SetTarget(g_startMenuOpen ? 1.0f : 0.0f);
                g_startMenuAnim.Update(dt);

                g_widgetsAnim.SetTarget(g_widgetsVisible ? 1.0f : 0.0f);
                g_widgetsAnim.Update(dt);

                g_wifiPanelAnim.SetTarget(g_wifiPanelOpen ? 1.0f : 0.0f);
                g_wifiPanelAnim.Update(dt);

                g_contextMenuAnim.SetTarget(g_contextMenuOpen ? 1.0f : 0.0f);
                g_contextMenuAnim.Update(dt);

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
                    g_taskbarAnims[i].labelAlpha.SetTarget(hov ? 1.0f : 0.0f);
                    g_taskbarAnims[i].labelAlpha.Update(dt);
                }

                for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
                    bool hov = (g_hoveredDesktopIcon == i);
                    g_desktopIcons[i].hoverAnim.SetTarget(hov ? 1.0f : 0.0f);
                    g_desktopIcons[i].hoverAnim.Update(dt);
                    g_desktopIcons[i].selectAnim.SetTarget(g_desktopIcons[i].selected ? 1.0f : 0.0f);
                    g_desktopIcons[i].selectAnim.Update(dt);
                }

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
            }

            // Send welcome notification when boot completes
            static bool bootNotifSent = false;
            if (g_boot.complete && !bootNotifSent) {
                PushNotification(L"VORTEX Desktop", L"Welcome! Your desktop is ready.");
                bootNotifSent = true;
            }

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
        if (!g_boot.complete) break;
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (g_activeExplorerId >= 0) {
            ExplorerWindow* win = FindExplorer(g_activeExplorerId);
            if (win && win->visible) {
                win->targetScroll -= delta / 2;
                int maxScroll = (int)win->items.size() * 34 - (win->h - 78);
                if (win->currentPath.empty()) maxScroll = (int)win->items.size() * 88 - (win->h - 78);
                if (maxScroll < 0) maxScroll = 0;
                if (win->targetScroll < 0) win->targetScroll = 0;
                if (win->targetScroll > maxScroll) win->targetScroll = (float)maxScroll;
            }
        }
        break;
    }

    case WM_MOUSEMOVE: {
        if (!g_boot.complete) break;
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

        // Explorer hover
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
                    RECT br; SetRect(&br, bx, wy2, bx + btnW2, wy2 + 36);
                    if (PtInRect(&br, pt)) { win.hoveredTitleBtn = b; break; }
                }

                win.hoveredSidebarItem = -1;
                int sidebarW2 = 205;
                int contentY2 = wy2 + 78;
                int sideY2 = contentY2 + 40;
                for (int si = 0; si < 7; si++) {
                    RECT sr; SetRect(&sr, wx2 + 6, sideY2, wx2 + sidebarW2 - 6, sideY2 + 32);
                    if (PtInRect(&sr, pt)) { win.hoveredSidebarItem = si; break; }
                    sideY2 += 32;
                    if (si == 5) sideY2 += 42;
                }

                win.hoveredItem = -1;
                int contentX2 = wx2 + sidebarW2;
                int contentW2 = ww2 - sidebarW2;

                if (mx > contentX2 && mx < wx2 + ww2 && my > contentY2 && my < wy2 + wh2) {
                    if (win.currentPath.empty()) {
                        int tileW = 248, tileH = 76;
                        int cols = (std::max)(1, (contentW2 - 48) / (tileW + 12));
                        int startY2 = contentY2 + 60 - (int)win.scrollAnim.current;
                        for (int i = 0; i < (int)win.items.size(); i++) {
                            int col = i % cols, row = i / cols;
                            int tx = contentX2 + 24 + col * (tileW + 12);
                            int ty = startY2 + row * (tileH + 12);
                            RECT tr; SetRect(&tr, tx, ty, tx + tileW, ty + tileH);
                            if (PtInRect(&tr, pt)) { win.hoveredItem = i; break; }
                        }
                    }
                    else {
                        int headerH = 36;
                        int itemH2 = 34;
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
        int sw2 = ScreenW(), sh2 = ScreenH();
        int barY = sh2 - TASKBAR_HEIGHT;
        if (my >= barY) {
            int totalW2 = (TASKBAR_ICON_SIZE + 8);
            int startX2 = (sw2 - totalW2) / 2;
            int iconY2 = barY + (TASKBAR_HEIGHT - TASKBAR_ICON_SIZE) / 2;

            RECT sr; SetRect(&sr, startX2, iconY2, startX2 + TASKBAR_ICON_SIZE, iconY2 + TASKBAR_ICON_SIZE);
            if (PtInRect(&sr, pt)) g_hoveredTaskbarIcon = -10;

            int searchBtnX = startX2 + (TASKBAR_ICON_SIZE + 8);
            RECT sr2; SetRect(&sr2, searchBtnX, iconY2, searchBtnX + TASKBAR_ICON_SIZE, iconY2 + TASKBAR_ICON_SIZE);
            if (PtInRect(&sr2, pt)) g_hoveredTaskbarIcon = -11;

            int tvBtnX = startX2 + 2 * (TASKBAR_ICON_SIZE + 8);
            RECT sr3; SetRect(&sr3, tvBtnX, iconY2, tvBtnX + TASKBAR_ICON_SIZE, iconY2 + TASKBAR_ICON_SIZE);
            if (PtInRect(&sr3, pt)) g_hoveredTaskbarIcon = -12;

            for (int i = 0; i < (int)g_taskApps.size(); i++) {
                if (PtInRect(&g_taskApps[i].bounds, pt)) { g_hoveredTaskbarIcon = i; break; }
            }

            int trayX = sw2 - 268;
            RECT wifiRect; SetRect(&wifiRect, trayX + 18, barY, trayX + 62, barY + TASKBAR_HEIGHT);
            if (PtInRect(&wifiRect, pt)) g_hoveredTaskbarIcon = -20;
        }

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

        g_hoveredWifiItem = -1;
        if (g_wifiPanelOpen && g_wifiPanelAnim.current > 0.5f) {
            int panelX = sw2 - WIFI_PANEL_W - 16;
            int panelY = sh2 - TASKBAR_HEIGHT - WIFI_PANEL_H - 16;
            int toggleH = 74;
            int listY2 = panelY + 20 + toggleH + 20 + 46 + 50 + 20 + 44;
            int itemHH = 54;
            for (int i = 0; i < (int)g_wifiNetworks.size() && i < 6; i++) {
                int iy = listY2 + i * itemHH;
                RECT ir; SetRect(&ir, panelX + 10, iy, panelX + WIFI_PANEL_W - 10, iy + itemHH);
                if (PtInRect(&ir, pt)) { g_hoveredWifiItem = i; break; }
            }
        }

        g_hoveredStartItem = -1;
        if (g_startMenuOpen && g_startMenuAnim.current > 0.5f) {
            int menuX = (sw2 - START_MENU_W) / 2;
            int menuY2 = sh2 - TASKBAR_HEIGHT - START_MENU_H - 16;
            int gridX = menuX + 32, gridY = menuY2 + 114;
            int cols = 6;
            int cellW = (START_MENU_W - 64) / cols, cellH = 76;
            for (int i = 0; i < (int)g_startApps.size() && i < 18; i++) {
                int row = i / cols, col = i % cols;
                int ix = gridX + col * cellW;
                int iy = gridY + row * cellH;
                RECT cr; SetRect(&cr, ix, iy, ix + cellW, iy + cellH);
                if (PtInRect(&cr, pt)) { g_hoveredStartItem = i; break; }
            }
        }

        // Resize cursors
        bool onResizeEdge = false;
        for (auto& win : g_explorers) {
            if (!win.visible || win.animAlpha.current < 0.5f) continue;
            int wx2 = win.x, wy2 = win.y, ww2 = win.w, wh2 = win.h;
            if (mx > wx2 + ww2 - 8 && mx < wx2 + ww2 + 4 && my > wy2 + wh2 - 8 && my < wy2 + wh2 + 4) {
                SetCursor(LoadCursor(NULL, IDC_SIZENWSE)); onResizeEdge = true; break;
            }
            if (mx > wx2 + ww2 - 4 && mx < wx2 + ww2 + 4 && my > wy2 + 36 && my < wy2 + wh2 - 8) {
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
        if (!g_boot.complete) break;
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
                int sw3 = ScreenW(), sh3 = ScreenH();
                int menuX = (sw3 - START_MENU_W) / 2;
                int menuY2 = sh3 - TASKBAR_HEIGHT - START_MENU_H - 16;
                RECT mr; SetRect(&mr, menuX, menuY2, menuX + START_MENU_W, menuY2 + START_MENU_H);
                if (!PtInRect(&mr, pt)) g_startMenuOpen = false;
            }
            break;
        }

        if (g_wifiPanelOpen && g_wifiPanelAnim.current > 0.5f) {
            int sw3 = ScreenW(), sh3 = ScreenH();
            int panelX = sw3 - WIFI_PANEL_W - 16;
            int panelY = sh3 - TASKBAR_HEIGHT - WIFI_PANEL_H - 16;
            RECT pr; SetRect(&pr, panelX, panelY, panelX + WIFI_PANEL_W, panelY + WIFI_PANEL_H);
            if (!PtInRect(&pr, pt) && my < ScreenH() - TASKBAR_HEIGHT)
                g_wifiPanelOpen = false;
        }

        // Explorer click handling
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
            if (mx > wx2 + ww2 - 4 && mx < wx2 + ww2 + 4 && my > wy2 + 36) {
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
                RECT br; SetRect(&br, bx, wy2, bx + btnW2, wy2 + 36);
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
                            win.x = (ScreenW() - 940) / 2; win.y = (ScreenH() - 640) / 2;
                            win.w = 940; win.h = 640;
                        }
                    }
                    else if (b == 1) { win.animAlpha.SetTarget(0.0f); }
                    clickedExplorer = true; break;
                }
            }

            // Title bar drag
            if (my < wy2 + 36 && mx < wx2 + ww2 - btnW2 * 3) {
                g_explorerDragging = win.id; win.dragging = true;
                win.dragOffX = mx - wx2; win.dragOffY = my - wy2;
                SetCapture(hWnd); break;
            }

            // Navigation buttons
            int toolY2 = wy2 + 36;
            int navX2 = wx2 + 14;
            int navBtnSize = 28;

            RECT backRect; SetRect(&backRect, navX2, toolY2 + 6, navX2 + navBtnSize, toolY2 + 36);
            if (PtInRect(&backRect, pt) && win.historyIndex > 0) {
                win.historyIndex--;
                win.currentPath = win.pathHistory[win.historyIndex];
                if (win.currentPath.empty()) { win.title = L"This PC"; LoadDrives(win); }
                else {
                    std::wstring t2 = win.currentPath;
                    if (t2.back() == L'\\') t2.pop_back();
                    size_t pos = t2.find_last_of(L'\\');
                    if (pos != std::wstring::npos) t2 = t2.substr(pos + 1);
                    win.title = t2;
                    LoadDirectory(win, win.currentPath);
                }
                win.scrollOffset = 0; win.scrollAnim = SpringValue(0, 180, 15); win.targetScroll = 0;
                break;
            }
            navX2 += navBtnSize + 4;

            RECT fwdRect; SetRect(&fwdRect, navX2, toolY2 + 6, navX2 + navBtnSize, toolY2 + 36);
            if (PtInRect(&fwdRect, pt) && win.historyIndex < (int)win.pathHistory.size() - 1) {
                win.historyIndex++;
                win.currentPath = win.pathHistory[win.historyIndex];
                if (win.currentPath.empty()) { win.title = L"This PC"; LoadDrives(win); }
                else {
                    std::wstring t2 = win.currentPath;
                    if (t2.back() == L'\\') t2.pop_back();
                    size_t pos = t2.find_last_of(L'\\');
                    if (pos != std::wstring::npos) t2 = t2.substr(pos + 1);
                    win.title = t2;
                    LoadDirectory(win, win.currentPath);
                }
                win.scrollOffset = 0; win.scrollAnim = SpringValue(0, 180, 15); win.targetScroll = 0;
                break;
            }
            navX2 += navBtnSize + 4;

            RECT upRect; SetRect(&upRect, navX2, toolY2 + 6, navX2 + navBtnSize, toolY2 + 36);
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

            // Sidebar
            if (win.hoveredSidebarItem >= 0) {
                std::wstring paths[] = {
                    GetSpecialFolderPath(L"Desktop"), GetSpecialFolderPath(L"Downloads"),
                    GetSpecialFolderPath(L"Documents"), GetSpecialFolderPath(L"Pictures"),
                    GetSpecialFolderPath(L"Music"), GetSpecialFolderPath(L"Videos"), L""
                };
                NavigateExplorer(win, paths[win.hoveredSidebarItem]);
                break;
            }

            // Content item selection
            if (win.hoveredItem >= 0 && win.hoveredItem < (int)win.items.size()) {
                win.selectedItem = win.hoveredItem;
            }
            break;
        }

        // Taskbar clicks
        int sw3 = ScreenW(), sh3 = ScreenH();
        if (my >= sh3 - TASKBAR_HEIGHT) {
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

        // Desktop icon clicks
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

        if (!clickedIcon && my < sh3 - TASKBAR_HEIGHT) {
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
        if (!g_boot.complete) break;
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
        if (!g_boot.complete) break;
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
        g_contextMenuAnim = SpringValue(0, 400, 24);
        g_contextMenuAnim.SetTarget(1.0f);
        g_hoveredContextItem = -1;
        break;
    }

    case WM_KEYDOWN:
        if (!g_boot.complete) {
            // Allow ESC during boot to skip
            if (wParam == VK_ESCAPE || wParam == VK_SPACE) {
                g_boot.phase = BOOT_FADE_TO_DESKTOP;
                g_boot.phaseTime = 0;
                g_desktopFadeIn.SetTarget(1.0f);
            }
            break;
        }

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
    wc.lpszClassName = L"VORTEX_Desktop_v60";
    RegisterClassExW(&wc);

    int sw = ScreenW(), sh = ScreenH();
    g_hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"VORTEX_Desktop_v60", L"VORTEX Desktop v6.0",
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
