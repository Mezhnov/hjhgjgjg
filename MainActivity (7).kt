package com.example.myapplication

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.animation.*
import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.BasicTextField
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.outlined.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.scale
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.focus.FocusDirection
import androidx.compose.ui.focus.FocusRequester
import androidx.compose.ui.focus.focusRequester
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.SolidColor
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.platform.LocalFocusManager
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import com.example.myapplication.ui.theme.MyApplicationTheme
import kotlinx.coroutines.delay

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            MyApplicationTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = TelegramColors.Background
                ) {
                    TelegramAuthApp()
                }
            }
        }
    }
}

// Состояния навигации
sealed class AuthScreen {
    object Welcome : AuthScreen()
    object PhoneInput : AuthScreen()
    data class CodeVerification(val phoneNumber: String, val countryCode: String) : AuthScreen()
    data class Registration(val phoneNumber: String) : AuthScreen()
    object Success : AuthScreen()
}

// Модель данных для страны
data class Country(
    val name: String,
    val code: String,
    val flag: String,
    val phoneCode: String
)

// Модель данных пользователя
data class UserData(
    var firstName: String = "",
    var lastName: String = "",
    var birthDay: String = "",
    var birthMonth: String = "",
    var birthYear: String = "",
    var bio: String = "",
    var username: String = ""
)

// Список стран
val countries = listOf(
    Country("Россия", "RU", "🇷🇺", "+7"),
    Country("Украина", "UA", "🇺🇦", "+380"),
    Country("Беларусь", "BY", "🇧🇾", "+375"),
    Country("Казахстан", "KZ", "🇰🇿", "+7"),
    Country("Узбекистан", "UZ", "🇺🇿", "+998"),
    Country("Германия", "DE", "🇩🇪", "+49"),
    Country("США", "US", "🇺🇸", "+1"),
    Country("Великобритания", "GB", "🇬🇧", "+44"),
    Country("Франция", "FR", "🇫🇷", "+33"),
    Country("Италия", "IT", "🇮🇹", "+39"),
    Country("Испания", "ES", "🇪🇸", "+34"),
    Country("Польша", "PL", "🇵🇱", "+48"),
    Country("Турция", "TR", "🇹🇷", "+90"),
    Country("Китай", "CN", "🇨🇳", "+86"),
    Country("Япония", "JP", "🇯🇵", "+81"),
    Country("Южная Корея", "KR", "🇰🇷", "+82"),
    Country("Индия", "IN", "🇮🇳", "+91"),
    Country("Бразилия", "BR", "🇧🇷", "+55"),
    Country("Канада", "CA", "🇨🇦", "+1"),
    Country("Австралия", "AU", "🇦🇺", "+61"),
    Country("ОАЭ", "AE", "🇦🇪", "+971"),
    Country("Грузия", "GE", "🇬🇪", "+995"),
    Country("Армения", "AM", "🇦🇲", "+374"),
    Country("Азербайджан", "AZ", "🇦🇿", "+994"),
    Country("Молдова", "MD", "🇲🇩", "+373"),
    Country("Литва", "LT", "🇱🇹", "+370"),
    Country("Латвия", "LV", "🇱🇻", "+371"),
    Country("Эстония", "EE", "🇪🇪", "+372")
)

// Telegram цвета - улучшенная палитра
object TelegramColors {
    val Primary = Color(0xFF0088CC)
    val PrimaryLight = Color(0xFF54C7FC)
    val PrimaryDark = Color(0xFF006699)
    val PrimaryVariant = Color(0xFF229ED9)
    
    val Background = Color(0xFFF7F8FA)
    val BackgroundDark = Color(0xFF17212B)
    val Surface = Color.White
    val SurfaceVariant = Color(0xFFF0F2F5)
    
    val TextPrimary = Color(0xFF1D2733)
    val TextSecondary = Color(0xFF707579)
    val TextTertiary = Color(0xFF9CA3AF)
    
    val Divider = Color(0xFFE7E8EA)
    val DividerDark = Color(0xFF303940)
    
    val Link = Color(0xFF168ACD)
    val Error = Color(0xFFE53935)
    val ErrorLight = Color(0xFFFFEBEE)
    val Success = Color(0xFF4CAF50)
    val SuccessLight = Color(0xFFE8F5E9)
    val Warning = Color(0xFFFF9800)
    
    val GradientStart = Color(0xFF2AABEE)
    val GradientMiddle = Color(0xFF229ED9)
    val GradientEnd = Color(0xFF1E88E5)
    
    val AccentBlue = Color(0xFF2AABEE)
    val AccentPurple = Color(0xFF9C27B0)
    val AccentGreen = Color(0xFF43A047)
    val AccentOrange = Color(0xFFFF7043)
    val AccentPink = Color(0xFFEC407A)
    
    val CardShadow = Color(0x1A000000)
    val Overlay = Color(0x80000000)
}

@Composable
fun TelegramAuthApp() {
    var currentScreen by remember { mutableStateOf<AuthScreen>(AuthScreen.Welcome) }
    var userData by remember { mutableStateOf(UserData()) }

    AnimatedContent(
        targetState = currentScreen,
        transitionSpec = {
            when {
                targetState is AuthScreen.Welcome -> {
                    fadeIn(animationSpec = tween(400)) + scaleIn(initialScale = 0.92f) togetherWith
                            fadeOut(animationSpec = tween(400)) + scaleOut(targetScale = 1.08f)
                }
                initialState is AuthScreen.Welcome -> {
                    fadeIn(animationSpec = tween(400)) + scaleIn(initialScale = 1.08f) togetherWith
                            fadeOut(animationSpec = tween(400)) + scaleOut(targetScale = 0.92f)
                }
                else -> {
                    slideInHorizontally(animationSpec = tween(350)) { it } + fadeIn() togetherWith
                            slideOutHorizontally(animationSpec = tween(350)) { -it } + fadeOut()
                }
            }
        },
        label = "screen_transition"
    ) { screen ->
        when (screen) {
            is AuthScreen.Welcome -> {
                WelcomeScreen(
                    onStartMessaging = { currentScreen = AuthScreen.PhoneInput }
                )
            }
            is AuthScreen.PhoneInput -> {
                LoginScreen(
                    onBack = { currentScreen = AuthScreen.Welcome },
                    onContinue = { phoneNumber, countryCode ->
                        currentScreen = AuthScreen.CodeVerification(phoneNumber, countryCode)
                    }
                )
            }
            is AuthScreen.CodeVerification -> {
                CodeVerificationScreen(
                    phoneNumber = screen.phoneNumber,
                    countryCode = screen.countryCode,
                    onBack = { currentScreen = AuthScreen.PhoneInput },
                    onCodeVerified = { isNewUser ->
                        currentScreen = if (isNewUser) {
                            AuthScreen.Registration(screen.phoneNumber)
                        } else {
                            AuthScreen.Success
                        }
                    }
                )
            }
            is AuthScreen.Registration -> {
                RegistrationScreen(
                    phoneNumber = screen.phoneNumber,
                    userData = userData,
                    onUserDataChanged = { userData = it },
                    onBack = { currentScreen = AuthScreen.PhoneInput },
                    onComplete = { currentScreen = AuthScreen.Success }
                )
            }
            is AuthScreen.Success -> {
                SuccessScreen(
                    userData = userData,
                    onLogout = {
                        userData = UserData()
                        currentScreen = AuthScreen.Welcome
                    }
                )
            }
        }
    }
}

@Composable
fun WelcomeScreen(
    onStartMessaging: () -> Unit
) {
    val infiniteTransition = rememberInfiniteTransition(label = "welcome_animation")
    
    val logoScale by infiniteTransition.animateFloat(
        initialValue = 1f,
        targetValue = 1.05f,
        animationSpec = infiniteRepeatable(
            animation = tween(2000, easing = EaseInOutCubic),
            repeatMode = RepeatMode.Reverse
        ),
        label = "logo_scale"
    )
    
    val glowAlpha by infiniteTransition.animateFloat(
        initialValue = 0.3f,
        targetValue = 0.6f,
        animationSpec = infiniteRepeatable(
            animation = tween(2000, easing = EaseInOutCubic),
            repeatMode = RepeatMode.Reverse
        ),
        label = "glow_alpha"
    )
    
    var isVisible by remember { mutableStateOf(false) }
    
    LaunchedEffect(Unit) {
        delay(100)
        isVisible = true
    }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(
                brush = Brush.verticalGradient(
                    colors = listOf(
                        TelegramColors.Background,
                        TelegramColors.Surface,
                        TelegramColors.Background
                    )
                )
            )
    ) {
        // Декоративные круги на фоне
        Box(
            modifier = Modifier
                .size(300.dp)
                .offset(x = (-80).dp, y = (-50).dp)
                .background(
                    brush = Brush.radialGradient(
                        colors = listOf(
                            TelegramColors.Primary.copy(alpha = 0.08f),
                            Color.Transparent
                        )
                    ),
                    shape = CircleShape
                )
        )
        
        Box(
            modifier = Modifier
                .size(250.dp)
                .align(Alignment.TopEnd)
                .offset(x = 80.dp, y = 100.dp)
                .background(
                    brush = Brush.radialGradient(
                        colors = listOf(
                            TelegramColors.AccentPurple.copy(alpha = 0.06f),
                            Color.Transparent
                        )
                    ),
                    shape = CircleShape
                )
        )
        
        Box(
            modifier = Modifier
                .size(200.dp)
                .align(Alignment.BottomStart)
                .offset(x = (-40).dp, y = (-100).dp)
                .background(
                    brush = Brush.radialGradient(
                        colors = listOf(
                            TelegramColors.AccentBlue.copy(alpha = 0.07f),
                            Color.Transparent
                        )
                    ),
                    shape = CircleShape
                )
        )

        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 32.dp)
                .padding(bottom = 48.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Spacer(modifier = Modifier.weight(0.8f))
            
            // Анимированный логотип
            AnimatedVisibility(
                visible = isVisible,
                enter = fadeIn(tween(800)) + scaleIn(tween(800), initialScale = 0.5f)
            ) {
                Box(contentAlignment = Alignment.Center) {
                    // Внешнее свечение
                    Box(
                        modifier = Modifier
                            .size(160.dp)
                            .scale(logoScale * 1.1f)
                            .background(
                                brush = Brush.radialGradient(
                                    colors = listOf(
                                        TelegramColors.Primary.copy(alpha = glowAlpha * 0.5f),
                                        Color.Transparent
                                    )
                                ),
                                shape = CircleShape
                            )
                    )
                    
                    // Основной логотип
                    Box(
                        modifier = Modifier
                            .size(130.dp)
                            .scale(logoScale)
                            .shadow(
                                elevation = 24.dp,
                                shape = CircleShape,
                                spotColor = TelegramColors.Primary.copy(alpha = 0.4f)
                            )
                            .clip(CircleShape)
                            .background(
                                brush = Brush.linearGradient(
                                    colors = listOf(
                                        TelegramColors.GradientStart,
                                        TelegramColors.GradientMiddle,
                                        TelegramColors.GradientEnd
                                    )
                                )
                            ),
                        contentAlignment = Alignment.Center
                    ) {
                        Text(
                            text = "✈",
                            fontSize = 64.sp,
                            color = Color.White
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(48.dp))

            // Заголовок
            AnimatedVisibility(
                visible = isVisible,
                enter = fadeIn(tween(800, delayMillis = 200)) + 
                        slideInVertically(tween(800, delayMillis = 200)) { it / 2 }
            ) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Text(
                        text = "Telegram",
                        style = MaterialTheme.typography.displayMedium,
                        color = TelegramColors.TextPrimary,
                        fontWeight = FontWeight.Bold
                    )
                    
                    Spacer(modifier = Modifier.height(16.dp))
                    
                    Text(
                        text = "Самый быстрый мессенджер в мире.
Бесплатный и безопасный.",
                        style = MaterialTheme.typography.bodyLarge,
                        color = TelegramColors.TextSecondary,
                        textAlign = TextAlign.Center,
                        lineHeight = 24.sp
                    )
                }
            }

            Spacer(modifier = Modifier.weight(1f))

            // Кнопка
            AnimatedVisibility(
                visible = isVisible,
                enter = fadeIn(tween(800, delayMillis = 400)) + 
                        slideInVertically(tween(800, delayMillis = 400)) { it }
            ) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Button(
                        onClick = onStartMessaging,
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(56.dp)
                            .shadow(
                                elevation = 12.dp,
                                shape = RoundedCornerShape(16.dp),
                                spotColor = TelegramColors.Primary.copy(alpha = 0.4f)
                            ),
                        shape = RoundedCornerShape(16.dp),
                        colors = ButtonDefaults.buttonColors(
                            containerColor = TelegramColors.Primary
                        )
                    ) {
                        Text(
                            text = "Начать общение",
                            style = MaterialTheme.typography.titleMedium,
                            color = Color.White,
                            fontWeight = FontWeight.SemiBold
                        )
                        Spacer(modifier = Modifier.width(8.dp))
                        Icon(
                            imageVector = Icons.Default.ArrowForward,
                            contentDescription = null,
                            tint = Color.White
                        )
                    }

                    Spacer(modifier = Modifier.height(24.dp))

                    // Особенности
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceEvenly
                    ) {
                        FeatureChip(icon = "🔒", text = "Безопасно")
                        FeatureChip(icon = "⚡", text = "Быстро")
                        FeatureChip(icon = "☁️", text = "Облачно")
                    }
                }
            }

            Spacer(modifier = Modifier.height(32.dp))
        }
    }
}

@Composable
fun FeatureChip(icon: String, text: String) {
    Row(
        verticalAlignment = Alignment.CenterVertically,
        modifier = Modifier
            .background(
                color = TelegramColors.SurfaceVariant,
                shape = RoundedCornerShape(20.dp)
            )
            .padding(horizontal = 12.dp, vertical = 8.dp)
    ) {
        Text(text = icon, fontSize = 14.sp)
        Spacer(modifier = Modifier.width(6.dp))
        Text(
            text = text,
            style = MaterialTheme.typography.labelMedium,
            color = TelegramColors.TextSecondary
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun LoginScreen(
    onBack: () -> Unit,
    onContinue: (String, String) -> Unit
) {
    var selectedCountry by remember { mutableStateOf(countries[0]) }
    var phoneNumber by remember { mutableStateOf("") }
    var showCountryPicker by remember { mutableStateOf(false) }
    var syncContacts by remember { mutableStateOf(true) }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.Background)
    ) {
        // Top Bar
        TopAppBar(
            title = { },
            navigationIcon = {
                IconButton(onClick = onBack) {
                    Icon(
                        imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                        contentDescription = "Назад",
                        tint = TelegramColors.Primary
                    )
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(
                containerColor = Color.Transparent
            )
        )
        
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 32.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Spacer(modifier = Modifier.height(24.dp))

            // Иконка
            Box(
                modifier = Modifier
                    .size(100.dp)
                    .shadow(
                        elevation = 16.dp,
                        shape = CircleShape,
                        spotColor = TelegramColors.Primary.copy(alpha = 0.3f)
                    )
                    .clip(CircleShape)
                    .background(
                        brush = Brush.linearGradient(
                            colors = listOf(
                                TelegramColors.GradientStart,
                                TelegramColors.GradientEnd
                            )
                        )
                    ),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    imageVector = Icons.Outlined.Phone,
                    contentDescription = null,
                    tint = Color.White,
                    modifier = Modifier.size(48.dp)
                )
            }

            Spacer(modifier = Modifier.height(32.dp))

            Text(
                text = "Ваш телефон",
                style = MaterialTheme.typography.headlineLarge,
                color = TelegramColors.TextPrimary,
                fontWeight = FontWeight.Bold
            )

            Spacer(modifier = Modifier.height(12.dp))

            Text(
                text = "Подтвердите код страны и введите
свой номер телефона",
                style = MaterialTheme.typography.bodyMedium,
                color = TelegramColors.TextSecondary,
                textAlign = TextAlign.Center,
                lineHeight = 22.sp
            )

            Spacer(modifier = Modifier.height(40.dp))

            // Карточка ввода
            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .shadow(
                        elevation = 8.dp,
                        shape = RoundedCornerShape(20.dp),
                        spotColor = TelegramColors.CardShadow
                    ),
                shape = RoundedCornerShape(20.dp),
                colors = CardDefaults.cardColors(containerColor = TelegramColors.Surface)
            ) {
                Column {
                    // Выбор страны
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .clickable { showCountryPicker = true }
                            .padding(20.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Box(
                            modifier = Modifier
                                .size(44.dp)
                                .background(
                                    color = TelegramColors.SurfaceVariant,
                                    shape = CircleShape
                                ),
                            contentAlignment = Alignment.Center
                        ) {
                            Text(text = selectedCountry.flag, fontSize = 24.sp)
                        }
                        Spacer(modifier = Modifier.width(16.dp))
                        Column(modifier = Modifier.weight(1f)) {
                            Text(
                                text = "Страна",
                                style = MaterialTheme.typography.labelSmall,
                                color = TelegramColors.TextTertiary
                            )
                            Text(
                                text = selectedCountry.name,
                                style = MaterialTheme.typography.bodyLarge,
                                color = TelegramColors.TextPrimary,
                                fontWeight = FontWeight.Medium
                            )
                        }
                        Icon(
                            imageVector = Icons.Default.KeyboardArrowRight,
                            contentDescription = null,
                            tint = TelegramColors.TextTertiary
                        )
                    }

                    HorizontalDivider(
                        modifier = Modifier.padding(horizontal = 20.dp),
                        color = TelegramColors.Divider,
                        thickness = 1.dp
                    )

                    // Номер телефона
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(20.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Box(
                            modifier = Modifier
                                .background(
                                    color = TelegramColors.Primary.copy(alpha = 0.1f),
                                    shape = RoundedCornerShape(10.dp)
                                )
                                .clickable { showCountryPicker = true }
                                .padding(horizontal = 14.dp, vertical = 12.dp)
                        ) {
                            Text(
                                text = selectedCountry.phoneCode,
                                style = MaterialTheme.typography.bodyLarge,
                                color = TelegramColors.Primary,
                                fontWeight = FontWeight.Bold
                            )
                        }

                        Spacer(modifier = Modifier.width(12.dp))

                        BasicTextField(
                            value = phoneNumber,
                            onValueChange = { newValue ->
                                val filtered = newValue.filter { it.isDigit() }
                                if (filtered.length <= 15) {
                                    phoneNumber = filtered
                                }
                            },
                            modifier = Modifier
                                .weight(1f)
                                .height(48.dp),
                            textStyle = MaterialTheme.typography.bodyLarge.copy(
                                color = TelegramColors.TextPrimary,
                                fontWeight = FontWeight.Medium,
                                fontSize = 18.sp
                            ),
                            keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Phone),
                            singleLine = true,
                            cursorBrush = SolidColor(TelegramColors.Primary),
                            decorationBox = { innerTextField ->
                                Box(
                                    modifier = Modifier.fillMaxSize(),
                                    contentAlignment = Alignment.CenterStart
                                ) {
                                    if (phoneNumber.isEmpty()) {
                                        Text(
                                            text = "Номер телефона",
                                            style = MaterialTheme.typography.bodyLarge,
                                            color = TelegramColors.TextTertiary
                                        )
                                    }
                                    innerTextField()
                                }
                            }
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(24.dp))

            // Чекбокс
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(12.dp))
                    .clickable(
                        indication = null,
                        interactionSource = remember { MutableInteractionSource() }
                    ) { syncContacts = !syncContacts }
                    .padding(vertical = 8.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Checkbox(
                    checked = syncContacts,
                    onCheckedChange = { syncContacts = it },
                    colors = CheckboxDefaults.colors(
                        checkedColor = TelegramColors.Primary,
                        uncheckedColor = TelegramColors.TextTertiary,
                        checkmarkColor = Color.White
                    )
                )
                Spacer(modifier = Modifier.width(8.dp))
                Text(
                    text = "Синхронизировать контакты",
                    style = MaterialTheme.typography.bodyMedium,
                    color = TelegramColors.TextPrimary
                )
            }

            Spacer(modifier = Modifier.weight(1f))

            // Кнопка
            Button(
                onClick = { onContinue(phoneNumber, selectedCountry.phoneCode) },
                modifier = Modifier
                    .fillMaxWidth()
                    .height(56.dp)
                    .shadow(
                        elevation = if (phoneNumber.length >= 6) 12.dp else 0.dp,
                        shape = RoundedCornerShape(16.dp),
                        spotColor = TelegramColors.Primary.copy(alpha = 0.4f)
                    ),
                shape = RoundedCornerShape(16.dp),
                colors = ButtonDefaults.buttonColors(
                    containerColor = TelegramColors.Primary,
                    disabledContainerColor = TelegramColors.Primary.copy(alpha = 0.4f)
                ),
                enabled = phoneNumber.length >= 6
            ) {
                Text(
                    text = "Продолжить",
                    style = MaterialTheme.typography.titleMedium,
                    color = Color.White,
                    fontWeight = FontWeight.SemiBold
                )
            }

            Spacer(modifier = Modifier.height(20.dp))

            // QR вход
            TextButton(
                onClick = { /* TODO */ },
                modifier = Modifier.fillMaxWidth()
            ) {
                Icon(
                    imageVector = Icons.Outlined.QrCode,
                    contentDescription = null,
                    tint = TelegramColors.Link,
                    modifier = Modifier.size(20.dp)
                )
                Spacer(modifier = Modifier.width(8.dp))
                Text(
                    text = "Войти по QR-коду",
                    style = MaterialTheme.typography.bodyMedium,
                    color = TelegramColors.Link,
                    fontWeight = FontWeight.Medium
                )
            }

            Spacer(modifier = Modifier.height(32.dp))
        }
    }

    if (showCountryPicker) {
        CountryPickerDialog(
            countries = countries,
            selectedCountry = selectedCountry,
            onCountrySelected = { country ->
                selectedCountry = country
                showCountryPicker = false
            },
            onDismiss = { showCountryPicker = false }
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CodeVerificationScreen(
    phoneNumber: String,
    countryCode: String,
    onBack: () -> Unit,
    onCodeVerified: (Boolean) -> Unit
) {
    var code by remember { mutableStateOf(List(5) { "" }) }
    var isError by remember { mutableStateOf(false) }
    var isLoading by remember { mutableStateOf(false) }
    var countdown by remember { mutableIntStateOf(60) }
    val focusRequesters = remember { List(5) { FocusRequester() } }

    LaunchedEffect(countdown) {
        if (countdown > 0) {
            delay(1000)
            countdown--
        }
    }

    LaunchedEffect(Unit) {
        delay(300)
        focusRequesters[0].requestFocus()
    }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.Background)
    ) {
        // Top Bar
        TopAppBar(
            title = { },
            navigationIcon = {
                IconButton(onClick = onBack) {
                    Icon(
                        imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                        contentDescription = "Назад",
                        tint = TelegramColors.Primary
                    )
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(
                containerColor = Color.Transparent
            )
        )

        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 32.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Spacer(modifier = Modifier.height(24.dp))

            // Иконка
            Box(
                modifier = Modifier
                    .size(100.dp)
                    .shadow(
                        elevation = 16.dp,
                        shape = CircleShape,
                        spotColor = TelegramColors.Primary.copy(alpha = 0.3f)
                    )
                    .clip(CircleShape)
                    .background(
                        brush = Brush.linearGradient(
                            colors = listOf(
                                TelegramColors.GradientStart,
                                TelegramColors.GradientEnd
                            )
                        )
                    ),
                contentAlignment = Alignment.Center
            ) {
                Text(text = "💬", fontSize = 48.sp)
            }

            Spacer(modifier = Modifier.height(32.dp))

            Text(
                text = "$countryCode $phoneNumber",
                style = MaterialTheme.typography.headlineMedium,
                color = TelegramColors.TextPrimary,
                fontWeight = FontWeight.Bold
            )

            Spacer(modifier = Modifier.height(12.dp))

            Text(
                text = "Мы отправили SMS с кодом
на указанный номер",
                style = MaterialTheme.typography.bodyMedium,
                color = TelegramColors.TextSecondary,
                textAlign = TextAlign.Center
            )

            Spacer(modifier = Modifier.height(48.dp))

            // Поля кода
            Row(
                horizontalArrangement = Arrangement.spacedBy(10.dp),
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Spacer(modifier = Modifier.weight(1f))
                code.forEachIndexed { index, digit ->
                    CodeDigitField(
                        value = digit,
                        onValueChange = { newValue ->
                            if (newValue.length <= 1 && newValue.all { it.isDigit() }) {
                                isError = false
                                val newCode = code.toMutableList()
                                newCode[index] = newValue
                                code = newCode

                                if (newValue.isNotEmpty() && index < 4) {
                                    focusRequesters[index + 1].requestFocus()
                                }

                                if (newCode.all { it.isNotEmpty() }) {
                                    isLoading = true
                                    val enteredCode = newCode.joinToString("")
                                    if (enteredCode == "12345") {
                                        onCodeVerified(true)
                                    } else if (enteredCode == "11111") {
                                        onCodeVerified(false)
                                    } else {
                                        isLoading = false
                                        isError = true
                                    }
                                }
                            }
                        },
                        isError = isError,
                        isFilled = digit.isNotEmpty(),
                        focusRequester = focusRequesters[index],
                        onBackspace = {
                            if (digit.isEmpty() && index > 0) {
                                focusRequesters[index - 1].requestFocus()
                                val newCode = code.toMutableList()
                                newCode[index - 1] = ""
                                code = newCode
                            }
                        }
                    )
                }
                Spacer(modifier = Modifier.weight(1f))
            }

            if (isError) {
                Spacer(modifier = Modifier.height(20.dp))
                Card(
                    shape = RoundedCornerShape(12.dp),
                    colors = CardDefaults.cardColors(
                        containerColor = TelegramColors.ErrorLight
                    )
                ) {
                    Row(
                        modifier = Modifier.padding(horizontal = 16.dp, vertical = 10.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Icon(
                            imageVector = Icons.Default.Error,
                            contentDescription = null,
                            tint = TelegramColors.Error,
                            modifier = Modifier.size(18.dp)
                        )
                        Spacer(modifier = Modifier.width(8.dp))
                        Text(
                            text = "Неверный код",
                            style = MaterialTheme.typography.bodySmall,
                            color = TelegramColors.Error,
                            fontWeight = FontWeight.Medium
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(32.dp))

            // Таймер
            if (countdown > 0) {
                Text(
                    text = "Запросить код повторно через $countdown сек",
                    style = MaterialTheme.typography.bodyMedium,
                    color = TelegramColors.TextTertiary
                )
            } else {
                TextButton(
                    onClick = {
                        countdown = 60
                        code = List(5) { "" }
                        isError = false
                        focusRequesters[0].requestFocus()
                    }
                ) {
                    Icon(
                        imageVector = Icons.Default.Refresh,
                        contentDescription = null,
                        tint = TelegramColors.Link,
                        modifier = Modifier.size(18.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(
                        text = "Отправить код повторно",
                        style = MaterialTheme.typography.bodyMedium,
                        color = TelegramColors.Link,
                        fontWeight = FontWeight.Medium
                    )
                }
            }

            Spacer(modifier = Modifier.height(12.dp))

            TextButton(onClick = { /* TODO */ }) {
                Icon(
                    imageVector = Icons.Outlined.Phone,
                    contentDescription = null,
                    tint = TelegramColors.Link,
                    modifier = Modifier.size(18.dp)
                )
                Spacer(modifier = Modifier.width(8.dp))
                Text(
                    text = "Позвонить на номер",
                    style = MaterialTheme.typography.bodyMedium,
                    color = TelegramColors.Link,
                    fontWeight = FontWeight.Medium
                )
            }

            if (isLoading) {
                Spacer(modifier = Modifier.height(32.dp))
                CircularProgressIndicator(
                    color = TelegramColors.Primary,
                    modifier = Modifier.size(40.dp),
                    strokeWidth = 3.dp
                )
            }

            Spacer(modifier = Modifier.weight(1f))

            // Подсказка
            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(16.dp),
                colors = CardDefaults.cardColors(
                    containerColor = TelegramColors.Primary.copy(alpha = 0.08f)
                )
            ) {
                Row(
                    modifier = Modifier.padding(16.dp),
                    verticalAlignment = Alignment.Top
                ) {
                    Box(
                        modifier = Modifier
                            .size(32.dp)
                            .background(
                                color = TelegramColors.Primary.copy(alpha = 0.15f),
                                shape = CircleShape
                            ),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(
                            imageVector = Icons.Outlined.Lightbulb,
                            contentDescription = null,
                            tint = TelegramColors.Primary,
                            modifier = Modifier.size(18.dp)
                        )
                    }
                    Spacer(modifier = Modifier.width(12.dp))
                    Column {
                        Text(
                            text = "Тестовые коды",
                            style = MaterialTheme.typography.labelLarge,
                            color = TelegramColors.Primary,
                            fontWeight = FontWeight.SemiBold
                        )
                        Spacer(modifier = Modifier.height(4.dp))
                        Text(
                            text = "12345 — новый пользователь\n11111 — существующий",
                            style = MaterialTheme.typography.bodySmall,
                            color = TelegramColors.Primary.copy(alpha = 0.8f)
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(32.dp))
        }
    }
}

@Composable
fun CodeDigitField(
    value: String,
    onValueChange: (String) -> Unit,
    isError: Boolean,
    isFilled: Boolean,
    focusRequester: FocusRequester,
    onBackspace: () -> Unit
) {
    val animatedBorderColor by animateColorAsState(
        targetValue = when {
            isError -> TelegramColors.Error
            isFilled -> TelegramColors.Primary
            else -> TelegramColors.Divider
        },
        animationSpec = tween(200),
        label = "border_color"
    )
    
    val animatedScale by animateFloatAsState(
        targetValue = if (isFilled) 1.05f else 1f,
        animationSpec = spring(dampingRatio = Spring.DampingRatioMediumBouncy),
        label = "scale"
    )

    BasicTextField(
        value = value,
        onValueChange = onValueChange,
        modifier = Modifier
            .size(56.dp)
            .scale(animatedScale)
            .shadow(
                elevation = if (isFilled) 8.dp else 2.dp,
                shape = RoundedCornerShape(14.dp),
                spotColor = if (isFilled) TelegramColors.Primary.copy(alpha = 0.3f) else TelegramColors.CardShadow
            )
            .clip(RoundedCornerShape(14.dp))
            .background(TelegramColors.Surface)
            .border(
                width = 2.dp,
                color = animatedBorderColor,
                shape = RoundedCornerShape(14.dp)
            )
            .focusRequester(focusRequester),
        textStyle = MaterialTheme.typography.headlineLarge.copy(
            color = TelegramColors.TextPrimary,
            textAlign = TextAlign.Center,
            fontWeight = FontWeight.Bold
        ),
        keyboardOptions = KeyboardOptions(
            keyboardType = KeyboardType.Number,
            imeAction = ImeAction.Next
        ),
        singleLine = true,
        cursorBrush = SolidColor(TelegramColors.Primary),
        decorationBox = { innerTextField ->
            Box(
                contentAlignment = Alignment.Center,
                modifier = Modifier.fillMaxSize()
            ) {
                innerTextField()
            }
        }
    )
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun RegistrationScreen(
    phoneNumber: String,
    userData: UserData,
    onUserDataChanged: (UserData) -> Unit,
    onBack: () -> Unit,
    onComplete: () -> Unit
) {
    var currentStep by remember { mutableIntStateOf(0) }
    val totalSteps = 3

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.Background)
    ) {
        // Top Bar
        TopAppBar(
            title = {
                Text(
                    text = "Шаг ${currentStep + 1} из $totalSteps",
                    style = MaterialTheme.typography.bodyMedium,
                    color = TelegramColors.TextSecondary
                )
            },
            navigationIcon = {
                IconButton(onClick = {
                    if (currentStep > 0) currentStep-- else onBack()
                }) {
                    Icon(
                        imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                        contentDescription = "Назад",
                        tint = TelegramColors.Primary
                    )
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(
                containerColor = Color.Transparent
            )
        )

        // Progress
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .height(4.dp)
                .padding(horizontal = 16.dp)
                .clip(RoundedCornerShape(2.dp))
                .background(TelegramColors.Divider)
        ) {
            val animatedProgress by animateFloatAsState(
                targetValue = (currentStep + 1).toFloat() / totalSteps,
                animationSpec = tween(400),
                label = "progress"
            )
            Box(
                modifier = Modifier
                    .fillMaxHeight()
                    .fillMaxWidth(animatedProgress)
                    .clip(RoundedCornerShape(2.dp))
                    .background(
                        brush = Brush.horizontalGradient(
                            colors = listOf(
                                TelegramColors.GradientStart,
                                TelegramColors.GradientEnd
                            )
                        )
                    )
            )
        }

        AnimatedContent(
            targetState = currentStep,
            transitionSpec = {
                if (targetState > initialState) {
                    slideInHorizontally { it } + fadeIn() togetherWith
                            slideOutHorizontally { -it } + fadeOut()
                } else {
                    slideInHorizontally { -it } + fadeIn() togetherWith
                            slideOutHorizontally { it } + fadeOut()
                }
            },
            label = "step_transition"
        ) { step ->
            when (step) {
                0 -> NameInputStep(
                    userData = userData,
                    onUserDataChanged = onUserDataChanged,
                    onNext = { currentStep++ }
                )
                1 -> BirthdayInputStep(
                    userData = userData,
                    onUserDataChanged = onUserDataChanged,
                    onNext = { currentStep++ }
                )
                2 -> ProfilePhotoStep(
                    userData = userData,
                    onComplete = onComplete
                )
            }
        }
    }
}

@Composable
fun NameInputStep(
    userData: UserData,
    onUserDataChanged: (UserData) -> Unit,
    onNext: () -> Unit
) {
    var firstName by remember { mutableStateOf(userData.firstName) }
    var lastName by remember { mutableStateOf(userData.lastName) }
    val focusManager = LocalFocusManager.current

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(horizontal = 32.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Spacer(modifier = Modifier.height(40.dp))

        // Иконка
        Box(
            modifier = Modifier
                .size(100.dp)
                .shadow(16.dp, CircleShape, spotColor = TelegramColors.Primary.copy(alpha = 0.3f))
                .clip(CircleShape)
                .background(
                    brush = Brush.linearGradient(
                        colors = listOf(TelegramColors.GradientStart, TelegramColors.GradientEnd)
                    )
                ),
            contentAlignment = Alignment.Center
        ) {
            Text(text = "👤", fontSize = 48.sp)
        }

        Spacer(modifier = Modifier.height(32.dp))

        Text(
            text = "Как вас зовут?",
            style = MaterialTheme.typography.headlineLarge,
            color = TelegramColors.TextPrimary,
            fontWeight = FontWeight.Bold
        )

        Spacer(modifier = Modifier.height(12.dp))

        Text(
            text = "Введите имя и фамилию",
            style = MaterialTheme.typography.bodyMedium,
            color = TelegramColors.TextSecondary
        )

        Spacer(modifier = Modifier.height(48.dp))

        // Поля
        OutlinedTextField(
            value = firstName,
            onValueChange = { firstName = it },
            modifier = Modifier.fillMaxWidth(),
            label = { Text("Имя") },
            placeholder = { Text("Ваше имя") },
            leadingIcon = {
                Icon(
                    imageVector = Icons.Outlined.Person,
                    contentDescription = null,
                    tint = TelegramColors.Primary
                )
            },
            shape = RoundedCornerShape(16.dp),
            colors = OutlinedTextFieldDefaults.colors(
                focusedBorderColor = TelegramColors.Primary,
                unfocusedBorderColor = TelegramColors.Divider,
                focusedContainerColor = TelegramColors.Surface,
                unfocusedContainerColor = TelegramColors.Surface
            ),
            keyboardOptions = KeyboardOptions(imeAction = ImeAction.Next),
            keyboardActions = KeyboardActions(
                onNext = { focusManager.moveFocus(FocusDirection.Down) }
            ),
            singleLine = true
        )

        Spacer(modifier = Modifier.height(16.dp))

        OutlinedTextField(
            value = lastName,
            onValueChange = { lastName = it },
            modifier = Modifier.fillMaxWidth(),
            label = { Text("Фамилия (необязательно)") },
            placeholder = { Text("Ваша фамилия") },
            leadingIcon = {
                Icon(
                    imageVector = Icons.Outlined.PersonOutline,
                    contentDescription = null,
                    tint = TelegramColors.Primary
                )
            },
            shape = RoundedCornerShape(16.dp),
            colors = OutlinedTextFieldDefaults.colors(
                focusedBorderColor = TelegramColors.Primary,
                unfocusedBorderColor = TelegramColors.Divider,
                focusedContainerColor = TelegramColors.Surface,
                unfocusedContainerColor = TelegramColors.Surface
            ),
            keyboardOptions = KeyboardOptions(imeAction = ImeAction.Done),
            keyboardActions = KeyboardActions(
                onDone = { focusManager.clearFocus() }
            ),
            singleLine = true
        )

        Spacer(modifier = Modifier.weight(1f))

        Button(
            onClick = {
                onUserDataChanged(userData.copy(firstName = firstName, lastName = lastName))
                onNext()
            },
            modifier = Modifier
                .fillMaxWidth()
                .height(56.dp)
                .shadow(
                    elevation = if (firstName.isNotBlank()) 12.dp else 0.dp,
                    shape = RoundedCornerShape(16.dp),
                    spotColor = TelegramColors.Primary.copy(alpha = 0.4f)
                ),
            shape = RoundedCornerShape(16.dp),
            colors = ButtonDefaults.buttonColors(
                containerColor = TelegramColors.Primary,
                disabledContainerColor = TelegramColors.Primary.copy(alpha = 0.4f)
            ),
            enabled = firstName.isNotBlank()
        ) {
            Text(
                text = "Продолжить",
                style = MaterialTheme.typography.titleMedium,
                color = Color.White,
                fontWeight = FontWeight.SemiBold
            )
        }

        Spacer(modifier = Modifier.height(32.dp))
    }
}

@Composable
fun BirthdayInputStep(
    userData: UserData,
    onUserDataChanged: (UserData) -> Unit,
    onNext: () -> Unit
) {
    var day by remember { mutableStateOf(userData.birthDay) }
    var month by remember { mutableStateOf(userData.birthMonth) }
    var year by remember { mutableStateOf(userData.birthYear) }
    var skipBirthday by remember { mutableStateOf(false) }
    val focusManager = LocalFocusManager.current

    val months = listOf(
        "Январь", "Февраль", "Март", "Апрель", "Май", "Июнь",
        "Июль", "Август", "Сентябрь", "Октябрь", "Ноябрь", "Декабрь"
    )

    var showMonthPicker by remember { mutableStateOf(false) }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(horizontal = 32.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Spacer(modifier = Modifier.height(40.dp))

        Box(
            modifier = Modifier
                .size(100.dp)
                .shadow(16.dp, CircleShape, spotColor = TelegramColors.Primary.copy(alpha = 0.3f))
                .clip(CircleShape)
                .background(
                    brush = Brush.linearGradient(
                        colors = listOf(TelegramColors.GradientStart, TelegramColors.GradientEnd)
                    )
                ),
            contentAlignment = Alignment.Center
        ) {
            Text(text = "🎂", fontSize = 48.sp)
        }

        Spacer(modifier = Modifier.height(32.dp))

        Text(
            text = "Дата рождения",
            style = MaterialTheme.typography.headlineLarge,
            color = TelegramColors.TextPrimary,
            fontWeight = FontWeight.Bold
        )

        Spacer(modifier = Modifier.height(12.dp))

        Text(
            text = "Это поможет друзьям найти вас",
            style = MaterialTheme.typography.bodyMedium,
            color = TelegramColors.TextSecondary
        )

        Spacer(modifier = Modifier.height(48.dp))

        // Поля даты
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            OutlinedTextField(
                value = day,
                onValueChange = {
                    if (it.length <= 2 && it.all { char -> char.isDigit() }) {
                        day = it
                        if (it.length == 2) focusManager.moveFocus(FocusDirection.Right)
                    }
                },
                modifier = Modifier.weight(1f),
                label = { Text("День") },
                placeholder = { Text("ДД") },
                shape = RoundedCornerShape(16.dp),
                colors = OutlinedTextFieldDefaults.colors(
                    focusedBorderColor = TelegramColors.Primary,
                    unfocusedBorderColor = TelegramColors.Divider,
                    focusedContainerColor = TelegramColors.Surface,
                    unfocusedContainerColor = TelegramColors.Surface
                ),
                keyboardOptions = KeyboardOptions(
                    keyboardType = KeyboardType.Number,
                    imeAction = ImeAction.Next
                ),
                singleLine = true,
                textStyle = MaterialTheme.typography.bodyLarge.copy(textAlign = TextAlign.Center)
            )

            Box(modifier = Modifier.weight(2f)) {
                OutlinedTextField(
                    value = if (month.isNotEmpty()) months[month.toInt() - 1] else "",
                    onValueChange = { },
                    modifier = Modifier.fillMaxWidth(),
                    label = { Text("Месяц") },
                    placeholder = { Text("Выбрать") },
                    readOnly = true,
                    enabled = false,
                    shape = RoundedCornerShape(16.dp),
                    colors = OutlinedTextFieldDefaults.colors(
                        disabledBorderColor = TelegramColors.Divider,
                        disabledContainerColor = TelegramColors.Surface,
                        disabledTextColor = TelegramColors.TextPrimary,
                        disabledLabelColor = TelegramColors.TextSecondary
                    ),
                    trailingIcon = {
                        Icon(
                            imageVector = Icons.Default.KeyboardArrowDown,
                            contentDescription = null,
                            tint = TelegramColors.TextSecondary
                        )
                    },
                    singleLine = true
                )
                Box(
                    modifier = Modifier
                        .matchParentSize()
                        .clickable { showMonthPicker = true }
                )
            }

            OutlinedTextField(
                value = year,
                onValueChange = {
                    if (it.length <= 4 && it.all { char -> char.isDigit() }) {
                        year = it
                    }
                },
                modifier = Modifier.weight(1.5f),
                label = { Text("Год") },
                placeholder = { Text("ГГГГ") },
                shape = RoundedCornerShape(16.dp),
                colors = OutlinedTextFieldDefaults.colors(
                    focusedBorderColor = TelegramColors.Primary,
                    unfocusedBorderColor = TelegramColors.Divider,
                    focusedContainerColor = TelegramColors.Surface,
                    unfocusedContainerColor = TelegramColors.Surface
                ),
                keyboardOptions = KeyboardOptions(
                    keyboardType = KeyboardType.Number,
                    imeAction = ImeAction.Done
                ),
                keyboardActions = KeyboardActions(
                    onDone = { focusManager.clearFocus() }
                ),
                singleLine = true,
                textStyle = MaterialTheme.typography.bodyLarge.copy(textAlign = TextAlign.Center)
            )
        }

        Spacer(modifier = Modifier.height(24.dp))

        Row(
            modifier = Modifier
                .fillMaxWidth()
                .clip(RoundedCornerShape(12.dp))
                .clickable(
                    indication = null,
                    interactionSource = remember { MutableInteractionSource() }
                ) { skipBirthday = !skipBirthday }
                .padding(vertical = 8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Checkbox(
                checked = skipBirthday,
                onCheckedChange = { skipBirthday = it },
                colors = CheckboxDefaults.colors(
                    checkedColor = TelegramColors.Primary,
                    uncheckedColor = TelegramColors.TextTertiary
                )
            )
            Spacer(modifier = Modifier.width(8.dp))
            Text(
                text = "Пропустить этот шаг",
                style = MaterialTheme.typography.bodyMedium,
                color = TelegramColors.TextPrimary
            )
        }

        Spacer(modifier = Modifier.weight(1f))

        Button(
            onClick = {
                onUserDataChanged(userData.copy(
                    birthDay = day,
                    birthMonth = month,
                    birthYear = year
                ))
                onNext()
            },
            modifier = Modifier
                .fillMaxWidth()
                .height(56.dp)
                .shadow(12.dp, RoundedCornerShape(16.dp), spotColor = TelegramColors.Primary.copy(alpha = 0.4f)),
            shape = RoundedCornerShape(16.dp),
            colors = ButtonDefaults.buttonColors(
                containerColor = TelegramColors.Primary,
                disabledContainerColor = TelegramColors.Primary.copy(alpha = 0.4f)
            ),
            enabled = skipBirthday || (day.isNotBlank() && month.isNotBlank() && year.length == 4)
        ) {
            Text(
                text = "Продолжить",
                style = MaterialTheme.typography.titleMedium,
                color = Color.White,
                fontWeight = FontWeight.SemiBold
            )
        }

        Spacer(modifier = Modifier.height(32.dp))
    }

    if (showMonthPicker) {
        MonthPickerDialog(
            months = months,
            selectedMonth = if (month.isNotEmpty()) month.toInt() - 1 else -1,
            onMonthSelected = { index ->
                month = (index + 1).toString()
                showMonthPicker = false
            },
            onDismiss = { showMonthPicker = false }
        )
    }
}

@Composable
fun MonthPickerDialog(
    months: List<String>,
    selectedMonth: Int,
    onMonthSelected: (Int) -> Unit,
    onDismiss: () -> Unit
) {
    Dialog(onDismissRequest = onDismiss) {
        Card(
            shape = RoundedCornerShape(24.dp),
            colors = CardDefaults.cardColors(containerColor = TelegramColors.Surface),
            modifier = Modifier.shadow(24.dp, RoundedCornerShape(24.dp))
        ) {
            Column(modifier = Modifier.padding(vertical = 16.dp)) {
                Text(
                    text = "Выберите месяц",
                    style = MaterialTheme.typography.titleLarge,
                    color = TelegramColors.TextPrimary,
                    fontWeight = FontWeight.Bold,
                    modifier = Modifier.padding(horizontal = 24.dp, vertical = 8.dp)
                )

                LazyColumn(
                    modifier = Modifier.heightIn(max = 400.dp)
                ) {
                    items(months.size) { index ->
                        val monthName = months[index]
                        Row(
                            modifier = Modifier
                                .fillMaxWidth()
                                .clickable { onMonthSelected(index) }
                                .background(
                                    if (index == selectedMonth) TelegramColors.Primary.copy(alpha = 0.1f)
                                    else Color.Transparent
                                )
                                .padding(horizontal = 24.dp, vertical = 16.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Text(
                                text = monthName,
                                style = MaterialTheme.typography.bodyLarge,
                                color = if (index == selectedMonth) TelegramColors.Primary else TelegramColors.TextPrimary,
                                fontWeight = if (index == selectedMonth) FontWeight.SemiBold else FontWeight.Normal,
                                modifier = Modifier.weight(1f)
                            )
                            if (index == selectedMonth) {
                                Icon(
                                    imageVector = Icons.Default.Check,
                                    contentDescription = null,
                                    tint = TelegramColors.Primary
                                )
                            }
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun ProfilePhotoStep(
    userData: UserData,
    onComplete: () -> Unit
) {
    var bio by remember { mutableStateOf(userData.bio) }
    var username by remember { mutableStateOf(userData.username) }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(horizontal = 32.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Spacer(modifier = Modifier.height(40.dp))

        // Аватар
        Box(contentAlignment = Alignment.Center) {
            Box(
                modifier = Modifier
                    .size(130.dp)
                    .shadow(20.dp, CircleShape, spotColor = TelegramColors.Primary.copy(alpha = 0.4f))
                    .clip(CircleShape)
                    .background(
                        brush = Brush.linearGradient(
                            colors = listOf(TelegramColors.GradientStart, TelegramColors.GradientEnd)
                        )
                    )
                    .clickable { },
                contentAlignment = Alignment.Center
            ) {
                if (userData.firstName.isNotEmpty()) {
                    Text(
                        text = userData.firstName.first().uppercase() +
                                (userData.lastName.firstOrNull()?.uppercase() ?: ""),
                        fontSize = 44.sp,
                        color = Color.White,
                        fontWeight = FontWeight.Bold
                    )
                } else {
                    Icon(
                        imageVector = Icons.Default.Person,
                        contentDescription = null,
                        tint = Color.White,
                        modifier = Modifier.size(64.dp)
                    )
                }
            }

            Box(
                modifier = Modifier
                    .align(Alignment.BottomEnd)
                    .offset(x = 4.dp, y = 4.dp)
                    .size(42.dp)
                    .shadow(8.dp, CircleShape)
                    .clip(CircleShape)
                    .background(TelegramColors.Primary)
                    .clickable { },
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    imageVector = Icons.Default.CameraAlt,
                    contentDescription = "Добавить фото",
                    tint = Color.White,
                    modifier = Modifier.size(22.dp)
                )
            }
        }

        Spacer(modifier = Modifier.height(32.dp))

        Text(
            text = "Ваш профиль",
            style = MaterialTheme.typography.headlineLarge,
            color = TelegramColors.TextPrimary,
            fontWeight = FontWeight.Bold
        )

        Spacer(modifier = Modifier.height(12.dp))

        Text(
            text = "Добавьте фото и информацию о себе",
            style = MaterialTheme.typography.bodyMedium,
            color = TelegramColors.TextSecondary
        )

        Spacer(modifier = Modifier.height(40.dp))

        OutlinedTextField(
            value = username,
            onValueChange = {
                val filtered = it.filter { char ->
                    char.isLetterOrDigit() || char == '_'
                }.lowercase()
                if (filtered.length <= 32) {
                    username = filtered
                }
            },
            modifier = Modifier.fillMaxWidth(),
            label = { Text("Имя пользователя") },
            placeholder = { Text("username") },
            prefix = { Text("@", color = TelegramColors.Primary, fontWeight = FontWeight.Bold) },
            leadingIcon = {
                Icon(
                    imageVector = Icons.Outlined.AlternateEmail,
                    contentDescription = null,
                    tint = TelegramColors.Primary
                )
            },
            shape = RoundedCornerShape(16.dp),
            colors = OutlinedTextFieldDefaults.colors(
                focusedBorderColor = TelegramColors.Primary,
                unfocusedBorderColor = TelegramColors.Divider,
                focusedContainerColor = TelegramColors.Surface,
                unfocusedContainerColor = TelegramColors.Surface
            ),
            supportingText = {
                Text(
                    text = "Мин. 5 символов • a-z, 0-9, _",
                    style = MaterialTheme.typography.bodySmall,
                    color = TelegramColors.TextTertiary
                )
            },
            singleLine = true
        )

        Spacer(modifier = Modifier.height(16.dp))

        OutlinedTextField(
            value = bio,
            onValueChange = { if (it.length <= 70) bio = it },
            modifier = Modifier.fillMaxWidth(),
            label = { Text("О себе") },
            placeholder = { Text("Расскажите о себе...") },
            leadingIcon = {
                Icon(
                    imageVector = Icons.Outlined.Edit,
                    contentDescription = null,
                    tint = TelegramColors.Primary
                )
            },
            shape = RoundedCornerShape(16.dp),
            colors = OutlinedTextFieldDefaults.colors(
                focusedBorderColor = TelegramColors.Primary,
                unfocusedBorderColor = TelegramColors.Divider,
                focusedContainerColor = TelegramColors.Surface,
                unfocusedContainerColor = TelegramColors.Surface
            ),
            supportingText = {
                Text(
                    text = "${bio.length}/70",
                    style = MaterialTheme.typography.bodySmall,
                    color = TelegramColors.TextTertiary,
                    modifier = Modifier.fillMaxWidth(),
                    textAlign = TextAlign.End
                )
            },
            maxLines = 3
        )

        Spacer(modifier = Modifier.weight(1f))

        Button(
            onClick = onComplete,
            modifier = Modifier
                .fillMaxWidth()
                .height(56.dp)
                .shadow(12.dp, RoundedCornerShape(16.dp), spotColor = TelegramColors.Primary.copy(alpha = 0.4f)),
            shape = RoundedCornerShape(16.dp),
            colors = ButtonDefaults.buttonColors(containerColor = TelegramColors.Primary)
        ) {
            Icon(
                imageVector = Icons.Default.Check,
                contentDescription = null,
                tint = Color.White
            )
            Spacer(modifier = Modifier.width(8.dp))
            Text(
                text = "Завершить",
                style = MaterialTheme.typography.titleMedium,
                color = Color.White,
                fontWeight = FontWeight.SemiBold
            )
        }

        Spacer(modifier = Modifier.height(12.dp))

        TextButton(onClick = onComplete) {
            Text(
                text = "Пропустить",
                style = MaterialTheme.typography.bodyMedium,
                color = TelegramColors.Link
            )
        }

        Spacer(modifier = Modifier.height(24.dp))
    }
}

@Composable
fun SuccessScreen(
    userData: UserData,
    onLogout: () -> Unit
) {
    var isVisible by remember { mutableStateOf(false) }
    
    LaunchedEffect(Unit) {
        delay(100)
        isVisible = true
    }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.Background)
            .padding(horizontal = 32.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        // Анимированная галочка
        AnimatedVisibility(
            visible = isVisible,
            enter = fadeIn(tween(500)) + scaleIn(tween(500), initialScale = 0.3f)
        ) {
            Box(
                modifier = Modifier
                    .size(130.dp)
                    .shadow(24.dp, CircleShape, spotColor = TelegramColors.Success.copy(alpha = 0.4f))
                    .clip(CircleShape)
                    .background(TelegramColors.Success),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    imageVector = Icons.Default.Check,
                    contentDescription = "Успех",
                    tint = Color.White,
                    modifier = Modifier.size(64.dp)
                )
            }
        }

        Spacer(modifier = Modifier.height(40.dp))

        AnimatedVisibility(
            visible = isVisible,
            enter = fadeIn(tween(500, delayMillis = 200)) + slideInVertically(tween(500, delayMillis = 200)) { it / 2 }
        ) {
            Text(
                text = "Добро пожаловать!",
                style = MaterialTheme.typography.headlineLarge,
                color = TelegramColors.TextPrimary,
                fontWeight = FontWeight.Bold
            )
        }

        Spacer(modifier = Modifier.height(24.dp))

        // Карточка профиля
        AnimatedVisibility(
            visible = isVisible,
            enter = fadeIn(tween(500, delayMillis = 300)) + slideInVertically(tween(500, delayMillis = 300)) { it / 2 }
        ) {
            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .shadow(16.dp, RoundedCornerShape(24.dp), spotColor = TelegramColors.CardShadow),
                shape = RoundedCornerShape(24.dp),
                colors = CardDefaults.cardColors(containerColor = TelegramColors.Surface)
            ) {
                Column(
                    modifier = Modifier.padding(28.dp),
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    Box(
                        modifier = Modifier
                            .size(90.dp)
                            .shadow(12.dp, CircleShape, spotColor = TelegramColors.Primary.copy(alpha = 0.3f))
                            .clip(CircleShape)
                            .background(
                                brush = Brush.linearGradient(
                                    colors = listOf(TelegramColors.GradientStart, TelegramColors.GradientEnd)
                                )
                            ),
                        contentAlignment = Alignment.Center
                    ) {
                        Text(
                            text = userData.firstName.firstOrNull()?.uppercase() ?: "?",
                            fontSize = 36.sp,
                            color = Color.White,
                            fontWeight = FontWeight.Bold
                        )
                    }

                    Spacer(modifier = Modifier.height(20.dp))

                    Text(
                        text = "${userData.firstName} ${userData.lastName}".trim(),
                        style = MaterialTheme.typography.headlineMedium,
                        color = TelegramColors.TextPrimary,
                        fontWeight = FontWeight.Bold
                    )

                    if (userData.username.isNotEmpty()) {
                        Spacer(modifier = Modifier.height(4.dp))
                        Text(
                            text = "@${userData.username}",
                            style = MaterialTheme.typography.bodyLarge,
                            color = TelegramColors.Link,
                            fontWeight = FontWeight.Medium
                        )
                    }

                    if (userData.birthDay.isNotEmpty() && userData.birthMonth.isNotEmpty()) {
                        Spacer(modifier = Modifier.height(12.dp))
                        val months = listOf(
                            "января", "февраля", "марта", "апреля", "мая", "июня",
                            "июля", "августа", "сентября", "октября", "ноября", "декабря"
                        )
                        val monthName = months.getOrElse(userData.birthMonth.toInt() - 1) { "" }
                        Row(
                            verticalAlignment = Alignment.CenterVertically,
                            modifier = Modifier
                                .background(TelegramColors.SurfaceVariant, RoundedCornerShape(12.dp))
                                .padding(horizontal = 12.dp, vertical = 8.dp)
                        ) {
                            Text(text = "🎂", fontSize = 16.sp)
                            Spacer(modifier = Modifier.width(8.dp))
                            Text(
                                text = "${userData.birthDay} $monthName ${userData.birthYear}",
                                style = MaterialTheme.typography.bodyMedium,
                                color = TelegramColors.TextSecondary
                            )
                        }
                    }

                    if (userData.bio.isNotEmpty()) {
                        Spacer(modifier = Modifier.height(16.dp))
                        Text(
                            text = userData.bio,
                            style = MaterialTheme.typography.bodyMedium,
                            color = TelegramColors.TextSecondary,
                            textAlign = TextAlign.Center
                        )
                    }
                }
            }
        }

        Spacer(modifier = Modifier.height(20.dp))

        AnimatedVisibility(
            visible = isVisible,
            enter = fadeIn(tween(500, delayMillis = 400))
        ) {
            Text(
                text = "Аккаунт успешно создан!\nТеперь можно начать общение",
                style = MaterialTheme.typography.bodyMedium,
                color = TelegramColors.TextSecondary,
                textAlign = TextAlign.Center
            )
        }

        Spacer(modifier = Modifier.height(40.dp))

        AnimatedVisibility(
            visible = isVisible,
            enter = fadeIn(tween(500, delayMillis = 500)) + slideInVertically(tween(500, delayMillis = 500)) { it }
        ) {
            Column {
                Button(
                    onClick = { },
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(56.dp)
                        .shadow(12.dp, RoundedCornerShape(16.dp), spotColor = TelegramColors.Primary.copy(alpha = 0.4f)),
                    shape = RoundedCornerShape(16.dp),
                    colors = ButtonDefaults.buttonColors(containerColor = TelegramColors.Primary)
                ) {
                    Icon(
                        imageVector = Icons.AutoMirrored.Filled.Chat,
                        contentDescription = null,
                        tint = Color.White
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(
                        text = "Начать общение",
                        style = MaterialTheme.typography.titleMedium,
                        color = Color.White,
                        fontWeight = FontWeight.SemiBold
                    )
                }

                Spacer(modifier = Modifier.height(16.dp))

                TextButton(
                    onClick = onLogout,
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Icon(
                        imageVector = Icons.Default.Logout,
                        contentDescription = null,
                        tint = TelegramColors.Error,
                        modifier = Modifier.size(18.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(
                        text = "Выйти из аккаунта",
                        style = MaterialTheme.typography.bodyMedium,
                        color = TelegramColors.Error
                    )
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CountryPickerDialog(
    countries: List<Country>,
    selectedCountry: Country,
    onCountrySelected: (Country) -> Unit,
    onDismiss: () -> Unit
) {
    var searchQuery by remember { mutableStateOf("") }

    val filteredCountries = remember(searchQuery) {
        if (searchQuery.isEmpty()) {
            countries
        } else {
            countries.filter { country ->
                country.name.contains(searchQuery, ignoreCase = true) ||
                        country.phoneCode.contains(searchQuery)
            }
        }
    }

    Dialog(
        onDismissRequest = onDismiss,
        properties = DialogProperties(usePlatformDefaultWidth = false)
    ) {
        Surface(
            modifier = Modifier
                .fillMaxSize()
                .padding(top = 48.dp),
            shape = RoundedCornerShape(topStart = 24.dp, topEnd = 24.dp),
            color = TelegramColors.Background
        ) {
            Column {
                // Header
                Surface(
                    color = TelegramColors.Surface,
                    shadowElevation = 4.dp
                ) {
                    Column(modifier = Modifier.padding(20.dp)) {
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.SpaceBetween,
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Text(
                                text = "Выберите страну",
                                style = MaterialTheme.typography.headlineSmall,
                                color = TelegramColors.TextPrimary,
                                fontWeight = FontWeight.Bold
                            )
                            TextButton(onClick = onDismiss) {
                                Text(
                                    text = "Отмена",
                                    color = TelegramColors.Link,
                                    fontWeight = FontWeight.Medium
                                )
                            }
                        }

                        Spacer(modifier = Modifier.height(16.dp))

                        OutlinedTextField(
                            value = searchQuery,
                            onValueChange = { searchQuery = it },
                            modifier = Modifier.fillMaxWidth(),
                            placeholder = {
                                Text(text = "Поиск страны", color = TelegramColors.TextTertiary)
                            },
                            leadingIcon = {
                                Icon(
                                    imageVector = Icons.Default.Search,
                                    contentDescription = "Поиск",
                                    tint = TelegramColors.TextTertiary
                                )
                            },
                            shape = RoundedCornerShape(14.dp),
                            colors = OutlinedTextFieldDefaults.colors(
                                focusedBorderColor = TelegramColors.Primary,
                                unfocusedBorderColor = TelegramColors.Divider,
                                focusedContainerColor = TelegramColors.SurfaceVariant,
                                unfocusedContainerColor = TelegramColors.SurfaceVariant
                            ),
                            singleLine = true
                        )
                    }
                }

                LazyColumn(modifier = Modifier.fillMaxSize()) {
                    items(filteredCountries) { country ->
                        CountryItem(
                            country = country,
                            isSelected = country == selectedCountry,
                            onClick = { onCountrySelected(country) }
                        )
                    }
                }
            }
        }
    }
}

@Composable
fun CountryItem(
    country: Country,
    isSelected: Boolean,
    onClick: () -> Unit
) {
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick),
        color = if (isSelected) TelegramColors.Primary.copy(alpha = 0.08f) else TelegramColors.Surface
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 20.dp, vertical = 16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Box(
                modifier = Modifier
                    .size(44.dp)
                    .background(TelegramColors.SurfaceVariant, CircleShape),
                contentAlignment = Alignment.Center
            ) {
                Text(text = country.flag, fontSize = 24.sp)
            }

            Spacer(modifier = Modifier.width(16.dp))

            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = country.name,
                    style = MaterialTheme.typography.bodyLarge,
                    color = if (isSelected) TelegramColors.Primary else TelegramColors.TextPrimary,
                    fontWeight = if (isSelected) FontWeight.SemiBold else FontWeight.Normal
                )
            }

            Text(
                text = country.phoneCode,
                style = MaterialTheme.typography.bodyMedium,
                color = TelegramColors.TextSecondary,
                fontWeight = FontWeight.Medium
            )

            if (isSelected) {
                Spacer(modifier = Modifier.width(12.dp))
                Box(
                    modifier = Modifier
                        .size(24.dp)
                        .background(TelegramColors.Primary, CircleShape),
                    contentAlignment = Alignment.Center
                ) {
                    Icon(
                        imageVector = Icons.Default.Check,
                        contentDescription = "Выбрано",
                        tint = Color.White,
                        modifier = Modifier.size(16.dp)
                    )
                }
            }
        }
    }

    HorizontalDivider(
        modifier = Modifier.padding(start = 80.dp),
        color = TelegramColors.Divider,
        thickness = 0.5.dp
    )
}

private val EaseInOutCubic = CubicBezierEasing(0.65f, 0f, 0.35f, 1f)
