# Play Store Clone - Android App

Дизайн-клон Google Play Store на Kotlin с поддержкой загрузки изображений по URL.

## 📱 Скриншоты интерфейса

Приложение включает следующие экраны:
- Главный экран с баннерами, рекомендациями и топ-чартами
- Страница детальной информации о приложении
- Нижняя навигация (Игры, Приложения, Фильмы, Книги)

## 🏗 Структура проекта

```
PlayStoreClone/
├── build.gradle                    # Project-level Gradle
├── settings.gradle                 # Settings Gradle
├── app/
│   ├── build.gradle               # App-level Gradle с зависимостями
│   └── src/main/
│       ├── AndroidManifest.xml    # Манифест приложения
│       ├── java/com/example/playstoreclone/
│       │   ├── MainActivity.kt              # Главная Activity
│       │   ├── AppDetailActivity.kt         # Страница приложения
│       │   ├── adapters/
│       │   │   ├── AppHorizontalAdapter.kt  # Горизонтальный список приложений
│       │   │   ├── AppListAdapter.kt        # Вертикальный список (Top Charts)
│       │   │   ├── AppLargeAdapter.kt       # Большие карточки (Editor's Choice)
│       │   │   ├── CategoryAdapter.kt       # Адаптер категорий
│       │   │   ├── BannerAdapter.kt         # Адаптер баннеров
│       │   │   ├── ScreenshotAdapter.kt     # Адаптер скриншотов
│       │   │   └── ViewPagerAdapter.kt      # Адаптер для вкладок
│       │   ├── fragments/
│       │   │   └── HomeFragment.kt          # Фрагмент главной страницы
│       │   └── models/
│       │       ├── AppModel.kt              # Модель приложения
│       │       ├── CategoryModel.kt         # Модель категории
│       │       └── BannerModel.kt           # Модель баннера
│       └── res/
│           ├── layout/
│           │   ├── activity_main.xml        # Layout главной Activity
│           │   ├── activity_app_detail.xml  # Layout детальной страницы
│           │   ├── fragment_home.xml        # Layout главного фрагмента
│           │   ├── item_app_horizontal.xml  # Item горизонтального списка
│           │   ├── item_app_list.xml        # Item вертикального списка
│           │   ├── item_app_large.xml       # Item большой карточки
│           │   ├── item_category.xml        # Item категории
│           │   ├── item_banner.xml          # Item баннера
│           │   ├── item_screenshot.xml      # Item скриншота
│           │   └── item_rating_bar.xml      # Рейтинг бары
│           ├── drawable/
│           │   ├── bg_search_bar.xml        # Фон поиска
│           │   ├── bg_install_button.xml    # Кнопка установки
│           │   ├── bg_app_card.xml          # Фон карточки
│           │   ├── bg_banner.xml            # Фон баннера
│           │   ├── ic_*.xml                 # Векторные иконки
│           │   └── ...                      # Другие drawable
│           ├── menu/
│           │   ├── bottom_nav_menu.xml      # Меню нижней навигации
│           │   └── menu_app_detail.xml      # Меню детальной страницы
│           └── values/
│               ├── colors.xml               # Цвета (как в Google Play)
│               ├── strings.xml              # Строковые ресурсы
│               ├── themes.xml               # Темы и стили
│               └── dimens.xml               # Размеры
```

## 🚀 Как запустить

1. Откройте проект в Android Studio (Arctic Fox или новее)
2. Синхронизируйте Gradle файлы
3. Запустите на эмуляторе или устройстве (API 24+)

## 📦 Зависимости

- Material Design 3
- Glide - для загрузки изображений по URL
- ViewPager2 - для свайпа между вкладками
- CircleImageView - для аватаров
- Navigation Component

## 🎨 Особенности дизайна

- Современный Material Design 3
- Цветовая схема как в Google Play (зеленый акцент #01875F)
- Поддержка ViewBinding
- Загрузка изображений и иконок по URL через Glide
- Адаптивные карточки и списки

## 🔗 Загрузка изображений по URL

Для добавления иконки приложения по ссылке используйте поле `iconUrl` в `AppModel`:

```kotlin
AppModel(
    id = "1",
    name = "My App",
    iconUrl = "https://example.com/icon.png",  // URL иконки
    featuredImageUrl = "https://example.com/banner.png",  // URL баннера
    // ... другие поля
)
```

## 📝 Лицензия

MIT License - используйте свободно для обучения и разработки.
