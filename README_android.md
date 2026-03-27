# КГЭУ Расписание — Android App

## Структура проекта для Android Studio

```
app/
├── src/main/
│   ├── java/com/kgeu/schedule/
│   │   └── MainActivity.kt          ← Весь код приложения
│   ├── AndroidManifest.xml           ← Манифест
│   └── res/
│       └── values/
│           └── themes.xml            ← Использовать Theme.AppCompat.NoActionBar
├── build.gradle.kts                  ← Конфигурация сборки
```

## Как использовать

1. Откройте Android Studio
2. Создайте новый проект "Empty Activity" (без Compose)
3. Замените содержимое файлов:
   - `app/src/main/java/com/kgeu/schedule/MainActivity.kt` → `MainActivity.kt`
   - `app/src/main/AndroidManifest.xml` → `AndroidManifest.xml`
   - `app/build.gradle.kts` → `build.gradle.kts`
4. Синхронизируйте Gradle и запустите

## Функционал

- ✅ Поиск по группам, преподавателям, аудиториям
- ✅ 3 вкладки переключения (Группа / Преподаватель / Аудитория)
- ✅ Фильтры по факультетам и курсам
- ✅ Просмотр расписания по неделям
- ✅ Навигация вперёд/назад по неделям
- ✅ Кнопка "Сегодня"
- ✅ Подсветка текущего дня
- ✅ Цветовая маркировка типов занятий (лекция, практика, лаб, семинар)
- ✅ Статистика (количество групп, преподавателей, аудиторий)
- ✅ Быстрые ссылки на популярные группы
- ✅ Темная тема в стиле оригинального сайта
- ✅ API: kabinet.kgeu.ru с fallback через CORS-прокси

## Минимальные требования

- Android 7.0 (API 24) и выше
- Доступ в интернет
