/*
 * VORTEX Desktop Environment v2.1 - Cyberpunk/Futuristic Design
 *
 * ОБНОВЛЕНИЯ v2.1:
 * - Убрана замедленность анимаций (мгновенный отклик)
 * - Добавлена Корзина на рабочий стол
 * - Добавлен "Этот компьютер" на рабочий стол
 * - Возможность создавать папки (ПКМ -> Создать папку)
 * - Просмотр WiFi сетей (панель WiFi)
 *
 * ESC       - выход / закрыть меню
 * SPACE     - открыть/закрыть Start Menu
 * M         - вкл/выкл музыку
 * W         - показать/скрыть виджеты
 * N         - новое уведомление
 * F         - показать/скрыть WiFi панель
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
//  ЦВЕТОВАЯ ПАЛИТРА VORTEX - CYBERPUNK NEON
// ============================================================================
namespace VX {
    // Основные акценты
    const Color Neon(255, 0, 255, 200);
    const Color NeonBright(255, 0, 255, 240);
    const Color NeonDim(255, 0, 180, 160);
    const Color Purple(255, 139, 92, 246);
    const Color PurpleBright(255, 180, 140, 255);
    const Color PurpleDim(255, 90, 50, 180);
    const Color Magenta(255, 255, 0, 128);
    const Color MagentaDim(255, 180, 0, 90);

    // Фоны
    const Color PanelBg(200, 12, 12, 20);
    const Color PanelBgLight(180, 20, 20, 35);
    const Color CardBg(150, 18, 18, 30);
    const Color CardBgHover(170, 30, 30, 50);

    // Текст
    const Color TextWhite(255, 240, 240, 255);
    const Color TextDim(180, 180, 190, 220);
    const Color TextMuted(120, 130, 140, 170);

    // Статусы
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
//  КОНСТАНТЫ И НАСТРОЙКИ
// ============================================================================
const wchar_t* WALLPAPER_URL = L"https://images.wallpaperscraft.com/image/single/lake_mountains_trees_1219008_1920x1080.jpg";
const wchar_t* WALLPAPER_CACHE = L"vortex_wallpaper_cache.jpg";

const int TASKBAR_HEIGHT = 58;
const int TASKBAR_ICON_SIZE = 40;
const int TASKBAR_ICON_SPACING = 6;
const int TASKBAR_RADIUS = 16;

const int STATUS_PANEL_H = 36;
const int STATUS_PANEL_RADIUS = 12;

const int WIDGET_W = 300;
const int WIDGET_MARGIN = 16;

const int START_MENU_W = 580;
const int START_MENU_H = 520;

const int NOTIF_W = 340;
const int NOTIF_H = 80;

const int DESKTOP_ICON_SIZE = 70;
const int DESKTOP_ICON_SPACING = 16;
const int DESKTOP_ICON_TEXT_H = 28;

const int WIFI_PANEL_W = 340;
const int WIFI_PANEL_H = 400;

// ============================================================================
//  ГЛОБАЛЬНОЕ СОСТОЯНИЕ
// ============================================================================
HWND g_hWnd = NULL;
ULONG_PTR g_gdiplusToken = 0;
DWORD g_tick = 0;

// Двойная буферизация
HDC g_memDC = NULL;
HBITMAP g_memBmp = NULL, g_oldBmp = NULL;
void* g_bits = NULL;
int g_bufW = 0, g_bufH = 0;

// Обои
Bitmap* g_wallpaper = NULL;
bool g_wallpaperReady = false;
bool g_wallpaperLoading = false;

// UI состояние
bool g_startMenuOpen = false;
bool g_widgetsVisible = true;
bool g_musicPlaying = false;
bool g_wifiPanelOpen = false;
bool g_contextMenuOpen = false;
int g_hoveredTaskbarIcon = -1;
int g_hoveredStartIcon = -1;
int g_hoveredWidget = -1;
int g_hoveredDesktopIcon = -1;
int g_hoveredWifiItem = -1;
int g_contextMenuX = 0, g_contextMenuY = 0;
int g_hoveredContextItem = -1;

// Анимации - БЫСТРЫЕ
float g_startMenuAnim = 0.0f;
float g_widgetsAnim = 1.0f;
float g_wifiPanelAnim = 0.0f;
float g_taskbarIconScale[16] = {};

// Частицы фона
struct Particle {
    float x, y, vx, vy, size, alpha, life;
    Color color;
};
std::vector<Particle> g_particles;

// Уведомления
struct Notification {
    std::wstring title, message;
    DWORD time;
    float alpha, offsetY;
    bool alive;
};
std::vector<Notification> g_notifs;

// ============================================================================
//  ИКОНКИ РАБОЧЕГО СТОЛА
// ============================================================================
struct DesktopIcon {
    std::wstring name;
    const wchar_t* icon;
    std::wstring action; // shell command или специальная команда
    Color color;
    RECT bounds;
    bool selected;
};
std::vector<DesktopIcon> g_desktopIcons;

void InitDesktopIcons() {
    g_desktopIcons.clear();
    auto add = [](const wchar_t* name, const wchar_t* ico, const wchar_t* act, Color c) {
        DesktopIcon d;
        d.name = name; d.icon = ico; d.action = act; d.color = c;
        d.selected = false;
        memset(&d.bounds, 0, sizeof(RECT));
        g_desktopIcons.push_back(d);
    };

    add(L"This PC",       L"\xD83D\xDCBB", L"shell:::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", VX::Neon);
    add(L"Recycle Bin",   L"\xD83D\xDDD1", L"shell:RecycleBinFolder", VX::Purple);
    add(L"Documents",     L"\xD83D\xDCC4", L"shell:Personal", VX::Yellow);
    add(L"Downloads",     L"\xD83D\xDCE5", L"shell:Downloads", VX::Green);
    add(L"Terminal",      L">_",           L"cmd.exe", VX::Green);
    add(L"Browser",       L"\xD83C\xDF10", L"msedge.exe", Color(255, 0, 150, 255));
}

// ============================================================================
//  WiFi СЕТИ
// ============================================================================
struct WifiNetwork {
    std::wstring ssid;
    int signal;          // 0-100%
    bool secured;
    bool connected;
};
std::vector<WifiNetwork> g_wifiNetworks;
bool g_wifiScanning = false;
DWORD g_lastWifiScan = 0;

DWORD WINAPI WifiScanThread(LPVOID) {
    g_wifiScanning = true;
    g_wifiNetworks.clear();

    // Запускаем netsh wlan show networks mode=bssid
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

        // Парсим вывод
        std::istringstream stream(output);
        std::string line;
        WifiNetwork current;
        current.signal = 0;
        current.secured = false;
        current.connected = false;
        bool hasNetwork = false;

        while (std::getline(stream, line)) {
            // SSID
            if (line.find("SSID") != std::string::npos && line.find("BSSID") == std::string::npos) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    if (hasNetwork && !current.ssid.empty()) {
                        g_wifiNetworks.push_back(current);
                    }
                    std::string ssid = line.substr(pos + 2);
                    // Trim
                    while (!ssid.empty() && (ssid.back() == '\r' || ssid.back() == '\n' || ssid.back() == ' '))
                        ssid.pop_back();
                    current.ssid = std::wstring(ssid.begin(), ssid.end());
                    current.signal = 0;
                    current.secured = false;
                    current.connected = false;
                    hasNetwork = true;
                }
            }
            // Signal
            if (line.find("Signal") != std::string::npos || line.find("%") != std::string::npos) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    std::string val = line.substr(pos + 1);
                    // Извлекаем число
                    int sig = 0;
                    for (char c : val) {
                        if (c >= '0' && c <= '9') sig = sig * 10 + (c - '0');
                    }
                    if (sig > 0 && sig <= 100) current.signal = sig;
                }
            }
            // Authentication / Security
            if (line.find("Authentication") != std::string::npos || line.find("Auth") != std::string::npos) {
                if (line.find("Open") == std::string::npos && line.find("open") == std::string::npos) {
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

    // Если не нашли сетей, добавим заглушки для демонстрации
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
        // Помечаем первую как подключённую (упрощение)
        if (!g_wifiNetworks.empty()) g_wifiNetworks[0].connected = true;
    }

    // Сортируем по сигналу
    std::sort(g_wifiNetworks.begin(), g_wifiNetworks.end(),
        [](const WifiNetwork& a, const WifiNetwork& b) { return a.signal > b.signal; });

    g_wifiScanning = false;
    g_lastWifiScan = GetTickCount();
    if (g_hWnd) InvalidateRect(g_hWnd, NULL, FALSE);
    return 0;
}

// ============================================================================
//  СОЗДАНИЕ ПАПКИ
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
            // Добавляем иконку на рабочий стол
            DesktopIcon d;
            // Извлекаем имя
            size_t pos = folderName.find_last_of(L'\\');
            d.name = (pos != std::wstring::npos) ? folderName.substr(pos + 1) : folderName;
            d.icon = L"\xD83D\xDCC1";
            d.action = L"explorer.exe \"" + folderName + L"\"";
            d.color = VX::Orange;
            d.selected = false;
            memset(&d.bounds, 0, sizeof(RECT));
            g_desktopIcons.push_back(d);
        }
    }
}

// ============================================================================
//  УТИЛИТЫ GDI+
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
        Color gc(c.GetA() / (BYTE)(i + 1), c.GetR(), c.GetG(), c.GetB());
        DrawRoundRect(g, x - spread, y - spread, w + spread * 2, h + spread * 2, r + spread, gc, 2.0f);
    }
}

// ============================================================================
//  ЛОГОТИП VORTEX
// ============================================================================
void DrawVortexLogo(Graphics& g, int cx, int cy, int radius, float rotation = 0.0f) {
    for (int i = 4; i >= 1; i--) {
        SolidBrush glow(Color(15 * i, 0, 255, 200));
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
//  ИКОНКИ ПАНЕЛИ ЗАДАЧ
// ============================================================================
struct TaskbarApp {
    std::wstring name;
    std::wstring exec;
    Color accentColor;
    const wchar_t* icon;
    bool running;
    RECT bounds;
};

std::vector<TaskbarApp> g_taskApps;

void InitTaskbarApps() {
    g_taskApps.clear();
    auto add = [](const wchar_t* name, const wchar_t* exec, Color c, const wchar_t* ico, bool run) {
        TaskbarApp a;
        a.name = name; a.exec = exec; a.accentColor = c;
        a.icon = ico; a.running = run;
        memset(&a.bounds, 0, sizeof(RECT));
        g_taskApps.push_back(a);
    };

    add(L"Files",    L"explorer.exe",    VX::Neon,    L"\x1F4C1", true);
    add(L"Browser",  L"msedge.exe",      Color(255, 0, 150, 255), L"\x1F310", true);
    add(L"Terminal", L"cmd.exe",          VX::Green,   L">_",      false);
    add(L"Code",     L"notepad.exe",     Color(255, 0, 122, 204), L"</>",    true);
    add(L"Music",    L"wmplayer.exe",    VX::Magenta, L"\x266B",  false);
    add(L"Photos",   L"mspaint.exe",     VX::Orange,  L"\x1F5BC", false);
    add(L"Notes",    L"notepad.exe",     VX::Yellow,  L"\x1F4DD", false);
    add(L"Settings", L"ms-settings:",    Color(255, 142, 142, 160), L"\x2699", false);
}

void DrawTaskbarIcon(Graphics& g, int x, int y, int size, const TaskbarApp& app,
                     bool hovered, float scale) {
    int s = (int)(size * scale);
    int ox = x + (size - s) / 2;
    int oy = y + (size - s) / 2;

    Color bgColor = hovered ?
        Color(200, app.accentColor.GetR() / 3, app.accentColor.GetG() / 3, app.accentColor.GetB() / 3) :
        Color(80, 30, 30, 45);
    FillRoundRectSolid(g, ox, oy, s, s, 10, bgColor);

    if (hovered) {
        DrawNeonGlow(g, ox, oy, s, s, 10, app.accentColor, 3);
        DrawRoundRect(g, ox, oy, s, s, 10, Color(200, app.accentColor.GetR(),
            app.accentColor.GetG(), app.accentColor.GetB()), 1.5f);
    } else {
        DrawRoundRect(g, ox, oy, s, s, 10, Color(40, 255, 255, 255), 0.5f);
    }

    FontFamily ff(L"Segoe UI");
    Font iconFont(&ff, s * 0.38f, FontStyleBold, UnitPixel);
    Color textColor = hovered ? Color(255, 255, 255, 255) : VX::TextDim;
    SolidBrush textBr(textColor);

    StringFormat fmt;
    fmt.SetAlignment(StringAlignmentCenter);
    fmt.SetLineAlignment(StringAlignmentCenter);

    RectF rc((REAL)ox, (REAL)oy, (REAL)s, (REAL)s);
    g.DrawString(app.icon, -1, &iconFont, rc, &fmt, &textBr);
}

// ============================================================================
//  ПАНЕЛЬ ЗАДАЧ
// ============================================================================
void DrawTaskbar(Graphics& g, int sw, int sh) {
    int iconCount = (int)g_taskApps.size();
    int contentW = iconCount * (TASKBAR_ICON_SIZE + TASKBAR_ICON_SPACING) - TASKBAR_ICON_SPACING + 24;

    int barW = contentW + 120;
    int barH = TASKBAR_HEIGHT;
    int barX = (sw - barW) / 2;
    int barY = sh - barH - 8;

    GraphicsPath barPath;
    RoundedRectPath(barPath, barX, barY, barW, barH, TASKBAR_RADIUS);

    LinearGradientBrush barBg(Rect(barX, barY, barW, barH),
        Color(180, 15, 15, 28), Color(160, 8, 8, 18), LinearGradientModeVertical);
    g.FillPath(&barBg, &barPath);

    Pen topLine(Color(120, 0, 255, 200), 1.0f);
    g.DrawLine(&topLine, barX + TASKBAR_RADIUS, barY, barX + barW - TASKBAR_RADIUS, barY);

    DrawRoundRect(g, barX, barY, barW, barH, TASKBAR_RADIUS, Color(35, 255, 255, 255), 1.0f);

    float logoRot = (g_tick % 6000) / 6000.0f * 360.0f;
    DrawVortexLogo(g, barX + 28, barY + barH / 2, 14, logoRot);

    int iconsStartX = barX + 56;
    int iconY = barY + (barH - TASKBAR_ICON_SIZE) / 2;

    for (int i = 0; i < iconCount; i++) {
        int ix = iconsStartX + i * (TASKBAR_ICON_SIZE + TASKBAR_ICON_SPACING);

        // БЫСТРАЯ анимация масштаба
        float target = (g_hoveredTaskbarIcon == i) ? 1.18f : 1.0f;
        g_taskbarIconScale[i] += (target - g_taskbarIconScale[i]) * 0.55f;

        bool hov = (g_hoveredTaskbarIcon == i);
        DrawTaskbarIcon(g, ix, iconY, TASKBAR_ICON_SIZE, g_taskApps[i], hov, g_taskbarIconScale[i]);

        if (g_taskApps[i].running) {
            int dotX = ix + TASKBAR_ICON_SIZE / 2;
            int dotY = barY + barH - 6;

            SolidBrush glow(Color(60, g_taskApps[i].accentColor.GetR(),
                g_taskApps[i].accentColor.GetG(), g_taskApps[i].accentColor.GetB()));
            g.FillEllipse(&glow, dotX - 5, dotY - 2, 10, 6);

            SolidBrush dot(g_taskApps[i].accentColor);
            g.FillEllipse(&dot, dotX - 2, dotY - 1, 4, 3);
        }

        SetRect(&g_taskApps[i].bounds, ix, iconY, ix + TASKBAR_ICON_SIZE, iconY + TASKBAR_ICON_SIZE);
    }

    time_t now = time(NULL);
    struct tm ti;
    localtime_s(&ti, &now);
    wchar_t timeStr[32];
    wcsftime(timeStr, 32, L"%H:%M", &ti);

    FontFamily ff(L"Segoe UI");
    Font timeFont(&ff, 14.0f, FontStyleBold, UnitPixel);
    SolidBrush timeBr(VX::TextWhite);

    StringFormat rfmt;
    rfmt.SetAlignment(StringAlignmentFar);
    rfmt.SetLineAlignment(StringAlignmentCenter);

    RectF timeRc((REAL)(barX + barW - 70), (REAL)barY, 60.0f, (REAL)barH);
    g.DrawString(timeStr, -1, &timeFont, timeRc, &rfmt, &timeBr);

    wchar_t dateStr[32];
    wcsftime(dateStr, 32, L"%d.%m", &ti);
    Font dateFont(&ff, 9.0f, FontStyleRegular, UnitPixel);
    SolidBrush dateBr(VX::TextMuted);
    RectF dateRc((REAL)(barX + barW - 70), (REAL)(barY + 20), 60.0f, 20.0f);
    g.DrawString(dateStr, -1, &dateFont, dateRc, &rfmt, &dateBr);
}

// ============================================================================
//  ПЛАВАЮЩАЯ СТАТУС-ПАНЕЛЬ
// ============================================================================
void DrawStatusPanel(Graphics& g, int sw) {
    int panelW = 280;
    int panelH = STATUS_PANEL_H;
    int panelX = sw - panelW - 12;
    int panelY = 10;

    FillRoundRectSolid(g, panelX, panelY, panelW, panelH, STATUS_PANEL_RADIUS,
                        Color(180, 12, 12, 22));
    DrawRoundRect(g, panelX, panelY, panelW, panelH, STATUS_PANEL_RADIUS,
                  Color(30, 0, 255, 200), 1.0f);

    FontFamily ff(L"Segoe UI");
    Font font(&ff, 11.0f, FontStyleRegular, UnitPixel);

    int cx = panelX + 14;
    int cy = panelY + panelH / 2;

    SolidBrush greenBr(VX::Green);
    g.FillEllipse(&greenBr, cx, cy - 4, 8, 8);
    cx += 14;

    SolidBrush dimBr(VX::TextDim);
    StringFormat lfmt;
    lfmt.SetAlignment(StringAlignmentNear);
    lfmt.SetLineAlignment(StringAlignmentCenter);

    RectF wifiRc((REAL)cx, (REAL)panelY, 35.0f, (REAL)panelH);
    g.DrawString(L"WiFi", -1, &font, wifiRc, &lfmt, &dimBr);
    cx += 40;

    Pen sepPen(Color(40, 255, 255, 255), 1.0f);
    g.DrawLine(&sepPen, cx, panelY + 8, cx, panelY + panelH - 8);
    cx += 10;

    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);

    int battPct = 85;
    wchar_t battStr[16];
    swprintf_s(battStr, L"%d%%", battPct);

    Color battColor = battPct > 50 ? VX::Green : (battPct > 20 ? VX::Yellow : VX::Red);
    SolidBrush battBr(battColor);

    Pen battPen(battColor, 1.5f);
    g.DrawRectangle(&battPen, cx, cy - 5, 18, 10);
    g.FillRectangle(&battBr, cx + 2, cy - 3, (int)(14.0f * battPct / 100.0f), 6);
    SolidBrush battTip(battColor);
    g.FillRectangle(&battTip, cx + 18, cy - 2, 3, 4);
    cx += 26;

    RectF battRc2((REAL)cx, (REAL)panelY, 30.0f, (REAL)panelH);
    g.DrawString(battStr, -1, &font, battRc2, &lfmt, &dimBr);
    cx += 36;

    g.DrawLine(&sepPen, cx, panelY + 8, cx, panelY + panelH - 8);
    cx += 10;

    int ramPct = (int)mem.dwMemoryLoad;
    wchar_t ramStr[32];
    swprintf_s(ramStr, L"RAM %d%%", ramPct);

    Color ramColor = ramPct > 80 ? VX::Red : (ramPct > 50 ? VX::Yellow : VX::Neon);
    SolidBrush ramBr(ramColor);

    g.FillEllipse(&ramBr, cx, cy - 4, 8, 8);
    cx += 14;

    RectF ramRc2((REAL)cx, (REAL)panelY, 60.0f, (REAL)panelH);
    g.DrawString(ramStr, -1, &font, ramRc2, &lfmt, &dimBr);
    cx += 65;

    if (g_musicPlaying) {
        for (int i = 0; i < 4; i++) {
            float h = 4.0f + sinf(g_tick / 200.0f + i * 1.5f) * 6.0f;
            SolidBrush eqBr(VX::Magenta);
            g.FillRectangle(&eqBr, cx + i * 5, (REAL)(cy + 5) - h, 3.0f, h);
        }
    }
}

// ============================================================================
//  ВИДЖЕТЫ
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
    w.height = 140;
    w.accent = VX::Neon;
    g_widgets.push_back(w);

    MEMORYSTATUSEX mem2;
    mem2.dwLength = sizeof(mem2);
    GlobalMemoryStatusEx(&mem2);

    int cpuSim = 25 + (g_tick / 800) % 30;
    swprintf_s(buf, L"CPU  %d%%\nRAM  %d%%\nGPU  45%%\nDISK 62%%", cpuSim, (int)mem2.dwMemoryLoad);
    w.title = L"SYSTEM";
    w.content = buf;
    w.height = 150;
    w.accent = VX::Purple;
    g_widgets.push_back(w);

    w.title = L"WEATHER";
    w.content = L"22\u00B0C  Sunny\nMoscow, RU\nWind: 3 m/s";
    w.height = 120;
    w.accent = VX::Yellow;
    g_widgets.push_back(w);

    w.title = L"NOW PLAYING";
    w.content = g_musicPlaying ? L"Blinding Lights\nThe Weeknd\n\x25B6 3:42 / 5:20" :
                                 L"Nothing playing\n\nPress M to play";
    w.height = 120;
    w.accent = VX::Magenta;
    g_widgets.push_back(w);
}

void DrawWidgets(Graphics& g, int sh) {
    if (g_widgetsAnim <= 0.01f) return;

    float alpha = g_widgetsAnim;
    int offsetX = (int)(-WIDGET_W * (1.0f - alpha));
    int baseX = 16 + offsetX;
    int baseY = 56;

    for (size_t i = 0; i < g_widgets.size(); i++) {
        const Widget& w = g_widgets[i];
        int x = baseX;
        int y = baseY;
        int width = WIDGET_W;
        int height = w.height;

        bool hov = (g_hoveredWidget == (int)i);

        Color bgColor = hov ? VX::CardBgHover : VX::CardBg;
        bgColor = Color((BYTE)(bgColor.GetA() * alpha), bgColor.GetR(), bgColor.GetG(), bgColor.GetB());
        FillRoundRectSolid(g, x, y, width, height, 14, bgColor);

        Color borderColor = hov ?
            Color((BYTE)(120 * alpha), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()) :
            Color((BYTE)(30 * alpha), 255, 255, 255);
        DrawRoundRect(g, x, y, width, height, 14, borderColor, hov ? 1.5f : 0.5f);

        if (hov) {
            DrawNeonGlow(g, x, y, width, height, 14,
                Color((BYTE)(40 * alpha), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()), 2);
        }

        FillRoundRectSolid(g, x + 1, y + 12, 3, height - 24, 2,
            Color((BYTE)(200 * alpha), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));

        FontFamily ff(L"Segoe UI");
        Font titleFont(&ff, 10.0f, FontStyleBold, UnitPixel);
        SolidBrush titleBr(Color((BYTE)(180 * alpha), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));

        StringFormat fmt;
        fmt.SetAlignment(StringAlignmentNear);
        fmt.SetLineAlignment(StringAlignmentNear);

        RectF titleRc((REAL)(x + 16), (REAL)(y + 12), (REAL)(width - 32), 14.0f);
        g.DrawString(w.title.c_str(), -1, &titleFont, titleRc, &fmt, &titleBr);

        Font contentFont(&ff, 13.0f, FontStyleRegular, UnitPixel);
        SolidBrush contentBr(Color((BYTE)(240 * alpha), 240, 240, 255));

        std::wistringstream ss(w.content);
        std::wstring line;
        float ly = (REAL)(y + 32);

        bool first = true;
        while (std::getline(ss, line)) {
            Font* f = &contentFont;
            Font bigFont(&ff, 28.0f, FontStyleBold, UnitPixel);
            if (first && w.title == L"CLOCK") {
                f = &bigFont;
                RectF lineRc((REAL)(x + 16), ly, (REAL)(width - 32), 34.0f);
                g.DrawString(line.c_str(), -1, f, lineRc, &fmt, &contentBr);
                ly += 36.0f;
            } else {
                RectF lineRc((REAL)(x + 16), ly, (REAL)(width - 32), 18.0f);
                g.DrawString(line.c_str(), -1, f, lineRc, &fmt, &contentBr);
                ly += 20.0f;
            }
            first = false;
        }

        if (w.title == L"SYSTEM") {
            MEMORYSTATUSEX mem3;
            mem3.dwLength = sizeof(mem3);
            GlobalMemoryStatusEx(&mem3);
            int vals[] = { 25 + (int)((g_tick / 800) % 30), (int)mem3.dwMemoryLoad, 45, 62 };
            Color colors[] = { VX::Neon, VX::Purple, VX::Green, VX::Orange };

            for (int j = 0; j < 4; j++) {
                int barX = x + 60;
                int barY2 = y + 34 + j * 28;
                int barW = width - 80;
                int barH2 = 6;

                FillRoundRectSolid(g, barX, barY2, barW, barH2, 3, Color((BYTE)(40 * alpha), 255, 255, 255));

                int fillW = (int)(barW * vals[j] / 100.0f);
                if (fillW > 0) {
                    FillRoundRectSolid(g, barX, barY2, fillW, barH2, 3,
                        Color((BYTE)(220 * alpha), colors[j].GetR(), colors[j].GetG(), colors[j].GetB()));
                }
            }
        }

        baseY += height + WIDGET_MARGIN;
    }
}

// ============================================================================
//  СТАРТОВОЕ МЕНЮ
// ============================================================================
struct StartApp {
    std::wstring name;
    const wchar_t* icon;
    std::wstring exec;
    Color color;
};

std::vector<StartApp> g_startApps;

void InitStartApps() {
    g_startApps.clear();
    auto add = [](const wchar_t* n, const wchar_t* ico, const wchar_t* exec, Color c) {
        StartApp a;
        a.name = n; a.icon = ico; a.exec = exec; a.color = c;
        g_startApps.push_back(a);
    };

    add(L"Files",      L"\x1F4C1", L"explorer.exe",   VX::Neon);
    add(L"Browser",    L"\x1F310", L"msedge.exe",     Color(255, 0, 150, 255));
    add(L"Terminal",   L">_",      L"cmd.exe",        VX::Green);
    add(L"Code",       L"</>",     L"notepad.exe",    Color(255, 0, 122, 204));
    add(L"Music",      L"\x266B",  L"wmplayer.exe",   VX::Magenta);
    add(L"Photos",     L"\x1F5BC", L"mspaint.exe",    VX::Orange);
    add(L"Notes",      L"\x1F4DD", L"notepad.exe",    VX::Yellow);
    add(L"Settings",   L"\x2699",  L"ms-settings:",   Color(255, 142, 142, 160));
    add(L"Calculator", L"=",       L"calc.exe",       Color(255, 100, 200, 255));
    add(L"Paint",      L"\x1F3A8", L"mspaint.exe",    Color(255, 255, 100, 100));
    add(L"Calendar",   L"\x1F4C5", L"notepad.exe",    VX::Red);
    add(L"System",     L"\x1F5A5", L"taskmgr.exe",    VX::Purple);
    add(L"Network",    L"\x1F4E1", L"ncpa.cpl",       VX::NeonDim);
    add(L"Security",   L"\x1F512", L"notepad.exe",    VX::Green);
    add(L"Store",      L"\x1F6D2", L"notepad.exe",    Color(255, 0, 180, 255));
    add(L"Help",       L"?",       L"notepad.exe",    VX::TextDim);
}

void DrawStartMenu(Graphics& g, int sw, int sh) {
    if (g_startMenuAnim <= 0.01f) return;

    float a = g_startMenuAnim;

    SolidBrush overlay(Color((BYTE)(140 * a), 0, 0, 0));
    g.FillRectangle(&overlay, 0, 0, sw, sh);

    int menuW = START_MENU_W;
    int menuH = START_MENU_H;
    int menuX = (sw - menuW) / 2;
    int targetY = (sh - menuH) / 2 - 20;
    int menuY = (int)(targetY + 40 * (1.0f - a));

    Color bgColor2(Color((BYTE)(210 * a), 14, 14, 24));
    FillRoundRectSolid(g, menuX, menuY, menuW, menuH, 20, bgColor2);

    DrawRoundRect(g, menuX, menuY, menuW, menuH, 20,
        Color((BYTE)(80 * a), 0, 255, 200), 1.5f);

    Pen topNeon(Color((BYTE)(150 * a), 0, 255, 200), 2.0f);
    g.DrawLine(&topNeon, menuX + 20, menuY, menuX + menuW - 20, menuY);

    float logoRot = (g_tick % 6000) / 6000.0f * 360.0f;
    DrawVortexLogo(g, menuX + 30, menuY + 28, 16, logoRot);

    FontFamily ff(L"Segoe UI");
    Font brandFont(&ff, 18.0f, FontStyleBold, UnitPixel);
    SolidBrush brandBr(Color((BYTE)(255 * a), 0, 255, 200));

    StringFormat lfmt;
    lfmt.SetAlignment(StringAlignmentNear);
    lfmt.SetLineAlignment(StringAlignmentCenter);

    RectF brandRc((REAL)(menuX + 56), (REAL)(menuY + 14), 120.0f, 30.0f);
    g.DrawString(L"VORTEX", -1, &brandFont, brandRc, &lfmt, &brandBr);

    int searchX = menuX + 24;
    int searchY = menuY + 56;
    int searchW = menuW - 48;
    int searchH = 40;

    FillRoundRectSolid(g, searchX, searchY, searchW, searchH, 10,
        Color((BYTE)(30 * a), 255, 255, 255));
    DrawRoundRect(g, searchX, searchY, searchW, searchH, 10,
        Color((BYTE)(40 * a), 0, 255, 200), 1.0f);

    Font searchFont(&ff, 13.0f, FontStyleRegular, UnitPixel);
    SolidBrush searchBr(Color((BYTE)(100 * a), 200, 200, 220));
    RectF searchRc((REAL)(searchX + 36), (REAL)searchY, (REAL)(searchW - 44), (REAL)searchH);
    g.DrawString(L"Search apps, files, settings...", -1, &searchFont, searchRc, &lfmt, &searchBr);

    Pen searchPen(Color((BYTE)(150 * a), 0, 255, 200), 2.0f);
    g.DrawEllipse(&searchPen, searchX + 12, searchY + 10, 14, 14);
    g.DrawLine(&searchPen, searchX + 24, searchY + 24, searchX + 28, searchY + 28);

    Font sectionFont(&ff, 11.0f, FontStyleBold, UnitPixel);
    SolidBrush sectionBr(Color((BYTE)(150 * a), 0, 255, 200));
    RectF pinnedRc((REAL)(menuX + 28), (REAL)(menuY + 108), 100.0f, 16.0f);
    g.DrawString(L"PINNED", -1, &sectionFont, pinnedRc, &lfmt, &sectionBr);

    int gridX = menuX + 28;
    int gridY = menuY + 132;
    int cols = 4;
    int cellW = (menuW - 56) / cols;
    int cellH = 80;

    for (int i = 0; i < (int)g_startApps.size() && i < 16; i++) {
        int row = i / cols;
        int col = i % cols;
        int ix = gridX + col * cellW;
        int iy = gridY + row * cellH;

        bool hov = (g_hoveredStartIcon == i);

        if (hov) {
            FillRoundRectSolid(g, ix + 4, iy + 2, cellW - 8, cellH - 4, 10,
                Color((BYTE)(60 * a), g_startApps[i].color.GetR(),
                      g_startApps[i].color.GetG(), g_startApps[i].color.GetB()));
            DrawNeonGlow(g, ix + 4, iy + 2, cellW - 8, cellH - 4, 10,
                Color((BYTE)(30 * a), g_startApps[i].color.GetR(),
                      g_startApps[i].color.GetG(), g_startApps[i].color.GetB()), 2);
        }

        int icoSize = 38;
        int icoX = ix + (cellW - icoSize) / 2;
        int icoY = iy + 6;

        FillRoundRectSolid(g, icoX, icoY, icoSize, icoSize, 10,
            Color((BYTE)(160 * a), g_startApps[i].color.GetR() / 3,
                  g_startApps[i].color.GetG() / 3, g_startApps[i].color.GetB() / 3));

        if (hov) {
            DrawRoundRect(g, icoX, icoY, icoSize, icoSize, 10,
                Color((BYTE)(180 * a), g_startApps[i].color.GetR(),
                      g_startApps[i].color.GetG(), g_startApps[i].color.GetB()), 1.5f);
        }

        Font icoFont(&ff, 16.0f, FontStyleBold, UnitPixel);
        SolidBrush icoBr(Color((BYTE)(230 * a), g_startApps[i].color.GetR(),
            g_startApps[i].color.GetG(), g_startApps[i].color.GetB()));

        StringFormat cfmt;
        cfmt.SetAlignment(StringAlignmentCenter);
        cfmt.SetLineAlignment(StringAlignmentCenter);

        RectF icoRc((REAL)icoX, (REAL)icoY, (REAL)icoSize, (REAL)icoSize);
        g.DrawString(g_startApps[i].icon, -1, &icoFont, icoRc, &cfmt, &icoBr);

        Font nameFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
        SolidBrush nameBr(Color((BYTE)(200 * a), 220, 220, 240));
        RectF nameRc((REAL)ix, (REAL)(iy + 48), (REAL)cellW, 20.0f);
        g.DrawString(g_startApps[i].name.c_str(), -1, &nameFont, nameRc, &cfmt, &nameBr);
    }
}

// ============================================================================
//  ИКОНКИ РАБОЧЕГО СТОЛА - ОТРИСОВКА
// ============================================================================
void DrawDesktopIcons(Graphics& g, int sw, int sh) {
    if (g_startMenuOpen) return; // не рисуем если меню открыто

    FontFamily ff(L"Segoe UI");
    Font iconFont(&ff, 28.0f, FontStyleRegular, UnitPixel);
    Font nameFont(&ff, 11.0f, FontStyleRegular, UnitPixel);

    int startX = sw - DESKTOP_ICON_SIZE - 40;
    int startY = 60;

    for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
        DesktopIcon& d = g_desktopIcons[i];

        int x = startX;
        int y = startY + i * (DESKTOP_ICON_SIZE + DESKTOP_ICON_TEXT_H + DESKTOP_ICON_SPACING);

        // Проверяем что не вылезаем за экран
        if (y + DESKTOP_ICON_SIZE + DESKTOP_ICON_TEXT_H > sh - TASKBAR_HEIGHT - 20) break;

        bool hov = (g_hoveredDesktopIcon == i);

        // Фон при наведении
        int totalH = DESKTOP_ICON_SIZE + DESKTOP_ICON_TEXT_H;
        if (hov || d.selected) {
            FillRoundRectSolid(g, x - 8, y - 4, DESKTOP_ICON_SIZE + 16, totalH + 8, 10,
                Color(hov ? 80 : 50, d.color.GetR(), d.color.GetG(), d.color.GetB()));
            if (hov) {
                DrawRoundRect(g, x - 8, y - 4, DESKTOP_ICON_SIZE + 16, totalH + 8, 10,
                    Color(80, d.color.GetR(), d.color.GetG(), d.color.GetB()), 1.0f);
            }
        }

        // Иконка-символ
        StringFormat cfmt;
        cfmt.SetAlignment(StringAlignmentCenter);
        cfmt.SetLineAlignment(StringAlignmentCenter);

        SolidBrush icoBr(Color(hov ? 255 : 200, d.color.GetR(), d.color.GetG(), d.color.GetB()));
        RectF icoRc((REAL)x, (REAL)y, (REAL)DESKTOP_ICON_SIZE, (REAL)DESKTOP_ICON_SIZE);
        g.DrawString(d.icon, -1, &iconFont, icoRc, &cfmt, &icoBr);

        // Тень под текстом для читаемости
        SolidBrush shadowBr(Color(160, 0, 0, 0));
        RectF shadowRc((REAL)(x - 7), (REAL)(y + DESKTOP_ICON_SIZE + 1), (REAL)(DESKTOP_ICON_SIZE + 14), (REAL)DESKTOP_ICON_TEXT_H);
        g.DrawString(d.name.c_str(), -1, &nameFont, shadowRc, &cfmt, &shadowBr);

        // Текст имени
        SolidBrush nameBr(VX::TextWhite);
        RectF nameRc((REAL)(x - 8), (REAL)(y + DESKTOP_ICON_SIZE), (REAL)(DESKTOP_ICON_SIZE + 16), (REAL)DESKTOP_ICON_TEXT_H);
        g.DrawString(d.name.c_str(), -1, &nameFont, nameRc, &cfmt, &nameBr);

        SetRect(&d.bounds, x - 8, y - 4, x + DESKTOP_ICON_SIZE + 8, y + totalH + 4);
    }
}

// ============================================================================
//  WiFi ПАНЕЛЬ
// ============================================================================
void DrawWifiPanel(Graphics& g, int sw, int sh) {
    if (g_wifiPanelAnim <= 0.01f) return;

    float a = g_wifiPanelAnim;

    int panelW = WIFI_PANEL_W;
    int panelH = WIFI_PANEL_H;
    int panelX = sw - panelW - 16;
    int panelY = (int)(STATUS_PANEL_H + 20 + 30 * (1.0f - a));

    // Фон
    FillRoundRectSolid(g, panelX, panelY, panelW, panelH, 16,
        Color((BYTE)(220 * a), 14, 14, 24));
    DrawRoundRect(g, panelX, panelY, panelW, panelH, 16,
        Color((BYTE)(60 * a), 0, 255, 200), 1.5f);

    // Верхняя линия
    Pen topLine(Color((BYTE)(120 * a), 0, 255, 200), 2.0f);
    g.DrawLine(&topLine, panelX + 16, panelY, panelX + panelW - 16, panelY);

    FontFamily ff(L"Segoe UI");
    Font titleFont(&ff, 15.0f, FontStyleBold, UnitPixel);
    Font itemFont(&ff, 13.0f, FontStyleRegular, UnitPixel);
    Font smallFont(&ff, 10.0f, FontStyleRegular, UnitPixel);

    StringFormat lfmt;
    lfmt.SetAlignment(StringAlignmentNear);
    lfmt.SetLineAlignment(StringAlignmentCenter);

    // Заголовок
    SolidBrush titleBr(Color((BYTE)(255 * a), 0, 255, 200));
    RectF titleRc((REAL)(panelX + 18), (REAL)(panelY + 12), (REAL)(panelW - 36), 24.0f);
    g.DrawString(L"\xD83D\xDCF6  WiFi Networks", -1, &titleFont, titleRc, &lfmt, &titleBr);

    // Кнопка Scan
    int scanBtnX = panelX + panelW - 80;
    int scanBtnY = panelY + 12;
    FillRoundRectSolid(g, scanBtnX, scanBtnY, 60, 24, 8,
        Color((BYTE)(60 * a), 0, 255, 200));
    DrawRoundRect(g, scanBtnX, scanBtnY, 60, 24, 8,
        Color((BYTE)(100 * a), 0, 255, 200), 1.0f);

    Font btnFont(&ff, 10.0f, FontStyleBold, UnitPixel);
    SolidBrush btnBr(Color((BYTE)(220 * a), 0, 255, 200));
    StringFormat cfmt;
    cfmt.SetAlignment(StringAlignmentCenter);
    cfmt.SetLineAlignment(StringAlignmentCenter);
    RectF btnRc((REAL)scanBtnX, (REAL)scanBtnY, 60.0f, 24.0f);
    g.DrawString(g_wifiScanning ? L"..." : L"SCAN", -1, &btnFont, btnRc, &cfmt, &btnBr);

    // Разделитель
    Pen sep(Color((BYTE)(30 * a), 255, 255, 255), 1.0f);
    g.DrawLine(&sep, panelX + 16, panelY + 44, panelX + panelW - 16, panelY + 44);

    // Список сетей
    int itemY = panelY + 52;
    int itemH = 52;

    for (int i = 0; i < (int)g_wifiNetworks.size() && i < 7; i++) {
        const WifiNetwork& net = g_wifiNetworks[i];
        bool hov = (g_hoveredWifiItem == i);

        int iy = itemY + i * itemH;

        // Фон при наведении
        if (hov) {
            FillRoundRectSolid(g, panelX + 8, iy, panelW - 16, itemH - 4, 10,
                Color((BYTE)(40 * a), 0, 255, 200));
        }

        // Иконка сигнала
        Color sigColor;
        if (net.signal >= 70) sigColor = VX::Green;
        else if (net.signal >= 40) sigColor = VX::Yellow;
        else sigColor = VX::Red;

        // Рисуем иконку WiFi сигнала (дуги)
        int sigX = panelX + 24;
        int sigY = iy + itemH / 2;

        for (int bar = 0; bar < 4; bar++) {
            int barH = 4 + bar * 5;
            Color barColor = (net.signal >= (bar + 1) * 25) ?
                Color((BYTE)(200 * a), sigColor.GetR(), sigColor.GetG(), sigColor.GetB()) :
                Color((BYTE)(30 * a), 100, 100, 100);
            SolidBrush barBr(barColor);
            g.FillRectangle(&barBr, sigX + bar * 6, sigY + 10 - barH, 4, barH);
        }

        // Имя сети
        SolidBrush nameBr(Color((BYTE)(240 * a), 240, 240, 255));
        RectF nameRc((REAL)(panelX + 54), (REAL)(iy + 6), (REAL)(panelW - 120), 20.0f);
        g.DrawString(net.ssid.c_str(), -1, &itemFont, nameRc, &lfmt, &nameBr);

        // Статус
        std::wstring status;
        if (net.connected) status = L"\x2705 Connected";
        else if (net.secured) status = L"\xD83D\xDD12 Secured";
        else status = L"\xD83D\xDD13 Open";

        SolidBrush statusBr(Color((BYTE)(140 * a), 150, 160, 180));
        RectF statusRc((REAL)(panelX + 54), (REAL)(iy + 26), (REAL)(panelW - 120), 16.0f);
        g.DrawString(status.c_str(), -1, &smallFont, statusRc, &lfmt, &statusBr);

        // Процент сигнала
        wchar_t sigStr[16];
        swprintf_s(sigStr, L"%d%%", net.signal);
        SolidBrush sigBr(Color((BYTE)(180 * a), sigColor.GetR(), sigColor.GetG(), sigColor.GetB()));
        StringFormat rfmt;
        rfmt.SetAlignment(StringAlignmentFar);
        rfmt.SetLineAlignment(StringAlignmentCenter);
        RectF sigRc((REAL)(panelX + panelW - 60), (REAL)iy, 44.0f, (REAL)itemH);
        g.DrawString(sigStr, -1, &itemFont, sigRc, &rfmt, &sigBr);

        // Кнопка Connect при ховере (если не подключена)
        if (hov && !net.connected) {
            int btnX2 = panelX + panelW - 100;
            int btnY2 = iy + itemH / 2 - 10;
            FillRoundRectSolid(g, btnX2, btnY2, 70, 20, 6,
                Color((BYTE)(80 * a), 0, 255, 200));
            DrawRoundRect(g, btnX2, btnY2, 70, 20, 6,
                Color((BYTE)(150 * a), 0, 255, 200), 1.0f);

            SolidBrush connBr(Color((BYTE)(240 * a), 0, 255, 200));
            RectF connRc((REAL)btnX2, (REAL)btnY2, 70.0f, 20.0f);
            g.DrawString(L"Connect", -1, &smallFont, connRc, &cfmt, &connBr);
        }
    }

    if (g_wifiNetworks.empty()) {
        SolidBrush emptyBr(Color((BYTE)(120 * a), 150, 160, 180));
        RectF emptyRc((REAL)panelX, (REAL)(panelY + 80), (REAL)panelW, 30.0f);
        g.DrawString(g_wifiScanning ? L"Scanning..." : L"No networks found. Press SCAN.",
            -1, &itemFont, emptyRc, &cfmt, &emptyBr);
    }
}

// ============================================================================
//  КОНТЕКСТНОЕ МЕНЮ (ПКМ)
// ============================================================================
struct ContextMenuItem {
    std::wstring label;
    const wchar_t* icon;
    int id; // 0=separator, 1=new_folder, 2=refresh, 3=settings, 4=terminal, 5=wallpaper
};

std::vector<ContextMenuItem> g_contextItems;

void InitContextMenu() {
    g_contextItems.clear();
    ContextMenuItem item;

    item.label = L"New Folder"; item.icon = L"\xD83D\xDCC1"; item.id = 1;
    g_contextItems.push_back(item);

    item.label = L""; item.icon = L""; item.id = 0; // separator
    g_contextItems.push_back(item);

    item.label = L"Open Terminal"; item.icon = L">_"; item.id = 4;
    g_contextItems.push_back(item);

    item.label = L"Open Explorer"; item.icon = L"\xD83D\xDCC2"; item.id = 6;
    g_contextItems.push_back(item);

    item.label = L""; item.icon = L""; item.id = 0;
    g_contextItems.push_back(item);

    item.label = L"WiFi Networks"; item.icon = L"\xD83D\xDCF6"; item.id = 7;
    g_contextItems.push_back(item);

    item.label = L"Refresh"; item.icon = L"\xD83D\xDD04"; item.id = 2;
    g_contextItems.push_back(item);

    item.label = L""; item.icon = L""; item.id = 0;
    g_contextItems.push_back(item);

    item.label = L"Settings"; item.icon = L"\x2699"; item.id = 3;
    g_contextItems.push_back(item);
}

void DrawContextMenu(Graphics& g) {
    if (!g_contextMenuOpen) return;

    int itemH = 36;
    int sepH = 10;
    int menuW = 220;
    int menuH = 8; // padding

    for (const auto& item : g_contextItems) {
        menuH += (item.id == 0) ? sepH : itemH;
    }
    menuH += 8; // bottom padding

    int mx = g_contextMenuX;
    int my = g_contextMenuY;

    // Не выходить за экран
    if (mx + menuW > ScreenW()) mx = ScreenW() - menuW - 8;
    if (my + menuH > ScreenH()) my = ScreenH() - menuH - 8;

    // Тень
    FillRoundRectSolid(g, mx + 4, my + 4, menuW, menuH, 12, Color(80, 0, 0, 0));

    // Фон
    FillRoundRectSolid(g, mx, my, menuW, menuH, 12, Color(230, 16, 16, 28));
    DrawRoundRect(g, mx, my, menuW, menuH, 12, Color(50, 0, 255, 200), 1.0f);

    // Верхняя линия
    Pen topLine(Color(100, 0, 255, 200), 1.5f);
    g.DrawLine(&topLine, mx + 12, my, mx + menuW - 12, my);

    FontFamily ff(L"Segoe UI");
    Font font(&ff, 12.0f, FontStyleRegular, UnitPixel);
    Font icoFont(&ff, 14.0f, FontStyleRegular, UnitPixel);

    StringFormat lfmt;
    lfmt.SetAlignment(StringAlignmentNear);
    lfmt.SetLineAlignment(StringAlignmentCenter);

    int cy = my + 8;
    for (int i = 0; i < (int)g_contextItems.size(); i++) {
        const auto& item = g_contextItems[i];

        if (item.id == 0) {
            // Separator
            Pen sep(Color(30, 255, 255, 255), 1.0f);
            g.DrawLine(&sep, mx + 16, cy + sepH / 2, mx + menuW - 16, cy + sepH / 2);
            cy += sepH;
            continue;
        }

        bool hov = (g_hoveredContextItem == i);

        if (hov) {
            FillRoundRectSolid(g, mx + 6, cy + 2, menuW - 12, itemH - 4, 8,
                Color(50, 0, 255, 200));
        }

        // Иконка
        SolidBrush icoBr(hov ? Color(255, 0, 255, 200) : Color(160, 180, 190, 220));
        RectF icoRc((REAL)(mx + 14), (REAL)cy, 24.0f, (REAL)itemH);
        g.DrawString(item.icon, -1, &icoFont, icoRc, &lfmt, &icoBr);

        // Текст
        SolidBrush textBr(hov ? Color(255, 240, 240, 255) : Color(200, 200, 210, 230));
        RectF textRc((REAL)(mx + 42), (REAL)cy, (REAL)(menuW - 56), (REAL)itemH);
        g.DrawString(item.label.c_str(), -1, &font, textRc, &lfmt, &textBr);

        cy += itemH;
    }
}

// ============================================================================
//  УВЕДОМЛЕНИЯ
// ============================================================================
void PushNotification(const std::wstring& title, const std::wstring& msg) {
    Notification n;
    n.title = title;
    n.message = msg;
    n.time = GetTickCount();
    n.alpha = 0.0f;
    n.offsetY = -40.0f;
    n.alive = true;
    g_notifs.push_back(n);
}

void DrawNotifications(Graphics& g, int sw) {
    int baseY = 56;
    for (size_t i = 0; i < g_notifs.size() && i < 3; i++) {
        Notification& n = g_notifs[i];
        if (!n.alive || n.alpha <= 0.01f) continue;

        float a = n.alpha;
        int nx = sw - NOTIF_W - 16;
        int ny = (int)(baseY + n.offsetY + i * (NOTIF_H + 8));

        FillRoundRectSolid(g, nx, ny, NOTIF_W, NOTIF_H, 14,
            Color((BYTE)(200 * a), 18, 18, 30));

        DrawRoundRect(g, nx, ny, NOTIF_W, NOTIF_H, 14,
            Color((BYTE)(60 * a), 0, 255, 200), 1.0f);

        FillRoundRectSolid(g, nx + 1, ny + 14, 3, NOTIF_H - 28, 2,
            Color((BYTE)(200 * a), 0, 255, 200));

        SolidBrush logoBg(Color((BYTE)(150 * a), 0, 255, 200));
        g.FillEllipse(&logoBg, nx + 16, ny + NOTIF_H / 2 - 14, 28, 28);
        SolidBrush logoDot(Color((BYTE)(255 * a), 255, 255, 255));
        g.FillEllipse(&logoDot, nx + 27, ny + NOTIF_H / 2 - 3, 6, 6);

        FontFamily ff(L"Segoe UI");
        Font titleFont(&ff, 12.0f, FontStyleBold, UnitPixel);
        Font msgFont(&ff, 11.0f, FontStyleRegular, UnitPixel);

        SolidBrush titleBr(Color((BYTE)(255 * a), 240, 240, 255));
        SolidBrush msgBr(Color((BYTE)(180 * a), 180, 190, 220));

        StringFormat fmt;
        fmt.SetAlignment(StringAlignmentNear);
        fmt.SetLineAlignment(StringAlignmentNear);

        RectF titleRc((REAL)(nx + 54), (REAL)(ny + 16), (REAL)(NOTIF_W - 70), 16.0f);
        g.DrawString(n.title.c_str(), -1, &titleFont, titleRc, &fmt, &titleBr);

        RectF msgRc2((REAL)(nx + 54), (REAL)(ny + 36), (REAL)(NOTIF_W - 70), 30.0f);
        g.DrawString(n.message.c_str(), -1, &msgFont, msgRc2, &fmt, &msgBr);

        DWORD elapsed = (GetTickCount() - n.time) / 1000;
        wchar_t timeStr2[32];
        if (elapsed < 60) swprintf_s(timeStr2, L"%ds ago", elapsed);
        else swprintf_s(timeStr2, L"%dm ago", elapsed / 60);

        Font timeFont(&ff, 9.0f, FontStyleRegular, UnitPixel);
        SolidBrush timeBr2(Color((BYTE)(100 * a), 120, 130, 160));

        StringFormat rfmt;
        rfmt.SetAlignment(StringAlignmentFar);
        rfmt.SetLineAlignment(StringAlignmentNear);

        RectF timeRc2((REAL)(nx + NOTIF_W - 80), (REAL)(ny + 16), 64.0f, 14.0f);
        g.DrawString(timeStr2, -1, &timeFont, timeRc2, &rfmt, &timeBr2);
    }
}

// ============================================================================
//  ЧАСТИЦЫ
// ============================================================================
void InitParticles() {
    g_particles.clear();
    std::mt19937 rng(42);
    int sw = ScreenW(), sh = ScreenH();
    for (int i = 0; i < 60; i++) {
        Particle p;
        p.x = (float)(rng() % sw);
        p.y = (float)(rng() % sh);
        p.vx = ((rng() % 100) - 50) / 200.0f;
        p.vy = -((rng() % 100) + 10) / 300.0f;
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
        SolidBrush br(Color((BYTE)(p.alpha * 255), p.color.GetR(), p.color.GetG(), p.color.GetB()));
        g.FillEllipse(&br, p.x - p.size, p.y - p.size, p.size * 2, p.size * 2);
    }
}

// ============================================================================
//  ОБОИ
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
    REAL scale = max((REAL)sw / iw, (REAL)sh / ih);
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

        for (int i = 0; i < 6; i++) {
            int cx = 150 + i * 280;
            int cy = 100 + (i % 3) * 200;
            int cr = 80 + i * 40;
            SolidBrush cb(Color(8, 0, 200 + i * 10, 200));
            g.FillEllipse(&cb, cx - cr, cy - cr, cr * 2, cr * 2);
        }
    }

    SolidBrush dimOverlay(Color(30, 0, 0, 0));
    g.FillRectangle(&dimOverlay, 0, 0, sw, sh);
}

// ============================================================================
//  ПОДСКАЗКИ
// ============================================================================
void DrawHotkeyHints(Graphics& g, int sh) {
    FontFamily ff(L"Segoe UI");
    Font font(&ff, 9.0f, FontStyleRegular, UnitPixel);
    SolidBrush br(Color(60, 200, 200, 220));

    StringFormat fmt;
    fmt.SetAlignment(StringAlignmentNear);
    fmt.SetLineAlignment(StringAlignmentFar);

    RectF rc(12.0f, (REAL)(sh - 28), 500.0f, 16.0f);
    g.DrawString(L"ESC exit | SPACE menu | M music | W widgets | N notify | F wifi | RMB context menu", -1, &font, rc, &fmt, &br);
}

// ============================================================================
//  ДВОЙНАЯ БУФЕРИЗАЦИЯ
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
//  ГЛАВНЫЙ РЕНДЕР
// ============================================================================
void Render(HWND hWnd, HDC hdc, int sw, int sh) {
    EnsureBuffer(hWnd, sw, sh);

    Graphics g(g_memDC);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    g.SetCompositingQuality(CompositingQualityHighQuality);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

    // 1. Фон
    DrawBackground(g, sw, sh);

    // 2. Частицы
    DrawParticles(g);

    // 3. Иконки рабочего стола
    DrawDesktopIcons(g, sw, sh);

    // 4. Виджеты (слева)
    DrawWidgets(g, sh);

    // 5. Статус-панель
    DrawStatusPanel(g, sw);

    // 6. Уведомления
    DrawNotifications(g, sw);

    // 7. WiFi панель
    DrawWifiPanel(g, sw, sh);

    // 8. Панель задач
    DrawTaskbar(g, sw, sh);

    // 9. Контекстное меню
    DrawContextMenu(g);

    // 10. Start Menu
    DrawStartMenu(g, sw, sh);

    // 11. Подсказки
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
        UpdateWidgets();
        CreateThread(NULL, 0, WallpaperThread, NULL, 0, NULL);
        CreateThread(NULL, 0, WifiScanThread, NULL, 0, NULL);
        SetTimer(hWnd, 1, 1000, NULL);
        SetTimer(hWnd, 2, 16, NULL);
        SetTimer(hWnd, 3, 45000, NULL);
        PushNotification(L"Welcome to VORTEX", L"Your cyberpunk desktop is ready!");
        break;

    case WM_TIMER:
        g_tick = GetTickCount();
        if (wParam == 1) {
            UpdateWidgets();
        }
        else if (wParam == 2) {
            // БЫСТРЫЕ анимации - высокий коэффициент интерполяции
            float targetStart = g_startMenuOpen ? 1.0f : 0.0f;
            g_startMenuAnim += (targetStart - g_startMenuAnim) * 0.40f;
            if (fabs(g_startMenuAnim - targetStart) < 0.01f) g_startMenuAnim = targetStart;

            float targetW = g_widgetsVisible ? 1.0f : 0.0f;
            g_widgetsAnim += (targetW - g_widgetsAnim) * 0.35f;
            if (fabs(g_widgetsAnim - targetW) < 0.01f) g_widgetsAnim = targetW;

            float targetWifi = g_wifiPanelOpen ? 1.0f : 0.0f;
            g_wifiPanelAnim += (targetWifi - g_wifiPanelAnim) * 0.40f;
            if (fabs(g_wifiPanelAnim - targetWifi) < 0.01f) g_wifiPanelAnim = targetWifi;

            UpdateParticles();

            // БЫСТРЫЕ уведомления
            for (auto& n : g_notifs) {
                if (!n.alive) continue;
                if (n.alpha < 1.0f) n.alpha = min(1.0f, n.alpha + 0.15f);
                if (n.offsetY < 0) n.offsetY = min(0.0f, n.offsetY + 8.0f);
                if (GetTickCount() - n.time > 6000) {
                    n.alpha -= 0.08f;
                    if (n.alpha <= 0) n.alive = false;
                }
            }
            g_notifs.erase(std::remove_if(g_notifs.begin(), g_notifs.end(),
                [](const Notification& n) { return !n.alive; }), g_notifs.end());

            InvalidateRect(hWnd, NULL, FALSE);
        }
        else if (wParam == 3) {
            if (g_notifs.empty()) {
                PushNotification(L"VORTEX System", L"All systems nominal. Performance optimal.");
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
            int itemH = 36;
            int sepH = 10;
            int cmx = g_contextMenuX;
            int cmy = g_contextMenuY;
            int menuW = 220;
            if (cmx + menuW > ScreenW()) cmx = ScreenW() - menuW - 8;

            int cy = cmy + 8;
            for (int i = 0; i < (int)g_contextItems.size(); i++) {
                if (g_contextItems[i].id == 0) {
                    cy += sepH;
                    continue;
                }
                RECT ir;
                SetRect(&ir, cmx + 6, cy, cmx + menuW - 6, cy + itemH);
                if (PtInRect(&ir, pt)) {
                    g_hoveredContextItem = i;
                    break;
                }
                cy += itemH;
            }
        }

        // Taskbar hover
        g_hoveredTaskbarIcon = -1;
        for (int i = 0; i < (int)g_taskApps.size(); i++) {
            RECT r = g_taskApps[i].bounds;
            InflateRect(&r, 4, 8);
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
            int wy = 56;
            for (int i = 0; i < (int)g_widgets.size(); i++) {
                RECT wr;
                SetRect(&wr, 16, wy, 16 + WIDGET_W, wy + g_widgets[i].height);
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
            int panelX = sw - WIFI_PANEL_W - 16;
            int panelY = STATUS_PANEL_H + 20;
            int itemY = panelY + 52;
            int itemH = 52;

            for (int i = 0; i < (int)g_wifiNetworks.size() && i < 7; i++) {
                int iy = itemY + i * itemH;
                RECT ir;
                SetRect(&ir, panelX + 8, iy, panelX + WIFI_PANEL_W - 8, iy + itemH);
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
            int gridX = menuX + 28;
            int gridY = menuY + 132;
            int cols = 4;
            int cellW = (START_MENU_W - 56) / cols;
            int cellH = 80;

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
        break;
    }

    case WM_LBUTTONDOWN: {
        int mx = GET_X_LPARAM(lParam);
        int my = GET_Y_LPARAM(lParam);
        POINT pt = { mx, my };

        // Закрываем контекстное меню при клике
        if (g_contextMenuOpen) {
            if (g_hoveredContextItem >= 0) {
                int id = g_contextItems[g_hoveredContextItem].id;
                switch (id) {
                case 1: // New Folder
                    CreateNewFolderOnDesktop();
                    PushNotification(L"Folder Created", L"New folder created on Desktop");
                    break;
                case 2: // Refresh
                    InvalidateRect(hWnd, NULL, FALSE);
                    PushNotification(L"Refresh", L"Desktop refreshed");
                    break;
                case 3: // Settings
                    ShellExecute(NULL, L"open", L"ms-settings:", NULL, NULL, SW_SHOW);
                    break;
                case 4: // Terminal
                    ShellExecute(NULL, L"open", L"cmd.exe", NULL, NULL, SW_SHOW);
                    break;
                case 6: // Explorer
                    ShellExecute(NULL, L"open", L"explorer.exe", NULL, NULL, SW_SHOW);
                    break;
                case 7: // WiFi
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

        // WiFi panel - scan button click
        if (g_wifiPanelOpen && g_wifiPanelAnim > 0.5f) {
            int sw = ScreenW();
            int panelX = sw - WIFI_PANEL_W - 16;
            int panelY = STATUS_PANEL_H + 20;
            int scanBtnX = panelX + WIFI_PANEL_W - 80;
            int scanBtnY = panelY + 12;
            RECT scanRect;
            SetRect(&scanRect, scanBtnX, scanBtnY, scanBtnX + 60, scanBtnY + 24);
            if (PtInRect(&scanRect, pt) && !g_wifiScanning) {
                CreateThread(NULL, 0, WifiScanThread, NULL, 0, NULL);
                PushNotification(L"WiFi", L"Scanning for networks...");
            }

            // Клик вне панели закрывает её
            RECT panelRect;
            SetRect(&panelRect, panelX, panelY, panelX + WIFI_PANEL_W, panelY + WIFI_PANEL_H);
            if (!PtInRect(&panelRect, pt)) {
                g_wifiPanelOpen = false;
            }
        }

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
            int barW = contentW + 120;
            int barX = (sw - barW) / 2;
            int barY = sh - TASKBAR_HEIGHT - 8;
            RECT logoRect;
            SetRect(&logoRect, barX + 8, barY + 8, barX + 48, barY + TASKBAR_HEIGHT - 8);
            if (PtInRect(&logoRect, pt)) {
                g_startMenuOpen = !g_startMenuOpen;
            }
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
            // Открыть контекстное меню
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

        // Двойной клик по иконкам рабочего стола
        for (int i = 0; i < (int)g_desktopIcons.size(); i++) {
            if (PtInRect(&g_desktopIcons[i].bounds, pt)) {
                ShellExecute(NULL, L"open", g_desktopIcons[i].action.c_str(), NULL, NULL, SW_SHOW);
                break;
            }
        }
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
        }
        else if (wParam == 'M') {
            g_musicPlaying = !g_musicPlaying;
            PushNotification(L"Music", g_musicPlaying ? L"Now playing: Blinding Lights" : L"Playback paused");
        }
        else if (wParam == 'W') {
            g_widgetsVisible = !g_widgetsVisible;
        }
        else if (wParam == 'N') {
            PushNotification(L"Test Notification", L"This is a test notification from VORTEX");
        }
        else if (wParam == 'F') {
            g_wifiPanelOpen = !g_wifiPanelOpen;
            if (g_wifiPanelOpen && !g_wifiScanning) {
                CreateThread(NULL, 0, WifiScanThread, NULL, 0, NULL);
            }
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
    wc.style = CS_DBLCLKS; // Поддержка двойного клика
    wc.lpszClassName = L"VORTEX_Desktop_v2";
    RegisterClassExW(&wc);

    int sw = ScreenW(), sh = ScreenH();

    g_hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"VORTEX_Desktop_v2", L"VORTEX Desktop",
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
