// ============================================================================
// YANDEX-STYLE MAUI BROWSER - Полнофункциональный браузер
// ============================================================================
// Функции: Вкладки, Закладки, Быстрые ссылки, История, Поиск Яндекса
// ============================================================================

using Microsoft.Maui.Controls;
using Microsoft.Maui.Graphics;
using Microsoft.Maui.Layouts;
using System.Collections.ObjectModel;

namespace MauiApp4;

public partial class MainPage : ContentPage
{
    // ========== КОНСТАНТЫ ==========
    private const string SearchEngine = "https://yandex.ru/search/?text=";
    private const string HomePage = "https://yandex.ru";
    
    // ========== UI ЭЛЕМЕНТЫ ==========
    private readonly Grid _mainGrid;
    private readonly Grid _browserGrid;
    private readonly Grid _homeGrid;
    private readonly Entry _searchEntry;
    private readonly Label _statusLabel;
    private readonly WebView _webView;
    private readonly ActivityIndicator _loadingIndicator;
    private readonly HorizontalStackLayout _tabsBar;
    private readonly ScrollView _tabsScrollView;
    private readonly Button _backButton;
    private readonly Button _forwardButton;
    private readonly Button _refreshButton;
    private readonly Button _homeButton;
    private readonly Button _tabsButton;
    private readonly Button _menuButton;
    private readonly Label _tabCountLabel;
    
    // ========== ДАННЫЕ ==========
    private readonly ObservableCollection<TabInfo> _tabs = new();
    private readonly ObservableCollection<BookmarkItem> _bookmarks = new();
    private readonly List<HistoryItem> _history = new();
    private int _currentTabIndex = 0;
    private int _historyIndex = -1;
    private bool _isNavigating = false;
    private bool _isHomeVisible = true;

    public MainPage()
    {
        Title = "Yandex Browser";
        BackgroundColor = Color.FromArgb("#FFFFFF");
        
        // Инициализация коллекций
        InitializeDefaultBookmarks();
        
        // Создаём UI
        _mainGrid = new Grid
        {
            RowDefinitions =
            {
                new RowDefinition { Height = GridLength.Auto }, // Title Bar
                new RowDefinition { Height = GridLength.Auto }, // Toolbar
                new RowDefinition { Height = GridLength.Auto }, // Tabs
                new RowDefinition { Height = GridLength.Star }, // Content
                new RowDefinition { Height = GridLength.Auto }  // Status
            }
        };

        // ========== TITLE BAR (с логотипом) ==========
        var titleBar = CreateTitleBar();
        _mainGrid.Add(titleBar, 0, 0);

        // ========== TOOLBAR ==========
        var toolbar = CreateToolbar();
        _mainGrid.Add(toolbar, 0, 1);

        // ========== TABS BAR ==========
        _tabsScrollView = new ScrollView
        {
            Orientation = ScrollOrientation.Horizontal,
            HorizontalScrollBarVisibility = ScrollBarVisibility.Never,
            HeightRequest = 0 // Скрыт по умолчанию
        };
        _tabsBar = new HorizontalStackLayout { Spacing = 5, Padding = new Thickness(5) };
        _tabsScrollView.Content = _tabsBar;
        _mainGrid.Add(_tabsScrollView, 0, 2);

        // ========== CONTENT AREA ==========
        _browserGrid = CreateBrowserGrid();
        _homeGrid = CreateHomeGrid();
        
        var contentGrid = new Grid();
        contentGrid.Add(_homeGrid, 0, 0);
        contentGrid.Add(_browserGrid, 0, 0);
        _mainGrid.Add(contentGrid, 0, 3);

        // ========== STATUS BAR ==========
        var statusBar = CreateStatusBar();
        _mainGrid.Add(statusBar, 0, 4);

        Content = _mainGrid;
        
        // Создаём первую вкладку
        CreateNewTab();
        
        // Показываем домашнюю страницу
        ShowHome();
    }

    // ========== ИНИЦИАЛИЗАЦИЯ ==========

    private void InitializeDefaultBookmarks()
    {
        _bookmarks.Add(new BookmarkItem { Title = "Яндекс", Url = "https://yandex.ru", Icon = "🔍", Color = Color.FromArgb("#FF0000") });
        _bookmarks.Add(new BookmarkItem { Title = "YouTube", Url = "https://youtube.com", Icon = "▶️", Color = Color.FromArgb("#FF0000") });
        _bookmarks.Add(new BookmarkItem { Title = "VK", Url = "https://vk.com", Icon = "💬", Color = Color.FromArgb("#0077FF") });
        _bookmarks.Add(new BookmarkItem { Title = "Wikipedia", Url = "https://ru.wikipedia.org", Icon = "📚", Color = Color.FromArgb("#333333") });
        _bookmarks.Add(new BookmarkItem { Title = "GitHub", Url = "https://github.com", Icon = "💻", Color = Color.FromArgb("#24292F") });
        _bookmarks.Add(new BookmarkItem { Title = "Telegram", Url = "https://web.telegram.org", Icon = "✈️", Color = Color.FromArgb("#0088CC") });
    }

    // ========== СОЗДАНИЕ UI ЭЛЕМЕНТОВ ==========

    private Grid CreateTitleBar()
    {
        var grid = new Grid
        {
            ColumnDefinitions =
            {
                new ColumnDefinition { Width = GridLength.Auto },
                new ColumnDefinition { Width = GridLength.Star },
                new ColumnDefinition { Width = GridLength.Auto },
            },
            BackgroundColor = Color.FromArgb("#FF0000"),
            Padding = new Thickness(15, 10)
        };

        var logoLabel = new Label
        {
            Text = "Я",
            FontSize = 28,
            FontAttributes = FontAttributes.Bold,
            TextColor = Colors.White,
            VerticalOptions = LayoutOptions.Center
        };

        var titleLabel = new Label
        {
            Text = "Браузер",
            FontSize = 18,
            FontAttributes = FontAttributes.Bold,
            TextColor = Colors.White,
            VerticalOptions = LayoutOptions.Center,
            HorizontalOptions = LayoutOptions.Center,
            HorizontalTextAlignment = TextAlignment.Center
        };

        var windowButtons = new HorizontalStackLayout
        {
            Spacing = 10,
            Children =
            {
                CreateWindowButton("─", Colors.Transparent),
                CreateWindowButton("□", Colors.Transparent),
                CreateWindowButton("✕", Colors.Transparent)
            }
        };

        grid.Add(logoLabel, 0, 0);
        grid.Add(titleLabel, 1, 0);
        grid.Add(windowButtons, 2, 0);

        return grid;
    }

    private Button CreateWindowButton(string text, Color bgColor)
    {
        return new Button
        {
            Text = text,
            BackgroundColor = bgColor,
            TextColor = Colors.White,
            FontSize = 14,
            WidthRequest = 35,
            HeightRequest = 30,
            CornerRadius = 4
        };
    }

    private Grid CreateToolbar()
    {
        var grid = new Grid
        {
            ColumnDefinitions =
            {
                new ColumnDefinition { Width = 45 },
                new ColumnDefinition { Width = 45 },
                new ColumnDefinition { Width = 45 },
                new ColumnDefinition { Width = GridLength.Star },
                new ColumnDefinition { Width = 45 },
                new ColumnDefinition { Width = 45 },
                new ColumnDefinition { Width = 45 },
                new ColumnDefinition { Width = 45 },
            },
            Padding = new Thickness(5, 8),
            ColumnSpacing = 3,
            BackgroundColor = Color.FromArgb("#FAFAFA")
        };

        _backButton = CreateToolbarButton("◀", OnBackClicked, Colors.Gray);
        _forwardButton = CreateToolbarButton("▶", OnForwardClicked, Colors.Gray);
        _homeButton = CreateToolbarButton("🏠", OnHomeClicked, Colors.Gray);
        
        // Search Entry
        _searchEntry = new Entry
        {
            Placeholder = "Найдётся всё...",
            FontSize = 15,
            BackgroundColor = Colors.White,
            PlaceholderColor = Colors.Gray,
            TextColor = Colors.Black,
            HeightRequest = 42,
            HorizontalOptions = LayoutOptions.Fill,
            ReturnType = ReturnType.Search,
            ClearButtonVisibility = ClearButtonVisibility.WhileEditing
        };
        _searchEntry.Completed += OnSearchCompleted;
        _searchEntry.Focused += OnSearchFocused;

        var searchBorder = new Border
        {
            Content = _searchEntry,
            BackgroundColor = Colors.White,
            Stroke = Color.FromArgb("#E0E0E0"),
            StrokeThickness = 1,
            StrokeShape = new RoundRectangle { CornerRadius = 21 },
            Padding = new Thickness(15, 0)
        };

        _refreshButton = CreateToolbarButton("↻", OnRefreshClicked, Colors.Gray);
        
        _tabsButton = new Button
        {
            Text = "1",
            FontSize = 12,
            FontAttributes = FontAttributes.Bold,
            BackgroundColor = Color.FromArgb("#FF0000"),
            TextColor = Colors.White,
            WidthRequest = 38,
            HeightRequest = 38,
            CornerRadius = 19
        };
        _tabsButton.Clicked += OnTabsClicked;
        _tabCountLabel = new Label { Text = "1" };

        var bookmarkBtn = CreateToolbarButton("☆", OnAddBookmarkClicked, Colors.Gray);
        _menuButton = CreateToolbarButton("⋮", OnMenuClicked, Colors.Gray);

        grid.Add(_backButton, 0, 0);
        grid.Add(_forwardButton, 1, 0);
        grid.Add(_homeButton, 2, 0);
        grid.Add(searchBorder, 3, 0);
        grid.Add(_refreshButton, 4, 0);
        grid.Add(bookmarkBtn, 5, 0);
        grid.Add(_tabsButton, 6, 0);
        grid.Add(_menuButton, 7, 0);

        return grid;
    }

    private Button CreateToolbarButton(string text, EventHandler handler, Color textColor)
    {
        var btn = new Button
        {
            Text = text,
            BackgroundColor = Colors.Transparent,
            TextColor = textColor,
            FontSize = 20,
            WidthRequest = 42,
            HeightRequest = 42,
            CornerRadius = 21
        };
        btn.Clicked += handler;
        return btn;
    }

    private Grid CreateBrowserGrid()
    {
        var grid = new Grid
        {
            RowDefinitions =
            {
                new RowDefinition { Height = GridLength.Star },
                new RowDefinition { Height = GridLength.Auto }
            }
        };

        _webView = new WebView
        {
            HorizontalOptions = LayoutOptions.Fill,
            VerticalOptions = LayoutOptions.Fill,
            BackgroundColor = Colors.White
        };
        _webView.Navigating += OnWebViewNavigating;
        _webView.Navigated += OnWebViewNavigated;
        _webView.IsVisible = false;

        _loadingIndicator = new ActivityIndicator
        {
            IsRunning = false,
            IsVisible = false,
            Color = Color.FromArgb("#FF0000"),
            WidthRequest = 40,
            HeightRequest = 40,
            HorizontalOptions = LayoutOptions.Center,
            VerticalOptions = LayoutOptions.Center
        };

        grid.Add(_webView, 0, 0);
        grid.Add(_loadingIndicator, 0, 0);

        return grid;
    }

    private Grid CreateHomeGrid()
    {
        var grid = new Grid
        {
            RowDefinitions =
            {
                new RowDefinition { Height = GridLength.Auto },
                new RowDefinition { Height = GridLength.Star },
                new RowDefinition { Height = GridLength.Auto }
            },
            BackgroundColor = Colors.White,
            Padding = new Thickness(20)
        };

        // Логотип
        var logoGrid = new Grid
        {
            RowDefinitions =
            {
                new RowDefinition { Height = GridLength.Auto },
                new RowDefinition { Height = GridLength.Auto }
            },
            Margin = new Thickness(0, 50, 0, 30)
        };

        var yandexLogo = new Label
        {
            Text = "Я",
            FontSize = 72,
            FontAttributes = FontAttributes.Bold,
            TextColor = Color.FromArgb("#FF0000"),
            HorizontalOptions = LayoutOptions.Center,
            HorizontalTextAlignment = TextAlignment.Center
        };

        var subtitleLabel = new Label
        {
            Text = "Найдётся всё",
            FontSize = 18,
            TextColor = Colors.Gray,
            HorizontalOptions = LayoutOptions.Center,
            HorizontalTextAlignment = TextAlignment.Center,
            Margin = new Thickness(0, 10, 0, 0)
        };

        logoGrid.Add(yandexLogo, 0, 0);
        logoGrid.Add(subtitleLabel, 0, 1);

        // Быстрые ссылки
        var quickLinksTitle = new Label
        {
            Text = "Быстрые ссылки",
            FontSize = 16,
            FontAttributes = FontAttributes.Bold,
            TextColor = Colors.Black,
            Margin = new Thickness(0, 20, 0, 15)
        };

        var quickLinksGrid = new Grid
        {
            ColumnDefinitions =
            {
                new ColumnDefinition { Width = GridLength.Star },
                new ColumnDefinition { Width = GridLength.Star },
                new ColumnDefinition { Width = GridLength.Star },
                new ColumnDefinition { Width = GridLength.Star },
            },
            RowDefinitions =
            {
                new RowDefinition { Height = GridLength.Auto },
                new RowDefinition { Height = GridLength.Auto },
            },
            RowSpacing = 15,
            ColumnSpacing = 15
        };

        for (int i = 0; i < _bookmarks.Count && i < 8; i++)
        {
            var bookmark = _bookmarks[i];
            var tile = CreateQuickLinkTile(bookmark);
            int row = i / 4;
            int col = i % 4;
            quickLinksGrid.Add(tile, col, row);
        }

        var quickLinksContainer = new VerticalStackLayout
        {
            Children = { quickLinksTitle, quickLinksGrid },
            HorizontalOptions = LayoutOptions.Center,
            MaximumWidthRequest = 600
        };

        grid.Add(logoGrid, 0, 0);
        grid.Add(quickLinksContainer, 0, 1);

        return grid;
    }

    private Border CreateQuickLinkTile(BookmarkItem bookmark)
    {
        var tapGesture = new TapGestureRecognizer();
        tapGesture.Tapped += (s, e) => NavigateToUrl(bookmark.Url);

        var iconLabel = new Label
        {
            Text = bookmark.Icon,
            FontSize = 28,
            HorizontalOptions = LayoutOptions.Center,
            HorizontalTextAlignment = TextAlignment.Center
        };

        var titleLabel = new Label
        {
            Text = bookmark.Title,
            FontSize = 11,
            TextColor = Colors.Gray,
            HorizontalOptions = LayoutOptions.Center,
            HorizontalTextAlignment = TextAlignment.Center,
            LineBreakMode = LineBreakMode.TailTruncation,
            MaximumHeightRequest = 30
        };

        var content = new VerticalStackLayout
        {
            Children = { iconLabel, titleLabel },
            Spacing = 8,
            Padding = new Thickness(10),
            HorizontalOptions = LayoutOptions.Center
        };

        var border = new Border
        {
            Content = content,
            BackgroundColor = Colors.White,
            Stroke = Color.FromArgb("#E8E8E8"),
            StrokeThickness = 1,
            StrokeShape = new RoundRectangle { CornerRadius = 12 },
            WidthRequest = 80,
            HeightRequest = 90,
            HorizontalOptions = LayoutOptions.Center,
            Shadow = new Shadow
            {
                Brush = Colors.Black,
                Offset = new Point(0, 2),
                Radius = 4,
                Opacity = 0.1f
            }
        };

        border.GestureRecognizers.Add(tapGesture);
        return border;
    }

    private Border CreateStatusBar()
    {
        _statusLabel = new Label
        {
            Text = "Готов к работе",
            FontSize = 12,
            TextColor = Colors.Gray,
            Padding = new Thickness(15, 8),
            HorizontalOptions = LayoutOptions.Fill
        };

        return new Border
        {
            Content = _statusLabel,
            BackgroundColor = Color.FromArgb("#F5F5F5"),
            StrokeThickness = 0,
            HorizontalOptions = LayoutOptions.Fill
        };
    }

    // ========== НАВИГАЦИЯ ==========

    private void OnSearchCompleted(object? sender, EventArgs e)
    {
        var query = _searchEntry.Text?.Trim();
        if (string.IsNullOrWhiteSpace(query))
            return;

        // Проверяем, является ли ввод URL
        string url;
        if (query.StartsWith("http://", StringComparison.OrdinalIgnoreCase) ||
            query.StartsWith("https://", StringComparison.OrdinalIgnoreCase))
        {
            url = query;
        }
        else if (query.Contains('.') && !query.Contains(' ') && !query.Contains('?'))
        {
            url = $"https://{query}";
        }
        else
        {
            url = SearchEngine + Uri.EscapeDataString(query);
        }

        NavigateToUrl(url);
    }

    private void OnSearchFocused(object? sender, FocusEventArgs e)
    {
        MainThread.BeginInvokeOnMainThread(() =>
        {
            _searchEntry.CursorPosition = 0;
            _searchEntry.SelectionLength = _searchEntry.Text?.Length ?? 0;
        });
    }

    private void NavigateToUrl(string url)
    {
        if (_tabs.Count == 0 || _currentTabIndex < 0)
        {
            CreateNewTab();
        }

        _isHomeVisible = false;
        _homeGrid.IsVisible = false;
        _webView.IsVisible = true;
        _searchEntry.Text = url;
        _webView.Source = url;
        
        AddToHistory(url);
    }

    private void ShowHome()
    {
        _isHomeVisible = true;
        _homeGrid.IsVisible = true;
        _webView.IsVisible = false;
        _searchEntry.Text = string.Empty;
        _searchEntry.Placeholder = "Найдётся всё...";
        _statusLabel.Text = "Готов к работе";
    }

    // ========== ОБРАБОТЧИКИ КНОПОК ==========

    private void OnBackClicked(object? sender, EventArgs e)
    {
        if (_webView.CanGoBack)
        {
            _webView.GoBack();
        }
    }

    private void OnForwardClicked(object? sender, EventArgs e)
    {
        if (_webView.CanGoForward)
        {
            _webView.GoForward();
        }
    }

    private void OnRefreshClicked(object? sender, EventArgs e)
    {
        if (!_isHomeVisible)
        {
            _webView.Reload();
        }
    }

    private void OnHomeClicked(object? sender, EventArgs e)
    {
        ShowHome();
    }

    private void OnTabsClicked(object? sender, EventArgs e)
    {
        ShowTabsPanel();
    }

    private void OnAddBookmarkClicked(object? sender, EventArgs e)
    {
        if (!_isHomeVisible && !string.IsNullOrWhiteSpace(_searchEntry.Text))
        {
            var url = _searchEntry.Text;
            var title = ExtractDomainFromUrl(url);
            
            _bookmarks.Add(new BookmarkItem
            {
                Title = title,
                Url = url,
                Icon = "🔗",
                Color = Colors.Gray
            });

            DisplayAlert("Закладка добавлена", $"Страница \"{title}\" добавлена в закладки", "OK");
        }
    }

    private void OnMenuClicked(object? sender, EventArgs e)
    {
        ShowMenu();
    }

    // ========== WEBVIEW СОБЫТИЯ ==========

    private void OnWebViewNavigating(object? sender, WebNavigatingEventArgs e)
    {
        _loadingIndicator.IsRunning = true;
        _loadingIndicator.IsVisible = true;
        _statusLabel.Text = $"Загрузка: {e.Url}";
        _searchEntry.Text = e.Url;

        if (!_isNavigating && e.Url != null)
        {
            AddToHistory(e.Url);
        }
        _isNavigating = false;

        UpdateNavigationButtons();
    }

    private void OnWebViewNavigated(object? sender, WebNavigatedEventArgs e)
    {
        _loadingIndicator.IsRunning = false;
        _loadingIndicator.IsVisible = false;

        if (e.Result == WebNavigationResult.Success)
        {
            _statusLabel.Text = $"Готово: {ExtractDomainFromUrl(e.Url)}";
        }
        else if (e.Result == WebNavigationResult.Failure)
        {
            _statusLabel.Text = "Ошибка загрузки страницы";
            DisplayAlert("Ошибка", $"Не удалось загрузить страницу", "OK");
        }
        else if (e.Result == WebNavigationResult.Timeout)
        {
            _statusLabel.Text = "Превышено время ожидания";
        }

        UpdateNavigationButtons();
    }

    private void UpdateNavigationButtons()
    {
        _backButton.TextColor = _webView.CanGoBack ? Colors.Black : Colors.Gray;
        _forwardButton.TextColor = _webView.CanGoForward ? Colors.Black : Colors.Gray;
    }

    // ========== ВКЛАДКИ ==========

    private void CreateNewTab(string url = "")
    {
        var tab = new TabInfo
        {
            Id = Guid.NewGuid(),
            Title = string.IsNullOrEmpty(url) ? "Новая вкладка" : ExtractDomainFromUrl(url),
            Url = url
        };

        _tabs.Add(tab);
        _currentTabIndex = _tabs.Count - 1;
        UpdateTabsButton();
    }

    private void UpdateTabsButton()
    {
        _tabsButton.Text = _tabs.Count.ToString();
    }

    private void ShowTabsPanel()
    {
        var actionSheet = new ActionSheet("Вкладки", "Отмена", null);

        foreach (var tab in _tabs)
        {
            actionSheet.Add(tab.Title, () =>
            {
                _currentTabIndex = _tabs.IndexOf(tab);
                if (!string.IsNullOrEmpty(tab.Url))
                {
                    NavigateToUrl(tab.Url);
                }
                else
                {
                    ShowHome();
                }
            });
        }

        actionSheet.Add("+ Новая вкладка", () =>
        {
            CreateNewTab();
            ShowHome();
        });

        DisplayActionSheet(actionSheet);
    }

    // ========== МЕНЮ ==========

    private void ShowMenu()
    {
        var action = DisplayActionSheet(
            "Меню",
            "Отмена",
            null,
            "📚 Закладки",
            "📜 История",
            "🔄 Обновить",
            "⭐ Добавить в закладки",
            "📤 Поделиться",
            "⚙️ Настройки",
            "❓ Справка");

        action.ContinueWith(t =>
        {
            MainThread.BeginInvokeOnMainThread(() =>
            {
                switch (t.Result)
                {
                    case "📚 Закладки":
                        ShowBookmarks();
                        break;
                    case "📜 История":
                        ShowHistory();
                        break;
                    case "🔄 Обновить":
                        _webView.Reload();
                        break;
                    case "⭐ Добавить в закладки":
                        OnAddBookmarkClicked(null, EventArgs.Empty);
                        break;
                }
            });
        });
    }

    private void ShowBookmarks()
    {
        var bookmarkList = string.Join("\n", _bookmarks.Take(10).Select(b => $"• {b.Title}: {b.Url}"));
        DisplayAlert("Закладки", bookmarkList ?? "Нет закладок", "OK");
    }

    private void ShowHistory()
    {
        var historyList = string.Join("\n", _history.TakeLast(10).Select(h => $"• {h.Title}"));
        DisplayAlert("История", historyList ?? "История пуста", "OK");
    }

    // ========== ИСТОРИЯ ==========

    private void AddToHistory(string url)
    {
        if (_history.Count > 0 && _history.Last().Url == url)
            return;

        _history.Add(new HistoryItem
        {
            Url = url,
            Title = ExtractDomainFromUrl(url),
            Timestamp = DateTime.Now
        });

        // Обновляем текущую вкладку
        if (_currentTabIndex >= 0 && _currentTabIndex < _tabs.Count)
        {
            _tabs[_currentTabIndex].Url = url;
            _tabs[_currentTabIndex].Title = ExtractDomainFromUrl(url);
        }
    }

    // ========== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ==========

    private string ExtractDomainFromUrl(string url)
    {
        try
        {
            var uri = new Uri(url);
            return uri.Host.Replace("www.", "");
        }
        catch
        {
            return url;
        }
    }

    // ========== КЛАССЫ ДАННЫХ ==========

    public class TabInfo
    {
        public Guid Id { get; set; }
        public string Title { get; set; } = "";
        public string Url { get; set; } = "";
    }

    public class BookmarkItem
    {
        public string Title { get; set; } = "";
        public string Url { get; set; } = "";
        public string Icon { get; set; } = "🔗";
        public Color Color { get; set; } = Colors.Gray;
    }

    public class HistoryItem
    {
        public string Url { get; set; } = "";
        public string Title { get; set; } = "";
        public DateTime Timestamp { get; set; }
    }
}
