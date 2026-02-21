package com.example.myapplication.ui.theme

import android.app.Activity
import android.os.Build
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.SideEffect
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.LocalView
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.sp
import androidx.core.view.WindowCompat

// Цвета Telegram
object TelegramColors {
    // Основные цвета фона
    val Background = Color(0xFF17212B)
    val DarkBackground = Color(0xFF0E1621)
    val DrawerBackground = Color(0xFF17212B)
    
    // Акцентный цвет
    val Accent = Color(0xFF5B8DEF)
    val AccentDark = Color(0xFF4A7BD4)
    val AccentLight = Color(0xFF6B9CFF)
    
    // Цвета сообщений
    val MyMessageBubble = Color(0xFF2B5278)
    val OtherMessageBubble = Color(0xFF182533)
    
    // Цвета текста
    val PrimaryText = Color.White
    val SecondaryText = Color(0xFF6C7883)
    val LinkText = Color(0xFF5B8DEF)
    
    // Цвета иконок
    val IconTint = Color(0xFF8E99A4)
    val IconActive = Color(0xFF5B8DEF)
    
    // Цвета статусов
    val Online = Color(0xFF4CAF50)
    val Offline = Color(0xFF6C7883)
    val Typing = Color(0xFF5B8DEF)
    
    // Цвета элементов
    val Divider = Color(0xFF2B3E50)
    val SelectedChat = Color(0xFF2B5278)
    val InputBackground = Color(0xFF242F3D)
    
    // Цвета уведомлений
    val UnreadBadge = Color(0xFF5B8DEF)
    val MutedBadge = Color(0xFF6C7883)
    val ErrorRed = Color(0xFFE53935)
    
    // Цвета Premium
    val Premium = Color(0xFFFFD700)
    val PremiumGradientStart = Color(0xFFFFD700)
    val PremiumGradientEnd = Color(0xFFFFA000)
    
    // Цвета для настроек
    val SettingsNotifications = Color(0xFFE53935)
    val SettingsPrivacy = Color(0xFF43A047)
    val SettingsData = Color(0xFF1E88E5)
    val SettingsAppearance = Color(0xFF8E24AA)
    val SettingsLanguage = Color(0xFFFF9800)
    val SettingsDevices = Color(0xFF00ACC1)
    val SettingsBattery = Color(0xFF00BCD4)
    val SettingsFolders = Color(0xFF7B1FA2)
    
    // Цвета для меню прикрепления
    val AttachGallery = Color(0xFF7C4DFF)
    val AttachFile = Color(0xFF00BFA5)
    val AttachLocation = Color(0xFFFF5252)
    val AttachPoll = Color(0xFFFFAB00)
    val AttachContact = Color(0xFF2196F3)
    val AttachMusic = Color(0xFFE91E63)
    
    // Цвета аватаров
    val AvatarRed = Color(0xFFEF5350)
    val AvatarPink = Color(0xFFEC407A)
    val AvatarPurple = Color(0xFFAB47BC)
    val AvatarDeepPurple = Color(0xFF7E57C2)
    val AvatarIndigo = Color(0xFF5C6BC0)
    val AvatarBlue = Color(0xFF42A5F5)
    val AvatarLightBlue = Color(0xFF29B6F6)
    val AvatarCyan = Color(0xFF26C6DA)
    val AvatarTeal = Color(0xFF26A69A)
    val AvatarGreen = Color(0xFF66BB6A)
    val AvatarLightGreen = Color(0xFF9CCC65)
    val AvatarLime = Color(0xFFD4E157)
    val AvatarYellow = Color(0xFFFFCA28)
    val AvatarAmber = Color(0xFFFFCA28)
    val AvatarOrange = Color(0xFFFF7043)
    val AvatarDeepOrange = Color(0xFFFF5722)
}

// Палитра цветов для тёмной темы
private val DarkColorScheme = darkColorScheme(
    primary = TelegramColors.Accent,
    onPrimary = Color.White,
    primaryContainer = TelegramColors.AccentDark,
    onPrimaryContainer = Color.White,
    secondary = TelegramColors.SecondaryText,
    onSecondary = Color.White,
    secondaryContainer = TelegramColors.InputBackground,
    onSecondaryContainer = Color.White,
    tertiary = TelegramColors.Online,
    onTertiary = Color.White,
    background = TelegramColors.DarkBackground,
    onBackground = Color.White,
    surface = TelegramColors.Background,
    onSurface = Color.White,
    surfaceVariant = TelegramColors.InputBackground,
    onSurfaceVariant = TelegramColors.SecondaryText,
    outline = TelegramColors.Divider,
    outlineVariant = TelegramColors.Divider,
    error = TelegramColors.ErrorRed,
    onError = Color.White
)

// Палитра цветов для светлой темы (при необходимости)
private val LightColorScheme = lightColorScheme(
    primary = Color(0xFF2196F3),
    onPrimary = Color.White,
    primaryContainer = Color(0xFFBBDEFB),
    onPrimaryContainer = Color(0xFF1565C0),
    secondary = Color(0xFF607D8B),
    onSecondary = Color.White,
    background = Color(0xFFF5F5F5),
    onBackground = Color(0xFF212121),
    surface = Color.White,
    onSurface = Color(0xFF212121),
    error = Color(0xFFE53935),
    onError = Color.White
)

// Типография
val TelegramTypography = Typography(
    // Заголовки
    displayLarge = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.Bold,
        fontSize = 57.sp,
        lineHeight = 64.sp,
        letterSpacing = (-0.25).sp,
        color = Color.White
    ),
    displayMedium = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.Bold,
        fontSize = 45.sp,
        lineHeight = 52.sp,
        letterSpacing = 0.sp,
        color = Color.White
    ),
    displaySmall = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.Bold,
        fontSize = 36.sp,
        lineHeight = 44.sp,
        letterSpacing = 0.sp,
        color = Color.White
    ),
    headlineLarge = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.SemiBold,
        fontSize = 32.sp,
        lineHeight = 40.sp,
        letterSpacing = 0.sp,
        color = Color.White
    ),
    headlineMedium = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.SemiBold,
        fontSize = 28.sp,
        lineHeight = 36.sp,
        letterSpacing = 0.sp,
        color = Color.White
    ),
    headlineSmall = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.SemiBold,
        fontSize = 24.sp,
        lineHeight = 32.sp,
        letterSpacing = 0.sp,
        color = Color.White
    ),
    
    // Заголовки среднего размера
    titleLarge = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.SemiBold,
        fontSize = 22.sp,
        lineHeight = 28.sp,
        letterSpacing = 0.sp,
        color = Color.White
    ),
    titleMedium = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.SemiBold,
        fontSize = 16.sp,
        lineHeight = 24.sp,
        letterSpacing = 0.15.sp,
        color = Color.White
    ),
    titleSmall = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.Medium,
        fontSize = 14.sp,
        lineHeight = 20.sp,
        letterSpacing = 0.1.sp,
        color = Color.White
    ),
    
    // Основной текст
    bodyLarge = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.Normal,
        fontSize = 16.sp,
        lineHeight = 24.sp,
        letterSpacing = 0.5.sp,
        color = Color.White
    ),
    bodyMedium = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.Normal,
        fontSize = 14.sp,
        lineHeight = 20.sp,
        letterSpacing = 0.25.sp,
        color = Color.White
    ),
    bodySmall = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.Normal,
        fontSize = 12.sp,
        lineHeight = 16.sp,
        letterSpacing = 0.4.sp,
        color = TelegramColors.SecondaryText
    ),
    
    // Метки
    labelLarge = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.Medium,
        fontSize = 14.sp,
        lineHeight = 20.sp,
        letterSpacing = 0.1.sp,
        color = Color.White
    ),
    labelMedium = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.Medium,
        fontSize = 12.sp,
        lineHeight = 16.sp,
        letterSpacing = 0.5.sp,
        color = Color.White
    ),
    labelSmall = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.Medium,
        fontSize = 11.sp,
        lineHeight = 16.sp,
        letterSpacing = 0.5.sp,
        color = TelegramColors.SecondaryText
    )
)

// Старая типография для совместимости
val Typography = Typography(
    bodyLarge = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.Normal,
        fontSize = 16.sp,
        lineHeight = 24.sp,
        letterSpacing = 0.5.sp
    ),
    titleLarge = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.SemiBold,
        fontSize = 22.sp,
        lineHeight = 28.sp,
        letterSpacing = 0.sp
    ),
    labelSmall = TextStyle(
        fontFamily = FontFamily.Default,
        fontWeight = FontWeight.Medium,
        fontSize = 11.sp,
        lineHeight = 16.sp,
        letterSpacing = 0.5.sp
    )
)

@Composable
fun MyApplicationTheme(
    darkTheme: Boolean = true, // Telegram всегда в тёмной теме по умолчанию
    dynamicColor: Boolean = false, // Отключаем динамические цвета для Telegram-стиля
    content: @Composable () -> Unit
) {
    val colorScheme = when {
        dynamicColor && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S -> {
            // Динамические цвета для Android 12+
            if (darkTheme) DarkColorScheme else LightColorScheme
        }
        darkTheme -> DarkColorScheme
        else -> LightColorScheme
    }

    val view = LocalView.current
    if (!view.isInEditMode) {
        SideEffect {
            val window = (view.context as Activity).window
            // Устанавливаем цвет статус-бара под фон приложения
            window.statusBarColor = TelegramColors.Background.toArgb()
            // Устанавливаем цвет навигационной панели
            window.navigationBarColor = TelegramColors.Background.toArgb()
            // Настраиваем цвет иконок статус-бара
            WindowCompat.getInsetsController(window, view).isAppearanceLightStatusBars = !darkTheme
            WindowCompat.getInsetsController(window, view).isAppearanceLightNavigationBars = !darkTheme
        }
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = TelegramTypography,
        content = content
    )
}

// Дополнительные утилиты для работы с цветами
object ColorUtils {
    /**
     * Генерирует случайный цвет аватара из палитры Telegram
     */
    fun getAvatarColor(seed: Int): Color {
        val colors = listOf(
            TelegramColors.AvatarRed,
            TelegramColors.AvatarPink,
            TelegramColors.AvatarPurple,
            TelegramColors.AvatarDeepPurple,
            TelegramColors.AvatarIndigo,
            TelegramColors.AvatarBlue,
            TelegramColors.AvatarLightBlue,
            TelegramColors.AvatarCyan,
            TelegramColors.AvatarTeal,
            TelegramColors.AvatarGreen,
            TelegramColors.AvatarLightGreen,
            TelegramColors.AvatarOrange,
            TelegramColors.AvatarDeepOrange
        )
        return colors[seed.mod(colors.size)]
    }
    
    /**
     * Получает инициалы из имени
     */
    fun getInitials(name: String): String {
        return name.split(" ")
            .take(2)
            .mapNotNull { it.firstOrNull()?.uppercaseChar() }
            .joinToString("")
    }
    
    /**
     * Определяет, нужен ли светлый текст на данном фоне
     */
    fun needsLightText(backgroundColor: Color): Boolean {
        val luminance = (0.299 * backgroundColor.red + 
                        0.587 * backgroundColor.green + 
                        0.114 * backgroundColor.blue)
        return luminance < 0.5
    }
}

// Размеры для Telegram UI
object TelegramDimens {
    // Аватары
    val AvatarSizeSmall = 36.dp
    val AvatarSizeMedium = 44.dp
    val AvatarSizeLarge = 56.dp
    val AvatarSizeXLarge = 70.dp
    val AvatarSizeProfile = 100.dp
    
    // Отступы
    val PaddingTiny = 4.dp
    val PaddingSmall = 8.dp
    val PaddingMedium = 12.dp
    val PaddingLarge = 16.dp
    val PaddingXLarge = 24.dp
    
    // Скругления
    val CornerRadiusSmall = 4.dp
    val CornerRadiusMedium = 8.dp
    val CornerRadiusLarge = 12.dp
    val CornerRadiusXLarge = 18.dp
    val CornerRadiusFull = 100.dp
    
    // Размеры элементов
    val IconSizeSmall = 16.dp
    val IconSizeMedium = 24.dp
    val IconSizeLarge = 32.dp
    
    val ButtonHeight = 48.dp
    val InputFieldHeight = 56.dp
    
    // Размеры сообщений
    val MessageMaxWidth = 320.dp
    val MessageBubbleCorner = 18.dp
    val MessageBubbleCornerTail = 4.dp
    
    // Бейджи
    val BadgeMinWidth = 24.dp
    val BadgeHeight = 24.dp
    
    // Drawer
    val DrawerWidth = 300.dp
    val DrawerHeaderHeight = 190.dp
}

// Единицы измерения
private val Int.dp: androidx.compose.ui.unit.Dp
    get() = androidx.compose.ui.unit.Dp(this.toFloat())
