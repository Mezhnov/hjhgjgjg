// ============================================================================
// MainPage.xaml.cs - БРАУЗЕР MAUI (Вставьте этот код в MainPage.xaml.cs)
// ============================================================================

using Microsoft.Maui.Controls;
using Microsoft.Maui.Graphics;
using Microsoft.Maui.Layouts;

namespace MauiApp4;

public partial class MainPage : ContentPage
{
    // UI элементы
    private readonly Entry _urlEntry;
    private readonly WebView _webView;
    private readonly Label _statusLabel;
    private readonly Button _backButton;
    private readonly Button _forwardButton;
    private readonly Button _refreshButton;
    private readonly Button _goButton;
    private readonly Button _homeButton;
    private readonly ActivityIndicator _loadingIndicator;
    
    // Настройки
    private const string HomePage = "https://www.google.com";
    private readonly List<string> _history = new();
    private int _historyIndex = -1;
    private bool _isNavigating = false;

    public MainPage()
    {
        Title = "MAUI Browser";
        BackgroundColor = Colors.White;
        
        // Инициализация элементов
        _loadingIndicator = new ActivityIndicator
        {
            IsRunning = false,
            IsVisible = false,
            Color = Colors.DodgerBlue,
            WidthRequest = 30,
            HeightRequest = 30
        };

        _backButton = CreateButton("◀", OnBackClicked, Colors.DodgerBlue);
        _forwardButton = CreateButton("▶", OnForwardClicked, Colors.DodgerBlue);
        _refreshButton = CreateButton("🔄", OnRefreshClicked, Colors.SeaGreen);
        _homeButton = CreateButton("🏠", OnHomeClicked, Colors.OrangeRed);
        
        _goButton = new Button
        {
            Text = "➤",
            BackgroundColor = Colors.DodgerBlue,
            TextColor = Colors.White,
            FontSize = 16,
            FontAttributes = FontAttributes.Bold,
            WidthRequest = 50,
            HeightRequest = 45,
            CornerRadius = 8
        };
        _goButton.Clicked += OnGoClicked;

        _urlEntry = new Entry
        {
            Placeholder = "Введите URL или поисковый запрос...",
            FontSize = 16,
            BackgroundColor = Colors.White,
            PlaceholderColor = Colors.Gray,
            TextColor = Colors.Black,
            HeightRequest = 45,
            HorizontalOptions = LayoutOptions.Fill,
            ReturnType = ReturnType.Go
        };
        _urlEntry.Completed += OnGoClicked;
        _urlEntry.Focused += OnUrlEntryFocused;

        _statusLabel = new Label
        {
            Text = "Готов к работе",
            FontSize = 12,
            TextColor = Colors.Gray,
            Padding = new Thickness(10, 5),
            HorizontalOptions = LayoutOptions.Start
        };

        _webView = new WebView
        {
            Source = HomePage,
            HorizontalOptions = LayoutOptions.Fill,
            VerticalOptions = LayoutOptions.Fill,
            BackgroundColor = Colors.White
        };
        
        _webView.Navigating += OnWebViewNavigating;
        _webView.Navigated += OnWebViewNavigated;

        // Используем Grid вместо устаревшего StackLayout с FillAndExpand
        var mainGrid = new Grid
        {
            RowDefinitions =
            {
                new RowDefinition { Height = GridLength.Auto }, // Toolbar
                new RowDefinition { Height = GridLength.Auto }, // Separator
                new RowDefinition { Height = GridLength.Star }, // WebView
                new RowDefinition { Height = GridLength.Auto }  // Status
            }
        };

        // Toolbar
        var toolbarGrid = new Grid
        {
            ColumnDefinitions =
            {
                new ColumnDefinition { Width = 50 },  // Back
                new ColumnDefinition { Width = 50 },  // Forward
                new ColumnDefinition { Width = 50 },  // Refresh
                new ColumnDefinition { Width = 50 },  // Home
                new ColumnDefinition { Width = GridLength.Star }, // URL Entry
                new ColumnDefinition { Width = 55 },  // Go
                new ColumnDefinition { Width = 40 },  // Loading
            },
            Padding = new Thickness(10, 5),
            ColumnSpacing = 5
        };

        toolbarGrid.Add(_backButton, 0, 0);
        toolbarGrid.Add(_forwardButton, 1, 0);
        toolbarGrid.Add(_refreshButton, 2, 0);
        toolbarGrid.Add(_homeButton, 3, 0);

        // Border вместо устаревшего Frame
        var urlBorder = new Border
        {
            Content = _urlEntry,
            BackgroundColor = Colors.White,
            Stroke = Colors.LightGray,
            StrokeThickness = 1,
            StrokeShape = new RoundRectangle { CornerRadius = 8 },
            Padding = new Thickness(10, 0),
            VerticalOptions = LayoutOptions.Center
        };
        toolbarGrid.Add(urlBorder, 4, 0);
        toolbarGrid.Add(_goButton, 5, 0);
        toolbarGrid.Add(_loadingIndicator, 6, 0);

        var separator = new BoxView
        {
            HeightRequest = 1,
            BackgroundColor = Colors.LightGray,
            HorizontalOptions = LayoutOptions.Fill
        };

        // Status bar с Border вместо Frame
        var statusBorder = new Border
        {
            Content = _statusLabel,
            BackgroundColor = Color.FromArgb("#F5F5F5"),
            StrokeThickness = 0,
            Padding = new Thickness(10, 3),
            HorizontalOptions = LayoutOptions.Fill
        };

        mainGrid.Add(toolbarGrid, 0, 0);
        mainGrid.Add(separator, 0, 1);
        mainGrid.Add(_webView, 0, 2);
        mainGrid.Add(statusBorder, 0, 3);

        Content = mainGrid;
        
        AddToHistory(HomePage);
    }

    private Button CreateButton(string text, EventHandler handler, Color bgColor)
    {
        var button = new Button
        {
            Text = text,
            BackgroundColor = bgColor,
            TextColor = Colors.White,
            FontSize = 18,
            WidthRequest = 45,
            HeightRequest = 45,
            CornerRadius = 8
        };
        button.Clicked += handler;
        return button;
    }

    private void OnBackClicked(object? sender, EventArgs e)
    {
        if (_historyIndex > 0)
        {
            _historyIndex--;
            _isNavigating = true;
            _webView.Source = _history[_historyIndex];
            UpdateButtonsState();
        }
    }

    private void OnForwardClicked(object? sender, EventArgs e)
    {
        if (_historyIndex < _history.Count - 1)
        {
            _historyIndex++;
            _isNavigating = true;
            _webView.Source = _history[_historyIndex];
            UpdateButtonsState();
        }
    }

    private void OnRefreshClicked(object? sender, EventArgs e)
    {
        _webView.Reload();
    }

    private void OnHomeClicked(object? sender, EventArgs e)
    {
        NavigateToUrl(HomePage);
    }

    private void OnGoClicked(object? sender, EventArgs e)
    {
        var input = _urlEntry.Text?.Trim();
        if (string.IsNullOrWhiteSpace(input))
            return;

        var url = ProcessInput(input);
        NavigateToUrl(url);
    }

    private void OnUrlEntryFocused(object? sender, FocusEventArgs e)
    {
        MainThread.BeginInvokeOnMainThread(() =>
        {
            _urlEntry.CursorPosition = 0;
            _urlEntry.SelectionLength = _urlEntry.Text?.Length ?? 0;
        });
    }

    private void OnWebViewNavigating(object? sender, WebNavigatingEventArgs e)
    {
        _loadingIndicator.IsRunning = true;
        _loadingIndicator.IsVisible = true;
        _statusLabel.Text = $"Загрузка: {e.Url}";
        _urlEntry.Text = e.Url;
        
        if (!_isNavigating && e.Url != null)
        {
            AddToHistory(e.Url);
        }
        _isNavigating = false;
    }

    private void OnWebViewNavigated(object? sender, WebNavigatedEventArgs e)
    {
        _loadingIndicator.IsRunning = false;
        _loadingIndicator.IsVisible = false;
        
        if (e.Result == WebNavigationResult.Success)
        {
            _statusLabel.Text = $"Готово: {e.Url}";
        }
        else if (e.Result == WebNavigationResult.Failure)
        {
            _statusLabel.Text = $"Ошибка загрузки: {e.Url}";
            DisplayAlert("Ошибка", $"Не удалось загрузить страницу: {e.Url}", "OK");
        }
        else if (e.Result == WebNavigationResult.Timeout)
        {
            _statusLabel.Text = "Превышено время ожидания";
        }
        
        UpdateButtonsState();
    }

    private void NavigateToUrl(string url)
    {
        _urlEntry.Text = url;
        _webView.Source = url;
    }

    private string ProcessInput(string input)
    {
        if (input.StartsWith("http://", StringComparison.OrdinalIgnoreCase) ||
            input.StartsWith("https://", StringComparison.OrdinalIgnoreCase))
        {
            return input;
        }
        
        if (input.Contains('.') && !input.Contains(' '))
        {
            return $"https://{input}";
        }
        
        return $"https://www.google.com/search?q={Uri.EscapeDataString(input)}";
    }

    private void AddToHistory(string url)
    {
        if (_historyIndex < _history.Count - 1)
        {
            _history.RemoveRange(_historyIndex + 1, _history.Count - _historyIndex - 1);
        }
        
        _history.Add(url);
        _historyIndex = _history.Count - 1;
        
        UpdateButtonsState();
    }

    private void UpdateButtonsState()
    {
        _backButton.IsEnabled = _historyIndex > 0;
        _forwardButton.IsEnabled = _historyIndex < _history.Count - 1;
        
        _backButton.BackgroundColor = _backButton.IsEnabled ? Colors.DodgerBlue : Colors.Gray;
        _forwardButton.BackgroundColor = _forwardButton.IsEnabled ? Colors.DodgerBlue : Colors.Gray;
    }
}
