
╔══════════════════════════════════════════════════════════════════════════════╗
║                     MESSENGER APP - KOTLIN JETPACK COMPOSE                    ║
║                          TELEGRAM-STYLE DESIGN                                ║
╚══════════════════════════════════════════════════════════════════════════════╝

================================================================================
                              СТРУКТУРА ПРОЕКТА
================================================================================

MessengerApp/
├── app/
│   ├── src/
│   │   ├── main/
│   │   │   ├── java/com/messenger/app/
│   │   │   │   ├── ui/
│   │   │   │   │   ├── theme/
│   │   │   │   │   │   ├── Color.kt
│   │   │   │   │   │   ├── Theme.kt
│   │   │   │   │   │   └── Type.kt
│   │   │   │   │   ├── screens/
│   │   │   │   │   │   ├── auth/
│   │   │   │   │   │   │   ├── LoginScreen.kt
│   │   │   │   │   │   │   └── RegisterScreen.kt
│   │   │   │   │   │   ├── main/
│   │   │   │   │   │   │   └── MainScreen.kt
│   │   │   │   │   │   └── components/
│   │   │   │   │   │       ├── ChatItem.kt
│   │   │   │   │   │       ├── CustomTextField.kt
│   │   │   │   │   │       ├── GradientButton.kt
│   │   │   │   │   │       └── Avatar.kt
│   │   │   │   │   └── navigation/
│   │   │   │   │       └── NavGraph.kt
│   │   │   │   ├── data/
│   │   │   │   │   └── model/
│   │   │   │   │       ├── User.kt
│   │   │   │   │       └── Chat.kt
│   │   │   │   └── MainActivity.kt
│   │   │   └── res/
│   ├── build.gradle.kts
│   └── proguard-rules.pro
├── build.gradle.kts
├── settings.gradle.kts
└── gradle.properties

================================================================================

================================================================================
                              THEME - Color.kt
================================================================================

package com.messenger.app.ui.theme

import androidx.compose.ui.graphics.Color

// Primary Colors - Telegram Style
val TelegramBlue = Color(0xFF0088CC)
val TelegramBlueDark = Color(0xFF006699)
val TelegramBlueLight = Color(0xFF54A9EB)

// Gradient Colors
val GradientStart = Color(0xFF6B8DD6)
val GradientEnd = Color(0xFF8E37D7)

// Light Theme
val LightBackground = Color(0xFFFFFFFF)
val LightSurface = Color(0xFFF7F7F7)
val LightSurfaceVariant = Color(0xFFEFEFEF)
val LightOnBackground = Color(0xFF000000)
val LightOnSurface = Color(0xFF1A1A1A)
val LightOnSurfaceVariant = Color(0xFF707579)

// Dark Theme
val DarkBackground = Color(0xFF17212B)
val DarkSurface = Color(0xFF232E3C)
val DarkSurfaceVariant = Color(0xFF2B3A4D)
val DarkOnBackground = Color(0xFFFFFFFF)
val DarkOnSurface = Color(0xFFFFFFFF)
val DarkOnSurfaceVariant = Color(0xFF8B9198)

// Additional Colors
val Green = Color(0xFF4DCD5E)
val GreenOnline = Color(0xFF4AC959)
val Red = Color(0xFFE53935)
val Orange = Color(0xFFFF9800)
val Gray = Color(0xFF999999)
val LightGray = Color(0xFFDADADA)
val DividerColor = Color(0xFFE0E0E0)

// Chat Colors
val MessageOutgoing = Color(0xFFEFFEDD)
val MessageOutgoingDark = Color(0xFF2B5278)
val MessageIncoming = Color(0xFFFFFFFF)
val MessageIncomingDark = Color(0xFF182533)

// Unread Badge
val UnreadBadge = Color(0xFF4CAF50)
val MutedBadge = Color(0xFF8B9198)



================================================================================
                              THEME - Type.kt
================================================================================

package com.messenger.app.ui.theme

import androidx.compose.material3.Typography
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.Font
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.sp

// Используем системный шрифт, но можно добавить кастомный
val MessengerFontFamily = FontFamily.Default

val Typography = Typography(
    // Заголовки
    displayLarge = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.Bold,
        fontSize = 32.sp,
        lineHeight = 40.sp,
        letterSpacing = (-0.25).sp
    ),
    displayMedium = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.Bold,
        fontSize = 28.sp,
        lineHeight = 36.sp
    ),
    displaySmall = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.Bold,
        fontSize = 24.sp,
        lineHeight = 32.sp
    ),
    
    // Headlines
    headlineLarge = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.SemiBold,
        fontSize = 22.sp,
        lineHeight = 28.sp
    ),
    headlineMedium = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.SemiBold,
        fontSize = 20.sp,
        lineHeight = 26.sp
    ),
    headlineSmall = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.SemiBold,
        fontSize = 18.sp,
        lineHeight = 24.sp
    ),
    
    // Title
    titleLarge = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.Medium,
        fontSize = 17.sp,
        lineHeight = 22.sp
    ),
    titleMedium = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.Medium,
        fontSize = 16.sp,
        lineHeight = 21.sp
    ),
    titleSmall = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.Medium,
        fontSize = 15.sp,
        lineHeight = 20.sp
    ),
    
    // Body
    bodyLarge = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.Normal,
        fontSize = 16.sp,
        lineHeight = 22.sp
    ),
    bodyMedium = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.Normal,
        fontSize = 15.sp,
        lineHeight = 20.sp
    ),
    bodySmall = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.Normal,
        fontSize = 14.sp,
        lineHeight = 18.sp
    ),
    
    // Label
    labelLarge = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.Medium,
        fontSize = 14.sp,
        lineHeight = 18.sp
    ),
    labelMedium = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.Medium,
        fontSize = 12.sp,
        lineHeight = 16.sp
    ),
    labelSmall = TextStyle(
        fontFamily = MessengerFontFamily,
        fontWeight = FontWeight.Medium,
        fontSize = 11.sp,
        lineHeight = 14.sp
    )
)



================================================================================
                              THEME - Theme.kt
================================================================================

package com.messenger.app.ui.theme

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

private val LightColorScheme = lightColorScheme(
    primary = TelegramBlue,
    onPrimary = Color.White,
    primaryContainer = TelegramBlueLight,
    onPrimaryContainer = Color.White,
    secondary = GradientEnd,
    onSecondary = Color.White,
    background = LightBackground,
    onBackground = LightOnBackground,
    surface = LightSurface,
    onSurface = LightOnSurface,
    surfaceVariant = LightSurfaceVariant,
    onSurfaceVariant = LightOnSurfaceVariant,
    outline = LightGray,
    error = Red,
    onError = Color.White
)

private val DarkColorScheme = darkColorScheme(
    primary = TelegramBlue,
    onPrimary = Color.White,
    primaryContainer = TelegramBlueDark,
    onPrimaryContainer = Color.White,
    secondary = GradientEnd,
    onSecondary = Color.White,
    background = DarkBackground,
    onBackground = DarkOnBackground,
    surface = DarkSurface,
    onSurface = DarkOnSurface,
    surfaceVariant = DarkSurfaceVariant,
    onSurfaceVariant = DarkOnSurfaceVariant,
    outline = DarkSurfaceVariant,
    error = Red,
    onError = Color.White
)

@Composable
fun MessengerTheme(
    darkTheme: Boolean = isSystemInDarkTheme(),
    content: @Composable () -> Unit
) {
    val colorScheme = if (darkTheme) DarkColorScheme else LightColorScheme
    
    val view = LocalView.current
    if (!view.isInEditMode) {
        SideEffect {
            val window = (view.context as Activity).window
            window.statusBarColor = colorScheme.background.toArgb()
            WindowCompat.getInsetsController(window, view).isAppearanceLightStatusBars = !darkTheme
        }
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}



================================================================================
                              DATA MODEL - User.kt
================================================================================

package com.messenger.app.data.model

data class User(
    val id: String,
    val firstName: String,
    val lastName: String,
    val username: String,
    val phone: String,
    val avatarUrl: String? = null,
    val isOnline: Boolean = false,
    val lastSeen: Long = 0
) {
    val fullName: String
        get() = "$firstName $lastName".trim()
        
    val initials: String
        get() = "${firstName.firstOrNull() ?: ""}${lastName.firstOrNull() ?: ""}".uppercase()
}



================================================================================
                              DATA MODEL - Chat.kt
================================================================================

package com.messenger.app.data.model

data class Chat(
    val id: String,
    val name: String,
    val avatarUrl: String? = null,
    val lastMessage: String,
    val lastMessageTime: Long,
    val unreadCount: Int = 0,
    val isOnline: Boolean = false,
    val isMuted: Boolean = false,
    val isPinned: Boolean = false,
    val isVerified: Boolean = false,
    val isGroup: Boolean = false,
    val isChannel: Boolean = false,
    val typing: String? = null, // "печатает..." или null
    val draft: String? = null
) {
    val initials: String
        get() = name.split(" ")
            .take(2)
            .mapNotNull { it.firstOrNull()?.uppercase() }
            .joinToString("")
}

// Пример данных для превью
object SampleData {
    val chats = listOf(
        Chat(
            id = "1",
            name = "Telegram",
            lastMessage = "Добро пожаловать в Telegram!",
            lastMessageTime = System.currentTimeMillis() - 60000,
            isVerified = true,
            isChannel = true
        ),
        Chat(
            id = "2",
            name = "Александр Иванов",
            lastMessage = "Привет! Как дела?",
            lastMessageTime = System.currentTimeMillis() - 120000,
            unreadCount = 3,
            isOnline = true
        ),
        Chat(
            id = "3",
            name = "Мария Петрова",
            lastMessage = "Увидимся завтра 👋",
            lastMessageTime = System.currentTimeMillis() - 3600000,
            typing = "печатает..."
        ),
        Chat(
            id = "4",
            name = "Рабочий чат",
            lastMessage = "Коллеги, напоминаю о встрече",
            lastMessageTime = System.currentTimeMillis() - 7200000,
            unreadCount = 12,
            isGroup = true,
            isPinned = true
        ),
        Chat(
            id = "5",
            name = "Дмитрий Козлов",
            lastMessage = "Отправил документы",
            lastMessageTime = System.currentTimeMillis() - 86400000,
            isMuted = true
        ),
        Chat(
            id = "6",
            name = "Семья",
            lastMessage = "Мама: Позвони когда сможешь",
            lastMessageTime = System.currentTimeMillis() - 172800000,
            unreadCount = 1,
            isGroup = true
        ),
        Chat(
            id = "7",
            name = "Новости Tech",
            lastMessage = "Apple представила новый iPhone",
            lastMessageTime = System.currentTimeMillis() - 259200000,
            isChannel = true,
            isVerified = true
        ),
        Chat(
            id = "8",
            name = "Анна Смирнова",
            lastMessage = "Спасибо за помощь!",
            lastMessageTime = System.currentTimeMillis() - 345600000,
            draft = "Привет, хотел спросить..."
        )
    )
}



================================================================================
                              COMPONENTS - CustomTextField.kt
================================================================================

package com.messenger.app.ui.screens.components

import androidx.compose.animation.animateColorAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.interaction.collectIsFocusedAsState
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.BasicTextField
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.focus.FocusRequester
import androidx.compose.ui.focus.focusRequester
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.SolidColor
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.input.VisualTransformation
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.unit.dp
import com.messenger.app.ui.theme.*

@Composable
fun MessengerTextField(
    value: String,
    onValueChange: (String) -> Unit,
    modifier: Modifier = Modifier,
    label: String = "",
    placeholder: String = "",
    leadingIcon: ImageVector? = null,
    trailingIcon: @Composable (() -> Unit)? = null,
    isPassword: Boolean = false,
    isError: Boolean = false,
    errorMessage: String? = null,
    keyboardType: KeyboardType = KeyboardType.Text,
    imeAction: ImeAction = ImeAction.Next,
    onImeAction: () -> Unit = {},
    singleLine: Boolean = true,
    enabled: Boolean = true
) {
    val interactionSource = remember { MutableInteractionSource() }
    val isFocused by interactionSource.collectIsFocusedAsState()
    
    val borderColor by animateColorAsState(
        targetValue = when {
            isError -> Red
            isFocused -> TelegramBlue
            else -> Color.Transparent
        },
        animationSpec = tween(200),
        label = "borderColor"
    )
    
    val backgroundColor = MaterialTheme.colorScheme.surfaceVariant
    
    Column(modifier = modifier) {
        if (label.isNotEmpty()) {
            Text(
                text = label,
                style = MaterialTheme.typography.labelMedium,
                color = if (isError) Red else MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.padding(bottom = 8.dp, start = 4.dp)
            )
        }
        
        BasicTextField(
            value = value,
            onValueChange = onValueChange,
            modifier = Modifier
                .fillMaxWidth()
                .height(56.dp)
                .clip(RoundedCornerShape(12.dp))
                .background(backgroundColor)
                .border(
                    width = 2.dp,
                    color = borderColor,
                    shape = RoundedCornerShape(12.dp)
                ),
            textStyle = MaterialTheme.typography.bodyLarge.copy(
                color = MaterialTheme.colorScheme.onSurface
            ),
            singleLine = singleLine,
            enabled = enabled,
            interactionSource = interactionSource,
            cursorBrush = SolidColor(TelegramBlue),
            visualTransformation = if (isPassword) PasswordVisualTransformation() else VisualTransformation.None,
            keyboardOptions = KeyboardOptions(
                keyboardType = keyboardType,
                imeAction = imeAction
            ),
            keyboardActions = KeyboardActions(
                onDone = { onImeAction() },
                onNext = { onImeAction() },
                onSearch = { onImeAction() }
            ),
            decorationBox = { innerTextField ->
                Row(
                    modifier = Modifier
                        .fillMaxSize()
                        .padding(horizontal = 16.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    if (leadingIcon != null) {
                        Icon(
                            imageVector = leadingIcon,
                            contentDescription = null,
                            tint = if (isFocused) TelegramBlue else MaterialTheme.colorScheme.onSurfaceVariant,
                            modifier = Modifier.size(22.dp)
                        )
                        Spacer(modifier = Modifier.width(12.dp))
                    }
                    
                    Box(modifier = Modifier.weight(1f)) {
                        if (value.isEmpty()) {
                            Text(
                                text = placeholder,
                                style = MaterialTheme.typography.bodyLarge,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                        innerTextField()
                    }
                    
                    if (trailingIcon != null) {
                        Spacer(modifier = Modifier.width(12.dp))
                        trailingIcon()
                    }
                }
            }
        )
        
        if (isError && !errorMessage.isNullOrEmpty()) {
            Text(
                text = errorMessage,
                style = MaterialTheme.typography.labelSmall,
                color = Red,
                modifier = Modifier.padding(top = 4.dp, start = 4.dp)
            )
        }
    }
}

@Composable
fun SearchTextField(
    value: String,
    onValueChange: (String) -> Unit,
    modifier: Modifier = Modifier,
    placeholder: String = "Поиск",
    onSearch: () -> Unit = {}
) {
    val backgroundColor = MaterialTheme.colorScheme.surfaceVariant
    
    BasicTextField(
        value = value,
        onValueChange = onValueChange,
        modifier = modifier
            .fillMaxWidth()
            .height(40.dp)
            .clip(RoundedCornerShape(20.dp))
            .background(backgroundColor),
        textStyle = MaterialTheme.typography.bodyMedium.copy(
            color = MaterialTheme.colorScheme.onSurface
        ),
        singleLine = true,
        cursorBrush = SolidColor(TelegramBlue),
        keyboardOptions = KeyboardOptions(
            keyboardType = KeyboardType.Text,
            imeAction = ImeAction.Search
        ),
        keyboardActions = KeyboardActions(onSearch = { onSearch() }),
        decorationBox = { innerTextField ->
            Row(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(horizontal = 14.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Icon(
                    imageVector = androidx.compose.material.icons.Icons.Outlined.Search,
                    contentDescription = "Search",
                    tint = MaterialTheme.colorScheme.onSurfaceVariant,
                    modifier = Modifier.size(20.dp)
                )
                Spacer(modifier = Modifier.width(10.dp))
                Box(modifier = Modifier.weight(1f)) {
                    if (value.isEmpty()) {
                        Text(
                            text = placeholder,
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                    innerTextField()
                }
            }
        }
    )
}



================================================================================
                              COMPONENTS - GradientButton.kt
================================================================================

package com.messenger.app.ui.screens.components

import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.interaction.collectIsPressedAsState
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.ripple.rememberRipple
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.scale
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.messenger.app.ui.theme.*

@Composable
fun GradientButton(
    text: String,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
    enabled: Boolean = true,
    isLoading: Boolean = false,
    gradientColors: List<Color> = listOf(TelegramBlue, TelegramBlueDark)
) {
    val interactionSource = remember { MutableInteractionSource() }
    val isPressed by interactionSource.collectIsPressedAsState()
    
    val scale by animateFloatAsState(
        targetValue = if (isPressed) 0.96f else 1f,
        animationSpec = spring(
            dampingRatio = Spring.DampingRatioMediumBouncy,
            stiffness = Spring.StiffnessLow
        ),
        label = "scale"
    )
    
    val alpha = if (enabled) 1f else 0.5f
    
    Box(
        modifier = modifier
            .scale(scale)
            .shadow(
                elevation = if (isPressed) 4.dp else 8.dp,
                shape = RoundedCornerShape(12.dp),
                ambientColor = TelegramBlue.copy(alpha = 0.3f),
                spotColor = TelegramBlue.copy(alpha = 0.3f)
            )
            .clip(RoundedCornerShape(12.dp))
            .background(
                brush = Brush.horizontalGradient(gradientColors),
                alpha = alpha
            )
            .clickable(
                interactionSource = interactionSource,
                indication = rememberRipple(color = Color.White.copy(alpha = 0.3f)),
                enabled = enabled && !isLoading,
                onClick = onClick
            )
            .height(56.dp),
        contentAlignment = Alignment.Center
    ) {
        if (isLoading) {
            CircularProgressIndicator(
                modifier = Modifier.size(24.dp),
                color = Color.White,
                strokeWidth = 2.dp
            )
        } else {
            Text(
                text = text,
                style = MaterialTheme.typography.titleMedium.copy(
                    fontWeight = FontWeight.SemiBold,
                    fontSize = 16.sp
                ),
                color = Color.White
            )
        }
    }
}

@Composable
fun OutlinedGradientButton(
    text: String,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
    enabled: Boolean = true
) {
    val interactionSource = remember { MutableInteractionSource() }
    val isPressed by interactionSource.collectIsPressedAsState()
    
    val scale by animateFloatAsState(
        targetValue = if (isPressed) 0.96f else 1f,
        animationSpec = spring(
            dampingRatio = Spring.DampingRatioMediumBouncy,
            stiffness = Spring.StiffnessLow
        ),
        label = "scale"
    )
    
    OutlinedButton(
        onClick = onClick,
        modifier = modifier
            .scale(scale)
            .height(56.dp),
        enabled = enabled,
        shape = RoundedCornerShape(12.dp),
        colors = ButtonDefaults.outlinedButtonColors(
            contentColor = TelegramBlue
        ),
        border = ButtonDefaults.outlinedButtonBorder.copy(
            brush = Brush.horizontalGradient(
                listOf(TelegramBlue, TelegramBlueLight)
            )
        ),
        interactionSource = interactionSource
    ) {
        Text(
            text = text,
            style = MaterialTheme.typography.titleMedium.copy(
                fontWeight = FontWeight.SemiBold,
                fontSize = 16.sp
            )
        )
    }
}

@Composable
fun TextLinkButton(
    text: String,
    onClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    TextButton(
        onClick = onClick,
        modifier = modifier
    ) {
        Text(
            text = text,
            style = MaterialTheme.typography.bodyMedium,
            color = TelegramBlue
        )
    }
}



================================================================================
                              COMPONENTS - Avatar.kt
================================================================================

package com.messenger.app.ui.screens.components

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import coil.compose.AsyncImage
import com.messenger.app.ui.theme.*

// Градиенты для аватаров (как в Telegram)
val avatarGradients = listOf(
    listOf(Color(0xFFFF6B6B), Color(0xFFFF8E53)),
    listOf(Color(0xFF4ECDC4), Color(0xFF44A08D)),
    listOf(Color(0xFF667EEA), Color(0xFF764BA2)),
    listOf(Color(0xFFF093FB), Color(0xFFF5576C)),
    listOf(Color(0xFF4FACFE), Color(0xFF00F2FE)),
    listOf(Color(0xFF43E97B), Color(0xFF38F9D7)),
    listOf(Color(0xFFFA709A), Color(0xFFFEE140)),
    listOf(Color(0xFFA8EDEA), Color(0xFFFED6E3))
)

fun getAvatarGradient(id: String): List<Color> {
    val index = id.hashCode().let { if (it < 0) -it else it } % avatarGradients.size
    return avatarGradients[index]
}

@Composable
fun Avatar(
    imageUrl: String?,
    initials: String,
    id: String,
    modifier: Modifier = Modifier,
    size: Dp = 52.dp,
    showOnlineIndicator: Boolean = false,
    isOnline: Boolean = false
) {
    Box(modifier = modifier.size(size)) {
        if (!imageUrl.isNullOrEmpty()) {
            AsyncImage(
                model = imageUrl,
                contentDescription = "Avatar",
                modifier = Modifier
                    .fillMaxSize()
                    .clip(CircleShape),
                contentScale = ContentScale.Crop
            )
        } else {
            // Градиентный аватар с инициалами
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .clip(CircleShape)
                    .background(
                        brush = Brush.linearGradient(getAvatarGradient(id))
                    ),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = initials.take(2),
                    color = Color.White,
                    fontSize = (size.value * 0.38f).sp,
                    fontWeight = FontWeight.Medium
                )
            }
        }
        
        // Индикатор онлайн
        if (showOnlineIndicator && isOnline) {
            Box(
                modifier = Modifier
                    .align(Alignment.BottomEnd)
                    .size(size * 0.27f)
                    .clip(CircleShape)
                    .background(MaterialTheme.colorScheme.background)
                    .padding(2.dp)
                    .clip(CircleShape)
                    .background(GreenOnline)
            )
        }
    }
}

@Composable
fun LargeAvatar(
    imageUrl: String?,
    initials: String,
    id: String,
    modifier: Modifier = Modifier,
    size: Dp = 120.dp
) {
    Box(
        modifier = modifier.size(size),
        contentAlignment = Alignment.Center
    ) {
        if (!imageUrl.isNullOrEmpty()) {
            AsyncImage(
                model = imageUrl,
                contentDescription = "Avatar",
                modifier = Modifier
                    .fillMaxSize()
                    .clip(CircleShape)
                    .border(3.dp, TelegramBlue.copy(alpha = 0.3f), CircleShape),
                contentScale = ContentScale.Crop
            )
        } else {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .clip(CircleShape)
                    .background(
                        brush = Brush.linearGradient(getAvatarGradient(id))
                    ),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = initials.take(2),
                    color = Color.White,
                    fontSize = (size.value * 0.35f).sp,
                    fontWeight = FontWeight.Medium
                )
            }
        }
    }
}



================================================================================
                              COMPONENTS - ChatItem.kt
================================================================================

package com.messenger.app.ui.screens.components

import androidx.compose.animation.animateColorAsState
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material.icons.outlined.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.messenger.app.data.model.Chat
import com.messenger.app.ui.theme.*
import java.text.SimpleDateFormat
import java.util.*

@Composable
fun ChatItem(
    chat: Chat,
    onClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    Row(
        modifier = modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
            .padding(horizontal = 16.dp, vertical = 10.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        // Аватар
        Box {
            Avatar(
                imageUrl = chat.avatarUrl,
                initials = chat.initials,
                id = chat.id,
                size = 54.dp,
                showOnlineIndicator = !chat.isGroup && !chat.isChannel,
                isOnline = chat.isOnline
            )
        }
        
        Spacer(modifier = Modifier.width(14.dp))
        
        // Контент
        Column(
            modifier = Modifier.weight(1f)
        ) {
            // Верхняя строка: имя + время
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    modifier = Modifier.weight(1f, fill = false)
                ) {
                    // Иконка закрепления
                    if (chat.isPinned) {
                        Icon(
                            imageVector = Icons.Filled.PushPin,
                            contentDescription = "Pinned",
                            modifier = Modifier
                                .size(14.dp)
                                .padding(end = 4.dp),
                            tint = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                    
                    Text(
                        text = chat.name,
                        style = MaterialTheme.typography.titleMedium.copy(
                            fontWeight = FontWeight.SemiBold
                        ),
                        color = MaterialTheme.colorScheme.onSurface,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis
                    )
                    
                    // Иконка верификации
                    if (chat.isVerified) {
                        Spacer(modifier = Modifier.width(4.dp))
                        Icon(
                            imageVector = Icons.Filled.Verified,
                            contentDescription = "Verified",
                            modifier = Modifier.size(18.dp),
                            tint = TelegramBlue
                        )
                    }
                }
                
                Spacer(modifier = Modifier.width(8.dp))
                
                // Время
                Text(
                    text = formatTime(chat.lastMessageTime),
                    style = MaterialTheme.typography.labelMedium,
                    color = if (chat.unreadCount > 0 && !chat.isMuted) 
                        TelegramBlue 
                    else 
                        MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
            
            Spacer(modifier = Modifier.height(4.dp))
            
            // Нижняя строка: сообщение + бейдж
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                // Сообщение или статус
                Row(
                    modifier = Modifier.weight(1f),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    // Draft
                    if (chat.draft != null) {
                        Text(
                            text = "Черновик: ",
                            style = MaterialTheme.typography.bodyMedium,
                            color = Red,
                            maxLines = 1
                        )
                        Text(
                            text = chat.draft,
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                    } else if (chat.typing != null) {
                        Text(
                            text = chat.typing,
                            style = MaterialTheme.typography.bodyMedium,
                            color = TelegramBlue,
                            maxLines = 1
                        )
                    } else {
                        Text(
                            text = chat.lastMessage,
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                    }
                }
                
                Spacer(modifier = Modifier.width(8.dp))
                
                // Бейджи
                Row(
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    // Иконка мьюта
                    if (chat.isMuted) {
                        Icon(
                            imageVector = Icons.Filled.VolumeOff,
                            contentDescription = "Muted",
                            modifier = Modifier.size(16.dp),
                            tint = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                        Spacer(modifier = Modifier.width(6.dp))
                    }
                    
                    // Счетчик непрочитанных
                    if (chat.unreadCount > 0) {
                        UnreadBadge(
                            count = chat.unreadCount,
                            isMuted = chat.isMuted
                        )
                    }
                }
            }
        }
    }
}

@Composable
fun UnreadBadge(
    count: Int,
    isMuted: Boolean,
    modifier: Modifier = Modifier
) {
    val backgroundColor = if (isMuted) MutedBadge else UnreadBadge
    val displayCount = if (count > 999) "999+" else count.toString()
    
    Box(
        modifier = modifier
            .height(22.dp)
            .widthIn(min = 22.dp)
            .clip(RoundedCornerShape(11.dp))
            .background(backgroundColor)
            .padding(horizontal = 6.dp),
        contentAlignment = Alignment.Center
    ) {
        Text(
            text = displayCount,
            style = MaterialTheme.typography.labelSmall.copy(
                fontWeight = FontWeight.SemiBold,
                fontSize = 12.sp
            ),
            color = Color.White
        )
    }
}

fun formatTime(timestamp: Long): String {
    val now = System.currentTimeMillis()
    val diff = now - timestamp
    
    return when {
        diff < 24 * 60 * 60 * 1000 -> {
            SimpleDateFormat("HH:mm", Locale.getDefault()).format(Date(timestamp))
        }
        diff < 7 * 24 * 60 * 60 * 1000 -> {
            SimpleDateFormat("EEE", Locale("ru")).format(Date(timestamp))
        }
        else -> {
            SimpleDateFormat("dd.MM.yy", Locale.getDefault()).format(Date(timestamp))
        }
    }
}



================================================================================
                              SCREENS - LoginScreen.kt
================================================================================

package com.messenger.app.ui.screens.auth

import androidx.compose.animation.*
import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.outlined.*
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.rotate
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.messenger.app.ui.screens.components.*
import com.messenger.app.ui.theme.*

@OptIn(ExperimentalAnimationApi::class)
@Composable
fun LoginScreen(
    onLoginClick: () -> Unit = {},
    onRegisterClick: () -> Unit = {},
    onForgotPasswordClick: () -> Unit = {}
) {
    var phone by remember { mutableStateOf("") }
    var password by remember { mutableStateOf("") }
    var isPasswordVisible by remember { mutableStateOf(false) }
    var isLoading by remember { mutableStateOf(false) }
    var phoneError by remember { mutableStateOf<String?>(null) }
    var passwordError by remember { mutableStateOf<String?>(null) }
    
    // Анимация логотипа
    val infiniteTransition = rememberInfiniteTransition(label = "logo")
    val logoScale by infiniteTransition.animateFloat(
        initialValue = 1f,
        targetValue = 1.05f,
        animationSpec = infiniteRepeatable(
            animation = tween(2000, easing = EaseInOutSine),
            repeatMode = RepeatMode.Reverse
        ),
        label = "logoScale"
    )
    
    val logoRotation by infiniteTransition.animateFloat(
        initialValue = -2f,
        targetValue = 2f,
        animationSpec = infiniteRepeatable(
            animation = tween(3000, easing = EaseInOutSine),
            repeatMode = RepeatMode.Reverse
        ),
        label = "logoRotation"
    )
    
    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background)
    ) {
        // Декоративные элементы фона
        Box(
            modifier = Modifier
                .size(300.dp)
                .offset(x = (-100).dp, y = (-100).dp)
                .clip(CircleShape)
                .background(
                    brush = Brush.radialGradient(
                        colors = listOf(
                            TelegramBlue.copy(alpha = 0.1f),
                            Color.Transparent
                        )
                    )
                )
        )
        
        Box(
            modifier = Modifier
                .size(200.dp)
                .align(Alignment.BottomEnd)
                .offset(x = 50.dp, y = 50.dp)
                .clip(CircleShape)
                .background(
                    brush = Brush.radialGradient(
                        colors = listOf(
                            GradientEnd.copy(alpha = 0.1f),
                            Color.Transparent
                        )
                    )
                )
        )
        
        Column(
            modifier = Modifier
                .fillMaxSize()
                .verticalScroll(rememberScrollState())
                .padding(horizontal = 24.dp)
                .padding(top = 60.dp, bottom = 32.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            // Логотип
            Box(
                modifier = Modifier
                    .size(100.dp)
                    .scale(logoScale)
                    .rotate(logoRotation)
                    .clip(CircleShape)
                    .background(
                        brush = Brush.linearGradient(
                            colors = listOf(TelegramBlue, TelegramBlueLight)
                        )
                    ),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    imageVector = Icons.Filled.Send,
                    contentDescription = "Logo",
                    modifier = Modifier
                        .size(50.dp)
                        .rotate(-45f),
                    tint = Color.White
                )
            }
            
            Spacer(modifier = Modifier.height(24.dp))
            
            // Заголовок
            Text(
                text = "Messenger",
                style = MaterialTheme.typography.displaySmall.copy(
                    fontWeight = FontWeight.Bold
                ),
                color = MaterialTheme.colorScheme.onBackground
            )
            
            Spacer(modifier = Modifier.height(8.dp))
            
            Text(
                text = "Войдите в свой аккаунт",
                style = MaterialTheme.typography.bodyLarge,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                textAlign = TextAlign.Center
            )
            
            Spacer(modifier = Modifier.height(48.dp))
            
            // Поле телефона
            MessengerTextField(
                value = phone,
                onValueChange = { 
                    phone = it
                    phoneError = null
                },
                label = "Номер телефона",
                placeholder = "+7 (999) 123-45-67",
                leadingIcon = Icons.Outlined.Phone,
                keyboardType = KeyboardType.Phone,
                imeAction = ImeAction.Next,
                isError = phoneError != null,
                errorMessage = phoneError,
                modifier = Modifier.fillMaxWidth()
            )
            
            Spacer(modifier = Modifier.height(16.dp))
            
            // Поле пароля
            MessengerTextField(
                value = password,
                onValueChange = { 
                    password = it
                    passwordError = null
                },
                label = "Пароль",
                placeholder = "Введите пароль",
                leadingIcon = Icons.Outlined.Lock,
                isPassword = !isPasswordVisible,
                keyboardType = KeyboardType.Password,
                imeAction = ImeAction.Done,
                isError = passwordError != null,
                errorMessage = passwordError,
                trailingIcon = {
                    IconButton(
                        onClick = { isPasswordVisible = !isPasswordVisible },
                        modifier = Modifier.size(24.dp)
                    ) {
                        Icon(
                            imageVector = if (isPasswordVisible) 
                                Icons.Outlined.VisibilityOff 
                            else 
                                Icons.Outlined.Visibility,
                            contentDescription = "Toggle password",
                            tint = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                },
                onImeAction = { 
                    if (phone.isNotBlank() && password.isNotBlank()) {
                        onLoginClick()
                    }
                },
                modifier = Modifier.fillMaxWidth()
            )
            
            Spacer(modifier = Modifier.height(8.dp))
            
            // Забыли пароль
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.End
            ) {
                TextLinkButton(
                    text = "Забыли пароль?",
                    onClick = onForgotPasswordClick
                )
            }
            
            Spacer(modifier = Modifier.height(32.dp))
            
            // Кнопка входа
            GradientButton(
                text = "Войти",
                onClick = {
                    // Валидация
                    var isValid = true
                    if (phone.isBlank()) {
                        phoneError = "Введите номер телефона"
                        isValid = false
                    }
                    if (password.isBlank()) {
                        passwordError = "Введите пароль"
                        isValid = false
                    }
                    if (isValid) {
                        isLoading = true
                        onLoginClick()
                    }
                },
                isLoading = isLoading,
                modifier = Modifier.fillMaxWidth()
            )
            
            Spacer(modifier = Modifier.height(24.dp))
            
            // Разделитель
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Divider(
                    modifier = Modifier.weight(1f),
                    color = MaterialTheme.colorScheme.outline.copy(alpha = 0.3f)
                )
                Text(
                    text = "  или  ",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                Divider(
                    modifier = Modifier.weight(1f),
                    color = MaterialTheme.colorScheme.outline.copy(alpha = 0.3f)
                )
            }
            
            Spacer(modifier = Modifier.height(24.dp))
            
            // Кнопка регистрации
            OutlinedGradientButton(
                text = "Создать аккаунт",
                onClick = onRegisterClick,
                modifier = Modifier.fillMaxWidth()
            )
            
            Spacer(modifier = Modifier.weight(1f))
            
            // Соглашение
            Text(
                text = "Продолжая, вы соглашаетесь с Условиями использования и Политикой конфиденциальности",
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                textAlign = TextAlign.Center,
                modifier = Modifier.padding(horizontal = 16.dp)
            )
        }
    }
}

@Preview(showBackground = true)
@Composable
fun LoginScreenPreview() {
    MessengerTheme {
        LoginScreen()
    }
}

@Preview(showBackground = true, uiMode = android.content.res.Configuration.UI_MODE_NIGHT_YES)
@Composable
fun LoginScreenDarkPreview() {
    MessengerTheme(darkTheme = true) {
        LoginScreen()
    }
}



================================================================================
                              SCREENS - RegisterScreen.kt
================================================================================

package com.messenger.app.ui.screens.auth

import androidx.compose.animation.*
import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material.icons.outlined.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.messenger.app.ui.screens.components.*
import com.messenger.app.ui.theme.*

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun RegisterScreen(
    onRegisterClick: () -> Unit = {},
    onBackClick: () -> Unit = {},
    onLoginClick: () -> Unit = {}
) {
    var firstName by remember { mutableStateOf("") }
    var lastName by remember { mutableStateOf("") }
    var phone by remember { mutableStateOf("") }
    var password by remember { mutableStateOf("") }
    var confirmPassword by remember { mutableStateOf("") }
    var isPasswordVisible by remember { mutableStateOf(false) }
    var isConfirmPasswordVisible by remember { mutableStateOf(false) }
    var isLoading by remember { mutableStateOf(false) }
    var agreedToTerms by remember { mutableStateOf(false) }
    
    // Ошибки валидации
    var firstNameError by remember { mutableStateOf<String?>(null) }
    var phoneError by remember { mutableStateOf<String?>(null) }
    var passwordError by remember { mutableStateOf<String?>(null) }
    var confirmPasswordError by remember { mutableStateOf<String?>(null) }
    
    // Анимация прогресса регистрации
    val progress by animateFloatAsState(
        targetValue = when {
            firstName.isNotBlank() && phone.isNotBlank() && 
            password.isNotBlank() && confirmPassword.isNotBlank() -> 1f
            firstName.isNotBlank() && phone.isNotBlank() && password.isNotBlank() -> 0.75f
            firstName.isNotBlank() && phone.isNotBlank() -> 0.5f
            firstName.isNotBlank() -> 0.25f
            else -> 0f
        },
        animationSpec = tween(300),
        label = "progress"
    )

    Scaffold(
        topBar = {
            TopAppBar(
                title = { },
                navigationIcon = {
                    IconButton(onClick = onBackClick) {
                        Icon(
                            imageVector = Icons.Filled.ArrowBack,
                            contentDescription = "Назад",
                            tint = MaterialTheme.colorScheme.onBackground
                        )
                    }
                },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = Color.Transparent
                )
            )
        }
    ) { paddingValues ->
        Box(
            modifier = Modifier
                .fillMaxSize()
                .background(MaterialTheme.colorScheme.background)
        ) {
            // Декоративные круги
            Box(
                modifier = Modifier
                    .size(250.dp)
                    .align(Alignment.TopEnd)
                    .offset(x = 80.dp, y = (-50).dp)
                    .clip(CircleShape)
                    .background(
                        brush = Brush.radialGradient(
                            colors = listOf(
                                GradientEnd.copy(alpha = 0.08f),
                                Color.Transparent
                            )
                        )
                    )
            )
            
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(paddingValues)
                    .verticalScroll(rememberScrollState())
                    .padding(horizontal = 24.dp)
                    .padding(bottom = 32.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                // Заголовок
                Text(
                    text = "Создать аккаунт",
                    style = MaterialTheme.typography.displaySmall.copy(
                        fontWeight = FontWeight.Bold
                    ),
                    color = MaterialTheme.colorScheme.onBackground
                )
                
                Spacer(modifier = Modifier.height(8.dp))
                
                Text(
                    text = "Заполните данные для регистрации",
                    style = MaterialTheme.typography.bodyLarge,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    textAlign = TextAlign.Center
                )
                
                Spacer(modifier = Modifier.height(16.dp))
                
                // Прогресс бар
                Column(
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween
                    ) {
                        Text(
                            text = "Прогресс заполнения",
                            style = MaterialTheme.typography.labelSmall,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                        Text(
                            text = "${(progress * 100).toInt()}%",
                            style = MaterialTheme.typography.labelSmall,
                            color = TelegramBlue
                        )
                    }
                    Spacer(modifier = Modifier.height(8.dp))
                    LinearProgressIndicator(
                        progress = progress,
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(4.dp)
                            .clip(CircleShape),
                        color = TelegramBlue,
                        trackColor = MaterialTheme.colorScheme.surfaceVariant
                    )
                }
                
                Spacer(modifier = Modifier.height(32.dp))
                
                // Аватар (можно загрузить фото)
                Box(
                    modifier = Modifier
                        .size(100.dp)
                        .clip(CircleShape)
                        .background(
                            brush = Brush.linearGradient(
                                colors = listOf(TelegramBlue, TelegramBlueLight)
                            )
                        ),
                    contentAlignment = Alignment.Center
                ) {
                    if (firstName.isNotBlank()) {
                        Text(
                            text = "${firstName.firstOrNull()?.uppercase() ?: ""}${lastName.firstOrNull()?.uppercase() ?: ""}",
                            style = MaterialTheme.typography.displaySmall,
                            color = Color.White
                        )
                    } else {
                        Icon(
                            imageVector = Icons.Outlined.CameraAlt,
                            contentDescription = "Add photo",
                            modifier = Modifier.size(36.dp),
                            tint = Color.White
                        )
                    }
                }
                
                Spacer(modifier = Modifier.height(8.dp))
                
                TextLinkButton(
                    text = "Добавить фото",
                    onClick = { /* TODO: открыть галерею */ }
                )
                
                Spacer(modifier = Modifier.height(24.dp))
                
                // Имя и Фамилия в ряд
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    MessengerTextField(
                        value = firstName,
                        onValueChange = { 
                            firstName = it
                            firstNameError = null
                        },
                        label = "Имя",
                        placeholder = "Иван",
                        leadingIcon = Icons.Outlined.Person,
                        imeAction = ImeAction.Next,
                        isError = firstNameError != null,
                        errorMessage = firstNameError,
                        modifier = Modifier.weight(1f)
                    )
                    
                    MessengerTextField(
                        value = lastName,
                        onValueChange = { lastName = it },
                        label = "Фамилия",
                        placeholder = "Петров",
                        leadingIcon = Icons.Outlined.Person,
                        imeAction = ImeAction.Next,
                        modifier = Modifier.weight(1f)
                    )
                }
                
                Spacer(modifier = Modifier.height(16.dp))
                
                // Телефон
                MessengerTextField(
                    value = phone,
                    onValueChange = { 
                        phone = it
                        phoneError = null
                    },
                    label = "Номер телефона",
                    placeholder = "+7 (999) 123-45-67",
                    leadingIcon = Icons.Outlined.Phone,
                    keyboardType = KeyboardType.Phone,
                    imeAction = ImeAction.Next,
                    isError = phoneError != null,
                    errorMessage = phoneError,
                    modifier = Modifier.fillMaxWidth()
                )
                
                Spacer(modifier = Modifier.height(16.dp))
                
                // Пароль
                MessengerTextField(
                    value = password,
                    onValueChange = { 
                        password = it
                        passwordError = null
                    },
                    label = "Пароль",
                    placeholder = "Минимум 8 символов",
                    leadingIcon = Icons.Outlined.Lock,
                    isPassword = !isPasswordVisible,
                    keyboardType = KeyboardType.Password,
                    imeAction = ImeAction.Next,
                    isError = passwordError != null,
                    errorMessage = passwordError,
                    trailingIcon = {
                        IconButton(
                            onClick = { isPasswordVisible = !isPasswordVisible },
                            modifier = Modifier.size(24.dp)
                        ) {
                            Icon(
                                imageVector = if (isPasswordVisible) 
                                    Icons.Outlined.VisibilityOff 
                                else 
                                    Icons.Outlined.Visibility,
                                contentDescription = "Toggle password",
                                tint = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                    },
                    modifier = Modifier.fillMaxWidth()
                )
                
                // Индикатор сложности пароля
                if (password.isNotEmpty()) {
                    Spacer(modifier = Modifier.height(8.dp))
                    PasswordStrengthIndicator(password = password)
                }
                
                Spacer(modifier = Modifier.height(16.dp))
                
                // Подтверждение пароля
                MessengerTextField(
                    value = confirmPassword,
                    onValueChange = { 
                        confirmPassword = it
                        confirmPasswordError = null
                    },
                    label = "Подтвердите пароль",
                    placeholder = "Повторите пароль",
                    leadingIcon = Icons.Outlined.Lock,
                    isPassword = !isConfirmPasswordVisible,
                    keyboardType = KeyboardType.Password,
                    imeAction = ImeAction.Done,
                    isError = confirmPasswordError != null,
                    errorMessage = confirmPasswordError,
                    trailingIcon = {
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            if (confirmPassword.isNotEmpty() && password == confirmPassword) {
                                Icon(
                                    imageVector = Icons.Filled.CheckCircle,
                                    contentDescription = "Passwords match",
                                    modifier = Modifier.size(20.dp),
                                    tint = Green
                                )
                                Spacer(modifier = Modifier.width(8.dp))
                            }
                            IconButton(
                                onClick = { isConfirmPasswordVisible = !isConfirmPasswordVisible },
                                modifier = Modifier.size(24.dp)
                            ) {
                                Icon(
                                    imageVector = if (isConfirmPasswordVisible) 
                                        Icons.Outlined.VisibilityOff 
                                    else 
                                        Icons.Outlined.Visibility,
                                    contentDescription = "Toggle password",
                                    tint = MaterialTheme.colorScheme.onSurfaceVariant
                                )
                            }
                        }
                    },
                    modifier = Modifier.fillMaxWidth()
                )
                
                Spacer(modifier = Modifier.height(24.dp))
                
                // Чекбокс соглашения
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Checkbox(
                        checked = agreedToTerms,
                        onCheckedChange = { agreedToTerms = it },
                        colors = CheckboxDefaults.colors(
                            checkedColor = TelegramBlue,
                            uncheckedColor = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    )
                    Text(
                        text = "Я согласен с Условиями использования и Политикой конфиденциальности",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant,
                        modifier = Modifier.padding(start = 4.dp)
                    )
                }
                
                Spacer(modifier = Modifier.height(32.dp))
                
                // Кнопка регистрации
                GradientButton(
                    text = "Зарегистрироваться",
                    onClick = {
                        // Валидация
                        var isValid = true
                        if (firstName.isBlank()) {
                            firstNameError = "Введите имя"
                            isValid = false
                        }
                        if (phone.isBlank()) {
                            phoneError = "Введите номер телефона"
                            isValid = false
                        }
                        if (password.length < 8) {
                            passwordError = "Пароль должен содержать минимум 8 символов"
                            isValid = false
                        }
                        if (password != confirmPassword) {
                            confirmPasswordError = "Пароли не совпадают"
                            isValid = false
                        }
                        if (isValid && agreedToTerms) {
                            isLoading = true
                            onRegisterClick()
                        }
                    },
                    isLoading = isLoading,
                    enabled = agreedToTerms,
                    modifier = Modifier.fillMaxWidth()
                )
                
                Spacer(modifier = Modifier.height(24.dp))
                
                // Уже есть аккаунт
                Row(
                    horizontalArrangement = Arrangement.Center,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = "Уже есть аккаунт?",
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    TextLinkButton(
                        text = "Войти",
                        onClick = onLoginClick
                    )
                }
            }
        }
    }
}

@Composable
fun PasswordStrengthIndicator(password: String) {
    val strength = calculatePasswordStrength(password)
    
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(4.dp)
    ) {
        repeat(4) { index ->
            Box(
                modifier = Modifier
                    .weight(1f)
                    .height(4.dp)
                    .clip(CircleShape)
                    .background(
                        if (index < strength.level) strength.color
                        else MaterialTheme.colorScheme.surfaceVariant
                    )
            )
        }
    }
    
    Spacer(modifier = Modifier.height(4.dp))
    
    Text(
        text = strength.text,
        style = MaterialTheme.typography.labelSmall,
        color = strength.color
    )
}

data class PasswordStrength(
    val level: Int,
    val text: String,
    val color: Color
)

fun calculatePasswordStrength(password: String): PasswordStrength {
    var score = 0
    
    if (password.length >= 8) score++
    if (password.length >= 12) score++
    if (password.any { it.isDigit() }) score++
    if (password.any { it.isUpperCase() }) score++
    if (password.any { !it.isLetterOrDigit() }) score++
    
    return when {
        score <= 1 -> PasswordStrength(1, "Слабый", Red)
        score == 2 -> PasswordStrength(2, "Средний", Orange)
        score == 3 -> PasswordStrength(3, "Хороший", TelegramBlue)
        else -> PasswordStrength(4, "Отличный", Green)
    }
}

@Preview(showBackground = true)
@Composable
fun RegisterScreenPreview() {
    MessengerTheme {
        RegisterScreen()
    }
}



================================================================================
                              SCREENS - MainScreen.kt
================================================================================

package com.messenger.app.ui.screens.main

import androidx.compose.animation.*
import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material.icons.outlined.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.nestedscroll.nestedScroll
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.messenger.app.data.model.Chat
import com.messenger.app.data.model.SampleData
import com.messenger.app.ui.screens.components.*
import com.messenger.app.ui.theme.*
import kotlinx.coroutines.launch

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MainScreen(
    chats: List<Chat> = SampleData.chats,
    onChatClick: (Chat) -> Unit = {},
    onMenuClick: () -> Unit = {},
    onSearchClick: () -> Unit = {},
    onNewChatClick: () -> Unit = {}
) {
    var searchQuery by remember { mutableStateOf("") }
    var isSearchActive by remember { mutableStateOf(false) }
    val drawerState = rememberDrawerState(initialValue = DrawerValue.Closed)
    val scope = rememberCoroutineScope()
    val listState = rememberLazyListState()
    val scrollBehavior = TopAppBarDefaults.pinnedScrollBehavior()
    
    // Анимация FAB при скролле
    val expandedFab by remember {
        derivedStateOf { listState.firstVisibleItemIndex == 0 }
    }
    
    // Фильтрация чатов по поиску
    val filteredChats = remember(searchQuery, chats) {
        if (searchQuery.isBlank()) chats
        else chats.filter { 
            it.name.contains(searchQuery, ignoreCase = true) ||
            it.lastMessage.contains(searchQuery, ignoreCase = true)
        }
    }
    
    // Сортировка: закрепленные сверху
    val sortedChats = remember(filteredChats) {
        filteredChats.sortedWith(
            compareByDescending<Chat> { it.isPinned }
                .thenByDescending { it.lastMessageTime }
        )
    }

    ModalNavigationDrawer(
        drawerState = drawerState,
        drawerContent = {
            NavigationDrawerContent(
                onCloseDrawer = { scope.launch { drawerState.close() } }
            )
        }
    ) {
        Scaffold(
            modifier = Modifier.nestedScroll(scrollBehavior.nestedScrollConnection),
            topBar = {
                AnimatedContent(
                    targetState = isSearchActive,
                    transitionSpec = {
                        fadeIn(animationSpec = tween(200)) togetherWith 
                        fadeOut(animationSpec = tween(200))
                    },
                    label = "topBar"
                ) { searchActive ->
                    if (searchActive) {
                        // Поисковый AppBar
                        SearchTopBar(
                            query = searchQuery,
                            onQueryChange = { searchQuery = it },
                            onClose = {
                                isSearchActive = false
                                searchQuery = ""
                            }
                        )
                    } else {
                        // Основной AppBar
                        MainTopAppBar(
                            onMenuClick = { scope.launch { drawerState.open() } },
                            onSearchClick = { isSearchActive = true },
                            scrollBehavior = scrollBehavior
                        )
                    }
                }
            },
            floatingActionButton = {
                ExtendedFloatingActionButton(
                    onClick = onNewChatClick,
                    expanded = expandedFab,
                    icon = {
                        Icon(
                            imageVector = Icons.Filled.Edit,
                            contentDescription = "Новый чат"
                        )
                    },
                    text = { Text("Написать") },
                    containerColor = TelegramBlue,
                    contentColor = Color.White,
                    modifier = Modifier.shadow(
                        elevation = 8.dp,
                        shape = if (expandedFab) RoundedCornerShape(16.dp) else CircleShape,
                        ambientColor = TelegramBlue.copy(alpha = 0.3f),
                        spotColor = TelegramBlue.copy(alpha = 0.3f)
                    )
                )
            }
        ) { paddingValues ->
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .background(MaterialTheme.colorScheme.background)
                    .padding(paddingValues)
            ) {
                if (sortedChats.isEmpty()) {
                    // Пустое состояние
                    EmptyChatsState(
                        isSearching = searchQuery.isNotBlank()
                    )
                } else {
                    LazyColumn(
                        state = listState,
                        modifier = Modifier.fillMaxSize()
                    ) {
                        // Закрепленные чаты
                        val pinnedChats = sortedChats.filter { it.isPinned }
                        val regularChats = sortedChats.filter { !it.isPinned }
                        
                        if (pinnedChats.isNotEmpty() && !isSearchActive) {
                            item {
                                SectionHeader(title = "Закрепленные")
                            }
                        }
                        
                        items(
                            items = pinnedChats,
                            key = { it.id }
                        ) { chat ->
                            ChatItem(
                                chat = chat,
                                onClick = { onChatClick(chat) },
                                modifier = Modifier.animateItemPlacement()
                            )
                        }
                        
                        if (pinnedChats.isNotEmpty() && regularChats.isNotEmpty() && !isSearchActive) {
                            item {
                                SectionHeader(title = "Все чаты")
                            }
                        }
                        
                        items(
                            items = regularChats,
                            key = { it.id }
                        ) { chat ->
                            ChatItem(
                                chat = chat,
                                onClick = { onChatClick(chat) },
                                modifier = Modifier.animateItemPlacement()
                            )
                        }
                        
                        // Отступ снизу для FAB
                        item {
                            Spacer(modifier = Modifier.height(80.dp))
                        }
                    }
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MainTopAppBar(
    onMenuClick: () -> Unit,
    onSearchClick: () -> Unit,
    scrollBehavior: TopAppBarScrollBehavior
) {
    TopAppBar(
        title = {
            Text(
                text = "Messenger",
                style = MaterialTheme.typography.headlineMedium.copy(
                    fontWeight = FontWeight.Bold
                )
            )
        },
        navigationIcon = {
            IconButton(onClick = onMenuClick) {
                Icon(
                    imageVector = Icons.Filled.Menu,
                    contentDescription = "Меню"
                )
            }
        },
        actions = {
            IconButton(onClick = onSearchClick) {
                Icon(
                    imageVector = Icons.Outlined.Search,
                    contentDescription = "Поиск"
                )
            }
        },
        colors = TopAppBarDefaults.topAppBarColors(
            containerColor = MaterialTheme.colorScheme.background,
            scrolledContainerColor = MaterialTheme.colorScheme.surface
        ),
        scrollBehavior = scrollBehavior
    )
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SearchTopBar(
    query: String,
    onQueryChange: (String) -> Unit,
    onClose: () -> Unit
) {
    TopAppBar(
        title = {
            SearchTextField(
                value = query,
                onValueChange = onQueryChange,
                placeholder = "Поиск чатов...",
                modifier = Modifier.fillMaxWidth()
            )
        },
        navigationIcon = {
            IconButton(onClick = onClose) {
                Icon(
                    imageVector = Icons.Filled.ArrowBack,
                    contentDescription = "Закрыть"
                )
            }
        },
        actions = {
            if (query.isNotEmpty()) {
                IconButton(onClick = { onQueryChange("") }) {
                    Icon(
                        imageVector = Icons.Filled.Close,
                        contentDescription = "Очистить"
                    )
                }
            }
        },
        colors = TopAppBarDefaults.topAppBarColors(
            containerColor = MaterialTheme.colorScheme.background
        )
    )
}

@Composable
fun SectionHeader(title: String) {
    Text(
        text = title,
        style = MaterialTheme.typography.labelMedium.copy(
            fontWeight = FontWeight.SemiBold
        ),
        color = TelegramBlue,
        modifier = Modifier.padding(
            horizontal = 16.dp,
            vertical = 12.dp
        )
    )
}

@Composable
fun EmptyChatsState(isSearching: Boolean) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(32.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        Icon(
            imageVector = if (isSearching) Icons.Outlined.SearchOff else Icons.Outlined.Chat,
            contentDescription = null,
            modifier = Modifier.size(80.dp),
            tint = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.5f)
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        Text(
            text = if (isSearching) "Ничего не найдено" else "Нет чатов",
            style = MaterialTheme.typography.titleLarge,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        
        Spacer(modifier = Modifier.height(8.dp))
        
        Text(
            text = if (isSearching) 
                "Попробуйте изменить запрос" 
            else 
                "Начните общение прямо сейчас",
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.7f)
        )
    }
}

@Composable
fun NavigationDrawerContent(
    onCloseDrawer: () -> Unit
) {
    ModalDrawerSheet(
        drawerContainerColor = MaterialTheme.colorScheme.background
    ) {
        // Шапка с профилем
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .background(
                    brush = androidx.compose.ui.graphics.Brush.linearGradient(
                        colors = listOf(TelegramBlue, TelegramBlueLight)
                    )
                )
                .padding(24.dp)
        ) {
            Column {
                Avatar(
                    imageUrl = null,
                    initials = "МП",
                    id = "user",
                    size = 64.dp
                )
                
                Spacer(modifier = Modifier.height(16.dp))
                
                Text(
                    text = "Мой Профиль",
                    style = MaterialTheme.typography.titleLarge.copy(
                        fontWeight = FontWeight.SemiBold
                    ),
                    color = Color.White
                )
                
                Spacer(modifier = Modifier.height(4.dp))
                
                Text(
                    text = "+7 (999) 123-45-67",
                    style = MaterialTheme.typography.bodyMedium,
                    color = Color.White.copy(alpha = 0.8f)
                )
            }
        }
        
        Spacer(modifier = Modifier.height(8.dp))
        
        // Пункты меню
        NavigationDrawerItem(
            icon = { Icon(Icons.Outlined.Group, contentDescription = null) },
            label = { Text("Новая группа") },
            selected = false,
            onClick = onCloseDrawer
        )
        
        NavigationDrawerItem(
            icon = { Icon(Icons.Outlined.Person, contentDescription = null) },
            label = { Text("Контакты") },
            selected = false,
            onClick = onCloseDrawer
        )
        
        NavigationDrawerItem(
            icon = { Icon(Icons.Outlined.Call, contentDescription = null) },
            label = { Text("Звонки") },
            selected = false,
            onClick = onCloseDrawer
        )
        
        NavigationDrawerItem(
            icon = { Icon(Icons.Outlined.Bookmark, contentDescription = null) },
            label = { Text("Избранное") },
            selected = false,
            onClick = onCloseDrawer
        )
        
        NavigationDrawerItem(
            icon = { Icon(Icons.Outlined.Settings, contentDescription = null) },
            label = { Text("Настройки") },
            selected = false,
            onClick = onCloseDrawer
        )
        
        Divider(
            modifier = Modifier.padding(vertical = 8.dp),
            color = MaterialTheme.colorScheme.outline.copy(alpha = 0.2f)
        )
        
        NavigationDrawerItem(
            icon = { Icon(Icons.Outlined.PersonAdd, contentDescription = null) },
            label = { Text("Пригласить друзей") },
            selected = false,
            onClick = onCloseDrawer
        )
        
        NavigationDrawerItem(
            icon = { Icon(Icons.Outlined.Help, contentDescription = null) },
            label = { Text("Помощь") },
            selected = false,
            onClick = onCloseDrawer
        )
    }
}

@Preview(showBackground = true)
@Composable
fun MainScreenPreview() {
    MessengerTheme {
        MainScreen()
    }
}

@Preview(showBackground = true, uiMode = android.content.res.Configuration.UI_MODE_NIGHT_YES)
@Composable
fun MainScreenDarkPreview() {
    MessengerTheme(darkTheme = true) {
        MainScreen()
    }
}



================================================================================
                              NAVIGATION - NavGraph.kt
================================================================================

package com.messenger.app.ui.navigation

import androidx.compose.animation.*
import androidx.compose.animation.core.*
import androidx.compose.runtime.Composable
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.messenger.app.ui.screens.auth.LoginScreen
import com.messenger.app.ui.screens.auth.RegisterScreen
import com.messenger.app.ui.screens.main.MainScreen

sealed class Screen(val route: String) {
    object Login : Screen("login")
    object Register : Screen("register")
    object Main : Screen("main")
    object Chat : Screen("chat/{chatId}") {
        fun createRoute(chatId: String) = "chat/$chatId"
    }
}

@OptIn(ExperimentalAnimationApi::class)
@Composable
fun MessengerNavGraph(
    navController: NavHostController = rememberNavController(),
    startDestination: String = Screen.Login.route
) {
    NavHost(
        navController = navController,
        startDestination = startDestination,
        enterTransition = {
            slideIntoContainer(
                towards = AnimatedContentTransitionScope.SlideDirection.Left,
                animationSpec = tween(300)
            ) + fadeIn(animationSpec = tween(300))
        },
        exitTransition = {
            slideOutOfContainer(
                towards = AnimatedContentTransitionScope.SlideDirection.Left,
                animationSpec = tween(300)
            ) + fadeOut(animationSpec = tween(300))
        },
        popEnterTransition = {
            slideIntoContainer(
                towards = AnimatedContentTransitionScope.SlideDirection.Right,
                animationSpec = tween(300)
            ) + fadeIn(animationSpec = tween(300))
        },
        popExitTransition = {
            slideOutOfContainer(
                towards = AnimatedContentTransitionScope.SlideDirection.Right,
                animationSpec = tween(300)
            ) + fadeOut(animationSpec = tween(300))
        }
    ) {
        // Экран входа
        composable(Screen.Login.route) {
            LoginScreen(
                onLoginClick = {
                    navController.navigate(Screen.Main.route) {
                        popUpTo(Screen.Login.route) { inclusive = true }
                    }
                },
                onRegisterClick = {
                    navController.navigate(Screen.Register.route)
                },
                onForgotPasswordClick = {
                    // TODO: Навигация на экран восстановления пароля
                }
            )
        }
        
        // Экран регистрации
        composable(Screen.Register.route) {
            RegisterScreen(
                onRegisterClick = {
                    navController.navigate(Screen.Main.route) {
                        popUpTo(Screen.Login.route) { inclusive = true }
                    }
                },
                onBackClick = {
                    navController.popBackStack()
                },
                onLoginClick = {
                    navController.popBackStack()
                }
            )
        }
        
        // Главный экран
        composable(Screen.Main.route) {
            MainScreen(
                onChatClick = { chat ->
                    navController.navigate(Screen.Chat.createRoute(chat.id))
                },
                onNewChatClick = {
                    // TODO: Открыть экран нового чата
                }
            )
        }
        
        // Экран чата (TODO)
        composable(Screen.Chat.route) { backStackEntry ->
            val chatId = backStackEntry.arguments?.getString("chatId")
            // TODO: ChatScreen(chatId = chatId)
        }
    }
}



================================================================================
                              MainActivity.kt
================================================================================

package com.messenger.app

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.ui.Modifier
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import androidx.navigation.compose.rememberNavController
import com.messenger.app.ui.navigation.MessengerNavGraph
import com.messenger.app.ui.theme.MessengerTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        // Splash Screen
        installSplashScreen()
        
        super.onCreate(savedInstanceState)
        
        // Edge-to-edge display
        enableEdgeToEdge()
        
        setContent {
            MessengerTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    val navController = rememberNavController()
                    MessengerNavGraph(navController = navController)
                }
            }
        }
    }
}



================================================================================
                              BUILD - app/build.gradle.kts
================================================================================

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
    id("com.google.devtools.ksp")
}

android {
    namespace = "com.messenger.app"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.messenger.app"
        minSdk = 26
        targetSdk = 34
        versionCode = 1
        versionName = "1.0.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        vectorDrawables {
            useSupportLibrary = true
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            isShrinkResources = true
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    
    kotlinOptions {
        jvmTarget = "17"
    }
    
    buildFeatures {
        compose = true
    }
    
    composeOptions {
        kotlinCompilerExtensionVersion = "1.5.8"
    }
    
    packaging {
        resources {
            excludes += "/META-INF/{AL2.0,LGPL2.1}"
        }
    }
}

dependencies {
    // Core Android
    implementation("androidx.core:core-ktx:1.12.0")
    implementation("androidx.lifecycle:lifecycle-runtime-ktx:2.7.0")
    implementation("androidx.activity:activity-compose:1.8.2")
    
    // Splash Screen
    implementation("androidx.core:core-splashscreen:1.0.1")
    
    // Compose BOM
    implementation(platform("androidx.compose:compose-bom:2024.01.00"))
    implementation("androidx.compose.ui:ui")
    implementation("androidx.compose.ui:ui-graphics")
    implementation("androidx.compose.ui:ui-tooling-preview")
    implementation("androidx.compose.material3:material3")
    implementation("androidx.compose.material:material-icons-extended")
    implementation("androidx.compose.animation:animation")
    
    // Navigation
    implementation("androidx.navigation:navigation-compose:2.7.6")
    
    // Coil for images
    implementation("io.coil-kt:coil-compose:2.5.0")
    
    // Accompanist
    implementation("com.google.accompanist:accompanist-systemuicontroller:0.33.2-alpha")
    
    // Testing
    testImplementation("junit:junit:4.13.2")
    androidTestImplementation("androidx.test.ext:junit:1.1.5")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.5.1")
    androidTestImplementation(platform("androidx.compose:compose-bom:2024.01.00"))
    androidTestImplementation("androidx.compose.ui:ui-test-junit4")
    debugImplementation("androidx.compose.ui:ui-tooling")
    debugImplementation("androidx.compose.ui:ui-test-manifest")
}



================================================================================
                              BUILD - build.gradle.kts (project)
================================================================================

// Top-level build file
plugins {
    id("com.android.application") version "8.2.2" apply false
    id("org.jetbrains.kotlin.android") version "1.9.22" apply false
    id("com.google.devtools.ksp") version "1.9.22-1.0.17" apply false
}



================================================================================
                              BUILD - settings.gradle.kts
================================================================================

pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}

dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
    }
}

rootProject.name = "MessengerApp"
include(":app")


