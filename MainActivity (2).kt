package com.example.myapplication

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.BasicTextField
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.KeyboardArrowDown
import androidx.compose.material.icons.filled.Search
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.SolidColor
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import com.example.myapplication.ui.theme.MyApplicationTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            MyApplicationTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    LoginScreen()
                }
            }
        }
    }
}

// Модель данных для страны
data class Country(
    val name: String,
    val code: String,
    val flag: String,
    val phoneCode: String
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

// Telegram цвета
object TelegramColors {
    val Primary = Color(0xFF5288C1)
    val PrimaryLight = Color(0xFF64B5F6)
    val Background = Color(0xFFF5F5F5)
    val Surface = Color.White
    val TextPrimary = Color(0xFF222222)
    val TextSecondary = Color(0xFF8E8E93)
    val Divider = Color(0xFFE5E5EA)
    val Link = Color(0xFF007AFF)
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun LoginScreen() {
    var selectedCountry by remember { mutableStateOf(countries[0]) }
    var phoneNumber by remember { mutableStateOf("") }
    var showCountryPicker by remember { mutableStateOf(false) }
    var syncContacts by remember { mutableStateOf(true) }
    
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.Background)
            .padding(horizontal = 32.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Spacer(modifier = Modifier.height(80.dp))
        
        // Логотип Telegram (круг с самолётиком)
        Box(
            modifier = Modifier
                .size(100.dp)
                .clip(CircleShape)
                .background(
                    brush = androidx.compose.ui.graphics.Brush.linearGradient(
                        colors = listOf(
                            Color(0xFF6CB3E8),
                            Color(0xFF3E99D6)
                        )
                    )
                ),
            contentAlignment = Alignment.Center
        ) {
            Text(
                text = "✈",
                fontSize = 48.sp,
                color = Color.White
            )
        }
        
        Spacer(modifier = Modifier.height(24.dp))
        
        // Заголовок
        Text(
            text = "Ваш телефон",
            style = MaterialTheme.typography.headlineLarge,
            color = TelegramColors.TextPrimary,
            fontWeight = FontWeight.Bold
        )
        
        Spacer(modifier = Modifier.height(12.dp))
        
        // Описание
        Text(
            text = "Пожалуйста, подтвердите код страны\nи введите свой номер телефона.",
            style = MaterialTheme.typography.bodyMedium,
            color = TelegramColors.TextSecondary,
            textAlign = TextAlign.Center,
            lineHeight = 20.sp
        )
        
        Spacer(modifier = Modifier.height(40.dp))
        
        // Карточка с полями ввода
        Card(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(12.dp),
            colors = CardDefaults.cardColors(
                containerColor = TelegramColors.Surface
            ),
            elevation = CardDefaults.cardElevation(
                defaultElevation = 0.dp
            )
        ) {
            Column {
                // Выбор страны
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clickable { showCountryPicker = true }
                        .padding(horizontal = 16.dp, vertical = 14.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = selectedCountry.flag,
                        fontSize = 24.sp
                    )
                    
                    Spacer(modifier = Modifier.width(12.dp))
                    
                    Text(
                        text = selectedCountry.name,
                        style = MaterialTheme.typography.bodyLarge,
                        color = TelegramColors.TextPrimary,
                        modifier = Modifier.weight(1f)
                    )
                    
                    Icon(
                        imageVector = Icons.Default.KeyboardArrowDown,
                        contentDescription = "Выбрать страну",
                        tint = TelegramColors.TextSecondary
                    )
                }
                
                // Разделитель
                HorizontalDivider(
                    modifier = Modifier.padding(start = 52.dp),
                    color = TelegramColors.Divider,
                    thickness = 0.5.dp
                )
                
                // Ввод номера телефона
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 16.dp, vertical = 14.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    // Код страны
                    Box(
                        modifier = Modifier
                            .width(60.dp)
                            .clickable { showCountryPicker = true }
                    ) {
                        Text(
                            text = selectedCountry.phoneCode,
                            style = MaterialTheme.typography.bodyLarge,
                            color = TelegramColors.Primary,
                            fontWeight = FontWeight.Medium
                        )
                    }
                    
                    Spacer(modifier = Modifier.width(8.dp))
                    
                    // Поле ввода номера
                    BasicTextField(
                        value = phoneNumber,
                        onValueChange = { newValue ->
                            // Фильтруем только цифры
                            val filtered = newValue.filter { it.isDigit() }
                            if (filtered.length <= 15) {
                                phoneNumber = filtered
                            }
                        },
                        modifier = Modifier
                            .weight(1f)
                            .height(24.dp),
                        textStyle = MaterialTheme.typography.bodyLarge.copy(
                            color = TelegramColors.TextPrimary
                        ),
                        keyboardOptions = KeyboardOptions(
                            keyboardType = KeyboardType.Phone
                        ),
                        singleLine = true,
                        cursorBrush = SolidColor(TelegramColors.Primary),
                        decorationBox = { innerTextField ->
                            Box(
                                modifier = Modifier.fillMaxWidth(),
                                contentAlignment = Alignment.CenterStart
                            ) {
                                if (phoneNumber.isEmpty()) {
                                    Text(
                                        text = "Номер телефона",
                                        style = MaterialTheme.typography.bodyLarge,
                                        color = TelegramColors.TextSecondary
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
        
        // Чекбокс синхронизации контактов
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .clickable(
                    indication = null,
                    interactionSource = remember { MutableInteractionSource() }
                ) { syncContacts = !syncContacts },
            verticalAlignment = Alignment.CenterVertically
        ) {
            Checkbox(
                checked = syncContacts,
                onCheckedChange = { syncContacts = it },
                colors = CheckboxDefaults.colors(
                    checkedColor = TelegramColors.Primary,
                    uncheckedColor = TelegramColors.TextSecondary
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
        
        // Кнопка продолжить
        Button(
            onClick = { /* TODO: Отправить код подтверждения */ },
            modifier = Modifier
                .fillMaxWidth()
                .height(50.dp),
            shape = RoundedCornerShape(10.dp),
            colors = ButtonDefaults.buttonColors(
                containerColor = TelegramColors.Primary
            ),
            enabled = phoneNumber.length >= 6
        ) {
            Text(
                text = "Продолжить",
                style = MaterialTheme.typography.titleMedium,
                color = Color.White
            )
        }
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Ссылка на вход по QR-коду
        Text(
            text = "Войти по QR-коду",
            style = MaterialTheme.typography.bodyMedium,
            color = TelegramColors.Link,
            modifier = Modifier
                .clickable { /* TODO: Открыть экран QR */ }
                .padding(vertical = 8.dp)
        )
        
        Spacer(modifier = Modifier.height(32.dp))
    }
    
    // Диалог выбора страны
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
        properties = DialogProperties(
            usePlatformDefaultWidth = false
        )
    ) {
        Surface(
            modifier = Modifier
                .fillMaxSize()
                .padding(top = 48.dp),
            shape = RoundedCornerShape(topStart = 16.dp, topEnd = 16.dp),
            color = TelegramColors.Background
        ) {
            Column {
                // Заголовок и поиск
                Surface(
                    color = TelegramColors.Surface,
                    shadowElevation = 2.dp
                ) {
                    Column(
                        modifier = Modifier.padding(16.dp)
                    ) {
                        // Заголовок
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.SpaceBetween,
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Text(
                                text = "Выберите страну",
                                style = MaterialTheme.typography.headlineMedium,
                                color = TelegramColors.TextPrimary,
                                fontWeight = FontWeight.Bold
                            )
                            
                            TextButton(onClick = onDismiss) {
                                Text(
                                    text = "Отмена",
                                    color = TelegramColors.Link
                                )
                            }
                        }
                        
                        Spacer(modifier = Modifier.height(12.dp))
                        
                        // Поле поиска
                        OutlinedTextField(
                            value = searchQuery,
                            onValueChange = { searchQuery = it },
                            modifier = Modifier.fillMaxWidth(),
                            placeholder = {
                                Text(
                                    text = "Поиск",
                                    color = TelegramColors.TextSecondary
                                )
                            },
                            leadingIcon = {
                                Icon(
                                    imageVector = Icons.Default.Search,
                                    contentDescription = "Поиск",
                                    tint = TelegramColors.TextSecondary
                                )
                            },
                            shape = RoundedCornerShape(10.dp),
                            colors = OutlinedTextFieldDefaults.colors(
                                focusedBorderColor = TelegramColors.Primary,
                                unfocusedBorderColor = TelegramColors.Divider,
                                focusedContainerColor = TelegramColors.Surface,
                                unfocusedContainerColor = TelegramColors.Surface
                            ),
                            singleLine = true
                        )
                    }
                }
                
                // Список стран
                LazyColumn(
                    modifier = Modifier.fillMaxSize()
                ) {
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
        color = if (isSelected) {
            TelegramColors.Primary.copy(alpha = 0.1f)
        } else {
            TelegramColors.Surface
        }
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp, vertical = 14.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = country.flag,
                fontSize = 28.sp
            )
            
            Spacer(modifier = Modifier.width(16.dp))
            
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = country.name,
                    style = MaterialTheme.typography.bodyLarge,
                    color = TelegramColors.TextPrimary,
                    fontWeight = if (isSelected) FontWeight.SemiBold else FontWeight.Normal
                )
            }
            
            Text(
                text = country.phoneCode,
                style = MaterialTheme.typography.bodyMedium,
                color = TelegramColors.TextSecondary
            )
            
            if (isSelected) {
                Spacer(modifier = Modifier.width(12.dp))
                Icon(
                    imageVector = Icons.Default.Check,
                    contentDescription = "Выбрано",
                    tint = TelegramColors.Primary,
                    modifier = Modifier.size(20.dp)
                )
            }
        }
    }
    
    HorizontalDivider(
        modifier = Modifier.padding(start = 60.dp),
        color = TelegramColors.Divider,
        thickness = 0.5.dp
    )
}
