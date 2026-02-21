package com.example.myapplication.ui.theme

import android.app.Activity
import android.os.Build
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.dynamicDarkColorScheme
import androidx.compose.material3.dynamicLightColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.SideEffect
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalView
import androidx.core.view.WindowCompat

/**
 * Цвета в стиле Telegram с улучшенными градиентами и оттенками
 */
object TelegramColors {
    // === ОСНОВНЫЕ ЦВЕТА ===
    val Accent = Color(0xFF5B8DEF)                      // Фирменный синий Telegram
    val AccentLight = Color(0xFF7AA3F5)                 // Светлый акцент
    val AccentDark = Color(0xFF3D7BF7)                  // Тёмный акцент
    
    // === ФОН ===
    val Background = Color(0xFF17212B)                  // Основной фон (темная тема)
    val DarkBackground = Color(0xFF0E1621)              // Более тёмный фон
    val ChatBackground = Color(0xFF0D1117)              // Фон чата
    val DrawerBackground = Color(0xFF17212B)            // Фон боковой панели
    val InputBackground = Color(0xFF242F3D)             // Фон поля ввода
    val SelectedChat = Color(0xFF2B5278)                // Выбранный чат
    val HoverBackground = Color(0xFF1E2A36)             // При наведении
    
    // === СООБЩЕНИЯ ===
    val MyMessageBubble = Color(0xFF2B5278)             // Мои сообщения (начало градиента)
    val MyMessageBubbleEnd = Color(0xFF1E3A5F)          // Мои сообщения (конец градиента)
    val OtherMessageBubble = Color(0xFF182533)          // Сообщения собеседника
    val OtherMessageBubbleHighlight = Color(0xFF1E2D3D) // Подсветка
    
    // === ТЕКСТ ===
    val TextPrimary = Color(0xFFFFFFFF)                 // Основной текст
    val TextSecondary = Color(0xFF6C7883)               // Вторичный текст
    val TextTertiary = Color(0xFF4A5568)                // Третичный текст
    val TextAccent = Color(0xFF5B8DEF)                  // Акцентный текст
    val TextLink = Color(0xFF58A6FF)                    // Ссылки
    
    // === ИКОНКИ ===
    val IconTint = Color(0xFFADBBC6)                    // Обычные иконки
    val IconActive = Color(0xFF5B8DEF)                  // Активные иконки
    val IconMuted = Color(0xFF6C7883)                   // Приглушённые иконки
    
    // === СТАТУСЫ ===
    val Online = Color(0xFF4FC36D)                      // В сети (Telegram green)
    val Typing = Color(0xFF5B8DEF)                      // Печатает
    val Recording = Color(0xFFE53935)                   // Записывает голос
    val Sending = Color(0xFFFFAB00)                     // Отправляется
    
    // === БЕЙДЖИ ===
    val BadgeUnread = Color(0xFF5B8DEF)                 // Непрочитанные
    val BadgeMuted = Color(0xFF6C7883)                  // С отключенными уведомлениями
    val BadgeMention = Color(0xFF5B8DEF)                // Упоминания
    val BadgePinned = Color(0xFF6C7883)                 // Закреплённые
    
    // === РАЗДЕЛИТЕЛИ ===
    val Divider = Color(0xFF0E1621)                     // Разделительные линии
    val DividerLight = Color(0xFF242F3D)                // Светлые разделители
    
    // === PREMIUM ===
    val Premium = Color(0xFFFFD700)                     // Золотой цвет Premium
    val PremiumStart = Color(0xFFFFD700)                // Градиент Premium (начало)
    val PremiumEnd = Color(0xFFFFA000)                  // Градиент Premium (конец)
    
    // === STORIES ===
    val StoryUnseenStart = Color(0xFF7C4DFF)            // Непросмотренные истории
    val StoryUnseenMid = Color(0xFF448AFF)
    val StoryUnseenEnd = Color(0xFF00BFA5)
    val StorySeen = Color(0xFF4A5568)                   // Просмотренные истории
    
    // === РЕАКЦИИ ===
    val ReactionSelected = Color(0xFF5B8DEF)            // Выбранная реакция
    val ReactionBackground = Color(0xFF242F3D)          // Фон реакции
    
    // === ВЕРИФИКАЦИЯ ===
    val Verified = Color(0xFF5B8DEF)                    // Верифицированные аккаунты
    val VerifiedPremium = Color(0xFFFFD700)             // Верифицированные Premium
    
    // === ОШИБКИ И ПРЕДУПРЕЖДЕНИЯ ===
    val Error = Color(0xFFE53935)                       // Ошибки
    val Warning = Color(0xFFFFAB00)                     // Предупреждения
    val Success = Color(0xFF4CAF50)                     // Успех
    val Info = Color(0xFF2196F3)                        // Информация
    
    // === МЕДИА ===
    val MediaOverlay = Color(0x99000000)                // Оверлей на медиа
    val MediaPlayButton = Color(0xFFFFFFFF)             // Кнопка воспроизведения
    val VoiceWaveform = Color(0xFF5B8DEF)               // Волна голосового сообщения
    val VoiceWaveformInactive = Color(0xFF4A5568)       // Неактивная волна
    
    // === КНОПКИ ===
    val ButtonPrimary = Color(0xFF5B8DEF)               // Основная кнопка
    val ButtonSecondary = Color(0xFF242F3D)             // Вторичная кнопка
    val ButtonDestructive = Color(0xFFE53935)           // Деструктивная кнопка
    val ButtonDisabled = Color(0xFF4A5568)              // Отключенная кнопка
    
    // === ГРАДИЕНТЫ ДЛЯ АВАТАРОВ ===
    object AvatarGradients {
        val Red = listOf(Color(0xFFFF5252), Color(0xFFE53935))
        val Orange = listOf(Color(0xFFFFAB40), Color(0xFFFF9100))
        val Yellow = listOf(Color(0xFFFFEA00), Color(0xFFFFD600))
        val Green = listOf(Color(0xFF69F0AE), Color(0xFF00E676))
        val Cyan = listOf(Color(0xFF64FFDA), Color(0xFF1DE9B6))
        val Blue = listOf(Color(0xFF448AFF), Color(0xFF2979FF))
        val Purple = listOf(Color(0xFFE040FB), Color(0xFFD500F9))
        val Pink = listOf(Color(0xFFFF4081), Color(0xFFF50057))
        val Indigo = listOf(Color(0xFF536DFE), Color(0xFF3D5AFE))
        val Teal = listOf(Color(0xFF64FFDA), Color(0xFF1DE9B6))
        
        val All = listOf(Red, Orange, Yellow, Green, Cyan, Blue, Purple, Pink, Indigo, Teal)
        
        fun getGradient(index: Int): List<Color> {
            return All[index % All.size]
        }
    }
    
    // === ГРАДИЕНТЫ ДЛЯ ШАПКИ ===
    object HeaderGradients {
        val Default = listOf(Color(0xFF667EEA), Color(0xFF764BA2))
        val Blue = listOf(Color(0xFF2193B0), Color(0xFF6DD5ED))
        val Purple = listOf(Color(0xFF834D9B), Color(0xFFD04ED6))
        val Green = listOf(Color(0xFF11998E), Color(0xFF38EF7D))
        val Orange = listOf(Color(0xFFFF8008), Color(0xFFFFC837))
        val Pink = listOf(Color(0xFFEC008C), Color(0xFFFC6767))
        val Dark = listOf(Color(0xFF0F2027), Color(0xFF203A43), Color(0xFF2C5364))
    }
}

// Тёмная цветовая схема Material3
private val DarkColorScheme = darkColorScheme(
    primary = TelegramColors.Accent,
    onPrimary = Color.White,
    primaryContainer = TelegramColors.SelectedChat,
    onPrimaryContainer = Color.White,
    secondary = TelegramColors.AccentLight,
    onSecondary = Color.White,
    secondaryContainer = TelegramColors.InputBackground,
    onSecondaryContainer = Color.White,
    tertiary = TelegramColors.Online,
    onTertiary = Color.White,
    error = TelegramColors.Error,
    onError = Color.White,
    errorContainer = TelegramColors.Error.copy(alpha = 0.2f),
    onErrorContainer = TelegramColors.Error,
    background = TelegramColors.Background,
    onBackground = Color.White,
    surface = TelegramColors.Background,
    onSurface = Color.White,
    surfaceVariant = TelegramColors.InputBackground,
    onSurfaceVariant = TelegramColors.TextSecondary,
    outline = TelegramColors.Divider,
    outlineVariant = TelegramColors.DividerLight,
    scrim = Color.Black.copy(alpha = 0.5f),
    inverseSurface = Color.White,
    inverseOnSurface = TelegramColors.Background,
    inversePrimary = TelegramColors.AccentDark,
    surfaceTint = TelegramColors.Accent
)

// Светлая цветовая схема (если потребуется)
private val LightColorScheme = lightColorScheme(
    primary = Color(0xFF2AABEE),
    onPrimary = Color.White,
    primaryContainer = Color(0xFFD1E4FF),
    onPrimaryContainer = Color(0xFF001D36),
    secondary = Color(0xFF54B9E8),
    onSecondary = Color.White,
    background = Color(0xFFF5F5F5),
    onBackground = Color(0xFF1A1A1A),
    surface = Color.White,
    onSurface = Color(0xFF1A1A1A),
    surfaceVariant = Color(0xFFE7E7E7),
    onSurfaceVariant = Color(0xFF5A5A5A),
    outline = Color(0xFFD0D0D0),
    error = Color(0xFFE53935),
    onError = Color.White
)

@Composable
fun MyApplicationTheme(
    darkTheme: Boolean = true, // Telegram по умолчанию тёмная
    dynamicColor: Boolean = false, // Отключаем Dynamic Color для сохранения фирменного стиля
    content: @Composable () -> Unit
) {
    val colorScheme = when {
        dynamicColor && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S -> {
            val context = LocalContext.current
            if (darkTheme) dynamicDarkColorScheme(context) else dynamicLightColorScheme(context)
        }
        darkTheme -> DarkColorScheme
        else -> LightColorScheme
    }
    
    val view = LocalView.current
    if (!view.isInEditMode) {
        SideEffect {
            val window = (view.context as Activity).window
            // Устанавливаем цвет статус-бара
            window.statusBarColor = TelegramColors.Background.toArgb()
            // Устанавливаем цвет навигационной панели
            window.navigationBarColor = TelegramColors.Background.toArgb()
            // Настраиваем иконки статус-бара
            WindowCompat.getInsetsController(window, view).isAppearanceLightStatusBars = !darkTheme
            WindowCompat.getInsetsController(window, view).isAppearanceLightNavigationBars = !darkTheme
        }
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}
