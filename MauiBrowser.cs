// ============================================================================
// MAUI BROWSER - Полнофункциональный браузер на .NET MAUI в одном файле
// ============================================================================
// Для запуска создайте проект: dotnet new maui -n MauiBrowser
// Замените содержимое MainPage.xaml.cs на этот файл
// Или используйте как отдельный файл с правильной структурой проекта
// ============================================================================

using Microsoft.Maui.Controls;
using Microsoft.Maui.Graphics;
using Microsoft.Maui.Layouts;

namespace MauiBrowser;

/// <summary>
/// Главная страница браузера с полной функциональностью
/// </summary>
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
    private readonly StackLayout _mainLayout;
    
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
        _refreshButton = CreateButton("🔄", OnRefreshClicked, Colors.Green);
        _homeButton = CreateButton("🏠", OnHomeClicked, Colors.Orange);
        
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
            HorizontalOptions = LayoutOptions.FillAndExpand,
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
            VerticalOptions = LayoutOptions.FillAndExpand,
            BackgroundColor = Colors.White
        };
        
        // Подписываемся на события навигации
        _webView.Navigating += OnWebViewNavigating;
        _webView.Navigated += OnWebViewNavigated;

        // Создание layout
        var toolbarLayout = new HorizontalStackLayout
        {
            Spacing = 5,
            Padding = new Thickness(10, 5),
            HorizontalOptions = LayoutOptions.Fill,
            Children =
            {
                _backButton,
                _forwardButton,
                _refreshButton,
                _homeButton,
                new Frame
                {
                    Content = _urlEntry,
                    BackgroundColor = Colors.White,
                    BorderColor = Colors.LightGray,
                    CornerRadius = 8,
                    Padding = new Thickness(10, 0),
                    HasShadow = false,
                    HorizontalOptions = LayoutOptions.FillAndExpand,
                    VerticalOptions = LayoutOptions.Center
                },
                _goButton,
                _loadingIndicator
            }
        };

        var separator = new BoxView
        {
            HeightRequest = 1,
            BackgroundColor = Colors.LightGray,
            HorizontalOptions = LayoutOptions.Fill
        };

        var statusBar = new Frame
        {
            Content = _statusLabel,
            BackgroundColor = Color.FromHex("#F5F5F5"),
            Padding = new Thickness(10, 3),
            HasShadow = false,
            HorizontalOptions = LayoutOptions.Fill
        };

        _mainLayout = new StackLayout
        {
            Spacing = 0,
            Children =
            {
                toolbarLayout,
                separator,
                _webView,
                separator,
                statusBar
            }
        };

        Content = _mainLayout;
        
        // Загружаем домашнюю страницу
        AddToHistory(HomePage);
    }

    /// <summary>
    /// Создание стилизованной кнопки
    /// </summary>
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

    /// <summary>
    /// Обработка нажатия кнопки "Назад"
    /// </summary>
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

    /// <summary>
    /// Обработка нажатия кнопки "Вперёд"
    /// </summary>
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

    /// <summary>
    /// Обработка нажатия кнопки "Обновить"
    /// </summary>
    private void OnRefreshClicked(object? sender, EventArgs e)
    {
        _webView.Reload();
    }

    /// </summary>
    /// Обработка нажатия кнопки "Домой"
    /// </summary>
    private void OnHomeClicked(object? sender, EventArgs e)
    {
        NavigateToUrl(HomePage);
    }

    /// <summary>
    /// Обработка нажатия кнопки "Перейти"
    /// </summary>
    private void OnGoClicked(object? sender, EventArgs e)
    {
        var input = _urlEntry.Text?.Trim();
        if (string.IsNullOrWhiteSpace(input))
            return;

        var url = ProcessInput(input);
        NavigateToUrl(url);
    }

    /// <summary>
    /// Обработка фокуса на поле ввода URL
    /// </summary>
    private void OnUrlEntryFocused(object? sender, FocusEventArgs e)
    {
        // Выделяем весь текст при фокусе
        MainThread.BeginInvokeOnMainThread(() =>
        {
            _urlEntry.CursorPosition = 0;
            _urlEntry.SelectionLength = _urlEntry.Text?.Length ?? 0;
        });
    }

    /// <summary>
    /// Начало навигации WebView
    /// </summary>
    private void OnWebViewNavigating(object? sender, WebNavigatingEventArgs e)
    {
        _loadingIndicator.IsRunning = true;
        _loadingIndicator.IsVisible = true;
        _statusLabel.Text = $"Загрузка: {e.Url}";
        
        // Обновляем поле ввода URL
        _urlEntry.Text = e.Url;
        
        // Добавляем в историю (только если это не навигация по истории)
        if (!_isNavigating && e.Url != null)
        {
            AddToHistory(e.Url);
        }
        _isNavigating = false;
    }

    /// <summary>
    /// Завершение навигации WebView
    /// </summary>
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

    /// <summary>
    /// Навигация по URL
    /// </summary>
    private void NavigateToUrl(string url)
    {
        _urlEntry.Text = url;
        _webView.Source = url;
    }

    /// <summary>
    /// Обработка пользовательского ввода (URL или поисковый запрос)
    /// </summary>
    private string ProcessInput(string input)
    {
        // Если это похоже на URL
        if (input.StartsWith("http://", StringComparison.OrdinalIgnoreCase) ||
            input.StartsWith("https://", StringComparison.OrdinalIgnoreCase))
        {
            return input;
        }
        
        // Если это домен (содержит точку и не содержит пробелов)
        if (input.Contains('.') && !input.Contains(' '))
        {
            return $"https://{input}";
        }
        
        // Иначе это поисковый запрос
        return $"https://www.google.com/search?q={Uri.EscapeDataString(input)}";
    }

    /// <summary>
    /// Добавление URL в историю
    /// </summary>
    private void AddToHistory(string url)
    {
        // Удаляем все элементы после текущей позиции
        if (_historyIndex < _history.Count - 1)
        {
            _history.RemoveRange(_historyIndex + 1, _history.Count - _historyIndex - 1);
        }
        
        // Добавляем новый URL
        _history.Add(url);
        _historyIndex = _history.Count - 1;
        
        UpdateButtonsState();
    }

    /// <summary>
    /// Обновление состояния кнопок
    /// </summary>
    private void UpdateButtonsState()
    {
        _backButton.IsEnabled = _historyIndex > 0;
        _forwardButton.IsEnabled = _historyIndex < _history.Count - 1;
        
        _backButton.BackgroundColor = _backButton.IsEnabled ? Colors.DodgerBlue : Colors.Gray;
        _forwardButton.BackgroundColor = _forwardButton.IsEnabled ? Colors.DodgerBlue : Colors.Gray;
    }
}

// ============================================================================
// АЛЬТЕРНАТИВНАЯ ВЕРСИЯ - MauiProgram.cs (для запуска приложения)
// ============================================================================
/*
public static class MauiProgram
{
    public static MauiApp CreateMauiApp()
    {
        var builder = MauiApp.CreateBuilder();
        builder
            .UseMauiApp<App>()
            .ConfigureFonts(fonts =>
            {
                fonts.AddFont("OpenSans-Regular.ttf", "OpenSansRegular");
                fonts.AddFont("OpenSans-Semibold.ttf", "OpenSansSemibold");
            });

        return builder.Build();
    }
}

public class App : Application
{
    public App()
    {
        MainPage = new NavigationPage(new MainPage());
    }
}
*/

// ============================================================================
// ИНСТРУКЦИЯ ПО ЗАПУСКУ
// ============================================================================
/*
 * 
 * СПОСОБ 1: Создание нового проекта
 * -------------------------------
 * 1. dotnet new maui -n MauiBrowser
 * 2. cd MauiBrowser
 * 3. Замените содержимое MainPage.xaml.cs на этот код
 * 4. Убедитесь, что MainPage.xaml содержит:
 * 
 *    <?xml version="1.0" encoding="utf-8" ?>
 *    <ContentPage xmlns="http://schemas.microsoft.com/dotnet/2021/maui"
 *                 xmlns:x="http://schemas.microsoft.com/winfx/2009/xaml"
 *                 x:Class="MauiBrowser.MainPage">
 *    </ContentPage>
 * 
 * 5. dotnet build
 * 6. dotnet run (или F5 в Visual Studio)
 * 
 * 
 * СПОСОБ 2: Только C# (без XAML)
 * ------------------------------
 * Создайте проект и замените весь код на этот файл.
 * MAUI позволяет создавать UI полностью в C# коде!
 * 
 * 
 * ФУНКЦИИ БРАУЗЕРА:
 * ----------------
 * ✅ Навигация по URL
 * ✅ Поисковый запрос в Google
 * ✅ Кнопки: Назад, Вперёд, Обновить, Домой
 * ✅ История посещений
 * ✅ Индикатор загрузки
 * ✅ Статус-бар
 * ✅ Обработка ошибок
 * ✅ Автодополнение https://
 * 
 */
