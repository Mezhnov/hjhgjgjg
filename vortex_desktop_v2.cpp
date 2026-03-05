/*
 * VORTEX Desktop Environment v2.0 - Cyberpunk/Futuristic Design
 *
 * УНИКАЛЬНЫЙ ДИЗАЙН - НЕ macOS, НЕ Windows:
 * - Нижняя панель задач с центрированными иконками (без magnify эффекта macOS)
 * - Плавающие статус-индикаторы в правом верхнем углу
 * - Стартовое меню по центру (как Win11, но кибер-стиль)
 * - Гексагональные/геометрические элементы
 * - Неоновые акценты и свечения
 * - Анимированные частицы на фоне
 * - Виджеты слева с glassmorphism
 * - Уведомления справа с анимацией
 * - Горячие клавиши
 *
 * ESC       - выход / закрыть меню
 * SPACE     - открыть/закрыть Start Menu
 * M         - вкл/выкл музыку
 * W         - показать/скрыть виджеты
 * N         - новое уведомление
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
    const Color Neon(255, 0, 255, 200);           // Cyan-Neon
    const Color NeonBright(255, 0, 255, 240);
    const Color NeonDim(255, 0, 180, 160);
    const Color Purple(255, 139, 92, 246);         // Фиолетовый
    const Color PurpleBright(255, 180, 140, 255);
    const Color PurpleDim(255, 90, 50, 180);
    const Color Magenta(255, 255, 0, 128);         // Маджента
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
int g_hoveredTaskbarIcon = -1;
int g_hoveredStartIcon = -1;
int g_hoveredWidget = -1;

// Анимации
float g_startMenuAnim = 0.0f;       // 0..1
float g_widgetsAnim = 1.0f;         // 0..1
float g_taskbarIconScale[16] = {};   // per-icon hover scale

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

// Неоновое свечение вокруг прямоугольника
void DrawNeonGlow(Graphics& g, int x, int y, int w, int h, int r, Color c, int layers = 4) {
    for (int i = layers; i >= 1; i--) {
        int spread = i * 3;
        Color gc(c.GetA() / (BYTE)(i + 1), c.GetR(), c.GetG(), c.GetB());
        DrawRoundRect(g, x - spread, y - spread, w + spread * 2, h + spread * 2, r + spread, gc, 2.0f);
    }
}

// ============================================================================
//  ЛОГОТИП VORTEX - СПИРАЛЬ С НЕОНОМ
// ============================================================================
void DrawVortexLogo(Graphics& g, int cx, int cy, int radius, float rotation = 0.0f) {
    // Внешнее свечение
    for (int i = 4; i >= 1; i--) {
        SolidBrush glow(Color(15 * i, 0, 255, 200));
        g.FillEllipse(&glow, cx - radius - i * 2, cy - radius - i * 2,
                       (radius + i * 2) * 2, (radius + i * 2) * 2);
    }

    // Основной круг
    LinearGradientBrush bg(Rect(cx - radius, cy - radius, radius * 2, radius * 2),
        VX::Purple, VX::Neon, LinearGradientModeForwardDiagonal);
    g.FillEllipse(&bg, cx - radius, cy - radius, radius * 2, radius * 2);

    // Вращающиеся спиральные лучи
    GraphicsState st = g.Save();
    g.TranslateTransform((REAL)cx, (REAL)cy);
    g.RotateTransform(rotation);

    for (int arm = 0; arm < 4; arm++) {
        float baseAngle = arm * 90.0f;
        std::vector<PointF> pts;
        for (float t = 0.0f; t < 3.14159f * 1.5f; t += 0.08f) {
            float r = radius * 0.15f + (t / (3.14159f * 1.5f)) * radius * 0.7f;
            float a = baseAngle * 3.14159f / 180.0f + t;
            pts.push_back(PointF(cosf(a) * r, sinf(a) * r));
        }
        if (pts.size() > 1) {
            Pen pen(Color(200, 255, 255, 255), 2.0f);
            pen.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
            g.DrawLines(&pen, pts.data(), (int)pts.size());
        }
    }

    g.Restore(st);

    // Центральная точка
    SolidBrush center(Color(255, 255, 255, 255));
    g.FillEllipse(&center, cx - 3, cy - 3, 6, 6);
}

// ============================================================================
//  ИКОНКИ ПАНЕЛИ ЗАДАЧ - ПЛОСКИЙ СТИЛЬ С НЕОНОМ
// ============================================================================
struct TaskbarApp {
    std::wstring name;
    std::wstring exec;
    Color accentColor;
    const wchar_t* icon; // символ-иконка
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

    add(L"Files",    L"explorer.exe",    VX::Neon,    L"\x1F4C1", true);   // 📁
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

    // Фон иконки
    Color bgColor = hovered ?
        Color(200, app.accentColor.GetR() / 3, app.accentColor.GetG() / 3, app.accentColor.GetB() / 3) :
        Color(80, 30, 30, 45);
    FillRoundRectSolid(g, ox, oy, s, s, 10, bgColor);

    // Граница с неоном при ховере
    if (hovered) {
        DrawNeonGlow(g, ox, oy, s, s, 10, app.accentColor, 3);
        DrawRoundRect(g, ox, oy, s, s, 10, Color(200, app.accentColor.GetR(),
            app.accentColor.GetG(), app.accentColor.GetB()), 1.5f);
    } else {
        DrawRoundRect(g, ox, oy, s, s, 10, Color(40, 255, 255, 255), 0.5f);
    }

    // Текст-иконка
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
//  ПАНЕЛЬ ЗАДАЧ - ЦЕНТРИРОВАННАЯ, СТЕКЛЯННАЯ, С НЕОНОМ
// ============================================================================
void DrawTaskbar(Graphics& g, int sw, int sh) {
    int iconCount = (int)g_taskApps.size();
    int contentW = iconCount * (TASKBAR_ICON_SIZE + TASKBAR_ICON_SPACING) - TASKBAR_ICON_SPACING + 24;

    // Общая панель за иконками
    int barW = contentW + 120; // extra для лого и часов
    int barH = TASKBAR_HEIGHT;
    int barX = (sw - barW) / 2;
    int barY = sh - barH - 8;

    // Фон панели
    GraphicsPath barPath;
    RoundedRectPath(barPath, barX, barY, barW, barH, TASKBAR_RADIUS);

    // Стеклянный градиент
    LinearGradientBrush barBg(Rect(barX, barY, barW, barH),
        Color(180, 15, 15, 28), Color(160, 8, 8, 18), LinearGradientModeVertical);
    g.FillPath(&barBg, &barPath);

    // Тонкая верхняя неоновая линия
    Pen topLine(Color(120, 0, 255, 200), 1.0f);
    g.DrawLine(&topLine, barX + TASKBAR_RADIUS, barY, barX + barW - TASKBAR_RADIUS, barY);

    // Граница
    DrawRoundRect(g, barX, barY, barW, barH, TASKBAR_RADIUS, Color(35, 255, 255, 255), 1.0f);

    // Логотип VORTEX слева в панели
    float logoRot = (g_tick % 6000) / 6000.0f * 360.0f;
    DrawVortexLogo(g, barX + 28, barY + barH / 2, 14, logoRot);

    // Иконки по центру
    int iconsStartX = barX + 56;
    int iconY = barY + (barH - TASKBAR_ICON_SIZE) / 2;

    for (int i = 0; i < iconCount; i++) {
        int ix = iconsStartX + i * (TASKBAR_ICON_SIZE + TASKBAR_ICON_SPACING);

        // Плавная анимация масштаба
        float target = (g_hoveredTaskbarIcon == i) ? 1.18f : 1.0f;
        g_taskbarIconScale[i] += (target - g_taskbarIconScale[i]) * 0.25f;

        bool hov = (g_hoveredTaskbarIcon == i);
        DrawTaskbarIcon(g, ix, iconY, TASKBAR_ICON_SIZE, g_taskApps[i], hov, g_taskbarIconScale[i]);

        // Индикатор запущенного приложения - неоновая точка
        if (g_taskApps[i].running) {
            int dotX = ix + TASKBAR_ICON_SIZE / 2;
            int dotY = barY + barH - 6;

            // Свечение
            SolidBrush glow(Color(60, g_taskApps[i].accentColor.GetR(),
                g_taskApps[i].accentColor.GetG(), g_taskApps[i].accentColor.GetB()));
            g.FillEllipse(&glow, dotX - 5, dotY - 2, 10, 6);

            SolidBrush dot(g_taskApps[i].accentColor);
            g.FillEllipse(&dot, dotX - 2, dotY - 1, 4, 3);
        }

        SetRect(&g_taskApps[i].bounds, ix, iconY, ix + TASKBAR_ICON_SIZE, iconY + TASKBAR_ICON_SIZE);
    }

    // Часы справа в панели
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

    // Дата под часами
    wchar_t dateStr[32];
    wcsftime(dateStr, 32, L"%d.%m", &ti);
    Font dateFont(&ff, 9.0f, FontStyleRegular, UnitPixel);
    SolidBrush dateBr(VX::TextMuted);
    RectF dateRc((REAL)(barX + barW - 70), (REAL)(barY + 20), 60.0f, 20.0f);
    g.DrawString(dateStr, -1, &dateFont, dateRc, &rfmt, &dateBr);
}

// ============================================================================
//  ПЛАВАЮЩАЯ СТАТУС-ПАНЕЛЬ (ПРАВЫЙ ВЕРХНИЙ УГОЛ)
// ============================================================================
void DrawStatusPanel(Graphics& g, int sw) {
    int panelW = 280;
    int panelH = STATUS_PANEL_H;
    int panelX = sw - panelW - 12;
    int panelY = 10;

    // Фон
    FillRoundRectSolid(g, panelX, panelY, panelW, panelH, STATUS_PANEL_RADIUS,
                        Color(180, 12, 12, 22));
    DrawRoundRect(g, panelX, panelY, panelW, panelH, STATUS_PANEL_RADIUS,
                  Color(30, 0, 255, 200), 1.0f);

    FontFamily ff(L"Segoe UI");
    Font font(&ff, 11.0f, FontStyleRegular, UnitPixel);
    Font fontBold(&ff, 11.0f, FontStyleBold, UnitPixel);

    int cx = panelX + 14;
    int cy = panelY + panelH / 2;

    // WiFi индикатор
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

    // Разделитель
    Pen sepPen(Color(40, 255, 255, 255), 1.0f);
    g.DrawLine(&sepPen, cx, panelY + 8, cx, panelY + panelH - 8);
    cx += 10;

    // Батарея
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);

    int battPct = 85; // Симуляция
    wchar_t battStr[16];
    swprintf_s(battStr, L"%d%%", battPct);

    Color battColor = battPct > 50 ? VX::Green : (battPct > 20 ? VX::Yellow : VX::Red);
    SolidBrush battBr(battColor);

    // Иконка батареи
    Pen battPen(battColor, 1.5f);
    g.DrawRectangle(&battPen, cx, cy - 5, 18, 10);
    g.FillRectangle(&battBr, cx + 2, cy - 3, (int)(14.0f * battPct / 100.0f), 6);
    SolidBrush battTip(battColor);
    g.FillRectangle(&battTip, cx + 18, cy - 2, 3, 4);
    cx += 26;

    RectF battRc((REAL)cx, (REAL)panelY, 30.0f, (REAL)panelH);
    g.DrawString(battStr, -1, &font, battRc, &lfmt, &dimBr);
    cx += 36;

    // Разделитель
    g.DrawLine(&sepPen, cx, panelY + 8, cx, panelY + panelH - 8);
    cx += 10;

    // RAM
    int ramPct = (int)mem.dwMemoryLoad;
    wchar_t ramStr[32];
    swprintf_s(ramStr, L"RAM %d%%", ramPct);

    Color ramColor = ramPct > 80 ? VX::Red : (ramPct > 50 ? VX::Yellow : VX::Neon);
    SolidBrush ramBr(ramColor);

    g.FillEllipse(&ramBr, cx, cy - 4, 8, 8);
    cx += 14;

    RectF ramRc((REAL)cx, (REAL)panelY, 60.0f, (REAL)panelH);
    g.DrawString(ramStr, -1, &font, ramRc, &lfmt, &dimBr);
    cx += 65;

    // Музыка индикатор
    if (g_musicPlaying) {
        // Анимированные эквалайзер-полоски
        for (int i = 0; i < 4; i++) {
            float h = 4.0f + sinf(g_tick / 200.0f + i * 1.5f) * 6.0f;
            SolidBrush eqBr(VX::Magenta);
            g.FillRectangle(&eqBr, cx + i * 5, (REAL)(cy + 5) - h, 3.0f, h);
        }
    }
}

// ============================================================================
//  ВИДЖЕТЫ - ЛЕВАЯ ПАНЕЛЬ С GLASSMORPHISM
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

    // Время
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

    // Система
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);

    int cpuSim = 25 + (g_tick / 800) % 30;
    swprintf_s(buf, L"CPU  %d%%\nRAM  %d%%\nGPU  45%%\nDISK 62%%", cpuSim, (int)mem.dwMemoryLoad);
    w.title = L"SYSTEM";
    w.content = buf;
    w.height = 150;
    w.accent = VX::Purple;
    g_widgets.push_back(w);

    // Погода
    w.title = L"WEATHER";
    w.content = L"22°C  Sunny\nMoscow, RU\nWind: 3 m/s";
    w.height = 120;
    w.accent = VX::Yellow;
    g_widgets.push_back(w);

    // Музыка
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

        // Фон с glassmorphism
        Color bgColor = hov ? VX::CardBgHover : VX::CardBg;
        bgColor = Color((BYTE)(bgColor.GetA() * alpha), bgColor.GetR(), bgColor.GetG(), bgColor.GetB());
        FillRoundRectSolid(g, x, y, width, height, 14, bgColor);

        // Граница
        Color borderColor = hov ?
            Color((BYTE)(120 * alpha), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()) :
            Color((BYTE)(30 * alpha), 255, 255, 255);
        DrawRoundRect(g, x, y, width, height, 14, borderColor, hov ? 1.5f : 0.5f);

        // Неон при ховере
        if (hov) {
            DrawNeonGlow(g, x, y, width, height, 14,
                Color((BYTE)(40 * alpha), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()), 2);
        }

        // Акцентная полоска слева
        SolidBrush accentBr(Color((BYTE)(200 * alpha), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));
        FillRoundRectSolid(g, x + 1, y + 12, 3, height - 24, 2,
            Color((BYTE)(200 * alpha), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));

        // Заголовок
        FontFamily ff(L"Segoe UI");
        Font titleFont(&ff, 10.0f, FontStyleBold, UnitPixel);
        SolidBrush titleBr(Color((BYTE)(180 * alpha), w.accent.GetR(), w.accent.GetG(), w.accent.GetB()));

        StringFormat fmt;
        fmt.SetAlignment(StringAlignmentNear);
        fmt.SetLineAlignment(StringAlignmentNear);

        RectF titleRc((REAL)(x + 16), (REAL)(y + 12), (REAL)(width - 32), 14.0f);
        g.DrawString(w.title.c_str(), -1, &titleFont, titleRc, &fmt, &titleBr);

        // Контент
        Font contentFont(&ff, 13.0f, FontStyleRegular, UnitPixel);
        SolidBrush contentBr(Color((BYTE)(240 * alpha), 240, 240, 255));

        // Парсим строки
        std::wistringstream ss(w.content);
        std::wstring line;
        float ly = (REAL)(y + 32);

        // Первая строка крупнее для часов
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

        // Прогресс-бары для SYSTEM
        if (w.title == L"SYSTEM") {
            MEMORYSTATUSEX mem2;
            mem2.dwLength = sizeof(mem2);
            GlobalMemoryStatusEx(&mem2);
            int vals[] = { 25 + (int)((g_tick / 800) % 30), (int)mem2.dwMemoryLoad, 45, 62 };
            Color colors[] = { VX::Neon, VX::Purple, VX::Green, VX::Orange };

            for (int j = 0; j < 4; j++) {
                int barX = x + 60;
                int barY2 = y + 34 + j * 28;
                int barW = width - 80;
                int barH2 = 6;

                // Фон бара
                FillRoundRectSolid(g, barX, barY2, barW, barH2, 3, Color((BYTE)(40 * alpha), 255, 255, 255));

                // Заполнение
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
//  СТАРТОВОЕ МЕНЮ - ЦЕНТРИРОВАННОЕ
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

    // Затемнение
    SolidBrush overlay(Color((BYTE)(140 * a), 0, 0, 0));
    g.FillRectangle(&overlay, 0, 0, sw, sh);

    int menuW = START_MENU_W;
    int menuH = START_MENU_H;
    int menuX = (sw - menuW) / 2;
    // Появляется снизу
    int targetY = (sh - menuH) / 2 - 20;
    int menuY = (int)(targetY + 40 * (1.0f - a));

    // Фон
    Color bgColor(Color((BYTE)(210 * a), 14, 14, 24));
    FillRoundRectSolid(g, menuX, menuY, menuW, menuH, 20, bgColor);

    // Неоновая граница
    DrawRoundRect(g, menuX, menuY, menuW, menuH, 20,
        Color((BYTE)(80 * a), 0, 255, 200), 1.5f);

    // Верхняя неоновая линия
    Pen topNeon(Color((BYTE)(150 * a), 0, 255, 200), 2.0f);
    g.DrawLine(&topNeon, menuX + 20, menuY, menuX + menuW - 20, menuY);

    // VORTEX логотип и текст
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

    // Поиск
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

    // Лупа
    Pen searchPen(Color((BYTE)(150 * a), 0, 255, 200), 2.0f);
    g.DrawEllipse(&searchPen, searchX + 12, searchY + 10, 14, 14);
    g.DrawLine(&searchPen, searchX + 24, searchY + 24, searchX + 28, searchY + 28);

    // Секция "Pinned"
    Font sectionFont(&ff, 11.0f, FontStyleBold, UnitPixel);
    SolidBrush sectionBr(Color((BYTE)(150 * a), 0, 255, 200));
    RectF pinnedRc((REAL)(menuX + 28), (REAL)(menuY + 108), 100.0f, 16.0f);
    g.DrawString(L"PINNED", -1, &sectionFont, pinnedRc, &lfmt, &sectionBr);

    // Сетка иконок
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

        // Фон ячейки
        if (hov) {
            FillRoundRectSolid(g, ix + 4, iy + 2, cellW - 8, cellH - 4, 10,
                Color((BYTE)(60 * a), g_startApps[i].color.GetR(),
                      g_startApps[i].color.GetG(), g_startApps[i].color.GetB()));
            DrawNeonGlow(g, ix + 4, iy + 2, cellW - 8, cellH - 4, 10,
                Color((BYTE)(30 * a), g_startApps[i].color.GetR(),
                      g_startApps[i].color.GetG(), g_startApps[i].color.GetB()), 2);
        }

        // Иконка (квадрат с цветом)
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

        // Символ
        Font icoFont(&ff, 16.0f, FontStyleBold, UnitPixel);
        SolidBrush icoBr(Color((BYTE)(230 * a), g_startApps[i].color.GetR(),
            g_startApps[i].color.GetG(), g_startApps[i].color.GetB()));

        StringFormat cfmt;
        cfmt.SetAlignment(StringAlignmentCenter);
        cfmt.SetLineAlignment(StringAlignmentCenter);

        RectF icoRc((REAL)icoX, (REAL)icoY, (REAL)icoSize, (REAL)icoSize);
        g.DrawString(g_startApps[i].icon, -1, &icoFont, icoRc, &cfmt, &icoBr);

        // Название
        Font nameFont(&ff, 10.0f, FontStyleRegular, UnitPixel);
        SolidBrush nameBr(Color((BYTE)(200 * a), 220, 220, 240));
        RectF nameRc((REAL)ix, (REAL)(iy + 48), (REAL)cellW, 20.0f);
        g.DrawString(g_startApps[i].name.c_str(), -1, &nameFont, nameRc, &cfmt, &nameBr);
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

        // Фон
        FillRoundRectSolid(g, nx, ny, NOTIF_W, NOTIF_H, 14,
            Color((BYTE)(200 * a), 18, 18, 30));

        // Неоновая граница
        DrawRoundRect(g, nx, ny, NOTIF_W, NOTIF_H, 14,
            Color((BYTE)(60 * a), 0, 255, 200), 1.0f);

        // Акцентная полоска слева
        FillRoundRectSolid(g, nx + 1, ny + 14, 3, NOTIF_H - 28, 2,
            Color((BYTE)(200 * a), 0, 255, 200));

        // Логотип
        float rot = (g_tick % 6000) / 6000.0f * 360.0f;
        // Маленький логотип
        SolidBrush logoBg(Color((BYTE)(150 * a), 0, 255, 200));
        g.FillEllipse(&logoBg, nx + 16, ny + NOTIF_H / 2 - 14, 28, 28);
        SolidBrush logoDot(Color((BYTE)(255 * a), 255, 255, 255));
        g.FillEllipse(&logoDot, nx + 27, ny + NOTIF_H / 2 - 3, 6, 6);

        // Текст
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

        RectF msgRc((REAL)(nx + 54), (REAL)(ny + 36), (REAL)(NOTIF_W - 70), 30.0f);
        g.DrawString(n.message.c_str(), -1, &msgFont, msgRc, &fmt, &msgBr);

        // Время
        DWORD elapsed = (GetTickCount() - n.time) / 1000;
        wchar_t timeStr[32];
        if (elapsed < 60) swprintf_s(timeStr, L"%ds ago", elapsed);
        else swprintf_s(timeStr, L"%dm ago", elapsed / 60);

        Font timeFont(&ff, 9.0f, FontStyleRegular, UnitPixel);
        SolidBrush timeBr(Color((BYTE)(100 * a), 120, 130, 160));

        StringFormat rfmt;
        rfmt.SetAlignment(StringAlignmentFar);
        rfmt.SetLineAlignment(StringAlignmentNear);

        RectF timeRc((REAL)(nx + NOTIF_W - 80), (REAL)(ny + 16), 64.0f, 14.0f);
        g.DrawString(timeStr, -1, &timeFont, timeRc, &rfmt, &timeBr);
    }
}

// ============================================================================
//  ЧАСТИЦЫ ФОНА
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
        // Красивый градиент как фолбек
        LinearGradientBrush bg(Point(0, 0), Point(sw, sh),
            Color(255, 8, 8, 20), Color(255, 20, 10, 40));
        g.FillRectangle(&bg, 0, 0, sw, sh);

        // Градиентные круги
        for (int i = 0; i < 6; i++) {
            int cx = 150 + i * 280;
            int cy = 100 + (i % 3) * 200;
            int cr = 80 + i * 40;
            SolidBrush cb(Color(8, 0, 200 + i * 10, 200));
            g.FillEllipse(&cb, cx - cr, cy - cr, cr * 2, cr * 2);
        }
    }

    // Тёмный оверлей для лучшей читаемости UI
    SolidBrush dimOverlay(Color(30, 0, 0, 0));
    g.FillRectangle(&dimOverlay, 0, 0, sw, sh);
}

// ============================================================================
//  ГОРЯЧИЕ КЛАВИШИ ПОДСКАЗКА (внизу слева)
// ============================================================================
void DrawHotkeyHints(Graphics& g, int sh) {
    FontFamily ff(L"Segoe UI");
    Font font(&ff, 9.0f, FontStyleRegular, UnitPixel);
    SolidBrush br(Color(60, 200, 200, 220));

    StringFormat fmt;
    fmt.SetAlignment(StringAlignmentNear);
    fmt.SetLineAlignment(StringAlignmentFar);

    RectF rc(12.0f, (REAL)(sh - 28), 400.0f, 16.0f);
    g.DrawString(L"ESC exit  |  SPACE menu  |  M music  |  W widgets  |  N notify", -1, &font, rc, &fmt, &br);
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

    // 3. Виджеты (слева)
    DrawWidgets(g, sh);

    // 4. Статус-панель (правый верх)
    DrawStatusPanel(g, sw);

    // 5. Уведомления (справа)
    DrawNotifications(g, sw);

    // 6. Панель задач (низ, по центру)
    DrawTaskbar(g, sw, sh);

    // 7. Start Menu (поверх всего)
    DrawStartMenu(g, sw, sh);

    // 8. Подсказки
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
        InitParticles();
        UpdateWidgets();
        CreateThread(NULL, 0, WallpaperThread, NULL, 0, NULL);
        SetTimer(hWnd, 1, 1000, NULL);    // Виджеты
        SetTimer(hWnd, 2, 16, NULL);      // Анимация 60fps
        SetTimer(hWnd, 3, 45000, NULL);   // Периодические уведомления
        PushNotification(L"Welcome to VORTEX", L"Your cyberpunk desktop is ready!");
        break;

    case WM_TIMER:
        g_tick = GetTickCount();
        if (wParam == 1) {
            UpdateWidgets();
        }
        else if (wParam == 2) {
            // Анимация Start Menu
            float targetStart = g_startMenuOpen ? 1.0f : 0.0f;
            g_startMenuAnim += (targetStart - g_startMenuAnim) * 0.15f;
            if (fabs(g_startMenuAnim - targetStart) < 0.005f) g_startMenuAnim = targetStart;

            // Анимация виджетов
            float targetW = g_widgetsVisible ? 1.0f : 0.0f;
            g_widgetsAnim += (targetW - g_widgetsAnim) * 0.12f;
            if (fabs(g_widgetsAnim - targetW) < 0.005f) g_widgetsAnim = targetW;

            // Частицы
            UpdateParticles();

            // Уведомления
            for (auto& n : g_notifs) {
                if (!n.alive) continue;
                if (n.alpha < 1.0f) n.alpha = min(1.0f, n.alpha + 0.06f);
                if (n.offsetY < 0) n.offsetY = min(0.0f, n.offsetY + 3.0f);
                if (GetTickCount() - n.time > 6000) {
                    n.alpha -= 0.03f;
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

        // Taskbar hover
        g_hoveredTaskbarIcon = -1;
        for (int i = 0; i < (int)g_taskApps.size(); i++) {
            // Расширенная зона для лёгкого наведения
            RECT r = g_taskApps[i].bounds;
            InflateRect(&r, 4, 8);
            if (PtInRect(&r, pt)) {
                g_hoveredTaskbarIcon = i;
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

        // Start menu click
        if (g_startMenuOpen && g_startMenuAnim > 0.5f) {
            if (g_hoveredStartIcon >= 0 && g_hoveredStartIcon < (int)g_startApps.size()) {
                ShellExecute(NULL, L"open", g_startApps[g_hoveredStartIcon].exec.c_str(),
                    NULL, NULL, SW_SHOW);
                g_startMenuOpen = false;
            } else {
                // Клик вне меню - закрыть
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

        // Taskbar click
        for (int i = 0; i < (int)g_taskApps.size(); i++) {
            if (PtInRect(&g_taskApps[i].bounds, pt)) {
                ShellExecute(NULL, L"open", g_taskApps[i].exec.c_str(), NULL, NULL, SW_SHOW);
                break;
            }
        }

        // Logo click - open start menu
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

    case WM_RBUTTONDOWN:
        if (g_startMenuOpen) {
            g_startMenuOpen = false;
        }
        break;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            if (g_startMenuOpen) g_startMenuOpen = false;
            else PostQuitMessage(0);
        }
        else if (wParam == VK_SPACE) {
            g_startMenuOpen = !g_startMenuOpen;
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
    wc.lpszClassName = L"VORTEX_Desktop_v2";
    RegisterClassExW(&wc);

    int sw = ScreenW(), sh = ScreenH();

    g_hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"VORTEX_Desktop_v2", L"VORTEX Desktop",
        WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN,
        0, 0, sw, sh,
        NULL, NULL, hInst, NULL);

    // Скрываем стандартный таскбар
    HWND hTaskbar = FindWindow(L"Shell_TrayWnd", NULL);
    if (hTaskbar) ShowWindow(hTaskbar, SW_HIDE);

    ShowWindow(g_hWnd, nShow);
    UpdateWindow(g_hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Восстанавливаем таскбар
    if (hTaskbar) ShowWindow(hTaskbar, SW_SHOW);

    GdiplusShutdown(g_gdiplusToken);
    return (int)msg.wParam;
}

int main() {
    return wWinMain(GetModuleHandle(NULL), NULL, NULL, SW_SHOW);
}
