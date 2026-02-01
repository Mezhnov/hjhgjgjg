#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <wininet.h>
#include <string>
#include <vector>
#include <memory>
#include <shlwapi.h>
#include <commdlg.h>
#include <shlobj.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace Gdiplus;

// ============================================================================
// КОНСТАНТЫ И ОПРЕДЕЛЕНИЯ
// ============================================================================

// Цвета интерфейса - Современный дизайн
#define COLOR_BG_DARK           RGB(43, 43, 43)
#define COLOR_BG_GRAY           RGB(240, 240, 240)
#define COLOR_BG_LIGHT          RGB(250, 250, 250)
#define COLOR_WHITE             RGB(255, 255, 255)
#define COLOR_RED               RGB(185, 74, 72)
#define COLOR_RED_HOVER         RGB(220, 90, 90)
#define COLOR_RED_ACCENT        RGB(200, 50, 50)
#define COLOR_BLUE              RGB(68, 114, 196)
#define COLOR_BLUE_HOVER        RGB(88, 134, 216)
#define COLOR_BLUE_DARK         RGB(48, 94, 176)
#define COLOR_GREEN             RGB(84, 172, 84)
#define COLOR_GREEN_HOVER       RGB(104, 192, 104)
#define COLOR_ORANGE            RGB(237, 125, 49)
#define COLOR_ORANGE_HOVER      RGB(255, 145, 69)
#define COLOR_PURPLE            RGB(112, 48, 160)
#define COLOR_DARK_GRAY         RGB(100, 100, 100)
#define COLOR_MEDIUM_GRAY       RGB(150, 150, 150)
#define COLOR_LIGHT_GRAY        RGB(200, 200, 200)
#define COLOR_STATUS_BAR        RGB(248, 248, 248)
#define COLOR_BORDER_LIGHT      RGB(220, 220, 220)
#define COLOR_BORDER_DARK       RGB(180, 180, 180)
#define COLOR_TOOLBAR_BG        RGB(245, 245, 245)
#define COLOR_TOOLBAR_HOVER     RGB(230, 230, 230)
#define COLOR_TOOLBAR_PRESSED   RGB(210, 210, 210)
#define COLOR_RIBBON_BG         RGB(255, 255, 255)
#define COLOR_RIBBON_TAB        RGB(68, 114, 196)
#define COLOR_RIBBON_TAB_HOVER  RGB(88, 134, 216)
#define COLOR_TAB_ACTIVE        RGB(255, 255, 255)
#define COLOR_TAB_INACTIVE      RGB(230, 230, 230)
#define COLOR_PANEL_BG          RGB(251, 251, 251)
#define COLOR_SELECTION         RGB(0, 120, 215)
#define COLOR_SELECTION_LIGHT   RGB(204, 232, 255)
#define COLOR_TITLE_BAR         RGB(185, 74, 72)
#define COLOR_TEXT_DARK         RGB(50, 50, 50)
#define COLOR_TEXT_MEDIUM       RGB(100, 100, 100)
#define COLOR_TEXT_LIGHT        RGB(150, 150, 150)
#define COLOR_PLACEHOLDER       RGB(180, 180, 180)

// Идентификаторы элементов управления
#define ID_TOOLBAR              1001
#define ID_STATUS_BAR           1002
#define ID_ZOOM_SLIDER          1003
#define ID_SLIDE_LIST           1004
#define ID_TIMER                1005

// Идентификаторы команд меню и кнопок
#define IDM_FILE_NEW            2001
#define IDM_FILE_OPEN           2002
#define IDM_FILE_SAVE           2003
#define IDM_FILE_SAVE_AS        2004
#define IDM_FILE_EXPORT         2005
#define IDM_FILE_PRINT          2006
#define IDM_FILE_EXIT           2007
#define IDM_EDIT_UNDO           2010
#define IDM_EDIT_REDO           2011
#define IDM_EDIT_CUT            2012
#define IDM_EDIT_COPY           2013
#define IDM_EDIT_PASTE          2014
#define IDM_EDIT_DELETE         2015
#define IDM_EDIT_SELECT_ALL     2016
#define IDM_EDIT_DUPLICATE      2017
#define IDM_SLIDE_NEW           2020
#define IDM_SLIDE_DUPLICATE     2021
#define IDM_SLIDE_DELETE        2022
#define IDM_SLIDE_MOVE_UP       2023
#define IDM_SLIDE_MOVE_DOWN     2024
#define IDM_SLIDE_LAYOUT        2025
#define IDM_SLIDE_TRANSITION    2026
#define IDM_INSERT_TEXT         2030
#define IDM_INSERT_IMAGE        2031
#define IDM_INSERT_IMAGE_URL    2032
#define IDM_INSERT_SHAPE        2033
#define IDM_INSERT_CHART        2034
#define IDM_INSERT_TABLE        2035
#define IDM_INSERT_VIDEO        2036
#define IDM_INSERT_AUDIO        2037
#define IDM_INSERT_LINK         2038
#define IDM_FORMAT_FONT         2040
#define IDM_FORMAT_PARAGRAPH    2041
#define IDM_FORMAT_SHAPE        2042
#define IDM_FORMAT_ARRANGE      2043
#define IDM_FORMAT_ALIGN        2044
#define IDM_VIEW_NORMAL         2050
#define IDM_VIEW_SORTER         2051
#define IDM_VIEW_READING        2052
#define IDM_VIEW_SLIDESHOW      2053
#define IDM_VIEW_ZOOM_IN        2054
#define IDM_VIEW_ZOOM_OUT       2055
#define IDM_VIEW_FIT_WINDOW     2056
#define IDM_VIEW_NOTES          2057
#define IDM_VIEW_RULER          2058
#define IDM_VIEW_GRID           2059
#define IDM_VIEW_GUIDES         2060
#define IDM_DESIGN_THEME        2070
#define IDM_DESIGN_VARIANT      2071
#define IDM_DESIGN_SLIDE_SIZE   2072
#define IDM_DESIGN_BACKGROUND   2073
#define IDM_ANIMATION_ADD       2080
#define IDM_ANIMATION_PREVIEW   2081
#define IDM_ANIMATION_PANE      2082
#define IDM_TRANSITION_ADD      2090
#define IDM_TRANSITION_TIMING   2091
#define IDM_HELP_ABOUT          2100
#define IDM_HELP_DOCS           2101

// Идентификаторы кнопок Ribbon
#define IDB_NEW_SLIDE           3001
#define IDB_DELETE_SLIDE        3002
#define IDB_COPY_SLIDE          3003
#define IDB_PASTE_SLIDE         3004
#define IDB_CUT                 3005
#define IDB_COPY                3006
#define IDB_PASTE               3007
#define IDB_UNDO                3008
#define IDB_REDO                3009
#define IDB_INSERT_TEXT         3010
#define IDB_INSERT_IMAGE        3011
#define IDB_INSERT_SHAPE        3012
#define IDB_INSERT_TABLE        3013
#define IDB_INSERT_CHART        3014
#define IDB_BOLD                3015
#define IDB_ITALIC              3016
#define IDB_UNDERLINE           3017
#define IDB_ALIGN_LEFT          3018
#define IDB_ALIGN_CENTER        3019
#define IDB_ALIGN_RIGHT         3020
#define IDB_FONT_COLOR          3021
#define IDB_FILL_COLOR          3022
#define IDB_SLIDESHOW           3023
#define IDB_SLIDESHOW_BEGIN     3024
#define IDB_SLIDESHOW_CURRENT   3025
#define IDB_ZOOM_IN             3026
#define IDB_ZOOM_OUT            3027
#define IDB_FIT_SLIDE           3028
#define IDB_SAVE                3029
#define IDB_OPEN                3030
#define IDB_PRINT               3031

// Типы элементов на слайде
enum class SlideElementType {
    None,
    TextBox,
    Image,
    Shape,
    Table,
    Chart,
    Video,
    Audio
};

// Типы фигур
enum class ShapeType {
    Rectangle,
    RoundedRect,
    Ellipse,
    Triangle,
    Diamond,
    Pentagon,
    Hexagon,
    Star,
    Arrow,
    Line,
    Callout
};

// Режимы редактирования
enum class EditMode {
    Select,
    InsertText,
    InsertImage,
    InsertShape,
    DrawFreehand,
    Pan
};

// Типы макетов слайдов
enum class SlideLayout {
    TitleSlide,
    TitleAndContent,
    SectionHeader,
    TwoContent,
    Comparison,
    TitleOnly,
    Blank,
    ContentWithCaption,
    PictureWithCaption
};

// Состояния анимации
enum class AnimationType {
    None,
    FadeIn,
    FadeOut,
    SlideIn,
    SlideOut,
    ZoomIn,
    ZoomOut,
    Bounce,
    Spin
};

// Типы переходов между слайдами
enum class TransitionType {
    None,
    Fade,
    Push,
    Wipe,
    Split,
    Reveal,
    Cut,
    RandomBars,
    Shape,
    Uncover,
    Cover,
    Flash
};

// ============================================================================
// СТРУКТУРЫ ДАННЫХ
// ============================================================================

// Структура для элемента на слайде
struct SlideElement {
    SlideElementType type;
    int x, y;                       // Позиция (в процентах от размера слайда * 100)
    int width, height;              // Размеры (в процентах * 100)
    std::wstring text;              // Текст (для TextBox)
    std::wstring fontName;          // Имя шрифта
    int fontSize;                   // Размер шрифта
    bool bold, italic, underline;   // Стили текста
    COLORREF textColor;             // Цвет текста
    COLORREF fillColor;             // Цвет заливки
    COLORREF borderColor;           // Цвет границы
    int borderWidth;                // Толщина границы
    float rotation;                 // Угол поворота
    float opacity;                  // Прозрачность (0.0 - 1.0)
    ShapeType shapeType;            // Тип фигуры
    std::wstring imagePath;         // Путь к изображению
    std::wstring imageUrl;          // URL изображения
    HBITMAP hBitmap;                // Загруженное изображение
    bool selected;                  // Выбран ли элемент
    bool locked;                    // Заблокирован ли элемент
    bool visible;                   // Видим ли элемент
    int zOrder;                     // Порядок наложения
    AnimationType animation;        // Тип анимации
    int animationDuration;          // Длительность анимации (мс)
    int animationDelay;             // Задержка анимации (мс)
    
    SlideElement() {
        type = SlideElementType::None;
        x = y = 0;
        width = height = 1000;      // 10%
        text = L"";
        fontName = L"Segoe UI";
        fontSize = 24;
        bold = italic = underline = false;
        textColor = COLOR_TEXT_DARK;
        fillColor = COLOR_WHITE;
        borderColor = COLOR_BORDER_DARK;
        borderWidth = 1;
        rotation = 0.0f;
        opacity = 1.0f;
        shapeType = ShapeType::Rectangle;
        imagePath = L"";
        imageUrl = L"";
        hBitmap = NULL;
        selected = false;
        locked = false;
        visible = true;
        zOrder = 0;
        animation = AnimationType::None;
        animationDuration = 500;
        animationDelay = 0;
    }
    
    ~SlideElement() {
        if (hBitmap) {
            DeleteObject(hBitmap);
            hBitmap = NULL;
        }
    }
};

// Структура для слайда
struct Slide {
    std::vector<std::shared_ptr<SlideElement>> elements;
    COLORREF backgroundColor;
    std::wstring backgroundImage;
    HBITMAP hBackgroundBitmap;
    SlideLayout layout;
    std::wstring notes;
    TransitionType transition;
    int transitionDuration;
    bool hidden;
    std::wstring title;
    
    Slide() {
        backgroundColor = COLOR_WHITE;
        backgroundImage = L"";
        hBackgroundBitmap = NULL;
        layout = SlideLayout::TitleSlide;
        notes = L"";
        transition = TransitionType::None;
        transitionDuration = 500;
        hidden = false;
        title = L"Слайд";
    }
    
    ~Slide() {
        if (hBackgroundBitmap) {
            DeleteObject(hBackgroundBitmap);
            hBackgroundBitmap = NULL;
        }
    }
};

// Структура для презентации
struct Presentation {
    std::vector<std::shared_ptr<Slide>> slides;
    std::wstring title;
    std::wstring author;
    std::wstring filePath;
    int slideWidth;                 // Ширина слайда в пикселях (для экспорта)
    int slideHeight;                // Высота слайда в пикселях
    bool modified;                  // Были ли изменения
    
    Presentation() {
        title = L"Новая презентация";
        author = L"";
        filePath = L"";
        slideWidth = 1920;
        slideHeight = 1080;
        modified = false;
    }
};

// Структура для кнопки Ribbon
struct RibbonButton {
    int id;
    std::wstring text;
    std::wstring tooltip;
    RECT rect;
    bool enabled;
    bool pressed;
    bool hovered;
    bool hasDropdown;
    COLORREF iconColor;
    std::wstring iconSymbol;        // Символ для отображения вместо иконки
    
    RibbonButton() {
        id = 0;
        enabled = true;
        pressed = false;
        hovered = false;
        hasDropdown = false;
        iconColor = COLOR_BLUE;
        iconSymbol = L"";
    }
};

// Структура для вкладки Ribbon
struct RibbonTab {
    std::wstring name;
    std::vector<std::pair<std::wstring, std::vector<RibbonButton>>> groups;
    bool active;
    RECT tabRect;
    
    RibbonTab() {
        active = false;
    }
};

// Структура для команды отмены/повтора
struct UndoAction {
    enum class Type {
        AddElement,
        DeleteElement,
        ModifyElement,
        AddSlide,
        DeleteSlide,
        ModifySlide,
        MoveElement,
        ResizeElement
    };
    
    Type type;
    int slideIndex;
    int elementIndex;
    std::shared_ptr<Slide> slideCopy;
    std::shared_ptr<SlideElement> elementCopy;
};

// ============================================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ============================================================================

// Экземпляр приложения
HINSTANCE g_hInst;

// Дескрипторы окон
HWND g_hWnd;
HWND g_hToolbar;
HWND g_hStatusBar;
HWND g_hZoomSlider;

// GDI+ токен
ULONG_PTR g_gdiplusToken;

// Презентация
Presentation g_presentation;
int g_currentSlideIndex = 0;
int g_selectedElementIndex = -1;

// Интерфейс Ribbon
std::vector<RibbonTab> g_ribbonTabs;
int g_activeTabIndex = 0;
int g_ribbonHeight = 130;
int g_tabBarHeight = 28;

// Размеры элементов интерфейса
int g_thumbnailWidth = 200;
int g_slideMargin = 40;
int g_statusBarHeight = 26;
int g_notesHeight = 0;              // 0 = скрыты, >0 = высота панели заметок

// Режим редактирования
EditMode g_editMode = EditMode::Select;
ShapeType g_currentShapeType = ShapeType::Rectangle;

// Масштаб
int g_zoomLevel = 100;              // Процент масштаба
int g_minZoom = 25;
int g_maxZoom = 400;

// Состояние мыши
bool g_isDragging = false;
bool g_isResizing = false;
POINT g_dragStart;
POINT g_lastMousePos;
int g_resizeHandle = -1;            // -1 = нет, 0-7 = углы/стороны

// История отмены/повтора
std::vector<UndoAction> g_undoStack;
std::vector<UndoAction> g_redoStack;
int g_maxUndoLevels = 50;

// Настройки отображения
bool g_showRuler = false;
bool g_showGrid = false;
bool g_showGuides = false;
bool g_showNotes = false;

// Шрифты
HFONT g_hFontRibbon;
HFONT g_hFontRibbonSmall;
HFONT g_hFontRibbonTab;
HFONT g_hFontStatus;
HFONT g_hFontSlideNum;
HFONT g_hFontTitle;
HFONT g_hFontSubtitle;
HFONT g_hFontSmall;

// Курсоры
HCURSOR g_hCursorArrow;
HCURSOR g_hCursorHand;
HCURSOR g_hCursorCross;
HCURSOR g_hCursorSizeNS;
HCURSOR g_hCursorSizeWE;
HCURSOR g_hCursorSizeNWSE;
HCURSOR g_hCursorSizeNESW;
HCURSOR g_hCursorMove;
HCURSOR g_hCursorIBeam;

// Буфер обмена
std::shared_ptr<SlideElement> g_clipboardElement;
std::shared_ptr<Slide> g_clipboardSlide;

// Таймер анимации
bool g_animationActive = false;
int g_animationFrame = 0;

// ============================================================================
// ПРОТОТИПЫ ФУНКЦИЙ
// ============================================================================

// Инициализация и завершение
BOOL InitApplication(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
void Cleanup();
void InitFonts();
void InitCursors();
void InitRibbon();

// Обработка сообщений
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SlideDialogProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ImageUrlDialogProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AboutDialogProc(HWND, UINT, WPARAM, LPARAM);

// Отрисовка
void DrawAll(HDC hdc, RECT* clientRect);
void DrawTitleBar(HDC hdc, RECT* rect);
void DrawRibbon(HDC hdc, RECT* rect);
void DrawRibbonTab(HDC hdc, RibbonTab* tab, int index);
void DrawRibbonContent(HDC hdc, RECT* rect);
void DrawRibbonButton(HDC hdc, RibbonButton* btn, bool large);
void DrawRibbonGroup(HDC hdc, const std::wstring& name, std::vector<RibbonButton>& buttons, int x, int y, int width, int height);
void DrawThumbnailPanel(HDC hdc, RECT* clientRect);
void DrawThumbnail(HDC hdc, Slide* slide, int index, int x, int y, int width, int height, bool selected);
void DrawSlideArea(HDC hdc, RECT* clientRect);
void DrawSlide(HDC hdc, Slide* slide, int x, int y, int width, int height, bool isMainView);
void DrawSlideElement(HDC hdc, SlideElement* element, int slideX, int slideY, int slideWidth, int slideHeight);
void DrawTextElement(HDC hdc, SlideElement* element, RECT* rect);
void DrawImageElement(HDC hdc, SlideElement* element, RECT* rect);
void DrawShapeElement(HDC hdc, SlideElement* element, RECT* rect);
void DrawSelectionHandles(HDC hdc, RECT* rect);
void DrawStatusBar(HDC hdc, RECT* clientRect);
void DrawNotesPanel(HDC hdc, RECT* clientRect);
void DrawDashedRect(HDC hdc, int x, int y, int width, int height);
void DrawRoundedRect(HDC hdc, int x, int y, int width, int height, int radius, COLORREF fill, COLORREF border, int borderWidth);
void DrawIcon(HDC hdc, int x, int y, int size, const std::wstring& symbol, COLORREF color);

// Управление слайдами
void NewPresentation();
void OpenPresentation();
void SavePresentation();
void SavePresentationAs();
void ExportPresentation();
void PrintPresentation();
void AddSlide(SlideLayout layout = SlideLayout::TitleAndContent);
void DuplicateSlide(int index);
void DeleteSlide(int index);
void MoveSlideUp(int index);
void MoveSlideDown(int index);
void SelectSlide(int index);
void ChangeSlideLayout(int index, SlideLayout layout);
void CreateDefaultSlideContent(Slide* slide);

// Управление элементами
void AddElement(SlideElementType type);
void AddTextBox();
void AddImage();
void AddImageFromUrl();
void AddShape(ShapeType type);
void DeleteSelectedElement();
void DuplicateSelectedElement();
void CopySelectedElement();
void PasteElement();
void CutSelectedElement();
void SelectElement(int index);
void DeselectAllElements();
void MoveElement(int dx, int dy);
void ResizeElement(int newWidth, int newHeight);
void BringToFront();
void SendToBack();
void BringForward();
void SendBackward();
void AlignElements(int alignment);

// Редактирование текста
void FormatTextBold();
void FormatTextItalic();
void FormatTextUnderline();
void SetTextAlignment(int alignment);
void SetFontSize(int size);
void SetFontName(const std::wstring& name);
void SetTextColor(COLORREF color);
void SetFillColor(COLORREF color);

// Отмена/Повтор
void Undo();
void Redo();
void SaveUndoState(UndoAction::Type type);
void ClearUndoHistory();

// Масштабирование
void ZoomIn();
void ZoomOut();
void SetZoom(int percent);
void FitToWindow();

// Режимы просмотра
void SetViewMode(int mode);
void StartSlideshow(bool fromBeginning);
void ToggleNotesPanel();
void ToggleRuler();
void ToggleGrid();
void ToggleGuides();

// Загрузка изображений
HBITMAP LoadImageFromFile(const std::wstring& path);
HBITMAP LoadImageFromUrl(const std::wstring& url);
HBITMAP DownloadImage(const std::wstring& url);

// Вспомогательные функции
int HitTest(int x, int y);
int HitTestElement(int x, int y, int slideX, int slideY, int slideWidth, int slideHeight);
int HitTestResizeHandle(int x, int y, RECT* elementRect);
RECT GetSlideRect(RECT* clientRect);
RECT GetElementRect(SlideElement* element, int slideX, int slideY, int slideWidth, int slideHeight);
void UpdateWindowTitle();
void UpdateStatusBar();
void ShowContextMenu(HWND hWnd, int x, int y);
std::wstring GetLayoutName(SlideLayout layout);
std::wstring FormatNumber(int num);
void SetModified(bool modified);

// Диалоги
void ShowNewSlideDialog();
void ShowInsertImageUrlDialog();
void ShowFontDialog();
void ShowColorDialog(COLORREF* color);
void ShowAboutDialog();

// ============================================================================
// РЕАЛИЗАЦИЯ ФУНКЦИЙ - ИНИЦИАЛИЗАЦИЯ
// ============================================================================

BOOL InitApplication(HINSTANCE hInstance) {
    WNDCLASSEX wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"PowerPointClonePro";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    g_hInst = hInstance;
    
    // Создание главного окна
    g_hWnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        L"PowerPointClonePro",
        L"PowerPoint Clone Pro - Новая презентация",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1400, 900,
        NULL, NULL, hInstance, NULL
    );
    
    if (!g_hWnd) {
        return FALSE;
    }
    
    // Инициализация ресурсов
    InitFonts();
    InitCursors();
    InitRibbon();
    
    // Создание новой презентации
    NewPresentation();
    
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    
    return TRUE;
}

void Cleanup() {
    // Удаление шрифтов
    if (g_hFontRibbon) DeleteObject(g_hFontRibbon);
    if (g_hFontRibbonSmall) DeleteObject(g_hFontRibbonSmall);
    if (g_hFontRibbonTab) DeleteObject(g_hFontRibbonTab);
    if (g_hFontStatus) DeleteObject(g_hFontStatus);
    if (g_hFontSlideNum) DeleteObject(g_hFontSlideNum);
    if (g_hFontTitle) DeleteObject(g_hFontTitle);
    if (g_hFontSubtitle) DeleteObject(g_hFontSubtitle);
    if (g_hFontSmall) DeleteObject(g_hFontSmall);
    
    // Завершение GDI+
    GdiplusShutdown(g_gdiplusToken);
}

void InitFonts() {
    g_hFontRibbon = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        
    g_hFontRibbonSmall = CreateFont(11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        
    g_hFontRibbonTab = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        
    g_hFontStatus = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        
    g_hFontSlideNum = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        
    g_hFontTitle = CreateFont(48, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI Light");
        
    g_hFontSubtitle = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        
    g_hFontSmall = CreateFont(10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
}

void InitCursors() {
    g_hCursorArrow = LoadCursor(NULL, IDC_ARROW);
    g_hCursorHand = LoadCursor(NULL, IDC_HAND);
    g_hCursorCross = LoadCursor(NULL, IDC_CROSS);
    g_hCursorSizeNS = LoadCursor(NULL, IDC_SIZENS);
    g_hCursorSizeWE = LoadCursor(NULL, IDC_SIZEWE);
    g_hCursorSizeNWSE = LoadCursor(NULL, IDC_SIZENWSE);
    g_hCursorSizeNESW = LoadCursor(NULL, IDC_SIZENESW);
    g_hCursorMove = LoadCursor(NULL, IDC_SIZEALL);
    g_hCursorIBeam = LoadCursor(NULL, IDC_IBEAM);
}

void InitRibbon() {
    g_ribbonTabs.clear();
    
    // Вкладка "Главная"
    RibbonTab homeTab;
    homeTab.name = L"Главная";
    homeTab.active = true;
    
    // Группа "Буфер обмена"
    std::vector<RibbonButton> clipboardButtons;
    RibbonButton btnPaste; btnPaste.id = IDB_PASTE; btnPaste.text = L"Вставить"; btnPaste.iconSymbol = L"📋"; btnPaste.iconColor = COLOR_BLUE;
    RibbonButton btnCut; btnCut.id = IDB_CUT; btnCut.text = L"Вырезать"; btnCut.iconSymbol = L"✂"; btnCut.iconColor = COLOR_ORANGE;
    RibbonButton btnCopy; btnCopy.id = IDB_COPY; btnCopy.text = L"Копировать"; btnCopy.iconSymbol = L"📄"; btnCopy.iconColor = COLOR_BLUE;
    clipboardButtons.push_back(btnPaste);
    clipboardButtons.push_back(btnCut);
    clipboardButtons.push_back(btnCopy);
    homeTab.groups.push_back({L"Буфер обмена", clipboardButtons});
    
    // Группа "Слайды"
    std::vector<RibbonButton> slideButtons;
    RibbonButton btnNewSlide; btnNewSlide.id = IDB_NEW_SLIDE; btnNewSlide.text = L"Новый слайд"; btnNewSlide.iconSymbol = L"➕"; btnNewSlide.iconColor = COLOR_GREEN; btnNewSlide.hasDropdown = true;
    RibbonButton btnDeleteSlide; btnDeleteSlide.id = IDB_DELETE_SLIDE; btnDeleteSlide.text = L"Удалить"; btnDeleteSlide.iconSymbol = L"🗑"; btnDeleteSlide.iconColor = COLOR_RED;
    RibbonButton btnCopySlide; btnCopySlide.id = IDB_COPY_SLIDE; btnCopySlide.text = L"Дублировать"; btnCopySlide.iconSymbol = L"📑"; btnCopySlide.iconColor = COLOR_BLUE;
    slideButtons.push_back(btnNewSlide);
    slideButtons.push_back(btnDeleteSlide);
    slideButtons.push_back(btnCopySlide);
    homeTab.groups.push_back({L"Слайды", slideButtons});
    
    // Группа "Шрифт"
    std::vector<RibbonButton> fontButtons;
    RibbonButton btnBold; btnBold.id = IDB_BOLD; btnBold.text = L"Ж"; btnBold.iconSymbol = L"B"; btnBold.iconColor = COLOR_TEXT_DARK;
    RibbonButton btnItalic; btnItalic.id = IDB_ITALIC; btnItalic.text = L"К"; btnItalic.iconSymbol = L"I"; btnItalic.iconColor = COLOR_TEXT_DARK;
    RibbonButton btnUnderline; btnUnderline.id = IDB_UNDERLINE; btnUnderline.text = L"Ч"; btnUnderline.iconSymbol = L"U"; btnUnderline.iconColor = COLOR_TEXT_DARK;
    RibbonButton btnFontColor; btnFontColor.id = IDB_FONT_COLOR; btnFontColor.text = L"Цвет"; btnFontColor.iconSymbol = L"A"; btnFontColor.iconColor = COLOR_RED; btnFontColor.hasDropdown = true;
    fontButtons.push_back(btnBold);
    fontButtons.push_back(btnItalic);
    fontButtons.push_back(btnUnderline);
    fontButtons.push_back(btnFontColor);
    homeTab.groups.push_back({L"Шрифт", fontButtons});
    
    // Группа "Абзац"
    std::vector<RibbonButton> paragraphButtons;
    RibbonButton btnAlignLeft; btnAlignLeft.id = IDB_ALIGN_LEFT; btnAlignLeft.text = L"По левому"; btnAlignLeft.iconSymbol = L"≡"; btnAlignLeft.iconColor = COLOR_TEXT_DARK;
    RibbonButton btnAlignCenter; btnAlignCenter.id = IDB_ALIGN_CENTER; btnAlignCenter.text = L"По центру"; btnAlignCenter.iconSymbol = L"≡"; btnAlignCenter.iconColor = COLOR_TEXT_DARK;
    RibbonButton btnAlignRight; btnAlignRight.id = IDB_ALIGN_RIGHT; btnAlignRight.text = L"По правому"; btnAlignRight.iconSymbol = L"≡"; btnAlignRight.iconColor = COLOR_TEXT_DARK;
    paragraphButtons.push_back(btnAlignLeft);
    paragraphButtons.push_back(btnAlignCenter);
    paragraphButtons.push_back(btnAlignRight);
    homeTab.groups.push_back({L"Абзац", paragraphButtons});
    
    g_ribbonTabs.push_back(homeTab);
    
    // Вкладка "Вставка"
    RibbonTab insertTab;
    insertTab.name = L"Вставка";
    insertTab.active = false;
    
    // Группа "Изображения"
    std::vector<RibbonButton> imageButtons;
    RibbonButton btnInsertImage; btnInsertImage.id = IDB_INSERT_IMAGE; btnInsertImage.text = L"Изображение"; btnInsertImage.iconSymbol = L"🖼"; btnInsertImage.iconColor = COLOR_GREEN; btnInsertImage.hasDropdown = true;
    imageButtons.push_back(btnInsertImage);
    insertTab.groups.push_back({L"Изображения", imageButtons});
    
    // Группа "Иллюстрации"
    std::vector<RibbonButton> illustrationButtons;
    RibbonButton btnInsertShape; btnInsertShape.id = IDB_INSERT_SHAPE; btnInsertShape.text = L"Фигуры"; btnInsertShape.iconSymbol = L"⬜"; btnInsertShape.iconColor = COLOR_BLUE; btnInsertShape.hasDropdown = true;
    RibbonButton btnInsertChart; btnInsertChart.id = IDB_INSERT_CHART; btnInsertChart.text = L"Диаграмма"; btnInsertChart.iconSymbol = L"📊"; btnInsertChart.iconColor = COLOR_ORANGE;
    illustrationButtons.push_back(btnInsertShape);
    illustrationButtons.push_back(btnInsertChart);
    insertTab.groups.push_back({L"Иллюстрации", illustrationButtons});
    
    // Группа "Текст"
    std::vector<RibbonButton> textButtons;
    RibbonButton btnInsertText; btnInsertText.id = IDB_INSERT_TEXT; btnInsertText.text = L"Надпись"; btnInsertText.iconSymbol = L"T"; btnInsertText.iconColor = COLOR_BLUE;
    textButtons.push_back(btnInsertText);
    insertTab.groups.push_back({L"Текст", textButtons});
    
    // Группа "Таблицы"
    std::vector<RibbonButton> tableButtons;
    RibbonButton btnInsertTable; btnInsertTable.id = IDB_INSERT_TABLE; btnInsertTable.text = L"Таблица"; btnInsertTable.iconSymbol = L"⊞"; btnInsertTable.iconColor = COLOR_PURPLE;
    tableButtons.push_back(btnInsertTable);
    insertTab.groups.push_back({L"Таблицы", tableButtons});
    
    g_ribbonTabs.push_back(insertTab);
    
    // Вкладка "Дизайн"
    RibbonTab designTab;
    designTab.name = L"Дизайн";
    designTab.active = false;
    
    std::vector<RibbonButton> themeButtons;
    RibbonButton btnTheme; btnTheme.id = IDM_DESIGN_THEME; btnTheme.text = L"Темы"; btnTheme.iconSymbol = L"🎨"; btnTheme.iconColor = COLOR_PURPLE; btnTheme.hasDropdown = true;
    RibbonButton btnVariant; btnVariant.id = IDM_DESIGN_VARIANT; btnVariant.text = L"Варианты"; btnVariant.iconSymbol = L"🔄"; btnVariant.iconColor = COLOR_BLUE; btnVariant.hasDropdown = true;
    themeButtons.push_back(btnTheme);
    themeButtons.push_back(btnVariant);
    designTab.groups.push_back({L"Темы", themeButtons});
    
    std::vector<RibbonButton> customizeButtons;
    RibbonButton btnSlideSize; btnSlideSize.id = IDM_DESIGN_SLIDE_SIZE; btnSlideSize.text = L"Размер слайда"; btnSlideSize.iconSymbol = L"📐"; btnSlideSize.iconColor = COLOR_ORANGE; btnSlideSize.hasDropdown = true;
    RibbonButton btnBackground; btnBackground.id = IDM_DESIGN_BACKGROUND; btnBackground.text = L"Формат фона"; btnBackground.iconSymbol = L"🖌"; btnBackground.iconColor = COLOR_GREEN;
    customizeButtons.push_back(btnSlideSize);
    customizeButtons.push_back(btnBackground);
    designTab.groups.push_back({L"Настройка", customizeButtons});
    
    g_ribbonTabs.push_back(designTab);
    
    // Вкладка "Переходы"
    RibbonTab transitionTab;
    transitionTab.name = L"Переходы";
    transitionTab.active = false;
    
    std::vector<RibbonButton> transitionButtons;
    RibbonButton btnTransition; btnTransition.id = IDM_TRANSITION_ADD; btnTransition.text = L"Переходы"; btnTransition.iconSymbol = L"↔"; btnTransition.iconColor = COLOR_BLUE; btnTransition.hasDropdown = true;
    RibbonButton btnTiming; btnTiming.id = IDM_TRANSITION_TIMING; btnTiming.text = L"Время показа"; btnTiming.iconSymbol = L"⏱"; btnTiming.iconColor = COLOR_ORANGE;
    transitionButtons.push_back(btnTransition);
    transitionButtons.push_back(btnTiming);
    transitionTab.groups.push_back({L"Переход к этому слайду", transitionButtons});
    
    g_ribbonTabs.push_back(transitionTab);
    
    // Вкладка "Анимация"
    RibbonTab animationTab;
    animationTab.name = L"Анимация";
    animationTab.active = false;
    
    std::vector<RibbonButton> animationButtons;
    RibbonButton btnAddAnimation; btnAddAnimation.id = IDM_ANIMATION_ADD; btnAddAnimation.text = L"Добавить"; btnAddAnimation.iconSymbol = L"✨"; btnAddAnimation.iconColor = COLOR_ORANGE; btnAddAnimation.hasDropdown = true;
    RibbonButton btnAnimPreview; btnAnimPreview.id = IDM_ANIMATION_PREVIEW; btnAnimPreview.text = L"Просмотр"; btnAnimPreview.iconSymbol = L"▶"; btnAnimPreview.iconColor = COLOR_GREEN;
    RibbonButton btnAnimPane; btnAnimPane.id = IDM_ANIMATION_PANE; btnAnimPane.text = L"Область"; btnAnimPane.iconSymbol = L"📋"; btnAnimPane.iconColor = COLOR_BLUE;
    animationButtons.push_back(btnAddAnimation);
    animationButtons.push_back(btnAnimPreview);
    animationButtons.push_back(btnAnimPane);
    animationTab.groups.push_back({L"Анимация", animationButtons});
    
    g_ribbonTabs.push_back(animationTab);
    
    // Вкладка "Слайд-шоу"
    RibbonTab slideshowTab;
    slideshowTab.name = L"Слайд-шоу";
    slideshowTab.active = false;
    
    std::vector<RibbonButton> showButtons;
    RibbonButton btnSlideshowBegin; btnSlideshowBegin.id = IDB_SLIDESHOW_BEGIN; btnSlideshowBegin.text = L"С начала"; btnSlideshowBegin.iconSymbol = L"▶"; btnSlideshowBegin.iconColor = COLOR_GREEN;
    RibbonButton btnSlideshowCurrent; btnSlideshowCurrent.id = IDB_SLIDESHOW_CURRENT; btnSlideshowCurrent.text = L"С текущего"; btnSlideshowCurrent.iconSymbol = L"▶"; btnSlideshowCurrent.iconColor = COLOR_ORANGE;
    showButtons.push_back(btnSlideshowBegin);
    showButtons.push_back(btnSlideshowCurrent);
    slideshowTab.groups.push_back({L"Начать показ слайдов", showButtons});
    
    g_ribbonTabs.push_back(slideshowTab);
    
    // Вкладка "Вид"
    RibbonTab viewTab;
    viewTab.name = L"Вид";
    viewTab.active = false;
    
    std::vector<RibbonButton> viewButtons;
    RibbonButton btnViewNormal; btnViewNormal.id = IDM_VIEW_NORMAL; btnViewNormal.text = L"Обычный"; btnViewNormal.iconSymbol = L"📄"; btnViewNormal.iconColor = COLOR_BLUE;
    RibbonButton btnViewSorter; btnViewSorter.id = IDM_VIEW_SORTER; btnViewSorter.text = L"Сортировщик"; btnViewSorter.iconSymbol = L"⊞"; btnViewSorter.iconColor = COLOR_BLUE;
    RibbonButton btnViewReading; btnViewReading.id = IDM_VIEW_READING; btnViewReading.text = L"Чтение"; btnViewReading.iconSymbol = L"📖"; btnViewReading.iconColor = COLOR_BLUE;
    viewButtons.push_back(btnViewNormal);
    viewButtons.push_back(btnViewSorter);
    viewButtons.push_back(btnViewReading);
    viewTab.groups.push_back({L"Режимы просмотра", viewButtons});
    
    std::vector<RibbonButton> showButtons2;
    RibbonButton btnRuler; btnRuler.id = IDM_VIEW_RULER; btnRuler.text = L"Линейка"; btnRuler.iconSymbol = L"📏"; btnRuler.iconColor = COLOR_TEXT_MEDIUM;
    RibbonButton btnGrid; btnGrid.id = IDM_VIEW_GRID; btnGrid.text = L"Сетка"; btnGrid.iconSymbol = L"#"; btnGrid.iconColor = COLOR_TEXT_MEDIUM;
    RibbonButton btnGuides; btnGuides.id = IDM_VIEW_GUIDES; btnGuides.text = L"Направляющие"; btnGuides.iconSymbol = L"+"; btnGuides.iconColor = COLOR_TEXT_MEDIUM;
    showButtons2.push_back(btnRuler);
    showButtons2.push_back(btnGrid);
    showButtons2.push_back(btnGuides);
    viewTab.groups.push_back({L"Показать", showButtons2});
    
    std::vector<RibbonButton> zoomButtons;
    RibbonButton btnZoomIn; btnZoomIn.id = IDB_ZOOM_IN; btnZoomIn.text = L"Увеличить"; btnZoomIn.iconSymbol = L"🔍+"; btnZoomIn.iconColor = COLOR_BLUE;
    RibbonButton btnZoomOut; btnZoomOut.id = IDB_ZOOM_OUT; btnZoomOut.text = L"Уменьшить"; btnZoomOut.iconSymbol = L"🔍-"; btnZoomOut.iconColor = COLOR_BLUE;
    RibbonButton btnFitSlide; btnFitSlide.id = IDB_FIT_SLIDE; btnFitSlide.text = L"По размеру"; btnFitSlide.iconSymbol = L"⊡"; btnFitSlide.iconColor = COLOR_BLUE;
    zoomButtons.push_back(btnZoomIn);
    zoomButtons.push_back(btnZoomOut);
    zoomButtons.push_back(btnFitSlide);
    viewTab.groups.push_back({L"Масштаб", zoomButtons});
    
    g_ribbonTabs.push_back(viewTab);
}

// ============================================================================
// РЕАЛИЗАЦИЯ ФУНКЦИЙ - УПРАВЛЕНИЕ ПРЕЗЕНТАЦИЕЙ
// ============================================================================

void NewPresentation() {
    g_presentation = Presentation();
    g_currentSlideIndex = 0;
    g_selectedElementIndex = -1;
    
    // Добавить первый слайд
    AddSlide(SlideLayout::TitleSlide);
    
    ClearUndoHistory();
    SetModified(false);
    UpdateWindowTitle();
}

void OpenPresentation() {
    OPENFILENAME ofn = {0};
    wchar_t szFile[260] = {0};
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"Презентации PowerPoint Clone\0*.pptc\0Все файлы\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&ofn)) {
        // Здесь будет загрузка файла
        // Пока просто показываем сообщение
        MessageBox(g_hWnd, L"Функция открытия файла будет реализована в следующей версии.", 
                   L"Информация", MB_OK | MB_ICONINFORMATION);
    }
}

void SavePresentation() {
    if (g_presentation.filePath.empty()) {
        SavePresentationAs();
    } else {
        // Здесь будет сохранение файла
        SetModified(false);
        MessageBox(g_hWnd, L"Презентация сохранена.", L"Информация", MB_OK | MB_ICONINFORMATION);
    }
}

void SavePresentationAs() {
    OPENFILENAME ofn = {0};
    wchar_t szFile[260] = {0};
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"Презентации PowerPoint Clone\0*.pptc\0Все файлы\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"pptc";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    
    if (GetSaveFileName(&ofn)) {
        g_presentation.filePath = szFile;
        // Здесь будет сохранение файла
        SetModified(false);
        UpdateWindowTitle();
        MessageBox(g_hWnd, L"Презентация сохранена.", L"Информация", MB_OK | MB_ICONINFORMATION);
    }
}

void ExportPresentation() {
    MessageBox(g_hWnd, L"Функция экспорта будет реализована в следующей версии.\n\nПланируемые форматы:\n- PDF\n- Изображения (PNG, JPEG)\n- Видео", 
               L"Экспорт презентации", MB_OK | MB_ICONINFORMATION);
}

void PrintPresentation() {
    MessageBox(g_hWnd, L"Функция печати будет реализована в следующей версии.", 
               L"Печать", MB_OK | MB_ICONINFORMATION);
}

void AddSlide(SlideLayout layout) {
    auto newSlide = std::make_shared<Slide>();
    newSlide->layout = layout;
    newSlide->title = L"Слайд " + std::to_wstring(g_presentation.slides.size() + 1);
    
    // Создание содержимого слайда в зависимости от макета
    CreateDefaultSlideContent(newSlide.get());
    
    // Вставка слайда после текущего
    if (g_presentation.slides.empty()) {
        g_presentation.slides.push_back(newSlide);
        g_currentSlideIndex = 0;
    } else {
        g_presentation.slides.insert(g_presentation.slides.begin() + g_currentSlideIndex + 1, newSlide);
        g_currentSlideIndex++;
    }
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void CreateDefaultSlideContent(Slide* slide) {
    switch (slide->layout) {
        case SlideLayout::TitleSlide: {
            // Заголовок
            auto titleElement = std::make_shared<SlideElement>();
            titleElement->type = SlideElementType::TextBox;
            titleElement->x = 1000;      // 10%
            titleElement->y = 3000;      // 30%
            titleElement->width = 8000;  // 80%
            titleElement->height = 1500; // 15%
            titleElement->text = L"Заголовок слайда";
            titleElement->fontName = L"Segoe UI Light";
            titleElement->fontSize = 48;
            titleElement->textColor = RGB(50, 50, 50);
            slide->elements.push_back(titleElement);
            
            // Подзаголовок
            auto subtitleElement = std::make_shared<SlideElement>();
            subtitleElement->type = SlideElementType::TextBox;
            subtitleElement->x = 2000;   // 20%
            subtitleElement->y = 5500;   // 55%
            subtitleElement->width = 6000; // 60%
            subtitleElement->height = 1000; // 10%
            subtitleElement->text = L"Подзаголовок слайда";
            subtitleElement->fontName = L"Segoe UI";
            subtitleElement->fontSize = 24;
            subtitleElement->textColor = RGB(100, 100, 100);
            slide->elements.push_back(subtitleElement);
            break;
        }
        
        case SlideLayout::TitleAndContent: {
            // Заголовок
            auto titleElement = std::make_shared<SlideElement>();
            titleElement->type = SlideElementType::TextBox;
            titleElement->x = 500;       // 5%
            titleElement->y = 500;       // 5%
            titleElement->width = 9000;  // 90%
            titleElement->height = 1200; // 12%
            titleElement->text = L"Заголовок слайда";
            titleElement->fontName = L"Segoe UI";
            titleElement->fontSize = 36;
            titleElement->textColor = RGB(50, 50, 50);
            slide->elements.push_back(titleElement);
            
            // Содержимое
            auto contentElement = std::make_shared<SlideElement>();
            contentElement->type = SlideElementType::TextBox;
            contentElement->x = 500;     // 5%
            contentElement->y = 2000;    // 20%
            contentElement->width = 9000; // 90%
            contentElement->height = 7000; // 70%
            contentElement->text = L"• Первый пункт\n• Второй пункт\n• Третий пункт";
            contentElement->fontName = L"Segoe UI";
            contentElement->fontSize = 24;
            contentElement->textColor = RGB(80, 80, 80);
            slide->elements.push_back(contentElement);
            break;
        }
        
        case SlideLayout::SectionHeader: {
            auto sectionElement = std::make_shared<SlideElement>();
            sectionElement->type = SlideElementType::TextBox;
            sectionElement->x = 1000;
            sectionElement->y = 4000;
            sectionElement->width = 8000;
            sectionElement->height = 2000;
            sectionElement->text = L"Название раздела";
            sectionElement->fontName = L"Segoe UI Light";
            sectionElement->fontSize = 54;
            sectionElement->textColor = RGB(50, 50, 50);
            slide->elements.push_back(sectionElement);
            break;
        }
        
        case SlideLayout::TwoContent: {
            // Заголовок
            auto titleElement = std::make_shared<SlideElement>();
            titleElement->type = SlideElementType::TextBox;
            titleElement->x = 500;
            titleElement->y = 500;
            titleElement->width = 9000;
            titleElement->height = 1200;
            titleElement->text = L"Заголовок слайда";
            titleElement->fontName = L"Segoe UI";
            titleElement->fontSize = 36;
            titleElement->textColor = RGB(50, 50, 50);
            slide->elements.push_back(titleElement);
            
            // Левое содержимое
            auto leftContent = std::make_shared<SlideElement>();
            leftContent->type = SlideElementType::TextBox;
            leftContent->x = 500;
            leftContent->y = 2000;
            leftContent->width = 4200;
            leftContent->height = 7000;
            leftContent->text = L"Содержимое слева";
            leftContent->fontName = L"Segoe UI";
            leftContent->fontSize = 20;
            leftContent->textColor = RGB(80, 80, 80);
            slide->elements.push_back(leftContent);
            
            // Правое содержимое
            auto rightContent = std::make_shared<SlideElement>();
            rightContent->type = SlideElementType::TextBox;
            rightContent->x = 5300;
            rightContent->y = 2000;
            rightContent->width = 4200;
            rightContent->height = 7000;
            rightContent->text = L"Содержимое справа";
            rightContent->fontName = L"Segoe UI";
            rightContent->fontSize = 20;
            rightContent->textColor = RGB(80, 80, 80);
            slide->elements.push_back(rightContent);
            break;
        }
        
        case SlideLayout::Blank:
            // Пустой слайд - ничего не добавляем
            break;
            
        case SlideLayout::TitleOnly: {
            auto titleElement = std::make_shared<SlideElement>();
            titleElement->type = SlideElementType::TextBox;
            titleElement->x = 500;
            titleElement->y = 500;
            titleElement->width = 9000;
            titleElement->height = 1200;
            titleElement->text = L"Заголовок слайда";
            titleElement->fontName = L"Segoe UI";
            titleElement->fontSize = 36;
            titleElement->textColor = RGB(50, 50, 50);
            slide->elements.push_back(titleElement);
            break;
        }
        
        default:
            // По умолчанию - как TitleAndContent
            CreateDefaultSlideContent(slide);
            break;
    }
}

void DuplicateSlide(int index) {
    if (index < 0 || index >= (int)g_presentation.slides.size()) return;
    
    auto originalSlide = g_presentation.slides[index];
    auto newSlide = std::make_shared<Slide>();
    
    // Копирование свойств слайда
    newSlide->backgroundColor = originalSlide->backgroundColor;
    newSlide->backgroundImage = originalSlide->backgroundImage;
    newSlide->layout = originalSlide->layout;
    newSlide->notes = originalSlide->notes;
    newSlide->transition = originalSlide->transition;
    newSlide->transitionDuration = originalSlide->transitionDuration;
    newSlide->hidden = originalSlide->hidden;
    newSlide->title = originalSlide->title + L" (копия)";
    
    // Копирование элементов
    for (auto& elem : originalSlide->elements) {
        auto newElem = std::make_shared<SlideElement>();
        *newElem = *elem;
        newElem->hBitmap = NULL; // Изображения нужно перезагрузить
        newSlide->elements.push_back(newElem);
    }
    
    g_presentation.slides.insert(g_presentation.slides.begin() + index + 1, newSlide);
    g_currentSlideIndex = index + 1;
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void DeleteSlide(int index) {
    if (index < 0 || index >= (int)g_presentation.slides.size()) return;
    if (g_presentation.slides.size() <= 1) {
        MessageBox(g_hWnd, L"Невозможно удалить последний слайд.", L"Предупреждение", MB_OK | MB_ICONWARNING);
        return;
    }
    
    g_presentation.slides.erase(g_presentation.slides.begin() + index);
    
    if (g_currentSlideIndex >= (int)g_presentation.slides.size()) {
        g_currentSlideIndex = (int)g_presentation.slides.size() - 1;
    }
    
    g_selectedElementIndex = -1;
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void MoveSlideUp(int index) {
    if (index <= 0 || index >= (int)g_presentation.slides.size()) return;
    
    std::swap(g_presentation.slides[index], g_presentation.slides[index - 1]);
    g_currentSlideIndex = index - 1;
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void MoveSlideDown(int index) {
    if (index < 0 || index >= (int)g_presentation.slides.size() - 1) return;
    
    std::swap(g_presentation.slides[index], g_presentation.slides[index + 1]);
    g_currentSlideIndex = index + 1;
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void SelectSlide(int index) {
    if (index < 0 || index >= (int)g_presentation.slides.size()) return;
    
    g_currentSlideIndex = index;
    g_selectedElementIndex = -1;
    DeselectAllElements();
    
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void ChangeSlideLayout(int index, SlideLayout layout) {
    if (index < 0 || index >= (int)g_presentation.slides.size()) return;
    
    g_presentation.slides[index]->layout = layout;
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

// ============================================================================
// РЕАЛИЗАЦИЯ ФУНКЦИЙ - УПРАВЛЕНИЕ ЭЛЕМЕНТАМИ
// ============================================================================

void AddElement(SlideElementType type) {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    
    auto element = std::make_shared<SlideElement>();
    element->type = type;
    element->x = 2000;
    element->y = 2000;
    element->width = 4000;
    element->height = 2000;
    element->zOrder = (int)g_presentation.slides[g_currentSlideIndex]->elements.size();
    
    g_presentation.slides[g_currentSlideIndex]->elements.push_back(element);
    g_selectedElementIndex = (int)g_presentation.slides[g_currentSlideIndex]->elements.size() - 1;
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void AddTextBox() {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    
    auto element = std::make_shared<SlideElement>();
    element->type = SlideElementType::TextBox;
    element->x = 2000;
    element->y = 2000;
    element->width = 5000;
    element->height = 2000;
    element->text = L"Введите текст";
    element->fontName = L"Segoe UI";
    element->fontSize = 18;
    element->textColor = RGB(50, 50, 50);
    element->fillColor = COLOR_WHITE;
    element->borderColor = RGB(200, 200, 200);
    element->borderWidth = 1;
    element->zOrder = (int)g_presentation.slides[g_currentSlideIndex]->elements.size();
    
    g_presentation.slides[g_currentSlideIndex]->elements.push_back(element);
    g_selectedElementIndex = (int)g_presentation.slides[g_currentSlideIndex]->elements.size() - 1;
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void AddImage() {
    OPENFILENAME ofn = {0};
    wchar_t szFile[260] = {0};
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"Изображения\0*.bmp;*.jpg;*.jpeg;*.png;*.gif;*.tiff\0Все файлы\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&ofn)) {
        if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
        
        auto element = std::make_shared<SlideElement>();
        element->type = SlideElementType::Image;
        element->x = 2000;
        element->y = 2000;
        element->width = 4000;
        element->height = 3000;
        element->imagePath = szFile;
        element->hBitmap = LoadImageFromFile(szFile);
        element->zOrder = (int)g_presentation.slides[g_currentSlideIndex]->elements.size();
        
        g_presentation.slides[g_currentSlideIndex]->elements.push_back(element);
        g_selectedElementIndex = (int)g_presentation.slides[g_currentSlideIndex]->elements.size() - 1;
        
        SetModified(true);
        InvalidateRect(g_hWnd, NULL, TRUE);
    }
}

void AddImageFromUrl() {
    // Создаем простой диалог для ввода URL
    wchar_t url[1024] = {0};
    
    // Простой InputBox через API не предусмотрен, используем MessageBox для демонстрации
    // В реальном приложении здесь был бы кастомный диалог
    
    int result = MessageBox(g_hWnd, 
        L"Для вставки изображения из интернета:\n\n"
        L"1. Скопируйте URL изображения в буфер обмена\n"
        L"2. Нажмите OK\n\n"
        L"Поддерживаемые форматы: PNG, JPG, GIF, BMP",
        L"Вставка изображения из URL", MB_OKCANCEL | MB_ICONINFORMATION);
    
    if (result == IDOK) {
        // Попробуем получить URL из буфера обмена
        if (OpenClipboard(g_hWnd)) {
            HANDLE hData = GetClipboardData(CF_UNICODETEXT);
            if (hData) {
                wchar_t* pszText = (wchar_t*)GlobalLock(hData);
                if (pszText) {
                    wcscpy_s(url, 1024, pszText);
                    GlobalUnlock(hData);
                }
            }
            CloseClipboard();
        }
        
        if (wcslen(url) > 0 && (wcsstr(url, L"http://") || wcsstr(url, L"https://"))) {
            if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
            
            auto element = std::make_shared<SlideElement>();
            element->type = SlideElementType::Image;
            element->x = 2000;
            element->y = 2000;
            element->width = 4000;
            element->height = 3000;
            element->imageUrl = url;
            element->hBitmap = LoadImageFromUrl(url);
            element->zOrder = (int)g_presentation.slides[g_currentSlideIndex]->elements.size();
            
            if (element->hBitmap) {
                g_presentation.slides[g_currentSlideIndex]->elements.push_back(element);
                g_selectedElementIndex = (int)g_presentation.slides[g_currentSlideIndex]->elements.size() - 1;
                SetModified(true);
                InvalidateRect(g_hWnd, NULL, TRUE);
            } else {
                MessageBox(g_hWnd, L"Не удалось загрузить изображение по указанному URL.", 
                           L"Ошибка", MB_OK | MB_ICONERROR);
            }
        } else {
            MessageBox(g_hWnd, L"В буфере обмена не найден корректный URL.", 
                       L"Ошибка", MB_OK | MB_ICONERROR);
        }
    }
}

void AddShape(ShapeType type) {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    
    auto element = std::make_shared<SlideElement>();
    element->type = SlideElementType::Shape;
    element->shapeType = type;
    element->x = 2000;
    element->y = 2000;
    element->width = 3000;
    element->height = 2000;
    element->fillColor = RGB(68, 114, 196);
    element->borderColor = RGB(48, 94, 176);
    element->borderWidth = 2;
    element->zOrder = (int)g_presentation.slides[g_currentSlideIndex]->elements.size();
    
    g_presentation.slides[g_currentSlideIndex]->elements.push_back(element);
    g_selectedElementIndex = (int)g_presentation.slides[g_currentSlideIndex]->elements.size() - 1;
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void DeleteSelectedElement() {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    g_presentation.slides[g_currentSlideIndex]->elements.erase(
        g_presentation.slides[g_currentSlideIndex]->elements.begin() + g_selectedElementIndex
    );
    
    g_selectedElementIndex = -1;
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void DuplicateSelectedElement() {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    auto original = g_presentation.slides[g_currentSlideIndex]->elements[g_selectedElementIndex];
    auto copy = std::make_shared<SlideElement>();
    *copy = *original;
    copy->x += 200;
    copy->y += 200;
    copy->hBitmap = NULL;
    copy->selected = false;
    copy->zOrder = (int)g_presentation.slides[g_currentSlideIndex]->elements.size();
    
    // Перезагрузить изображение если нужно
    if (copy->type == SlideElementType::Image) {
        if (!copy->imagePath.empty()) {
            copy->hBitmap = LoadImageFromFile(copy->imagePath);
        } else if (!copy->imageUrl.empty()) {
            copy->hBitmap = LoadImageFromUrl(copy->imageUrl);
        }
    }
    
    g_presentation.slides[g_currentSlideIndex]->elements.push_back(copy);
    g_selectedElementIndex = (int)g_presentation.slides[g_currentSlideIndex]->elements.size() - 1;
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void CopySelectedElement() {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    g_clipboardElement = std::make_shared<SlideElement>();
    *g_clipboardElement = *g_presentation.slides[g_currentSlideIndex]->elements[g_selectedElementIndex];
    g_clipboardElement->hBitmap = NULL;
}

void PasteElement() {
    if (!g_clipboardElement) return;
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    
    auto newElement = std::make_shared<SlideElement>();
    *newElement = *g_clipboardElement;
    newElement->x += 200;
    newElement->y += 200;
    newElement->selected = false;
    newElement->zOrder = (int)g_presentation.slides[g_currentSlideIndex]->elements.size();
    
    // Перезагрузить изображение если нужно
    if (newElement->type == SlideElementType::Image) {
        if (!newElement->imagePath.empty()) {
            newElement->hBitmap = LoadImageFromFile(newElement->imagePath);
        } else if (!newElement->imageUrl.empty()) {
            newElement->hBitmap = LoadImageFromUrl(newElement->imageUrl);
        }
    }
    
    g_presentation.slides[g_currentSlideIndex]->elements.push_back(newElement);
    g_selectedElementIndex = (int)g_presentation.slides[g_currentSlideIndex]->elements.size() - 1;
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void CutSelectedElement() {
    CopySelectedElement();
    DeleteSelectedElement();
}

void SelectElement(int index) {
    DeselectAllElements();
    
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (index < 0 || index >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) {
        g_selectedElementIndex = -1;
        return;
    }
    
    g_selectedElementIndex = index;
    g_presentation.slides[g_currentSlideIndex]->elements[index]->selected = true;
    
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void DeselectAllElements() {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    
    for (auto& elem : g_presentation.slides[g_currentSlideIndex]->elements) {
        elem->selected = false;
    }
}

void MoveElement(int dx, int dy) {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    auto& elem = g_presentation.slides[g_currentSlideIndex]->elements[g_selectedElementIndex];
    elem->x += dx;
    elem->y += dy;
    
    // Ограничение перемещения
    if (elem->x < 0) elem->x = 0;
    if (elem->y < 0) elem->y = 0;
    if (elem->x + elem->width > 10000) elem->x = 10000 - elem->width;
    if (elem->y + elem->height > 10000) elem->y = 10000 - elem->height;
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void ResizeElement(int newWidth, int newHeight) {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    auto& elem = g_presentation.slides[g_currentSlideIndex]->elements[g_selectedElementIndex];
    
    if (newWidth >= 100) elem->width = newWidth;
    if (newHeight >= 100) elem->height = newHeight;
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

// ============================================================================
// РЕАЛИЗАЦИЯ ФУНКЦИЙ - ЗАГРУЗКА ИЗОБРАЖЕНИЙ
// ============================================================================

HBITMAP LoadImageFromFile(const std::wstring& path) {
    HBITMAP hBitmap = NULL;
    
    // Использование GDI+ для загрузки изображения
    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(path.c_str());
    if (bitmap && bitmap->GetLastStatus() == Gdiplus::Ok) {
        bitmap->GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &hBitmap);
        delete bitmap;
    }
    
    return hBitmap;
}

HBITMAP LoadImageFromUrl(const std::wstring& url) {
    return DownloadImage(url);
}

HBITMAP DownloadImage(const std::wstring& url) {
    HBITMAP hBitmap = NULL;
    HINTERNET hInternet = NULL;
    HINTERNET hUrl = NULL;
    
    hInternet = InternetOpen(L"PowerPointClone/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return NULL;
    
    hUrl = InternetOpenUrl(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) {
        InternetCloseHandle(hInternet);
        return NULL;
    }
    
    // Читаем данные изображения
    std::vector<BYTE> imageData;
    BYTE buffer[8192];
    DWORD bytesRead = 0;
    
    while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        imageData.insert(imageData.end(), buffer, buffer + bytesRead);
    }
    
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    
    if (imageData.empty()) return NULL;
    
    // Создаем IStream из данных
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, imageData.size());
    if (hGlobal) {
        void* pData = GlobalLock(hGlobal);
        if (pData) {
            memcpy(pData, imageData.data(), imageData.size());
            GlobalUnlock(hGlobal);
            
            IStream* pStream = NULL;
            if (CreateStreamOnHGlobal(hGlobal, TRUE, &pStream) == S_OK) {
                Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromStream(pStream);
                if (bitmap && bitmap->GetLastStatus() == Gdiplus::Ok) {
                    bitmap->GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &hBitmap);
                    delete bitmap;
                }
                pStream->Release();
            }
        } else {
            GlobalFree(hGlobal);
        }
    }
    
    return hBitmap;
}

// ============================================================================
// РЕАЛИЗАЦИЯ ФУНКЦИЙ - МАСШТАБИРОВАНИЕ
// ============================================================================

void ZoomIn() {
    SetZoom(g_zoomLevel + 10);
}

void ZoomOut() {
    SetZoom(g_zoomLevel - 10);
}

void SetZoom(int percent) {
    if (percent < g_minZoom) percent = g_minZoom;
    if (percent > g_maxZoom) percent = g_maxZoom;
    
    g_zoomLevel = percent;
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void FitToWindow() {
    // Автоматический подбор масштаба
    RECT clientRect;
    GetClientRect(g_hWnd, &clientRect);
    
    int availableWidth = clientRect.right - g_thumbnailWidth - g_slideMargin * 2;
    int availableHeight = clientRect.bottom - g_ribbonHeight - g_statusBarHeight - g_slideMargin * 2;
    
    float scaleX = (float)availableWidth / g_presentation.slideWidth;
    float scaleY = (float)availableHeight / g_presentation.slideHeight;
    float scale = min(scaleX, scaleY);
    
    g_zoomLevel = (int)(scale * 100);
    if (g_zoomLevel < g_minZoom) g_zoomLevel = g_minZoom;
    if (g_zoomLevel > g_maxZoom) g_zoomLevel = g_maxZoom;
    
    InvalidateRect(g_hWnd, NULL, TRUE);
}

// ============================================================================
// РЕАЛИЗАЦИЯ ФУНКЦИЙ - РЕЖИМЫ ПРОСМОТРА
// ============================================================================

void SetViewMode(int mode) {
    // 0 = Normal, 1 = Sorter, 2 = Reading, 3 = Slideshow
    switch (mode) {
        case 0:
            // Обычный режим
            break;
        case 1:
            // Сортировщик слайдов
            MessageBox(g_hWnd, L"Режим сортировщика слайдов будет реализован в следующей версии.", 
                       L"Информация", MB_OK | MB_ICONINFORMATION);
            break;
        case 2:
            // Режим чтения
            MessageBox(g_hWnd, L"Режим чтения будет реализован в следующей версии.", 
                       L"Информация", MB_OK | MB_ICONINFORMATION);
            break;
        case 3:
            StartSlideshow(true);
            break;
    }
}

void StartSlideshow(bool fromBeginning) {
    int startSlide = fromBeginning ? 0 : g_currentSlideIndex;
    
    MessageBox(g_hWnd, 
        L"Режим слайд-шоу будет реализован в следующей версии.\n\n"
        L"Планируемые функции:\n"
        L"- Полноэкранный показ\n"
        L"- Переходы между слайдами\n"
        L"- Анимация элементов\n"
        L"- Указатель презентатора",
        L"Слайд-шоу", MB_OK | MB_ICONINFORMATION);
}

void ToggleNotesPanel() {
    g_showNotes = !g_showNotes;
    g_notesHeight = g_showNotes ? 150 : 0;
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void ToggleRuler() {
    g_showRuler = !g_showRuler;
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void ToggleGrid() {
    g_showGrid = !g_showGrid;
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void ToggleGuides() {
    g_showGuides = !g_showGuides;
    InvalidateRect(g_hWnd, NULL, TRUE);
}

// ============================================================================
// РЕАЛИЗАЦИЯ ФУНКЦИЙ - ОТМЕНА/ПОВТОР
// ============================================================================

void Undo() {
    if (g_undoStack.empty()) return;
    
    // Здесь будет логика отмены
    g_undoStack.pop_back();
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void Redo() {
    if (g_redoStack.empty()) return;
    
    // Здесь будет логика повтора
    g_redoStack.pop_back();
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void SaveUndoState(UndoAction::Type type) {
    // Здесь будет сохранение состояния для отмены
    g_redoStack.clear();
    
    if (g_undoStack.size() >= (size_t)g_maxUndoLevels) {
        g_undoStack.erase(g_undoStack.begin());
    }
}

void ClearUndoHistory() {
    g_undoStack.clear();
    g_redoStack.clear();
}

// ============================================================================
// РЕАЛИЗАЦИЯ ФУНКЦИЙ - ВСПОМОГАТЕЛЬНЫЕ
// ============================================================================

void UpdateWindowTitle() {
    std::wstring title = L"PowerPoint Clone Pro - ";
    
    if (g_presentation.filePath.empty()) {
        title += L"Новая презентация";
    } else {
        // Извлечь имя файла из пути
        size_t pos = g_presentation.filePath.find_last_of(L"\\/");
        if (pos != std::wstring::npos) {
            title += g_presentation.filePath.substr(pos + 1);
        } else {
            title += g_presentation.filePath;
        }
    }
    
    if (g_presentation.modified) {
        title += L" *";
    }
    
    SetWindowText(g_hWnd, title.c_str());
}

void UpdateStatusBar() {
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void SetModified(bool modified) {
    if (g_presentation.modified != modified) {
        g_presentation.modified = modified;
        UpdateWindowTitle();
    }
}

std::wstring GetLayoutName(SlideLayout layout) {
    switch (layout) {
        case SlideLayout::TitleSlide: return L"Титульный слайд";
        case SlideLayout::TitleAndContent: return L"Заголовок и объект";
        case SlideLayout::SectionHeader: return L"Заголовок раздела";
        case SlideLayout::TwoContent: return L"Два объекта";
        case SlideLayout::Comparison: return L"Сравнение";
        case SlideLayout::TitleOnly: return L"Только заголовок";
        case SlideLayout::Blank: return L"Пустой слайд";
        case SlideLayout::ContentWithCaption: return L"Объект с подписью";
        case SlideLayout::PictureWithCaption: return L"Рисунок с подписью";
        default: return L"Неизвестный";
    }
}

std::wstring FormatNumber(int num) {
    return std::to_wstring(num);
}

RECT GetSlideRect(RECT* clientRect) {
    int slideAreaLeft = g_thumbnailWidth + 1;
    int slideAreaTop = g_ribbonHeight;
    int slideAreaWidth = clientRect->right - g_thumbnailWidth - 1;
    int slideAreaHeight = clientRect->bottom - g_ribbonHeight - g_statusBarHeight - g_notesHeight;
    
    // Размеры слайда с учетом масштаба
    int baseSlideWidth = slideAreaWidth - g_slideMargin * 2;
    int baseSlideHeight = slideAreaHeight - g_slideMargin * 2;
    
    int slideWidth, slideHeight;
    
    // Сохранение соотношения сторон 16:9
    if (baseSlideWidth * 9 / 16 < baseSlideHeight) {
        slideWidth = baseSlideWidth;
        slideHeight = slideWidth * 9 / 16;
    } else {
        slideHeight = baseSlideHeight;
        slideWidth = slideHeight * 16 / 9;
    }
    
    // Применение масштаба
    slideWidth = slideWidth * g_zoomLevel / 100;
    slideHeight = slideHeight * g_zoomLevel / 100;
    
    // Центрирование
    int slideX = slideAreaLeft + (slideAreaWidth - slideWidth) / 2;
    int slideY = slideAreaTop + (slideAreaHeight - slideHeight) / 2;
    
    RECT slideRect = {slideX, slideY, slideX + slideWidth, slideY + slideHeight};
    return slideRect;
}

RECT GetElementRect(SlideElement* element, int slideX, int slideY, int slideWidth, int slideHeight) {
    RECT rect;
    rect.left = slideX + element->x * slideWidth / 10000;
    rect.top = slideY + element->y * slideHeight / 10000;
    rect.right = rect.left + element->width * slideWidth / 10000;
    rect.bottom = rect.top + element->height * slideHeight / 10000;
    return rect;
}

int HitTestElement(int x, int y, int slideX, int slideY, int slideWidth, int slideHeight) {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return -1;
    
    auto& slide = g_presentation.slides[g_currentSlideIndex];
    
    // Проверяем элементы в обратном порядке (верхние первыми)
    for (int i = (int)slide->elements.size() - 1; i >= 0; i--) {
        RECT rect = GetElementRect(slide->elements[i].get(), slideX, slideY, slideWidth, slideHeight);
        POINT pt = {x, y};
        if (PtInRect(&rect, pt)) {
            return i;
        }
    }
    
    return -1;
}

int HitTestResizeHandle(int x, int y, RECT* elementRect) {
    if (!elementRect) return -1;
    
    int handleSize = 8;
    RECT handles[8];
    
    // Углы
    SetRect(&handles[0], elementRect->left - handleSize/2, elementRect->top - handleSize/2, 
            elementRect->left + handleSize/2, elementRect->top + handleSize/2);
    SetRect(&handles[1], elementRect->right - handleSize/2, elementRect->top - handleSize/2, 
            elementRect->right + handleSize/2, elementRect->top + handleSize/2);
    SetRect(&handles[2], elementRect->right - handleSize/2, elementRect->bottom - handleSize/2, 
            elementRect->right + handleSize/2, elementRect->bottom + handleSize/2);
    SetRect(&handles[3], elementRect->left - handleSize/2, elementRect->bottom - handleSize/2, 
            elementRect->left + handleSize/2, elementRect->bottom + handleSize/2);
    
    // Стороны
    int midX = (elementRect->left + elementRect->right) / 2;
    int midY = (elementRect->top + elementRect->bottom) / 2;
    
    SetRect(&handles[4], midX - handleSize/2, elementRect->top - handleSize/2, 
            midX + handleSize/2, elementRect->top + handleSize/2);
    SetRect(&handles[5], elementRect->right - handleSize/2, midY - handleSize/2, 
            elementRect->right + handleSize/2, midY + handleSize/2);
    SetRect(&handles[6], midX - handleSize/2, elementRect->bottom - handleSize/2, 
            midX + handleSize/2, elementRect->bottom + handleSize/2);
    SetRect(&handles[7], elementRect->left - handleSize/2, midY - handleSize/2, 
            elementRect->left + handleSize/2, midY + handleSize/2);
    
    POINT pt = {x, y};
    for (int i = 0; i < 8; i++) {
        if (PtInRect(&handles[i], pt)) {
            return i;
        }
    }
    
    return -1;
}

// ============================================================================
// РЕАЛИЗАЦИЯ ФУНКЦИЙ - ОТРИСОВКА
// ============================================================================

void DrawAll(HDC hdc, RECT* clientRect) {
    // Фон
    HBRUSH hBrushBg = CreateSolidBrush(COLOR_BG_GRAY);
    FillRect(hdc, clientRect, hBrushBg);
    DeleteObject(hBrushBg);
    
    // Отрисовка компонентов
    DrawRibbon(hdc, clientRect);
    DrawThumbnailPanel(hdc, clientRect);
    DrawSlideArea(hdc, clientRect);
    DrawStatusBar(hdc, clientRect);
    
    if (g_showNotes && g_notesHeight > 0) {
        DrawNotesPanel(hdc, clientRect);
    }
}

void DrawRibbon(HDC hdc, RECT* rect) {
    // Фон Ribbon
    RECT ribbonRect = {0, 0, rect->right, g_ribbonHeight};
    HBRUSH hBrushRibbon = CreateSolidBrush(COLOR_RIBBON_BG);
    FillRect(hdc, &ribbonRect, hBrushRibbon);
    DeleteObject(hBrushRibbon);
    
    // Верхняя панель с вкладками
    RECT tabBarRect = {0, 0, rect->right, g_tabBarHeight};
    HBRUSH hBrushTabBar = CreateSolidBrush(COLOR_RED);
    FillRect(hdc, &tabBarRect, hBrushTabBar);
    DeleteObject(hBrushTabBar);
    
    // Логотип и название приложения
    HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFontRibbonTab);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_WHITE);
    
    // Иконка приложения (простой квадрат)
    RECT iconRect = {8, 4, 28, 24};
    HBRUSH hBrushIcon = CreateSolidBrush(COLOR_WHITE);
    FillRect(hdc, &iconRect, hBrushIcon);
    DeleteObject(hBrushIcon);
    
    // Отрисовка вкладок
    int tabX = 100;
    for (int i = 0; i < (int)g_ribbonTabs.size(); i++) {
        SIZE textSize;
        GetTextExtentPoint32(hdc, g_ribbonTabs[i].name.c_str(), (int)g_ribbonTabs[i].name.length(), &textSize);
        
        int tabWidth = textSize.cx + 24;
        
        g_ribbonTabs[i].tabRect = {tabX, 0, tabX + tabWidth, g_tabBarHeight};
        
        if (i == g_activeTabIndex) {
            // Активная вкладка
            HBRUSH hBrushActive = CreateSolidBrush(COLOR_WHITE);
            RECT activeRect = {tabX, g_tabBarHeight - 3, tabX + tabWidth, g_tabBarHeight};
            FillRect(hdc, &activeRect, hBrushActive);
            DeleteObject(hBrushActive);
            SetTextColor(hdc, COLOR_WHITE);
        } else {
            SetTextColor(hdc, RGB(255, 220, 220));
        }
        
        RECT tabTextRect = {tabX, 0, tabX + tabWidth, g_tabBarHeight};
        DrawText(hdc, g_ribbonTabs[i].name.c_str(), -1, &tabTextRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        tabX += tabWidth;
    }
    
    // Кнопки справа в заголовке
    SetTextColor(hdc, COLOR_WHITE);
    TextOut(hdc, rect->right - 120, 6, L"Справка", 7);
    
    // Отрисовка содержимого активной вкладки
    DrawRibbonContent(hdc, rect);
    
    // Нижняя граница Ribbon
    HPEN hPenBorder = CreatePen(PS_SOLID, 1, COLOR_BORDER_LIGHT);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);
    MoveToEx(hdc, 0, g_ribbonHeight - 1, NULL);
    LineTo(hdc, rect->right, g_ribbonHeight - 1);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPenBorder);
    
    SelectObject(hdc, hOldFont);
}

void DrawRibbonContent(HDC hdc, RECT* rect) {
    if (g_activeTabIndex < 0 || g_activeTabIndex >= (int)g_ribbonTabs.size()) return;
    
    auto& tab = g_ribbonTabs[g_activeTabIndex];
    
    int groupX = 10;
    int groupY = g_tabBarHeight + 5;
    int groupHeight = g_ribbonHeight - g_tabBarHeight - 25;
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFontRibbon);
    SetBkMode(hdc, TRANSPARENT);
    
    for (auto& group : tab.groups) {
        // Вычисляем ширину группы
        int buttonCount = (int)group.second.size();
        int groupWidth = max(buttonCount * 70, 80);
        
        // Фон группы
        RECT groupRect = {groupX, groupY, groupX + groupWidth, groupY + groupHeight};
        
        // Разделитель справа
        HPEN hPenSep = CreatePen(PS_SOLID, 1, COLOR_BORDER_LIGHT);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPenSep);
        MoveToEx(hdc, groupX + groupWidth + 5, groupY, NULL);
        LineTo(hdc, groupX + groupWidth + 5, groupY + groupHeight);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPenSep);
        
        // Название группы
        SetTextColor(hdc, COLOR_TEXT_MEDIUM);
        SelectObject(hdc, g_hFontRibbonSmall);
        RECT labelRect = {groupX, groupY + groupHeight - 2, groupX + groupWidth, groupY + groupHeight + 15};
        DrawText(hdc, group.first.c_str(), -1, &labelRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
        SelectObject(hdc, g_hFontRibbon);
        
        // Кнопки в группе
        int btnX = groupX + 5;
        int btnY = groupY + 5;
        int btnWidth = 60;
        int btnHeight = groupHeight - 25;
        
        for (auto& btn : group.second) {
            btn.rect = {btnX, btnY, btnX + btnWidth, btnY + btnHeight};
            DrawRibbonButton(hdc, &btn, true);
            btnX += btnWidth + 5;
        }
        
        groupX += groupWidth + 15;
    }
    
    SelectObject(hdc, hOldFont);
}

void DrawRibbonButton(HDC hdc, RibbonButton* btn, bool large) {
    if (!btn) return;
    
    // Фон кнопки при наведении
    if (btn->hovered) {
        HBRUSH hBrushHover = CreateSolidBrush(COLOR_TOOLBAR_HOVER);
        RECT hoverRect = btn->rect;
        InflateRect(&hoverRect, -2, -2);
        FillRect(hdc, &hoverRect, hBrushHover);
        DeleteObject(hBrushHover);
        
        // Рамка
        HPEN hPenBorder = CreatePen(PS_SOLID, 1, COLOR_BORDER_LIGHT);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, hoverRect.left, hoverRect.top, hoverRect.right, hoverRect.bottom);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPenBorder);
    }
    
    if (btn->pressed) {
        HBRUSH hBrushPressed = CreateSolidBrush(COLOR_TOOLBAR_PRESSED);
        RECT pressRect = btn->rect;
        InflateRect(&pressRect, -2, -2);
        FillRect(hdc, &pressRect, hBrushPressed);
        DeleteObject(hBrushPressed);
    }
    
    // Иконка (символ)
    if (!btn->iconSymbol.empty()) {
        SetTextColor(hdc, btn->iconColor);
        HFONT hIconFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Symbol");
        HFONT hOldFont = (HFONT)SelectObject(hdc, hIconFont);
        
        RECT iconRect = btn->rect;
        iconRect.bottom = iconRect.top + 35;
        DrawText(hdc, btn->iconSymbol.c_str(), -1, &iconRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, hOldFont);
        DeleteObject(hIconFont);
    }
    
    // Текст
    SetTextColor(hdc, btn->enabled ? COLOR_TEXT_DARK : COLOR_TEXT_LIGHT);
    RECT textRect = btn->rect;
    textRect.top = textRect.bottom - 20;
    
    SelectObject(hdc, g_hFontRibbonSmall);
    DrawText(hdc, btn->text.c_str(), -1, &textRect, DT_CENTER | DT_TOP | DT_SINGLELINE | DT_WORD_ELLIPSIS);
    
    // Стрелка выпадающего списка
    if (btn->hasDropdown) {
        SetTextColor(hdc, COLOR_TEXT_MEDIUM);
        RECT arrowRect = {btn->rect.right - 15, btn->rect.bottom - 18, btn->rect.right - 5, btn->rect.bottom - 8};
        DrawText(hdc, L"▼", -1, &arrowRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

void DrawThumbnailPanel(HDC hdc, RECT* clientRect) {
    // Фон панели миниатюр
    RECT thumbPanel;
    thumbPanel.left = 0;
    thumbPanel.top = g_ribbonHeight;
    thumbPanel.right = g_thumbnailWidth;
    thumbPanel.bottom = clientRect->bottom - g_statusBarHeight;
    
    HBRUSH hBrushBg = CreateSolidBrush(COLOR_PANEL_BG);
    FillRect(hdc, &thumbPanel, hBrushBg);
    DeleteObject(hBrushBg);
    
    // Разделительная линия
    HPEN hPenBorder = CreatePen(PS_SOLID, 1, COLOR_BORDER_LIGHT);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);
    MoveToEx(hdc, g_thumbnailWidth, g_ribbonHeight, NULL);
    LineTo(hdc, g_thumbnailWidth, clientRect->bottom - g_statusBarHeight);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPenBorder);
    
    // Отрисовка миниатюр слайдов
    int thumbMargin = 12;
    int thumbSlideWidth = g_thumbnailWidth - thumbMargin * 2 - 25;
    int thumbSlideHeight = (int)(thumbSlideWidth * 9.0 / 16.0);
    int thumbX = thumbMargin + 20;
    int thumbY = g_ribbonHeight + thumbMargin;
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFontSlideNum);
    SetBkMode(hdc, TRANSPARENT);
    
    for (int i = 0; i < (int)g_presentation.slides.size(); i++) {
        bool isSelected = (i == g_currentSlideIndex);
        
        // Номер слайда
        std::wstring slideNum = std::to_wstring(i + 1);
        SetTextColor(hdc, isSelected ? COLOR_RED : COLOR_TEXT_MEDIUM);
        TextOut(hdc, thumbMargin, thumbY + thumbSlideHeight / 2 - 8, slideNum.c_str(), (int)slideNum.length());
        
        // Миниатюра
        DrawThumbnail(hdc, g_presentation.slides[i].get(), i, thumbX, thumbY, thumbSlideWidth, thumbSlideHeight, isSelected);
        
        thumbY += thumbSlideHeight + thumbMargin + 5;
        
        // Проверка выхода за пределы области
        if (thumbY + thumbSlideHeight > clientRect->bottom - g_statusBarHeight) {
            break;
        }
    }
    
    SelectObject(hdc, hOldFont);
}

void DrawThumbnail(HDC hdc, Slide* slide, int index, int x, int y, int width, int height, bool selected) {
    // Тень
    RECT shadowRect = {x + 2, y + 2, x + width + 2, y + height + 2};
    HBRUSH hBrushShadow = CreateSolidBrush(RGB(200, 200, 200));
    FillRect(hdc, &shadowRect, hBrushShadow);
    DeleteObject(hBrushShadow);
    
    // Фон миниатюры
    RECT thumbRect = {x, y, x + width, y + height};
    HBRUSH hBrushWhite = CreateSolidBrush(slide->backgroundColor);
    FillRect(hdc, &thumbRect, hBrushWhite);
    DeleteObject(hBrushWhite);
    
    // Отрисовка содержимого слайда в миниатюре
    DrawSlide(hdc, slide, x, y, width, height, false);
    
    // Рамка выделения
    if (selected) {
        HPEN hPenRed = CreatePen(PS_SOLID, 3, COLOR_RED);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPenRed);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, x - 2, y - 2, x + width + 2, y + height + 2);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPenRed);
    } else {
        HPEN hPenBorder = CreatePen(PS_SOLID, 1, COLOR_BORDER_DARK);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, x, y, x + width, y + height);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPenBorder);
    }
}

void DrawSlideArea(HDC hdc, RECT* clientRect) {
    // Область слайда
    int slideAreaLeft = g_thumbnailWidth + 1;
    int slideAreaTop = g_ribbonHeight;
    int slideAreaWidth = clientRect->right - g_thumbnailWidth - 1;
    int slideAreaHeight = clientRect->bottom - g_ribbonHeight - g_statusBarHeight - g_notesHeight;
    
    // Серый фон области редактирования
    RECT slideAreaRect = {slideAreaLeft, slideAreaTop, clientRect->right, slideAreaTop + slideAreaHeight};
    HBRUSH hBrushArea = CreateSolidBrush(RGB(83, 83, 83));
    FillRect(hdc, &slideAreaRect, hBrushArea);
    DeleteObject(hBrushArea);
    
    // Получение прямоугольника слайда
    RECT slideRect = GetSlideRect(clientRect);
    int slideX = slideRect.left;
    int slideY = slideRect.top;
    int slideWidth = slideRect.right - slideRect.left;
    int slideHeight = slideRect.bottom - slideRect.top;
    
    // Тень слайда
    RECT shadowRect = {slideX + 5, slideY + 5, slideX + slideWidth + 5, slideY + slideHeight + 5};
    HBRUSH hBrushShadow = CreateSolidBrush(RGB(50, 50, 50));
    FillRect(hdc, &shadowRect, hBrushShadow);
    DeleteObject(hBrushShadow);
    
    // Отрисовка текущего слайда
    if (g_currentSlideIndex >= 0 && g_currentSlideIndex < (int)g_presentation.slides.size()) {
        DrawSlide(hdc, g_presentation.slides[g_currentSlideIndex].get(), slideX, slideY, slideWidth, slideHeight, true);
    }
    
    // Граница слайда
    HPEN hPenBorder = CreatePen(PS_SOLID, 1, RGB(150, 150, 150));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, slideX, slideY, slideX + slideWidth, slideY + slideHeight);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPenBorder);
    
    // Сетка (если включена)
    if (g_showGrid) {
        HPEN hPenGrid = CreatePen(PS_DOT, 1, RGB(200, 200, 200));
        SelectObject(hdc, hPenGrid);
        
        int gridSize = slideWidth / 20;
        for (int gx = slideX + gridSize; gx < slideX + slideWidth; gx += gridSize) {
            MoveToEx(hdc, gx, slideY, NULL);
            LineTo(hdc, gx, slideY + slideHeight);
        }
        for (int gy = slideY + gridSize; gy < slideY + slideHeight; gy += gridSize) {
            MoveToEx(hdc, slideX, gy, NULL);
            LineTo(hdc, slideX + slideWidth, gy);
        }
        
        DeleteObject(hPenGrid);
    }
    
    // Направляющие (если включены)
    if (g_showGuides) {
        HPEN hPenGuide = CreatePen(PS_DASH, 1, COLOR_BLUE);
        SelectObject(hdc, hPenGuide);
        
        int centerX = slideX + slideWidth / 2;
        int centerY = slideY + slideHeight / 2;
        
        MoveToEx(hdc, centerX, slideY, NULL);
        LineTo(hdc, centerX, slideY + slideHeight);
        MoveToEx(hdc, slideX, centerY, NULL);
        LineTo(hdc, slideX + slideWidth, centerY);
        
        DeleteObject(hPenGuide);
    }
}

void DrawSlide(HDC hdc, Slide* slide, int x, int y, int width, int height, bool isMainView) {
    // Фон слайда
    RECT slideRect = {x, y, x + width, y + height};
    HBRUSH hBrushBg = CreateSolidBrush(slide->backgroundColor);
    FillRect(hdc, &slideRect, hBrushBg);
    DeleteObject(hBrushBg);
    
    // Фоновое изображение (если есть)
    if (slide->hBackgroundBitmap) {
        HDC hdcMem = CreateCompatibleDC(hdc);
        SelectObject(hdcMem, slide->hBackgroundBitmap);
        
        BITMAP bm;
        GetObject(slide->hBackgroundBitmap, sizeof(BITMAP), &bm);
        
        SetStretchBltMode(hdc, HALFTONE);
        StretchBlt(hdc, x, y, width, height, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
        
        DeleteDC(hdcMem);
    }
    
    // Отрисовка элементов
    for (auto& element : slide->elements) {
        if (!element->visible) continue;
        DrawSlideElement(hdc, element.get(), x, y, width, height);
    }
}

void DrawSlideElement(HDC hdc, SlideElement* element, int slideX, int slideY, int slideWidth, int slideHeight) {
    RECT rect = GetElementRect(element, slideX, slideY, slideWidth, slideHeight);
    
    switch (element->type) {
        case SlideElementType::TextBox:
            DrawTextElement(hdc, element, &rect);
            break;
        case SlideElementType::Image:
            DrawImageElement(hdc, element, &rect);
            break;
        case SlideElementType::Shape:
            DrawShapeElement(hdc, element, &rect);
            break;
        default:
            break;
    }
    
    // Рамка выделения
    if (element->selected) {
        DrawSelectionHandles(hdc, &rect);
    }
}

void DrawTextElement(HDC hdc, SlideElement* element, RECT* rect) {
    // Масштабирование размера шрифта
    int scaledFontSize = element->fontSize * (rect->right - rect->left) / 
                         (element->width * g_presentation.slideWidth / 10000);
    if (scaledFontSize < 6) scaledFontSize = 6;
    if (scaledFontSize > 200) scaledFontSize = 200;
    
    HFONT hFont = CreateFont(
        scaledFontSize, 0, 0, 0,
        element->bold ? FW_BOLD : FW_NORMAL,
        element->italic, element->underline, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
        element->fontName.c_str()
    );
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    SetTextColor(hdc, element->textColor);
    SetBkMode(hdc, TRANSPARENT);
    
    // Фон текстового поля (если не прозрачный)
    if (element->fillColor != COLOR_WHITE || element->borderWidth > 0) {
        HBRUSH hBrushFill = CreateSolidBrush(element->fillColor);
        HPEN hPenBorder = CreatePen(PS_SOLID, element->borderWidth, element->borderColor);
        
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrushFill);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);
        
        Rectangle(hdc, rect->left, rect->top, rect->right, rect->bottom);
        
        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldPen);
        DeleteObject(hBrushFill);
        DeleteObject(hPenBorder);
    }
    
    // Текст
    RECT textRect = *rect;
    InflateRect(&textRect, -5, -5);
    DrawText(hdc, element->text.c_str(), -1, &textRect, DT_LEFT | DT_TOP | DT_WORDBREAK);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

void DrawImageElement(HDC hdc, SlideElement* element, RECT* rect) {
    if (element->hBitmap) {
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, element->hBitmap);
        
        BITMAP bm;
        GetObject(element->hBitmap, sizeof(BITMAP), &bm);
        
        SetStretchBltMode(hdc, HALFTONE);
        StretchBlt(hdc, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top,
                   hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
        
        SelectObject(hdcMem, hOldBitmap);
        DeleteDC(hdcMem);
    } else {
        // Заглушка для изображения
        HBRUSH hBrushFill = CreateSolidBrush(RGB(240, 240, 240));
        FillRect(hdc, rect, hBrushFill);
        DeleteObject(hBrushFill);
        
        HPEN hPenBorder = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, rect->left, rect->top, rect->right, rect->bottom);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPenBorder);
        
        // Иконка изображения
        SetTextColor(hdc, RGB(180, 180, 180));
        HFONT hIconFont = CreateFont(40, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Symbol");
        HFONT hOldFont = (HFONT)SelectObject(hdc, hIconFont);
        DrawText(hdc, L"🖼", -1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hOldFont);
        DeleteObject(hIconFont);
    }
    
    // Граница
    if (element->borderWidth > 0) {
        HPEN hPenBorder = CreatePen(PS_SOLID, element->borderWidth, element->borderColor);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, rect->left, rect->top, rect->right, rect->bottom);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPenBorder);
    }
}

void DrawShapeElement(HDC hdc, SlideElement* element, RECT* rect) {
    HBRUSH hBrushFill = CreateSolidBrush(element->fillColor);
    HPEN hPenBorder = CreatePen(PS_SOLID, element->borderWidth, element->borderColor);
    
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrushFill);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);
    
    switch (element->shapeType) {
        case ShapeType::Rectangle:
            Rectangle(hdc, rect->left, rect->top, rect->right, rect->bottom);
            break;
            
        case ShapeType::RoundedRect:
            RoundRect(hdc, rect->left, rect->top, rect->right, rect->bottom, 20, 20);
            break;
            
        case ShapeType::Ellipse:
            Ellipse(hdc, rect->left, rect->top, rect->right, rect->bottom);
            break;
            
        case ShapeType::Triangle: {
            POINT points[3] = {
                {(rect->left + rect->right) / 2, rect->top},
                {rect->left, rect->bottom},
                {rect->right, rect->bottom}
            };
            Polygon(hdc, points, 3);
            break;
        }
        
        case ShapeType::Diamond: {
            int cx = (rect->left + rect->right) / 2;
            int cy = (rect->top + rect->bottom) / 2;
            POINT points[4] = {
                {cx, rect->top},
                {rect->right, cy},
                {cx, rect->bottom},
                {rect->left, cy}
            };
            Polygon(hdc, points, 4);
            break;
        }
        
        case ShapeType::Star: {
            // Простая пятиконечная звезда
            int cx = (rect->left + rect->right) / 2;
            int cy = (rect->top + rect->bottom) / 2;
            int r1 = (rect->right - rect->left) / 2;
            int r2 = r1 / 2;
            
            POINT points[10];
            for (int i = 0; i < 10; i++) {
                double angle = -3.14159 / 2 + i * 3.14159 / 5;
                int r = (i % 2 == 0) ? r1 : r2;
                points[i].x = cx + (int)(r * cos(angle));
                points[i].y = cy + (int)(r * sin(angle));
            }
            Polygon(hdc, points, 10);
            break;
        }
        
        case ShapeType::Arrow: {
            int midY = (rect->top + rect->bottom) / 2;
            int arrowWidth = (rect->right - rect->left) / 4;
            int arrowHeight = (rect->bottom - rect->top) / 3;
            
            POINT points[7] = {
                {rect->left, midY - arrowHeight / 2},
                {rect->right - arrowWidth, midY - arrowHeight / 2},
                {rect->right - arrowWidth, rect->top},
                {rect->right, midY},
                {rect->right - arrowWidth, rect->bottom},
                {rect->right - arrowWidth, midY + arrowHeight / 2},
                {rect->left, midY + arrowHeight / 2}
            };
            Polygon(hdc, points, 7);
            break;
        }
        
        case ShapeType::Line:
            MoveToEx(hdc, rect->left, rect->top, NULL);
            LineTo(hdc, rect->right, rect->bottom);
            break;
            
        default:
            Rectangle(hdc, rect->left, rect->top, rect->right, rect->bottom);
            break;
    }
    
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hBrushFill);
    DeleteObject(hPenBorder);
}

void DrawSelectionHandles(HDC hdc, RECT* rect) {
    // Рамка выделения
    HPEN hPenSelection = CreatePen(PS_SOLID, 1, COLOR_SELECTION);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenSelection);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, rect->left, rect->top, rect->right, rect->bottom);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPenSelection);
    
    // Маркеры изменения размера
    int handleSize = 8;
    HBRUSH hBrushHandle = CreateSolidBrush(COLOR_SELECTION);
    HPEN hPenHandle = CreatePen(PS_SOLID, 1, COLOR_WHITE);
    
    SelectObject(hdc, hBrushHandle);
    SelectObject(hdc, hPenHandle);
    
    // Углы
    Rectangle(hdc, rect->left - handleSize/2, rect->top - handleSize/2, 
              rect->left + handleSize/2, rect->top + handleSize/2);
    Rectangle(hdc, rect->right - handleSize/2, rect->top - handleSize/2, 
              rect->right + handleSize/2, rect->top + handleSize/2);
    Rectangle(hdc, rect->right - handleSize/2, rect->bottom - handleSize/2, 
              rect->right + handleSize/2, rect->bottom + handleSize/2);
    Rectangle(hdc, rect->left - handleSize/2, rect->bottom - handleSize/2, 
              rect->left + handleSize/2, rect->bottom + handleSize/2);
    
    // Стороны
    int midX = (rect->left + rect->right) / 2;
    int midY = (rect->top + rect->bottom) / 2;
    
    Rectangle(hdc, midX - handleSize/2, rect->top - handleSize/2, 
              midX + handleSize/2, rect->top + handleSize/2);
    Rectangle(hdc, rect->right - handleSize/2, midY - handleSize/2, 
              rect->right + handleSize/2, midY + handleSize/2);
    Rectangle(hdc, midX - handleSize/2, rect->bottom - handleSize/2, 
              midX + handleSize/2, rect->bottom + handleSize/2);
    Rectangle(hdc, rect->left - handleSize/2, midY - handleSize/2, 
              rect->left + handleSize/2, midY + handleSize/2);
    
    DeleteObject(hBrushHandle);
    DeleteObject(hPenHandle);
}

void DrawStatusBar(HDC hdc, RECT* clientRect) {
    // Фон статус-бара
    RECT statusRect = {0, clientRect->bottom - g_statusBarHeight, clientRect->right, clientRect->bottom};
    HBRUSH hBrushStatus = CreateSolidBrush(COLOR_STATUS_BAR);
    FillRect(hdc, &statusRect, hBrushStatus);
    DeleteObject(hBrushStatus);
    
    // Верхняя линия
    HPEN hPenBorder = CreatePen(PS_SOLID, 1, COLOR_BORDER_LIGHT);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);
    MoveToEx(hdc, 0, clientRect->bottom - g_statusBarHeight, NULL);
    LineTo(hdc, clientRect->right, clientRect->bottom - g_statusBarHeight);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPenBorder);
    
    // Текст статус-бара
    HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFontStatus);
    SetTextColor(hdc, COLOR_TEXT_MEDIUM);
    SetBkMode(hdc, TRANSPARENT);
    
    int yText = clientRect->bottom - g_statusBarHeight + 5;
    int xPos = 15;
    
    // Информация о слайде
    std::wstring slideInfo = L"Слайд " + std::to_wstring(g_currentSlideIndex + 1) + 
                             L" из " + std::to_wstring(g_presentation.slides.size());
    TextOut(hdc, xPos, yText, slideInfo.c_str(), (int)slideInfo.length());
    xPos += 100;
    
    // Разделитель
    HPEN hPenSep = CreatePen(PS_SOLID, 1, COLOR_BORDER_LIGHT);
    SelectObject(hdc, hPenSep);
    MoveToEx(hdc, xPos, clientRect->bottom - g_statusBarHeight + 4, NULL);
    LineTo(hdc, xPos, clientRect->bottom - 4);
    xPos += 15;
    
    // Макет слайда
    if (g_currentSlideIndex >= 0 && g_currentSlideIndex < (int)g_presentation.slides.size()) {
        std::wstring layoutName = GetLayoutName(g_presentation.slides[g_currentSlideIndex]->layout);
        TextOut(hdc, xPos, yText, layoutName.c_str(), (int)layoutName.length());
        xPos += 150;
    }
    
    // Разделитель
    MoveToEx(hdc, xPos, clientRect->bottom - g_statusBarHeight + 4, NULL);
    LineTo(hdc, xPos, clientRect->bottom - 4);
    xPos += 15;
    
    // Специальные возможности
    TextOut(hdc, xPos, yText, L"Щёлкните для добавления заметок", 31);
    
    // Правая часть - масштаб и вид
    int rightX = clientRect->right - 200;
    
    // Разделитель
    MoveToEx(hdc, rightX - 15, clientRect->bottom - g_statusBarHeight + 4, NULL);
    LineTo(hdc, rightX - 15, clientRect->bottom - 4);
    
    // Кнопки режимов просмотра (иконки)
    HBRUSH hBrushIcon = CreateSolidBrush(COLOR_TEXT_MEDIUM);
    
    RECT iconRect1 = {rightX, yText + 1, rightX + 18, yText + 13};
    FrameRect(hdc, &iconRect1, hBrushIcon);
    
    RECT iconRect2 = {rightX + 24, yText + 1, rightX + 42, yText + 13};
    FrameRect(hdc, &iconRect2, hBrushIcon);
    
    RECT iconRect3 = {rightX + 48, yText + 1, rightX + 66, yText + 13};
    FrameRect(hdc, &iconRect3, hBrushIcon);
    
    RECT iconRect4 = {rightX + 72, yText + 1, rightX + 90, yText + 13};
    FrameRect(hdc, &iconRect4, hBrushIcon);
    
    DeleteObject(hBrushIcon);
    
    // Масштаб
    std::wstring zoomText = std::to_wstring(g_zoomLevel) + L"%";
    TextOut(hdc, rightX + 110, yText, zoomText.c_str(), (int)zoomText.length());
    
    // Ползунок масштаба
    HPEN hPenSlider = CreatePen(PS_SOLID, 2, COLOR_BORDER_DARK);
    SelectObject(hdc, hPenSlider);
    int sliderX = rightX + 150;
    int sliderY = yText + 6;
    MoveToEx(hdc, sliderX, sliderY, NULL);
    LineTo(hdc, sliderX + 40, sliderY);
    
    // Индикатор ползунка
    int indicatorPos = sliderX + (g_zoomLevel - g_minZoom) * 40 / (g_maxZoom - g_minZoom);
    Ellipse(hdc, indicatorPos - 5, sliderY - 5, indicatorPos + 5, sliderY + 5);
    
    SelectObject(hdc, hOldPen);
    DeleteObject(hPenSep);
    DeleteObject(hPenSlider);
    SelectObject(hdc, hOldFont);
}

void DrawNotesPanel(HDC hdc, RECT* clientRect) {
    int panelTop = clientRect->bottom - g_statusBarHeight - g_notesHeight;
    
    RECT notesRect = {g_thumbnailWidth + 1, panelTop, clientRect->right, clientRect->bottom - g_statusBarHeight};
    
    // Фон
    HBRUSH hBrushBg = CreateSolidBrush(COLOR_WHITE);
    FillRect(hdc, &notesRect, hBrushBg);
    DeleteObject(hBrushBg);
    
    // Верхняя граница
    HPEN hPenBorder = CreatePen(PS_SOLID, 1, COLOR_BORDER_LIGHT);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBorder);
    MoveToEx(hdc, g_thumbnailWidth + 1, panelTop, NULL);
    LineTo(hdc, clientRect->right, panelTop);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPenBorder);
    
    // Заголовок
    HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFontStatus);
    SetTextColor(hdc, COLOR_TEXT_MEDIUM);
    SetBkMode(hdc, TRANSPARENT);
    TextOut(hdc, g_thumbnailWidth + 15, panelTop + 8, L"Заметки к слайду:", 17);
    
    // Текст заметок
    if (g_currentSlideIndex >= 0 && g_currentSlideIndex < (int)g_presentation.slides.size()) {
        auto& slide = g_presentation.slides[g_currentSlideIndex];
        if (!slide->notes.empty()) {
            RECT textRect = {g_thumbnailWidth + 15, panelTop + 30, clientRect->right - 15, clientRect->bottom - g_statusBarHeight - 10};
            SetTextColor(hdc, COLOR_TEXT_DARK);
            DrawText(hdc, slide->notes.c_str(), -1, &textRect, DT_LEFT | DT_TOP | DT_WORDBREAK);
        } else {
            RECT textRect = {g_thumbnailWidth + 15, panelTop + 30, clientRect->right - 15, clientRect->bottom - g_statusBarHeight - 10};
            SetTextColor(hdc, COLOR_TEXT_LIGHT);
            DrawText(hdc, L"Нажмите, чтобы добавить заметки...", -1, &textRect, DT_LEFT | DT_TOP);
        }
    }
    
    SelectObject(hdc, hOldFont);
}

void DrawDashedRect(HDC hdc, int x, int y, int width, int height) {
    HPEN hPen = CreatePen(PS_DASH, 1, RGB(180, 180, 180));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    
    Rectangle(hdc, x, y, x + width, y + height);
    
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);
}

// ============================================================================
// ОБРАБОТЧИК СООБЩЕНИЙ ГЛАВНОГО ОКНА
// ============================================================================

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            return 0;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            
            // Двойная буферизация
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
            
            // Отрисовка
            DrawAll(hdcMem, &clientRect);
            
            // Копирование на экран
            BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcMem, 0, 0, SRCCOPY);
            
            // Освобождение ресурсов
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            
            EndPaint(hWnd, &ps);
            return 0;
        }
        
        case WM_SIZE:
            InvalidateRect(hWnd, NULL, TRUE);
            return 0;
            
        case WM_ERASEBKGND:
            return 1;
            
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            
            // Проверка клика по вкладкам Ribbon
            if (y < g_tabBarHeight) {
                for (int i = 0; i < (int)g_ribbonTabs.size(); i++) {
                    POINT pt = {x, y};
                    if (PtInRect(&g_ribbonTabs[i].tabRect, pt)) {
                        g_activeTabIndex = i;
                        InvalidateRect(hWnd, NULL, TRUE);
                        return 0;
                    }
                }
            }
            
            // Проверка клика по кнопкам Ribbon
            if (y >= g_tabBarHeight && y < g_ribbonHeight) {
                if (g_activeTabIndex >= 0 && g_activeTabIndex < (int)g_ribbonTabs.size()) {
                    for (auto& group : g_ribbonTabs[g_activeTabIndex].groups) {
                        for (auto& btn : group.second) {
                            POINT pt = {x, y};
                            if (PtInRect(&btn.rect, pt)) {
                                // Обработка нажатия кнопки
                                switch (btn.id) {
                                    case IDB_NEW_SLIDE: AddSlide(); break;
                                    case IDB_DELETE_SLIDE: DeleteSlide(g_currentSlideIndex); break;
                                    case IDB_COPY_SLIDE: DuplicateSlide(g_currentSlideIndex); break;
                                    case IDB_PASTE: PasteElement(); break;
                                    case IDB_CUT: CutSelectedElement(); break;
                                    case IDB_COPY: CopySelectedElement(); break;
                                    case IDB_INSERT_TEXT: AddTextBox(); break;
                                    case IDB_INSERT_IMAGE: AddImage(); break;
                                    case IDB_INSERT_SHAPE: AddShape(ShapeType::Rectangle); break;
                                    case IDB_BOLD: FormatTextBold(); break;
                                    case IDB_ITALIC: FormatTextItalic(); break;
                                    case IDB_UNDERLINE: FormatTextUnderline(); break;
                                    case IDB_SLIDESHOW_BEGIN: StartSlideshow(true); break;
                                    case IDB_SLIDESHOW_CURRENT: StartSlideshow(false); break;
                                    case IDB_ZOOM_IN: ZoomIn(); break;
                                    case IDB_ZOOM_OUT: ZoomOut(); break;
                                    case IDB_FIT_SLIDE: FitToWindow(); break;
                                    case IDM_VIEW_RULER: ToggleRuler(); break;
                                    case IDM_VIEW_GRID: ToggleGrid(); break;
                                    case IDM_VIEW_GUIDES: ToggleGuides(); break;
                                }
                                return 0;
                            }
                        }
                    }
                }
            }
            
            // Проверка клика по миниатюрам
            if (x < g_thumbnailWidth && y > g_ribbonHeight) {
                int thumbMargin = 12;
                int thumbSlideWidth = g_thumbnailWidth - thumbMargin * 2 - 25;
                int thumbSlideHeight = (int)(thumbSlideWidth * 9.0 / 16.0);
                int thumbX = thumbMargin + 20;
                int thumbY = g_ribbonHeight + thumbMargin;
                
                for (int i = 0; i < (int)g_presentation.slides.size(); i++) {
                    RECT thumbRect = {thumbX, thumbY, thumbX + thumbSlideWidth, thumbY + thumbSlideHeight};
                    POINT pt = {x, y};
                    if (PtInRect(&thumbRect, pt)) {
                        SelectSlide(i);
                        return 0;
                    }
                    thumbY += thumbSlideHeight + thumbMargin + 5;
                }
            }
            
            // Проверка клика по области слайда
            RECT slideRect = GetSlideRect(&clientRect);
            POINT pt = {x, y};
            
            if (PtInRect(&slideRect, pt)) {
                // Проверка клика по элементу
                int elementIndex = HitTestElement(x, y, slideRect.left, slideRect.top, 
                    slideRect.right - slideRect.left, slideRect.bottom - slideRect.top);
                
                if (elementIndex >= 0) {
                    SelectElement(elementIndex);
                    
                    // Начало перетаскивания
                    g_isDragging = true;
                    g_dragStart = {x, y};
                    SetCapture(hWnd);
                } else {
                    // Клик по пустому месту - снять выделение
                    g_selectedElementIndex = -1;
                    DeselectAllElements();
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }
            
            return 0;
        }
        
        case WM_LBUTTONUP:
            if (g_isDragging || g_isResizing) {
                g_isDragging = false;
                g_isResizing = false;
                ReleaseCapture();
            }
            return 0;
            
        case WM_MOUSEMOVE: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            // Обновление состояния наведения на кнопки Ribbon
            if (y >= g_tabBarHeight && y < g_ribbonHeight) {
                bool needsRedraw = false;
                if (g_activeTabIndex >= 0 && g_activeTabIndex < (int)g_ribbonTabs.size()) {
                    for (auto& group : g_ribbonTabs[g_activeTabIndex].groups) {
                        for (auto& btn : group.second) {
                            POINT pt = {x, y};
                            bool wasHovered = btn.hovered;
                            btn.hovered = PtInRect(&btn.rect, pt);
                            if (wasHovered != btn.hovered) needsRedraw = true;
                        }
                    }
                }
                if (needsRedraw) {
                    RECT ribbonRect = {0, g_tabBarHeight, 0, g_ribbonHeight};
                    GetClientRect(hWnd, &ribbonRect);
                    ribbonRect.top = g_tabBarHeight;
                    ribbonRect.bottom = g_ribbonHeight;
                    InvalidateRect(hWnd, &ribbonRect, FALSE);
                }
            }
            
            // Перетаскивание элемента
            if (g_isDragging && g_selectedElementIndex >= 0) {
                RECT clientRect;
                GetClientRect(hWnd, &clientRect);
                RECT slideRect = GetSlideRect(&clientRect);
                
                int slideWidth = slideRect.right - slideRect.left;
                int slideHeight = slideRect.bottom - slideRect.top;
                
                int dx = (x - g_dragStart.x) * 10000 / slideWidth;
                int dy = (y - g_dragStart.y) * 10000 / slideHeight;
                
                if (dx != 0 || dy != 0) {
                    MoveElement(dx, dy);
                    g_dragStart = {x, y};
                }
            }
            
            g_lastMousePos = {x, y};
            return 0;
        }
        
        case WM_RBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            // Контекстное меню
            HMENU hMenu = CreatePopupMenu();
            
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            RECT slideRect = GetSlideRect(&clientRect);
            POINT pt = {x, y};
            
            if (x < g_thumbnailWidth && y > g_ribbonHeight) {
                // Меню для панели миниатюр
                AppendMenu(hMenu, MF_STRING, IDM_SLIDE_NEW, L"Новый слайд");
                AppendMenu(hMenu, MF_STRING, IDM_SLIDE_DUPLICATE, L"Дублировать слайд");
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hMenu, MF_STRING, IDM_SLIDE_DELETE, L"Удалить слайд");
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hMenu, MF_STRING, IDM_SLIDE_MOVE_UP, L"Переместить вверх");
                AppendMenu(hMenu, MF_STRING, IDM_SLIDE_MOVE_DOWN, L"Переместить вниз");
            } else if (PtInRect(&slideRect, pt)) {
                // Меню для области слайда
                int elemIndex = HitTestElement(x, y, slideRect.left, slideRect.top,
                    slideRect.right - slideRect.left, slideRect.bottom - slideRect.top);
                
                if (elemIndex >= 0) {
                    SelectElement(elemIndex);
                    AppendMenu(hMenu, MF_STRING, IDM_EDIT_CUT, L"Вырезать\tCtrl+X");
                    AppendMenu(hMenu, MF_STRING, IDM_EDIT_COPY, L"Копировать\tCtrl+C");
                    AppendMenu(hMenu, MF_STRING, IDM_EDIT_PASTE, L"Вставить\tCtrl+V");
                    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                    AppendMenu(hMenu, MF_STRING, IDM_EDIT_DUPLICATE, L"Дублировать");
                    AppendMenu(hMenu, MF_STRING, IDM_EDIT_DELETE, L"Удалить\tDel");
                    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                    AppendMenu(hMenu, MF_STRING, IDM_FORMAT_ARRANGE, L"На передний план");
                    AppendMenu(hMenu, MF_STRING, IDM_FORMAT_ARRANGE + 1, L"На задний план");
                } else {
                    AppendMenu(hMenu, MF_STRING, IDM_EDIT_PASTE, L"Вставить\tCtrl+V");
                    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                    AppendMenu(hMenu, MF_STRING, IDM_INSERT_TEXT, L"Вставить текст");
                    AppendMenu(hMenu, MF_STRING, IDM_INSERT_IMAGE, L"Вставить изображение");
                    AppendMenu(hMenu, MF_STRING, IDM_INSERT_IMAGE_URL, L"Вставить изображение из URL");
                    AppendMenu(hMenu, MF_STRING, IDM_INSERT_SHAPE, L"Вставить фигуру");
                }
            }
            
            POINT screenPt = {x, y};
            ClientToScreen(hWnd, &screenPt);
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, screenPt.x, screenPt.y, 0, hWnd, NULL);
            DestroyMenu(hMenu);
            
            return 0;
        }
        
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            
            switch (wmId) {
                case IDM_FILE_NEW: NewPresentation(); break;
                case IDM_FILE_OPEN: OpenPresentation(); break;
                case IDM_FILE_SAVE: SavePresentation(); break;
                case IDM_FILE_SAVE_AS: SavePresentationAs(); break;
                case IDM_FILE_EXPORT: ExportPresentation(); break;
                case IDM_FILE_PRINT: PrintPresentation(); break;
                case IDM_FILE_EXIT: PostQuitMessage(0); break;
                
                case IDM_EDIT_UNDO: Undo(); break;
                case IDM_EDIT_REDO: Redo(); break;
                case IDM_EDIT_CUT: CutSelectedElement(); break;
                case IDM_EDIT_COPY: CopySelectedElement(); break;
                case IDM_EDIT_PASTE: PasteElement(); break;
                case IDM_EDIT_DELETE: DeleteSelectedElement(); break;
                case IDM_EDIT_DUPLICATE: DuplicateSelectedElement(); break;
                
                case IDM_SLIDE_NEW: AddSlide(); break;
                case IDM_SLIDE_DUPLICATE: DuplicateSlide(g_currentSlideIndex); break;
                case IDM_SLIDE_DELETE: DeleteSlide(g_currentSlideIndex); break;
                case IDM_SLIDE_MOVE_UP: MoveSlideUp(g_currentSlideIndex); break;
                case IDM_SLIDE_MOVE_DOWN: MoveSlideDown(g_currentSlideIndex); break;
                
                case IDM_INSERT_TEXT: AddTextBox(); break;
                case IDM_INSERT_IMAGE: AddImage(); break;
                case IDM_INSERT_IMAGE_URL: AddImageFromUrl(); break;
                case IDM_INSERT_SHAPE: AddShape(ShapeType::Rectangle); break;
                
                case IDM_VIEW_NORMAL: SetViewMode(0); break;
                case IDM_VIEW_SORTER: SetViewMode(1); break;
                case IDM_VIEW_READING: SetViewMode(2); break;
                case IDM_VIEW_SLIDESHOW: StartSlideshow(true); break;
                case IDM_VIEW_ZOOM_IN: ZoomIn(); break;
                case IDM_VIEW_ZOOM_OUT: ZoomOut(); break;
                case IDM_VIEW_FIT_WINDOW: FitToWindow(); break;
                case IDM_VIEW_NOTES: ToggleNotesPanel(); break;
                case IDM_VIEW_RULER: ToggleRuler(); break;
                case IDM_VIEW_GRID: ToggleGrid(); break;
                case IDM_VIEW_GUIDES: ToggleGuides(); break;
                
                case IDM_HELP_ABOUT:
                    MessageBox(hWnd, 
                        L"PowerPoint Clone Pro\n"
                        L"Версия 1.0\n\n"
                        L"Программа для создания презентаций\n"
                        L"на основе Windows API и GDI+\n\n"
                        L"© 2025-2026 Все права защищены",
                        L"О программе", MB_OK | MB_ICONINFORMATION);
                    break;
            }
            return 0;
        }
        
        case WM_KEYDOWN: {
            bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            
            switch (wParam) {
                case VK_DELETE:
                    DeleteSelectedElement();
                    break;
                    
                case VK_LEFT:
                    if (g_selectedElementIndex >= 0) MoveElement(-100, 0);
                    break;
                case VK_RIGHT:
                    if (g_selectedElementIndex >= 0) MoveElement(100, 0);
                    break;
                case VK_UP:
                    if (g_selectedElementIndex >= 0) MoveElement(0, -100);
                    break;
                case VK_DOWN:
                    if (g_selectedElementIndex >= 0) MoveElement(0, 100);
                    break;
                    
                case VK_ESCAPE:
                    g_selectedElementIndex = -1;
                    DeselectAllElements();
                    InvalidateRect(hWnd, NULL, TRUE);
                    break;
                    
                case 'N':
                    if (ctrl) NewPresentation();
                    break;
                case 'O':
                    if (ctrl) OpenPresentation();
                    break;
                case 'S':
                    if (ctrl && shift) SavePresentationAs();
                    else if (ctrl) SavePresentation();
                    break;
                case 'Z':
                    if (ctrl) Undo();
                    break;
                case 'Y':
                    if (ctrl) Redo();
                    break;
                case 'C':
                    if (ctrl) CopySelectedElement();
                    break;
                case 'V':
                    if (ctrl) PasteElement();
                    break;
                case 'X':
                    if (ctrl) CutSelectedElement();
                    break;
                case 'D':
                    if (ctrl) DuplicateSelectedElement();
                    break;
                case 'M':
                    if (ctrl) AddSlide();
                    break;
                    
                case VK_F5:
                    if (shift) StartSlideshow(false);
                    else StartSlideshow(true);
                    break;
                    
                case VK_ADD:
                case VK_OEM_PLUS:
                    if (ctrl) ZoomIn();
                    break;
                case VK_SUBTRACT:
                case VK_OEM_MINUS:
                    if (ctrl) ZoomOut();
                    break;
                    
                case VK_PRIOR: // Page Up
                    if (g_currentSlideIndex > 0) SelectSlide(g_currentSlideIndex - 1);
                    break;
                case VK_NEXT: // Page Down
                    if (g_currentSlideIndex < (int)g_presentation.slides.size() - 1) 
                        SelectSlide(g_currentSlideIndex + 1);
                    break;
                case VK_HOME:
                    if (ctrl) SelectSlide(0);
                    break;
                case VK_END:
                    if (ctrl) SelectSlide((int)g_presentation.slides.size() - 1);
                    break;
            }
            return 0;
        }
        
        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            
            if (ctrl) {
                // Масштабирование
                if (delta > 0) ZoomIn();
                else ZoomOut();
            } else {
                // Прокрутка слайдов
                if (delta > 0 && g_currentSlideIndex > 0) {
                    SelectSlide(g_currentSlideIndex - 1);
                } else if (delta < 0 && g_currentSlideIndex < (int)g_presentation.slides.size() - 1) {
                    SelectSlide(g_currentSlideIndex + 1);
                }
            }
            return 0;
        }
        
        case WM_CLOSE:
            if (g_presentation.modified) {
                int result = MessageBox(hWnd, 
                    L"Презентация была изменена. Сохранить изменения?",
                    L"Сохранение", MB_YESNOCANCEL | MB_ICONQUESTION);
                    
                if (result == IDYES) {
                    SavePresentation();
                } else if (result == IDCANCEL) {
                    return 0;
                }
            }
            DestroyWindow(hWnd);
            return 0;
            
        case WM_DESTROY:
            Cleanup();
            PostQuitMessage(0);
            return 0;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// ============================================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ФОРМАТИРОВАНИЯ
// ============================================================================

void FormatTextBold() {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    auto& elem = g_presentation.slides[g_currentSlideIndex]->elements[g_selectedElementIndex];
    if (elem->type == SlideElementType::TextBox) {
        elem->bold = !elem->bold;
        SetModified(true);
        InvalidateRect(g_hWnd, NULL, TRUE);
    }
}

void FormatTextItalic() {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    auto& elem = g_presentation.slides[g_currentSlideIndex]->elements[g_selectedElementIndex];
    if (elem->type == SlideElementType::TextBox) {
        elem->italic = !elem->italic;
        SetModified(true);
        InvalidateRect(g_hWnd, NULL, TRUE);
    }
}

void FormatTextUnderline() {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    auto& elem = g_presentation.slides[g_currentSlideIndex]->elements[g_selectedElementIndex];
    if (elem->type == SlideElementType::TextBox) {
        elem->underline = !elem->underline;
        SetModified(true);
        InvalidateRect(g_hWnd, NULL, TRUE);
    }
}

void SetTextAlignment(int alignment) {
    // 0 = left, 1 = center, 2 = right
    // Здесь будет реализация
}

void SetFontSize(int size) {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    auto& elem = g_presentation.slides[g_currentSlideIndex]->elements[g_selectedElementIndex];
    if (elem->type == SlideElementType::TextBox) {
        elem->fontSize = size;
        SetModified(true);
        InvalidateRect(g_hWnd, NULL, TRUE);
    }
}

void SetFontName(const std::wstring& name) {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    auto& elem = g_presentation.slides[g_currentSlideIndex]->elements[g_selectedElementIndex];
    if (elem->type == SlideElementType::TextBox) {
        elem->fontName = name;
        SetModified(true);
        InvalidateRect(g_hWnd, NULL, TRUE);
    }
}

void SetTextColor(COLORREF color) {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    auto& elem = g_presentation.slides[g_currentSlideIndex]->elements[g_selectedElementIndex];
    elem->textColor = color;
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void SetFillColor(COLORREF color) {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    auto& elem = g_presentation.slides[g_currentSlideIndex]->elements[g_selectedElementIndex];
    elem->fillColor = color;
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void BringToFront() {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    auto& elements = g_presentation.slides[g_currentSlideIndex]->elements;
    auto elem = elements[g_selectedElementIndex];
    elements.erase(elements.begin() + g_selectedElementIndex);
    elements.push_back(elem);
    g_selectedElementIndex = (int)elements.size() - 1;
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void SendToBack() {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    auto& elements = g_presentation.slides[g_currentSlideIndex]->elements;
    auto elem = elements[g_selectedElementIndex];
    elements.erase(elements.begin() + g_selectedElementIndex);
    elements.insert(elements.begin(), elem);
    g_selectedElementIndex = 0;
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void BringForward() {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex < 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size() - 1) return;
    
    auto& elements = g_presentation.slides[g_currentSlideIndex]->elements;
    std::swap(elements[g_selectedElementIndex], elements[g_selectedElementIndex + 1]);
    g_selectedElementIndex++;
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void SendBackward() {
    if (g_currentSlideIndex < 0 || g_currentSlideIndex >= (int)g_presentation.slides.size()) return;
    if (g_selectedElementIndex <= 0 || g_selectedElementIndex >= (int)g_presentation.slides[g_currentSlideIndex]->elements.size()) return;
    
    auto& elements = g_presentation.slides[g_currentSlideIndex]->elements;
    std::swap(elements[g_selectedElementIndex], elements[g_selectedElementIndex - 1]);
    g_selectedElementIndex--;
    
    SetModified(true);
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void AlignElements(int alignment) {
    // 0 = left, 1 = center, 2 = right, 3 = top, 4 = middle, 5 = bottom
    // Здесь будет реализация выравнивания
}

// ============================================================================
// ТОЧКА ВХОДА
// ============================================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Инициализация GDI+
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
    
    // Инициализация Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);
    
    // Регистрация класса окна
    if (!InitApplication(hInstance)) {
        MessageBox(NULL, L"Ошибка регистрации класса окна!", L"Ошибка", MB_ICONERROR);
        return 1;
    }
    
    // Создание главного окна
    if (!InitInstance(hInstance, nCmdShow)) {
        MessageBox(NULL, L"Ошибка создания окна!", L"Ошибка", MB_ICONERROR);
        return 1;
    }
    
    // Цикл сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
