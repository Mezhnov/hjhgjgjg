/*
 * VORTEX Desktop Environment v5.3 - Chrome-Grade Premium UI
 * Improved Start Menu with Mica material, Power Menu, Search filtering,
 * Improved Context Menus, Notifications, and Taskbar refinements.
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
//  FONT STYLE FIX
// ============================================================================
#define FONT_SEMIBOLD FontStyleBold

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

static float pOutCubic(float t) {
    float f = t - 1.0f;
    return f * f * f + 1.0f;
}

static float pOutQuint(float t) {
    float f = t - 1.0f;
    return f * f * f * f * f + 1.0f;
}

static float pOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(t - 1.0f, 3) + c1 * powf(t - 1.0f, 2);
}

static float pOutElastic(float t) {
    if (t == 0.0f || t == 1.0f) return t;
    return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * (2.0f * (float)M_PI) / 3.0f) + 1.0f;
}

static float pOutSpring(float t) {
    return 1.0f - powf(cosf(t * 4.5f * (float)M_PI), 3) * expf(-t * 6.0f);
}

static float pInCubic(float t) {
    return t * t * t;
}

static float pInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3) / 2.0f;
}

static float pLerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static float pClamp(float v, float lo, float hi) {
    return max(lo, min(hi, v));
}

static float pSmoothstep(float lo, float hi, float t) {
    float x = pClamp((t - lo) / (hi - lo), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

static int pLerpInt(int a, int b, float t) {
    return (int)(a + (b - a) * t);
}

static Gdiplus::Color pLerpColor(Gdiplus::Color a, Gdiplus::Color b, float t) {
    return Gdiplus::Color(
        (BYTE)pLerp(a.GetA(), b.GetA(), t),
        (BYTE)pLerp(a.GetR(), b.GetR(), t),
        (BYTE)pLerp(a.GetG(), b.GetG(), t),
        (BYTE)pLerp(a.GetB(), b.GetB(), t)
    );
}

// ============================================================================
//  GLOBAL STATE
// ============================================================================

static HWND g_hWnd = NULL;
static int g_width = 1920;
static int g_height = 1080;
static ULONG_PTR g_gdiplusToken = 0;

// Mouse
static POINT g_mouse = {0, 0};
static bool g_mouseDown = false;
static POINT g_mouseDownPos = {0, 0};
static bool g_mouseDragged = false;

// Taskbar
static int g_taskbarHeight = 48;
static SpringValue g_taskbarSlide(0.0f, 220.0f, 18.0f);

// Start Menu
static bool g_startMenuOpen = false;
static SpringValue g_startMenuAnim(0.0f, 200.0f, 16.0f);
static SpringValue g_startMenuBgAnim(0.0f, 160.0f, 14.0f);
static float g_startMenuOpenTime = 0.0f;
static int g_hoveredStartItem = -1;
static int g_hoveredRecommendedItem = -1;
static int g_hoveredPowerBtn = 0;

// Search (v5.3)
static std::wstring g_searchQuery;
static bool g_searchActive = false;
static SpringValue g_searchGlowAnim(0.0f, 260.0f, 20.0f);

// Power Menu (v5.3)
static bool g_powerMenuOpen = false;
static SpringValue g_powerMenuAnim(0.0f, 350.0f, 22.0f);
static int g_hoveredPowerItem = -1;

// Context Menu
static bool g_contextMenuOpen = false;
static POINT g_contextMenuPos = {0, 0};
static SpringValue g_contextMenuAnim(0.0f, 240.0f, 18.0f);
static int g_hoveredContextItem = -1;
static float g_contextMenuOpenTime = 0.0f;

// Notifications
static struct Notification {
    std::wstring title;
    std::wstring body;
    std::wstring icon;
    float alpha;
    float slideX;
    float targetAlpha;
    float targetSlideX;
    float openTime;
    int uid;
    bool closing;
    bool hovered;
    SpringValue slideAnim;
    SpringValue fadeAnim;
} g_notifications[5];
static int g_notificationCount = 0;
static int g_notificationUidCounter = 0;
static int g_hoveredNotifCloseBtn = -1;

// Widgets
static bool g_widgetsOpen = false;
static SpringValue g_widgetsAnim(0.0f, 180.0f, 14.0f);

// Music Widget
static bool g_musicOpen = false;
static SpringValue g_musicAnim(0.0f, 200.0f, 16.0f);
static float g_musicProgress = 0.35f;
static bool g_musicPlaying = true;
static SpringValue g_musicProgressAnim(0.35f, 60.0f, 8.0f);
static std::wstring g_musicTitle = L"Midnight City";
static std::wstring g_musicArtist = L"M83";

// WiFi Panel
static bool g_wifiOpen = false;
static SpringValue g_wifiAnim(0.0f, 220.0f, 18.0f);
static int g_hoveredWifiNetwork = -1;

// Desktop Icons
static struct DesktopIcon {
    std::wstring label;
    std::wstring icon;
    int x, y;
    bool hovered;
    SpringValue hoverAnim;
} g_desktopIcons[10];

// Clock pulse
static float g_clockPulse = 0.0f;

// WiFi signal animation
static float g_wifiSignalPulse = 0.0f;

// Frame time
static float g_deltaTime = 0.016f;
static float g_totalTime = 0.0f;

// Drag select
static bool g_dragSelecting = false;
static RECT g_dragSelectRect = {0, 0, 0, 0};

// Hovered taskbar items
static int g_hoveredTaskbarItem = -1;
static int g_hoveredTrayItem = -1;

// Show desktop strip
static bool g_showDesktopHovered = false;

// ============================================================================
//  POWER MENU ITEM STRUCT (v5.3)
// ============================================================================

struct PowerMenuItem {
    std::wstring label;
    std::wstring icon;
    int action; // 0=lock, 1=signout, 2=sleep, 3=restart, 4=shutdown
    bool separator; // draw separator below this item
};

static PowerMenuItem g_powerMenuItems[] = {
    { L"Lock",         L"\uE72E", 0, false },
    { L"Sign out",     L"\uE748", 1, true  },
    { L"Sleep",        L"\uE703", 2, false },
    { L"Restart",      L"\uE777", 3, false },
    { L"Shutdown",     L"\uE7A8", 4, false },
};
static const int g_powerMenuItemCount = 5;

// ============================================================================
//  CONTEXT MENU ITEMS
// ============================================================================

static struct ContextMenuItem {
    std::wstring label;
    std::wstring icon;
    bool separator;
} g_contextMenuItems[] = {
    { L"View",           L"\uE890", false },
    { L"Sort by",        L"\uE8CB", false },
    { L"Refresh",        L"\uE72C", false },
    { L"",               L"",       true  },
    { L"New",            L"\uE710", false },
    { L"Open in Terminal", L"\uE756", false },
    { L"",               L"",       true  },
    { L"Display settings", L"\uE770", false },
    { L"Personalize",    L"\uE771", false },
    { L"",               L"",       true  },
    { L"Properties",     L"\uE946", false },
};
static const int g_contextMenuItemCount = 11;

// ============================================================================
//  PINNED APPS (for start menu)
// ============================================================================

static struct PinnedApp {
    std::wstring name;
    std::wstring icon;
    std::wstring type; // for search filtering
} g_pinnedApps[] = {
    { L"Edge",          L"\uE773", L"browser" },
    { L"Explorer",      L"\uE8B7", L"file" },
    { L"Settings",      L"\uE713", L"system" },
    { L"Terminal",      L"\uE756", L"app" },
    { L"VS Code",       L"\uE70F", L"editor" },
    { L"Spotify",       L"\uE7C1", L"music" },
    { L"Photos",        L"\uE8B9", L"media" },
    { L"Store",         L"\uE719", L"app" },
    { L"Mail",          L"\uE715", L"email" },
    { L"Calendar",      L"\uE787", L"app" },
    { L"Calculator",    L"\uE8EF", L"app" },
    { L"Snipping Tool", L"\uE793", L"tool" },
};
static const int g_pinnedAppCount = 12;

// ============================================================================
//  RECOMMENDED ITEMS
// ============================================================================

static struct RecommendedItem {
    std::wstring name;
    std::wstring detail;
    std::wstring icon;
    std::wstring fileType; // for file type icon coloring
} g_recommendedItems[] = {
    { L"Project Report.docx",  L"Yesterday",  L"\uE7C3", L"doc" },
    { L"Budget 2024.xlsx",     L"2 hours ago", L"\uE7C4", L"xls" },
    { L"Presentation.pptx",    L"Today",      L"\uE7C5", L"ppt" },
    { L"Screenshot_001.png",   L"Today",      L"\uE8B9", L"img" },
    { L"Notes.txt",            L"Last week",  L"\uE8A5", L"txt" },
    { L"Meeting Recording.mp4",L"3 days ago", L"\uE714", L"vid" },
};
static const int g_recommendedItemCount = 6;

// ============================================================================
//  WIFI NETWORKS
// ============================================================================

static struct WifiNetwork {
    std::wstring name;
    std::wstring security;
    int signal; // 1-4
    bool connected;
} g_wifiNetworks[] = {
    { L"VORTEX-5G",      L"WPA3", 4, true  },
    { L"HomeNetwork",    L"WPA2", 3, false },
    { L"CoffeeShop_Free", L"Open", 2, false },
    { L"Neighbor_Net",   L"WPA2", 2, false },
    { L"Guest_Network",  L"Open", 1, false },
};
static const int g_wifiNetworkCount = 5;

// ============================================================================
//  HELPER: INIT DESKTOP ICONS
// ============================================================================

static void InitDesktopIcons() {
    const wchar_t* labels[] = { L"This PC", L"Recycle Bin", L"Documents", L"Pictures",
                                 L"Downloads", L"Music", L"Videos", L"Control Panel",
                                 L"Network", L"Edge" };
    const wchar_t* icons[] = { L"\uE8A5", L"\uE74D", L"\uE8B7", L"\uEB9F",
                                L"\uE896", L"\uEC4F", L"\uE714", L"\uE70F",
                                L"\uE8C4", L"\uE773" };
    int cols = 1, row = 0;
    for (int i = 0; i < 10; i++) {
        g_desktopIcons[i].label = labels[i];
        g_desktopIcons[i].icon = icons[i];
        g_desktopIcons[i].x = 20;
        g_desktopIcons[i].y = 20 + row * 90;
        g_desktopIcons[i].hovered = false;
        g_desktopIcons[i].hoverAnim = SpringValue(0.0f, 220.0f, 14.0f);
        row++;
        if (row >= 8) { row = 0; cols++; g_desktopIcons[i].x = 20 + (cols - 1) * 90; }
    }
}

// ============================================================================
//  HELPER: ADD NOTIFICATION
// ============================================================================

static void AddNotification(const std::wstring& title, const std::wstring& body,
                            const std::wstring& icon) {
    if (g_notificationCount >= 5) {
        // Close oldest
        g_notifications[0].closing = true;
        g_notifications[0].targetAlpha = 0;
        g_notifications[0].targetSlideX = 400;
        memmove(&g_notifications[0], &g_notifications[1],
                sizeof(Notification) * (g_notificationCount - 1));
        g_notificationCount--;
    }
    int idx = g_notificationCount;
    g_notifications[idx].title = title;
    g_notifications[idx].body = body;
    g_notifications[idx].icon = icon;
    g_notifications[idx].alpha = 0;
    g_notifications[idx].slideX = 400;
    g_notifications[idx].targetAlpha = 1.0f;
    g_notifications[idx].targetSlideX = 0;
    g_notifications[idx].openTime = g_totalTime;
    g_notifications[idx].uid = ++g_notificationUidCounter;
    g_notifications[idx].closing = false;
    g_notifications[idx].hovered = false;
    g_notifications[idx].slideAnim = SpringValue(400.0f, 200.0f, 16.0f);
    g_notifications[idx].fadeAnim = SpringValue(0.0f, 200.0f, 14.0f);
    g_notifications[idx].slideAnim.SetTarget(0);
    g_notifications[idx].fadeAnim.SetTarget(1.0f);
    g_notificationCount++;
}

// ============================================================================
//  HELPER: CLOSE NOTIFICATION
// ============================================================================

static void CloseNotification(int idx) {
    if (idx >= 0 && idx < g_notificationCount) {
        g_notifications[idx].closing = true;
        g_notifications[idx].slideAnim.SetTarget(400);
        g_notifications[idx].fadeAnim.SetTarget(0);
    }
}

// ============================================================================
//  HELPER: UPDATE NOTIFICATIONS
// ============================================================================

static void UpdateNotifications() {
    for (int i = g_notificationCount - 1; i >= 0; i--) {
        g_notifications[i].slideAnim.Update(g_deltaTime);
        g_notifications[i].fadeAnim.Update(g_deltaTime);
        g_notifications[i].slideX = g_notifications[i].slideAnim.current;
        g_notifications[i].alpha = g_notifications[i].fadeAnim.current;
        if (g_notifications[i].closing &&
            g_notifications[i].slideAnim.IsSettled() &&
            g_notifications[i].fadeAnim.IsSettled()) {
            memmove(&g_notifications[i], &g_notifications[i + 1],
                    sizeof(Notification) * (g_notificationCount - i - 1));
            g_notificationCount--;
        }
    }
}

// ============================================================================
//  HELPER: DRAW ROUNDED RECT (GDI+)
// ============================================================================

static void DrawRoundRect(Graphics* g, Pen* pen, float x, float y, float w, float h,
                           float r) {
    r = min(r, w / 2.0f, h / 2.0f);
    GraphicsPath path;
    path.AddArc(x, y, r * 2, r * 2, 180, 90);
    path.AddArc(x + w - r * 2, y, r * 2, r * 2, 270, 90);
    path.AddArc(x + w - r * 2, y + h - r * 2, r * 2, r * 2, 0, 90);
    path.AddArc(x, y + h - r * 2, r * 2, r * 2, 90, 90);
    path.CloseFigure();
    g->DrawPath(pen, &path);
}

static void FillRoundRect(Graphics* g, Brush* brush, float x, float y, float w, float h,
                           float r) {
    r = min(r, w / 2.0f, h / 2.0f);
    GraphicsPath path;
    path.AddArc(x, y, r * 2, r * 2, 180, 90);
    path.AddArc(x + w - r * 2, y, r * 2, r * 2, 270, 90);
    path.AddArc(x + w - r * 2, y + h - r * 2, r * 2, r * 2, 0, 90);
    path.AddArc(x, y + h - r * 2, r * 2, r * 2, 90, 90);
    path.CloseFigure();
    g->FillPath(brush, &path);
}

// ============================================================================
//  HELPER: FILL ROUNDED RECT WITH GRADIENT
// ============================================================================

static void FillRoundRectGradient(Graphics* g, float x, float y, float w, float h,
                                   float r, Color c1, Color c2, bool vertical) {
    r = min(r, w / 2.0f, h / 2.0f);
    GraphicsPath path;
    path.AddArc(x, y, r * 2, r * 2, 180, 90);
    path.AddArc(x + w - r * 2, y, r * 2, r * 2, 270, 90);
    path.AddArc(x + w - r * 2, y + h - r * 2, r * 2, r * 2, 0, 90);
    path.AddArc(x, y + h - r * 2, r * 2, r * 2, 90, 90);
    path.CloseFigure();
    LinearGradientBrush brush(
        vertical ? PointF(x, y) : PointF(x, y),
        vertical ? PointF(x, y + h) : PointF(x + w, y),
        c1, c2);
    g->FillPath(&brush, &path);
}

// ============================================================================
//  HELPER: CREATE CLIP ROUNDED RECT
// ============================================================================

static GraphicsPath CreateRoundRectPath(float x, float y, float w, float h, float r) {
    r = min(r, w / 2.0f, h / 2.0f);
    GraphicsPath path;
    path.AddArc(x, y, r * 2, r * 2, 180, 90);
    path.AddArc(x + w - r * 2, y, r * 2, r * 2, 270, 90);
    path.AddArc(x + w - r * 2, y + h - r * 2, r * 2, r * 2, 0, 90);
    path.AddArc(x, y + h - r * 2, r * 2, r * 2, 90, 90);
    path.CloseFigure();
    return path;
}

// ============================================================================
//  HELPER: DRAW TEXT WITH FONT FALLBACK
// ============================================================================

static void DrawTextString(Graphics* g, const std::wstring& text, const Font& font,
                            const Brush& brush, float x, float y,
                            StringAlignment align = StringAlignmentNear) {
    StringFormat fmt;
    fmt.SetAlignment(align);
    fmt.SetLineAlignment(StringAlignmentNear);
    fmt.SetTrimming(StringTrimmingEllipsisCharacter);
    RectF bounds(x, y, 10000.0f, 10000.0f);
    g->DrawString(text.c_str(), (INT)text.length(), &font, bounds, &fmt, &brush);
}

// ============================================================================
//  HELPER: MEASURE TEXT SIZE
// ============================================================================

static RectF MeasureText(Graphics* g, const std::wstring& text, const Font& font) {
    RectF bounds(0, 0, 10000.0f, 10000.0f);
    g->MeasureString(text.c_str(), (INT)text.length(), &font, bounds, &bounds);
    return bounds;
}

// ============================================================================
//  HELPER: DRAW SHADOW
// ============================================================================

static void DrawDropShadow(Graphics* g, float x, float y, float w, float h,
                            float r, float blur, float spread, int alpha) {
    // Multi-layered shadow for premium quality
    for (int layer = 0; layer < 3; layer++) {
        float layerBlur = blur + (float)layer * blur * 0.5f;
        float layerAlpha = alpha / (layer + 1);
        float offset = (float)layer * 2.0f;
        SolidBrush shadowBrush(Color((BYTE)layerAlpha, 0, 0, 0));
        FillRoundRect(g, &shadowBrush,
                      x - layerBlur + spread - offset,
                      y - layerBlur + spread - offset,
                      w + layerBlur * 2.0f + offset,
                      h + layerBlur * 2.0f + offset,
                      r + layerBlur);
    }
}

// ============================================================================
//  DRAW: DESKTOP BACKGROUND
// ============================================================================

static void DrawDesktop(Graphics* g) {
    // Premium gradient background
    LinearGradientBrush bgBrush(
        PointF(0, 0), PointF((float)g_width, (float)g_height),
        Color(15, 15, 30), Color(35, 15, 55));
    g->FillRectangle(&bgBrush, 0, 0, g_width, g_height);

    // Subtle radial glow in center
    float cx = (float)g_width * 0.5f;
    float cy = (float)g_height * 0.4f;
    float radius = (float)max(g_width, g_height) * 0.7f;
    GraphicsPath glowPath;
    glowPath.AddEllipse(cx - radius, cy - radius, radius * 2, radius * 2);
    PathGradientBrush glowBrush(&glowPath);
    glowBrush.SetCenterColor(Color(30, 20, 60));
    glowBrush.SetSurroundColor(Color(0, 0, 0, 0));
    glowBrush.SetCenterPoint(PointF(cx, cy));
    g->FillPath(&glowBrush, &glowPath);

    // Secondary glow - bottom right warm accent
    float cx2 = (float)g_width * 0.85f;
    float cy2 = (float)g_height * 0.8f;
    float radius2 = (float)max(g_width, g_height) * 0.45f;
    GraphicsPath glow2Path;
    glow2Path.AddEllipse(cx2 - radius2, cy2 - radius2, radius2 * 2, radius2 * 2);
    PathGradientBrush glow2Brush(&glow2Path);
    glow2Brush.SetCenterColor(Color(40, 15, 35, 80));
    glow2Brush.SetSurroundColor(Color(0, 0, 0, 0));
    glow2Brush.SetCenterPoint(PointF(cx2, cy2));
    g->FillPath(&glow2Brush, &glow2Path);

    // Subtle animated particle dots
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> distX(0.0f, (float)g_width);
    std::uniform_real_distribution<float> distY(0.0f, (float)g_height);
    std::uniform_real_distribution<float> distSize(0.5f, 2.0f);
    for (int i = 0; i < 60; i++) {
        float px = distX(rng);
        float py = distY(rng);
        float sz = distSize(rng);
        float twinkle = 0.3f + 0.7f * (float)(sin(g_totalTime * 0.8f + i * 1.3f) * 0.5f + 0.5f);
        int alpha = (int)(20 * twinkle);
        SolidBrush dotBrush(Color((BYTE)alpha, 180, 200, 255));
        g->FillEllipse(&dotBrush, px - sz, py - sz, sz * 2, sz * 2);
    }

    // Draw desktop icons
    Font iconFont(L"Segoe MDL2 Assets", 24, FontStyleRegular, UnitPixel);
    Font labelFont(L"Segoe UI", 12, FontStyleRegular, UnitPixel);
    float iconAreaHeight = (float)(g_height - g_taskbarHeight);

    for (int i = 0; i < 10; i++) {
        DesktopIcon& di = g_desktopIcons[i];
        di.hoverAnim.SetTarget(di.hovered ? 1.0f : 0.0f);
        di.hoverAnim.Update(g_deltaTime);
        float hv = di.hoverAnim.current;

        // Icon bounds
        float ix = (float)di.x;
        float iy = (float)di.y;
        float iw = 70.0f;
        float ih = 70.0f;

        if (iy + ih > iconAreaHeight) continue;

        // Hover background
        if (hv > 0.01f) {
            SolidBrush hoverBg(Color((BYTE)(25 * hv), 255, 255, 255));
            FillRoundRect(g, &hoverBg, ix - 5, iy - 2, iw + 10, ih + 4, 6);
        }

        // Icon
        int iconAlpha = (int)pLerp(200.0f, 255.0f, hv);
        SolidBrush iconBrush(Color((BYTE)iconAlpha, 230, 235, 250));
        StringFormat sf;
        sf.SetAlignment(StringAlignmentCenter);
        sf.SetLineAlignment(StringAlignmentCenter);
        RectF iconRect(ix, iy, iw, 40);
        g->DrawString(di.icon.c_str(), (INT)di.icon.length(), &iconFont, iconRect, &sf, &iconBrush);

        // Label
        int labelAlpha = (int)pLerp(210.0f, 255.0f, hv);
        SolidBrush labelBrush(Color((BYTE)labelAlpha, 230, 235, 250));
        StringFormat lf;
        lf.SetAlignment(StringAlignmentCenter);
        lf.SetLineAlignment(StringAlignmentNear);
        lf.SetTrimming(StringTrimmingEllipsisCharacter);
        RectF labelRect(ix - 10, iy + 42, iw + 20, 30);
        g->DrawString(di.label.c_str(), (INT)di.label.length(), &labelFont, labelRect, &lf, &labelBrush);
    }
}

// ============================================================================
//  DRAW: TASKBAR (Improved v5.3)
// ============================================================================

static void DrawTaskbar(Graphics* g) {
    g_taskbarSlide.SetTarget(1.0f);
    g_taskbarSlide.Update(g_deltaTime);
    float slide = g_taskbarSlide.current;
    if (slide < 0.01f) return;

    int tbH = g_taskbarHeight;
    float tbY = (float)(g_height - tbH);
    float tbAlpha = slide;

    // Taskbar background - layered glass effect
    SolidBrush tbBg(Color((BYTE)(235 * tbAlpha), 20, 20, 28));
    g->FillRectangle(&tbBg, 0, tbY, (float)g_width, (float)tbH));

    // Subtle top highlight
    LinearGradientBrush topLine(
        PointF(0, tbY), PointF((float)g_width, tbY),
        Color((BYTE)(50 * tbAlpha), 100, 140, 255),
        Color((BYTE)(10 * tbAlpha), 100, 140, 255));
    g->FillRectangle(&topLine, 0, tbY, (float)g_width, 1);

    // Bottom glass gradient
    LinearGradientBrush glassGrad(
        PointF(0, tbY), PointF(0, (float)g_height),
        Color((BYTE)(10 * tbAlpha), 255, 255, 255),
        Color((BYTE)(0), 255, 255, 255));
    g->FillRectangle(&glassGrad, 0, tbY, (float)g_width, (float)tbH);

    // ---- Show Desktop strip (far left) ----
    float stripW = 6.0f;
    float stripH = (float)tbH;
    Color stripColor = g_showDesktopHovered
        ? Color((BYTE)(180 * tbAlpha), 100, 180, 255)
        : Color((BYTE)(80 * tbAlpha), 100, 140, 220);
    SolidBrush stripBrush(stripColor);
    g->FillRectangle(&stripBrush, 0, tbY, stripW, stripH);

    // ---- Start Button ----
    float startX = 12.0f;
    float startBtnW = 44.0f;
    float startBtnH = 36.0f;
    float startBtnY = tbY + (float)(tbH - (int)startBtnH) / 2.0f;

    bool startHover = g_mouse.x >= startX && g_mouse.x <= startX + startBtnW &&
                      g_mouse.y >= startBtnY && g_mouse.y <= startBtnY + startBtnH;

    if (startHover || g_startMenuOpen) {
        float bgAlpha = g_startMenuOpen ? 40.0f : 25.0f;
        SolidBrush startBg(Color((BYTE)(bgAlpha * tbAlpha), 255, 255, 255));
        FillRoundRect(g, &startBg, startX, startBtnY, startBtnW, startBtnH, 6);
    }

    Font startIconFont(L"Segoe MDL2 Assets", 16, FontStyleRegular, UnitPixel);
    int startIconAlpha = (int)(g_startMenuOpen ? 255.0f : (startHover ? 240.0f : 180.0f));
    SolidBrush startIconColor(Color((BYTE)(startIconAlpha * tbAlpha), 240, 245, 255));
    StringFormat sf;
    sf.SetAlignment(StringAlignmentCenter);
    sf.SetLineAlignment(StringAlignmentCenter);
    RectF startIconRect(startX, startBtnY, startBtnW, startBtnH);
    g->DrawString(L"\uE700", -1, &startIconFont, startIconRect, &sf, &startIconColor);

    // ---- Search Box ----
    float searchX = startX + startBtnW + 12.0f;
    float searchW = 220.0f;
    float searchH = 32.0f;
    float searchY = tbY + (float)(tbH - (int)searchH) / 2.0f;

    bool searchHover = g_mouse.x >= searchX && g_mouse.x <= searchX + searchW &&
                       g_mouse.y >= searchY && g_mouse.y <= searchY + searchH;

    SolidBrush searchBg(Color((BYTE)((searchHover ? 35 : 25) * tbAlpha), 255, 255, 255));
    FillRoundRect(g, &searchBg, searchX, searchY, searchW, searchH, 16);

    Font searchIconFont(L"Segoe MDL2 Assets", 13, FontStyleRegular, UnitPixel);
    SolidBrush searchIconColor(Color((BYTE)(120 * tbAlpha), 180, 190, 210));
    g->DrawString(L"\uE721", -1, &searchIconFont,
                  RectF(searchX + 12, searchY, 20, searchH), &sf, &searchIconColor);

    Font searchTextFont(L"Segoe UI", 12, FontStyleRegular, UnitPixel);
    SolidBrush searchTextColor(Color((BYTE)(100 * tbAlpha), 150, 160, 180));
    g->DrawString(L"Search", -1, &searchTextFont,
                  RectF(searchX + 38, searchY, 150, searchH), &sf, &searchTextColor);

    // ---- Center Taskbar Icons ----
    Font taskIconFont(L"Segoe MDL2 Assets", 16, FontStyleRegular, UnitPixel);
    float taskIconSize = 40.0f;
    float taskIconGap = 4.0f;
    const wchar_t* taskIcons[] = { L"\uE756", L"\uE773", L"\uE714", L"\uE8B7", L"\uE713" };
    const wchar_t* taskLabels[] = { L"Terminal", L"Edge", L"Media", L"Files", L"Settings" };
    int taskIconCount = 5;

    float totalTaskW = (float)taskIconCount * (taskIconSize + taskIconGap) - taskIconGap;
    float taskStartX = ((float)g_width - totalTaskW) / 2.0f;
    float taskY = tbY + (float)(tbH - (int)taskIconSize) / 2.0f;

    for (int i = 0; i < taskIconCount; i++) {
        float tx = taskStartX + (float)i * (taskIconSize + taskIconGap);
        bool hovered = g_mouse.x >= tx && g_mouse.x <= tx + taskIconSize &&
                       g_mouse.y >= taskY && g_mouse.y <= taskY + taskIconSize;
        bool isActive = (i == 0); // Terminal is active

        if (hovered) {
            g_hoveredTaskbarItem = i;
            SolidBrush hoverBg(Color((BYTE)(30 * tbAlpha), 255, 255, 255));
            FillRoundRect(g, &hoverBg, tx, taskY, taskIconSize, taskIconSize, 8);
        } else if (isActive) {
            SolidBrush activeBg(Color((BYTE)(25 * tbAlpha), 255, 255, 255));
            FillRoundRect(g, &activeBg, tx, taskY, taskIconSize, taskIconSize, 8);
        }

        int iconAlpha = (int)(hovered ? 255 : (isActive ? 240 : 190));
        SolidBrush iconColor(Color((BYTE)(iconAlpha * tbAlpha), 225, 230, 245));
        g->DrawString(taskIcons[i], -1, &taskIconFont,
                      RectF(tx, taskY, taskIconSize, taskIconSize), &sf, &iconColor);

        // Active indicator
        if (isActive) {
            SolidBrush indicator(Color((BYTE)(200 * tbAlpha), 96, 165, 250));
            g->FillRoundedRectangle(&indicator, tx + taskIconSize / 2.0f - 8.0f,
                                     taskY + taskIconSize - 4.0f, 16.0f, 3.0f, 1.5f);
        }
    }

    // ---- System Tray (improved capsule) ----
    float trayCapsuleW = 320.0f;
    float trayCapsuleH = 36.0f;
    float trayCapsuleX = (float)g_width - trayCapsuleW - 10.0f;
    float trayCapsuleY = tbY + (float)(tbH - (int)trayCapsuleH) / 2.0f;

    // Capsule glass effect
    SolidBrush capsuleBg(Color((BYTE)(30 * tbAlpha), 255, 255, 255));
    FillRoundRect(g, &capsuleBg, trayCapsuleX, trayCapsuleY, trayCapsuleW, trayCapsuleH,
                  trayCapsuleH / 2.0f);

    // Inner glass highlight
    LinearGradientBrush innerGlass(
        PointF(trayCapsuleX, trayCapsuleY),
        PointF(trayCapsuleX, trayCapsuleY + trayCapsuleH * 0.5f),
        Color((BYTE)(8 * tbAlpha), 255, 255, 255),
        Color((BYTE)(0), 255, 255, 255));
    GraphicsPath capsulePath = CreateRoundRectPath(trayCapsuleX, trayCapsuleY,
                                                    trayCapsuleW, trayCapsuleH,
                                                    trayCapsuleH / 2.0f);
    g->FillPath(&innerGlass, &capsulePath);

    // Tray items
    float trayItemX = trayCapsuleX + 14.0f;
    float trayItemY = trayCapsuleY;
    float trayItemH = trayCapsuleH;

    // WiFi icon with animated signal pulse
    Font wifiFont(L"Segoe MDL2 Assets", 14, FontStyleRegular, UnitPixel);
    g_wifiSignalPulse += g_deltaTime * 2.0f;
    float wifiPulse = 0.7f + 0.3f * sinf(g_wifiSignalPulse);
    int wifiAlpha = (int)(180 * tbAlpha * wifiPulse);
    SolidBrush wifiColor(Color((BYTE)wifiAlpha, 200, 210, 230));
    g->DrawString(L"\uEC1F", -1, &wifiFont,
                  RectF(trayItemX, trayItemY, 24, trayItemH), &sf, &wifiColor);
    trayItemX += 28.0f;

    // Battery icon (with lightning bolt when "charging")
    Font battFont(L"Segoe MDL2 Assets", 14, FontStyleRegular, UnitPixel);
    int battAlpha = (int)(180 * tbAlpha);
    SolidBrush battColor(Color((BYTE)battAlpha, 120, 220, 140));
    g->DrawString(L"\uE8A1", -1, &battFont,
                  RectF(trayItemX, trayItemY, 24, trayItemH), &sf, &battColor);
    // Lightning bolt overlay
    SolidBrush boltColor(Color((BYTE)(battAlpha), 255, 220, 50));
    g->DrawString(L"\uEA8E", -1, &battFont,
                  RectF(trayItemX + 2, trayItemY, 20, trayItemH), &sf, &boltColor);
    trayItemX += 28.0f;

    // Volume icon
    SolidBrush volColor(Color((BYTE)(180 * tbAlpha), 200, 210, 230));
    g->DrawString(L"\uE767", -1, &wifiFont,
                  RectF(trayItemX, trayItemY, 24, trayItemH), &sf, &volColor);
    trayItemX += 32.0f;

    // Separator
    SolidBrush sepColor(Color((BYTE)(40 * tbAlpha), 255, 255, 255));
    g->FillRectangle(&sepColor, trayItemX, trayCapsuleY + 8.0f, 1.0f, trayCapsuleH - 16.0f);
    trayItemX += 14.0f;

    // Clock - improved typography
    time_t now = time(NULL);
    struct tm* tinfo = localtime(&now);
    wchar_t timeStr[32];
    wchar_t dateStr[64];
    GetTimeFormatW(LOCALE_USER_DEFAULT, 0, NULL, L"hh:mm tt", timeStr, 32);
    GetDateFormatW(LOCALE_USER_DEFAULT, 0, NULL, L"ddd, MMM dd", dateStr, 64);

    g_clockPulse += g_deltaTime;
    float clockBreath = 0.95f + 0.05f * sinf(g_clockPulse * 1.2f);
    int clockAlpha = (int)(235 * tbAlpha * clockBreath);

    Font clockFont(L"Segoe UI", 12, FONT_SEMIBOLD, UnitPixel);
    SolidBrush clockColor(Color((BYTE)clockAlpha, 230, 235, 250));
    g->DrawString(timeStr, -1, &clockFont,
                  RectF(trayItemX, trayCapsuleY, 80, trayItemH), &sf, &clockColor);

    // Date display below time (in tray area, slightly adjusted)
    // We show date to the left of time within the capsule
    Font dateFont(L"Segoe UI", 11, FontStyleRegular, UnitPixel);
    SolidBrush dateColor(Color((BYTE)(160 * tbAlpha), 180, 190, 210));
    // Display date inline: push time right, put date left
    RectF dateRect = MeasureText(g, dateStr, dateFont);
    float dateX = trayItemX + 80.0f + 8.0f;
    g->DrawString(dateStr, -1, &dateFont,
                  RectF(dateX, trayCapsuleY, dateRect.Width + 20, trayItemH), &sf, &dateColor);

    // Notification bell
    float bellX = trayCapsuleX + trayCapsuleW - 30.0f;
    bool bellHover = g_mouse.x >= bellX && g_mouse.x <= bellX + 20 &&
                     g_mouse.y >= trayCapsuleY && g_mouse.y <= trayCapsuleY + trayItemH;
    int bellAlpha = (int)((bellHover ? 240 : 160) * tbAlpha);
    SolidBrush bellColor(Color((BYTE)bellAlpha, 200, 210, 230));
    g->DrawString(L"\uE7F4", -1, &wifiFont,
                  RectF(bellX, trayCapsuleY, 20, trayItemH), &sf, &bellColor);
}

// ============================================================================
//  DRAW: START MENU (World-class redesign v5.3)
// ============================================================================

static void DrawStartMenu(Graphics* g) {
    g_startMenuAnim.SetTarget(g_startMenuOpen ? 1.0f : 0.0f);
    g_startMenuBgAnim.SetTarget(g_startMenuOpen ? 1.0f : 0.0f);
    g_startMenuAnim.Update(g_deltaTime);
    g_startMenuBgAnim.Update(g_deltaTime);

    float anim = g_startMenuAnim.current;
    float bgAnim = g_startMenuBgAnim.current;
    if (bgAnim < 0.01f && !g_startMenuOpen) return;

    // Menu dimensions
    float menuW = 620.0f;
    float menuH = 580.0f;
    float menuR = 12.0f;
    float menuX = (float)g_width / 2.0f - menuW / 2.0f;
    float menuBaseY = (float)g_height - g_taskbarHeight - menuH - 12.0f;

    // Spring animation offset
    float springOffset = (1.0f - pOutBack(pClamp(anim, 0, 1))) * 40.0f;
    float menuY = menuBaseY + springOffset;
    float menuAlpha = pSmoothstep(0.0f, 0.5f, bgAnim);

    // ---- Backdrop overlay ----
    if (menuAlpha > 0.01f) {
        SolidBrush overlay(Color((BYTE)(menuAlpha * 100), 0, 0, 0));
        g->FillRectangle(&overlay, 0, 0, (float)g_width, (float)g_height);
    }

    // ---- Shadow ----
    DrawDropShadow(g, menuX, menuY, menuW, menuH, menuR, 30, 0, (int)(menuAlpha * 120));

    // ---- Mica-like material effect ----
    // Layer 1: Base dark
    SolidBrush baseLayer(Color((BYTE)(menuAlpha * 245), 30, 30, 40));
    FillRoundRect(g, &baseLayer, menuX, menuY, menuW, menuH, menuR);

    // Layer 2: Subtle gradient overlay simulating backdrop blur
    GraphicsPath clipPath = CreateRoundRectPath(menuX, menuY, menuW, menuH, menuR);
    g->SetClip(&clipPath);

    // Mica gradient 1 - top luminance
    LinearGradientBrush mica1(PointF(menuX, menuY), PointF(menuX, menuY + 120),
                               Color((BYTE)(menuAlpha * 18), 80, 100, 140),
                               Color((BYTE)(menuAlpha * 0), 0, 0, 0));
    g->FillRectangle(&mica1, menuX, menuY, menuW, 120);

    // Mica gradient 2 - bottom ambient
    LinearGradientBrush mica2(PointF(menuX, menuY + menuH - 80),
                               PointF(menuX, menuY + menuH),
                               Color((BYTE)(menuAlpha * 0), 0, 0, 0),
                               Color((BYTE)(menuAlpha * 12), 60, 50, 80));
    g->FillRectangle(&mica2, menuX, menuY + menuH - 80, menuW, 80);

    // Mica gradient 3 - subtle side luminance
    LinearGradientBrush mica3(PointF(menuX, menuY),
                               PointF(menuX + menuW * 0.3f, menuY),
                               Color((BYTE)(menuAlpha * 10), 100, 120, 160),
                               Color((BYTE)(menuAlpha * 0), 0, 0, 0));
    g->FillRectangle(&mica3, menuX, menuY, menuW * 0.3f, menuH);

    // Noise texture simulation using tiny dots
    if (menuAlpha > 0.3f) {
        std::mt19937 noiseRng(123);
        std::uniform_real_distribution<float> nDist(0, 1);
        for (int ny = 0; ny < (int)menuH; ny += 4) {
            for (int nx = 0; nx < (int)menuW; nx += 4) {
                float n = nDist(noiseRng);
                if (n > 0.92f) {
                    int a = (int)(3 * menuAlpha);
                    SolidBrush noiseDot(Color((BYTE)a, 255, 255, 255));
                    g->FillRectangle(&noiseDot, menuX + nx, menuY + ny, 2, 2);
                }
            }
        }
    }

    g->SetClip(&clipPath); // re-apply

    // Inner shadow at top for depth (v5.3)
    LinearGradientBrush innerShadowTop(
        PointF(menuX, menuY),
        PointF(menuX, menuY + 30),
        Color((BYTE)(menuAlpha * 60), 0, 0, 0),
        Color((BYTE)(menuAlpha * 0), 0, 0, 0));
    g->FillRectangle(&innerShadowTop, menuX, menuY, menuW, 30);

    // ---- Border ----
    Pen borderPen(Color((BYTE)(menuAlpha * 35), 80, 90, 110), 1.0f);
    DrawRoundRect(g, &borderPen, menuX, menuY, menuW, menuH, menuR);

    // ---- Content ----
    float padX = 24.0f;
    float contentW = menuW - padX * 2.0f;
    float curY = menuY + 20.0f;

    // -- Search Bar with glow on focus --
    float searchH = 36.0f;
    g_searchGlowAnim.SetTarget(g_searchActive ? 1.0f : 0.0f);
    g_searchGlowAnim.Update(g_deltaTime);
    float searchGlow = g_searchGlowAnim.current;

    // Search glow simulation
    if (searchGlow > 0.01f) {
        SolidBrush glowBg(Color((BYTE)(searchGlow * 15 * menuAlpha), 96, 165, 250));
        FillRoundRect(g, &glowBg, menuX + padX - 4, curY - 4,
                      contentW + 8, searchH + 8, 22);
    }

    SolidBrush searchBg(Color((BYTE)(menuAlpha * (searchActive ? 40 : 25)), 255, 255, 255));
    FillRoundRect(g, &searchBg, menuX + padX, curY, contentW, searchH, 18);

    Pen searchBorder(Color((BYTE)(menuAlpha * (searchActive ? 50 : 25)), 100, 110, 130), 1.0f);
    DrawRoundRect(g, &searchBorder, menuX + padX, curY, contentW, searchH, 18);

    Font searchIconFont(L"Segoe MDL2 Assets", 14, FontStyleRegular, UnitPixel);
    StringFormat sf;
    sf.SetAlignment(StringAlignmentNear);
    sf.SetLineAlignment(StringAlignmentCenter);
    SolidBrush searchIconColor(Color((BYTE)(menuAlpha * 130), 150, 160, 180));
    g->DrawString(L"\uE721", -1, &searchIconFont,
                  RectF(menuX + padX + 14, curY, 24, searchH), &sf, &searchIconColor);

    Font searchTextFont(L"Segoe UI", 13, FontStyleRegular, UnitPixel);
    if (g_searchQuery.empty()) {
        SolidBrush placeholderColor(Color((BYTE)(menuAlpha * 100), 130, 140, 160));
        g->DrawString(L"Type to search...", -1, &searchTextFont,
                      RectF(menuX + padX + 44, curY, contentW - 60, searchH), &sf,
                      &placeholderColor);
    } else {
        SolidBrush queryColor(Color((BYTE)(menuAlpha * 240), 230, 235, 250));
        g->DrawString(g_searchQuery.c_str(), (INT)g_searchQuery.length(), &searchTextFont,
                      RectF(menuX + padX + 44, curY, contentW - 60, searchH), &sf,
                      &queryColor);
        // Cursor blink
        float cursorVis = (float)fmod(g_totalTime * 2.0, 2.0);
        if (cursorVis < 1.0f) {
            RectF cursorRect = MeasureText(g, g_searchQuery, searchTextFont);
            SolidBrush cursorColor(Color((BYTE)(menuAlpha * 200), 96, 165, 250));
            g->FillRectangle(&cursorColor,
                             menuX + padX + 44 + cursorRect.Width + 2, curY + 8, 1.5f, 20);
        }
    }

    curY += searchH + 20.0f;

    // -- Pinned Section Header --
    Font sectionFont(L"Segoe UI", 13, FONT_SEMIBOLD, UnitPixel);
    SolidBrush sectionColor(Color((BYTE)(menuAlpha * 220), 230, 235, 250));
    g->DrawString(L"Pinned", -1, &sectionFont,
                  RectF(menuX + padX, curY, 100, 22), &sf, &sectionColor);

    // All apps button
    Font allAppsFont(L"Segoe UI", 12, FontStyleRegular, UnitPixel);
    float allAppsW = 80.0f;
    float allAppsX = menuX + menuW - padX - allAppsW;
    bool allAppsHover = g_mouse.x >= allAppsX && g_mouse.x <= allAppsX + allAppsW &&
                        g_mouse.y >= curY && g_mouse.y <= curY + 22;
    SolidBrush allAppsColor(Color((BYTE)(menuAlpha * (allAppsHover ? 180 : 100)), 96, 165, 250));
    g->DrawString(L"All apps  \uE76E", -1, &allAppsFont,
                  RectF(allAppsX, curY, allAppsW + 20, 22), &sf, &allAppsColor);

    curY += 30.0f;

    // -- Pinned Apps Grid with staggered spring animations --
    float appCols = 6;
    float appCellW = contentW / appCols;
    float appCellH = 68.0f;
    int visibleApps = 0;

    // Filter by search query
    bool filtering = !g_searchQuery.empty();
    for (int i = 0; i < g_pinnedAppCount; i++) {
        if (filtering) {
            // Case-insensitive substring search
            std::wstring name = g_pinnedApps[i].name;
            std::wstring query = g_searchQuery;
            std::transform(name.begin(), name.end(), name.begin(), ::towlower);
            std::transform(query.begin(), query.end(), query.begin(), ::towlower);
            if (name.find(query) == std::wstring::npos) continue;
        }
        if (menuY + menuH < curY + appCellH) break;

        int col = visibleApps % (int)appCols;
        int row = visibleApps / (int)appCols;
        float ax = menuX + padX + (float)col * appCellW;
        float ay = curY + (float)row * appCellH;

        // Staggered animation: each row delayed
        float staggerDelay = (float)row * 0.06f;
        float staggerProgress = pClamp((anim - staggerDelay) / (1.0f - staggerDelay), 0, 1);
        float staggerEase = pOutCubic(staggerProgress);

        if (staggerEase < 0.01f) { visibleApps++; continue; }

        bool hovered = g_hoveredStartItem == i;
        float cellAlpha = menuAlpha * staggerEase;
        float hoverScale = 0.0f;

        // Check hover
        if (g_mouse.x >= ax && g_mouse.x <= ax + appCellW &&
            g_mouse.y >= ay && g_mouse.y <= ay + appCellH && !hovered) {
            g_hoveredStartItem = i;
            hoverScale = 1.0f;
        }
        if (hovered) hoverScale = 1.0f;

        // Hover background
        if (hoverScale > 0.01f) {
            SolidBrush hoverBg(Color((BYTE)(hoverScale * 30 * cellAlpha), 255, 255, 255));
            FillRoundRect(g, &hoverBg, ax + 2, ay + 2, appCellW - 4, appCellH - 4, 8);
        }

        // App icon
        float iconScale = pLerp(0.85f, 1.0f, hoverScale);
        Font appIconFont(L"Segoe MDL2 Assets", (INT)(22 * iconScale), FontStyleRegular, UnitPixel);
        StringFormat iconSf;
        iconSf.SetAlignment(StringAlignmentCenter);
        iconSf.SetLineAlignment(StringAlignmentCenter);
        int ialpha = (int)pLerp(180.0f, 255.0f, hoverScale) * (int)cellAlpha / 255;
        SolidBrush appIconColor(Color((BYTE)ialpha, 210, 215, 235));
        g->DrawString(g_pinnedApps[i].icon.c_str(), -1, &appIconFont,
                      RectF(ax, ay, appCellW, 34), &iconSf, &appIconColor);

        // App name
        Font appNameFont(L"Segoe UI", 11, FontStyleRegular, UnitPixel);
        int nameAlpha = (int)pLerp(180.0f, 255.0f, hoverScale) * (int)cellAlpha / 255;
        SolidBrush appNameColor(Color((BYTE)nameAlpha, 200, 210, 230));
        g->DrawString(g_pinnedApps[i].name.c_str(), (INT)g_pinnedApps[i].name.length(),
                      &appNameFont,
                      RectF(ax, ay + 38, appCellW, 22), &iconSf, &appNameColor);

        visibleApps++;
    }

    int pinnedRows = (visibleApps + (int)appCols - 1) / (int)appCols;
    curY += (float)pinnedRows * appCellH + 16.0f;

    // -- Separator --
    float sepY = curY;
    LinearGradientBrush sepGrad(
        PointF(menuX + padX, sepY),
        PointF(menuX + padX + contentW, sepY),
        Color((BYTE)(menuAlpha * 0), 80, 90, 110),
        Color((BYTE)(menuAlpha * 35), 80, 90, 110),
        Color((BYTE)(menuAlpha * 0), 80, 90, 110));
    g->FillRectangle(&sepGrad, menuX + padX, sepY, contentW, 1.0f);
    curY += 14.0f;

    // -- Recommended Section --
    Font recHeaderFont(L"Segoe UI", 13, FONT_SEMIBOLD, UnitPixel);
    SolidBrush recHeaderColor(Color((BYTE)(menuAlpha * 220), 230, 235, 250));
    g->DrawString(L"Recommended", -1, &recHeaderFont,
                  RectF(menuX + padX, curY, 150, 22), &sf, &recHeaderColor);

    Font moreBtnFont(L"Segoe UI", 12, FontStyleRegular, UnitPixel);
    SolidBrush moreBtnColor(Color((BYTE)(menuAlpha * 100), 96, 165, 250));
    float moreW = 60.0f;
    g->DrawString(L"More \uE76E", -1, &moreBtnFont,
                  RectF(menuX + menuW - padX - moreW, curY, moreW + 20, 22), &sf,
                  &moreBtnColor);

    curY += 28.0f;

    // Recommended items - 2 columns, proper file type icons
    float recCols = 2;
    float recCellW = contentW / recCols;
    float recCellH = 52.0f;

    for (int i = 0; i < g_recommendedItemCount && i < 4; i++) {
        int col = i % (int)recCols;
        int row = i / (int)recCols;
        float rx = menuX + padX + (float)col * recCellW;
        float ry = curY + (float)row * recCellH;

        if (ry + recCellH > menuY + menuH - 60) break;

        bool hovered = g_hoveredRecommendedItem == i;
        if (g_mouse.x >= rx && g_mouse.x <= rx + recCellW &&
            g_mouse.y >= ry && g_mouse.y <= ry + recCellH && !hovered) {
            g_hoveredRecommendedItem = i;
        }

        // Hover background
        if (g_hoveredRecommendedItem == i) {
            SolidBrush recHoverBg(Color((BYTE)(menuAlpha * 25), 255, 255, 255));
            FillRoundRect(g, &recHoverBg, rx + 2, ry + 2, recCellW - 4, recCellH - 4, 6);
        }

        // File type icon with color coding
        Font recIconFont(L"Segoe MDL2 Assets", 20, FontStyleRegular, UnitPixel);
        StringFormat recSf;
        recSf.SetAlignment(StringAlignmentCenter);
        recSf.SetLineAlignment(StringAlignmentCenter);

        // Color based on file type
        Color fileIconColor;
        const std::wstring& ft = g_recommendedItems[i].fileType;
        if (ft == L"doc") fileIconColor = Color((BYTE)(menuAlpha * 220), 70, 130, 230);
        else if (ft == L"xls") fileIconColor = Color((BYTE)(menuAlpha * 220), 60, 180, 80);
        else if (ft == L"ppt") fileIconColor = Color((BYTE)(menuAlpha * 220), 220, 120, 40);
        else if (ft == L"img") fileIconColor = Color((BYTE)(menuAlpha * 220), 200, 80, 160);
        else if (ft == L"vid") fileIconColor = Color((BYTE)(menuAlpha * 220), 180, 60, 60);
        else fileIconColor = Color((BYTE)(menuAlpha * 200), 160, 170, 190);

        SolidBrush recIconBrush(fileIconColor);
        g->DrawString(g_recommendedItems[i].icon.c_str(), -1, &recIconFont,
                      RectF(rx + 4, ry, 40, recCellH), &recSf, &recIconBrush);

        // File name
        Font recNameFont(L"Segoe UI", 12, FontStyleRegular, UnitPixel);
        SolidBrush recNameColor(Color((BYTE)(menuAlpha * 220), 220, 225, 240));
        g->DrawString(g_recommendedItems[i].name.c_str(),
                      (INT)g_recommendedItems[i].name.length(), &recNameFont,
                      RectF(rx + 48, ry + 4, recCellW - 56, 24), &sf, &recNameColor);

        // Detail
        Font recDetailFont(L"Segoe UI", 10, FontStyleRegular, UnitPixel);
        SolidBrush recDetailColor(Color((BYTE)(menuAlpha * 120), 140, 150, 170));
        g->DrawString(g_recommendedItems[i].detail.c_str(),
                      (INT)g_recommendedItems[i].detail.length(), &recDetailFont,
                      RectF(rx + 48, ry + 28, recCellW - 56, 18), &sf, &recDetailColor);
    }

    // -- User Profile + Power Section (bottom) --
    float bottomY = menuY + menuH - 52.0f;

    // Separator
    LinearGradientBrush bottomSep(
        PointF(menuX + padX, bottomY - 12),
        PointF(menuX + padX + contentW, bottomY - 12),
        Color((BYTE)(menuAlpha * 0), 80, 90, 110),
        Color((BYTE)(menuAlpha * 30), 80, 90, 110),
        Color((BYTE)(menuAlpha * 0), 80, 90, 110));
    g->FillRectangle(&bottomSep, menuX + padX, bottomY - 12, contentW, 1.0f);

    // User profile with gradient avatar circle
    float avatarR = 16.0f;
    float avatarCX = menuX + padX + avatarR;
    float avatarCY = bottomY + 20.0f;

    // Gradient avatar circle
    GraphicsPath avatarPath;
    avatarPath.AddEllipse(avatarCX - avatarR, avatarCY - avatarR, avatarR * 2, avatarR * 2);
    LinearGradientBrush avatarGrad(
        PointF(avatarCX - avatarR, avatarCY - avatarR),
        PointF(avatarCX + avatarR, avatarCY + avatarR),
        Color((BYTE)(menuAlpha * 255), 96, 165, 250),
        Color((BYTE)(menuAlpha * 255), 160, 100, 220));
    g->FillPath(&avatarGrad, &avatarPath);

    // Avatar letter
    Font avatarFont(L"Segoe UI", 14, FONT_SEMIBOLD, UnitPixel);
    StringFormat avatarSf;
    avatarSf.SetAlignment(StringAlignmentCenter);
    avatarSf.SetLineAlignment(StringAlignmentCenter);
    SolidBrush avatarLetterColor(Color((BYTE)(menuAlpha * 255), 255, 255, 255));
    g->DrawString(L"U", -1, &avatarFont,
                  RectF(avatarCX - avatarR, avatarCY - avatarR, avatarR * 2, avatarR * 2),
                  &avatarSf, &avatarLetterColor);

    // User name
    Font userFont(L"Segoe UI", 12, FontStyleRegular, UnitPixel);
    SolidBrush userColor(Color((BYTE)(menuAlpha * 210), 220, 225, 240));
    g->DrawString(L"User", -1, &userFont,
                  RectF(menuX + padX + avatarR * 2 + 12, bottomY + 12, 100, 20), &sf,
                  &userColor);

    // Power button with dropdown arrow
    float powerBtnW = 36.0f;
    float powerBtnH = 32.0f;
    float powerBtnX = menuX + menuW - padX - powerBtnW;
    float powerBtnY = bottomY + 10.0f;

    bool powerHover = g_mouse.x >= powerBtnX && g_mouse.x <= powerBtnX + powerBtnW &&
                      g_mouse.y >= powerBtnY && g_mouse.y <= powerBtnY + powerBtnH;
    g_hoveredPowerBtn = powerHover ? 1 : 0;

    if (powerHover || g_powerMenuOpen) {
        SolidBrush powerHoverBg(Color((BYTE)(menuAlpha * 30), 255, 255, 255));
        FillRoundRect(g, &powerHoverBg, powerBtnX - 4, powerBtnY - 4,
                      powerBtnW + 8, powerBtnH + 8, 8);
    }

    Font powerIconFont(L"Segoe MDL2 Assets", 14, FontStyleRegular, UnitPixel);
    int powerAlpha = (int)(menuAlpha * (powerHover || g_powerMenuOpen ? 230 : 160));
    SolidBrush powerIconColor(Color((BYTE)powerAlpha, 200, 210, 230));
    StringFormat powerSf;
    powerSf.SetAlignment(StringAlignmentCenter);
    powerSf.SetLineAlignment(StringAlignmentCenter);
    g->DrawString(L"\uE7E8", -1, &powerIconFont,
                  RectF(powerBtnX, powerBtnY, powerBtnW - 10, powerBtnH), &powerSf,
                  &powerIconColor);
    // Dropdown arrow
    Font arrowFont(L"Segoe MDL2 Assets", 8, FontStyleRegular, UnitPixel);
    g->DrawString(L"\uE70E", -1, &arrowFont,
                  RectF(powerBtnX + powerBtnW - 14, powerBtnY + 16, 12, 16), &powerSf,
                  &powerIconColor);

    g->ResetClip();
}

// ============================================================================
//  DRAW: POWER MENU (v5.3)
// ============================================================================

static void DrawPowerMenu(Graphics* g) {
    if (!g_startMenuOpen) {
        g_powerMenuOpen = false;
    }

    g_powerMenuAnim.SetTarget(g_powerMenuOpen ? 1.0f : 0.0f);
    g_powerMenuAnim.Update(g_deltaTime);

    float anim = g_powerMenuAnim.current;
    if (anim < 0.01f && !g_powerMenuOpen) return;

    // Position above power button in start menu
    float menuW = 620.0f;
    float menuH = 580.0f;
    float menuX = (float)g_width / 2.0f - menuW / 2.0f;
    float menuBaseY = (float)g_height - g_taskbarHeight - menuH - 12.0f;
    float startAnim = g_startMenuAnim.current;
    float springOffset = (1.0f - pOutBack(pClamp(startAnim, 0, 1))) * 40.0f;
    float startMenuY = menuBaseY + springOffset;

    float pmW = 200.0f;
    float pmH = 230.0f;
    float pmR = 10.0f;
    float pmX = menuX + menuW - 24.0f - pmW;
    float pmBaseY = startMenuY - pmH + 40.0f;

    // Animation
    float pmOffset = (1.0f - pOutCubic(pClamp(anim, 0, 1))) * 15.0f;
    float pmAlpha = pSmoothstep(0.0f, 0.4f, anim);
    float pmY = pmBaseY + pmOffset;

    if (pmAlpha < 0.01f) return;

    // Shadow
    DrawDropShadow(g, pmX, pmY, pmW, pmH, pmR, 20, 0, (int)(pmAlpha * 100));

    // Background
    SolidBrush pmBg(Color((BYTE)(pmAlpha * 240), 35, 35, 48));
    FillRoundRect(g, &pmBg, pmX, pmY, pmW, pmH, pmR);

    // Subtle glass overlay
    LinearGradientBrush pmGlass(PointF(pmX, pmY), PointF(pmX, pmY + pmH * 0.4f),
                                 Color((BYTE)(pmAlpha * 10), 80, 100, 140),
                                 Color((BYTE)(pmAlpha * 0), 0, 0, 0));
    GraphicsPath pmPath = CreateRoundRectPath(pmX, pmY, pmW, pmH, pmR);
    g->FillPath(&pmGlass, &pmPath);

    // Border
    Pen pmBorder(Color((BYTE)(pmAlpha * 40), 80, 90, 120), 1.0f);
    DrawRoundRect(g, &pmBorder, pmX, pmY, pmW, pmH, pmR);

    // Items
    float itemH = 40.0f;
    float itemX = pmX + 8.0f;
    float itemW = pmW - 16.0f;
    float curY = pmY + 8.0f;

    for (int i = 0; i < g_powerMenuItemCount; i++) {
        const PowerMenuItem& item = g_powerMenuItems[i];
        float iy = curY;

        bool hovered = g_hoveredPowerItem == i;
        if (g_mouse.x >= itemX && g_mouse.x <= itemX + itemW &&
            g_mouse.y >= iy && g_mouse.y <= iy + itemH && !hovered) {
            g_hoveredPowerItem = i;
        }

        // Hover background with subtle gradient
        if (g_hoveredPowerItem == i) {
            FillRoundRectGradient(g, itemX, iy + 2, itemW, itemH - 4, 6,
                                   Color((BYTE)(pmAlpha * 35), 70, 100, 160),
                                   Color((BYTE)(pmAlpha * 20), 60, 80, 140), true);
        }

        // Icon
        Font pmIconFont(L"Segoe MDL2 Assets", 16, FontStyleRegular, UnitPixel);
        int iconAlpha = (int)(pmAlpha * (hovered ? 240 : 180));
        SolidBrush pmIconColor(Color((BYTE)iconAlpha, 200, 210, 235));
        StringFormat pmSf;
        pmSf.SetAlignment(StringAlignmentNear);
        pmSf.SetLineAlignment(StringAlignmentCenter);
        g->DrawString(item.icon.c_str(), -1, &pmIconFont,
                      RectF(itemX + 8, iy, 28, itemH), &pmSf, &pmIconColor);

        // Label
        Font pmLabelFont(L"Segoe UI", 13, FontStyleRegular, UnitPixel);
        int labelAlpha = (int)(pmAlpha * (hovered ? 240 : 190));
        SolidBrush pmLabelColor(Color((BYTE)labelAlpha, 220, 225, 240));
        g->DrawString(item.label.c_str(), (INT)item.label.length(), &pmLabelFont,
                      RectF(itemX + 40, iy, itemW - 50, itemH), &pmSf, &pmLabelColor);

        curY += itemH;

        // Separator after sign out
        if (item.separator) {
            float sepY2 = curY - 4;
            LinearGradientBrush pmSep(
                PointF(itemX + 12, sepY2), PointF(itemX + itemW - 12, sepY2),
                Color((BYTE)(pmAlpha * 0), 80, 90, 120),
                Color((BYTE)(pmAlpha * 30), 80, 90, 120),
                Color((BYTE)(pmAlpha * 0), 80, 90, 120));
            g->FillRectangle(&pmSep, itemX + 12, sepY2, itemW - 24, 1.0f);
            curY += 8.0f;
        }
    }
}

// ============================================================================
//  DRAW: CONTEXT MENU (Improved v5.3)
// ============================================================================

static void DrawContextMenu(Graphics* g) {
    g_contextMenuAnim.SetTarget(g_contextMenuOpen ? 1.0f : 0.0f);
    g_contextMenuAnim.Update(g_deltaTime);

    float anim = g_contextMenuAnim.current;
    if (anim < 0.01f && !g_contextMenuOpen) return;

    // Measure menu dimensions
    Font ctxFont(L"Segoe UI", 13, FontStyleRegular, UnitPixel);
    float itemH = 34.0f;
    float menuW = 220.0f;
    int separatorCount = 0;
    for (int i = 0; i < g_contextMenuItemCount; i++) {
        if (g_contextMenuItems[i].separator) separatorCount++;
    }
    float totalH = (float)(g_contextMenuItemCount - separatorCount) * itemH +
                   (float)separatorCount * 11.0f + 8.0f;

    float menuX = (float)g_contextMenuPos.x;
    float menuY = (float)g_contextMenuPos.y;
    float menuR = 10.0f;

    // Clamp to screen
    if (menuX + menuW > (float)g_width) menuX = (float)g_width - menuW - 8;
    if (menuY + totalH > (float)g_height - g_taskbarHeight)
        menuY = (float)g_height - g_taskbarHeight - totalH - 8;

    // Clip animation
    float clipProgress = pOutCubic(pClamp(anim, 0, 1));
    float alpha = pSmoothstep(0.0f, 0.5f, anim);

    if (alpha < 0.01f) return;

    // Shadow (multiple layers)
    DrawDropShadow(g, menuX, menuY, menuW, totalH, menuR, 25, 0, (int)(alpha * 110));

    // Background
    SolidBrush ctxBg(Color((BYTE)(alpha * 240), 40, 40, 52));
    FillRoundRect(g, &ctxBg, menuX, menuY, menuW, totalH, menuR);

    // Subtle glass effect
    LinearGradientBrush ctxGlass(
        PointF(menuX, menuY), PointF(menuX, menuY + totalH * 0.3f),
        Color((BYTE)(alpha * 8), 80, 100, 140),
        Color((BYTE)(alpha * 0), 0, 0, 0));
    GraphicsPath ctxPath = CreateRoundRectPath(menuX, menuY, menuW, totalH, menuR);
    g->FillPath(&ctxGlass, &ctxPath);

    // Border
    Pen ctxBorder(Color((BYTE)(alpha * 40), 80, 90, 120), 1.0f);
    DrawRoundRect(g, &ctxBorder, menuX, menuY, menuW, totalH, menuR);

    // Apply clip animation
    GraphicsPath clipPath = CreateRoundRectPath(menuX, menuY,
                                                 menuW * clipProgress, totalH, menuR);
    g->SetClip(&clipPath);

    // Items
    float curY = menuY + 4.0f;
    for (int i = 0; i < g_contextMenuItemCount; i++) {
        const ContextMenuItem& item = g_contextMenuItems[i];

        if (item.separator) {
            // Gradient fade separator (v5.3)
            float sepY = curY + 5.0f;
            LinearGradientBrush sepGrad(
                PointF(menuX + 12, sepY), PointF(menuX + menuW - 12, sepY),
                Color((BYTE)(alpha * 0), 80, 90, 110),
                Color((BYTE)(alpha * 35), 80, 90, 110),
                Color((BYTE)(alpha * 0), 80, 90, 110));
            g->FillRectangle(&sepGrad, menuX + 12, sepY, menuW - 24, 1.0f);
            curY += 11.0f;
            continue;
        }

        float iy = curY;
        bool hovered = g_hoveredContextItem == i;

        if (g_mouse.x >= menuX && g_mouse.x <= menuX + menuW &&
            g_mouse.y >= iy && g_mouse.y <= iy + itemH && !hovered) {
            g_hoveredContextItem = i;
        }

        // Hover effect with subtle gradient background (v5.3)
        if (g_hoveredContextItem == i) {
            FillRoundRectGradient(g, menuX + 4, iy + 1, menuW - 8, itemH - 2, 6,
                                   Color((BYTE)(alpha * 40), 60, 90, 150),
                                   Color((BYTE)(alpha * 25), 50, 75, 130), true);
        }

        // Icon (v5.3) - small unicode icons next to each item
        if (!item.icon.empty()) {
            Font ctxIconFont(L"Segoe MDL2 Assets", 13, FontStyleRegular, UnitPixel);
            int iconAlpha = (int)(alpha * (hovered ? 230 : 150));
            SolidBrush ctxIconColor(Color((BYTE)iconAlpha, 180, 190, 215));
            StringFormat ctxIconSf;
            ctxIconSf.SetAlignment(StringAlignmentCenter);
            ctxIconSf.SetLineAlignment(StringAlignmentCenter);
            g->DrawString(item.icon.c_str(), -1, &ctxIconFont,
                          RectF(menuX + 14, iy, 24, itemH), &ctxIconSf, &ctxIconColor);
        }

        // Label
        int labelAlpha = (int)(alpha * (hovered ? 245 : 195));
        SolidBrush ctxLabelColor(Color((BYTE)labelAlpha, 220, 225, 240));
        StringFormat ctxSf;
        ctxSf.SetAlignment(StringAlignmentNear);
        ctxSf.SetLineAlignment(StringAlignmentCenter);
        g->DrawString(item.label.c_str(), (INT)item.label.length(), &ctxFont,
                      RectF(menuX + 42, iy, menuW - 56, itemH), &ctxSf, &ctxLabelColor);

        curY += itemH;
    }

    g->ResetClip();
}

// ============================================================================
//  DRAW: NOTIFICATIONS (Improved v5.3)
// ============================================================================

static void DrawNotifications(Graphics* g) {
    UpdateNotifications();
    if (g_notificationCount == 0) return;

    float notifW = 360.0f;
    float notifGap = 10.0f;
    float notifX = (float)g_width - notifW - 20.0f;
    float notifStartY = 20.0f;

    for (int i = 0; i < g_notificationCount; i++) {
        Notification& n = g_notifications[i];
        float a = n.alpha;
        float slideX = n.slideX;

        if (a < 0.01f) continue;

        float ny = notifStartY + (float)i * (110.0f + notifGap);
        float nx = notifX + slideX;
        float notifH = 100.0f;
        float notifR = 10.0f;

        // Shadow (improved)
        DrawDropShadow(g, nx, ny, notifW, notifH, notifR, 20, 0, (int)(a * 90));

        // Background
        SolidBrush notifBg(Color((BYTE)(a * 240), 38, 38, 50));
        FillRoundRect(g, &notifBg, nx, ny, notifW, notifH, notifR);

        // Glass highlight
        LinearGradientBrush notifGlass(
            PointF(nx, ny), PointF(nx, ny + notifH * 0.3f),
            Color((BYTE)(a * 8), 80, 100, 140),
            Color((BYTE)(a * 0), 0, 0, 0));
        GraphicsPath notifPath = CreateRoundRectPath(nx, ny, notifW, notifH, notifR);
        g->FillPath(&notifGlass, &notifPath);

        // Gradient accent bar on left side (v5.3)
        float accentW = 3.0f;
        float accentR = 2.0f;
        GraphicsPath accentPath;
        accentPath.AddArc(nx + 4, ny + 4, accentR * 2, accentR * 2, 180, 90);
        accentPath.AddLine(nx + 4 + accentR, ny + 4, nx + 4 + accentR, ny + notifH - 4 - accentR);
        accentPath.AddArc(nx + 4, ny + notifH - 4 - accentR * 2, accentR * 2, accentR * 2, 90, 90);
        accentPath.CloseFigure();
        LinearGradientBrush accentGrad(
            PointF(nx + 4, ny + 10), PointF(nx + 4, ny + notifH - 10),
            Color((BYTE)(a * 255), 96, 165, 250),
            Color((BYTE)(a * 255), 140, 100, 220));
        g->FillPath(&accentGrad, &accentPath);

        // Border
        Pen notifBorder(Color((BYTE)(a * 30), 70, 80, 100), 1.0f);
        DrawRoundRect(g, &notifBorder, nx, ny, notifW, notifH, notifR);

        // Close button (v5.3)
        float closeBtnSize = 20.0f;
        float closeBtnX = nx + notifW - closeBtnSize - 8;
        float closeBtnY = ny + 8;
        bool closeHovered = g_mouse.x >= closeBtnX && g_mouse.x <= closeBtnX + closeBtnSize &&
                            g_mouse.y >= closeBtnY && g_mouse.y <= closeBtnY + closeBtnSize;
        if (closeHovered) g_hoveredNotifCloseBtn = i;

        if (closeHovered) {
            SolidBrush closeHoverBg(Color((BYTE)(a * 40), 255, 255, 255));
            FillRoundRect(g, &closeHoverBg, closeBtnX - 2, closeBtnY - 2,
                          closeBtnSize + 4, closeBtnSize + 4, 4);
        }
        Font closeFont(L"Segoe MDL2 Assets", 10, FontStyleRegular, UnitPixel);
        StringFormat closeSf;
        closeSf.SetAlignment(StringAlignmentCenter);
        closeSf.SetLineAlignment(StringAlignmentCenter);
        int closeAlpha = (int)(a * (closeHovered ? 220 : 100));
        SolidBrush closeColor(Color((BYTE)closeAlpha, 180, 190, 210));
        g->DrawString(L"\uE711", -1, &closeFont,
                      RectF(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize), &closeSf,
                      &closeColor);

        // Icon area (larger in v5.3)
        float iconAreaSize = 40.0f;
        float iconAreaX = nx + 16;
        float iconAreaY = ny + (notifH - iconAreaSize) / 2.0f;

        SolidBrush iconAreaBg(Color((BYTE)(a * 25), 96, 165, 250));
        FillRoundRect(g, &iconAreaBg, iconAreaX, iconAreaY, iconAreaSize, iconAreaSize, 10);

        Font notifIconFont(L"Segoe MDL2 Assets", 18, FontStyleRegular, UnitPixel);
        SolidBrush notifIconColor(Color((BYTE)(a * 230), 220, 230, 250));
        g->DrawString(n.icon.c_str(), -1, &notifIconFont,
                      RectF(iconAreaX, iconAreaY, iconAreaSize, iconAreaSize), &closeSf,
                      &notifIconColor);

        // Title (better typography hierarchy in v5.3)
        Font notifTitleFont(L"Segoe UI", 13, FONT_SEMIBOLD, UnitPixel);
        SolidBrush notifTitleColor(Color((BYTE)(a * 240), 230, 235, 250));
        StringFormat notifSf;
        notifSf.SetAlignment(StringAlignmentNear);
        notifSf.SetLineAlignment(StringAlignmentNear);
        notifSf.SetTrimming(StringTrimmingEllipsisCharacter);
        g->DrawString(n.title.c_str(), (INT)n.title.length(), &notifTitleFont,
                      RectF(iconAreaX + iconAreaSize + 14, ny + 14,
                            notifW - iconAreaSize - 60, 24),
                      &notifSf, &notifTitleColor);

        // Body
        Font notifBodyFont(L"Segoe UI", 12, FontStyleRegular, UnitPixel);
        SolidBrush notifBodyColor(Color((BYTE)(a * 170), 180, 190, 210));
        g->DrawString(n.body.c_str(), (INT)n.body.length(), &notifBodyFont,
                      RectF(iconAreaX + iconAreaSize + 14, ny + 40,
                            notifW - iconAreaSize - 60, 44),
                      &notifSf, &notifBodyColor);
    }
}

// ============================================================================
//  DRAW: MUSIC WIDGET
// ============================================================================

static void DrawMusicWidget(Graphics* g) {
    g_musicAnim.SetTarget(g_musicOpen ? 1.0f : 0.0f);
    g_musicAnim.Update(g_deltaTime);
    g_musicProgressAnim.Update(g_deltaTime);

    float anim = g_musicAnim.current;
    if (anim < 0.01f && !g_musicOpen) return;

    float wW = 320.0f;
    float wH = 180.0f;
    float wR = 14.0f;
    float wX = (float)g_width - wW - 20.0f;
    float wBaseY = (float)g_height - g_taskbarHeight - wH - 20.0f;

    float wOffset = (1.0f - pOutCubic(pClamp(anim, 0, 1))) * 30.0f;
    float wY = wBaseY + wOffset;
    float wAlpha = pSmoothstep(0.0f, 0.4f, anim);

    if (wAlpha < 0.01f) return;

    // Shadow
    DrawDropShadow(g, wX, wY, wW, wH, wR, 25, 0, (int)(wAlpha * 100));

    // Background
    SolidBrush wBg(Color((BYTE)(wAlpha * 240), 32, 32, 45));
    FillRoundRect(g, &wBg, wX, wY, wW, wH, wR);

    // Album art gradient
    float artX = wX + 16;
    float artY = wY + 16;
    float artSize = wH - 32.0f;
    GraphicsPath artPath;
    artPath.AddRectangle(artX, artY, artSize, artSize);
    LinearGradientBrush artGrad(
        PointF(artX, artY), PointF(artX + artSize, artY + artSize),
        Color((BYTE)(wAlpha * 255), 96, 60, 180),
        Color((BYTE)(wAlpha * 255), 30, 80, 160));
    g->FillPath(&artGrad, &artPath);
    // Rounded clip for art
    GraphicsPath artClip = CreateRoundRectPath(artX, artY, artSize, artSize, 10);
    g->SetClip(&artClip);
    g->FillPath(&artGrad, &artPath);
    g->ResetClip();

    // Music note on art
    Font noteFont(L"Segoe MDL2 Assets", 32, FontStyleRegular, UnitPixel);
    StringFormat noteSf;
    noteSf.SetAlignment(StringAlignmentCenter);
    noteSf.SetLineAlignment(StringAlignmentCenter);
    SolidBrush noteColor(Color((BYTE)(wAlpha * 80), 255, 255, 255));
    g->DrawString(L"\uEC4F", -1, &noteFont,
                  RectF(artX, artY, artSize, artSize), &noteSf, &noteColor);

    // Border
    Pen artBorder(Color((BYTE)(wAlpha * 30), 100, 80, 160), 1.0f);
    DrawRoundRect(g, &artBorder, artX, artY, artSize, artSize, 10);

    // Info section
    float infoX = artX + artSize + 18.0f;
    float infoW = wW - artSize - 50.0f;

    // Title
    Font titleFont(L"Segoe UI", 14, FONT_SEMIBOLD, UnitPixel);
    SolidBrush titleColor(Color((BYTE)(wAlpha * 230), 225, 230, 245));
    StringFormat titleSf;
    titleSf.SetAlignment(StringAlignmentNear);
    titleSf.SetLineAlignment(StringAlignmentNear);
    titleSf.SetTrimming(StringTrimmingEllipsisCharacter);
    g->DrawString(g_musicTitle.c_str(), (INT)g_musicTitle.length(), &titleFont,
                  RectF(infoX, wY + 20, infoW, 24), &titleSf, &titleColor);

    // Artist
    Font artistFont(L"Segoe UI", 12, FontStyleRegular, UnitPixel);
    SolidBrush artistColor(Color((BYTE)(wAlpha * 160), 170, 180, 200));
    g->DrawString(g_musicArtist.c_str(), (INT)g_musicArtist.length(), &artistFont,
                  RectF(infoX, wY + 44, infoW, 20), &titleSf, &artistColor);

    // Progress bar
    float progY = wY + 80;
    float progW = infoW;
    float progH = 4.0f;

    SolidBrush progBg(Color((BYTE)(wAlpha * 30), 255, 255, 255));
    FillRoundRect(g, &progBg, infoX, progY, progW, progH, 2);

    float progFill = g_musicProgressAnim.current;
    SolidBrush progFill_brush(Color((BYTE)(wAlpha * 220), 96, 165, 250));
    FillRoundRect(g, &progFill_brush, infoX, progY, progW * progFill, progH, 2);

    // Time labels
    Font timeFont(L"Segoe UI", 9, FontStyleRegular, UnitPixel);
    SolidBrush timeColor(Color((BYTE)(wAlpha * 120), 150, 160, 180));
    int elapsed = (int)(progFill * 240);
    int total = 240;
    wchar_t elapsedStr[16], totalStr[16];
    swprintf_s(elapsedStr, L"%d:%02d", elapsed / 60, elapsed % 60);
    swprintf_s(totalStr, L"%d:%02d", total / 60, total % 60);
    g->DrawString(elapsedStr, -1, &timeFont,
                  RectF(infoX, progY + 8, 40, 16), &titleSf, &timeColor);
    StringFormat timeEndSf;
    timeEndSf.SetAlignment(StringAlignmentFar);
    timeEndSf.SetLineAlignment(StringAlignmentNear);
    g->DrawString(totalStr, -1, &timeFont,
                  RectF(infoX, progY + 8, progW, 16), &timeEndSf, &timeColor);

    // Controls
    float ctrlY = wY + wH - 44.0f;
    float ctrlBtnSize = 28.0f;

    // Previous
    Font ctrlFont(L"Segoe MDL2 Assets", 14, FontStyleRegular, UnitPixel);
    SolidBrush ctrlColor(Color((BYTE)(wAlpha * 170), 200, 210, 230));
    StringFormat ctrlSf;
    ctrlSf.SetAlignment(StringAlignmentCenter);
    ctrlSf.SetLineAlignment(StringAlignmentCenter);
    g->DrawString(L"\uE892", -1, &ctrlFont,
                  RectF(infoX, ctrlY, ctrlBtnSize, ctrlBtnSize), &ctrlSf, &ctrlColor);

    // Play/Pause
    Font playFont(L"Segoe MDL2 Assets", 18, FontStyleRegular, UnitPixel);
    SolidBrush playColor(Color((BYTE)(wAlpha * 240), 230, 235, 250));
    g->DrawString(g_musicPlaying ? L"\uE769" : L"\uE768", -1, &playFont,
                  RectF(infoX + ctrlBtnSize + 8, ctrlY - 2, ctrlBtnSize + 4, ctrlBtnSize + 4),
                  &ctrlSf, &playColor);

    // Next
    g->DrawString(L"\uE893", -1, &ctrlFont,
                  RectF(infoX + ctrlBtnSize * 2 + 16, ctrlY, ctrlBtnSize, ctrlBtnSize),
                  &ctrlSf, &ctrlColor);

    // Border
    Pen wBorder(Color((BYTE)(wAlpha * 30), 70, 80, 100), 1.0f);
    DrawRoundRect(g, &wBorder, wX, wY, wW, wH, wR);

    // Animate progress
    if (g_musicPlaying) {
        g_musicProgress += g_deltaTime / 240.0f;
        if (g_musicProgress > 1.0f) g_musicProgress = 0.0f;
        g_musicProgressAnim.SetTarget(g_musicProgress);
    }
}

// ============================================================================
//  DRAW: WIDGETS PANEL
// ============================================================================

static void DrawWidgets(Graphics* g) {
    g_widgetsAnim.SetTarget(g_widgetsOpen ? 1.0f : 0.0f);
    g_widgetsAnim.Update(g_deltaTime);

    float anim = g_widgetsAnim.current;
    if (anim < 0.01f && !g_widgetsOpen) return;

    float wW = 380.0f;
    float wH = 500.0f;
    float wR = 14.0f;
    float wX = (float)g_width - wW - 20.0f;
    float wBaseY = 20.0f;

    float wOffset = (1.0f - pOutCubic(pClamp(anim, 0, 1))) * 30.0f;
    float wY = wBaseY - wOffset;
    float wAlpha = pSmoothstep(0.0f, 0.4f, anim);

    if (wAlpha < 0.01f) return;

    // Shadow
    DrawDropShadow(g, wX, wY, wW, wH, wR, 25, 0, (int)(wAlpha * 100));

    // Background
    SolidBrush wBg(Color((BYTE)(wAlpha * 240), 32, 32, 45));
    FillRoundRect(g, &wBg, wX, wY, wW, wH, wR);

    // Glass effect
    LinearGradientBrush wGlass(
        PointF(wX, wY), PointF(wX, wY + wH * 0.3f),
        Color((BYTE)(wAlpha * 10), 80, 100, 140),
        Color((BYTE)(wAlpha * 0), 0, 0, 0));
    GraphicsPath wPath = CreateRoundRectPath(wX, wY, wW, wH, wR);
    g->FillPath(&wGlass, &wPath);

    // Border
    Pen wBorder(Color((BYTE)(wAlpha * 30), 70, 80, 100), 1.0f);
    DrawRoundRect(g, &wBorder, wX, wY, wW, wH, wR);

    // Content
    float padX = 20.0f;
    float curY = wY + 20.0f;

    // Header
    Font headerFont(L"Segoe UI", 16, FONT_SEMIBOLD, UnitPixel);
    SolidBrush headerColor(Color((BYTE)(wAlpha * 230), 225, 230, 245));
    StringFormat sf;
    sf.SetAlignment(StringAlignmentNear);
    sf.SetLineAlignment(StringAlignmentNear);
    g->DrawString(L"Widgets", -1, &headerFont,
                  RectF(wX + padX, curY, 100, 28), &sf, &headerColor);
    curY += 36.0f;

    // Weather widget card
    float cardW = wW - padX * 2.0f;
    float cardH = 100.0f;
    FillRoundRectGradient(g, wX + padX, curY, cardW, cardH, 10,
                           Color((BYTE)(wAlpha * 35), 40, 60, 100),
                           Color((BYTE)(wAlpha * 25), 30, 45, 80), true);
    Pen cardBorder(Color((BYTE)(wAlpha * 25), 60, 70, 90), 1.0f);
    DrawRoundRect(g, &cardBorder, wX + padX, curY, cardW, cardH, 10);

    // Weather icon and info
    Font weatherIconFont(L"Segoe MDL2 Assets", 28, FontStyleRegular, UnitPixel);
    SolidBrush weatherIconColor(Color((BYTE)(wAlpha * 200), 250, 210, 80));
    StringFormat wsf;
    wsf.SetAlignment(StringAlignmentCenter);
    wsf.SetLineAlignment(StringAlignmentCenter);
    g->DrawString(L"\uE70A", -1, &weatherIconFont,
                  RectF(wX + padX + 10, curY + 10, 50, 50), &wsf, &weatherIconColor);

    Font tempFont(L"Segoe UI", 28, FONT_SEMIBOLD, UnitPixel);
    SolidBrush tempColor(Color((BYTE)(wAlpha * 230), 230, 235, 250));
    g->DrawString(L"72\u00B0F", -1, &tempFont,
                  RectF(wX + padX + 60, curY + 8, 120, 36), &sf, &tempColor);

    Font weatherDescFont(L"Segoe UI", 12, FontStyleRegular, UnitPixel);
    SolidBrush weatherDescColor(Color((BYTE)(wAlpha * 160), 170, 180, 200));
    g->DrawString(L"Partly Cloudy  \u2022  H:78\u00B0  L:65\u00B0", -1,
                  &weatherDescFont,
                  RectF(wX + padX + 60, curY + 46, 250, 20), &sf, &weatherDescColor);

    // Location
    Font locFont(L"Segoe UI", 10, FontStyleRegular, UnitPixel);
    SolidBrush locColor(Color((BYTE)(wAlpha * 120), 140, 150, 170));
    g->DrawString(L"\uE81D  San Francisco, CA", -1, &locFont,
                  RectF(wX + padX + 14, curY + cardH - 26, 200, 16), &sf, &locColor);

    curY += cardH + 16.0f;

    // Quick Notes card
    float noteH = 120.0f;
    FillRoundRectGradient(g, wX + padX, curY, cardW, noteH, 10,
                           Color((BYTE)(wAlpha * 30), 45, 50, 70),
                           Color((BYTE)(wAlpha * 20), 35, 40, 60), true);
    DrawRoundRect(g, &cardBorder, wX + padX, curY, cardW, noteH, 10);

    Font noteHeaderFont(L"Segoe UI", 13, FONT_SEMIBOLD, UnitPixel);
    SolidBrush noteHeaderColor(Color((BYTE)(wAlpha * 200), 200, 210, 230));
    g->DrawString(L"\uE70F  Quick Notes", -1, &noteHeaderFont,
                  RectF(wX + padX + 14, curY + 12, 150, 22), &sf, &noteHeaderColor);

    Font noteBodyFont(L"Segoe UI", 11, FontStyleRegular, UnitPixel);
    SolidBrush noteBodyColor(Color((BYTE)(wAlpha * 150), 160, 170, 190));
    g->DrawString(L"\u2022 Review pull request #42\n\u2022 Update project timeline\n\u2022 "
                  L"Schedule team sync",
                  -1, &noteBodyFont,
                  RectF(wX + padX + 14, curY + 40, cardW - 28, 70), &sf, &noteBodyColor);

    curY += noteH + 16.0f;

    // Calendar mini card
    float calH = 100.0f;
    FillRoundRectGradient(g, wX + padX, curY, cardW, calH, 10,
                           Color((BYTE)(wAlpha * 30), 45, 50, 70),
                           Color((BYTE)(wAlpha * 20), 35, 40, 60), true);
    DrawRoundRect(g, &cardBorder, wX + padX, curY, cardW, calH, 10);

    Font calHeaderFont(L"Segoe UI", 13, FONT_SEMIBOLD, UnitPixel);
    SolidBrush calHeaderColor(Color((BYTE)(wAlpha * 200), 200, 210, 230));
    g->DrawString(L"\uE787  Calendar", -1, &calHeaderFont,
                  RectF(wX + padX + 14, curY + 12, 150, 22), &sf, &calHeaderColor);

    Font calBodyFont(L"Segoe UI", 11, FontStyleRegular, UnitPixel);
    SolidBrush calBodyColor(Color((BYTE)(wAlpha * 150), 160, 170, 190));
    g->DrawString(L"2:00 PM  \u2022  Team Standup\n3:30 PM  \u2022  Design Review\n"
                  L"5:00 PM  \u2022  Client Call",
                  -1, &calBodyFont,
                  RectF(wX + padX + 14, curY + 40, cardW - 28, 54), &sf, &calBodyColor);
}

// ============================================================================
//  DRAW: WIFI PANEL
// ============================================================================

static void DrawWiFiPanel(Graphics* g) {
    g_wifiAnim.SetTarget(g_wifiOpen ? 1.0f : 0.0f);
    g_wifiAnim.Update(g_deltaTime);

    float anim = g_wifiAnim.current;
    if (anim < 0.01f && !g_wifiOpen) return;

    float pW = 340.0f;
    float pH = 340.0f;
    float pR = 12.0f;
    float pX = (float)g_width / 2.0f - pW / 2.0f;
    float pBaseY = (float)g_height - g_taskbarHeight - pH - 20.0f;

    float pOffset = (1.0f - pOutCubic(pClamp(anim, 0, 1))) * 20.0f;
    float pY = pBaseY + pOffset;
    float pAlpha = pSmoothstep(0.0f, 0.4f, anim);

    if (pAlpha < 0.01f) return;

    // Backdrop
    SolidBrush backdrop(Color((BYTE)(pAlpha * 80), 0, 0, 0));
    g->FillRectangle(&backdrop, 0, 0, (float)g_width, (float)g_height);

    // Shadow
    DrawDropShadow(g, pX, pY, pW, pH, pR, 25, 0, (int)(pAlpha * 100));

    // Background
    SolidBrush pBg(Color((BYTE)(pAlpha * 240), 35, 35, 48));
    FillRoundRect(g, &pBg, pX, pY, pW, pH, pR);

    // Glass
    LinearGradientBrush pGlass(
        PointF(pX, pY), PointF(pX, pY + pH * 0.3f),
        Color((BYTE)(pAlpha * 10), 80, 100, 140),
        Color((BYTE)(pAlpha * 0), 0, 0, 0));
    GraphicsPath pPath = CreateRoundRectPath(pX, pY, pW, pH, pR);
    g->FillPath(&pGlass, &pPath);

    // Border
    Pen pBorder(Color((BYTE)(pAlpha * 35), 70, 80, 100), 1.0f);
    DrawRoundRect(g, &pBorder, pX, pY, pW, pH, pR);

    float padX = 20.0f;
    float curY = pY + 20.0f;

    // Header
    Font pHeaderFont(L"Segoe UI", 15, FONT_SEMIBOLD, UnitPixel);
    SolidBrush pHeaderColor(Color((BYTE)(pAlpha * 230), 225, 230, 245));
    StringFormat sf;
    sf.SetAlignment(StringAlignmentNear);
    sf.SetLineAlignment(StringAlignmentNear);
    g->DrawString(L"WiFi Networks", -1, &pHeaderFont,
                  RectF(pX + padX, curY, 200, 26), &sf, &pHeaderColor);
    curY += 34.0f;

    // Networks
    float itemH = 52.0f;
    for (int i = 0; i < g_wifiNetworkCount; i++) {
        const WifiNetwork& net = g_wifiNetworks[i];
        float iy = curY;

        bool hovered = g_hoveredWifiNetwork == i;
        if (g_mouse.x >= pX + padX && g_mouse.x <= pX + pW - padX &&
            g_mouse.y >= iy && g_mouse.y <= iy + itemH && !hovered) {
            g_hoveredWifiNetwork = i;
        }

        if (g_hoveredWifiNetwork == i) {
            SolidBrush hoverBg(Color((BYTE)(pAlpha * 25), 255, 255, 255));
            FillRoundRect(g, &hoverBg, pX + padX, iy, pW - padX * 2, itemH, 8);
        }

        // WiFi icon with signal bars
        Font wifiIconFont(L"Segoe MDL2 Assets", 16, FontStyleRegular, UnitPixel);
        int wifiAlpha = (int)(pAlpha * (hovered ? 220 : 160));
        SolidBrush wifiIconColor(Color((BYTE)wifiAlpha, 200, 210, 230));
        StringFormat wifiSf;
        wifiSf.SetAlignment(StringAlignmentCenter);
        wifiSf.SetLineAlignment(StringAlignmentCenter);

        const wchar_t* signalIcons[] = { L"\uEC1A", L"\uEC1B", L"\uEC1D", L"\uEC1F" };
        int sigIdx = min(net.signal - 1, 3);
        if (net.connected) sigIdx = 3;
        g->DrawString(signalIcons[sigIdx], -1, &wifiIconFont,
                      RectF(pX + padX + 8, iy, 30, itemH), &wifiSf, &wifiIconColor);

        // Network name
        Font netNameFont(L"Segoe UI", 13, net.connected ? FONT_SEMIBOLD : FontStyleRegular,
                         UnitPixel);
        SolidBrush netNameColor(Color((BYTE)(pAlpha * (net.connected ? 230 : 200)),
                                       220, 225, 240));
        g->DrawString(net.name.c_str(), (INT)net.name.length(), &netNameFont,
                      RectF(pX + padX + 44, iy + 4, pW - padX * 2 - 100, 22), &sf,
                      &netNameColor);

        // Security + status
        Font netStatusFont(L"Segoe UI", 10, FontStyleRegular, UnitPixel);
        SolidBrush netStatusColor(Color((BYTE)(pAlpha * 120), 140, 150, 170));
        std::wstring status = net.security;
        if (net.connected) status = L"Connected  \u2022  " + net.security;
        g->DrawString(status.c_str(), (INT)status.length(), &netStatusFont,
                      RectF(pX + padX + 44, iy + 28, pW - padX * 2 - 100, 18), &sf,
                      &netStatusColor);

        // Connected indicator
        if (net.connected) {
            SolidBrush connDot(Color((BYTE)(pAlpha * 200), 96, 220, 120));
            g->FillEllipse(&connDot, pX + pW - padX - 20, iy + itemH / 2.0f - 4, 8, 8);
        }

        curY += itemH;
    }
}

// ============================================================================
//  DRAW: DRAG SELECT RECTANGLE
// ============================================================================

static void DrawDragSelect(Graphics* g) {
    if (!g_dragSelecting) return;

    int left = min(g_dragSelectRect.left, g_dragSelectRect.right);
    int top = min(g_dragSelectRect.top, g_dragSelectRect.bottom);
    int w = abs(g_dragSelectRect.right - g_dragSelectRect.left);
    int h = abs(g_dragSelectRect.bottom - g_dragSelectRect.top);

    if (w < 2 || h < 2) return;

    // Selection rectangle
    SolidBrush selBg(Color(30, 96, 165, 250));
    g->FillRectangle(&selBg, left, top, w, h);

    Pen selBorder(Color(180, 120, 190, 255), 1.5f);
    selBorder.SetDashStyle(DashStyleDash);
    g->DrawRectangle(&selBorder, left, top, w, h);
}

// ============================================================================
//  POWER MENU ACTION EXECUTION
// ============================================================================

static void ExecutePowerAction(int action) {
    switch (action) {
        case 0: // Lock
            LockWorkStation();
            break;
        case 1: // Sign out
            ExitWindowsEx(EWX_LOGOFF, 0);
            break;
        case 2: // Sleep
            SetSuspendState(FALSE, TRUE, FALSE);
            break;
        case 3: // Restart
            ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
            break;
        case 4: // Shutdown
            ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
            break;
    }
    g_powerMenuOpen = false;
    g_startMenuOpen = false;
}

// ============================================================================
//  UPDATE ALL ANIMATIONS
// ============================================================================

static void UpdateAllAnimations() {
    // Delta time is set from the timer

    // Desktop icon hover animations
    for (int i = 0; i < 10; i++) {
        g_desktopIcons[i].hovered = false;
    }

    // Check desktop icon hovers
    float iconAreaHeight = (float)(g_height - g_taskbarHeight);
    for (int i = 0; i < 10; i++) {
        float ix = (float)g_desktopIcons[i].x;
        float iy = (float)g_desktopIcons[i].y;
        if (g_mouse.x >= ix - 5 && g_mouse.x <= ix + 75 &&
            g_mouse.y >= iy && g_mouse.y <= iy + 70 &&
            iy + 70 <= iconAreaHeight) {
            g_desktopIcons[i].hovered = true;
        }
        g_desktopIcons[i].hoverAnim.Update(g_deltaTime);
    }

    // Show desktop strip hover
    g_showDesktopHovered = (g_mouse.x >= 0 && g_mouse.x <= 6 &&
                            g_mouse.y >= g_height - g_taskbarHeight &&
                            g_mouse.y <= g_height);

    // Reset hover states for menus
    g_hoveredStartItem = -1;
    g_hoveredRecommendedItem = -1;
    g_hoveredContextItem = -1;
    g_hoveredWifiNetwork = -1;
    g_hoveredPowerItem = -1;
    g_hoveredNotifCloseBtn = -1;
}

// ============================================================================
//  MAIN RENDER
// ============================================================================

static void Render(Graphics* g) {
    g->SetSmoothingMode(SmoothingModeHighQuality);
    g->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    g->SetInterpolationMode(InterpolationModeHighQualityBicubic);

    // Clear
    g->Clear(Color(0, 0, 0));

    // Draw layers
    DrawDesktop(g);
    DrawDragSelect(g);
    DrawContextMenu(g);
    DrawNotifications(g);
    DrawMusicWidget(g);
    DrawWidgets(g);
    DrawWiFiPanel(g);
    DrawStartMenu(g);
    DrawPowerMenu(g);
    DrawTaskbar(g);
}

// ============================================================================
//  WINDOW PROCEDURE
// ============================================================================

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Initialize GDI+
            GdiplusStartupInput gdiplusStartupInput;
            GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

            InitDesktopIcons();

            // Add initial notification
            AddNotification(L"VORTEX Desktop v5.3",
                            L"Welcome to the improved desktop environment. Press N for notifications, SPACE for start menu.",
                            L"\uE946");

            // Start animation timer
            SetTimer(hWnd, 1, 16, NULL);
            break;
        }

        case WM_TIMER: {
            if (wParam == 1) {
                g_totalTime += g_deltaTime;

                UpdateAllAnimations();

                // Invalidate for repaint
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            Graphics g(hdc);
            Render(&g);

            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_SIZE: {
            g_width = LOWORD(lParam);
            g_height = HIWORD(lParam);
            break;
        }

        case WM_MOUSEMOVE: {
            g_mouse.x = GET_X_LPARAM(lParam);
            g_mouse.y = GET_Y_LPARAM(lParam);

            if (g_mouseDown && !g_mouseDragged) {
                float dx = (float)(g_mouse.x - g_mouseDownPos.x);
                float dy = (float)(g_mouse.y - g_mouseDownPos.y);
                if (dx * dx + dy * dy > 25.0f) {
                    g_mouseDragged = true;
                }
            }

            if (g_mouseDragged && g_mouseDown) {
                g_dragSelecting = true;
                g_dragSelectRect.left = g_mouseDownPos.x;
                g_dragSelectRect.top = g_mouseDownPos.y;
                g_dragSelectRect.right = g_mouse.x;
                g_dragSelectRect.bottom = g_mouse.y;
            }

            break;
        }

        case WM_LBUTTONDOWN: {
            g_mouseDown = true;
            g_mouseDragged = false;
            g_mouseDownPos.x = GET_X_LPARAM(lParam);
            g_mouseDownPos.y = GET_Y_LPARAM(lParam);
            break;
        }

        case WM_LBUTTONUP: {
            if (!g_mouseDragged) {
                int mx = GET_X_LPARAM(lParam);
                int my = GET_Y_LPARAM(lParam);

                // -- Check notification close buttons --
                for (int i = 0; i < g_notificationCount; i++) {
                    if (g_hoveredNotifCloseBtn == i) {
                        CloseNotification(i);
                        break;
                    }
                }

                // -- Check power menu items --
                if (g_powerMenuOpen && g_hoveredPowerItem >= 0) {
                    int action = g_powerMenuItems[g_hoveredPowerItem].action;
                    ExecutePowerAction(action);
                    break;
                }

                // -- Check power button in start menu --
                float menuW = 620.0f;
                float menuH = 580.0f;
                float menuX = (float)g_width / 2.0f - menuW / 2.0f;
                float menuBaseY = (float)g_height - g_taskbarHeight - menuH - 12.0f;

                if (g_startMenuOpen && !g_powerMenuOpen) {
                    float powerBtnW = 36.0f;
                    float powerBtnH = 32.0f;
                    float powerBtnX = menuX + menuW - 24.0f - powerBtnW;
                    float bottomY = menuBaseY + menuH - 52.0f;
                    float powerBtnY = bottomY + 10.0f;

                    if (mx >= powerBtnX - 4 && mx <= powerBtnX + powerBtnW + 4 &&
                        my >= powerBtnY - 4 && my <= powerBtnY + powerBtnH + 4) {
                        g_powerMenuOpen = !g_powerMenuOpen;
                        break;
                    }
                }

                // -- Close power menu if clicking elsewhere in start menu --
                if (g_powerMenuOpen && g_startMenuOpen) {
                    g_powerMenuOpen = false;
                }

                // -- Check start button --
                float startX = 12.0f;
                float startBtnW = 44.0f;
                float startBtnH = 36.0f;
                float startBtnY = (float)(g_height - g_taskbarHeight) +
                                  (float)(g_taskbarHeight - (int)startBtnH) / 2.0f;

                if (mx >= startX && mx <= startX + startBtnW &&
                    my >= startBtnY && my <= startBtnY + startBtnH) {
                    g_startMenuOpen = !g_startMenuOpen;
                    g_powerMenuOpen = false;
                    g_searchActive = false;
                    g_searchQuery.clear();
                    if (g_startMenuOpen) g_startMenuOpenTime = g_totalTime;
                    break;
                }

                // -- Show Desktop strip --
                if (mx >= 0 && mx <= 6 && my >= g_height - g_taskbarHeight &&
                    my <= g_height) {
                    // Minimize all (simulate)
                    g_startMenuOpen = false;
                    g_powerMenuOpen = false;
                    g_contextMenuOpen = false;
                    g_widgetsOpen = false;
                    g_musicOpen = false;
                    g_wifiOpen = false;
                    break;
                }

                // -- Close start menu if clicking outside --
                if (g_startMenuOpen &&
                    (mx < menuX || mx > menuX + menuW ||
                     my < menuBaseY || my > menuBaseY + menuH)) {
                    // But check if clicking on power menu area
                    if (!g_powerMenuOpen) {
                        g_startMenuOpen = false;
                        g_powerMenuOpen = false;
                        g_searchActive = false;
                        g_searchQuery.clear();
                    }
                }

                // -- Context menu --
                if (g_contextMenuOpen) {
                    g_contextMenuOpen = false;
                    break;
                }

                // -- WiFi panel --
                if (g_wifiOpen) {
                    g_wifiOpen = false;
                    break;
                }

                // -- Right-click context menu on desktop --
                // (handled in WM_RBUTTONUP)
            }

            g_mouseDown = false;
            g_mouseDragged = false;
            g_dragSelecting = false;
            break;
        }

        case WM_RBUTTONUP: {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);
            float tbTop = (float)(g_height - g_taskbarHeight);

            // Only show context menu on desktop area
            if (my < tbTop) {
                // Close other panels
                g_startMenuOpen = false;
                g_powerMenuOpen = false;
                g_wifiOpen = false;

                g_contextMenuOpen = true;
                g_contextMenuPos.x = mx;
                g_contextMenuPos.y = my;
                g_contextMenuOpenTime = g_totalTime;
            }
            break;
        }

        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_ESCAPE:
                    if (g_powerMenuOpen) {
                        g_powerMenuOpen = false;
                    } else if (g_startMenuOpen) {
                        g_startMenuOpen = false;
                        g_powerMenuOpen = false;
                        g_searchActive = false;
                        g_searchQuery.clear();
                    } else if (g_contextMenuOpen) {
                        g_contextMenuOpen = false;
                    } else if (g_wifiOpen) {
                        g_wifiOpen = false;
                    } else if (g_widgetsOpen) {
                        g_widgetsOpen = false;
                    } else if (g_musicOpen) {
                        g_musicOpen = false;
                    } else {
                        DestroyWindow(hWnd);
                    }
                    break;

                case VK_SPACE:
                    g_startMenuOpen = !g_startMenuOpen;
                    g_powerMenuOpen = false;
                    g_searchActive = false;
                    g_searchQuery.clear();
                    if (g_startMenuOpen) g_startMenuOpenTime = g_totalTime;
                    break;

                case 0x4D: // M - music widget
                    g_musicOpen = !g_musicOpen;
                    break;

                case 0x57: // W - widgets
                    g_widgetsOpen = !g_widgetsOpen;
                    break;

                case 0x4E: // N - new notification
                    AddNotification(
                        L"System Update Available",
                        L"A new system update is ready to install. Restart to apply changes.",
                        L"\uE7E8");
                    break;

                case 0x46: // F - WiFi panel
                    g_wifiOpen = !g_wifiOpen;
                    break;
            }

            // Handle search input in start menu
            if (g_startMenuOpen && g_searchActive) {
                if (wParam == VK_BACK) {
                    if (!g_searchQuery.empty()) {
                        g_searchQuery.pop_back();
                    }
                } else if (wParam == VK_ESCAPE) {
                    g_searchActive = false;
                    g_searchQuery.clear();
                } else if (wParam == VK_RETURN) {
                    // Could open filtered app
                    g_searchActive = false;
                }
            }
            break;
        }

        case WM_CHAR: {
            // Handle search text input
            if (g_startMenuOpen && g_searchActive) {
                wchar_t ch = (wchar_t)wParam;
                if (ch >= 32 && ch <= 126 || ch >= 0x80) {
                    g_searchQuery += ch;
                }
            }
            break;
        }

        case WM_LBUTTONDBLCLK: {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);

            // Double click on search bar activates search
            if (g_startMenuOpen) {
                float menuW = 620.0f;
                float menuX = (float)g_width / 2.0f - menuW / 2.0f;
                float menuH = 580.0f;
                float menuBaseY = (float)g_height - g_taskbarHeight - menuH - 12.0f;
                float startAnim = g_startMenuAnim.current;
                float springOffset = (1.0f - pOutBack(pClamp(startAnim, 0, 1))) * 40.0f;
                float menuY = menuBaseY + springOffset;

                float padX = 24.0f;
                float searchH = 36.0f;
                float searchY = menuY + 20.0f;

                if (mx >= menuX + padX && mx <= menuX + menuW - padX &&
                    my >= searchY && my <= searchY + searchH) {
                    g_searchActive = !g_searchActive;
                    if (!g_searchActive) g_searchQuery.clear();
                }
            }

            // Double click on desktop icon
            for (int i = 0; i < 10; i++) {
                float ix = (float)g_desktopIcons[i].x;
                float iy = (float)g_desktopIcons[i].y;
                if (mx >= ix && mx <= ix + 70 && my >= iy && my <= iy + 70) {
                    // Simulate open
                    AddNotification(L"Opening " + g_desktopIcons[i].label,
                                    L"Application is starting...",
                                    g_desktopIcons[i].icon);
                    break;
                }
            }

            break;
        }

        case WM_DESTROY: {
            KillTimer(hWnd, 1);
            GdiplusShutdown(g_gdiplusToken);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// ============================================================================
//  WINMAIN
// ============================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    // Register window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszClassName = L"VortexDesktop";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"Failed to register window class.", L"Error", MB_ICONERROR);
        return 1;
    }

    // Get screen dimensions
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    // Create window
    g_hWnd = CreateWindowExW(
        0, L"VortexDesktop", L"VORTEX Desktop v5.3",
        WS_POPUP | WS_VISIBLE,
        0, 0, screenW, screenH,
        NULL, NULL, hInstance, NULL);

    if (!g_hWnd) {
        MessageBoxW(NULL, L"Failed to create window.", L"Error", MB_ICONERROR);
        return 1;
    }

    // Make truly fullscreen
    SetWindowPos(g_hWnd, HWND_TOPMOST, 0, 0, screenW, screenH, SWP_FRAMECHANGED);

    // DWM extended frame for potential blur (if DWM supports it)
    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(g_hWnd, &margins);

    g_width = screenW;
    g_height = screenH;

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
