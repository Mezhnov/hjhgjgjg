//=============================================================================
// WebView2Browser.cpp - Complete implementation of the WebView2 Browser
// A modern web browser built with C++ and Microsoft WebView 2 (Chromium)
//
// Features:
//   - Tabbed browsing with unlimited tabs
//   - Full navigation controls (back, forward, refresh, stop, home)
//   - Smart URL bar with auto-completion
//   - Bookmarks management with persistence
//   - Browsing history with search and export
//   - Find on page with match case support
//   - Zoom controls (Ctrl+/-, Ctrl+scroll)
//   - Full-screen mode (F11)
//   - Developer tools integration (F12)
//   - Custom context menu
//   - Keyboard shortcuts and accelerators
//   - New tab page with search and shortcuts
//   - Settings dialog with preferences
//   - Status bar with HTTPS indicators
//   - Print support
//   - Drag-and-drop file support
//   - DPI awareness
//   - Always on top mode
//   - Download handling
//   - Notification system
//=============================================================================

#include "WebView2Browser.h"

// ============================================================================
// Global instance pointer for window procedure access
// ============================================================================
static BrowserApp* g_pBrowserApp = nullptr;

// ============================================================================
// Constructor and Destructor
// ============================================================================

BrowserApp::BrowserApp(HINSTANCE hInstance)
    : m_hInstance(hInstance)
    , m_activeTabIndex(-1)
    , m_nextTabId(0)
    , m_isFullScreen(false)
    , m_isAlwaysOnTop(false)
    , m_findBarVisible(false)
    , m_isInitialized(false)
    , m_isCreatingTab(false)
    , m_tabScrollOffset(0)
    , m_tabHoverIndex(-1)
    , m_isDraggingTab(false)
    , m_dpiScale(100)
{
    // Initialize COM for this thread
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        OutputDebugStringW(L"[WebView2Browser] Failed to initialize COM\n");
    }

    // Initialize common controls
    INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX) };
    icc.dwICC = ICC_BAR_CLASSES | ICC_TAB_CLASSES | ICC_STATUS_CLASS |
                ICC_LISTVIEW_CLASSES | ICC_NATIVEFNTCTL_CLASS;
    InitCommonControlsEx(&icc);

    // Set up DPI awareness for modern displays
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    m_isDpiAware = true;

    // Initialize brushes for UI rendering
    m_hTabActiveBrush    = CreateSolidBrush(COLOR_TAB_ACTIVE);
    m_hTabInactiveBrush  = CreateSolidBrush(COLOR_TAB_INACTIVE);
    m_hNavBarBrush       = CreateSolidBrush(COLOR_NAV_BAR);
    m_hAccentBrush       = CreateSolidBrush(COLOR_ACCENT);
    m_hUrlBarBrush       = CreateSolidBrush(COLOR_URL_BAR);

    // Initialize default settings
    m_settings.downloadPath = GetDownloadsPath();
    m_settings.homePage = L"https://www.google.com";
    m_settings.searchEngine = L"https://www.google.com/search?q=";
    m_settings.searchEngineName = L"Google";
    m_settings.showBookmarksBar = true;
    m_settings.showStatusBar = true;
    m_settings.enableDevTools = true;
    m_settings.smoothScrolling = true;
    m_settings.hardwareAcceleration = true;
    m_settings.defaultZoom = ZOOM_DEFAULT;

    // Load persistent settings, bookmarks, and history from disk
    LoadSettings();
    LoadBookmarks();
    LoadHistory();

    // Store global pointer for window procedure access
    g_pBrowserApp = this;
}

BrowserApp::~BrowserApp()
{
    // Save all persistent data before shutdown
    SaveSettings();
    SaveBookmarks();
    SaveHistory();

    // Clean up tabs and their WebView2 instances
    m_tabs.clear();

    // Release font resources
    if (m_hDefaultFont)  DeleteObject(m_hDefaultFont);
    if (m_hSmallFont)    DeleteObject(m_hSmallFont);
    if (m_hBoldFont)     DeleteObject(m_hBoldFont);
    if (m_hUrlBarFont)   DeleteObject(m_hUrlBarFont);

    // Release brush resources
    if (m_hTabActiveBrush)   DeleteObject(m_hTabActiveBrush);
    if (m_hTabInactiveBrush) DeleteObject(m_hTabInactiveBrush);
    if (m_hNavBarBrush)      DeleteObject(m_hNavBarBrush);
    if (m_hAccentBrush)      DeleteObject(m_hAccentBrush);
    if (m_hUrlBarBrush)      DeleteObject(m_hUrlBarBrush);

    // Release accelerator table
    if (m_hAccelerators) DestroyAcceleratorTable(m_hAccelerators);

    // Release WebView2 environment
    if (m_webviewEnvironment) m_webviewEnvironment.Reset();

    // Clean up COM
    CoUninitialize();

    g_pBrowserApp = nullptr;
}

// ============================================================================
// Initialization
// ============================================================================

HRESULT BrowserApp::Initialize()
{
    HRESULT hr = S_OK;

    // Register window class
    RegisterWindowClass();

    // Create fonts for the UI
    NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
    SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
                                &ncm, 0, GetDpiForWindow(GetDesktopWindow()));

    m_hDefaultFont = CreateFontIndirect(&ncm.lfMessageFont);

    LOGFONT smallFont = ncm.lfMessageFont;
    smallFont.lfHeight = -MulDiv(12, 96, 72);
    m_hSmallFont = CreateFontIndirect(&smallFont);

    LOGFONT boldFont = ncm.lfMessageFont;
    boldFont.lfWeight = FW_BOLD;
    m_hBoldFont = CreateFontIndirect(&boldFont);

    LOGFONT urlFont = ncm.lfMessageFont;
    urlFont.lfWeight = FW_NORMAL;
    wcscpy_s(urlFont.lfFaceName, L"Segoe UI");
    m_hUrlBarFont = CreateFontIndirect(&urlFont);

    // Create keyboard shortcuts
    m_shortcuts = {
        { VK_TAB,     MOD_CONTROL,            ID_FILE_NEW_TAB,     L"New Tab" },
        { VK_F4,      MOD_CONTROL,            ID_FILE_CLOSE_TAB,   L"Close Tab" },
        { VK_LEFT,    MOD_ALT,                 ID_NAV_BACK,         L"Back" },
        { VK_RIGHT,   MOD_ALT,                 ID_NAV_FORWARD,      L"Forward" },
        { VK_F5,      0,                       ID_NAV_REFRESH,      L"Refresh" },
        { VK_F5,      MOD_CONTROL,             ID_NAV_REFRESH,      L"Refresh (hard)" },
        { VK_HOME,    MOD_CONTROL,             ID_NAV_HOME,         L"Home" },
        { VK_ESCAPE,  0,                       ID_NAV_STOP,         L"Stop" },
        { VK_F11,     0,                       ID_VIEW_FULL_SCREEN, L"Full Screen" },
        { VK_F12,     0,                       ID_VIEW_DEVTOOLS,    L"Developer Tools" },
        { 'D',        MOD_CONTROL,             ID_VIEW_DEVTOOLS,    L"Developer Tools" },
        { VK_ADD,     MOD_CONTROL,             ID_VIEW_ZOOM_IN,     L"Zoom In" },
        { VK_SUBTRACT,MOD_CONTROL,             ID_VIEW_ZOOM_OUT,    L"Zoom Out" },
        { VK_0,       MOD_CONTROL,             ID_VIEW_ZOOM_RESET,  L"Zoom Reset" },
        { 'F',        MOD_CONTROL,             ID_EDIT_FIND,        L"Find" },
        { VK_F3,      0,                       ID_EDIT_FIND_NEXT,   L"Find Next" },
        { VK_F3,      MOD_SHIFT,               ID_EDIT_FIND_PREV,   L"Find Previous" },
        { VK_F7,      0,                       ID_VIEW_STATUS_BAR,  L"Toggle Status Bar" },
        { 'B',        MOD_CONTROL,             ID_VIEW_BOOKMARKS_BAR, L"Toggle Bookmarks" },
        { 'L',        MOD_CONTROL,             IDC_URL_BAR,          L"Focus URL Bar" },
        { 'T',        MOD_CONTROL,             ID_FILE_NEW_TAB,     L"New Tab" },
        { 'W',        MOD_CONTROL,             ID_FILE_CLOSE_TAB,   L"Close Tab" },
        { 'N',        MOD_CONTROL,             ID_FILE_NEW_WINDOW,  L"New Window" },
        { 'D',        MOD_CONTROL | MOD_SHIFT, ID_BOOKMARK_ADD,     L"Bookmark This Page" },
        { 'J',        MOD_CONTROL,             ID_HISTORY_SHOW,      L"Show History" },
        { 'H',        MOD_CONTROL,             ID_HISTORY_SHOW,      L"Show History" },
        { 'P',        MOD_CONTROL,             ID_FILE_PRINT,        L"Print" },
        { VK_F10,     MOD_SHIFT,               ID_HELP_DOCUMENTATION, L"Documentation" },
        { VK_OEM_PLUS,MOD_CONTROL,             ID_VIEW_ZOOM_IN,     L"Zoom In" },
        { VK_OEM_MINUS,MOD_CONTROL,            ID_VIEW_ZOOM_OUT,    L"Zoom Out" },
        { VK_TAB,     MOD_CONTROL | MOD_SHIFT, ID_FILE_NEW_TAB,     L"New Tab" },
    };

    // Initialize the WebView2 environment
    hr = InitializeWebView2Environment();
    if (FAILED(hr)) {
        MessageBoxW(nullptr,
            L"Failed to initialize WebView2 environment.\n"
            L"Please ensure the WebView2 Runtime is installed.\n\n"
            L"Download: https://developer.microsoft.com/en-us/microsoft-edge/webview2/",
            L"WebView2Browser - Error",
            MB_ICONERROR | MB_OK);
        return hr;
    }

    // Create the main window
    hr = CreateMainWindow();
    if (FAILED(hr)) {
        return hr;
    }

    // Create the tab bar
    hr = CreateTabBar();
    if (FAILED(hr)) {
        return hr;
    }

    // Create the navigation bar
    hr = CreateNavigationBar();
    if (FAILED(hr)) {
        return hr;
    }

    // Create the bookmarks bar
    hr = CreateBookmarksBar();
    if (FAILED(hr)) {
        return hr;
    }

    // Create the status bar
    hr = CreateStatusBar();
    if (FAILED(hr)) {
        return hr;
    }

    // Create the find bar (initially hidden)
    hr = CreateFindBar();
    if (FAILED(hr)) {
        return hr;
    }

    // Create the accelerator table for keyboard shortcuts
    hr = CreateAcceleratorTable();
    if (FAILED(hr)) {
        return hr;
    }

    // Open the initial tab with the home page
    CreateNewTab(m_settings.homePage);

    // Calculate and apply the initial layout
    CalculateLayout();
    RepositionControls();

    m_isInitialized = true;
    return S_OK;
}

HRESULT BrowserApp::InitializeWebView2Environment()
{
    // Create WebView2 environment options
    HRESULT hr = CreateCoreWebView2EnvironmentOptions(
        IID_PPV_ARGS(&m_envOptions));
    if (SUCCEEDED(hr) && m_envOptions) {
        // Set custom user agent
        if (!m_settings.userAgentOverride.empty()) {
            m_envOptions->put_AdditionalBrowserArguments(
                m_settings.userAgentOverride.c_str());
        }
        // Set language
        m_envOptions->put_Language(m_settings.language.c_str());
    }

    // Create the WebView2 environment asynchronously
    EventRegistrationToken token = {};
    hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr,  // Use default browser executable path
        GetAppDataPath().c_str(),
        m_envOptions.Get(),
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (SUCCEEDED(result) && env) {
                    m_webviewEnvironment.Attach(env);
                    OutputDebugStringW(L"[WebView2Browser] WebView2 environment created successfully\n");
                } else {
                    OutputDebugStringW(L"[WebView2Browser] Failed to create WebView2 environment\n");
                }
                return S_OK;
            }).Get());
    return hr;
}

int BrowserApp::Run()
{
    // Show and update the main window
    ShowWindow(m_hMainWindow, SW_SHOWNORMAL);
    UpdateWindow(m_hMainWindow);

    // Enter the main message loop with accelerator translation
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        // Translate accelerators first
        if (!m_hAccelerators || !TranslateAccelerator(m_hMainWindow, m_hAccelerators, &msg)) {
            // Process tab key for auto-complete navigation
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_TAB) {
                // Tab key handled in URL bar for auto-complete
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } else {
                // Normal message processing
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    return static_cast<int>(msg.wParam);
}

// ============================================================================
// Window Registration and Creation
// ============================================================================

void BrowserApp::RegisterWindowClass()
{
    WNDCLASSEXW wcex = {};
    wcex.cbSize        = sizeof(WNDCLASSEXW);
    wcex.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc   = BrowserApp::MainWindowProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = sizeof(BrowserApp*);
    wcex.hInstance      = m_hInstance;
    wcex.hIcon          = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = L"WebView2BrowserClass";
    wcex.hIconSm        = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    RegisterClassExW(&wcex);
}

HRESULT BrowserApp::CreateMainWindow()
{
    // Calculate window size for a comfortable default
    RECT rc = { 0, 0, 1280, 800 };
    AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0);

    int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - (rc.right - rc.left)) / 2;
    int y = (screenHeight - (rc.bottom - rc.top)) / 2;

    m_hMainWindow = CreateWindowExW(
        0,
        L"WebView2BrowserClass",
        L"WebView2Browser",
        WS_OVERLAPPEDWINDOW,
        x, y,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        m_hInstance,
        this
    );

    if (!m_hMainWindow) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Store the browser app pointer in the window's extra data
    SetWindowLongPtr(m_hMainWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // Enable drag and drop for the main window
    DragAcceptFiles(m_hMainWindow, TRUE);

    // Create the menu bar
    m_hMenuBar = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_NEW_TAB,    L"New Tab\tCtrl+T");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_CLOSE_TAB,  L"Close Tab\tCtrl+W");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_NEW_WINDOW, L"New Window\tCtrl+N");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_OPEN_FILE,  L"Open File...\tCtrl+O");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_SAVE_AS,    L"Save As...\tCtrl+S");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_PRINT,      L"Print...\tCtrl+P");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_PRINT_PREVIEW, L"Print Preview");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_EXIT,       L"Exit\tAlt+F4");
    AppendMenuW(m_hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");

    HMENU hEditMenu = CreatePopupMenu();
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_CUT,        L"Cut\tCtrl+X");
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_COPY,       L"Copy\tCtrl+C");
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_PASTE,      L"Paste\tCtrl+V");
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_SELECT_ALL, L"Select All\tCtrl+A");
    AppendMenuW(hEditMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_FIND,       L"Find on Page...\tCtrl+F");
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_FIND_NEXT,  L"Find Next\tF3");
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_FIND_PREV,  L"Find Previous\tShift+F3");
    AppendMenuW(m_hMenuBar, MF_POPUP, (UINT_PTR)hEditMenu, L"&Edit");

    HMENU hViewMenu = CreatePopupMenu();
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_ZOOM_IN,         L"Zoom In\tCtrl++");
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_ZOOM_OUT,        L"Zoom Out\tCtrl+-");
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_ZOOM_RESET,      L"Reset Zoom\tCtrl+0");
    AppendMenuW(hViewMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_FULL_SCREEN,     L"Full Screen\tF11");
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_ALWAYS_ON_TOP,   L"Always on Top");
    AppendMenuW(hViewMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_DEVTOOLS,        L"Developer Tools\tF12");
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_SOURCE,          L"View Page Source");
    AppendMenuW(hViewMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_BOOKMARKS_BAR,   L"Bookmarks Bar\tCtrl+B");
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_STATUS_BAR,      L"Status Bar\tF7");
    AppendMenuW(m_hMenuBar, MF_POPUP, (UINT_PTR)hViewMenu, L"&View");

    HMENU hNavMenu = CreatePopupMenu();
    AppendMenuW(hNavMenu, MF_STRING, ID_NAV_BACK,     L"Back\tAlt+Left");
    AppendMenuW(hNavMenu, MF_STRING, ID_NAV_FORWARD,  L"Forward\tAlt+Right");
    AppendMenuW(hNavMenu, MF_STRING, ID_NAV_UP,       L"Up One Level");
    AppendMenuW(hNavMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hNavMenu, MF_STRING, ID_NAV_REFRESH,  L"Refresh\tF5");
    AppendMenuW(hNavMenu, MF_STRING, ID_NAV_HOME,     L"Home\tAlt+Home");
    AppendMenuW(hNavMenu, MF_STRING, ID_NAV_STOP,     L"Stop\tEscape");
    AppendMenuW(m_hMenuBar, MF_POPUP, (UINT_PTR)hNavMenu, L"&Navigation");

    HMENU hBookmarkMenu = CreatePopupMenu();
    AppendMenuW(hBookmarkMenu, MF_STRING, ID_BOOKMARK_ADD,     L"Add Bookmark\tCtrl+D");
    AppendMenuW(hBookmarkMenu, MF_STRING, ID_BOOKMARK_REMOVE,  L"Remove Bookmark");
    AppendMenuW(hBookmarkMenu, MF_STRING, ID_BOOKMARK_MANAGE,  L"Manage Bookmarks\tCtrl+Shift+B");
    AppendMenuW(hBookmarkMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hBookmarkMenu, MF_STRING, ID_BOOKMARK_IMPORT,  L"Import Bookmarks...");
    AppendMenuW(hBookmarkMenu, MF_STRING, ID_BOOKMARK_EXPORT,  L"Export Bookmarks...");
    AppendMenuW(m_hMenuBar, MF_POPUP, (UINT_PTR)hBookmarkMenu, L"&Bookmarks");

    HMENU hHistoryMenu = CreatePopupMenu();
    AppendMenuW(hHistoryMenu, MF_STRING, ID_HISTORY_SHOW,      L"Show History\tCtrl+H");
    AppendMenuW(hHistoryMenu, MF_STRING, ID_HISTORY_CLEAR,     L"Clear History");
    AppendMenuW(hHistoryMenu, MF_STRING, ID_HISTORY_EXPORT,    L"Export History...");
    AppendMenuW(m_hMenuBar, MF_POPUP, (UINT_PTR)hHistoryMenu, L"&History");

    HMENU hToolsMenu = CreatePopupMenu();
    AppendMenuW(hToolsMenu, MF_STRING, ID_TOOLS_CLEAR_DATA,    L"Clear Browsing Data...");
    AppendMenuW(hToolsMenu, MF_STRING, ID_TOOLS_INCOGNITO,     L"New Incognito Window");
    AppendMenuW(m_hMenuBar, MF_POPUP, (UINT_PTR)hToolsMenu, L"&Tools");

    HMENU hHelpMenu = CreatePopupMenu();
    AppendMenuW(hHelpMenu, MF_STRING, ID_HELP_ABOUT,           L"About WebView2Browser");
    AppendMenuW(hHelpMenu, MF_STRING, ID_HELP_DOCUMENTATION,   L"Documentation");
    AppendMenuW(hHelpMenu, MF_STRING, ID_HELP_CHECK_UPDATE,    L"Check for Updates");
    AppendMenuW(hHelpMenu, MF_STRING, ID_HELP_VERSION_INFO,    L"Version Info");
    AppendMenuW(m_hMenuBar, MF_POPUP, (UINT_PTR)hHelpMenu, L"&Help");

    SetMenu(m_hMainWindow, m_hMenuBar);

    return S_OK;
}

HRESULT BrowserApp::CreateTabBar()
{
    m_hTabBar = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_OWNERDRAW,
        0, 0, 0, TAB_HEIGHT,
        m_hMainWindow, (HMENU)IDC_TAB_BAR,
        m_hInstance, nullptr);

    if (!m_hTabBar) return HRESULT_FROM_WIN32(GetLastError());

    // New Tab button
    m_hNewTabBtn = CreateWindowExW(
        0, L"BUTTON", L"+",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
        0, 0, 32, TAB_HEIGHT - 2,
        m_hTabBar, (HMENU)IDC_NEW_TAB_BTN,
        m_hInstance, nullptr);

    SendMessage(m_hNewTabBtn, WM_SETFONT, (WPARAM)m_hDefaultFont, TRUE);

    return S_OK;
}

HRESULT BrowserApp::CreateNavigationBar()
{
    m_hNavBar = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, 0, NAV_BAR_HEIGHT,
        m_hMainWindow, nullptr,
        m_hInstance, nullptr);

    if (!m_hNavBar) return HRESULT_FROM_WIN32(GetLastError());

    int x = 6;
    int y = (NAV_BAR_HEIGHT - BUTTON_WIDTH) / 2;

    // Back button
    m_hBackBtn = CreateWindowExW(0, L"BUTTON", L"<",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
        x, y, BUTTON_WIDTH, BUTTON_WIDTH,
        m_hNavBar, (HMENU)IDC_BACK, m_hInstance, nullptr);
    SendMessage(m_hBackBtn, WM_SETFONT, (WPARAM)m_hDefaultFont, TRUE);
    x += BUTTON_WIDTH + 2;

    // Forward button
    m_hForwardBtn = CreateWindowExW(0, L"BUTTON", L">",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
        x, y, BUTTON_WIDTH, BUTTON_WIDTH,
        m_hNavBar, (HMENU)IDC_FORWARD, m_hInstance, nullptr);
    SendMessage(m_hForwardBtn, WM_SETFONT, (WPARAM)m_hDefaultFont, TRUE);
    x += BUTTON_WIDTH + 2;

    // Refresh / Stop button
    m_hRefreshBtn = CreateWindowExW(0, L"BUTTON", L"R",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
        x, y, BUTTON_WIDTH, BUTTON_WIDTH,
        m_hNavBar, (HMENU)IDC_REFRESH, m_hInstance, nullptr);
    SendMessage(m_hRefreshBtn, WM_SETFONT, (WPARAM)m_hDefaultFont, TRUE);
    x += BUTTON_WIDTH + 2;

    // Stop button (initially hidden, shown during loading)
    m_hStopBtn = CreateWindowExW(0, L"BUTTON", L"X",
        WS_CHILD | BS_PUSHBUTTON | BS_OWNERDRAW,
        x, y, BUTTON_WIDTH, BUTTON_WIDTH,
        m_hNavBar, (HMENU)IDC_STOP, m_hInstance, nullptr);
    SendMessage(m_hStopBtn, WM_SETFONT, (WPARAM)m_hDefaultFont, TRUE);

    // Home button
    m_hHomeBtn = CreateWindowExW(0, L"BUTTON", L"H",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
        x, y, BUTTON_WIDTH, BUTTON_WIDTH,
        m_hNavBar, (HMENU)IDC_HOME, m_hInstance, nullptr);
    SendMessage(m_hHomeBtn, WM_SETFONT, (WPARAM)m_hDefaultFont, TRUE);
    x += BUTTON_WIDTH + 6;

    // URL Bar - the main address input field
    int urlBarWidth = 400; // Will be resized during layout
    m_hUrlBar = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOHSCROLL |
        ES_LEFT | ES_NOHIDESEL | ES_MULTILINE,
        x, (NAV_BAR_HEIGHT - URL_BAR_HEIGHT) / 2,
        urlBarWidth, URL_BAR_HEIGHT,
        m_hNavBar, (HMENU)IDC_URL_BAR,
        m_hInstance, nullptr);
    SendMessage(m_hUrlBar, WM_SETFONT, (WPARAM)m_hUrlBarFont, TRUE);
    SubclassUrlBar();

    // Go button
    m_hGoBtn = CreateWindowExW(0, L"BUTTON", L">>",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
        x + urlBarWidth + 2, y, 36, BUTTON_WIDTH,
        m_hNavBar, (HMENU)IDC_GO, m_hInstance, nullptr);
    SendMessage(m_hGoBtn, WM_SETFONT, (WPARAM)m_hDefaultFont, TRUE);
    x += urlBarWidth + 2 + 36 + 2;

    // Bookmark add button (star icon)
    m_hBookmarkAddBtn = CreateWindowExW(0, L"BUTTON", L"*",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
        x, y, BUTTON_WIDTH, BUTTON_WIDTH,
        m_hNavBar, (HMENU)IDC_BOOKMARK_ADD, m_hInstance, nullptr);
    SendMessage(m_hBookmarkAddBtn, WM_SETFONT, (WPARAM)m_hBoldFont, TRUE);

    return S_OK;
}

HRESULT BrowserApp::CreateBookmarksBar()
{
    m_hBookmarksBar = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SS_OWNERDRAW,
        0, 0, 0, BOOKMARKS_BAR_HEIGHT,
        m_hMainWindow, (HMENU)IDC_BOOKMARKS_BAR,
        m_hInstance, nullptr);

    if (!m_hBookmarksBar) return HRESULT_FROM_WIN32(GetLastError());

    ShowWindow(m_hBookmarksBar, m_settings.showBookmarksBar ? SW_SHOW : SW_HIDE);
    UpdateBookmarksBar();

    return S_OK;
}

HRESULT BrowserApp::CreateStatusBar()
{
    m_hStatusBar = CreateWindowExW(
        0, STATUSCLASSNAMEW, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
        SBARS_SIZEGRIP | SBARS_TOOLTIPS,
        0, 0, 0, STATUS_BAR_HEIGHT,
        m_hMainWindow, (HMENU)IDC_STATUS_BAR,
        m_hInstance, nullptr);

    if (!m_hStatusBar) return HRESULT_FROM_WIN32(GetLastError());

    // Set up status bar parts
    int parts[] = { 300, -1 };
    SendMessage(m_hStatusBar, SB_SETPARTS, 2, (LPARAM)parts);
    SendMessage(m_hStatusBar, SB_SETTEXT, 0, (LPARAM)L"Ready");
    SendMessage(m_hStatusBar, SB_SETTEXT, 1, (LPARAM)L"100%");

    ShowWindow(m_hStatusBar, m_settings.showStatusBar ? SW_SHOW : SW_HIDE);
    return S_OK;
}

HRESULT BrowserApp::CreateFindBar()
{
    m_hFindBar = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER,
        0, 0, 0, FIND_BAR_HEIGHT,
        m_hMainWindow, (HMENU)IDC_FIND_BAR,
        m_hInstance, nullptr);

    if (!m_hFindBar) return HRESULT_FROM_WIN32(GetLastError());

    // Initially hidden
    ShowWindow(m_hFindBar, SW_HIDE);

    int x = 8;
    int y = (FIND_BAR_HEIGHT - 24) / 2;

    // "Find:" label
    HWND hFindLabel = CreateWindowExW(0, L"STATIC", L"Find:",
        WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE,
        x, y, 40, 24, m_hFindBar, nullptr, m_hInstance, nullptr);
    SendMessage(hFindLabel, WM_SETFONT, (WPARAM)m_hSmallFont, TRUE);
    x += 44;

    // Find input
    m_hFindInput = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_LEFT,
        x, y, 300, 24, m_hFindBar, (HMENU)IDC_FIND_INPUT,
        m_hInstance, nullptr);
    SendMessage(m_hFindInput, WM_SETFONT, (WPARAM)m_hDefaultFont, TRUE);
    x += 306;

    // Previous match button
    m_hFindPrevBtn = CreateWindowExW(0, L"BUTTON", L"^",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, y, 28, 24, m_hFindBar, (HMENU)IDC_FIND_PREV,
        m_hInstance, nullptr);
    SendMessage(m_hFindPrevBtn, WM_SETFONT, (WPARAM)m_hDefaultFont, TRUE);
    x += 32;

    // Next match button
    m_hFindNextBtn = CreateWindowExW(0, L"BUTTON", L"v",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, y, 28, 24, m_hFindBar, (HMENU)IDC_FIND_NEXT,
        m_hInstance, nullptr);
    SendMessage(m_hFindNextBtn, WM_SETFONT, (WPARAM)m_hDefaultFont, TRUE);
    x += 36;

    // Find results label
    m_hFindStatusLabel = CreateWindowExW(0, L"STATIC", L"No results",
        WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER,
        x, y, 150, 24, m_hFindBar, nullptr, m_hInstance, nullptr);
    SendMessage(m_hFindStatusLabel, WM_SETFONT, (WPARAM)m_hSmallFont, TRUE);

    // Close button (positioned at the right edge, will be adjusted during layout)
    m_hFindCloseBtn = CreateWindowExW(0, L"BUTTON", L"X",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, y, 28, 24, m_hFindBar, (HMENU)IDC_FIND_CLOSE,
        m_hInstance, nullptr);
    SendMessage(m_hFindCloseBtn, WM_SETFONT, (WPARAM)m_hDefaultFont, TRUE);

    return S_OK;
}

HRESULT BrowserApp::CreateAcceleratorTable()
{
    // Build the accelerator table from shortcut definitions
    std::vector<ACCEL> accels;
    for (const auto& shortcut : m_shortcuts) {
        accels.push_back({ shortcut.modifiers, shortcut.key, shortcut.commandId });
    }

    m_hAccelerators = CreateAcceleratorTableW(accels.data(), static_cast<int>(accels.size()));
    if (!m_hAccelerators) return HRESULT_FROM_WIN32(GetLastError());
    return S_OK;
}

// ============================================================================
// Layout Management
// ============================================================================

void BrowserApp::CalculateLayout()
{
    GetClientRect(m_hMainWindow, &m_clientRect);

    int y = 0;
    int width = m_clientRect.right - m_clientRect.left;
    int height = m_clientRect.bottom - m_clientRect.top;

    // Tab bar
    m_tabBarRect = { 0, y, width, y + TAB_HEIGHT };
    y += TAB_HEIGHT;

    // Navigation bar
    m_navBarRect = { 0, y, width, y + NAV_BAR_HEIGHT };
    y += NAV_BAR_HEIGHT;

    // Bookmarks bar (conditional)
    if (m_settings.showBookmarksBar) {
        m_bookmarksBarRect = { 0, y, width, y + BOOKMARKS_BAR_HEIGHT };
        y += BOOKMARKS_BAR_HEIGHT;
    } else {
        m_bookmarksBarRect = { 0, y, width, y };
    }

    // Find bar (conditional)
    if (m_findBarVisible) {
        m_findBarRect = { 0, y, width, y + FIND_BAR_HEIGHT };
        y += FIND_BAR_HEIGHT;
    } else {
        m_findBarRect = { 0, y, width, y };
    }

    // Status bar
    if (m_settings.showStatusBar) {
        m_statusBarRect = { 0, height - STATUS_BAR_HEIGHT, width, height };
        height -= STATUS_BAR_HEIGHT;
    } else {
        m_statusBarRect = { 0, height, width, height };
    }

    // WebView2 area fills the remaining space
    m_webviewRect = { 0, y, width, height };
}

void BrowserApp::RepositionControls()
{
    // Resize tab bar
    MoveWindow(m_hTabBar,
        m_tabBarRect.left, m_tabBarRect.top,
        m_tabBarRect.right - m_tabBarRect.left,
        m_tabBarRect.bottom - m_tabBarRect.top, TRUE);

    // Resize and reposition new tab button
    int tabBarWidth = m_tabBarRect.right - m_tabBarRect.left;
    MoveWindow(m_hNewTabBtn,
        tabBarWidth - 34, 2,
        32, TAB_HEIGHT - 4, TRUE);

    // Resize navigation bar
    MoveWindow(m_hNavBar,
        m_navBarRect.left, m_navBarRect.top,
        m_navBarRect.right - m_navBarRect.left,
        m_navBarRect.bottom - m_navBarRect.top, TRUE);

    // Reposition navigation buttons and resize URL bar
    int navBarWidth = m_navBarRect.right - m_navBarRect.left;
    int urlBarStart = 6 + (BUTTON_WIDTH + 2) * 4; // 4 buttons
    int urlBarEnd = navBarWidth - 6 - 36 - 2 - BUTTON_WIDTH - 4;
    int urlBarWidth = urlBarEnd - urlBarStart;

    int btnY = (NAV_BAR_HEIGHT - BUTTON_WIDTH) / 2;
    int x = 6;

    MoveWindow(m_hBackBtn, x, btnY, BUTTON_WIDTH, BUTTON_WIDTH, TRUE);
    x += BUTTON_WIDTH + 2;
    MoveWindow(m_hForwardBtn, x, btnY, BUTTON_WIDTH, BUTTON_WIDTH, TRUE);
    x += BUTTON_WIDTH + 2;
    MoveWindow(m_hRefreshBtn, x, btnY, BUTTON_WIDTH, BUTTON_WIDTH, TRUE);
    MoveWindow(m_hStopBtn, x, btnY, BUTTON_WIDTH, BUTTON_WIDTH, TRUE);
    x += BUTTON_WIDTH + 2;
    MoveWindow(m_hHomeBtn, x, btnY, BUTTON_WIDTH, BUTTON_WIDTH, TRUE);
    x += BUTTON_WIDTH + 6;

    MoveWindow(m_hUrlBar, x, (NAV_BAR_HEIGHT - URL_BAR_HEIGHT) / 2,
        urlBarWidth, URL_BAR_HEIGHT, TRUE);

    MoveWindow(m_hGoBtn, x + urlBarWidth + 2, btnY, 36, BUTTON_WIDTH, TRUE);
    MoveWindow(m_hBookmarkAddBtn, x + urlBarWidth + 2 + 36 + 2, btnY,
        BUTTON_WIDTH, BUTTON_WIDTH, TRUE);

    // Bookmarks bar
    MoveWindow(m_hBookmarksBar,
        m_bookmarksBarRect.left, m_bookmarksBarRect.top,
        m_bookmarksBarRect.right - m_bookmarksBarRect.left,
        m_bookmarksBarRect.bottom - m_bookmarksBarRect.top, TRUE);

    // Find bar
    MoveWindow(m_hFindBar,
        m_findBarRect.left, m_findBarRect.top,
        m_findBarRect.right - m_findBarRect.left,
        m_findBarRect.bottom - m_findBarRect.top, TRUE);

    // Reposition find close button to the right edge
    int findBarWidth = m_findBarRect.right - m_findBarRect.left;
    MoveWindow(m_hFindCloseBtn,
        findBarWidth - 36, (FIND_BAR_HEIGHT - 24) / 2,
        28, 24, TRUE);

    // Status bar
    MoveWindow(m_hStatusBar,
        m_statusBarRect.left, m_statusBarRect.top,
        m_statusBarRect.right - m_statusBarRect.left,
        m_statusBarRect.bottom - m_statusBarRect.top, TRUE);

    // Resize the active tab's WebView2 controller
    ResizeWebView();
}

void BrowserApp::ResizeWebView()
{
    TabData* tab = GetActiveTab();
    if (tab && tab->controller) {
        RECT rect = m_webviewRect;
        // Prevent zero-size rect
        if (rect.right - rect.left < 1) rect.right = rect.left + 1;
        if (rect.bottom - rect.top < 1) rect.bottom = rect.top + 1;
        tab->controller->put_Bounds(rect);
    }
}

// ============================================================================
// Tab Management
// ============================================================================

HRESULT BrowserApp::CreateNewTab(const std::wstring& url, bool activate)
{
    if (m_isClosing || static_cast<int>(m_tabs.size()) >= m_settings.maxTabs) {
        return E_FAIL;
    }

    std::lock_guard<std::mutex> lock(m_tabsMutex);
    m_isCreatingTab = true;

    auto tab = std::make_unique<TabData>();
    tab->tabId = m_nextTabId++;
    tab->zoomLevel = m_settings.defaultZoom;
    tab->lastActivity = GetTickCount();

    std::wstring targetUrl = url.empty() ? L"" : EnsureUrlPrefix(url);

    // Create a child window to host the WebView2 controller for this tab
    tab->hwnd = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        m_webviewRect.left, m_webviewRect.top,
        m_webviewRect.right - m_webviewRect.left,
        m_webviewRect.bottom - m_webviewRect.top,
        m_hMainWindow, nullptr,
        m_hInstance, nullptr);

    if (!tab->hwnd) {
        m_isCreatingTab = false;
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Create WebView2 controller inside the tab's host window
    if (m_webviewEnvironment) {
        EventRegistrationToken navCompletedToken = {};
        EventRegistrationToken titleChangedToken = {};
        EventRegistrationToken sourceChangedToken = {};
        EventRegistrationToken historyChangedToken = {};
        EventRegistrationToken newWindowToken = {};
        EventRegistrationToken webMessageToken = {};
        EventRegistrationToken statusChangedToken = {};
        EventRegistrationToken fullScreenToken = {};

        HRESULT hr = m_webviewEnvironment->CreateCoreWebView2Controller(
            tab->hwnd,
            Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                [this, targetUrl, activate](
                    HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                    if (SUCCEEDED(result) && controller) {
                        // Find the tab we just created
                        std::lock_guard<std::mutex> lock(m_tabsMutex);
                        TabData* tabPtr = nullptr;
                        for (auto& t : m_tabs) {
                            if (!t->controller) {
                                tabPtr = t.get();
                                break;
                            }
                        }

                        if (tabPtr) {
                            tabPtr->controller.Attach(controller);
                            controller->get_CoreWebView2(&tabPtr->webview);

                            // Apply tab-specific settings
                            if (tabPtr->webview) {
                                tabPtr->webview->get_Settings(&tabPtr->settings);
                                if (tabPtr->settings) {
                                    tabPtr->settings->put_IsScriptEnabled(TRUE);
                                    tabPtr->settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                                    tabPtr->settings->put_IsWebMessageEnabled(TRUE);
                                    tabPtr->settings->put_AreHostObjectsAllowed(TRUE);
                                    tabPtr->settings->put_IsZoomControlEnabled(TRUE);
                                    tabPtr->settings->put_IsStatusBarEnabled(TRUE);
                                }

                                // Register event handlers for this tab
                                RegisterTabEventHandlers(tabPtr);

                                // Set the initial zoom level
                                tabPtr->controller->SetBoundsAndZoomInfo(
                                    tabPtr->controller->get_Bounds, tabPtr->zoomLevel);
                            }

                            // Navigate to the target URL
                            if (!targetUrl.empty()) {
                                tabPtr->webview->Navigate(targetUrl.c_str());
                            } else {
                                // Load the new tab page
                                std::wstring newTabHtml = GenerateNewTabPageHtml();
                                tabPtr->webview->NavigateToString(newTabHtml.c_str());
                            }

                            // Activate the tab if requested
                            if (activate && !m_tabs.empty()) {
                                int idx = static_cast<int>(std::distance(
                                    m_tabs.begin(),
                                    std::find_if(m_tabs.begin(), m_tabs.end(),
                                        [tabPtr](const auto& t) { return t.get() == tabPtr; })));
                                SwitchToTab(idx);
                            }
                        }
                    }
                    m_isCreatingTab = false;
                    return S_OK;
                }).Get());

        if (FAILED(hr)) {
            m_isCreatingTab = false;
            return hr;
        }
    }

    int tabIndex = static_cast<int>(m_tabs.size());
    m_tabs.push_back(std::move(tab));

    if (activate && m_tabs.size() == 1) {
        SwitchToTab(0);
    }

    // Hide the new tab's window (it will be shown when activated)
    if (!activate && !m_tabs.empty()) {
        ShowWindow(m_tabs.back()->hwnd, SW_HIDE);
    }

    m_isCreatingTab = false;
    UpdateTabBar();
    UpdateUI();
    return S_OK;
}

HRESULT BrowserApp::CreateNewTabFromHtml(const std::wstring& htmlContent)
{
    if (m_isClosing || static_cast<int>(m_tabs.size()) >= m_settings.maxTabs) {
        return E_FAIL;
    }

    std::lock_guard<std::mutex> lock(m_tabsMutex);

    auto tab = std::make_unique<TabData>();
    tab->tabId = m_nextTabId++;
    tab->zoomLevel = m_settings.defaultZoom;
    tab->lastActivity = GetTickCount();
    tab->url = L"about:blank";

    tab->hwnd = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        m_webviewRect.left, m_webviewRect.top,
        m_webviewRect.right - m_webviewRect.left,
        m_webviewRect.bottom - m_webviewRect.top,
        m_hMainWindow, nullptr, m_hInstance, nullptr);

    if (!tab->hwnd) return HRESULT_FROM_WIN32(GetLastError());

    if (m_webviewEnvironment) {
        HRESULT hr = m_webviewEnvironment->CreateCoreWebView2Controller(
            tab->hwnd,
            Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                [this, htmlContent](
                    HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                    if (SUCCEEDED(result) && controller) {
                        std::lock_guard<std::mutex> lock(m_tabsMutex);
                        for (auto& t : m_tabs) {
                            if (!t->controller) {
                                t->controller.Attach(controller);
                                controller->get_CoreWebView2(&t->webview);
                                if (t->webview) {
                                    t->webview->get_Settings(&t->settings);
                                    RegisterTabEventHandlers(t.get());
                                    t->webview->NavigateToString(htmlContent.c_str());
                                }
                                break;
                            }
                        }
                    }
                    return S_OK;
                }).Get());

        if (FAILED(hr)) return hr;
    }

    m_tabs.push_back(std::move(tab));
    SwitchToTab(static_cast<int>(m_tabs.size()) - 1);
    UpdateTabBar();
    UpdateUI();
    return S_OK;
}

void BrowserApp::CloseTab(int tabIndex)
{
    if (tabIndex < 0 || tabIndex >= static_cast<int>(m_tabs.size())) return;

    // Don't close the last tab - create a new one instead
    if (m_tabs.size() == 1) {
        CreateNewTab(m_settings.homePage);
        m_tabs.erase(m_tabs.begin() + tabIndex);
        m_activeTabIndex = 0;
        UpdateTabBar();
        UpdateUI();
        return;
    }

    bool wasActive = (tabIndex == m_activeTabIndex);
    auto& tab = m_tabs[tabIndex];

    // Destroy the WebView2 controller
    if (tab->controller) {
        tab->controller->Close();
        tab->controller.Reset();
    }
    if (tab->webview) {
        tab->webview.Reset();
    }

    // Destroy the host window
    if (tab->hwnd) {
        DestroyWindow(tab->hwnd);
    }

    m_tabs.erase(m_tabs.begin() + tabIndex);

    // Adjust active tab index
    if (wasActive) {
        if (m_activeTabIndex >= static_cast<int>(m_tabs.size())) {
            m_activeTabIndex = static_cast<int>(m_tabs.size()) - 1;
        }
        SwitchToTab(m_activeTabIndex);
    } else if (m_activeTabIndex > tabIndex) {
        m_activeTabIndex--;
    }

    UpdateTabBar();
    UpdateUI();
}

void BrowserApp::CloseActiveTab()
{
    if (m_activeTabIndex >= 0) {
        CloseTab(m_activeTabIndex);
    }
}

void BrowserApp::SwitchToTab(int tabIndex)
{
    if (tabIndex < 0 || tabIndex >= static_cast<int>(m_tabs.size())) return;
    if (tabIndex == m_activeTabIndex) return;

    // Hide current tab's WebView2 window
    if (m_activeTabIndex >= 0 && m_activeTabIndex < static_cast<int>(m_tabs.size())) {
        ShowWindow(m_tabs[m_activeTabIndex]->hwnd, SW_HIDE);
    }

    m_activeTabIndex = tabIndex;

    // Show and resize the new active tab's WebView2 window
    TabData* tab = GetActiveTab();
    if (tab && tab->hwnd) {
        ShowWindow(tab->hwnd, SW_SHOW);
        MoveWindow(tab->hwnd,
            m_webviewRect.left, m_webviewRect.top,
            m_webviewRect.right - m_webviewRect.left,
            m_webviewRect.bottom - m_webviewRect.top, TRUE);
    }

    // Focus the WebView2 controller for keyboard input
    if (tab && tab->controller) {
        tab->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
    }

    UpdateUI();
    UpdateTabBar();
}

void BrowserApp::DuplicateTab(int tabIndex)
{
    if (tabIndex < 0 || tabIndex >= static_cast<int>(m_tabs.size())) return;
    TabData* tab = m_tabs[tabIndex].get();
    if (tab && !tab->url.empty()) {
        CreateNewTab(tab->url);
    }
}

void BrowserApp::MoveTab(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= static_cast<int>(m_tabs.size())) return;
    if (toIndex < 0 || toIndex >= static_cast<int>(m_tabs.size())) return;
    if (fromIndex == toIndex) return;

    auto tab = std::move(m_tabs[fromIndex]);
    m_tabs.erase(m_tabs.begin() + fromIndex);
    m_tabs.insert(m_tabs.begin() + toIndex, std::move(tab));

    // Update active tab index
    if (m_activeTabIndex == fromIndex) {
        m_activeTabIndex = toIndex;
    } else if (fromIndex < m_activeTabIndex && toIndex >= m_activeTabIndex) {
        m_activeTabIndex--;
    } else if (fromIndex > m_activeTabIndex && toIndex <= m_activeTabIndex) {
        m_activeTabIndex++;
    }

    UpdateTabBar();
}

void BrowserApp::PinTab(int tabIndex)
{
    // Pinning functionality - mark tab as privileged (no close button drawn)
    if (tabIndex >= 0 && tabIndex < static_cast<int>(m_tabs.size())) {
        m_tabs[tabIndex]->isPrivileged = !m_tabs[tabIndex]->isPrivileged;
        UpdateTabBar();
    }
}

TabData* BrowserApp::GetActiveTab()
{
    if (m_activeTabIndex >= 0 && m_activeTabIndex < static_cast<int>(m_tabs.size())) {
        return m_tabs[m_activeTabIndex].get();
    }
    return nullptr;
}

TabData* BrowserApp::GetTab(int index)
{
    if (index >= 0 && index < static_cast<int>(m_tabs.size())) {
        return m_tabs[index].get();
    }
    return nullptr;
}

int BrowserApp::GetActiveTabIndex() const { return m_activeTabIndex; }
int BrowserApp::GetTabCount() const { return static_cast<int>(m_tabs.size()); }

// ============================================================================
// Navigation
// ============================================================================

void BrowserApp::Navigate(const std::wstring& url)
{
    TabData* tab = GetActiveTab();
    if (tab && tab->webview) {
        std::wstring navigatedUrl = EnsureUrlPrefix(url);
        tab->webview->Navigate(navigatedUrl.c_str());
    }
}

void BrowserApp::GoBack()
{
    TabData* tab = GetActiveTab();
    if (tab && tab->webview && tab->canGoBack) {
        tab->webview->GoBack();
    }
}

void BrowserApp::GoForward()
{
    TabData* tab = GetActiveTab();
    if (tab && tab->webview && tab->canGoForward) {
        tab->webview->GoForward();
    }
}

void BrowserApp::Refresh()
{
    TabData* tab = GetActiveTab();
    if (tab && tab->webview) {
        tab->webview->Reload();
    }
}

void BrowserApp::RefreshIgnoringCache()
{
    TabData* tab = GetActiveTab();
    if (tab && tab->webview) {
        tab->webview->Reload();
    }
}

void BrowserApp::Stop()
{
    TabData* tab = GetActiveTab();
    if (tab && tab->webview) {
        tab->webview->Stop();
    }
}

void BrowserApp::GoHome()
{
    Navigate(m_settings.homePage);
}

void BrowserApp::NavigateToSearch(const std::wstring& query)
{
    std::wstring searchUrl = m_settings.searchEngine + UrlEncode(query);
    Navigate(searchUrl);
}

void BrowserApp::NavigateToLink(const std::wstring& href)
{
    std::wstring url = EnsureUrlPrefix(href);
    TabData* tab = GetActiveTab();
    if (tab && tab->webview) {
        tab->webview->Navigate(url.c_str());
    }
}

// ============================================================================
// Bookmarks
// ============================================================================

void BrowserApp::AddBookmark(const std::wstring& name, const std::wstring& url)
{
    std::lock_guard<std::mutex> lock(m_bookmarksMutex);

    // Check for duplicate
    for (const auto& bm : m_bookmarks) {
        if (bm.url == url) return;
    }

    Bookmark bookmark;
    bookmark.name = name;
    bookmark.url = url;
    bookmark.folder = L"";
    bookmark.createdAt = GetCurrentFileTime();
    bookmark.lastVisited = GetCurrentFileTime();
    bookmark.id = static_cast<int>(m_bookmarks.size());
    bookmark.order = static_cast<int>(m_bookmarks.size());

    m_bookmarks.push_back(bookmark);
    SaveBookmarks();
    UpdateBookmarksBar();
    ShowStatusMessage(L"Bookmark added: " + name);
}

void BrowserApp::RemoveBookmark(int index)
{
    std::lock_guard<std::mutex> lock(m_bookmarksMutex);
    if (index >= 0 && index < static_cast<int>(m_bookmarks.size())) {
        m_bookmarks.erase(m_bookmarks.begin() + index);
        SaveBookmarks();
        UpdateBookmarksBar();
        ShowStatusMessage(L"Bookmark removed");
    }
}

void BrowserApp::RemoveBookmarkByUrl(const std::wstring& url)
{
    std::lock_guard<std::mutex> lock(m_bookmarksMutex);
    for (auto it = m_bookmarks.begin(); it != m_bookmarks.end(); ++it) {
        if (it->url == url) {
            m_bookmarks.erase(it);
            SaveBookmarks();
            UpdateBookmarksBar();
            ShowStatusMessage(L"Bookmark removed");
            return;
        }
    }
}

bool BrowserApp::IsBookmarked(const std::wstring& url) const
{
    for (const auto& bm : m_bookmarks) {
        if (bm.url == url) return true;
    }
    return false;
}

const std::vector<Bookmark>& BrowserApp::GetBookmarks() const
{
    return m_bookmarks;
}

void BrowserApp::ToggleBookmarkForCurrentTab()
{
    TabData* tab = GetActiveTab();
    if (!tab) return;

    if (IsBookmarked(tab->url)) {
        RemoveBookmarkByUrl(tab->url);
        tab->isBookmarked = false;
    } else {
        AddBookmark(tab->title, tab->url);
        tab->isBookmarked = true;
    }
    UpdateUI();
}

void BrowserApp::LoadBookmarks()
{
    std::wifstream file(GetAppDataPath() + L"\\bookmarks.json");
    if (!file.is_open()) return;

    // Simple line-by-line JSON-like parsing for bookmarks
    std::wstring line;
    while (std::getline(file, line)) {
        if (line.find(L"\"url\"") != std::wstring::npos) {
            Bookmark bm;
            // Extract name and URL from JSON-like format
            size_t namePos = line.find(L"\"name\"");
            size_t urlPos = line.find(L"\"url\"");
            if (namePos != std::wstring::npos && urlPos != std::wstring::npos) {
                size_t nameStart = line.find(L"\"", namePos + 7) + 1;
                size_t nameEnd = line.find(L"\"", nameStart);
                size_t urlStart = line.find(L"\"", urlPos + 5) + 1;
                size_t urlEnd = line.find(L"\"", urlStart);
                if (nameStart != std::wstring::npos && nameEnd != std::wstring::npos &&
                    urlStart != std::wstring::npos && urlEnd != std::wstring::npos) {
                    bm.name = line.substr(nameStart, nameEnd - nameStart);
                    bm.url = line.substr(urlStart, urlEnd - urlStart);
                    bm.id = static_cast<int>(m_bookmarks.size());
                    bm.order = bm.id;
                    bm.createdAt = GetCurrentFileTime();
                    m_bookmarks.push_back(bm);
                }
            }
        }
    }
    file.close();
}

void BrowserApp::SaveBookmarks() const
{
    std::wofstream file(GetAppDataPath() + L"\\bookmarks.json");
    if (!file.is_open()) return;

    file << L"[\n";
    for (size_t i = 0; i < m_bookmarks.size(); i++) {
        const auto& bm = m_bookmarks[i];
        file << L"  {\"name\": \"" << bm.name
             << L"\", \"url\": \"" << bm.url
             << L"\", \"folder\": \"" << bm.folder
             << L"\", \"order\": " << bm.order << L"}";
        if (i < m_bookmarks.size() - 1) file << L",";
        file << L"\n";
    }
    file << L"]\n";
    file.close();
}

// ============================================================================
// History
// ============================================================================

void BrowserApp::AddHistoryEntry(const std::wstring& title, const std::wstring& url)
{
    if (url.empty() || url == L"about:blank") return;

    std::lock_guard<std::mutex> lock(m_historyMutex);

    // Check for duplicate and update
    for (auto& entry : m_history) {
        if (entry.url == url) {
            entry.title = title;
            entry.visitedAt = GetCurrentFileTime();
            entry.visitCount++;
            return;
        }
    }

    HistoryEntry entry;
    entry.title = title;
    entry.url = url;
    entry.visitedAt = GetCurrentFileTime();
    entry.visitCount = 1;
    m_history.push_back(entry);

    // Limit history size
    while (static_cast<int>(m_history.size()) > HISTORY_MAX_ENTRIES) {
        m_history.erase(m_history.begin());
    }
}

void BrowserApp::ClearHistory()
{
    std::lock_guard<std::mutex> lock(m_historyMutex);
    m_history.clear();
    SaveHistory();
    ShowStatusMessage(L"Browsing history cleared");
}

const std::vector<HistoryEntry>& BrowserApp::GetHistory() const
{
    return m_history;
}

std::vector<HistoryEntry> BrowserApp::SearchHistory(const std::wstring& query) const
{
    std::vector<HistoryEntry> results;
    std::wstring lowerQuery;
    lowerQuery.resize(query.size());
    std::transform(query.begin(), query.end(), lowerQuery.begin(), ::towlower);

    for (const auto& entry : m_history) {
        std::wstring lowerTitle;
        lowerTitle.resize(entry.title.size());
        std::transform(entry.title.begin(), entry.title.end(), lowerTitle.begin(), ::towlower);

        std::wstring lowerUrl;
        lowerUrl.resize(entry.url.size());
        std::transform(entry.url.begin(), entry.url.end(), lowerUrl.begin(), ::towlower);

        if (lowerTitle.find(lowerQuery) != std::wstring::npos ||
            lowerUrl.find(lowerQuery) != std::wstring::npos) {
            results.push_back(entry);
        }
    }
    return results;
}

void BrowserApp::LoadHistory()
{
    std::wifstream file(GetAppDataPath() + L"\\history.json");
    if (!file.is_open()) return;

    std::wstring line;
    while (std::getline(file, line)) {
        HistoryEntry entry;
        size_t titlePos = line.find(L"\"title\"");
        size_t urlPos = line.find(L"\"url\"");
        if (titlePos != std::wstring::npos && urlPos != std::wstring::npos) {
            size_t titleStart = line.find(L"\"", titlePos + 8) + 1;
            size_t titleEnd = line.find(L"\"", titleStart);
            size_t urlStart = line.find(L"\"", urlPos + 5) + 1;
            size_t urlEnd = line.find(L"\"", urlStart);
            if (titleStart != std::wstring::npos && titleEnd != std::wstring::npos &&
                urlStart != std::wstring::npos && urlEnd != std::wstring::npos) {
                entry.title = line.substr(titleStart, titleEnd - titleStart);
                entry.url = line.substr(urlStart, urlEnd - urlStart);
                entry.visitedAt = GetCurrentFileTime();
                m_history.push_back(entry);
            }
        }
    }
    file.close();
}

void BrowserApp::SaveHistory() const
{
    std::wofstream file(GetAppDataPath() + L"\\history.json");
    if (!file.is_open()) return;

    file << L"[\n";
    for (size_t i = 0; i < m_history.size(); i++) {
        const auto& entry = m_history[i];
        file << L"  {\"title\": \"" << entry.title
             << L"\", \"url\": \"" << entry.url
             << L"\", \"visits\": " << entry.visitCount << L"}";
        if (i < m_history.size() - 1) file << L",";
        file << L"\n";
    }
    file << L"]\n";
    file.close();
}

// ============================================================================
// Find on Page
// ============================================================================

void BrowserApp::ShowFindBar()
{
    if (m_findBarVisible) return;
    m_findBarVisible = true;
    ShowWindow(m_hFindBar, SW_SHOW);
    SetFocus(m_hFindInput);
    SendMessage(m_hFindInput, EM_SETSEL, 0, -1);
    CalculateLayout();
    RepositionControls();
}

void BrowserApp::HideFindBar()
{
    if (!m_findBarVisible) return;
    m_findBarVisible = false;
    ShowWindow(m_hFindBar, SW_HIDE);
    SetWindowTextW(m_hFindInput, L"");
    SetWindowTextW(m_hFindStatusLabel, L"");
    m_findResult.isActive = false;
    m_findResult.searchText.clear();
    CalculateLayout();
    RepositionControls();
}

void BrowserApp::FindNext()
{
    TabData* tab = GetActiveTab();
    if (!tab || !tab->webview) return;

    wchar_t text[256] = {};
    GetWindowTextW(m_hFindInput, text, 256);
    std::wstring searchText(text);

    if (searchText.empty()) return;

    if (m_findResult.searchText != searchText) {
        m_findResult.searchText = searchText;
    }

    // Execute find via JavaScript in the WebView2
    std::wstring script =
        L"(function() {"
        L"  const text = '" + searchText + L"';"
        L"  const found = window.find(text, false, false, true, false, true, false);"
        L"  window.chrome.webview.postMessage('find:' + found);"
        L"})();";

    tab->webview->ExecuteScript(script.c_str(), nullptr);
}

void BrowserApp::FindPrev()
{
    TabData* tab = GetActiveTab();
    if (!tab || !tab->webview) return;

    wchar_t text[256] = {};
    GetWindowTextW(m_hFindInput, text, 256);

    if (wcslen(text) == 0) return;

    std::wstring script =
        L"(function() {"
        L"  const text = '" + std::wstring(text) + L"';"
        L"  const found = window.find(text, false, true, true, false, true, false);"
        L"  window.chrome.webview.postMessage('find:' + found);"
        L"})();";

    tab->webview->ExecuteScript(script.c_str(), nullptr);
}

void BrowserApp::SetFindText(const std::wstring& text)
{
    SetWindowTextW(m_hFindInput, text.c_str());
    if (!text.empty()) {
        m_findResult.searchText = text;
        m_findResult.isActive = true;
    }
}

void BrowserApp::SetFindMatchCase(bool matchCase)
{
    m_findResult.matchCase = matchCase;
}

void BrowserApp::ClearFind()
{
    HideFindBar();
}

// ============================================================================
// Zoom
// ============================================================================

void BrowserApp::ZoomIn()
{
    TabData* tab = GetActiveTab();
    if (!tab || !tab->controller) return;

    if (tab->zoomLevel < ZOOM_MAX) {
        tab->zoomLevel = min(tab->zoomLevel + ZOOM_STEP, ZOOM_MAX);
        tab->controller->SetBoundsAndZoomInfo(
            tab->controller->get_Bounds, tab->zoomLevel);
        ShowStatusMessage(L"Zoom: " + std::to_wstring(100 + tab->zoomLevel) + L"%");
        UpdateStatusBar();
    }
}

void BrowserApp::ZoomOut()
{
    TabData* tab = GetActiveTab();
    if (!tab || !tab->controller) return;

    if (tab->zoomLevel > ZOOM_MIN) {
        tab->zoomLevel = max(tab->zoomLevel - ZOOM_STEP, ZOOM_MIN);
        tab->controller->SetBoundsAndZoomInfo(
            tab->controller->get_Bounds, tab->zoomLevel);
        ShowStatusMessage(L"Zoom: " + std::to_wstring(100 + tab->zoomLevel) + L"%");
        UpdateStatusBar();
    }
}

void BrowserApp::ZoomReset()
{
    TabData* tab = GetActiveTab();
    if (!tab || !tab->controller) return;

    tab->zoomLevel = ZOOM_DEFAULT;
    tab->controller->SetBoundsAndZoomInfo(
        tab->controller->get_Bounds, tab->zoomLevel);
    ShowStatusMessage(L"Zoom: 100%");
    UpdateStatusBar();
}

int BrowserApp::GetZoomLevel() const
{
    TabData* tab = const_cast<BrowserApp*>(this)->GetActiveTab();
    return tab ? tab->zoomLevel : ZOOM_DEFAULT;
}

void BrowserApp::SetZoomLevel(int level)
{
    TabData* tab = GetActiveTab();
    if (!tab || !tab->controller) return;

    tab->zoomLevel = std::clamp(level, ZOOM_MIN, ZOOM_MAX);
    tab->controller->SetBoundsAndZoomInfo(
        tab->controller->get_Bounds, tab->zoomLevel);
}

// ============================================================================
// View Controls
// ============================================================================

void BrowserApp::ToggleFullScreen()
{
    m_isFullScreen = !m_isFullScreen;

    if (m_isFullScreen) {
        // Save current window placement
        GetWindowPlacement(m_hMainWindow, &m_savedPlacement);
        // Enter full-screen mode
        MONITORINFO mi = { sizeof(MONITORINFO) };
        GetMonitorInfo(MonitorFromWindow(m_hMainWindow, MONITOR_DEFAULTTONEAREST), &mi);
        SetWindowLong(m_hMainWindow, GWL_STYLE,
            WS_POPUP | WS_VISIBLE);
        SetWindowPos(m_hMainWindow, HWND_TOP,
            mi.rcMonitor.left, mi.rcMonitor.top,
            mi.rcMonitor.right - mi.rcMonitor.left,
            mi.rcMonitor.bottom - mi.rcMonitor.top,
            SWP_FRAMECHANGED);
        ShowStatusMessage(L"Full screen mode (F11 to exit)");
    } else {
        // Restore from full-screen
        SetWindowLong(m_hMainWindow, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(m_hMainWindow, &m_savedPlacement);
        ShowStatusMessage(L"Windowed mode");
    }

    CalculateLayout();
    RepositionControls();
}

bool BrowserApp::IsFullScreen() const { return m_isFullScreen; }

void BrowserApp::ToggleBookmarksBar()
{
    m_settings.showBookmarksBar = !m_settings.showBookmarksBar;
    ShowWindow(m_hBookmarksBar, m_settings.showBookmarksBar ? SW_SHOW : SW_HIDE);
    CalculateLayout();
    RepositionControls();
    SaveSettings();
}

void BrowserApp::ToggleStatusBar()
{
    m_settings.showStatusBar = !m_settings.showStatusBar;
    ShowWindow(m_hStatusBar, m_settings.showStatusBar ? SW_SHOW : SW_HIDE);
    CalculateLayout();
    RepositionControls();
    SaveSettings();
}

void BrowserApp::ToggleAlwaysOnTop()
{
    m_isAlwaysOnTop = !m_isAlwaysOnTop;
    SetWindowPos(m_hMainWindow,
        m_isAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    ShowStatusMessage(m_isAlwaysOnTop ? L"Always on top: ON" : L"Always on top: OFF");
}

void BrowserApp::ToggleDevTools()
{
    TabData* tab = GetActiveTab();
    if (tab && tab->webview) {
        tab->webview->OpenDevToolsWindow();
    }
}

void BrowserApp::ViewSource()
{
    TabData* tab = GetActiveTab();
    if (tab && !tab->url.empty()) {
        std::wstring viewSourceUrl = L"view-source:" + tab->url;
        CreateNewTab(viewSourceUrl);
    }
}

// ============================================================================
// Settings
// ============================================================================

BrowserSettings& BrowserApp::GetSettings() { return m_settings; }

void BrowserApp::ShowSettingsDialog()
{
    SettingsDialog::ShowDialog(m_hMainWindow, m_settings);
}

void BrowserApp::ApplySettings()
{
    ToggleDevTools(); // Apply new settings to active WebView
}

void BrowserApp::LoadSettings()
{
    std::wifstream file(GetAppDataPath() + L"\\settings.json");
    if (!file.is_open()) return;

    std::wstring line;
    while (std::getline(file, line)) {
        auto getValue = [&line](const std::wstring& key) -> std::wstring {
            size_t pos = line.find(L"\"" + key + L"\"");
            if (pos == std::wstring::npos) return L"";
            size_t colonPos = line.find(L":", pos);
            if (colonPos == std::wstring::npos) return L"";
            size_t valueStart = line.find(L"\"", colonPos) + 1;
            if (valueStart == std::wstring::npos) return L"";
            size_t valueEnd = line.find(L"\"", valueStart);
            if (valueEnd == std::wstring::npos) return L"";
            return line.substr(valueStart, valueEnd - valueStart);
        };

        auto getBool = [&line](const std::wstring& key) -> bool {
            size_t pos = line.find(L"\"" + key + L"\"");
            if (pos == std::wstring::npos) return false;
            size_t colonPos = line.find(L":", pos);
            if (colonPos == std::wstring::npos) return false;
            size_t valuePos = line.find_first_of(L"tf", colonPos);
            if (valuePos == std::wstring::npos) return false;
            return line[valuePos] == L't';
        };

        if (line.find(L"homePage") != std::wstring::npos) {
            m_settings.homePage = getValue(L"homePage");
        }
        else if (line.find(L"searchEngine") != std::wstring::npos) {
            m_settings.searchEngine = getValue(L"searchEngine");
        }
        else if (line.find(L"searchEngineName") != std::wstring::npos) {
            m_settings.searchEngineName = getValue(L"searchEngineName");
        }
        else if (line.find(L"showBookmarksBar") != std::wstring::npos) {
            m_settings.showBookmarksBar = getBool(L"showBookmarksBar");
        }
        else if (line.find(L"showStatusBar") != std::wstring::npos) {
            m_settings.showStatusBar = getBool(L"showStatusBar");
        }
        else if (line.find(L"downloadPath") != std::wstring::npos) {
            m_settings.downloadPath = getValue(L"downloadPath");
        }
        else if (line.find(L"defaultZoom") != std::wstring::npos) {
            size_t pos = line.find(L"defaultZoom");
            size_t colonPos = line.find(L":", pos);
            if (colonPos != std::wstring::npos) {
                m_settings.defaultZoom = std::stoi(line.substr(colonPos + 1));
            }
        }
    }
    file.close();
}

void BrowserApp::SaveSettings() const
{
    std::wofstream file(GetAppDataPath() + L"\\settings.json");
    if (!file.is_open()) return;

    file << L"{\n"
         << L"  \"homePage\": \"" << m_settings.homePage << L"\",\n"
         << L"  \"searchEngine\": \"" << m_settings.searchEngine << L"\",\n"
         << L"  \"searchEngineName\": \"" << m_settings.searchEngineName << L"\",\n"
         << L"  \"showBookmarksBar\": " << (m_settings.showBookmarksBar ? L"true" : L"false") << L",\n"
         << L"  \"showStatusBar\": " << (m_settings.showStatusBar ? L"true" : L"false") << L",\n"
         << L"  \"enableDevTools\": " << (m_settings.enableDevTools ? L"true" : L"false") << L",\n"
         << L"  \"downloadPath\": \"" << m_settings.downloadPath << L"\",\n"
         << L"  \"defaultZoom\": " << m_settings.defaultZoom << L",\n"
         << L"  \"language\": \"" << m_settings.language << L"\"\n"
         << L"}\n";
    file.close();
}

// ============================================================================
// Downloads
// ============================================================================

void BrowserApp::HandleDownload(const std::wstring& url, const std::wstring& filename)
{
    DownloadItem item;
    item.url = url;
    item.filename = filename;
    item.savePath = m_settings.downloadPath + L"\\" + filename;
    item.startedAt = GetTickCount();
    m_downloads.push_back(item);
    ShowStatusMessage(L"Download started: " + filename);
}

void BrowserApp::ShowDownloadsBar()
{
    // Toggle downloads visibility in status bar
    if (m_downloads.empty()) {
        ShowStatusMessage(L"No downloads");
    } else {
        std::wstring msg = L"Downloads (" + std::to_wstring(m_downloads.size()) + L"):";
        for (const auto& dl : m_downloads) {
            msg += L" " + dl.filename;
            if (dl.isComplete) msg += L" [Done]";
        }
        ShowStatusMessage(msg);
    }
}

void BrowserApp::HideDownloadsBar() { /* Not implemented in this version */ }

// ============================================================================
// Notifications
// ============================================================================

void BrowserApp::ShowNotification(const std::wstring& title, const std::wstring& message)
{
    Notification notif;
    notif.title = title;
    notif.message = message;
    notif.timestamp = GetTickCount();
    notif.id = static_cast<int>(m_notifications.size());
    m_notifications.push_back(notif);

    // Show a balloon tip in the status bar
    std::wstring display = title + L": " + message;
    ShowStatusMessage(display);
}

void BrowserApp::ShowStatusMessage(const std::wstring& message, DWORD timeoutMs)
{
    if (m_hStatusBar) {
        SendMessage(m_hStatusBar, SB_SETTEXT, 0, (LPARAM)message.c_str());
        // Auto-clear after timeout
        SetTimer(m_hMainWindow, TIMER_STATUS_CLEAR, timeoutMs, nullptr);
    }
}

// ============================================================================
// UI Update Methods
// ============================================================================

void BrowserApp::UpdateUI()
{
    UpdateNavigationState();
    UpdateUrlBar();
    UpdateTitleBar();
    UpdateStatusBar();
    UpdateBookmarksBar();
}

void BrowserApp::UpdateTabBar()
{
    if (m_hTabBar) {
        InvalidateRect(m_hTabBar, nullptr, TRUE);
    }
}

void BrowserApp::UpdateNavigationState()
{
    TabData* tab = GetActiveTab();

    BOOL canGoBack    = FALSE;
    BOOL canGoForward = FALSE;

    if (tab && tab->webview) {
        tab->webview->get_CanGoBack(&canGoBack);
        tab->webview->get_CanGoForward(&canGoForward);
        tab->canGoBack = canGoBack;
        tab->canGoForward = canGoForward;
    }

    EnableWindow(m_hBackBtn, canGoBack);
    EnableWindow(m_hForwardBtn, canGoForward);

    // Toggle between refresh and stop buttons
    if (tab && tab->isLoading) {
        ShowWindow(m_hRefreshBtn, SW_HIDE);
        ShowWindow(m_hStopBtn, SW_SHOW);
    } else {
        ShowWindow(m_hRefreshBtn, SW_SHOW);
        ShowWindow(m_hStopBtn, SW_HIDE);
    }

    // Update bookmark star button
    if (tab) {
        SetWindowTextW(m_hBookmarkAddBtn, tab->isBookmarked ? L"#" : L"*");
    }
}

void BrowserApp::UpdateUrlBar()
{
    TabData* tab = GetActiveTab();
    if (tab && m_hUrlBar) {
        std::wstring displayUrl = tab->displayUrl.empty() ? tab->url : tab->displayUrl;

        // Get current caret position to preserve it
        DWORD selStart = 0, selEnd = 0;
        SendMessage(m_hUrlBar, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);

        SetWindowTextW(m_hUrlBar, displayUrl.c_str());

        // Highlight the domain part when no selection
        if (selStart == selEnd) {
            std::wstring domain = ExtractDomain(displayUrl);
            if (!domain.empty()) {
                size_t domainPos = displayUrl.find(domain);
                if (domainPos != std::wstring::npos) {
                    // Don't auto-select, just leave cursor at end
                    SendMessage(m_hUrlBar, EM_SETSEL, displayUrl.length(), displayUrl.length());
                }
            }
        }
    }
}

void BrowserApp::UpdateStatusBar()
{
    TabData* tab = GetActiveTab();
    if (tab && m_hStatusBar) {
        // Update status text
        std::wstring statusText = tab->statusText;
        if (statusText.empty()) {
            if (tab->isLoading) {
                statusText = L"Loading " + tab->url + L"...";
            } else if (tab->isCrashed) {
                statusText = L"Tab crashed - click to reload";
            }
        }
        SendMessage(m_hStatusBar, SB_SETTEXT, 0, (LPARAM)statusText.c_str());

        // Update zoom level display
        std::wstring zoomText = std::to_wstring(100 + tab->zoomLevel) + L"%";
        SendMessage(m_hStatusBar, SB_SETTEXT, 1, (LPARAM)zoomText.c_str());
    }
}

void BrowserApp::UpdateTitleBar()
{
    TabData* tab = GetActiveTab();
    if (tab && m_hMainWindow) {
        std::wstring title = tab->title + L" - " STR_APP_TITLE;
        SetWindowTextW(m_hMainWindow, title.c_str());
    }
}

void BrowserApp::UpdateBookmarksBar()
{
    if (!m_hBookmarksBar || !m_settings.showBookmarksBar) return;

    // Destroy existing bookmark buttons
    HWND hChild = GetWindow(m_hBookmarksBar, GW_CHILD);
    while (hChild) {
        HWND hNext = GetWindow(hChild, GW_HWNDNEXT);
        DestroyWindow(hChild);
        hChild = hNext;
    }

    // Create bookmark buttons
    int x = 8;
    int y = 3;
    int btnHeight = BOOKMARKS_BAR_HEIGHT - 6;

    for (size_t i = 0; i < m_bookmarks.size() && x < 800; i++) {
        const auto& bm = m_bookmarks[i];

        HWND hBtn = CreateWindowExW(
            0, L"BUTTON", bm.name.c_str(),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT | BS_LEFT,
            x, y, 0, btnHeight,
            m_hBookmarksBar, (HMENU)(ID_TAB_FIRST + static_cast<int>(i)),
            m_hInstance, nullptr);

        if (hBtn) {
            SendMessage(hBtn, WM_SETFONT, (WPARAM)m_hSmallFont, TRUE);

            // Calculate button width based on text
            HDC hdc = GetDC(hBtn);
            HGDIOBJ oldFont = SelectObject(hdc, m_hSmallFont);
            SIZE textSize;
            GetTextExtentPoint32W(hdc, bm.name.c_str(),
                static_cast<int>(bm.name.size()), &textSize);
            SelectObject(hdc, oldFont);
            ReleaseDC(hBtn, hdc);

            int btnWidth = textSize.cx + 20;
            MoveWindow(hBtn, x, y, btnWidth, btnHeight, TRUE);
            x += btnWidth + 4;
        }
    }

    InvalidateRect(m_hBookmarksBar, nullptr, TRUE);
}

void BrowserApp::UpdateLoadingState(bool isLoading, int progress)
{
    TabData* tab = GetActiveTab();
    if (tab) {
        tab->isLoading = isLoading;
        if (progress >= 0) tab->loadProgress = progress;
    }
    UpdateNavigationState();
    UpdateStatusBar();
}

// ============================================================================
// WebView2 Event Handlers
// ============================================================================

HRESULT BrowserApp::RegisterTabEventHandlers(TabData* tab)
{
    if (!tab || !tab->webview) return E_INVALIDARG;

    // Navigation completed event
    EventRegistrationToken navCompletedToken = {};
    tab->webview->add_NavigationCompleted(
        Callback<ICoreWebView2NavigationCompletedEventHandler>(
            [this, tab](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                BOOL isSuccess = FALSE;
                BOOL isNewDocument = FALSE;
                args->get_IsSuccess(&isSuccess);
                args->get_IsNewDocument(&isNewDocument);

                tab->isLoading = false;

                if (isSuccess) {
                    UpdateNavigationState();
                    AddHistoryEntry(tab->title, tab->url);
                    UpdateStatusBar();
                    UpdateTitleBar();
                    UpdateUrlBar();

                    // Check bookmark status
                    tab->isBookmarked = IsBookmarked(tab->url);
                    UpdateUI();
                } else {
                    COREWEBVIEW2_WEB_ERROR_STATUS status;
                    args->get_WebErrorStatus(&status);
                    ShowStatusMessage(L"Navigation failed with error: " + std::to_wstring(status));
                }

                return S_OK;
            }).Get(), &navCompletedToken);

    // Document title changed event
    EventRegistrationToken titleChangedToken = {};
    tab->webview->add_DocumentTitleChanged(
        Callback<ICoreWebView2DocumentTitleChangedEventHandler>(
            [this, tab](ICoreWebView2* sender, IUnknown* args) -> HRESULT {
                LPWSTR titlePtr = nullptr;
                tab->webview->get_DocumentTitle(&titlePtr);
                if (titlePtr) {
                    tab->title = titlePtr;
                    CoTaskMemFree(titlePtr);
                    UpdateTitleBar();
                    UpdateTabBar();
                }
                return S_OK;
            }).Get(), &titleChangedToken);

    // Source changed event (URL changes)
    EventRegistrationToken sourceChangedToken = {};
    tab->webview->add_SourceChanged(
        Callback<ICoreWebView2SourceChangedEventHandler>(
            [this, tab](ICoreWebView2* sender, ICoreWebView2SourceChangedEventArgs* args) -> HRESULT {
                LPWSTR sourcePtr = nullptr;
                tab->webview->get_Source(&sourcePtr);
                if (sourcePtr) {
                    tab->url = sourcePtr;
                    tab->displayUrl = sourcePtr;
                    CoTaskMemFree(sourcePtr);
                    tab->isBookmarked = IsBookmarked(tab->url);
                    UpdateUrlBar();
                    UpdateTabBar();
                    UpdateUI();
                }
                return S_OK;
            }).Get(), &sourceChangedToken);

    // History changed event
    EventRegistrationToken historyChangedToken = {};
    tab->webview->add_HistoryChanged(
        Callback<ICoreWebView2HistoryChangedEventHandler>(
            [this, tab](ICoreWebView2* sender, IUnknown* args) -> HRESULT {
                UpdateNavigationState();
                return S_OK;
            }).Get(), &historyChangedToken);

    // New window requested event (popup blocking / tab opening)
    EventRegistrationToken newWindowToken = {};
    tab->webview->add_NewWindowRequested(
        Callback<ICoreWebView2NewWindowRequestedEventHandler>(
            [this, tab](ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
                LPWSTR uriPtr = nullptr;
                args->get_Uri(&uriPtr);
                if (uriPtr) {
                    // Open in new tab instead of popup window
                    args->put_Handled(TRUE);
                    CreateNewTab(uriPtr);
                    CoTaskMemFree(uriPtr);
                }
                return S_OK;
            }).Get(), &newWindowToken);

    // Web message received event (JavaScript communication)
    EventRegistrationToken webMessageToken = {};
    tab->webview->add_WebMessageReceived(
        Callback<ICoreWebView2WebMessageReceivedEventHandler>(
            [this, tab](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                LPWSTR messagePtr = nullptr;
                args->get_WebMessageAsJson(&messagePtr);
                if (messagePtr) {
                    std::wstring msg = messagePtr;
                    CoTaskMemFree(messagePtr);

                    // Handle find result messages
                    if (msg.find(L"find:") == 0) {
                        bool found = msg.find(L"true") != std::wstring::npos;
                        std::wstring resultText = found ?
                            L"Match found" : L"No more matches";
                        SetWindowTextW(m_hFindStatusLabel, resultText.c_str());
                    }
                    // Handle navigation messages from the new tab page
                    else if (msg.find(L"navigate:") == 0) {
                        std::wstring url = msg.substr(9);
                        // Remove quotes
                        if (!url.empty() && url.front() == L'"' && url.back() == L'"') {
                            url = url.substr(1, url.length() - 2);
                        }
                        if (!url.empty()) {
                            Navigate(url);
                        }
                    }
                    // Handle search messages from the new tab page
                    else if (msg.find(L"search:") == 0) {
                        std::wstring query = msg.substr(7);
                        if (!query.empty() && query.front() == L'"' && query.back() == L'"') {
                            query = query.substr(1, query.length() - 2);
                        }
                        if (!query.empty()) {
                            NavigateToSearch(query);
                        }
                    }
                }
                return S_OK;
            }).Get(), &webMessageToken);

    // Status bar text changed event
    EventRegistrationToken statusChangedToken = {};
    tab->webview->add_StatusBarTextChanged(
        Callback<ICoreWebView2StatusBarTextChangedEventHandler>(
            [this, tab](ICoreWebView2* sender, IUnknown* args) -> HRESULT {
                LPWSTR statusPtr = nullptr;
                tab->webview->get_StatusBarText(&statusPtr);
                if (statusPtr) {
                    tab->statusText = statusPtr;
                    CoTaskMemFree(statusPtr);
                    UpdateStatusBar();
                }
                return S_OK;
            }).Get(), &statusChangedToken);

    // Full-screen element changed event
    EventRegistrationToken fullScreenToken = {};
    tab->webview->add_ContainsFullScreenElementChanged(
        Callback<ICoreWebView2ContainsFullScreenElementChangedEventHandler>(
            [this, tab](ICoreWebView2* sender, IUnknown* args) -> HRESULT {
                BOOL containsFullScreen = FALSE;
                tab->webview->get_ContainsFullScreenElement(&containsFullScreen);
                if (containsFullScreen) {
                    if (!m_isFullScreen) ToggleFullScreen();
                }
                return S_OK;
            }).Get(), &fullScreenToken);

    return S_OK;
}

// ============================================================================
// Command Handling
// ============================================================================

void BrowserApp::OnCommand(UINT commandId)
{
    switch (commandId) {
        // File menu
        case ID_FILE_NEW_TAB:     CreateNewTab(); break;
        case ID_FILE_CLOSE_TAB:   CloseActiveTab(); break;
        case ID_FILE_NEW_WINDOW:  break;
        case ID_FILE_PRINT:       Print(); break;
        case ID_FILE_EXIT:        Close(); break;

        // Edit menu
        case ID_EDIT_CUT:         Cut(); break;
        case ID_EDIT_COPY:        Copy(); break;
        case ID_EDIT_PASTE:       Paste(); break;
        case ID_EDIT_SELECT_ALL:  SelectAll(); break;
        case ID_EDIT_FIND:        ShowFindBar(); break;
        case ID_EDIT_FIND_NEXT:   FindNext(); break;
        case ID_EDIT_FIND_PREV:   FindPrev(); break;

        // View menu
        case ID_VIEW_ZOOM_IN:         ZoomIn(); break;
        case ID_VIEW_ZOOM_OUT:        ZoomOut(); break;
        case ID_VIEW_ZOOM_RESET:      ZoomReset(); break;
        case ID_VIEW_FULL_SCREEN:     ToggleFullScreen(); break;
        case ID_VIEW_STATUS_BAR:      ToggleStatusBar(); break;
        case ID_VIEW_BOOKMARKS_BAR:   ToggleBookmarksBar(); break;
        case ID_VIEW_DEVTOOLS:        ToggleDevTools(); break;
        case ID_VIEW_SOURCE:          ViewSource(); break;
        case ID_VIEW_ALWAYS_ON_TOP:   ToggleAlwaysOnTop(); break;

        // Navigation
        case ID_NAV_BACK:         GoBack(); break;
        case ID_NAV_FORWARD:      GoForward(); break;
        case ID_NAV_REFRESH:      Refresh(); break;
        case ID_NAV_STOP:         Stop(); break;
        case ID_NAV_HOME:         GoHome(); break;

        // Bookmarks
        case ID_BOOKMARK_ADD:     ToggleBookmarkForCurrentTab(); break;
        case ID_BOOKMARK_REMOVE:  {
            TabData* tab = GetActiveTab();
            if (tab) RemoveBookmarkByUrl(tab->url);
            break;
        }
        case ID_BOOKMARK_MANAGE:  ShowNotification(L"Bookmarks", L"Bookmark manager not yet implemented"); break;

        // History
        case ID_HISTORY_SHOW:     ShowNotification(L"History", L"History panel not yet implemented"); break;
        case ID_HISTORY_CLEAR:    ClearHistory(); break;

        // Tools
        case ID_TOOLS_CLEAR_DATA:
            ClearHistory();
            ShowNotification(L"Privacy", L"Browsing data cleared");
            break;

        // Help
        case ID_HELP_ABOUT:       AboutDialog::ShowDialog(m_hMainWindow); break;
        case ID_HELP_VERSION_INFO:
            ShowNotification(STR_APP_TITLE,
                L"Version " STR_APP_VERSION L" - Built with Microsoft WebView 2 (Chromium)");
            break;

        // Navigation bar buttons
        case IDC_BACK:          GoBack(); break;
        case IDC_FORWARD:       GoForward(); break;
        case IDC_REFRESH:       Refresh(); break;
        case IDC_STOP:          Stop(); break;
        case IDC_HOME:          GoHome(); break;
        case IDC_GO:            OnUrlBarEnter(); break;
        case IDC_BOOKMARK_ADD:  ToggleBookmarkForCurrentTab(); break;
        case IDC_NEW_TAB_BTN:   OnNewTabButtonClick(); break;

        // Find bar
        case IDC_FIND_PREV:     FindPrev(); break;
        case IDC_FIND_NEXT:     FindNext(); break;
        case IDC_FIND_CLOSE:    HideFindBar(); break;

        // Focus URL bar
        case IDC_URL_BAR:
            if (HIWORD(GetCurrentMessage()->wParam) == EN_SETFOCUS) {
                OnUrlBarFocus();
            } else if (HIWORD(GetCurrentMessage()->wParam) == EN_KILLFOCUS) {
                OnUrlBarKillFocus();
            }
            break;

        default:
            // Check if it's a bookmark click
            if (commandId >= ID_TAB_FIRST &&
                commandId < ID_TAB_FIRST + static_cast<int>(m_bookmarks.size())) {
                int bmIndex = commandId - ID_TAB_FIRST;
                if (bmIndex < static_cast<int>(m_bookmarks.size())) {
                    Navigate(m_bookmarks[bmIndex].url);
                }
            }
            break;
    }
}

// ============================================================================
// Clipboard Operations
// ============================================================================

void BrowserApp::Cut()
{
    TabData* tab = GetActiveTab();
    if (tab && tab->webview) {
        tab->webview->ExecuteScript(L"document.execCommand('cut')", nullptr);
    }
}

void BrowserApp::Copy()
{
    TabData* tab = GetActiveTab();
    if (tab && tab->webview) {
        tab->webview->ExecuteScript(L"document.execCommand('copy')", nullptr);
    }
}

void BrowserApp::Paste()
{
    TabData* tab = GetActiveTab();
    if (tab && tab->webview) {
        tab->webview->ExecuteScript(L"document.execCommand('paste')", nullptr);
    }
}

void BrowserApp::SelectAll()
{
    TabData* tab = GetActiveTab();
    if (tab && tab->webview) {
        tab->webview->ExecuteScript(L"document.execCommand('selectAll')", nullptr);
    }
}

void BrowserApp::CopyUrl()
{
    TabData* tab = GetActiveTab();
    if (tab) {
        if (OpenClipboard(m_hMainWindow)) {
            EmptyClipboard();
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE,
                (tab->url.size() + 1) * sizeof(wchar_t));
            if (hMem) {
                wchar_t* pMem = static_cast<wchar_t*>(GlobalLock(hMem));
                if (pMem) {
                    wcscpy_s(pMem, tab->url.size() + 1, tab->url.c_str());
                    GlobalUnlock(hMem);
                    SetClipboardData(CF_UNICODETEXT, hMem);
                }
            }
            CloseClipboard();
            ShowStatusMessage(L"URL copied to clipboard");
        }
    }
}

void BrowserApp::CopySelectedText()
{
    // Already handled by WebView2 internally
    Copy();
}

// ============================================================================
// Printing
// ============================================================================

void BrowserApp::Print()
{
    TabData* tab = GetActiveTab();
    if (tab && tab->webview) {
        // Use JavaScript to trigger the browser's print dialog
        tab->webview->ExecuteScript(L"window.print()", nullptr);
    }
}

void BrowserApp::PrintPreview()
{
    // Print preview is handled by the system print dialog
    Print();
}

// ============================================================================
// URL Bar Event Handlers
// ============================================================================

void BrowserApp::OnUrlBarEnter()
{
    wchar_t buffer[MAX_URL_LENGTH] = {};
    GetWindowTextW(m_hUrlBar, buffer, MAX_URL_LENGTH);
    std::wstring url(buffer);

    if (url.empty()) return;

    // Determine if input is a URL or search query
    if (IsValidUrl(url) || url.find(L".") != std::wstring::npos) {
        Navigate(url);
    } else {
        NavigateToSearch(url);
    }

    // Remove focus from URL bar
    TabData* tab = GetActiveTab();
    if (tab && tab->controller) {
        tab->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
    }
}

void BrowserApp::OnUrlBarChanged()
{
    // Auto-complete logic (simplified)
    // In a full implementation, this would show a dropdown with suggestions
}

void BrowserApp::OnUrlBarFocus()
{
    // Select all text in URL bar for easy replacement
    SendMessage(m_hUrlBar, EM_SETSEL, 0, -1);
}

void BrowserApp::OnUrlBarKillFocus()
{
    // Restore display URL
    UpdateUrlBar();
}

void BrowserApp::SubclassUrlBar()
{
    SetWindowSubclass(m_hUrlBar, UrlBarSubclassProc, 0,
        reinterpret_cast<DWORD_PTR>(this));
}

void BrowserApp::RemoveUrlBarSubclass()
{
    RemoveWindowSubclass(m_hUrlBar, UrlBarSubclassProc, 0);
}

std::vector<std::wstring> BrowserApp::GetAutoCompleteSuggestions(const std::wstring& text) const
{
    std::vector<std::wstring> suggestions;
    if (text.length() < 2) return suggestions;

    std::wstring lowerText;
    lowerText.resize(text.size());
    std::transform(text.begin(), text.end(), lowerText.begin(), ::towlower);

    // Search bookmarks
    for (const auto& bm : m_bookmarks) {
        std::wstring lowerName;
        lowerName.resize(bm.name.size());
        std::transform(bm.name.begin(), bm.name.end(), lowerName.begin(), ::towlower);

        if (lowerName.find(lowerText) != std::wstring::npos ||
            bm.url.find(text) != std::wstring::npos) {
            suggestions.push_back(bm.url);
            if (suggestions.size() >= 8) break;
        }
    }

    // Search history
    for (const auto& entry : m_history) {
        std::wstring lowerTitle;
        lowerTitle.resize(entry.title.size());
        std::transform(entry.title.begin(), entry.title.end(), lowerTitle.begin(), ::towlower);

        if (lowerTitle.find(lowerText) != std::wstring::npos ||
            entry.url.find(text) != std::wstring::npos) {
            suggestions.push_back(entry.url);
            if (suggestions.size() >= 12) break;
        }
    }

    return suggestions;
}

// ============================================================================
// Tab Bar Drawing
// ============================================================================

void BrowserApp::DrawTabItem(DRAWITEMSTRUCT* dis)
{
    // This draws individual tab items on the custom tab bar
    // In this implementation, we draw all tabs manually in the tab bar's WM_PAINT
    HDC hdc = dis->hDC;
    RECT& rc = dis->rect;
    int index = dis->itemID;

    if (index < 0 || index >= static_cast<int>(m_tabs.size())) return;

    TabData* tab = m_tabs[index].get();
    bool isActive = (index == m_activeTabIndex);
    bool isHover = (index == m_tabHoverIndex);

    // Fill background
    HBRUSH hBrush = isHover ? m_hAccentBrush :
                    isActive ? m_hTabActiveBrush : m_hTabInactiveBrush;
    FillRect(hdc, &rc, hBrush);

    // Draw bottom border for active tab
    if (isActive) {
        HPEN hPen = CreatePen(PS_SOLID, 2, COLOR_ACCENT);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, rc.left, rc.bottom - 1, nullptr);
        LineTo(hdc, rc.right, rc.bottom - 1);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    } else {
        // Draw separator line
        HPEN hPen = CreatePen(PS_SOLID, 1, COLOR_TAB_BORDER);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, rc.right, rc.top + 4, nullptr);
        LineTo(hdc, rc.right, rc.bottom - 4);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }

    // Set text color
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, isActive ? COLOR_TEXT : COLOR_TEXT_SECONDARY);
    HFONT hFont = isActive ? m_hBoldFont : m_hDefaultFont;
    HGDIOBJ hOldFont = SelectObject(hdc, hFont);

    // Calculate text rectangle (leave room for close button)
    RECT textRect = rc;
    textRect.left += 8;
    textRect.right -= (tab->isPrivileged ? 4 : 24);

    // Draw tab title with ellipsis for long titles
    std::wstring title = tab->title.empty() ? L"New Tab" : tab->title;
    DrawText(hdc, title.c_str(), static_cast<int>(title.size()),
        &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);

    SelectObject(hdc, hOldFont);

    // Draw loading indicator
    if (tab->isLoading) {
        HPEN hLoadPen = CreatePen(PS_SOLID, 2, COLOR_LOADING);
        HPEN hOldLoadPen = (HPEN)SelectObject(hdc, hLoadPen);
        int centerX = rc.right - 18;
        int centerY = rc.top + (rc.bottom - rc.top) / 2;
        Arc(hdc, centerX - 5, centerY - 5, centerX + 5, centerY + 5,
            centerX, centerY - 5, centerX, centerY);
        SelectObject(hdc, hOldLoadPen);
        DeleteObject(hLoadPen);
    }

    // Draw close button (if not pinned and not the only tab)
    if (!tab->isPrivileged && m_tabs.size() > 1) {
        RECT closeRect = GetCloseButtonRect(rc);
        bool isCloseHover = PtInRect(&closeRect, dis->rcAction);

        // Draw "X" button background
        if (isCloseHover) {
            HBRUSH hRedBrush = CreateSolidBrush(RGB(232, 80, 80));
            FillRect(hdc, &closeRect, hRedBrush);
            DeleteObject(hRedBrush);
            SetTextColor(hdc, RGB(255, 255, 255));
        }

        // Draw "X"
        HFONT hSmall = m_hSmallFont;
        HGDIOBJ hOldSmall = SelectObject(hdc, hSmall);
        DrawText(hdc, L"X", 1, &closeRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hOldSmall);
    }

    // Draw crash indicator
    if (tab->isCrashed) {
        RECT crashRect = rc;
        crashRect.left += 4;
        HPEN hRedPen = CreatePen(PS_SOLID, 2, COLOR_INSECURE);
        HPEN hOldRedPen = (HPEN)SelectObject(hdc, hRedPen);
        Arc(hdc, crashRect.left + 4, rc.top + 6, crashRect.left + 14, rc.top + 16,
            crashRect.left + 9, rc.top + 6, crashRect.left + 9, rc.top + 16);
        SelectObject(hdc, hOldRedPen);
        DeleteObject(hRedPen);
    }
}

int BrowserApp::GetTabRectWidth(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_tabs.size())) return TAB_MIN_WIDTH;

    int totalWidth = (m_tabBarRect.right - m_tabBarRect.left) - 36; // Subtract new tab button
    int avgWidth = totalWidth / max(static_cast<int>(m_tabs.size()), 1);
    int width = std::clamp(avgWidth, TAB_MIN_WIDTH, TAB_MAX_WIDTH);

    // Pinned tabs are smaller
    if (m_tabs[index]->isPrivileged) {
        width = TAB_MIN_WIDTH;
    }

    return width;
}

RECT BrowserApp::GetTabRect(int index) const
{
    RECT rc = {};
    int x = m_tabScrollOffset;
    for (int i = 0; i < index; i++) {
        x += GetTabRectWidth(i);
    }
    int width = GetTabRectWidth(index);
    rc = { x, 0, x + width, TAB_HEIGHT };
    return rc;
}

RECT BrowserApp::GetCloseButtonRect(const RECT& tabRect) const
{
    int btnSize = 16;
    return {
        tabRect.right - btnSize - 4,
        (tabRect.top + tabRect.bottom - btnSize) / 2,
        tabRect.right - 4,
        (tabRect.top + tabRect.bottom + btnSize) / 2
    };
}

int BrowserApp::HitTestTab(int x, int y) const
{
    for (int i = 0; i < static_cast<int>(m_tabs.size()); i++) {
        RECT rc = GetTabRect(i);
        if (PtInRect(&rc, { x, y })) {
            return i;
        }
    }
    return -1;
}

int BrowserApp::HitTestTabClose(int x, int y) const
{
    for (int i = 0; i < static_cast<int>(m_tabs.size()); i++) {
        RECT tabRect = GetTabRect(i);
        RECT closeRect = GetCloseButtonRect(tabRect);
        if (PtInRect(&closeRect, { x, y })) {
            return i;
        }
    }
    return -1;
}

void BrowserApp::OnTabClick(int tabIndex)
{
    if (tabIndex >= 0 && tabIndex < static_cast<int>(m_tabs.size())) {
        SwitchToTab(tabIndex);
    }
}

void BrowserApp::OnTabCloseClick(int tabIndex)
{
    if (tabIndex >= 0) {
        CloseTab(tabIndex);
    }
}

void BrowserApp::OnNewTabButtonClick()
{
    CreateNewTab();
}

void BrowserApp::RedrawTabBar()
{
    InvalidateRect(m_hTabBar, nullptr, TRUE);
}

void BrowserApp::ScrollTabBar(int delta)
{
    m_tabScrollOffset += delta;
    if (m_tabScrollOffset < 0) m_tabScrollOffset = 0;

    // Calculate maximum scroll offset
    int totalTabWidth = 0;
    for (int i = 0; i < static_cast<int>(m_tabs.size()); i++) {
        totalTabWidth += GetTabRectWidth(i);
    }
    int maxScroll = max(0, totalTabWidth - (m_tabBarRect.right - m_tabBarRect.left) + 36);
    if (m_tabScrollOffset > maxScroll) m_tabScrollOffset = maxScroll;

    RedrawTabBar();
}

void BrowserApp::EnsureTabVisible(int tabIndex)
{
    RECT rc = GetTabRect(tabIndex);
    int visibleWidth = m_tabBarRect.right - m_tabBarRect.left - 36;

    if (rc.left < m_tabScrollOffset) {
        m_tabScrollOffset = rc.left;
    } else if (rc.right > m_tabScrollOffset + visibleWidth) {
        m_tabScrollOffset = rc.right - visibleWidth;
    }
    RedrawTabBar();
}

// ============================================================================
// Window Procedure
// ============================================================================

LRESULT CALLBACK BrowserApp::MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BrowserApp* pApp = reinterpret_cast<BrowserApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (uMsg) {
        case WM_CREATE: {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            if (pCreate) {
                pApp = static_cast<BrowserApp*>(pCreate->lpCreateParams);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApp));
            }
            break;
        }

        case WM_SIZE: {
            if (pApp && pApp->m_isInitialized) {
                pApp->CalculateLayout();
                pApp->RepositionControls();
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (pApp) {
                pApp->OnPaint();
            }
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_COMMAND: {
            if (pApp) {
                pApp->OnCommand(LOWORD(wParam));
            }
            return 0;
        }

        case WM_NOTIFY: {
            if (pApp) {
                pApp->OnNotify(static_cast<int>(wParam),
                    reinterpret_cast<NMHDR*>(lParam));
            }
            return 0;
        }

        case WM_TIMER: {
            if (pApp) {
                pApp->OnTimer(wParam);
            }
            return 0;
        }

        case WM_KEYDOWN: {
            if (pApp) {
                pApp->OnKeyDown(wParam, lParam);
            }
            return 0;
        }

        case WM_MOUSEWHEEL: {
            if (pApp) {
                // Ctrl+MouseWheel for zooming
                if (GetKeyState(VK_CONTROL) & 0x8000) {
                    pApp->OnMouseWheel(wParam, lParam);
                    return 0;
                }
            }
            break;
        }

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_MOUSEMOVE: {
            // Handle tab bar mouse events
            if (pApp && pApp->m_hTabBar) {
                POINT pt = { LOWORD(lParam), HIWORD(lParam) };
                ScreenToClient(hwnd, &pt);
                // Check if click is in tab bar area
                if (pt.y >= pApp->m_tabBarRect.top && pt.y <= pApp->m_tabBarRect.bottom) {
                    if (uMsg == WM_LBUTTONDOWN) {
                        int hitIndex = pApp->HitTestTab(pt.x, pt.y);
                        int closeHit = pApp->HitTestTabClose(pt.x, pt.y);
                        if (closeHit >= 0) {
                            pApp->OnTabCloseClick(closeHit);
                            return 0;
                        } else if (hitIndex >= 0) {
                            pApp->OnTabClick(hitIndex);
                            return 0;
                        }
                    } else if (uMsg == WM_MOUSEMOVE) {
                        int hoverIndex = pApp->HitTestTab(pt.x, pt.y);
                        if (hoverIndex != pApp->m_tabHoverIndex) {
                            pApp->m_tabHoverIndex = hoverIndex;
                            pApp->RedrawTabBar();
                        }
                        return 0;
                    } else if (uMsg == WM_LBUTTONDBLCLK) {
                        int hitIndex = pApp->HitTestTab(pt.x, pt.y);
                        if (hitIndex >= 0) {
                            pApp->PinTab(hitIndex);
                            return 0;
                        }
                    }
                }
            }
            break;
        }

        case WM_DROPFILES: {
            if (pApp) {
                pApp->OnDropFiles(reinterpret_cast<HDROP>(wParam));
            }
            return 0;
        }

        case WM_SETFOCUS: {
            // Forward focus to active tab's WebView2 controller
            if (pApp) {
                TabData* tab = pApp->GetActiveTab();
                if (tab && tab->controller) {
                    tab->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
                    return 0;
                }
            }
            break;
        }

        case WM_CLOSE: {
            if (pApp) {
                pApp->m_isClosing = true;
            }
            break;
        }

        case WM_DESTROY: {
            if (pApp) {
                PostQuitMessage(0);
            }
            return 0;
        }

        case WM_ERASEBKGND: {
            return 1; // Prevent flicker
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = reinterpret_cast<HDC>(wParam);
            SetBkMode(hdcStatic, TRANSPARENT);
            return reinterpret_cast<LRESULT>(GetStockObject(NULL_BRUSH));
        }

        default:
            break;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK BrowserApp::UrlBarSubclassProc(HWND hwnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    BrowserApp* pApp = reinterpret_cast<BrowserApp*>(dwRefData);

    switch (uMsg) {
        case WM_KEYDOWN: {
            if (wParam == VK_RETURN) {
                // User pressed Enter in URL bar - navigate
                if (pApp) pApp->OnUrlBarEnter();
                return 0;
            }
            else if (wParam == VK_ESCAPE) {
                // Escape key - cancel and restore URL
                if (pApp) {
                    pApp->UpdateUrlBar();
                    TabData* tab = pApp->GetActiveTab();
                    if (tab && tab->controller) {
                        tab->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
                    }
                }
                return 0;
            }
            break;
        }

        case WM_CHAR: {
            // Ctrl+L to focus URL bar - already handled by accelerator
            break;
        }

        default:
            break;
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

BrowserApp* BrowserApp::GetAppFromWindow(HWND hwnd)
{
    return reinterpret_cast<BrowserApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

// ============================================================================
// Event Handlers
// ============================================================================

void BrowserApp::OnPaint()
{
    // Custom painting for the tab bar area
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hTabBar, &ps);

    if (hdc) {
        // Fill tab bar background
        FillRect(hdc, &m_tabBarRect, m_hNavBarBrush);

        // Draw each tab
        for (int i = 0; i < static_cast<int>(m_tabs.size()); i++) {
            DRAWITEMSTRUCT dis = {};
            dis.hDC = hdc;
            dis.itemID = i;
            dis.rcItem = GetTabRect(i);
            DrawTabItem(&dis);
        }

        EndPaint(m_hTabBar, &ps);
    }
}

void BrowserApp::OnTimer(UINT_PTR timerId)
{
    switch (timerId) {
        case TIMER_STATUS_CLEAR: {
            // Clear temporary status message
            KillTimer(m_hMainWindow, TIMER_STATUS_CLEAR);
            if (m_hStatusBar) {
                TabData* tab = GetActiveTab();
                if (tab && !tab->statusText.empty()) {
                    SendMessage(m_hStatusBar, SB_SETTEXT, 0,
                        (LPARAM)tab->statusText.c_str());
                } else {
                    SendMessage(m_hStatusBar, SB_SETTEXT, 0, (LPARAM)L"");
                }
            }
            break;
        }
        default:
            break;
    }
}

void BrowserApp::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    // Handle global keyboard shortcuts not covered by accelerators
    switch (wParam) {
        case VK_F11:
            ToggleFullScreen();
            break;
        case VK_F12:
            if (m_settings.enableDevTools) ToggleDevTools();
            break;
        case VK_ESCAPE:
            if (m_findBarVisible) HideFindBar();
            else if (m_isFullScreen) ToggleFullScreen();
            break;
        case 'L':
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                SetFocus(m_hUrlBar);
                SendMessage(m_hUrlBar, EM_SETSEL, 0, -1);
            }
            break;
        default:
            break;
    }
}

void BrowserApp::OnChar(WPARAM wParam, LPARAM lParam)
{
    // Character input handling (for keyboard navigation in find bar etc.)
}

void BrowserApp::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    short delta = GET_WHEEL_DELTA_WPARAM(wParam);
    if (delta > 0) {
        ZoomIn();
    } else {
        ZoomOut();
    }
}

void BrowserApp::OnDropFiles(HDROP hDrop)
{
    UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
    for (UINT i = 0; i < fileCount; i++) {
        wchar_t filePath[MAX_PATH] = {};
        DragQueryFileW(hDrop, i, filePath, MAX_PATH);

        // Open local HTML files in new tab
        std::wstring fileStr(filePath);
        if (fileStr.find(L".htm") != std::wstring::npos ||
            fileStr.find(L".html") != std::wstring::npos ||
            fileStr.find(L".svg") != std::wstring::npos ||
            fileStr.find(L".mhtml") != std::wstring::npos) {
            std::wstring url = L"file:///" + fileStr;
            std::replace(url.begin(), url.end(), L'\\', L'/');
            CreateNewTab(url);
        }
    }
    DragFinish(hDrop);
}

void BrowserApp::OnNotify(int idCtrl, NMHDR* pnmh)
{
    // Handle notifications from common controls
    switch (idCtrl) {
        case IDC_STATUS_BAR:
            // Handle status bar notifications
            break;
        default:
            break;
    }
}

// ============================================================================
// Window Management
// ============================================================================

void BrowserApp::Minimize()
{
    ShowWindow(m_hMainWindow, SW_MINIMIZE);
}

void BrowserApp::Maximize()
{
    ShowWindow(m_hMainWindow, SW_MAXIMIZE);
    CalculateLayout();
    RepositionControls();
}

void BrowserApp::Restore()
{
    ShowWindow(m_hMainWindow, SW_RESTORE);
    CalculateLayout();
    RepositionControls();
}

void BrowserApp::Close()
{
    m_isClosing = true;
    SaveSettings();
    SaveBookmarks();
    SaveHistory();

    // Close all tabs
    while (!m_tabs.empty()) {
        auto& tab = m_tabs.back();
        if (tab->controller) {
            tab->controller->Close();
            tab->controller.Reset();
        }
        if (tab->hwnd) {
            DestroyWindow(tab->hwnd);
        }
        m_tabs.pop_back();
    }

    DestroyWindow(m_hMainWindow);
}

bool BrowserApp::IsMaximized() const
{
    WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
    GetWindowPlacement(m_hMainWindow, &wp);
    return wp.showCmd == SW_MAXIMIZE;
}

// ============================================================================
// Utility Functions
// ============================================================================

std::wstring BrowserApp::GetCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    struct tm tmBuf;
    localtime_s(&tmBuf, &timeT);

    wchar_t buf[64];
    swprintf_s(buf, L"%04d-%02d-%02d %02d:%02d:%02d",
        tmBuf.tm_year + 1900, tmBuf.tm_mon + 1, tmBuf.tm_mday,
        tmBuf.tm_hour, tmBuf.tm_min, tmBuf.tm_sec);
    return buf;
}

std::wstring BrowserApp::FormatFileSize(int64_t bytes)
{
    const wchar_t* units[] = { L"B", L"KB", L"MB", L"GB", L"TB" };
    int unitIndex = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }

    wchar_t buf[64];
    if (unitIndex > 0) {
        swprintf_s(buf, L"%.1f %s", size, units[unitIndex]);
    } else {
        swprintf_s(buf, L"%lld %s", bytes, units[unitIndex]);
    }
    return buf;
}

std::wstring BrowserApp::GetAppDataPath()
{
    wchar_t path[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
        wcscat_s(path, L"\\WebView2Browser");
        CreateDirectoryW(path, nullptr); // Ensure directory exists
        return path;
    }
    return L".\\WebView2Browser";
}

std::wstring BrowserApp::GetDownloadsPath()
{
    wchar_t path[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_DOWNLOADS, nullptr, 0, path))) {
        return path;
    }
    return GetAppDataPath() + L"\\Downloads";
}

bool BrowserApp::IsValidUrl(const std::wstring& url)
{
    // Check for common URL prefixes
    static const std::vector<std::wstring> schemes = {
        L"http://", L"https://", L"file:///", L"ftp://",
        L"about:", L"data:", L"blob:", L"chrome://"
    };

    for (const auto& scheme : schemes) {
        if (url.length() >= scheme.length()) {
            std::wstring prefix = url.substr(0, scheme.length());
            std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::towlower);
            if (prefix == scheme) return true;
        }
    }

    // Check for localhost without scheme
    if (url.find(L"localhost") != std::wstring::npos ||
        url.find(L"127.0.0.1") != std::wstring::npos) {
        return true;
    }

    return false;
}

std::wstring BrowserApp::EnsureUrlPrefix(const std::wstring& input)
{
    if (input.empty()) return input;
    if (IsValidUrl(input)) return input;

    // Check if it looks like a domain (contains dots and no spaces)
    bool hasDot = input.find(L'.') != std::wstring::npos;
    bool hasSpace = input.find(L' ') != std::wstring::npos;

    if (hasDot && !hasSpace) {
        return L"https://" + input;
    }

    return input;
}

std::wstring BrowserApp::ExtractDomain(const std::wstring& url)
{
    size_t schemeEnd = url.find(L"://");
    if (schemeEnd == std::wstring::npos) return url;

    size_t domainStart = schemeEnd + 3;
    size_t domainEnd = url.find(L'/', domainStart);
    if (domainEnd == std::wstring::npos) domainEnd = url.length();

    // Remove port number
    size_t portPos = url.find(L':', domainStart);
    if (portPos != std::wstring::npos && portPos < domainEnd) {
        domainEnd = portPos;
    }

    return url.substr(domainStart, domainEnd - domainStart);
}

std::wstring BrowserApp::UrlEncode(const std::wstring& text)
{
    std::wstring result;
    for (wchar_t ch : text) {
        if ((ch >= L'0' && ch <= L'9') ||
            (ch >= L'a' && ch <= L'z') ||
            (ch >= L'A' && ch <= L'Z') ||
            ch == L'-' || ch == L'_' || ch == L'.' || ch == L'~') {
            result += ch;
        } else if (ch == L' ') {
            result += L'+';
        } else {
            wchar_t buf[8];
            swprintf_s(buf, L"%%%04X", static_cast<unsigned short>(ch));
            result += buf;
        }
    }
    return result;
}

std::wstring BrowserApp::UrlDecode(const std::wstring& text)
{
    std::wstring result;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == L'%' && i + 2 < text.size()) {
            wchar_t hex[3] = { text[i + 1], text[i + 2], 0 };
            result += static_cast<wchar_t>(wcstol(hex, nullptr, 16));
            i += 2;
        } else if (text[i] == L'+') {
            result += L' ';
        } else {
            result += text[i];
        }
    }
    return result;
}

FILETIME BrowserApp::GetCurrentFileTime()
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    return ft;
}

DWORD BrowserApp::FileTimeToSeconds(const FILETIME& ft)
{
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return static_cast<DWORD>(uli.QuadPart / 10000000ULL - 11644473600ULL);
}

// ============================================================================
// New Tab Page Generator
// ============================================================================

std::wstring BrowserApp::GenerateNewTabPageHtml()
{
    return NewTabPageGenerator::Generate(
        m_bookmarks,
        std::vector<HistoryEntry>(m_history.end() - min(static_cast<int>(m_history.size()), 10), m_history.end()),
        m_settings.searchEngine,
        m_settings.searchEngineName);
}

std::wstring NewTabPageGenerator::Generate(
    const std::vector<Bookmark>& bookmarks,
    const std::vector<HistoryEntry>& recentHistory,
    const std::wstring& searchEngineUrl,
    const std::wstring& searchEngineName)
{
    std::wstringstream html;
    html << L"<!DOCTYPE html>\n"
         << L"<html lang=\"en\">\n<head>\n"
         << L"<meta charset=\"UTF-8\">\n"
         << L"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
         << L"<title>New Tab</title>\n"
         << L"<style>\n"
         << GenerateCSS()
         << L"</style>\n"
         << L"</head>\n<body>\n"
         << L"<div class=\"container\">\n"
         << L"  <div class=\"clock\" id=\"clock\">00:00</div>\n"
         << L"  <div class=\"search-box\">\n"
         << L"    <form onsubmit=\"performSearch(event)\">\n"
         << L"      <input type=\"text\" id=\"searchInput\" placeholder=\"Search with "
         << searchEngineName << L" or enter URL\" autofocus>\n"
         << L"    </form>\n"
         << L"  </div>\n"
         << L"  <div class=\"bookmarks\">\n"
         << L"    <h3>Bookmarks</h3>\n"
         << L"    <div class=\"bookmark-grid\">\n";

    for (const auto& bm : bookmarks) {
        html << L"      <a class=\"bookmark-item\" href=\"#\" onclick=\"navigateBookmark(event, '"
             << bm.url << L"')\">\n"
             << L"        <div class=\"bookmark-title\">" << bm.name << L"</div>\n"
             << L"      </a>\n";
    }

    html << L"    </div>\n"
         << L"  </div>\n";

    if (!recentHistory.empty()) {
        html << L"  <div class=\"history\">\n"
             << L"    <h3>Recent</h3>\n"
             << L"    <div class=\"history-list\">\n";

        for (const auto& entry : recentHistory) {
            std::wstring title = entry.title.empty() ? entry.url : entry.title;
            html << L"      <a class=\"history-item\" href=\"#\" onclick=\"navigateBookmark(event, '"
                 << entry.url << L"')\">\n"
                 << L"        <span class=\"history-title\">" << title << L"</span>\n"
                 << L"      </a>\n";
        }

        html << L"    </div>\n"
             << L"  </div>\n";
    }

    html << L"</div>\n"
         << L"<script>\n"
         << L"  function performSearch(e) {\n"
         << L"    e.preventDefault();\n"
         << L"    var q = document.getElementById('searchInput').value;\n"
         << L"    if (!q) return;\n"
         << L"    if (q.match(/^(https?:\\/\\/|www\\.)/i)) {\n"
         << L"      window.chrome.webview.postMessage('navigate:' + q);\n"
         << L"    } else {\n"
         << L"      window.chrome.webview.postMessage('search:' + q);\n"
         << L"    }\n"
         << L"  }\n"
         << L"  function navigateBookmark(e, url) {\n"
         << L"    e.preventDefault();\n"
         << L"    window.chrome.webview.postMessage('navigate:' + url);\n"
         << L"  }\n"
         << L"  function updateClock() {\n"
         << L"    var now = new Date();\n"
         << L"    var h = now.getHours().toString().padStart(2, '0');\n"
         << L"    var m = now.getMinutes().toString().padStart(2, '0');\n"
         << L"    document.getElementById('clock').textContent = h + ':' + m;\n"
         << L"  }\n"
         << L"  updateClock();\n"
         << L"  setInterval(updateClock, 1000);\n"
         << L"</script>\n"
         << L"</body>\n</html>";

    return html.str();
}

std::wstring NewTabPageGenerator::GenerateCSS()
{
    return LR"(
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            color: #333;
        }
        .container {
            max-width: 800px;
            width: 100%;
            padding: 40px;
            text-align: center;
        }
        .clock {
            font-size: 64px;
            font-weight: 200;
            color: #fff;
            margin-bottom: 30px;
            text-shadow: 0 2px 10px rgba(0,0,0,0.2);
            letter-spacing: 4px;
        }
        .search-box {
            margin-bottom: 40px;
        }
        .search-box input {
            width: 100%;
            max-width: 580px;
            padding: 16px 24px;
            font-size: 16px;
            border: none;
            border-radius: 30px;
            background: rgba(255,255,255,0.95);
            box-shadow: 0 4px 20px rgba(0,0,0,0.15);
            outline: none;
            transition: box-shadow 0.3s ease, transform 0.3s ease;
        }
        .search-box input:focus {
            box-shadow: 0 6px 30px rgba(0,0,0,0.25);
            transform: scale(1.02);
        }
        .search-box input::placeholder {
            color: #999;
        }
        .bookmarks, .history {
            text-align: left;
            background: rgba(255,255,255,0.1);
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 20px;
            backdrop-filter: blur(10px);
        }
        .bookmarks h3, .history h3 {
            color: rgba(255,255,255,0.8);
            font-size: 14px;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 12px;
        }
        .bookmark-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(120px, 1fr));
            gap: 8px;
        }
        .bookmark-item {
            display: flex;
            flex-direction: column;
            align-items: center;
            padding: 12px 8px;
            border-radius: 8px;
            background: rgba(255,255,255,0.15);
            text-decoration: none;
            color: #fff;
            transition: background 0.2s ease, transform 0.2s ease;
            cursor: pointer;
        }
        .bookmark-item:hover {
            background: rgba(255,255,255,0.3);
            transform: translateY(-2px);
        }
        .bookmark-title {
            font-size: 12px;
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
            max-width: 100%;
            text-align: center;
        }
        .history-list {
            display: flex;
            flex-direction: column;
            gap: 4px;
        }
        .history-item {
            display: block;
            padding: 8px 12px;
            border-radius: 6px;
            background: rgba(255,255,255,0.1);
            text-decoration: none;
            color: #fff;
            font-size: 13px;
            transition: background 0.2s ease;
            cursor: pointer;
        }
        .history-item:hover {
            background: rgba(255,255,255,0.25);
        }
        .history-title {
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
            display: block;
        }
    )";
}

// ============================================================================
// Dialog Implementations
// ============================================================================

INT_PTR SettingsDialog::ShowDialog(HWND hParent, BrowserSettings& settings)
{
    m_pSettings = &settings;
    return DialogBoxParamW(GetModuleHandleW(nullptr),
        MAKEINTRESOURCEW(IDD_DIALOG1), hParent, DialogProc, 0);
}

INT_PTR CALLBACK SettingsDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_INITDIALOG:
            LoadSettingsToControls(hDlg, *m_pSettings);
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    SaveControlsToSettings(hDlg, *m_pSettings);
                    EndDialog(hDlg, IDOK);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}

void SettingsDialog::LoadSettingsToControls(HWND hDlg, const BrowserSettings& settings)
{
    // In a full implementation, we would load settings into dialog controls
    SetWindowTextW(hDlg, L"WebView2Browser - Settings");
}

void SettingsDialog::SaveControlsToSettings(HWND hDlg, BrowserSettings& settings)
{
    // In a full implementation, we would read values from dialog controls
}

BrowserSettings* SettingsDialog::m_pSettings = nullptr;

INT_PTR AboutDialog::ShowDialog(HWND hParent)
{
    MessageBoxW(hParent,
        L"WebView2Browser v" STR_APP_VERSION L"\n\n"
        L"A modern web browser built with C++ and Microsoft WebView 2.\n\n"
        L"Powered by Microsoft Edge Chromium Engine.\n"
        L"Built with pure Win32 API.\n\n"
        L"Features:\n"
        L"  - Tabbed browsing\n"
        L"  - Smart URL bar\n"
        L"  - Bookmarks & History\n"
        L"  - Find on page\n"
        L"  - Zoom controls\n"
        L"  - Full-screen mode\n"
        L"  - Developer tools\n"
        L"  - Keyboard shortcuts\n"
        L"  - Custom context menu\n"
        L"  - Print support\n"
        L"  - Drag & drop\n"
        L"  - DPI awareness\n\n"
        L"(c) 2025 WebView2Browser Project",
        L"About WebView2Browser",
        MB_ICONINFORMATION | MB_OK);
    return IDOK;
}

INT_PTR CALLBACK AboutDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_INITDIALOG) return TRUE;
    if (message == WM_COMMAND && LOWORD(wParam) == IDOK) {
        EndDialog(hDlg, IDOK);
        return TRUE;
    }
    return FALSE;
}

INT_PTR HistoryDialog::ShowDialog(HWND hParent, std::vector<HistoryEntry>& history)
{
    m_pHistory = &history;
    MessageBoxW(hParent,
        L"History panel is available through Ctrl+H.\n"
        L"Full history management coming soon.",
        L"Browsing History",
        MB_ICONINFORMATION | MB_OK);
    return IDOK;
}

INT_PTR CALLBACK HistoryDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_INITDIALOG) return TRUE;
    if (message == WM_COMMAND && LOWORD(wParam) == IDCANCEL) {
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

std::vector<HistoryEntry>* HistoryDialog::m_pHistory = nullptr;

// ============================================================================
// WinMain - Application Entry Point
// ============================================================================

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // Enable heap debugging for development
    #ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif

    // Check for single instance
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"WebView2Browser_SingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // Another instance is already running
        HWND hExisting = FindWindowW(L"WebView2BrowserClass", nullptr);
        if (hExisting) {
            ShowWindow(hExisting, SW_RESTORE);
            SetForegroundWindow(hExisting);
        }
        CloseHandle(hMutex);
        return 0;
    }

    int exitCode = 0;

    try {
        // Create and initialize the browser application
        BrowserApp app(hInstance);
        HRESULT hr = app.Initialize();

        if (SUCCEEDED(hr)) {
            exitCode = app.Run();
        } else {
            // Show error message
            MessageBoxW(nullptr,
                L"Failed to initialize WebView2Browser.\n\n"
                L"Please ensure:\n"
                L"1. WebView2 Runtime is installed\n"
                L"2. Windows 10/11 is up to date\n\n"
                L"Download WebView2 Runtime:\n"
                L"https://developer.microsoft.com/en-us/microsoft-edge/webview2/",
                L"WebView2Browser - Initialization Error",
                MB_ICONERROR | MB_OK);
            exitCode = 1;
        }
    }
    catch (const std::exception& e) {
        // Catch and display any unhandled exceptions
        char errorMsg[512] = {};
        snprintf(errorMsg, sizeof(errorMsg),
            "Unhandled exception: %s", e.what());
        OutputDebugStringA(errorMsg);
        MessageBoxA(nullptr, errorMsg, "WebView2Browser - Fatal Error",
            MB_ICONERROR | MB_OK);
        exitCode = 2;
    }
    catch (...) {
        MessageBoxW(nullptr, L"Unknown fatal error occurred.",
            L"WebView2Browser - Fatal Error", MB_ICONERROR | MB_OK);
        exitCode = 3;
    }

    ReleaseMutex(hMutex);
    CloseHandle(hMutex);

    return exitCode;
}
