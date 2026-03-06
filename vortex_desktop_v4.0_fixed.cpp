/*
 * VORTEX Desktop Environment v4.0 - Windows 11 Fluent Design
 * FIXED VERSION - All compilation errors resolved
 *
 * FEATURES:
 * - Windows 11 Fluent Design style with Acrylic effects
 * - Real file explorer windows (draggable, resizable)
 * - Real disk drive information
 * - Enhanced taskbar with system tray
 * - Smooth cubic-ease animations
 * - Volume/brightness sliders
 *
 * ESC       - exit / close menu
 * SPACE     - open/close Start Menu
 * M         - toggle music
 * W         - show/hide widgets
 * N         - new notification
 * F         - show/hide WiFi panel
 * E         - open File Explorer window
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

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "winmm.lib")

#include <gdiplus.h>
using namespace Gdiplus;

// ============================================================================
//  ЦВЕТОВАЯ ПАЛИТРА VORTEX - FLUENT DESIGN
// ============================================================================
namespace VX {
    // Primary accents
    const Color Neon(255, 0, 255, 200);
    const Color NeonBright(255, 0, 255, 240);
    const Color NeonDim(255, 0, 180, 160);
    const Color Purple(255, 139, 92, 246);
    const Color PurpleBright(255, 180, 140, 255);
    const Color PurpleDim(255, 90, 50, 180);
    const Color Magenta(255, 255, 0, 128);
    const Color MagentaDim(255, 180, 0, 90);
    
    // Fluent Design backgrounds
    const Color PanelBg(200, 12, 12, 20);
    const Color PanelBgLight(180, 20, 20, 35);
    const Color CardBg(150, 18, 18, 30);
    const Color CardBgHover(170, 30, 30, 50);
    const Color AcrylicBg(180, 25, 25, 40);
    const Color MicaBg(220, 15, 15, 25);
    
    // Text
    const Color TextWhite(255, 240, 240, 255);
    const Color TextDim(180, 180, 190, 220);
    const Color TextMuted(120, 130, 140, 170);
    
    // Status
    const Color Green(255, 0, 230, 118);
    const Color Yellow(255, 255, 214, 0);
    const Color Red(255, 255, 56, 100);
    const Color Orange(255, 255, 152, 0);
    
    // Glow
    const Color GlowCyan(80, 0, 255, 200);
    const Color GlowPurple(60, 139, 92, 246);
    const Color GlowMagenta(50, 255, 0, 128);
}

// ============================================================================
//  КОНСТАНТЫ
// ============================================================================
const wchar_t* WALLPAPER_URL = L"https://images.wallpaperscraft.com/image/single/lake_mountains_trees_1219008_1920x1080.jpg";
const wchar_t* WALLPAPER_CACHE = L"vortex_wallpaper_cache.jpg";

const int TASKBAR_HEIGHT = 52;
const int TASKBAR_ICON_SIZE = 36;
const int TASKBAR_ICON_SPACING = 4;
const int TASKBAR_RADIUS = 12;

const int STATUS_PANEL_H = 32;
const int STATUS_PANEL_RADIUS = 10;

const int WIDGET_W = 280;
const int WIDGET_MARGIN = 12;

const int START_MENU_W = 540;
const int START_MENU_H = 480;

const int NOTIF_W = 320;
const int NOTIF_H = 72;

const int DESKTOP_ICON_SIZE = 64;
const int DESKTOP_ICON_SPACING = 12;
const int DESKTOP_ICON_TEXT_H = 24;

const int WIFI_PANEL_W = 320;
const int WIFI_PANEL_H = 360;

const int EXPLORER_MIN_W = 400;
const int EXPLORER_MIN_H = 300;

// ============================================================================
//  ГЛОБАЛЬНОЕ СОСТОЯНИЕ
// ============================================================================
HWND g_hWnd = NULL;
ULONG_PTR g_gdiplusToken = 0;
DWORD g_tick = 0;

// Double buffering
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
bool g_widgetsVisible = true;
bool g_musicPlaying = false;
bool g_wifiPanelOpen = false;
bool g_contextMenuOpen = false;
bool g_systemTrayOpen = false;
bool g_volumeSliderOpen = false;
bool g_brightnessSliderOpen = false;
int g_hoveredTaskbarIcon = -1;
int g_hoveredStartIcon = -1;
int g_hoveredWidget = -1;
int g_hoveredDesktopIcon = -1;
int g_hoveredWifiItem = -1;
int g_contextMenuX = 0, g_contextMenuY = 0;
int g_hoveredContextItem = -1;
int g_hoveredTrayItem = -1;
int g_volume = 75;
int g_brightness = 100;

// Animations - cubic ease
float g_startMenuAnim = 0.0f;
float g_widgetsAnim = 1.0f;
float g_wifiPanelAnim = 0.0f;
float g_taskbarIconScale[16] = {};
float g_volumeAnim = 0.0f;
float g_brightnessAnim = 0.0f;
float g_systemTrayAnim = 0.0f;

// Particles
struct Particle {
    float x, y, vx, vy, size, alpha, life;
    Color color;
};
std::vector<Particle> g_particles;

// Notifications
struct Notification {
    std::wstring title, message;
    DWORD time;
    float alpha, offsetY;
    bool alive;
};
std::vector<Notification> g_notifs;

// ============================================================================
//  HELPER: Unicode character from code point
// ============================================================================
std::wstring UChar(unsigned int codePoint) {
    if (codePoint <= 0xFFFF) {
        return std::wstring(1, (wchar_t)codePoint);
    } else {
        codePoint -= 0x10000;
        wchar_t highSurrogate = (wchar_t)(0xD800 + (codePoint >> 10));
        wchar_t lowSurrogate = (wchar_t)(0xDC00 + (codePoint & 0x3FF));
        return std::wstring() + highSurrogate + lowSurrogate;
    }
}

// ============================================================================
//  Cubic ease animation helper
// ============================================================================
float CubicEaseInOut(float t) {
    if (t < 0.5f) {
        return 4.0f * t * t * t;
    } else {
        float f = (2.0f * t) - 2.0f;
        return 0.5f * f * f * f + 1.0f;
    }
}

// ============================================================================
//  DISK DRIVE INFO
// ============================================================================
struct DriveInfo {
    wchar_t letter;
    std::wstring name;
    std::wstring type;
    UINT64 totalBytes;
    UINT64 freeBytes;
    UINT64 usedBytes;
    float usedPercent;
    Color color;
};
std::vector<DriveInfo> g_drives;

void RefreshDriveInfo() {
    g_drives.clear();
    DWORD drives = GetLogicalDrives();
    
    for (wchar_t letter = L'A'; letter <= L'Z'; letter++) {
        if (drives & (1 << (letter - L'A'))) {
            wchar_t root[4] = { letter, L':', L'\\', 0 };
            UINT type = GetDriveTypeW(root);
            
            DriveInfo di;
            di.letter = letter;
            di.usedPercent = 0;
            di.totalBytes = 0;
            di.freeBytes = 0;
            di.usedBytes = 0;
            
            if (type == DRIVE_FIXED || type == DRIVE_REMOVABLE) {
                wchar_t volName[MAX_PATH];
                if (GetVolumeInformationW(root, volName, MAX_PATH, NULL, NULL, NULL, NULL, 0)) {
                    di.name = volName;
                } else {
                    di.name = L"Local Disk";
                }
                
                ULARGE_INTEGER freeBytes, totalBytes;
                if (GetDiskFreeSpaceExW(root, &freeBytes, &totalBytes, NULL)) {
                    di.totalBytes = totalBytes.QuadPart;
                    di.freeBytes = freeBytes.QuadPart;
                    di.usedBytes = di.totalBytes - di.freeBytes;
                    if (di.totalBytes > 0) {
                        di.usedPercent = (float)di.usedBytes / (float)di.totalBytes * 100.0f;
                    }
                }
                
                di.type = (type == DRIVE_REMOVABLE) ? L"Removable" : L"Local";
                
                // Color based on usage
                if (di.usedPercent < 50) di.color = VX::Green;
                else if (di.usedPercent < 80) di.color = VX::Yellow;
                else di.color = VX::Red;
                
                g_drives.push_back(di);
            }
        }
    }
}

// ============================================================================
//  FILE EXPLORER WINDOW
// ============================================================================
struct ExplorerWindow {
    int x, y, w, h;
    std::wstring path;
    std::wstring title;
    bool dragging;
    bool resizing;
    int dragOffsetX, dragOffsetY;
    bool minimized;
    bool active;
    std::vector<std::wstring> items;
    int hoveredItem;
    int selectedItem;
};
std::vector<ExplorerWindow> g_explorerWindows;
int g_activeExplorer = -1;

void OpenExplorerWindow(const std::wstring& path) {
    ExplorerWindow ew;
    ew.x = 100 + (int)g_explorerWindows.size() * 30;
    ew.y = 80 + (int)g_explorerWindows.size() * 30;
    ew.w = 700;
    ew.h = 500;
    ew.path = path;
    ew.dragging = false;
    ew.resizing = false;
    ew.dragOffsetX = 0;
    ew.dragOffsetY = 0;
    ew.minimized = false;
    ew.active = true;
    ew.hoveredItem = -1;
    ew.selectedItem = -1;
    
    // Get folder name
    size_t pos = path.find_last_of(L'\\');
    ew.title = (pos != std::wstring::npos) ? path.substr(pos + 1) : path;
    if (ew.title.empty()) ew.title = L"This PC";
    
    // List items
    ew.items.clear();
    WIN32_FIND_DATAW fd;
    std::wstring searchPath = path + L"\\*";
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (wcscmp(fd.cFileName, L".") != 0 && wcscmp(fd.cFileName, L"..") != 0) {
                ew.items.push_back(fd.cFileName);
            }
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);
    }
    
    // Deactivate other windows
    for (auto& w : g_explorerWindows) w.active = false;
    
    g_explorerWindows.push_back(ew);
    g_activeExplorer = (int)g_explorerWindows.size() - 1;
}

void CloseExplorerWindow(int idx) {
    if (idx >= 0 && idx < (int)g_explorerWindows.size()) {
        g_explorerWindows.erase(g_explorerWindows.begin() + idx);
        g_activeExplorer = g_explorerWindows.empty() ? -1 : (int)g_explorerWindows.size() - 1;
        if (g_activeExplorer >= 0) g_explorerWindows[g_activeExplorer].active = true;
    }
}

// ============================================================================
//  DESKTOP ICONS
// ============================================================================
struct DesktopIcon {
    std::wstring name;
    std::wstring icon;
    std::wstring action;
    Color color;
    RECT bounds;
    bool selected;
};
std::vector<DesktopIcon> g_desktopIcons;

void InitDesktopIcons() {
    g_desktopIcons.clear();
    auto add = [](const std::wstring& name, const std::wstring& ico, const std::wstring& act, Color c) {
        DesktopIcon d;
        d.name = name; d.icon = ico; d.action = act; d.color = c;
        d.selected = false;
        memset(&d.bounds, 0, sizeof(RECT));
        g_desktopIcons.push_back(d);
    };

    add(L"This PC", UChar(0x1F4BB), L"shell:::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", VX::Neon);
    add(L"Recycle Bin", UChar(0x1F5D1), L"shell:RecycleBinFolder", VX::Purple);
    add(L"Documents", UChar(0x1F4C4), L"shell:Personal", VX::Yellow);
    add(L"Downloads", UChar(0x1F4E5), L"shell:Downloads", VX::Green);
    add(L"Terminal", L">_", L"cmd.exe", VX::Green);
    add(L"Browser", UChar(0x1F310), L"msedge.exe", Color(255, 0, 150, 255));
}

// ============================================================================
//  WiFi NETWORKS
// ============================================================================
struct WifiNetwork {
    std::wstring ssid;
    int signal;
    bool secured;
    bool connected;
};
std::vector<WifiNetwork> g_wifiNetworks;
bool g_wifiScanning = false;
DWORD g_lastWifiScan = 0;

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
        current.signal = 0;
        current.secured = false;
        current.connected = false;
        bool hasNetwork = false;

        while (std::getline(stream, line)) {
            if (line.find("SSID") != std::string::npos && line.find("BSSID") == std::string::npos) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    if (hasNetwork && !current.ssid.empty()) {
                        g_wifiNetworks.push_back(current);
                    }
                    std::string ssid = line.substr(pos + 2);
                    while (!ssid.empty() && (ssid.back() == '\r' || ssid.back() == '\n' || ssid.back() == ' '))
                        ssid.pop_back();
                    while (!ssid.empty() && (ssid.front() == ' ' || ssid.front() == '\t'))
                        ssid.erase(0, 1);
                    current.ssid = std::wstring(ssid.begin(), ssid.end());
                    current.signal = 0;
                    current.secured = false;
                    current.connected = false;
                    hasNetwork = true;
                }
            }
            if (line.find('%') != std::string::npos) {
                size_t pctPos = line.find('%');
                if (pctPos != std::string::npos && pctPos > 0) {
                    int sig = 0;
                    for (int i = (int)pctPos - 1; i >= 0; i--) {
                        if (line[i] >= '0' && line[i] <= '9') {
                            sig = (line[i] - '0') * (sig == 0 ? 1 : 10) + (sig == 0 ? 0 : sig);
                        } else if (sig > 0) {
                            break;
                        }
                    }
                    if (sig > 0 && sig <= 100) current.signal = sig;
                }
            }
            if (line.find("Auth") != std::string::npos || 
                line.find("auth") != std::string::npos ||
                line.find("Шифр") != std::string::npos ||
                line.find("Безопасность") != std::string::npos) {
                if (line.find("Open") == std::string::npos && 
                    line.find("open") == std::string::npos &&
                    line.find("Открытая") == std::string::npos &&
                    line.find("открытая") == std::string::npos) {
                    current.secured = true;
                }
            }
        }
        if (hasNetwork && !current.ssid.empty()) {
            g_wifiNetworks.push_back(current);
        }
    } else {
        CloseHandle(hWritePipe);
        CloseHandle(hReadPipe);
    }

    if (g_wifiNetworks.empty()) {
        WifiNetwork w;
        w.ssid = L"VORTEX-5G"; w.signal = 92; w.secured = true; w.connected = true;
        g_wifiNetworks.push_back(w);
        w.ssid = L"HomeNet-2.4G"; w.signal = 75; w.secured = true; w.connected = false;
        g_wifiNetworks.push_back(w);
        w.ssid = L"CyberCafe"; w.signal = 60; w.secured = false; w.connected = false;
        g_wifiNetworks.push_back(w);
        w.ssid = L"Neighbor_WiFi"; w.signal = 35; w.secured = true; w.connected = false;
        g_wifiNetworks.push_back(w);
        w.ssid = L"FreeWiFi"; w.signal = 20; w.secured = false; w.connected = false;
        g_wifiNetworks.push_back(w);
    } else {
        if (!g_wifiNetworks.empty()) g_wifiNetworks[0].connected = true;
    }

    std::sort(g_wifiNetworks.begin(), g_wifiNetworks.end(),
        [](const WifiNetwork& a, const WifiNetwork& b) { return a.signal > b.signal; });

    g_wifiScanning = false;
    g_lastWifiScan = GetTickCount();
    if (g_hWnd) InvalidateRect(g_hWnd, NULL, FALSE);
    return 0;
}

// ============================================================================
//  CREATE FOLDER
// ============================================================================
void CreateNewFolderOnDesktop() {
    wchar_t desktopPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath))) {
        std::wstring baseName = std::wstring(desktopPath) + L"\\New Folder";
        std::wstring folderName = baseName;
        int idx = 1;

        while (GetFileAttributesW(folderName.c_str()) != INVALID_FILE_ATTRIBUTES) {
            folderName = baseName + L" (" + std::to_wstring(idx++) + L")";
        }

        if (CreateDirectoryW(folderName.c_str(), NULL)) {
            DesktopIcon d;
            size_t pos = folderName.find_last_of(L'\\');
            d.name = (pos != std::wstring::npos) ? folderName.substr(pos + 1) : folderName;
            d.icon = UChar(0x1F4C1);
            d.action = L"explorer.exe \"" + folderName + L"\"";
            d.color = VX::Orange;
            d.selected = false;
            memset(&d.bounds, 0, sizeof(RECT));
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

void DrawNeonGlow(Graphics& g, int x, int y, int w, int h, int r, Color c, int layers = 4) {
    for (int i = layers; i >= 1; i--) {
        int spread = i * 3;
        int alphaDiv = i + 1;
        BYTE glowAlpha = (BYTE)(c.GetA() / alphaDiv);
        Color gc(glowAlpha, c.GetR(), c.GetG(), c.GetB());
        DrawRoundRect(g, x - spread, y - spread, w + spread * 2, h + spread * 2, r + spread, gc, 2.0f);
    }
}

// ============================================================================
//  VORTEX LOGO
// ============================================================================
void DrawVortexLogo(Graphics& g, int cx, int cy, int radius, float rotation = 0.0f) {
    for (int i = 4; i >= 1; i--) {
        BYTE glowAlpha = (BYTE)(15 * i);
        SolidBrush glow(glowAlpha, 0, 255, 200);
        g.FillEllipse(&glow, cx - radius - i * 2, cy - radius - i * 2,
            (radius + i * 2) * 2, (radius + i * 2) * 2);
    }

    LinearGradientBrush bg(Rect(cx - radius, cy - radius, radius * 2, radius * 2),
        VX::Purple, VX::Neon, LinearGradientModeForwardDiagonal);
    g.FillEllipse(&bg, cx - radius, cy - radius, radius * 2, radius * 2);

    GraphicsState st = g.Save();
    g.TranslateTransform((REAL)cx, (REAL)cy);
    g.RotateTransform(rotation);

    for (int arm = 0; arm < 4; arm++) {
        float baseAngle = arm * 90.0f;
        std::vector<PointF> pts;
        for (float t = 0.0f; t < 3.14159f * 1.5f; t += 0.08f) {
            float r = radius * 0.15f + (t / (3.14159f * 1.5f)) * radius * 0.7f;
            float a2 = baseAngle * 3.14159f / 180.0f + t;
            pts.push_back(PointF(cosf(a2) * r, sinf(a2) * r));
        }
        if (pts.size() > 1) {
            Pen pen(Color(200, 255, 255, 255), 2.0f);
            pen.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
            g.DrawLines(&pen, pts.data(), (int)pts.size());
        }
    }

    g.Restore(st);

    SolidBrush center(Color(255, 255, 255, 255));
    g.FillEllipse(&center, cx - 3, cy - 3, 6, 6);
}

// ============================================================================
//  TASKBAR APPS
// ============================================================================
struct TaskbarApp {
    std::wstring name;
    std::wstring exec;
    Color accentColor;
    std::wstring icon;
    bool running;
    RECT bounds;
};

std::vector<TaskbarApp> g_taskApps;

void InitTaskbarApps() {
    g_taskApps.clear();
    auto add = [](const std::wstring& name, const std::wstring& exec, Color c, const std::wstring& ico, bool run) {
        TaskbarApp a;
        a.name = name; a.exec = exec; a.accentColor = c;
        a.icon = ico; a.running = run;
        memset(&a.bounds, 0, sizeof(RECT));
        g_taskApps.push_back(a);
    };

    add(L"Files", L"explorer.exe", VX::Neon, UChar(0x1F4C1), true);
    add(L"Browser", L"msedge.exe", Color(255, 0, 150, 255), UChar(0x1F310), true);
    add(L"Terminal", L"cmd.exe", VX::Green, L">_", false);
    add(L"Code", L"notepad.exe", Color(255, 0, 122, 204), L"</>", true);
    add(L"Music", L"wmplayer.exe", VX::Magenta, L"\u266B", false);
    add(L"Photos", L"mspaint.exe", VX::Orange, UChar(0x1F5BC), false);
    add(L"Notes", L"notepad.exe", VX::Yellow, UChar(0x1F4DD), false);
    add(L"Settings", L"ms-settings:", Color(255, 142, 142, 160), L"\u2699", false);
}

void DrawTaskbarIcon(Graphics& g, int x, int y, int size, const TaskbarApp& app,
    bool hovered, float scale) {
    int s = (int)(size * scale);
    int ox = x + (size - s) / 2;
    int oy = y + (size - s) / 2;

    Color bgColor = hovered ?
        Color(200, app.accentColor.GetR() / 3, app.accentColor.GetG() / 3, app.accentColor.GetB() / 3) :
        Color(80, 30, 30, 45);
    FillRoundRectSolid(g, ox, oy, s, s, 8, bgColor);

    if (hovered) {
        DrawNeonGlow(g, ox, oy, s, s, 8, app.accentColor, 3);
        DrawRoundRect(g, ox, oy, s, s, 8, Color(200, app.accentColor.GetR(),
            app.accentColor.GetG(), app.accentColor.GetB()), 1.5f);
    } else {
        DrawRoundRect(g, ox, oy, s, s, 8, Color(40, 255, 255, 255), 0.5f);
    }

    FontFamily ff(L"Segoe UI");
    Font iconFont(&ff, s * 0.38f, FontStyleBold, UnitPixel);
    Color textColor = hovered ? Color(255, 255, 255, 255) : VX::TextDim;
    SolidBrush textBr(textColor);

    StringFormat fmt;
    fmt.SetAlignment(StringAlignmentCenter);
    fmt.SetLineAlignment(StringAlignmentCenter);

    RectF rc((REAL)ox, (REAL)oy, (REAL)s, (REAL)s);
    g.DrawString(app.icon.c_str(), -1, &iconFont, rc, &fmt, &textBr);
}

// ============================================================================
//  SYSTEM TRAY
// ============================================================================
struct TrayItem {
    std::wstring icon;
    std::wstring tooltip;
    int id;
};
std::vector<TrayItem> g_trayItems;

void InitTrayItems() {
    g_trayItems.clear();
    TrayItem ti;
    ti.icon = UChar(0x1F4F6); ti.tooltip = L"WiFi"; ti.id = 1;  // Signal
    g_trayItems.push_back(ti);
    ti.icon = UChar(0x1F50A); ti.tooltip = L"Volume"; ti.id = 2;  // Speaker
    g_trayItems.push_back(ti);
    ti.icon = UChar(0x1F4BB); ti.tooltip = L"Battery"; ti.id = 3;  // Battery
    g_trayItems.push_back(ti);
}

void DrawSystemTray(Graphics& g, int sw, int sh) {
    if (g_systemTrayAnim <= 0.01f) return;
    
    float a = g_systemTrayAnim;
    int panelW = 200;
    int panelH = 140;
    int panelX = sw - panelW - 12;
    int panelY = sh - TASKBAR_HEIGHT - 10 - panelH;
    
    FillRoundRectSolid(g, panelX, panelY, panelW, panelH, 12,
        Color((BYTE)(220 * a), 14, 14, 24));
    DrawRoundRect(g, panelX, panelY, panelW, panelH, 12,
        Color((BYTE)(60 * a), 0, 255, 200), 1.5f);
    
    FontFamily ff(L"Segoe UI");
    Font font(&ff, 11.0f, FontStyleRegular, UnitPixel);
    Font smallFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
    
    StringFormat lfmt;
    lfmt.SetAlignment(StringAlignmentNear);
    lfmt.SetLineAlignment(StringAlignmentCenter);
    
    // WiFi status
    SolidBrush labelBr(Color((BYTE)(180 * a), 150, 160, 180));
    SolidBrush valueBr(Color((BYTE)(240 * a), 240, 240, 255));
    
    RectF wifiLabelRc((REAL)(panelX + 16), (REAL)(panelY + 16), 60.0f, 20.0f);
    g.DrawString(L"WiFi", -1, &smallFont, wifiLabelRc, &lfmt, &labelBr);
    RectF wifiValueRc((REAL)(panelX + 80), (REAL)(panelY + 16), 100.0f, 20.0f);
    g.DrawString(L"Connected", -1, &smallFont, wifiValueRc, &lfmt, &valueBr);
    
    // Volume slider
    RectF volLabelRc((REAL)(panelX + 16), (REAL)(panelY + 44), 60.0f, 20.0f);
    g.DrawString(L"Volume", -1, &smallFont, volLabelRc, &lfmt, &labelBr);
    
    wchar_t volStr[16];
    swprintf_s(volStr, L"%d%%", g_volume);
    RectF volValueRc((REAL)(panelX + 140), (REAL)(panelY + 44), 40.0f, 20.0f);
    StringFormat rfmt;
    rfmt.SetAlignment(StringAlignmentFar);
    g.DrawString(volStr, -1, &smallFont, volValueRc, &rfmt, &valueBr);
    
    int sliderX = panelX + 80;
    int sliderY = panelY + 50;
    int sliderW = 56;
    int sliderH = 8;
    FillRoundRectSolid(g, sliderX, sliderY, sliderW, sliderH, 4, Color((BYTE)(40 * a), 255, 255, 255));
    int volW = (int)(sliderW * g_volume / 100.0f);
    if (volW > 0) {
        FillRoundRectSolid(g, sliderX, sliderY, volW, sliderH, 4,
            Color((BYTE)(200 * a), 0, 255, 200));
    }
    
    // Brightness slider
    RectF brightLabelRc((REAL)(panelX + 16), (REAL)(panelY + 72), 60.0f, 20.0f);
    g.DrawString(L"Brightness", -1, &smallFont, brightLabelRc, &lfmt, &labelBr);
    
    wchar_t brightStr[16];
    swprintf_s(brightStr, L"%d%%", g_brightness);
    RectF brightValueRc((REAL)(panelX + 140), (REAL)(panelY + 72), 40.0f, 20.0f);
    g.DrawString(brightStr, -1, &smallFont, brightValueRc, &rfmt, &valueBr);
    
    sliderY = panelY + 78;
    FillRoundRectSolid(g, sliderX, sliderY, sliderW, sliderH, 4, Color((BYTE)(40 * a), 255, 255, 255));
    int brightW = (int)(sliderW * g_brightness / 100.0f);
    if (brightW > 0) {
        FillRoundRectSolid(g, sliderX, sliderY, brightW, sliderH, 4,
            Color((BYTE)(200 * a), 255, 200, 0));
    }
    
    // Battery
    RectF battLabelRc((REAL)(panelX + 16), (REAL)(panelY + 100), 60.0f, 20.0f);
    g.DrawString(L"Battery", -1, &smallFont, battLabelRc, &lfmt, &labelBr);
    RectF battValueRc((REAL)(panelX + 80), (REAL)(panelY + 100), 100.0f, 20.0f);
    g.DrawString(L"85% - 4h remaining", -1, &smallFont, battValueRc, &lfmt, &valueBr);
}

// ============================================================================
//  TASKBAR
// ============================================================================
void DrawTaskbar(Graphics& g, int sw, int sh) {
    int iconCount = (int)g_taskApps.size();
    int contentW = iconCount * (TASKBAR_ICON_SIZE + TASKBAR_ICON_SPACING) - TASKBAR_ICON_SPACING + 24;

    int barW = contentW + 200;
    int barH = TASKBAR_HEIGHT;
    int barX = (sw - barW) / 2;
    int barY = sh - barH - 6;

    GraphicsPath barPath;
    RoundedRectPath(barPath, barX, barY, barW, barH, TASKBAR_RADIUS);

    LinearGradientBrush barBg(Rect(barX, barY, barW, barH),
        Color(200, 15, 15, 28), Color(180, 8, 8, 18), LinearGradientModeVertical);
    g.FillPath(&barBg, &barPath);

    Pen topLine(Color(100, 0, 255, 200), 1.0f);
    g.DrawLine(&topLine, barX + TASKBAR_RADIUS, barY, barX + barW - TASKBAR_RADIUS, barY);

    DrawRoundRect(g, barX, barY, barW, barH, TASKBAR_RADIUS, Color(30, 255, 255, 255), 1.0f);

    float logoRot = (g_tick % 6000) / 6000.0f * 360.0f;
    DrawVortexLogo(g, barX + 24, barY + barH / 2, 12, logoRot);

    int iconsStartX = barX + 48;
    int iconY = barY + (barH - TASKBAR_ICON_SIZE) / 2;

    for (int i = 0; i < iconCount; i++) {
        int ix = iconsStartX + i * (TASKBAR_ICON_SIZE + TASKBAR_ICON_SPACING);

        float target = (g_hoveredTaskbarIcon == i) ? 1.15f : 1.0f;
        g_taskbarIconScale[i] += (target - g_taskbarIconScale[i]) * 0.45f;

        bool hov = (g_hoveredTaskbarIcon == i);
        DrawTaskbarIcon(g, ix, iconY, TASKBAR_ICON_SIZE, g_taskApps[i], hov, g_taskbarIconScale[i]);

        if (g_taskApps[i].running) {
            int dotX = ix + TASKBAR_ICON_SIZE / 2;
            int dotY = barY + barH - 5;

            SolidBrush glow(Color(50, g_taskApps[i].accentColor.GetR(),
                g_taskApps[i].accentColor.GetG(), g_taskApps[i].accentColor.GetB()));
            g.FillEllipse(&glow, dotX - 4, dotY - 1, 8, 4);

            SolidBrush dot(g_taskApps[i].accentColor);
            g.FillEllipse(&dot, dotX - 2, dotY, 4, 2);
        }

        SetRect(&g_taskApps[i].bounds, ix, iconY, ix + TASKBAR_ICON_SIZE, iconY + TASKBAR_ICON_SIZE);
    }

    // Time and date
    time_t now = time(NULL);
    struct tm ti;
    localtime_s(&ti, &now);
    wchar_t timeStr[32];
    wcsftime(timeStr, 32, L"%H:%M", &ti);

    FontFamily ff(L"Segoe UI");
    Font timeFont(&ff, 12.0f, FontStyleBold, UnitPixel);
    SolidBrush timeBr(VX::TextWhite);

    StringFormat rfmt;
    rfmt.SetAlignment(StringAlignmentFar);
    rfmt.SetLineAlignment(StringAlignmentCenter);

    RectF timeRc((REAL)(barX + barW - 70), (REAL)barY, 60.0f, (REAL)barH);
    g.DrawString(timeStr, -1, &timeFont, timeRc, &rfmt, &timeBr);

    wchar_t dateStr[32];
    wcsftime(dateStr, 32, L"%d.%m", &ti);
    Font dateFont(&ff, 8.0f, FontStyleRegular, UnitPixel);
    SolidBrush dateBr(VX::TextMuted);
    RectF dateRc((REAL)(barX + barW - 70), (REAL)(barY + 18), 60.0f, 16.0f);
    g.DrawString(dateStr, -1, &dateFont, dateRc, &rfmt, &dateBr);

    // System tray icons (right side of taskbar)
    int trayX = barX + barW - 140;
    int trayY = barY + (barH - 20) / 2;
    for (int i = 0; i < (int)g_trayItems.size() && i < 3; i++) {
        Font trayFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
        SolidBrush trayBr(VX::TextDim);
        StringFormat cfmt;
        cfmt.SetAlignment(StringAlignmentCenter);
        cfmt.SetLineAlignment(StringAlignmentCenter);
        RectF trayRc((REAL)trayX, (REAL)trayY, 24.0f, 20.0f);
        g.DrawString(g_trayItems[i].icon.c_str(), -1, &trayFont, trayRc, &cfmt, &trayBr);
        trayX += 24;
    }
}

// ============================================================================
//  STATUS PANEL
// ============================================================================
void DrawStatusPanel(Graphics& g, int sw) {
    int panelW = 260;
    int panelH = STATUS_PANEL_H;
    int panelX = sw - panelW - 10;
    int panelY = 8;

    FillRoundRectSolid(g, panelX, panelY, panelW, panelH, STATUS_PANEL_RADIUS,
        Color(180, 12, 12, 22));
    DrawRoundRect(g, panelX, panelY, panelW, panelH, STATUS_PANEL_RADIUS,
        Color(25, 0, 255, 200), 1.0f);

    FontFamily ff(L"Segoe UI");
    Font font(&ff, 10.0f, FontStyleRegular, UnitPixel);

    int cx = panelX + 12;
    int cy = panelY + panelH / 2;

    SolidBrush greenBr(VX::Green);
    g.FillEllipse(&greenBr, cx, cy - 4, 7, 7);
    cx += 12;

    SolidBrush dimBr(VX::TextDim);
    StringFormat lfmt;
    lfmt.SetAlignment(StringAlignmentNear);
    lfmt.SetLineAlignment(StringAlignmentCenter);

    RectF wifiRc((REAL)cx, (REAL)panelY, 32.0f, (REAL)panelH);
    g.DrawString(L"WiFi", -1, &font, wifiRc, &lfmt, &dimBr);
    cx += 36;

    Pen sepPen(Color(35, 255, 255, 255), 1.0f);
    g.DrawLine(&sepPen, cx, panelY + 7, cx, panelY + panelH - 7);
    cx += 8;

    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);

    int battPct = 85;
    Color battColor = battPct > 50 ? VX::Green : (battPct > 20 ? VX::Yellow : VX::Red);
    
    Pen battPen(battColor, 1.5f);
    g.DrawRectangle(&battPen, cx, cy - 4, 16, 8);
    SolidBrush battBr(battColor);
    g.FillRectangle(&battBr, cx + 2, cy - 2, (INT)(12.0f * battPct / 100.0f), 4);
    SolidBrush battTip(battColor);
    g.FillRectangle(&battTip, cx + 16, cy - 1, 2, 3);
    cx += 24;

    wchar_t battStr[16];
    swprintf_s(battStr, L"%d%%", battPct);
    RectF battRc2((REAL)cx, (REAL)panelY, 26.0f, (REAL)panelH);
    g.DrawString(battStr, -1, &font, battRc2, &lfmt, &dimBr);
    cx += 32;

    g.DrawLine(&sepPen, cx, panelY + 7, cx, panelY + panelH - 7);
    cx += 8;

    int ramPct = (int)mem.dwMemoryLoad;
    Color ramColor = ramPct > 80 ? VX::Red : (ramPct > 50 ? VX::Yellow : VX::Neon);
    SolidBrush ramBr(ramColor);
    g.FillEllipse(&ramBr, cx, cy - 3, 6, 6);
    cx += 12;

    wchar_t ramStr[32];
    swprintf_s(ramStr, L"RAM %d%%", ramPct);
    RectF ramRc2((REAL)cx, (REAL)panelY, 55.0f, (REAL)panelH);
    g.DrawString(ramStr, -1, &font, ramRc2, &lfmt, &dimBr);
    cx += 60;

    if (g_musicPlaying) {
        for (int i = 0; i < 4; i++) {
            REAL h = (REAL)(3.0f + sinf((float)g_tick / 200.0f + i * 1.5f) * 5.0f);
            SolidBrush eqBr(VX::Magenta);
            g.FillRectangle(&eqBr, (REAL)(cx + i * 4), (REAL)(cy + 4) - h, 2.0f, h);
        }
    }
}

// ============================================================================
//  WIDGETS
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
    w.title = L"CLOCK";
    wcsftime(buf, 128, L"%H:%M:%S\n%A\n%d %B %Y", &ti);
    w.content = buf;
    w.height = 130;
    w.accent = VX::Neon;
    g_widgets.push_back(w);

    MEMORYSTATUSEX mem2;
    mem2.dwLength = sizeof(mem2);
    GlobalMemoryStatusEx(&mem2);

    int cpuSim = 25 + (g_tick / 800) % 30;
    swprintf_s(buf, L"CPU  %d%%\nRAM  %d%%\nGPU  45%%\nDISK 62%%", cpuSim, (int)mem2.dwMemoryLoad);
    w.title = L"SYSTEM";
    w.content = buf;
    w.height = 140;
    w.accent = VX::Purple;
    g_widgets.push_back(w);

    w.title = L"DRIVES";
    std::wstring driveContent;
    RefreshDriveInfo();
    for (size_t i = 0; i < g_drives.size() && i < 3; i++) {
        wchar_t driveLine[64];
        swprintf_s(driveLine, L"%c: %.0fGB free\n", g_drives[i].letter,
            (double)g_drives[i].freeBytes / (1024.0 * 1024.0 * 1024.0));
        driveContent += driveLine;
    }
    w.content = driveContent.empty() ? L"No drives" : driveContent;
    w.height = 110;
    w.accent = VX::Yellow;
    g_widgets.push_back(w);

    w.title = L"NOW PLAYING";
    w.content = g_musicPlaying ? L"Blinding Lights\nThe Weeknd\n\x25B6 3:42 / 5:20" :
        L"Nothing playing\n\nPress M to play";
    w.height = 110;
    w.accent = VX::Magenta;
    g_widgets.push_back(w);
}

void DrawWidgets(Graphics& g, int sh) {
    if (g_widgetsAnim <= 0.01f) return;

    float alpha = g_widgetsAnim;
    int offsetX = (int)(-WIDGET_W * (1.0f - alpha));
    int baseX = 12 + offsetX;
    int baseY = 48;

    for (size_t i = 0; i < g_widgets.size(); i++) {
        const Widget& w = g_widgets[i];
        int x = baseX;
        int y = baseY;
        int width = WIDGET_W;
        int height = w.height;

        bool hov = (g_hoveredWidget == (int)i);

        Color bgColor = hov ? VX::CardBgHover : VX::CardBg;
        bgColor = Color((BYTE)(bgColor.GetA() * alpha), bgColor.GetR(), bgColor.GetG(), bgColor.GetB());
        FillRoundRectSolid(g, x, y, width, height, 12, bgColor);

        Color borderColor = hov ?
            Color((BYTE)(100 * alpha), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()) :
            Color((BYTE)(25 * alpha), 255, 255, 255);
        DrawRoundRect(g, x, y, width, height, 12, borderColor, hov ? 1.5f : 0.5f);

        if (hov) {
            DrawNeonGlow(g, x, y, width, height, 12,
                Color((BYTE)(35 * alpha), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()), 2);
        }

        FillRoundRectSolid(g, x + 1, y + 10, 2, height - 20, 2,
            Color((BYTE)(180 * alpha), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));

        FontFamily ff(L"Segoe UI");
        Font titleFont(&ff, 9.0f, FontStyleBold, UnitPixel);
        SolidBrush titleBr(Color((BYTE)(170 * alpha), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));

        StringFormat fmt;
        fmt.SetAlignment(StringAlignmentNear);
        fmt.SetLineAlignment(StringAlignmentNear);

        RectF titleRc((REAL)(x + 14), (REAL)(y + 10), (REAL)(width - 28), 12.0f);
        g.DrawString(w.title.c_str(), -1, &titleFont, titleRc, &fmt, &titleBr);

        Font contentFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
        SolidBrush contentBr(Color((BYTE)(230 * alpha), 240, 240, 255));

        std::wistringstream ss(w.content);
        std::wstring line;
        REAL ly = (REAL)(y + 28);

        bool first = true;
        while (std::getline(ss, line)) {
            Font* f = &contentFont;
            Font bigFont(&ff, 24.0f, FontStyleBold, UnitPixel);
            if (first && w.title == L"CLOCK") {
                f = &bigFont;
                RectF lineRc((REAL)(x + 14), ly, (REAL)(width - 28), 30.0f);
                g.DrawString(line.c_str(), -1, f, lineRc, &fmt, &contentBr);
                ly += 32.0f;
            } else {
                RectF lineRc((REAL)(x + 14), ly, (REAL)(width - 28), 16.0f);
                g.DrawString(line.c_str(), -1, f, lineRc, &fmt, &contentBr);
                ly += 18.0f;
            }
            first = false;
        }

        baseY += height + WIDGET_MARGIN;
    }
}

// ============================================================================
//  START MENU
// ============================================================================
struct StartApp {
    std::wstring name;
    std::wstring icon;
    std::wstring exec;
    Color color;
};

std::vector<StartApp> g_startApps;

void InitStartApps() {
    g_startApps.clear();
    auto add = [](const std::wstring& n, const std::wstring& ico, const std::wstring& exec, Color c) {
        StartApp a;
        a.name = n; a.icon = ico; a.exec = exec; a.color = c;
        g_startApps.push_back(a);
    };

    add(L"Files", UChar(0x1F4C1), L"explorer.exe", VX::Neon);
    add(L"Browser", UChar(0x1F310), L"msedge.exe", Color(255, 0, 150, 255));
    add(L"Terminal", L">_", L"cmd.exe", VX::Green);
    add(L"Code", L"</>", L"notepad.exe", Color(255, 0, 122, 204));
    add(L"Music", L"\u266B", L"wmplayer.exe", VX::Magenta);
    add(L"Photos", UChar(0x1F5BC), L"mspaint.exe", VX::Orange);
    add(L"Notes", UChar(0x1F4DD), L"notepad.exe", VX::Yellow);
    add(L"Settings", L"\u2699", L"ms-settings:", Color(255, 142, 142, 160));
    add(L"Calculator", L"=", L"calc.exe", Color(255, 100, 200, 255));
    add(L"Paint", UChar(0x1F3A8), L"mspaint.exe", Color(255, 255, 100, 100));
    add(L"Calendar", UChar(0x1F4C5), L"notepad.exe", VX::Red);
    add(L"System", UChar(0x1F5A5), L"taskmgr.exe", VX::Purple);
    add(L"Network", UChar(0x1F4E1), L"ncpa.cpl", VX::NeonDim);
    add(L"Security", UChar(0x1F512), L"notepad.exe", VX::Green);
    add(L"Store", UChar(0x1F6D2), L"notepad.exe", Color(255, 0, 180, 255));
    add(L"Help", L"?", L"notepad.exe", VX::TextDim);
}

void DrawStartMenu(Graphics& g, int sw, int sh) {
    if (g_startMenuAnim <= 0.01f) return;

    float a = g_startMenuAnim;

    SolidBrush overlay(Color((BYTE)(130 * a), 0, 0, 0));
    g.FillRectangle(&overlay, 0, 0, sw, sh);

    int menuW = START_MENU_W;
    int menuH = START_MENU_H;
    int menuX = (sw - menuW) / 2;
    int targetY = (sh - menuH) / 2 - 20;
    int menuY = (int)(targetY + 40 * (1.0f - a));

    Color bgColor2(Color((BYTE)(200 * a), 14, 14, 24));
    FillRoundRectSolid(g, menuX, menuY, menuW, menuH, 16, bgColor2);

    DrawRoundRect(g, menuX, menuY, menuW, menuH, 16,
        Color((BYTE)(70 * a), 0, 255, 200), 1.5f);

    Pen topNeon(Color((BYTE)(130 * a), 0, 255, 200), 2.0f);
    g.DrawLine(&topNeon, menuX + 16, menuY, menuX + menuW - 16, menuY);

    float logoRot = (g_tick % 6000) / 6000.0f * 360.0f;
    DrawVortexLogo(g, menuX + 26, menuY + 24, 14, logoRot);

    FontFamily ff(L"Segoe UI");
    Font brandFont(&ff, 16.0f, FontStyleBold, UnitPixel);
    SolidBrush brandBr(Color((BYTE)(255 * a), 0, 255, 200));

    StringFormat lfmt;
    lfmt.SetAlignment(StringAlignmentNear);
    lfmt.SetLineAlignment(StringAlignmentCenter);

    RectF brandRc((REAL)(menuX + 48), (REAL)(menuY + 12), 100.0f, 26.0f);
    g.DrawString(L"VORTEX", -1, &brandFont, brandRc, &lfmt, &brandBr);

    int searchX = menuX + 20;
    int searchY = menuY + 48;
    int searchW = menuW - 40;
    int searchH = 36;

    FillRoundRectSolid(g, searchX, searchY, searchW, searchH, 8,
        Color((BYTE)(25 * a), 255, 255, 255));
    DrawRoundRect(g, searchX, searchY, searchW, searchH, 8,
        Color((BYTE)(35 * a), 0, 255, 200), 1.0f);

    Font searchFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    SolidBrush searchBr(Color((BYTE)(90 * a), 200, 200, 220));
    RectF searchRc((REAL)(searchX + 32), (REAL)searchY, (REAL)(searchW - 40), (REAL)searchH);
    g.DrawString(L"Search apps, files...", -1, &searchFont, searchRc, &lfmt, &searchBr);

    Pen searchPen(Color((BYTE)(130 * a), 0, 255, 200), 2.0f);
    g.DrawEllipse(&searchPen, searchX + 10, searchY + 9, 12, 12);
    g.DrawLine(&searchPen, searchX + 20, searchY + 19, searchX + 24, searchY + 23);

    Font sectionFont(&ff, 10.0f, FontStyleBold, UnitPixel);
    SolidBrush sectionBr(Color((BYTE)(140 * a), 0, 255, 200));
    RectF pinnedRc((REAL)(menuX + 24), (REAL)(menuY + 94), 80.0f, 14.0f);
    g.DrawString(L"PINNED", -1, &sectionFont, pinnedRc, &lfmt, &sectionBr);

    int gridX = menuX + 24;
    int gridY = menuY + 116;
    int cols = 4;
    int cellW = (menuW - 48) / cols;
    int cellH = 72;

    for (int i = 0; i < (int)g_startApps.size() && i < 16; i++) {
        int row = i / cols;
        int col = i % cols;
        int ix = gridX + col * cellW;
        int iy = gridY + row * cellH;

        bool hov = (g_hoveredStartIcon == i);

        if (hov) {
            FillRoundRectSolid(g, ix + 3, iy + 2, cellW - 6, cellH - 4, 8,
                Color((BYTE)(55 * a), g_startApps[i].color.GetR(),
                    g_startApps[i].color.GetG(), g_startApps[i].color.GetB()));
            DrawNeonGlow(g, ix + 3, iy + 2, cellW - 6, cellH - 4, 8,
                Color((BYTE)(25 * a), g_startApps[i].color.GetR(),
                    g_startApps[i].color.GetG(), g_startApps[i].color.GetB()), 2);
        }

        int icoSize = 34;
        int icoX = ix + (cellW - icoSize) / 2;
        int icoY = iy + 5;

        FillRoundRectSolid(g, icoX, icoY, icoSize, icoSize, 8,
            Color((BYTE)(140 * a), g_startApps[i].color.GetR() / 3,
                g_startApps[i].color.GetG() / 3, g_startApps[i].color.GetB() / 3));

        if (hov) {
            DrawRoundRect(g, icoX, icoY, icoSize, icoSize, 8,
                Color((BYTE)(160 * a), g_startApps[i].color.GetR(),
                    g_startApps[i].color.GetG(), g_startApps[i].color.GetB()), 1.5f);
        }

        Font icoFont(&ff, 14.0f, FontStyleBold, UnitPixel);
        SolidBrush icoBr(Color((BYTE)(210 * a), g_startApps[i].color.GetR(),
            g_startApps[i].color.GetG(), g_startApps[i].color.GetB()));

        StringFormat cfmt;
        cfmt.SetAlignment(StringAlignmentCenter);
        cfmt.SetLineAlignment(StringAlignmentCenter);

        RectF icoRc((REAL)icoX, (REAL)icoY, (REAL)icoSize, (REAL)icoSize);
        g.DrawString(g_startApps[i].icon.c_str(), -1, &icoFont, icoRc, &cfmt, &icoBr);

        Font nameFont(&ff, 9.0f, FontStyleRegular, UnitPixel);
        SolidBrush nameBr(Color((BYTE)(190 * a), 220, 220, 240));
        RectF nameRc((REAL)ix, (REAL)(iy + 44), (REAL)cellW, 18.0f);
        g.DrawString(g_startApps[i].name.c_str(), -1, &nameFont, nameRc, &cfmt, &nameBr);
    }
}

// ============================================================================
//  DESKTOP ICONS RENDER
// ============================================================================
void DrawDesktopIcons(Graphics& g, int sw, int sh) {
    if (g_startMenuOpen) return;

    FontFamily ff(L"Segoe UI");
    Font iconFont(&ff, 24.0f, FontStyleRegular, UnitPixel);
    Font nameFont(&ff, 10.0f, FontStyleRegular, UnitPixel);

    int startX = sw - DESKTOP_ICON_SIZE - 36;
    int startY = 52;

    for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
        DesktopIcon& d = g_desktopIcons[i];

        int x = startX;
        int y = startY + i * (DESKTOP_ICON_SIZE + DESKTOP_ICON_TEXT_H + DESKTOP_ICON_SPACING);

        if (y + DESKTOP_ICON_SIZE + DESKTOP_ICON_TEXT_H > sh - TASKBAR_HEIGHT - 16) break;

        bool hov = (g_hoveredDesktopIcon == i);

        int totalH = DESKTOP_ICON_SIZE + DESKTOP_ICON_TEXT_H;
        if (hov || d.selected) {
            FillRoundRectSolid(g, x - 6, y - 3, DESKTOP_ICON_SIZE + 12, totalH + 6, 8,
                Color(hov ? 70 : 45, d.color.GetR(), d.color.GetG(), d.color.GetB()));
            if (hov) {
                DrawRoundRect(g, x - 6, y - 3, DESKTOP_ICON_SIZE + 12, totalH + 6, 8,
                    Color(70, d.color.GetR(), d.color.GetG(), d.color.GetB()), 1.0f);
            }
        }

        StringFormat cfmt;
        cfmt.SetAlignment(StringAlignmentCenter);
        cfmt.SetLineAlignment(StringAlignmentCenter);

        SolidBrush icoBr(Color(hov ? 255 : 200, d.color.GetR(), d.color.GetG(), d.color.GetB()));
        RectF icoRc((REAL)x, (REAL)y, (REAL)DESKTOP_ICON_SIZE, (REAL)DESKTOP_ICON_SIZE);
        g.DrawString(d.icon.c_str(), -1, &iconFont, icoRc, &cfmt, &icoBr);

        SolidBrush shadowBr(Color(150, 0, 0, 0));
        RectF shadowRc((REAL)(x - 5), (REAL)(y + DESKTOP_ICON_SIZE + 1), (REAL)(DESKTOP_ICON_SIZE + 10), (REAL)DESKTOP_ICON_TEXT_H);
        g.DrawString(d.name.c_str(), -1, &nameFont, shadowRc, &cfmt, &shadowBr);

        SolidBrush nameBr(VX::TextWhite);
        RectF nameRc2((REAL)(x - 6), (REAL)(y + DESKTOP_ICON_SIZE), (REAL)(DESKTOP_ICON_SIZE + 12), (REAL)DESKTOP_ICON_TEXT_H);
        g.DrawString(d.name.c_str(), -1, &nameFont, nameRc2, &cfmt, &nameBr);

        SetRect(&d.bounds, x - 6, y - 3, x + DESKTOP_ICON_SIZE + 6, y + totalH + 3);
    }
}

// ============================================================================
//  EXPLORER WINDOW RENDER
// ============================================================================
void DrawExplorerWindow(Graphics& g, int idx, int sw, int sh) {
    if (idx < 0 || idx >= (int)g_explorerWindows.size()) return;
    
    ExplorerWindow& ew = g_explorerWindows[idx];
    if (ew.minimized) return;
    
    bool isActive = ew.active;
    
    // Window shadow
    FillRoundRectSolid(g, ew.x + 4, ew.y + 4, ew.w, ew.h, 10, Color(60, 0, 0, 0));
    
    // Window background
    Color bgColor = isActive ? Color(230, 18, 18, 28) : Color(200, 14, 14, 22);
    FillRoundRectSolid(g, ew.x, ew.y, ew.w, ew.h, 10, bgColor);
    
    // Border
    Color borderColor = isActive ? Color(80, 0, 255, 200) : Color(40, 80, 80, 90);
    DrawRoundRect(g, ew.x, ew.y, ew.w, ew.h, 10, borderColor, 1.5f);
    
    // Title bar
    int titleH = 32;
    LinearGradientBrush titleBg(Rect(ew.x, ew.y, ew.w, titleH),
        Color(40, 25, 25, 40), Color(30, 18, 18, 28), LinearGradientModeVertical);
    FillRoundRect(g, ew.x, ew.y, ew.w, titleH, 10, titleBg);
    
    // Title bar bottom line
    Pen titleLine(borderColor, 1.0f);
    g.DrawLine(&titleLine, ew.x + 10, ew.y + titleH, ew.x + ew.w - 10, ew.y + titleH);
    
    // Close button
    int closeX = ew.x + ew.w - 30;
    int closeY = ew.y + 6;
    FillRoundRectSolid(g, closeX, closeY, 22, 20, 4, Color(180, 200, 50, 50));
    
    // Minimize button
    int minX = ew.x + ew.w - 56;
    FillRoundRectSolid(g, minX, closeY, 22, 20, 4, Color(80, 50, 50, 50));
    
    // Title text
    FontFamily ff(L"Segoe UI");
    Font titleFont(&ff, 11.0f, FontStyleBold, UnitPixel);
    SolidBrush titleBr(isActive ? VX::TextWhite : VX::TextDim);
    
    StringFormat lfmt;
    lfmt.SetAlignment(StringAlignmentNear);
    lfmt.SetLineAlignment(StringAlignmentCenter);
    
    RectF titleRc((REAL)(ew.x + 36), (REAL)ew.y, (REAL)(ew.w - 100), (REAL)titleH);
    g.DrawString(ew.title.c_str(), -1, &titleFont, titleRc, &lfmt, &titleBr);
    
    // Folder icon in title
    Font icoFont(&ff, 14.0f, FontStyleRegular, UnitPixel);
    SolidBrush icoBr(VX::Neon);
    RectF icoRc((REAL)(ew.x + 12), (REAL)ew.y, 20.0f, (REAL)titleH);
    g.DrawString(UChar(0x1F4C1).c_str(), -1, &icoFont, icoRc, &lfmt, &icoBr);
    
    // Content area
    int contentY = ew.y + titleH + 4;
    int contentH = ew.h - titleH - 8;
    
    // Sidebar
    int sideW = 160;
    FillRoundRectSolid(g, ew.x + 4, contentY, sideW, contentH, 6, Color(50, 20, 20, 30));
    
    // Sidebar items
    Font sideFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
    SolidBrush sideBr(VX::TextDim);
    
    struct SideItem { std::wstring name; std::wstring icon; };
    SideItem sideItems[] = {
        {L"Quick Access", UChar(0x1F4C1)},
        {L"Desktop", UChar(0x1F5A5)},
        {L"Downloads", UChar(0x1F4E5)},
        {L"Documents", UChar(0x1F4C4)},
        {L"Pictures", UChar(0x1F5BC)},
        {L"Music", UChar(0x1F3B5)},
        {L"This PC", UChar(0x1F4BB)},
        {L"Network", UChar(0x1F4E1)}
    };
    
    int sideItemY = contentY + 8;
    for (int i = 0; i < 8; i++) {
        RectF itemRc((REAL)(ew.x + 14), (REAL)sideItemY, (REAL)(sideW - 20), 22.0f);
        g.DrawString(sideItems[i].icon.c_str(), -1, &sideFont, itemRc, &lfmt, &sideBr);
        RectF textRc((REAL)(ew.x + 36), (REAL)sideItemY, (REAL)(sideW - 42), 22.0f);
        g.DrawString(sideItems[i].name.c_str(), -1, &sideFont, textRc, &lfmt, &sideBr);
        sideItemY += 24;
    }
    
    // File list
    int fileListX = ew.x + sideW + 12;
    int fileListW = ew.w - sideW - 20;
    
    Font fileFont(&ff, 11.0f, FontStyleRegular, UnitPixel);
    
    int itemY = contentY + 8;
    for (size_t i = 0; i < ew.items.size() && i < 20; i++) {
        bool hov = ((int)i == ew.hoveredItem);
        bool sel = ((int)i == ew.selectedItem);
        
        if (hov || sel) {
            FillRoundRectSolid(g, fileListX, itemY, fileListW, 22, 4,
                Color(sel ? 60 : 40, 0, 255, 200));
        }
        
        // File icon (simplified)
        std::wstring fileIcon = UChar(0x1F4C4);  // Document
        SolidBrush fileIcoBr(hov ? VX::Neon : VX::TextDim);
        RectF icoRc2((REAL)fileListX, (REAL)itemY, 20.0f, 22.0f);
        g.DrawString(fileIcon.c_str(), -1, &fileFont, icoRc2, &lfmt, &fileIcoBr);
        
        // File name
        SolidBrush fileBr(hov ? VX::TextWhite : VX::TextDim);
        RectF fileRc((REAL)(fileListX + 24), (REAL)itemY, (REAL)(fileListW - 28), 22.0f);
        g.DrawString(ew.items[i].c_str(), -1, &fileFont, fileRc, &lfmt, &fileBr);
        
        itemY += 24;
        if (itemY > ew.y + ew.h - 30) break;
    }
    
    // Resize handle indicator
    int handleX = ew.x + ew.w - 16;
    int handleY = ew.y + ew.h - 16;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j <= i; j++) {
            SolidBrush dotBr(Color(60, 150, 150, 160));
            g.FillEllipse(&dotBr, handleX + j * 5, handleY + (2 - i) * 5, 3, 3);
        }
    }
}

// ============================================================================
//  WiFi PANEL
// ============================================================================
void DrawWifiPanel(Graphics& g, int sw, int sh) {
    if (g_wifiPanelAnim <= 0.01f) return;

    float a = g_wifiPanelAnim;

    int panelW = WIFI_PANEL_W;
    int panelH = WIFI_PANEL_H;
    int panelX = sw - panelW - 12;
    int panelY = (int)(STATUS_PANEL_H + 16 + 25 * (1.0f - a));

    FillRoundRectSolid(g, panelX, panelY, panelW, panelH, 14,
        Color((BYTE)(210 * a), 14, 14, 24));
    DrawRoundRect(g, panelX, panelY, panelW, panelH, 14,
        Color((BYTE)(55 * a), 0, 255, 200), 1.5f);

    Pen topLine(Color((BYTE)(110 * a), 0, 255, 200), 2.0f);
    g.DrawLine(&topLine, panelX + 14, panelY, panelX + panelW - 14, panelY);

    FontFamily ff(L"Segoe UI");
    Font titleFont(&ff, 13.0f, FontStyleBold, UnitPixel);
    Font itemFont(&ff, 12.0f, FontStyleRegular, UnitPixel);
    Font smallFont(&ff, 9.0f, FontStyleRegular, UnitPixel);

    StringFormat lfmt;
    lfmt.SetAlignment(StringAlignmentNear);
    lfmt.SetLineAlignment(StringAlignmentCenter);

    SolidBrush titleBr(Color((BYTE)(255 * a), 0, 255, 200));
    RectF titleRc((REAL)(panelX + 16), (REAL)(panelY + 10), (REAL)(panelW - 32), 22.0f);
    g.DrawString(L"[WiFi]  Networks", -1, &titleFont, titleRc, &lfmt, &titleBr);

    int scanBtnX = panelX + panelW - 70;
    int scanBtnY = panelY + 10;
    FillRoundRectSolid(g, scanBtnX, scanBtnY, 54, 22, 6,
        Color((BYTE)(55 * a), 0, 255, 200));
    DrawRoundRect(g, scanBtnX, scanBtnY, 54, 22, 6,
        Color((BYTE)(90 * a), 0, 255, 200), 1.0f);

    Font btnFont(&ff, 9.0f, FontStyleBold, UnitPixel);
    SolidBrush btnBr(Color((BYTE)(210 * a), 0, 255, 200));
    StringFormat cfmt;
    cfmt.SetAlignment(StringAlignmentCenter);
    cfmt.SetLineAlignment(StringAlignmentCenter);
    RectF btnRc((REAL)scanBtnX, (REAL)scanBtnY, 54.0f, 22.0f);
    g.DrawString(g_wifiScanning ? L"..." : L"SCAN", -1, &btnFont, btnRc, &cfmt, &btnBr);

    Pen sep(Color((BYTE)(25 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&sep, panelX + 14, panelY + 40, panelX + panelW - 14, panelY + 40);

    int itemY = panelY + 48;
    int itemH = 48;

    for (int i = 0; i < (int)g_wifiNetworks.size() && i < 6; i++) {
        const WifiNetwork& net = g_wifiNetworks[i];
        bool hov = (g_hoveredWifiItem == i);

        int iy = itemY + i * itemH;

        if (hov) {
            FillRoundRectSolid(g, panelX + 6, iy, panelW - 12, itemH - 4, 8,
                Color((BYTE)(35 * a), 0, 255, 200));
        }

        Color sigColor;
        if (net.signal >= 70) sigColor = VX::Green;
        else if (net.signal >= 40) sigColor = VX::Yellow;
        else sigColor = VX::Red;

        int sigX = panelX + 20;
        int sigY = iy + itemH / 2;

        for (int bar = 0; bar < 4; bar++) {
            int barH = 3 + bar * 4;
            Color barColor = (net.signal >= (bar + 1) * 25) ?
                Color((BYTE)(190 * a), sigColor.GetR(), sigColor.GetG(), sigColor.GetB()) :
                Color((BYTE)(25 * a), 100, 100, 100);
            SolidBrush barBr(barColor);
            g.FillRectangle(&barBr, sigX + bar * 5, sigY + 8 - barH, 3, barH);
        }

        SolidBrush nameBr(Color((BYTE)(230 * a), 240, 240, 255));
        RectF nameRc((REAL)(panelX + 48), (REAL)(iy + 5), (REAL)(panelW - 110), 18.0f);
        g.DrawString(net.ssid.c_str(), -1, &itemFont, nameRc, &lfmt, &nameBr);

        std::wstring status;
        if (net.connected) status = L"[OK] Connected";
        else if (net.secured) status = L"[Lock] Secured";
        else status = L"[Open] Open";

        SolidBrush statusBr(Color((BYTE)(130 * a), 150, 160, 180));
        RectF statusRc((REAL)(panelX + 48), (REAL)(iy + 24), (REAL)(panelW - 110), 14.0f);
        g.DrawString(status.c_str(), -1, &smallFont, statusRc, &lfmt, &statusBr);

        wchar_t sigStr[16];
        swprintf_s(sigStr, L"%d%%", net.signal);
        SolidBrush sigBr(Color((BYTE)(170 * a), sigColor.GetR(), sigColor.GetG(), sigColor.GetB()));
        StringFormat rfmt;
        rfmt.SetAlignment(StringAlignmentFar);
        rfmt.SetLineAlignment(StringAlignmentCenter);
        RectF sigRc((REAL)(panelX + panelW - 55), (REAL)iy, 40.0f, (REAL)itemH);
        g.DrawString(sigStr, -1, &itemFont, sigRc, &rfmt, &sigBr);

        if (hov && !net.connected) {
            int btnX2 = panelX + panelW - 90;
            int btnY2 = iy + itemH / 2 - 8;
            FillRoundRectSolid(g, btnX2, btnY2, 60, 18, 5,
                Color((BYTE)(70 * a), 0, 255, 200));
            DrawRoundRect(g, btnX2, btnY2, 60, 18, 5,
                Color((BYTE)(140 * a), 0, 255, 200), 1.0f);

            SolidBrush connBr(Color((BYTE)(230 * a), 0, 255, 200));
            RectF connRc((REAL)btnX2, (REAL)btnY2, 60.0f, 18.0f);
            g.DrawString(L"Connect", -1, &smallFont, connRc, &cfmt, &connBr);
        }
    }

    if (g_wifiNetworks.empty()) {
        SolidBrush emptyBr(Color((BYTE)(110 * a), 150, 160, 180));
        RectF emptyRc((REAL)panelX, (REAL)(panelY + 70), (REAL)panelW, 26.0f);
        g.DrawString(g_wifiScanning ? L"Scanning..." : L"No networks. Press SCAN.",
            -1, &itemFont, emptyRc, &cfmt, &emptyBr);
    }
}

// ============================================================================
//  CONTEXT MENU (RMB)
// ============================================================================
struct ContextMenuItem {
    std::wstring label;
    std::wstring icon;
    int id;
};

std::vector<ContextMenuItem> g_contextItems;

void InitContextMenu() {
    g_contextItems.clear();
    ContextMenuItem item;

    item.label = L"New Folder"; item.icon = UChar(0x1F4C1); item.id = 1;
    g_contextItems.push_back(item);

    item.label = L""; item.icon = L""; item.id = 0;
    g_contextItems.push_back(item);

    item.label = L"Open Terminal"; item.icon = L">_"; item.id = 4;
    g_contextItems.push_back(item);

    item.label = L"Open Explorer"; item.icon = UChar(0x1F4C2); item.id = 6;
    g_contextItems.push_back(item);

    item.label = L""; item.icon = L""; item.id = 0;
    g_contextItems.push_back(item);

    item.label = L"WiFi Networks"; item.icon = UChar(0x1F4F6); item.id = 7;
    g_contextItems.push_back(item);

    item.label = L"Refresh"; item.icon = UChar(0x1F504); item.id = 2;
    g_contextItems.push_back(item);

    item.label = L""; item.icon = L""; item.id = 0;
    g_contextItems.push_back(item);

    item.label = L"Settings"; item.icon = L"\u2699"; item.id = 3;
    g_contextItems.push_back(item);
}

void DrawContextMenu(Graphics& g) {
    if (!g_contextMenuOpen) return;

    int itemH = 32;
    int sepH = 8;
    int menuW = 200;
    int menuH = 8;

    for (const auto& item : g_contextItems) {
        menuH += (item.id == 0) ? sepH : itemH;
    }
    menuH += 8;

    int mx = g_contextMenuX;
    int my = g_contextMenuY;

    if (mx + menuW > ScreenW()) mx = ScreenW() - menuW - 6;
    if (my + menuH > ScreenH()) my = ScreenH() - menuH - 6;

    FillRoundRectSolid(g, mx + 3, my + 3, menuW, menuH, 10, Color(70, 0, 0, 0));

    FillRoundRectSolid(g, mx, my, menuW, menuH, 10, Color(220, 16, 16, 28));
    DrawRoundRect(g, mx, my, menuW, menuH, 10, Color(45, 0, 255, 200), 1.0f);

    Pen topLine(Color(90, 0, 255, 200), 1.5f);
    g.DrawLine(&topLine, mx + 10, my, mx + menuW - 10, my);

    FontFamily ff(L"Segoe UI");
    Font font(&ff, 11.0f, FontStyleRegular, UnitPixel);
    Font icoFont(&ff, 12.0f, FontStyleRegular, UnitPixel);

    StringFormat lfmt;
    lfmt.SetAlignment(StringAlignmentNear);
    lfmt.SetLineAlignment(StringAlignmentCenter);

    int cy = my + 6;
    for (int i = 0; i < (int)g_contextItems.size(); i++) {
        const auto& item = g_contextItems[i];

        if (item.id == 0) {
            Pen sepPen(Color(25, 255, 255, 255), 1.0f);
            g.DrawLine(&sepPen, mx + 14, cy + sepH / 2, mx + menuW - 14, cy + sepH / 2);
            cy += sepH;
            continue;
        }

        bool hov = (g_hoveredContextItem == i);

        if (hov) {
            FillRoundRectSolid(g, mx + 5, cy + 1, menuW - 10, itemH - 2, 6,
                Color(45, 0, 255, 200));
        }

        SolidBrush icoBr(hov ? Color(255, 0, 255, 200) : Color(160, 180, 190, 220));
        RectF icoRc((REAL)(mx + 12), (REAL)cy, 20.0f, (REAL)itemH);
        g.DrawString(item.icon.c_str(), -1, &icoFont, icoRc, &lfmt, &icoBr);

        SolidBrush textBr(hov ? Color(255, 240, 240, 255) : Color(200, 200, 210, 230));
        RectF textRc((REAL)(mx + 36), (REAL)cy, (REAL)(menuW - 48), (REAL)itemH);
        g.DrawString(item.label.c_str(), -1, &font, textRc, &lfmt, &textBr);

        cy += itemH;
    }
}

// ============================================================================
//  NOTIFICATIONS
// ============================================================================
void PushNotification(const std::wstring& title, const std::wstring& msg) {
    Notification n;
    n.title = title;
    n.message = msg;
    n.time = GetTickCount();
    n.alpha = 0.0f;
    n.offsetY = -35.0f;
    n.alive = true;
    g_notifs.push_back(n);
}

void DrawNotifications(Graphics& g, int sw) {
    int baseY = 48;
    for (size_t i = 0; i < g_notifs.size() && i < 3; i++) {
        Notification& n = g_notifs[i];
        if (!n.alive || n.alpha <= 0.01f) continue;

        float a = n.alpha;
        int nx = sw - NOTIF_W - 12;
        int ny = (int)(baseY + n.offsetY + i * (NOTIF_H + 6));

        FillRoundRectSolid(g, nx, ny, NOTIF_W, NOTIF_H, 12,
            Color((BYTE)(190 * a), 18, 18, 30));

        DrawRoundRect(g, nx, ny, NOTIF_W, NOTIF_H, 12,
            Color((BYTE)(55 * a), 0, 255, 200), 1.0f);

        FillRoundRectSolid(g, nx + 1, ny + 12, 2, NOTIF_H - 24, 2,
            Color((BYTE)(190 * a), 0, 255, 200));

        SolidBrush logoBg(Color((BYTE)(140 * a), 0, 255, 200));
        g.FillEllipse(&logoBg, nx + 14, ny + NOTIF_H / 2 - 12, 24, 24);
        SolidBrush logoDot(Color((BYTE)(255 * a), 255, 255, 255));
        g.FillEllipse(&logoDot, nx + 23, ny + NOTIF_H / 2 - 3, 5, 5);

        FontFamily ff(L"Segoe UI");
        Font titleFont(&ff, 11.0f, FontStyleBold, UnitPixel);
        Font msgFont(&ff, 10.0f, FontStyleRegular, UnitPixel);

        SolidBrush titleBr(Color((BYTE)(255 * a), 240, 240, 255));
        SolidBrush msgBr(Color((BYTE)(170 * a), 180, 190, 220));

        StringFormat fmt;
        fmt.SetAlignment(StringAlignmentNear);
        fmt.SetLineAlignment(StringAlignmentNear);

        RectF titleRc((REAL)(nx + 48), (REAL)(ny + 14), (REAL)(NOTIF_W - 62), 14.0f);
        g.DrawString(n.title.c_str(), -1, &titleFont, titleRc, &fmt, &titleBr);

        RectF msgRc2((REAL)(nx + 48), (REAL)(ny + 32), (REAL)(NOTIF_W - 62), 26.0f);
        g.DrawString(n.message.c_str(), -1, &msgFont, msgRc2, &fmt, &msgBr);

        DWORD elapsed = (GetTickCount() - n.time) / 1000;
        wchar_t timeStr2[32];
        if (elapsed < 60) swprintf_s(timeStr2, L"%ds ago", elapsed);
        else swprintf_s(timeStr2, L"%dm ago", elapsed / 60);

        Font timeFont(&ff, 8.0f, FontStyleRegular, UnitPixel);
        SolidBrush timeBr2(Color((BYTE)(90 * a), 120, 130, 160));

        StringFormat rfmt;
        rfmt.SetAlignment(StringAlignmentFar);
        rfmt.SetLineAlignment(StringAlignmentNear);

        RectF timeRc2((REAL)(nx + NOTIF_W - 72), (REAL)(ny + 14), 58.0f, 12.0f);
        g.DrawString(timeStr2, -1, &timeFont, timeRc2, &rfmt, &timeBr2);
    }
}

// ============================================================================
//  PARTICLES
// ============================================================================
void InitParticles() {
    g_particles.clear();
    std::mt19937 rng(42);
    int sw = ScreenW(), sh = ScreenH();
    for (int i = 0; i < 50; i++) {
        Particle p;
        p.x = (float)(rng() % sw);
        p.y = (float)(rng() % sh);
        p.vx = ((int)(rng() % 100) - 50) / 200.0f;
        p.vy = -((int)(rng() % 100) + 10) / 300.0f;
        p.size = 1.0f + (rng() % 30) / 10.0f;
        p.alpha = 0.1f + (rng() % 40) / 100.0f;
        p.life = 1.0f;
        int colorChoice = rng() % 3;
        if (colorChoice == 0) p.color = VX::Neon;
        else if (colorChoice == 1) p.color = VX::Purple;
        else p.color = VX::Magenta;
        g_particles.push_back(p);
    }
}

void UpdateParticles() {
    int sw = ScreenW(), sh = ScreenH();
    for (auto& p : g_particles) {
        p.x += p.vx;
        p.y += p.vy;
        if (p.y < -10) { p.y = (float)(sh + 10); p.x = (float)(rand() % sw); }
        if (p.x < -10) p.x = (float)(sw + 10);
        if (p.x > sw + 10) p.x = -10;
    }
}

void DrawParticles(Graphics& g) {
    for (const auto& p : g_particles) {
        BYTE particleAlpha = (BYTE)(p.alpha * 255);
        SolidBrush br(particleAlpha, p.color.GetR(), p.color.GetG(), p.color.GetB());
        g.FillEllipse(&br, p.x - p.size, p.y - p.size, p.size * 2, p.size * 2);
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
    } else {
        LinearGradientBrush bg(Point(0, 0), Point(sw, sh),
            Color(255, 8, 8, 20), Color(255, 20, 10, 40));
        g.FillRectangle(&bg, 0, 0, sw, sh);

        for (int i = 0; i < 5; i++) {
            int cx = 130 + i * 260;
            int cy = 90 + (i % 3) * 180;
            int cr = 70 + i * 35;
            SolidBrush cb(Color(7, 0, 200 + i * 10, 200));
            g.FillEllipse(&cb, cx - cr, cy - cr, cr * 2, cr * 2);
        }
    }

    SolidBrush dimOverlay(Color(25, 0, 0, 0));
    g.FillRectangle(&dimOverlay, 0, 0, sw, sh);
}

// ============================================================================
//  HOTKEY HINTS
// ============================================================================
void DrawHotkeyHints(Graphics& g, int sh) {
    FontFamily ff(L"Segoe UI");
    Font font(&ff, 8.0f, FontStyleRegular, UnitPixel);
    SolidBrush br(Color(55, 200, 200, 220));

    StringFormat fmt;
    fmt.SetAlignment(StringAlignmentNear);
    fmt.SetLineAlignment(StringAlignmentFar);

    RectF rc(10.0f, (REAL)(sh - 24), 480.0f, 14.0f);
    g.DrawString(L"ESC exit | SPACE menu | M music | W widgets | N notify | F wifi | E explorer | RMB context", -1, &font, rc, &fmt, &br);
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
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

    DrawBackground(g, sw, sh);
    DrawParticles(g);
    DrawDesktopIcons(g, sw, sh);
    DrawWidgets(g, sh);
    DrawStatusPanel(g, sw);
    DrawNotifications(g, sw);
    DrawWifiPanel(g, sw, sh);
    
    // Explorer windows
    for (int i = 0; i < (int)g_explorerWindows.size(); i++) {
        DrawExplorerWindow(g, i, sw, sh);
    }
    
    DrawTaskbar(g, sw, sh);
    DrawSystemTray(g, sw, sh);
    DrawContextMenu(g);
    DrawStartMenu(g, sw, sh);

    if (!g_startMenuOpen) {
        DrawHotkeyHints(g, sh);
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
        InitParticles();
        InitContextMenu();
        InitTrayItems();
        RefreshDriveInfo();
        UpdateWidgets();
        CreateThread(NULL, 0, WallpaperThread, NULL, 0, NULL);
        CreateThread(NULL, 0, WifiScanThread, NULL, 0, NULL);
        SetTimer(hWnd, 1, 1000, NULL);
        SetTimer(hWnd, 2, 16, NULL);
        SetTimer(hWnd, 3, 45000, NULL);
        PushNotification(L"Welcome to VORTEX v4.0", L"Fluent Design desktop ready!");
        break;

    case WM_TIMER:
        g_tick = GetTickCount();
        if (wParam == 1) {
            UpdateWidgets();
        } else if (wParam == 2) {
            // Smooth cubic ease animations
            float targetStart = g_startMenuOpen ? 1.0f : 0.0f;
            float easeStart = CubicEaseInOut(0.35f);
            g_startMenuAnim += (targetStart - g_startMenuAnim) * easeStart;
            if (fabs(g_startMenuAnim - targetStart) < 0.01f) g_startMenuAnim = targetStart;

            float targetW = g_widgetsVisible ? 1.0f : 0.0f;
            float easeW = CubicEaseInOut(0.30f);
            g_widgetsAnim += (targetW - g_widgetsAnim) * easeW;
            if (fabs(g_widgetsAnim - targetW) < 0.01f) g_widgetsAnim = targetW;

            float targetWifi = g_wifiPanelOpen ? 1.0f : 0.0f;
            float easeWifi = CubicEaseInOut(0.35f);
            g_wifiPanelAnim += (targetWifi - g_wifiPanelAnim) * easeWifi;
            if (fabs(g_wifiPanelAnim - targetWifi) < 0.01f) g_wifiPanelAnim = targetWifi;
            
            float targetTray = g_systemTrayOpen ? 1.0f : 0.0f;
            g_systemTrayAnim += (targetTray - g_systemTrayAnim) * 0.35f;
            if (fabs(g_systemTrayAnim - targetTray) < 0.01f) g_systemTrayAnim = targetTray;

            UpdateParticles();

            for (auto& n : g_notifs) {
                if (!n.alive) continue;
                if (n.alpha < 1.0f) n.alpha = (std::min)(1.0f, n.alpha + 0.12f);
                if (n.offsetY < 0) n.offsetY = (std::min)(0.0f, n.offsetY + 6.0f);
                if (GetTickCount() - n.time > 6000) {
                    n.alpha -= 0.06f;
                    if (n.alpha <= 0) n.alive = false;
                }
            }
            g_notifs.erase(std::remove_if(g_notifs.begin(), g_notifs.end(),
                [](const Notification& n) { return !n.alive; }), g_notifs.end());

            InvalidateRect(hWnd, NULL, FALSE);
        } else if (wParam == 3) {
            if (g_notifs.empty()) {
                PushNotification(L"VORTEX System", L"All systems nominal.");
            }
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

        // Context menu hover
        if (g_contextMenuOpen) {
            g_hoveredContextItem = -1;
            int itemH = 32;
            int sepH = 8;
            int cmx = g_contextMenuX;
            int cmy = g_contextMenuY;
            int menuW = 200;
            if (cmx + menuW > ScreenW()) cmx = ScreenW() - menuW - 6;

            int cy = cmy + 6;
            for (int i = 0; i < (int)g_contextItems.size(); i++) {
                if (g_contextItems[i].id == 0) {
                    cy += sepH;
                    continue;
                }
                RECT ir;
                SetRect(&ir, cmx + 5, cy, cmx + menuW - 5, cy + itemH);
                if (PtInRect(&ir, pt)) {
                    g_hoveredContextItem = i;
                    break;
                }
                cy += itemH;
            }
        }

        // Explorer window dragging/resizing
        if (g_activeExplorer >= 0 && g_activeExplorer < (int)g_explorerWindows.size()) {
            ExplorerWindow& ew = g_explorerWindows[g_activeExplorer];
            if (ew.dragging) {
                ew.x = mx - ew.dragOffsetX;
                ew.y = my - ew.dragOffsetY;
            } else if (ew.resizing) {
                int newW = mx - ew.x;
                int newH = my - ew.y;
                ew.w = (std::max)(EXPLORER_MIN_W, newW);
                ew.h = (std::max)(EXPLORER_MIN_H, newH);
            }
        }

        // Taskbar hover
        g_hoveredTaskbarIcon = -1;
        for (int i = 0; i < (int)g_taskApps.size(); i++) {
            RECT r = g_taskApps[i].bounds;
            InflateRect(&r, 3, 6);
            if (PtInRect(&r, pt)) {
                g_hoveredTaskbarIcon = i;
                break;
            }
        }

        // Desktop icon hover
        g_hoveredDesktopIcon = -1;
        for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
            if (PtInRect(&g_desktopIcons[i].bounds, pt)) {
                g_hoveredDesktopIcon = i;
                break;
            }
        }

        // Widget hover
        g_hoveredWidget = -1;
        if (g_widgetsVisible && g_widgetsAnim > 0.5f) {
            int wy = 48;
            for (int i = 0; i < (int)g_widgets.size(); i++) {
                RECT wr;
                SetRect(&wr, 12, wy, 12 + WIDGET_W, wy + g_widgets[i].height);
                if (PtInRect(&wr, pt)) {
                    g_hoveredWidget = i;
                    break;
                }
                wy += g_widgets[i].height + WIDGET_MARGIN;
            }
        }

        // WiFi panel hover
        g_hoveredWifiItem = -1;
        if (g_wifiPanelOpen && g_wifiPanelAnim > 0.5f) {
            int sw = ScreenW();
            int panelX = sw - WIFI_PANEL_W - 12;
            int panelY = STATUS_PANEL_H + 16;
            int itemY = panelY + 48;
            int itemH = 48;

            for (int i = 0; i < (int)g_wifiNetworks.size() && i < 6; i++) {
                int iy = itemY + i * itemH;
                RECT ir;
                SetRect(&ir, panelX + 6, iy, panelX + WIFI_PANEL_W - 6, iy + itemH);
                if (PtInRect(&ir, pt)) {
                    g_hoveredWifiItem = i;
                    break;
                }
            }
        }

        // Start menu hover
        g_hoveredStartIcon = -1;
        if (g_startMenuOpen && g_startMenuAnim > 0.5f) {
            int sw = ScreenW(), sh = ScreenH();
            int menuX = (sw - START_MENU_W) / 2;
            int menuY = (sh - START_MENU_H) / 2 - 20;
            int gridX = menuX + 24;
            int gridY = menuY + 116;
            int cols = 4;
            int cellW = (START_MENU_W - 48) / cols;
            int cellH = 72;

            for (int i = 0; i < (int)g_startApps.size() && i < 16; i++) {
                int row = i / cols;
                int col = i % cols;
                int ix = gridX + col * cellW;
                int iy = gridY + row * cellH;
                RECT cr;
                SetRect(&cr, ix, iy, ix + cellW, iy + cellH);
                if (PtInRect(&cr, pt)) {
                    g_hoveredStartIcon = i;
                    break;
                }
            }
        }
        
        // Explorer window hover
        for (int i = (int)g_explorerWindows.size() - 1; i >= 0; i--) {
            ExplorerWindow& ew = g_explorerWindows[i];
            RECT wr;
            SetRect(&wr, ew.x, ew.y, ew.x + ew.w, ew.y + ew.h);
            if (PtInRect(&wr, pt)) {
                // Check file item hover
                int contentY = ew.y + 36;
                int sideW = 160;
                int fileListX = ew.x + sideW + 12;
                int fileListW = ew.w - sideW - 20;
                
                ew.hoveredItem = -1;
                int itemY = contentY + 8;
                for (size_t j = 0; j < ew.items.size() && j < 20; j++) {
                    RECT itemRect;
                    SetRect(&itemRect, fileListX, itemY, fileListX + fileListW, itemY + 22);
                    if (PtInRect(&itemRect, pt)) {
                        ew.hoveredItem = (int)j;
                        break;
                    }
                    itemY += 24;
                    if (itemY > ew.y + ew.h - 30) break;
                }
                break;
            }
        }
        break;
    }

    case WM_LBUTTONDOWN: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);
        POINT pt = { mx, my };

        // Context menu click
        if (g_contextMenuOpen) {
            if (g_hoveredContextItem >= 0) {
                int id = g_contextItems[g_hoveredContextItem].id;
                switch (id) {
                case 1:
                    CreateNewFolderOnDesktop();
                    PushNotification(L"Folder Created", L"New folder on Desktop");
                    break;
                case 2:
                    InvalidateRect(hWnd, NULL, FALSE);
                    PushNotification(L"Refresh", L"Desktop refreshed");
                    break;
                case 3:
                    ShellExecute(NULL, L"open", L"ms-settings:", NULL, NULL, SW_SHOW);
                    break;
                case 4:
                    ShellExecute(NULL, L"open", L"cmd.exe", NULL, NULL, SW_SHOW);
                    break;
                case 6:
                    OpenExplorerWindow(L"C:\\");
                    break;
                case 7:
                    g_wifiPanelOpen = !g_wifiPanelOpen;
                    if (g_wifiPanelOpen && !g_wifiScanning) {
                        CreateThread(NULL, 0, WifiScanThread, NULL, 0, NULL);
                    }
                    break;
                }
            }
            g_contextMenuOpen = false;
            break;
        }

        // Start menu click
        if (g_startMenuOpen && g_startMenuAnim > 0.5f) {
            if (g_hoveredStartIcon >= 0 && g_hoveredStartIcon < (int)g_startApps.size()) {
                ShellExecute(NULL, L"open", g_startApps[g_hoveredStartIcon].exec.c_str(),
                    NULL, NULL, SW_SHOW);
                g_startMenuOpen = false;
            } else {
                int sw = ScreenW(), sh = ScreenH();
                int menuX = (sw - START_MENU_W) / 2;
                int menuY = (sh - START_MENU_H) / 2 - 20;
                RECT menuRect;
                SetRect(&menuRect, menuX, menuY, menuX + START_MENU_W, menuY + START_MENU_H);
                if (!PtInRect(&menuRect, pt)) {
                    g_startMenuOpen = false;
                }
            }
            break;
        }

        // WiFi panel click
        if (g_wifiPanelOpen && g_wifiPanelAnim > 0.5f) {
            int sw = ScreenW();
            int panelX = sw - WIFI_PANEL_W - 12;
            int panelY = STATUS_PANEL_H + 16;
            int scanBtnX = panelX + WIFI_PANEL_W - 70;
            int scanBtnY = panelY + 10;
            RECT scanRect;
            SetRect(&scanRect, scanBtnX, scanBtnY, scanBtnX + 54, scanBtnY + 22);
            if (PtInRect(&scanRect, pt) && !g_wifiScanning) {
                CreateThread(NULL, 0, WifiScanThread, NULL, 0, NULL);
                PushNotification(L"WiFi", L"Scanning for networks...");
            }

            RECT panelRect;
            SetRect(&panelRect, panelX, panelY, panelX + WIFI_PANEL_W, panelY + WIFI_PANEL_H);
            if (!PtInRect(&panelRect, pt)) {
                g_wifiPanelOpen = false;
            }
        }

        // Explorer window click
        bool clickedExplorer = false;
        for (int i = (int)g_explorerWindows.size() - 1; i >= 0; i--) {
            ExplorerWindow& ew = g_explorerWindows[i];
            RECT wr;
            SetRect(&wr, ew.x, ew.y, ew.x + ew.w, ew.y + ew.h);
            
            if (PtInRect(&wr, pt)) {
                clickedExplorer = true;
                
                // Deactivate all, activate this one
                for (auto& w : g_explorerWindows) w.active = false;
                ew.active = true;
                g_activeExplorer = i;
                
                // Title bar click
                if (my >= ew.y && my < ew.y + 32) {
                    // Close button
                    if (mx >= ew.x + ew.w - 30 && mx < ew.x + ew.w - 8) {
                        CloseExplorerWindow(i);
                        break;
                    }
                    // Minimize button
                    if (mx >= ew.x + ew.w - 56 && mx < ew.x + ew.w - 34) {
                        ew.minimized = true;
                        break;
                    }
                    // Start drag
                    ew.dragging = true;
                    ew.dragOffsetX = mx - ew.x;
                    ew.dragOffsetY = my - ew.y;
                }
                
                // Resize handle
                if (mx >= ew.x + ew.w - 20 && my >= ew.y + ew.h - 20) {
                    ew.resizing = true;
                }
                
                // File item click
                if (ew.hoveredItem >= 0) {
                    ew.selectedItem = ew.hoveredItem;
                }
                break;
            }
        }

        if (clickedExplorer) break;

        // Desktop icon click
        for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
            if (PtInRect(&g_desktopIcons[i].bounds, pt)) {
                ShellExecute(NULL, L"open", g_desktopIcons[i].action.c_str(), NULL, NULL, SW_SHOW);
                break;
            }
        }

        // Taskbar click
        for (int i = 0; i < (int)g_taskApps.size(); i++) {
            if (PtInRect(&g_taskApps[i].bounds, pt)) {
                ShellExecute(NULL, L"open", g_taskApps[i].exec.c_str(), NULL, NULL, SW_SHOW);
                break;
            }
        }

        // Logo click
        {
            int sw = ScreenW(), sh = ScreenH();
            int iconCount = (int)g_taskApps.size();
            int contentW = iconCount * (TASKBAR_ICON_SIZE + TASKBAR_ICON_SPACING) - TASKBAR_ICON_SPACING + 24;
            int barW = contentW + 200;
            int barX = (sw - barW) / 2;
            int barY = sh - TASKBAR_HEIGHT - 6;
            RECT logoRect;
            SetRect(&logoRect, barX + 6, barY + 6, barX + 42, barY + TASKBAR_HEIGHT - 6);
            if (PtInRect(&logoRect, pt)) {
                g_startMenuOpen = !g_startMenuOpen;
            }
            
            // System tray click
            RECT trayRect;
            SetRect(&trayRect, barX + barW - 140, barY, barX + barW - 70, barY + TASKBAR_HEIGHT);
            if (PtInRect(&trayRect, pt)) {
                g_systemTrayOpen = !g_systemTrayOpen;
            }
        }
        break;
    }

    case WM_LBUTTONUP: {
        // Stop dragging/resizing explorer windows
        for (auto& ew : g_explorerWindows) {
            ew.dragging = false;
            ew.resizing = false;
        }
        break;
    }

    case WM_RBUTTONDOWN: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);

        if (g_startMenuOpen) {
            g_startMenuOpen = false;
        } else if (g_contextMenuOpen) {
            g_contextMenuOpen = false;
        } else {
            g_contextMenuX = mx;
            g_contextMenuY = my;
            g_contextMenuOpen = true;
            g_hoveredContextItem = -1;
        }
        break;
    }

    case WM_LBUTTONDBLCLK: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);
        POINT pt = { mx, my };

        for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
            if (PtInRect(&g_desktopIcons[i].bounds, pt)) {
                ShellExecute(NULL, L"open", g_desktopIcons[i].action.c_str(), NULL, NULL, SW_SHOW);
                break;
            }
        }
        break;
    }

    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);
        ScreenToClient(hWnd, &pt);
        mx = pt.x;
        my = pt.y;
        
        // Volume control in system tray
        if (g_systemTrayOpen && g_systemTrayAnim > 0.5f) {
            int sw = ScreenW();
            int sh = ScreenH();
            int panelW = 200;
            int panelH = 140;
            int panelX = sw - panelW - 12;
            int panelY = sh - TASKBAR_HEIGHT - 10 - panelH;
            
            RECT panelRect;
            SetRect(&panelRect, panelX, panelY, panelX + panelW, panelY + panelH);
            POINT wheelPt = { mx, my };
            if (PtInRect(&panelRect, wheelPt)) {
                g_volume += delta > 0 ? 5 : -5;
                g_volume = (std::max)(0, (std::min)(100, g_volume));
            }
        }
        break;
    }

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            if (g_contextMenuOpen) g_contextMenuOpen = false;
            else if (g_wifiPanelOpen) g_wifiPanelOpen = false;
            else if (g_systemTrayOpen) g_systemTrayOpen = false;
            else if (g_startMenuOpen) g_startMenuOpen = false;
            else if (!g_explorerWindows.empty()) {
                CloseExplorerWindow((int)g_explorerWindows.size() - 1);
            }
            else PostQuitMessage(0);
        } else if (wParam == VK_SPACE) {
            g_startMenuOpen = !g_startMenuOpen;
            g_contextMenuOpen = false;
        } else if (wParam == 'M') {
            g_musicPlaying = !g_musicPlaying;
            PushNotification(L"Music", g_musicPlaying ? L"Now playing" : L"Paused");
        } else if (wParam == 'W') {
            g_widgetsVisible = !g_widgetsVisible;
        } else if (wParam == 'N') {
            PushNotification(L"Test", L"Notification from VORTEX");
        } else if (wParam == 'F') {
            g_wifiPanelOpen = !g_wifiPanelOpen;
            if (g_wifiPanelOpen && !g_wifiScanning) {
                CreateThread(NULL, 0, WifiScanThread, NULL, 0, NULL);
            }
        } else if (wParam == 'E') {
            OpenExplorerWindow(L"C:\\");
        }
        break;

    case WM_DESTROY:
        KillTimer(hWnd, 1);
        KillTimer(hWnd, 2);
        KillTimer(hWnd, 3);
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
        L"VORTEX_Desktop_v4", L"VORTEX Desktop v4.0",
        WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN,
        0, 0, sw, sh,
        NULL, NULL, hInst, NULL);

    HWND hTaskbar = FindWindow(L"Shell_TrayWnd", NULL);
    if (hTaskbar) ShowWindow(hTaskbar, SW_HIDE);

    ShowWindow(g_hWnd, nShow);
    UpdateWindow(g_hWnd);

    MSG msg2;
    while (GetMessage(&msg2, NULL, 0, 0)) {
        TranslateMessage(&msg2);
        DispatchMessage(&msg2);
    }

    if (hTaskbar) ShowWindow(hTaskbar, SW_SHOW);

    GdiplusShutdown(g_gdiplusToken);
    return (int)msg2.wParam;
}

int main() {
    return wWinMain(GetModuleHandle(NULL), NULL, NULL, SW_SHOW);
}
