package com.example.myapplication.ui.theme

import android.app.Activity
import android.os.Build
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.SideEffect
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.LocalView
import androidx.core.view.WindowCompat

// Telegram цветовая палитра - Светлая тема
private val TelegramLightColorScheme = lightColorScheme(
    primary = Color(0xFF0088CC),
    onPrimary = Color.White,
    primaryContainer = Color(0xFFE3F2FD),
    onPrimaryContainer = Color(0xFF001F2A),
    
    secondary = Color(0xFF229ED9),
    onSecondary = Color.White,
    secondaryContainer = Color(0xFFE1F5FE),
    onSecondaryContainer = Color(0xFF001F29),
    
    tertiary = Color(0xFF54C7FC),
    onTertiary = Color.White,
    tertiaryContainer = Color(0xFFB3E5FC),
    onTertiaryContainer = Color(0xFF001F28),
    
    error = Color(0xFFE53935),
    onError = Color.White,
    errorContainer = Color(0xFFFFEBEE),
    onErrorContainer = Color(0xFF410002),
    
    background = Color(0xFFF7F8FA),
    onBackground = Color(0xFF1D2733),
    
    surface = Color.White,
    onSurface = Color(0xFF1D2733),
    surfaceVariant = Color(0xFFF0F2F5),
    onSurfaceVariant = Color(0xFF707579),
    
    outline = Color(0xFFE7E8EA),
    outlineVariant = Color(0xFFCAC4D0),
    
    scrim = Color(0x80000000),
    inverseSurface = Color(0xFF1D2733),
    inverseOnSurface = Color(0xFFF7F8FA),
    inversePrimary = Color(0xFF54C7FC)
)

// Telegram цветовая палитра - Тёмная тема
private val TelegramDarkColorScheme = darkColorScheme(
    primary = Color(0xFF54C7FC),
    onPrimary = Color(0xFF003547),
    primaryContainer = Color(0xFF004D67),
    onPrimaryContainer = Color(0xFFB3E5FC),
    
    secondary = Color(0xFF64B5F6),
    onSecondary = Color(0xFF003548),
    secondaryContainer = Color(0xFF004D68),
    onSecondaryContainer = Color(0xFFB3E5FC),
    
    tertiary = Color(0xFF81D4FA),
    onTertiary = Color(0xFF003546),
    tertiaryContainer = Color(0xFF004D65),
    onTertiaryContainer = Color(0xFFB3E5FC),
    
    error = Color(0xFFFFB4AB),
    onError = Color(0xFF690005),
    errorContainer = Color(0xFF93000A),
    onErrorContainer = Color(0xFFFFDAD6),
    
    background = Color(0xFF17212B),
    onBackground = Color(0xFFE7E8EA),
    
    surface = Color(0xFF1E2C3A),
    onSurface = Color(0xFFE7E8EA),
    surfaceVariant = Color(0xFF242F3D),
    onSurfaceVariant = Color(0xFF9CA3AF),
    
    outline = Color(0xFF303940),
    outlineVariant = Color(0xFF404952),
    
    scrim = Color(0x80000000),
    inverseSurface = Color(0xFFE7E8EA),
    inverseOnSurface = Color(0xFF17212B),
    inversePrimary = Color(0xFF0088CC)
)

@Composable
fun MyApplicationTheme(
    darkTheme: Boolean = isSystemInDarkTheme(),
    content: @Composable () -> Unit
) {
    val colorScheme = if (darkTheme) TelegramDarkColorScheme else TelegramLightColorScheme

    val view = LocalView.current
    if (!view.isInEditMode) {
        SideEffect {
            val window = (view.context as Activity).window
            // Устанавливаем цвет статус бара
            window.statusBarColor = if (darkTheme) {
                Color(0xFF17212B).toArgb()
            } else {
                Color(0xFFF7F8FA).toArgb()
            }
            // Устанавливаем стиль иконок статус бара
            WindowCompat.getInsetsController(window, view).isAppearanceLightStatusBars = !darkTheme
        }
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}
