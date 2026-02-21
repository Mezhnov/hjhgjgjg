package com.example.myapplication

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.compose.BackHandler
import androidx.compose.animation.*
import androidx.compose.animation.core.*
import androidx.compose.foundation.*
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material.icons.outlined.*
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.blur
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.drawBehind
import androidx.compose.ui.draw.rotate
import androidx.compose.ui.draw.scale
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.Shadow
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.IntOffset
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Popup
import androidx.compose.ui.window.PopupProperties
import com.example.myapplication.ui.theme.MyApplicationTheme
import com.example.myapplication.ui.theme.TelegramColors
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.*
import kotlin.math.absoluteValue

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MyApplicationTheme {
                TelegramChatApp()
            }
        }
    }
}

// Навигация
sealed class Screen {
    object ChatList : Screen()
    data class Chat(val chat: ChatData) : Screen()
    object Profile : Screen()
    object Settings : Screen()
    object Contacts : Screen()
    object Calls : Screen()
    object SavedMessages : Screen()
    object ArchivedChats : Screen()
    object CreateGroup : Screen()
    object NewChannel : Screen()
}

// Модель данных для чата
data class ChatData(
    val id: Int,
    val name: String,
    val lastMessage: String,
    val time: String,
    val unreadCount: Int = 0,
    val isOnline: Boolean = false,
    val isPinned: Boolean = false,
    val isMuted: Boolean = false,
    val isVerified: Boolean = false,
    val isChannel: Boolean = false,
    val isGroup: Boolean = false,
    val isBot: Boolean = false,
    val avatarColor: Color,
    val avatarGradient: List<Color>? = null,
    val lastMessageSender: String? = null,
    val isTyping: Boolean = false,
    val draft: String? = null,
    val hasStory: Boolean = false,
    val storyUnseen: Boolean = false
)

// Модель данных для сообщения
data class Message(
    val id: Int,
    val text: String,
    val time: String,
    val isFromMe: Boolean,
    val isRead: Boolean = true,
    val replyTo: Message? = null,
    val forwardedFrom: String? = null,
    val isEdited: Boolean = false,
    val reactions: List<Reaction> = emptyList(),
    val isVoice: Boolean = false,
    val voiceDuration: String? = null,
    val imageUrl: String? = null,
    val isSticker: Boolean = false,
    val stickerEmoji: String? = null
)

data class Reaction(
    val emoji: String,
    val count: Int,
    val isSelected: Boolean = false
)

// Пункты меню боковой панели
data class DrawerMenuItem(
    val icon: ImageVector,
    val title: String,
    val badge: String? = null,
    val badgeColor: Color = Color(0xFF5B8DEF),
    val screen: Screen? = null
)

// Данные пользователя
data class UserProfile(
    val name: String,
    val username: String,
    val phone: String,
    val bio: String,
    val avatarColor: Color,
    val avatarGradient: List<Color>? = null,
    val isOnline: Boolean = true,
    val lastSeen: String = "в сети",
    val isPremium: Boolean = false
)

// Настройка
data class SettingItem(
    val icon: ImageVector,
    val title: String,
    val subtitle: String? = null,
    val iconColor: Color = Color(0xFF8E99A4),
    val hasSwitch: Boolean = false,
    val isEnabled: Boolean = false
)

// Список эмодзи
val emojiList = listOf(
    "😀", "😃", "😄", "😁", "😆", "😅", "🤣", "😂", "🙂", "🙃",
    "😉", "😊", "😇", "🥰", "😍", "🤩", "😘", "😗", "☺️", "😚",
    "😋", "😛", "😜", "🤪", "😝", "🤑", "🤗", "🤭", "🤫", "🤔",
    "🤐", "🤨", "😐", "😑", "😶", "😏", "😒", "🙄", "😬", "🤥",
    "😌", "😔", "😪", "🤤", "😴", "😷", "🤒", "🤕", "🤢", "🤮",
    "🤧", "🥵", "🥶", "🥴", "😵", "🤯", "🤠", "🥳", "🥸", "😎",
    "🤓", "🧐", "😕", "😟", "🙁", "☹️", "😮", "😯", "😲", "😳",
    "🥺", "😦", "😧", "😨", "😰", "😥", "😢", "😭", "😱", "😖",
    "👍", "👎", "👌", "✌️", "🤞", "🤟", "🤘", "🤙", "👋", "🖐️",
    "✋", "🖖", "👏", "🙌", "👐", "🤲", "🤝", "🙏", "✍️", "💪",
    "❤️", "🧡", "💛", "💚", "💙", "💜", "🖤", "🤍", "🤎", "💔",
    "❣️", "💕", "💞", "💓", "💗", "💖", "💘", "💝", "💟", "🔥"
)

// Быстрые реакции
val quickReactions = listOf("👍", "❤️", "🔥", "😂", "😮", "😢", "🎉")

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TelegramChatApp() {
    val drawerState = rememberDrawerState(initialValue = DrawerValue.Closed)
    val scope = rememberCoroutineScope()
    var currentScreen by remember { mutableStateOf<Screen>(Screen.ChatList) }
    var selectedChat by remember { mutableStateOf<ChatData?>(null) }

    val configuration = LocalConfiguration.current
    val isTablet = configuration.screenWidthDp >= 600

    // Текущий пользователь
    val currentUser = remember {
        UserProfile(
            name = "Иван Петров",
            username = "@ivan_petrov",
            phone = "+7 999 123-45-67",
            bio = "Разработчик Android приложений \n📱 Kotlin • Jetpack Compose",
            avatarColor = Color(0xFF5B8DEF),
            avatarGradient = listOf(Color(0xFF6B9FFF), Color(0xFF3D7BF7)),
            isPremium = true
        )
    }

    // Список чатов с улучшенными градиентами
    val chats = remember {
        listOf(
            ChatData(
                1, "Алексей Петров", "Привет! Как дела? 👋", "12:45", 2, true, true, false, false, false, false, false,
                Color(0xFF5C6BC0), listOf(Color(0xFF7986CB), Color(0xFF3F51B5)),
                hasStory = true, storyUnseen = true
            ),
            ChatData(
                2, "Команда проекта", "Андрей: Встреча в 15:00 📅", "11:30", 5, false, true, false, false, false, true, false,
                Color(0xFF26A69A), listOf(Color(0xFF4DB6AC), Color(0xFF00897B)), "Андрей"
            ),
            ChatData(
                3, "Мария Иванова", "Спасибо за помощь! 🙏", "10:15", 0, true, false, false, false, false, false, false,
                Color(0xFFEF5350), listOf(Color(0xFFEF5350), Color(0xFFE53935)),
                isTyping = true, hasStory = true, storyUnseen = false
            ),
            ChatData(
                4, "Telegram", "Добро пожаловать в Telegram! ✅", "Вчера", 0, false, false, false, true, true, false, false,
                Color(0xFF5B8DEF), listOf(Color(0xFF64B5F6), Color(0xFF2196F3))
            ),
            ChatData(
                5, "Дмитрий Сидоров", "Документы отправил 📄", "Вчера", 1, false, false, false, false, false, false, false,
                Color(0xFF42A5F5), listOf(Color(0xFF64B5F6), Color(0xFF1E88E5))
            ),
            ChatData(
                6, "Анна Козлова", "Отличная идея! 💡", "Пн", 0, true, false, false, false, false, false, false,
                Color(0xFFAB47BC), listOf(Color(0xFFCE93D8), Color(0xFF8E24AA))
            ),
            ChatData(
                7, "Бот помощник", "Чем могу помочь? 🤖", "Пн", 0, false, false, false, true, false, false, true,
                Color(0xFFFF9800), listOf(Color(0xFFFFB74D), Color(0xFFF57C00))
            ),
            ChatData(
                8, "Семья", "Мама: Фото с праздника 📷🎉", "Вс", 0, false, false, false, false, false, true, false,
                Color(0xFFFFCA28), listOf(Color(0xFFFFD54F), Color(0xFFFFA000)), "Мама"
            ),
            ChatData(
                9, "Новости IT", "Срочные новости технологий! 📰", "Вс", 12, false, false, true, true, true, false, false,
                Color(0xFF78909C), listOf(Color(0xFF90A4AE), Color(0xFF546E7A))
            ),
            ChatData(
                10, "Спортзал", "Тренировка в 18:00 💪", "Сб", 0, false, false, false, false, false, false, false,
                Color(0xFFFF5722), listOf(Color(0xFFFF7043), Color(0xFFE64A19)), draft = "Буду в 18:"
            )
        )
    }

    // Пункты меню
    val menuItems = listOf(
        DrawerMenuItem(Icons.Rounded.Person, "Мой профиль", screen = Screen.Profile),
        DrawerMenuItem(Icons.Rounded.GroupAdd, "Создать группу", screen = Screen.CreateGroup),
        DrawerMenuItem(Icons.Rounded.Campaign, "Создать канал", screen = Screen.NewChannel),
        DrawerMenuItem(Icons.Rounded.Contacts, "Контакты", screen = Screen.Contacts),
        DrawerMenuItem(Icons.Rounded.Call, "Звонки", badge = "3", screen = Screen.Calls),
        DrawerMenuItem(Icons.Rounded.Bookmark, "Избранное", screen = Screen.SavedMessages),
        DrawerMenuItem(Icons.Rounded.Settings, "Настройки", screen = Screen.Settings)
    )

    val bottomMenuItems = listOf(
        DrawerMenuItem(Icons.Rounded.Inventory2, "Архив", badge = "24", badgeColor = Color(0xFF6C7883), screen = Screen.ArchivedChats),
        DrawerMenuItem(Icons.Rounded.PersonAdd, "Пригласить друзей")
    )

    // Навигация назад
    BackHandler(enabled = currentScreen != Screen.ChatList) {
        when (currentScreen) {
            is Screen.Chat -> {
                if (!isTablet) {
                    selectedChat = null
                    currentScreen = Screen.ChatList
                }
            }
            else -> currentScreen = Screen.ChatList
        }
    }

    ModalNavigationDrawer(
        drawerState = drawerState,
        gesturesEnabled = currentScreen == Screen.ChatList || (isTablet && currentScreen is Screen.Chat),
        drawerContent = {
            ModalDrawerSheet(
                modifier = Modifier.width(300.dp),
                drawerContainerColor = TelegramColors.DrawerBackground,
                drawerContentColor = Color.White
            ) {
                DrawerContent(
                    user = currentUser,
                    menuItems = menuItems,
                    bottomMenuItems = bottomMenuItems,
                    onItemClick = { screen ->
                        scope.launch { drawerState.close() }
                        if (screen != null) {
                            currentScreen = screen
                        }
                    }
                )
            }
        }
    ) {
        when (val screen = currentScreen) {
            is Screen.ChatList, is Screen.Chat -> {
                if (isTablet) {
                    Row(modifier = Modifier.fillMaxSize()) {
                        ChatListPanel(
                            chats = chats,
                            selectedChat = selectedChat,
                            onChatSelected = { chat ->
                                selectedChat = chat
                                currentScreen = Screen.Chat(chat)
                            },
                            onMenuClick = { scope.launch { drawerState.open() } },
                            modifier = Modifier.width(360.dp)
                        )
                        if (selectedChat != null) {
                            ChatScreen(
                                chat = selectedChat!!,
                                onBackClick = { selectedChat = null },
                                showBackButton = false,
                                modifier = Modifier.weight(1f)
                            )
                        } else {
                            EmptyChatScreen(modifier = Modifier.weight(1f))
                        }
                    }
                } else {
                    AnimatedContent(
                        targetState = selectedChat,
                        transitionSpec = {
                            if (targetState != null) {
                                (slideInHorizontally { it } + fadeIn(tween(300))) togetherWith
                                        (slideOutHorizontally { -it / 3 } + fadeOut(tween(200)))
                            } else {
                                (slideInHorizontally { -it / 3 } + fadeIn(tween(300))) togetherWith
                                        (slideOutHorizontally { it } + fadeOut(tween(200)))
                            }
                        },
                        label = "chat_transition"
                    ) { chat ->
                        if (chat == null) {
                            ChatListPanel(
                                chats = chats,
                                selectedChat = null,
                                onChatSelected = {
                                    selectedChat = it
                                    currentScreen = Screen.Chat(it)
                                },
                                onMenuClick = { scope.launch { drawerState.open() } },
                                modifier = Modifier.fillMaxWidth()
                            )
                        } else {
                            ChatScreen(
                                chat = chat,
                                onBackClick = {
                                    selectedChat = null
                                    currentScreen = Screen.ChatList
                                },
                                showBackButton = true,
                                modifier = Modifier.fillMaxWidth()
                            )
                        }
                    }
                }
            }
            is Screen.Profile -> {
                ProfileScreen(
                    user = currentUser,
                    onBackClick = { currentScreen = Screen.ChatList }
                )
            }
            is Screen.Settings -> {
                SettingsScreen(
                    user = currentUser,
                    onBackClick = { currentScreen = Screen.ChatList }
                )
            }
            is Screen.Contacts -> {
                ContactsScreen(onBackClick = { currentScreen = Screen.ChatList })
            }
            is Screen.Calls -> {
                CallsScreen(onBackClick = { currentScreen = Screen.ChatList })
            }
            is Screen.SavedMessages -> {
                SavedMessagesScreen(onBackClick = { currentScreen = Screen.ChatList })
            }
            is Screen.ArchivedChats -> {
                ArchivedChatsScreen(
                    chats = chats.filter { it.isMuted },
                    onBackClick = { currentScreen = Screen.ChatList }
                )
            }
            is Screen.CreateGroup -> {
                CreateGroupScreen(onBackClick = { currentScreen = Screen.ChatList })
            }
            is Screen.NewChannel -> {
                CreateChannelScreen(onBackClick = { currentScreen = Screen.ChatList })
            }
        }
    }
}

@Composable
fun DrawerContent(
    user: UserProfile,
    menuItems: List<DrawerMenuItem>,
    bottomMenuItems: List<DrawerMenuItem>,
    onItemClick: (Screen?) -> Unit
) {
    var isAccountExpanded by remember { mutableStateOf(false) }
    val rotationAngle by animateFloatAsState(
        targetValue = if (isAccountExpanded) 180f else 0f,
        animationSpec = spring(dampingRatio = Spring.DampingRatioMediumBouncy),
        label = "arrow_rotation"
    )

    Column(modifier = Modifier.fillMaxSize()) {
        // Шапка с профилем - улучшенный градиент
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .height(200.dp)
                .background(
                    Brush.linearGradient(
                        colors = listOf(
                            Color(0xFF667EEA),
                            Color(0xFF764BA2)
                        ),
                        start = Offset(0f, 0f),
                        end = Offset(Float.POSITIVE_INFINITY, Float.POSITIVE_INFINITY)
                    )
                )
                .clickable { isAccountExpanded = !isAccountExpanded }
                .padding(20.dp)
        ) {
            // Декоративные круги на фоне
            Canvas(modifier = Modifier.fillMaxSize()) {
                drawCircle(
                    color = Color.White.copy(alpha = 0.1f),
                    radius = 100.dp.toPx(),
                    center = Offset(size.width + 20.dp.toPx(), -30.dp.toPx())
                )
                drawCircle(
                    color = Color.White.copy(alpha = 0.05f),
                    radius = 150.dp.toPx(),
                    center = Offset(-50.dp.toPx(), size.height + 50.dp.toPx())
                )
            }

            Column(
                modifier = Modifier.align(Alignment.BottomStart)
            ) {
                // Аватар с градиентом
                Box {
                    Box(
                        modifier = Modifier
                            .size(72.dp)
                            .shadow(8.dp, CircleShape)
                            .clip(CircleShape)
                            .background(
                                Brush.linearGradient(
                                    user.avatarGradient ?: listOf(user.avatarColor, user.avatarColor)
                                )
                            ),
                        contentAlignment = Alignment.Center
                    ) {
                        Text(
                            text = user.name.split(" ").take(2).map { it.firstOrNull() ?: "" }.joinToString("").uppercase(),
                            color = Color.White,
                            fontSize = 28.sp,
                            fontWeight = FontWeight.Bold
                        )
                    }
                    // Premium badge с анимацией
                    if (user.isPremium) {
                        val infiniteTransition = rememberInfiniteTransition(label = "premium")
                        val shimmer by infiniteTransition.animateFloat(
                            initialValue = -0.2f,
                            targetValue = 1.2f,
                            animationSpec = infiniteRepeatable(
                                animation = tween(2000, easing = LinearEasing),
                                repeatMode = RepeatMode.Restart
                            ),
                            label = "shimmer"
                        )
                        
                        Box(
                            modifier = Modifier
                                .align(Alignment.BottomEnd)
                                .size(26.dp)
                                .shadow(4.dp, CircleShape)
                                .clip(CircleShape)
                                .background(
                                    Brush.linearGradient(
                                        colors = listOf(Color(0xFFFFD700), Color(0xFFFFA000), Color(0xFFFFD700))
                                    )
                                ),
                            contentAlignment = Alignment.Center
                        ) {
                            Icon(
                                Icons.Filled.Star,
                                contentDescription = null,
                                tint = Color.White,
                                modifier = Modifier.size(16.dp)
                            )
                        }
                    }
                }

                Spacer(modifier = Modifier.height(16.dp))

                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text(
                        text = user.name,
                        color = Color.White,
                        fontSize = 18.sp,
                        fontWeight = FontWeight.SemiBold
                    )
                    if (user.isPremium) {
                        Spacer(modifier = Modifier.width(6.dp))
                        Icon(
                            Icons.Filled.Verified,
                            contentDescription = "Premium",
                            tint = Color(0xFFFFD700),
                            modifier = Modifier.size(18.dp)
                        )
                    }
                }

                Spacer(modifier = Modifier.height(2.dp))

                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text(
                        text = user.phone,
                        color = Color.White.copy(alpha = 0.9f),
                        fontSize = 14.sp,
                        fontWeight = FontWeight.Normal
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    // Индикатор онлайн с пульсацией
                    val pulseAnim = rememberInfiniteTransition(label = "pulse")
                    val scale by pulseAnim.animateFloat(
                        initialValue = 1f,
                        targetValue = 1.3f,
                        animationSpec = infiniteRepeatable(
                            animation = tween(1000),
                            repeatMode = RepeatMode.Reverse
                        ),
                        label = "pulse_scale"
                    )
                    Box(
                        modifier = Modifier
                            .size(8.dp)
                            .scale(scale)
                            .clip(CircleShape)
                            .background(Color(0xFF4CAF50))
                    )
                }
            }

            // Стрелка для переключения аккаунтов
            Icon(
                Icons.Rounded.KeyboardArrowDown,
                contentDescription = null,
                tint = Color.White.copy(alpha = 0.9f),
                modifier = Modifier
                    .align(Alignment.BottomEnd)
                    .padding(bottom = 4.dp)
                    .rotate(rotationAngle)
            )

            // Ночной режим
            IconButton(
                onClick = { },
                modifier = Modifier.align(Alignment.TopEnd)
            ) {
                Icon(
                    Icons.Rounded.DarkMode,
                    contentDescription = "Night Mode",
                    tint = Color.White.copy(alpha = 0.9f)
                )
            }
        }

        // Выбор аккаунта (если раскрыт)
        AnimatedVisibility(
            visible = isAccountExpanded,
            enter = expandVertically(animationSpec = spring(dampingRatio = Spring.DampingRatioMediumBouncy)) + fadeIn(),
            exit = shrinkVertically(animationSpec = spring(dampingRatio = Spring.DampingRatioMediumBouncy)) + fadeOut()
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .background(TelegramColors.DrawerBackground)
            ) {
                AccountItem(
                    name = user.name,
                    phone = user.phone,
                    isSelected = true,
                    isPremium = user.isPremium
                )
                HorizontalDivider(color = TelegramColors.Divider.copy(alpha = 0.5f), thickness = 0.5.dp)
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clickable { }
                        .padding(16.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Icon(
                        Icons.Rounded.Add,
                        contentDescription = null,
                        tint = TelegramColors.Accent,
                        modifier = Modifier.size(24.dp)
                    )
                    Spacer(modifier = Modifier.width(32.dp))
                    Text(
                        "Добавить аккаунт",
                        color = TelegramColors.Accent,
                        fontSize = 15.sp,
                        fontWeight = FontWeight.Medium
                    )
                }
                HorizontalDivider(color = TelegramColors.Divider.copy(alpha = 0.5f), thickness = 0.5.dp)
            }
        }

        Spacer(modifier = Modifier.height(8.dp))

        // Основные пункты меню
        menuItems.forEach { item ->
            DrawerMenuItemRow(item = item, onClick = { onItemClick(item.screen) })
        }

        Spacer(modifier = Modifier.weight(1f))

        HorizontalDivider(color = TelegramColors.Divider.copy(alpha = 0.5f), thickness = 0.5.dp)

        // Нижние пункты меню
        bottomMenuItems.forEach { item ->
            DrawerMenuItemRow(item = item, onClick = { onItemClick(item.screen) })
        }

        // Версия приложения
        Text(
            text = "Telegram Clone v2.0",
            color = Color(0xFF6C7883),
            fontSize = 12.sp,
            modifier = Modifier.padding(16.dp)
        )
    }
}

@Composable
fun AccountItem(
    name: String,
    phone: String,
    isSelected: Boolean,
    isPremium: Boolean
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { }
            .padding(16.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Box(
            modifier = Modifier
                .size(44.dp)
                .clip(CircleShape)
                .background(
                    Brush.linearGradient(
                        colors = listOf(Color(0xFF667EEA), Color(0xFF764BA2))
                    )
                ),
            contentAlignment = Alignment.Center
        ) {
            Text(
                text = name.split(" ").take(2).map { it.firstOrNull() ?: "" }.joinToString("").uppercase(),
                color = Color.White,
                fontSize = 16.sp,
                fontWeight = FontWeight.Bold
            )
        }

        Spacer(modifier = Modifier.width(16.dp))

        Column(modifier = Modifier.weight(1f)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Text(
                    text = name,
                    color = Color.White,
                    fontSize = 16.sp,
                    fontWeight = FontWeight.Medium
                )
                if (isPremium) {
                    Spacer(modifier = Modifier.width(4.dp))
                    Icon(
                        Icons.Filled.Verified,
                        contentDescription = null,
                        tint = Color(0xFFFFD700),
                        modifier = Modifier.size(16.dp)
                    )
                }
            }
            Text(
                text = phone,
                color = Color(0xFF6C7883),
                fontSize = 14.sp
            )
        }

        if (isSelected) {
            Icon(
                Icons.Rounded.Check,
                contentDescription = null,
                tint = TelegramColors.Accent,
                modifier = Modifier.size(24.dp)
            )
        }
    }
}

@Composable
fun DrawerMenuItemRow(item: DrawerMenuItem, onClick: () -> Unit) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
            .padding(horizontal = 24.dp, vertical = 14.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Icon(
            item.icon,
            contentDescription = null,
            tint = Color(0xFF8A9AA5),
            modifier = Modifier.size(24.dp)
        )

        Spacer(modifier = Modifier.width(24.dp))

        Text(
            text = item.title,
            color = Color.White,
            fontSize = 15.sp,
            fontWeight = FontWeight.Normal,
            modifier = Modifier.weight(1f)
        )

        if (item.badge != null) {
            Box(
                modifier = Modifier
                    .background(item.badgeColor, RoundedCornerShape(10.dp))
                    .padding(horizontal = 8.dp, vertical = 3.dp),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = item.badge,
                    color = Color.White,
                    fontSize = 12.sp,
                    fontWeight = FontWeight.SemiBold
                )
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ChatListPanel(
    chats: List<ChatData>,
    selectedChat: ChatData?,
    onChatSelected: (ChatData) -> Unit,
    onMenuClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    var searchQuery by remember { mutableStateOf("") }
    var isSearchActive by remember { mutableStateOf(false) }

    Column(
        modifier = modifier
            .fillMaxHeight()
            .background(TelegramColors.Background)
    ) {
        // Верхняя панель - улучшенная
        TopAppBar(
            title = {
                if (isSearchActive) {
                    TextField(
                        value = searchQuery,
                        onValueChange = { searchQuery = it },
                        placeholder = { 
                            Text(
                                "Поиск", 
                                color = Color(0xFF6C7883),
                                fontSize = 17.sp
                            ) 
                        },
                        colors = TextFieldDefaults.colors(
                            focusedContainerColor = Color.Transparent,
                            unfocusedContainerColor = Color.Transparent,
                            focusedTextColor = Color.White,
                            cursorColor = TelegramColors.Accent,
                            focusedIndicatorColor = Color.Transparent,
                            unfocusedIndicatorColor = Color.Transparent
                        ),
                        textStyle = TextStyle(fontSize = 17.sp),
                        singleLine = true,
                        modifier = Modifier.fillMaxWidth()
                    )
                } else {
                    Text(
                        "Telegram",
                        color = Color.White,
                        fontWeight = FontWeight.Bold,
                        fontSize = 21.sp,
                        letterSpacing = 0.sp
                    )
                }
            },
            navigationIcon = {
                IconButton(onClick = {
                    if (isSearchActive) {
                        isSearchActive = false
                        searchQuery = ""
                    } else {
                        onMenuClick()
                    }
                }) {
                    Icon(
                        if (isSearchActive) Icons.Rounded.ArrowBack else Icons.Rounded.Menu,
                        contentDescription = "Menu",
                        tint = TelegramColors.IconTint,
                        modifier = Modifier.size(24.dp)
                    )
                }
            },
            actions = {
                IconButton(onClick = { isSearchActive = !isSearchActive }) {
                    Icon(
                        Icons.Rounded.Search,
                        contentDescription = "Search",
                        tint = TelegramColors.IconTint,
                        modifier = Modifier.size(24.dp)
                    )
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(
                containerColor = TelegramColors.Background
            )
        )

        // Фильтр по папкам - улучшенный
        ScrollableTabRow(
            selectedTabIndex = 0,
            containerColor = TelegramColors.Background,
            contentColor = Color.White,
            edgePadding = 16.dp,
            divider = {},
            indicator = { tabPositions ->
                if (tabPositions.isNotEmpty()) {
                    Box(
                        Modifier
                            .tabIndicatorOffset(tabPositions[0])
                            .height(3.dp)
                            .padding(horizontal = 16.dp)
                            .background(TelegramColors.Accent, RoundedCornerShape(topStart = 3.dp, topEnd = 3.dp))
                    )
                }
            }
        ) {
            listOf("Все", "Личные", "Каналы", "Группы", "Боты").forEachIndexed { index, title ->
                Tab(
                    selected = index == 0,
                    onClick = { },
                    text = {
                        Text(
                            title,
                            color = if (index == 0) TelegramColors.Accent else Color(0xFF6C7883),
                            fontWeight = if (index == 0) FontWeight.SemiBold else FontWeight.Normal,
                            fontSize = 15.sp
                        )
                    }
                )
            }
        }

        HorizontalDivider(color = Color(0xFF0E1621), thickness = 1.dp)

        // Список чатов
        LazyColumn(modifier = Modifier.fillMaxSize()) {
            val pinnedChats = chats.filter { it.isPinned }
            val regularChats = chats.filter { !it.isPinned }

            items(pinnedChats, key = { it.id }) { chat ->
                ChatListItem(
                    chat = chat,
                    isSelected = chat == selectedChat,
                    onClick = { onChatSelected(chat) }
                )
            }

            if (pinnedChats.isNotEmpty() && regularChats.isNotEmpty()) {
                item {
                    Spacer(modifier = Modifier.height(8.dp))
                }
            }

            items(regularChats, key = { it.id }) { chat ->
                ChatListItem(
                    chat = chat,
                    isSelected = chat == selectedChat,
                    onClick = { onChatSelected(chat) }
                )
            }
        }
    }
}

@Composable
fun ChatListItem(
    chat: ChatData,
    isSelected: Boolean,
    onClick: () -> Unit
) {
    val interactionScale by animateFloatAsState(
        targetValue = if (isSelected) 0.98f else 1f,
        animationSpec = spring(stiffness = Spring.StiffnessHigh),
        label = "scale"
    )
    
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .scale(interactionScale)
            .background(
                if (isSelected) TelegramColors.SelectedChat.copy(alpha = 0.5f) else Color.Transparent
            )
            .clickable(onClick = onClick)
            .padding(horizontal = 16.dp, vertical = 10.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        // Аватар с возможной историей
        Box(contentAlignment = Alignment.BottomEnd) {
            // Story ring
            Box(
                modifier = Modifier
                    .size(if (chat.hasStory) 62.dp else 58.dp)
                    .then(
                        if (chat.hasStory) {
                            Modifier
                                .clip(CircleShape)
                                .background(
                                    if (chat.storyUnseen) {
                                        Brush.sweepGradient(
                                            colors = listOf(
                                                Color(0xFF7C4DFF),
                                                Color(0xFF448AFF),
                                                Color(0xFF00BFA5),
                                                Color(0xFFFFEA00),
                                                Color(0xFFFF5722),
                                                Color(0xFFE91E63),
                                                Color(0xFF7C4DFF)
                                            )
                                        )
                                    } else {
                                        Brush.sweepGradient(
                                            colors = listOf(Color(0xFF4A5568), Color(0xFF4A5568))
                                        )
                                    }
                                )
                                .padding(2.5.dp)
                        } else Modifier
                    ),
                contentAlignment = Alignment.Center
            ) {
                Box(
                    modifier = Modifier
                        .size(54.dp)
                        .clip(CircleShape)
                        .background(
                            if (chat.hasStory) TelegramColors.Background else Color.Transparent
                        )
                        .padding(if (chat.hasStory) 2.dp else 0.dp)
                ) {
                    Box(
                        modifier = Modifier
                            .fillMaxSize()
                            .clip(CircleShape)
                            .background(
                                Brush.linearGradient(
                                    chat.avatarGradient ?: listOf(chat.avatarColor, chat.avatarColor)
                                )
                            ),
                        contentAlignment = Alignment.Center
                    ) {
                        when {
                            chat.isChannel -> {
                                Icon(
                                    Icons.Rounded.Campaign,
                                    contentDescription = null,
                                    tint = Color.White,
                                    modifier = Modifier.size(26.dp)
                                )
                            }
                            chat.isGroup -> {
                                Icon(
                                    Icons.Rounded.Group,
                                    contentDescription = null,
                                    tint = Color.White,
                                    modifier = Modifier.size(26.dp)
                                )
                            }
                            chat.isBot -> {
                                Icon(
                                    Icons.Rounded.SmartToy,
                                    contentDescription = null,
                                    tint = Color.White,
                                    modifier = Modifier.size(26.dp)
                                )
                            }
                            else -> {
                                Text(
                                    text = chat.name.split(" ").take(2).map { it.firstOrNull() ?: "" }.joinToString("").uppercase(),
                                    color = Color.White,
                                    fontSize = 19.sp,
                                    fontWeight = FontWeight.SemiBold
                                )
                            }
                        }
                    }
                }
            }
            
            // Online indicator
            if (chat.isOnline && !chat.isChannel && !chat.isGroup) {
                Box(
                    modifier = Modifier
                        .size(16.dp)
                        .offset(x = 1.dp, y = 1.dp)
                        .clip(CircleShape)
                        .background(TelegramColors.Background)
                        .padding(2.5.dp)
                ) {
                    Box(
                        modifier = Modifier
                            .fillMaxSize()
                            .clip(CircleShape)
                            .background(TelegramColors.Online)
                    )
                }
            }
        }

        Spacer(modifier = Modifier.width(14.dp))

        Column(modifier = Modifier.weight(1f)) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    modifier = Modifier.weight(1f)
                ) {
                    Text(
                        text = chat.name,
                        color = Color.White,
                        fontSize = 16.sp,
                        fontWeight = FontWeight.SemiBold,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis
                    )
                    if (chat.isVerified) {
                        Spacer(modifier = Modifier.width(4.dp))
                        Icon(
                            Icons.Filled.Verified,
                            contentDescription = "Verified",
                            tint = TelegramColors.Accent,
                            modifier = Modifier.size(18.dp)
                        )
                    }
                    if (chat.isMuted) {
                        Spacer(modifier = Modifier.width(4.dp))
                        Icon(
                            Icons.Rounded.VolumeOff,
                            contentDescription = null,
                            tint = Color(0xFF6C7883),
                            modifier = Modifier.size(16.dp)
                        )
                    }
                }
                Row(verticalAlignment = Alignment.CenterVertically) {
                    if (chat.isPinned) {
                        Icon(
                            Icons.Rounded.PushPin,
                            contentDescription = null,
                            tint = Color(0xFF6C7883),
                            modifier = Modifier
                                .size(16.dp)
                                .rotate(45f)
                        )
                        Spacer(modifier = Modifier.width(4.dp))
                    }
                    Text(
                        text = chat.time,
                        color = if (chat.unreadCount > 0) TelegramColors.Accent else Color(0xFF6C7883),
                        fontSize = 13.sp,
                        fontWeight = if (chat.unreadCount > 0) FontWeight.Medium else FontWeight.Normal
                    )
                }
            }

            Spacer(modifier = Modifier.height(4.dp))

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                if (chat.draft != null) {
                    Row(modifier = Modifier.weight(1f)) {
                        Text(
                            text = "Черновик: ",
                            color = Color(0xFFE53935),
                            fontSize = 15.sp
                        )
                        Text(
                            text = chat.draft,
                            color = Color(0xFF6C7883),
                            fontSize = 15.sp,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                    }
                } else if (chat.isTyping) {
                    TypingIndicator()
                } else {
                    Row(modifier = Modifier.weight(1f)) {
                        if (chat.lastMessageSender != null) {
                            Text(
                                text = "${chat.lastMessageSender}: ",
                                color = Color.White.copy(alpha = 0.9f),
                                fontSize = 15.sp,
                                fontWeight = FontWeight.Medium
                            )
                        }
                        Text(
                            text = chat.lastMessage,
                            color = Color(0xFF6C7883),
                            fontSize = 15.sp,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                    }
                }

                if (chat.unreadCount > 0) {
                    Spacer(modifier = Modifier.width(8.dp))
                    Box(
                        modifier = Modifier
                            .defaultMinSize(minWidth = 22.dp)
                            .height(22.dp)
                            .clip(RoundedCornerShape(11.dp))
                            .background(if (chat.isMuted) Color(0xFF6C7883) else TelegramColors.Accent)
                            .padding(horizontal = 6.dp),
                        contentAlignment = Alignment.Center
                    ) {
                        Text(
                            text = if (chat.unreadCount > 99) "99+" else chat.unreadCount.toString(),
                            color = Color.White,
                            fontSize = 12.sp,
                            fontWeight = FontWeight.Bold
                        )
                    }
                }
            }
        }
    }
}

@Composable
fun TypingIndicator() {
    val infiniteTransition = rememberInfiniteTransition(label = "typing")

    Row(verticalAlignment = Alignment.CenterVertically) {
        Text(
            text = "печатает",
            color = TelegramColors.Accent,
            fontSize = 15.sp,
            fontWeight = FontWeight.Medium
        )
        Spacer(modifier = Modifier.width(4.dp))
        repeat(3) { index ->
            val delay = index * 150
            val offsetY by infiniteTransition.animateFloat(
                initialValue = 0f,
                targetValue = -4f,
                animationSpec = infiniteRepeatable(
                    animation = tween(400, delayMillis = delay),
                    repeatMode = RepeatMode.Reverse
                ),
                label = "dot_$index"
            )
            Box(
                modifier = Modifier
                    .padding(horizontal = 1.5.dp)
                    .offset(y = offsetY.dp)
                    .size(5.dp)
                    .clip(CircleShape)
                    .background(TelegramColors.Accent)
            )
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class, ExperimentalFoundationApi::class)
@Composable
fun ChatScreen(
    chat: ChatData,
    onBackClick: () -> Unit,
    showBackButton: Boolean,
    modifier: Modifier = Modifier
) {
    val messages = remember {
        listOf(
            Message(1, "Привет! Как дела? 👋", "12:30", false, true),
            Message(2, "Привет! Все отлично, спасибо! 😊", "12:32", true, true),
            Message(3, "Что нового?", "12:33", false, true),
            Message(4, "Работаю над новым проектом. Очень интересно! 💻", "12:35", true, true,
                reactions = listOf(Reaction("👍", 1, false), Reaction("🔥", 2, true))),
            Message(5, "Звучит здорово! Расскажи подробнее 🤔", "12:40", false, true,
                replyTo = Message(4, "Работаю над новым проектом...", "12:35", true, true)),
            Message(6, "Это приложение-мессенджер в стиле Telegram. Использую Jetpack Compose и Material 3 😎✨", "12:42", true, true, isEdited = true),
            Message(7, "Вау, круто! Покажешь когда закончишь? 🚀", "12:45", false, true),
            Message(8, "Конечно! Скину ссылку на GitHub 📦", "12:46", true, false,
                reactions = listOf(Reaction("❤️", 1, true)))
        )
    }

    var messageText by remember { mutableStateOf("") }
    var showEmojiPicker by remember { mutableStateOf(false) }
    var showAttachMenu by remember { mutableStateOf(false) }
    var replyingTo by remember { mutableStateOf<Message?>(null) }
    var selectedMessage by remember { mutableStateOf<Message?>(null) }
    val listState = rememberLazyListState()
    val scope = rememberCoroutineScope()

    // Фон чата с паттерном
    Box(
        modifier = modifier
            .fillMaxSize()
            .background(TelegramColors.ChatBackground)
    ) {
        // Декоративный паттерн на фоне
        Canvas(modifier = Modifier.fillMaxSize()) {
            val patternColor = Color(0xFF0A1220).copy(alpha = 0.5f)
            for (i in 0..50) {
                for (j in 0..100) {
                    if ((i + j) % 3 == 0) {
                        drawCircle(
                            color = patternColor,
                            radius = 2.dp.toPx(),
                            center = Offset(
                                (i * 30).dp.toPx() + (j % 2) * 15.dp.toPx(),
                                (j * 30).dp.toPx()
                            )
                        )
                    }
                }
            }
        }

        Column(modifier = Modifier.fillMaxSize()) {
            // Шапка чата - улучшенная
            Surface(
                modifier = Modifier.fillMaxWidth(),
                color = TelegramColors.Background,
                shadowElevation = 4.dp
            ) {
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 4.dp, vertical = 8.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    if (showBackButton) {
                        IconButton(onClick = onBackClick) {
                            Icon(
                                Icons.Rounded.ArrowBack,
                                contentDescription = "Back",
                                tint = TelegramColors.IconTint
                            )
                        }
                    }

                    Row(
                        verticalAlignment = Alignment.CenterVertically,
                        modifier = Modifier
                            .weight(1f)
                            .clickable { }
                            .padding(horizontal = 8.dp, vertical = 4.dp)
                    ) {
                        // Аватар
                        Box(
                            modifier = Modifier
                                .size(44.dp)
                                .clip(CircleShape)
                                .background(
                                    Brush.linearGradient(
                                        chat.avatarGradient ?: listOf(chat.avatarColor, chat.avatarColor)
                                    )
                                ),
                            contentAlignment = Alignment.Center
                        ) {
                            when {
                                chat.isChannel -> Icon(Icons.Rounded.Campaign, contentDescription = null, tint = Color.White, modifier = Modifier.size(22.dp))
                                chat.isGroup -> Icon(Icons.Rounded.Group, contentDescription = null, tint = Color.White, modifier = Modifier.size(22.dp))
                                chat.isBot -> Icon(Icons.Rounded.SmartToy, contentDescription = null, tint = Color.White, modifier = Modifier.size(22.dp))
                                else -> Text(
                                    text = chat.name.split(" ").take(2).map { it.firstOrNull() ?: "" }.joinToString("").uppercase(),
                                    color = Color.White,
                                    fontSize = 16.sp,
                                    fontWeight = FontWeight.SemiBold
                                )
                            }
                        }

                        Spacer(modifier = Modifier.width(12.dp))

                        Column {
                            Row(verticalAlignment = Alignment.CenterVertically) {
                                Text(
                                    text = chat.name,
                                    color = Color.White,
                                    fontSize = 17.sp,
                                    fontWeight = FontWeight.SemiBold
                                )
                                if (chat.isVerified) {
                                    Spacer(modifier = Modifier.width(4.dp))
                                    Icon(
                                        Icons.Filled.Verified,
                                        contentDescription = null,
                                        tint = TelegramColors.Accent,
                                        modifier = Modifier.size(16.dp)
                                    )
                                }
                            }

                            // Статус с анимацией
                            val statusText = when {
                                chat.isTyping -> "печатает..."
                                chat.isChannel -> "канал"
                                chat.isGroup -> "участников: 128"
                                chat.isBot -> "бот"
                                chat.isOnline -> "в сети"
                                else -> "был(а) недавно"
                            }
                            val statusColor = when {
                                chat.isTyping -> TelegramColors.Accent
                                chat.isOnline -> TelegramColors.Online
                                else -> Color(0xFF6C7883)
                            }

                            Text(
                                text = statusText,
                                color = statusColor,
                                fontSize = 13.sp,
                                fontWeight = if (chat.isTyping || chat.isOnline) FontWeight.Medium else FontWeight.Normal
                            )
                        }
                    }

                    // Кнопки действий
                    IconButton(onClick = { }) {
                        Icon(
                            Icons.Rounded.Videocam,
                            contentDescription = "Video Call",
                            tint = TelegramColors.IconTint
                        )
                    }
                    IconButton(onClick = { }) {
                        Icon(
                            Icons.Rounded.Call,
                            contentDescription = "Call",
                            tint = TelegramColors.IconTint
                        )
                    }
                    IconButton(onClick = { }) {
                        Icon(
                            Icons.Rounded.MoreVert,
                            contentDescription = "More",
                            tint = TelegramColors.IconTint
                        )
                    }
                }
            }

            // Сообщения
            Box(modifier = Modifier.weight(1f)) {
                LazyColumn(
                    state = listState,
                    modifier = Modifier
                        .fillMaxSize()
                        .padding(horizontal = 8.dp),
                    reverseLayout = true,
                    contentPadding = PaddingValues(vertical = 12.dp)
                ) {
                    items(messages.reversed(), key = { it.id }) { message ->
                        MessageBubble(
                            message = message,
                            onLongPress = { selectedMessage = message },
                            onReplyClick = { replyingTo = message },
                            onReactionClick = { }
                        )
                    }

                    // Разделитель с датой
                    item {
                        DateDivider(date = "Сегодня")
                    }
                }

                // Кнопка прокрутки вниз
                AnimatedVisibility(
                    visible = listState.firstVisibleItemIndex > 2,
                    enter = scaleIn() + fadeIn(),
                    exit = scaleOut() + fadeOut(),
                    modifier = Modifier
                        .align(Alignment.BottomEnd)
                        .padding(16.dp)
                ) {
                    FloatingActionButton(
                        onClick = {
                            scope.launch {
                                listState.animateScrollToItem(0)
                            }
                        },
                        modifier = Modifier.size(42.dp),
                        containerColor = TelegramColors.Background,
                        contentColor = TelegramColors.IconTint,
                        elevation = FloatingActionButtonDefaults.elevation(
                            defaultElevation = 6.dp
                        )
                    ) {
                        Icon(
                            Icons.Rounded.KeyboardArrowDown,
                            contentDescription = "Scroll down",
                            modifier = Modifier.size(28.dp)
                        )
                    }
                }
            }

            // Reply preview
            AnimatedVisibility(
                visible = replyingTo != null,
                enter = expandVertically() + fadeIn(),
                exit = shrinkVertically() + fadeOut()
            ) {
                ReplyPreview(
                    message = replyingTo,
                    onCancel = { replyingTo = null }
                )
            }

            // Emoji picker
            AnimatedVisibility(
                visible = showEmojiPicker,
                enter = expandVertically() + fadeIn(),
                exit = shrinkVertically() + fadeOut()
            ) {
                EmojiPicker(
                    onEmojiSelected = { emoji -> messageText += emoji },
                    onDismiss = { showEmojiPicker = false }
                )
            }

            // Attach menu
            AnimatedVisibility(
                visible = showAttachMenu,
                enter = expandVertically() + fadeIn(),
                exit = shrinkVertically() + fadeOut()
            ) {
                AttachMenu(onDismiss = { showAttachMenu = false })
            }

            // Поле ввода - улучшенное
            Surface(
                color = TelegramColors.Background,
                shadowElevation = 8.dp
            ) {
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 8.dp, vertical = 8.dp),
                    verticalAlignment = Alignment.Bottom
                ) {
                    // Кнопка прикрепления
                    IconButton(
                        onClick = { showAttachMenu = !showAttachMenu },
                        modifier = Modifier.size(44.dp)
                    ) {
                        Icon(
                            Icons.Rounded.AttachFile,
                            contentDescription = "Attach",
                            tint = if (showAttachMenu) TelegramColors.Accent else TelegramColors.IconTint,
                            modifier = Modifier
                                .size(24.dp)
                                .rotate(45f)
                        )
                    }

                    // Поле ввода
                    Box(
                        modifier = Modifier
                            .weight(1f)
                            .clip(RoundedCornerShape(24.dp))
                            .background(TelegramColors.InputBackground)
                    ) {
                        TextField(
                            value = messageText,
                            onValueChange = { messageText = it },
                            placeholder = {
                                Text(
                                    "Сообщение",
                                    color = Color(0xFF6C7883),
                                    fontSize = 16.sp
                                )
                            },
                            colors = TextFieldDefaults.colors(
                                focusedContainerColor = Color.Transparent,
                                unfocusedContainerColor = Color.Transparent,
                                focusedTextColor = Color.White,
                                cursorColor = TelegramColors.Accent,
                                focusedIndicatorColor = Color.Transparent,
                                unfocusedIndicatorColor = Color.Transparent
                            ),
                            textStyle = TextStyle(fontSize = 16.sp),
                            modifier = Modifier
                                .fillMaxWidth()
                                .heightIn(min = 48.dp, max = 150.dp),
                            maxLines = 6
                        )
                    }

                    // Кнопка эмодзи
                    IconButton(
                        onClick = { showEmojiPicker = !showEmojiPicker },
                        modifier = Modifier.size(44.dp)
                    ) {
                        Icon(
                            Icons.Rounded.EmojiEmotions,
                            contentDescription = "Emoji",
                            tint = if (showEmojiPicker) TelegramColors.Accent else TelegramColors.IconTint,
                            modifier = Modifier.size(24.dp)
                        )
                    }

                    // Кнопка отправки/записи
                    val sendButtonScale by animateFloatAsState(
                        targetValue = if (messageText.isNotEmpty()) 1.1f else 1f,
                        animationSpec = spring(dampingRatio = Spring.DampingRatioMediumBouncy),
                        label = "send_scale"
                    )

                    IconButton(
                        onClick = { },
                        modifier = Modifier
                            .size(44.dp)
                            .scale(sendButtonScale)
                    ) {
                        AnimatedContent(
                            targetState = messageText.isNotEmpty(),
                            transitionSpec = {
                                scaleIn() + fadeIn() togetherWith scaleOut() + fadeOut()
                            },
                            label = "send_icon"
                        ) { hasText ->
                            if (hasText) {
                                Box(
                                    modifier = Modifier
                                        .size(40.dp)
                                        .clip(CircleShape)
                                        .background(TelegramColors.Accent),
                                    contentAlignment = Alignment.Center
                                ) {
                                    Icon(
                                        Icons.Rounded.Send,
                                        contentDescription = "Send",
                                        tint = Color.White,
                                        modifier = Modifier.size(20.dp)
                                    )
                                }
                            } else {
                                Icon(
                                    Icons.Rounded.Mic,
                                    contentDescription = "Voice",
                                    tint = TelegramColors.IconTint,
                                    modifier = Modifier.size(24.dp)
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
fun DateDivider(date: String) {
    Box(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 16.dp),
        contentAlignment = Alignment.Center
    ) {
        Box(
            modifier = Modifier
                .shadow(2.dp, RoundedCornerShape(16.dp))
                .background(
                    TelegramColors.Background.copy(alpha = 0.95f),
                    RoundedCornerShape(16.dp)
                )
                .padding(horizontal = 14.dp, vertical = 6.dp)
        ) {
            Text(
                text = date,
                color = Color.White,
                fontSize = 13.sp,
                fontWeight = FontWeight.Medium
            )
        }
    }
}

@Composable
fun ReplyPreview(message: Message?, onCancel: () -> Unit) {
    if (message == null) return

    Surface(color = TelegramColors.Background) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp, vertical = 10.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Box(
                modifier = Modifier
                    .width(3.dp)
                    .height(40.dp)
                    .background(TelegramColors.Accent, RoundedCornerShape(2.dp))
            )

            Spacer(modifier = Modifier.width(12.dp))

            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = "Ответ",
                    color = TelegramColors.Accent,
                    fontSize = 14.sp,
                    fontWeight = FontWeight.SemiBold
                )
                Text(
                    text = message.text,
                    color = Color.White.copy(alpha = 0.8f),
                    fontSize = 14.sp,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
            }

            IconButton(onClick = onCancel) {
                Icon(
                    Icons.Rounded.Close,
                    contentDescription = "Cancel",
                    tint = TelegramColors.IconTint
                )
            }
        }
    }
}

@Composable
fun AttachMenu(onDismiss: () -> Unit) {
    val attachOptions = listOf(
        Triple(Icons.Rounded.Image, "Галерея", listOf(Color(0xFF7C4DFF), Color(0xFF536DFE))),
        Triple(Icons.Rounded.InsertDriveFile, "Файл", listOf(Color(0xFF00BFA5), Color(0xFF1DE9B6))),
        Triple(Icons.Rounded.LocationOn, "Геолокация", listOf(Color(0xFFFF5252), Color(0xFFFF1744))),
        Triple(Icons.Rounded.Poll, "Опрос", listOf(Color(0xFFFFAB00), Color(0xFFFFD740))),
        Triple(Icons.Rounded.Person, "Контакт", listOf(Color(0xFF2196F3), Color(0xFF448AFF))),
        Triple(Icons.Rounded.MusicNote, "Музыка", listOf(Color(0xFFE91E63), Color(0xFFFF4081)))
    )

    Surface(color = TelegramColors.Background) {
        LazyVerticalGrid(
            columns = GridCells.Fixed(4),
            modifier = Modifier
                .fillMaxWidth()
                .padding(20.dp),
            horizontalArrangement = Arrangement.spacedBy(16.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            items(attachOptions) { (icon, title, colors) ->
                Column(
                    horizontalAlignment = Alignment.CenterHorizontally,
                    modifier = Modifier.clickable { onDismiss() }
                ) {
                    Box(
                        modifier = Modifier
                            .size(52.dp)
                            .shadow(4.dp, CircleShape)
                            .clip(CircleShape)
                            .background(Brush.linearGradient(colors)),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(icon, contentDescription = title, tint = Color.White, modifier = Modifier.size(26.dp))
                    }
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = title,
                        color = Color(0xFF6C7883),
                        fontSize = 12.sp,
                        textAlign = TextAlign.Center,
                        fontWeight = FontWeight.Medium
                    )
                }
            }
        }
    }
}

@Composable
fun EmojiPicker(
    onEmojiSelected: (String) -> Unit,
    onDismiss: () -> Unit
) {
    var selectedCategory by remember { mutableStateOf(0) }
    val categories = listOf("😀", "🐱", "🍔", "⚽", "🚗", "💡", "❤️", "🏳️")

    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .height(300.dp),
        color = TelegramColors.InputBackground
    ) {
        Column {
            // Категории эмодзи
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .background(TelegramColors.Background)
                    .padding(vertical = 8.dp, horizontal = 4.dp),
                horizontalArrangement = Arrangement.SpaceEvenly
            ) {
                categories.forEachIndexed { index, emoji ->
                    Box(
                        modifier = Modifier
                            .clip(CircleShape)
                            .background(
                                if (selectedCategory == index) TelegramColors.Accent.copy(alpha = 0.2f)
                                else Color.Transparent
                            )
                            .clickable { selectedCategory = index }
                            .padding(10.dp)
                    ) {
                        Text(text = emoji, fontSize = 24.sp)
                    }
                }
            }

            // Сетка эмодзи
            LazyVerticalGrid(
                columns = GridCells.Fixed(8),
                modifier = Modifier
                    .fillMaxSize()
                    .padding(8.dp),
                contentPadding = PaddingValues(4.dp),
                horizontalArrangement = Arrangement.spacedBy(4.dp),
                verticalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                items(emojiList) { emoji ->
                    Box(
                        modifier = Modifier
                            .clip(RoundedCornerShape(8.dp))
                            .clickable { onEmojiSelected(emoji) }
                            .padding(6.dp),
                        contentAlignment = Alignment.Center
                    ) {
                        Text(text = emoji, fontSize = 28.sp)
                    }
                }
            }
        }
    }
}

@OptIn(ExperimentalFoundationApi::class)
@Composable
fun MessageBubble(
    message: Message,
    onLongPress: () -> Unit,
    onReplyClick: () -> Unit,
    onReactionClick: (String) -> Unit
) {
    var showReactions by remember { mutableStateOf(false) }
    val bubbleScale by animateFloatAsState(
        targetValue = if (showReactions) 0.97f else 1f,
        animationSpec = spring(dampingRatio = Spring.DampingRatioMediumBouncy),
        label = "bubble_scale"
    )

    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 2.dp),
        horizontalAlignment = if (message.isFromMe) Alignment.End else Alignment.Start
    ) {
        // Quick reactions popup
        AnimatedVisibility(
            visible = showReactions,
            enter = scaleIn(transformOrigin = if (message.isFromMe) TransformOrigin(1f, 1f) else TransformOrigin(0f, 1f)) + fadeIn(),
            exit = scaleOut(transformOrigin = if (message.isFromMe) TransformOrigin(1f, 1f) else TransformOrigin(0f, 1f)) + fadeOut()
        ) {
            Row(
                modifier = Modifier
                    .shadow(8.dp, RoundedCornerShape(24.dp))
                    .background(TelegramColors.Background, RoundedCornerShape(24.dp))
                    .padding(horizontal = 12.dp, vertical = 8.dp),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                quickReactions.forEach { emoji ->
                    Box(
                        modifier = Modifier
                            .size(36.dp)
                            .clip(CircleShape)
                            .clickable {
                                onReactionClick(emoji)
                                showReactions = false
                            },
                        contentAlignment = Alignment.Center
                    ) {
                        Text(
                            text = emoji,
                            fontSize = 24.sp
                        )
                    }
                }
            }
        }

        Spacer(modifier = Modifier.height(4.dp))

        // Message bubble
        Box(
            modifier = Modifier
                .widthIn(max = 300.dp)
                .scale(bubbleScale)
                .shadow(
                    elevation = 2.dp,
                    shape = RoundedCornerShape(
                        topStart = 18.dp,
                        topEnd = 18.dp,
                        bottomStart = if (message.isFromMe) 18.dp else 6.dp,
                        bottomEnd = if (message.isFromMe) 6.dp else 18.dp
                    ),
                    ambientColor = Color.Black.copy(alpha = 0.2f)
                )
                .clip(
                    RoundedCornerShape(
                        topStart = 18.dp,
                        topEnd = 18.dp,
                        bottomStart = if (message.isFromMe) 18.dp else 6.dp,
                        bottomEnd = if (message.isFromMe) 6.dp else 18.dp
                    )
                )
                .background(
                    if (message.isFromMe) {
                        Brush.linearGradient(
                            colors = listOf(
                                TelegramColors.MyMessageBubble,
                                TelegramColors.MyMessageBubbleEnd
                            )
                        )
                    } else {
                        Brush.linearGradient(
                            colors = listOf(
                                TelegramColors.OtherMessageBubble,
                                TelegramColors.OtherMessageBubble
                            )
                        )
                    }
                )
                .combinedClickable(
                    onClick = { },
                    onLongClick = {
                        showReactions = !showReactions
                        onLongPress()
                    }
                )
                .padding(horizontal = 14.dp, vertical = 10.dp)
        ) {
            Column {
                // Reply preview
                if (message.replyTo != null) {
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .clip(RoundedCornerShape(8.dp))
                            .background(Color.Black.copy(alpha = 0.15f))
                            .padding(8.dp)
                    ) {
                        Box(
                            modifier = Modifier
                                .width(3.dp)
                                .height(36.dp)
                                .background(TelegramColors.Accent, RoundedCornerShape(2.dp))
                        )
                        Spacer(modifier = Modifier.width(10.dp))
                        Column {
                            Text(
                                text = if (message.replyTo.isFromMe) "Вы" else "Собеседник",
                                color = TelegramColors.Accent,
                                fontSize = 13.sp,
                                fontWeight = FontWeight.SemiBold
                            )
                            Text(
                                text = message.replyTo.text,
                                color = Color.White.copy(alpha = 0.75f),
                                fontSize = 13.sp,
                                maxLines = 2,
                                overflow = TextOverflow.Ellipsis
                            )
                        }
                    }
                    Spacer(modifier = Modifier.height(8.dp))
                }

                // Forwarded from
                if (message.forwardedFrom != null) {
                    Row(
                        modifier = Modifier.padding(bottom = 6.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Icon(
                            Icons.Rounded.Reply,
                            contentDescription = null,
                            tint = TelegramColors.Accent,
                            modifier = Modifier.size(14.dp)
                        )
                        Spacer(modifier = Modifier.width(4.dp))
                        Text(
                            text = "Переслано от ${message.forwardedFrom}",
                            color = TelegramColors.Accent,
                            fontSize = 12.sp,
                            fontWeight = FontWeight.Medium
                        )
                    }
                }

                // Message text
                Text(
                    text = message.text,
                    color = Color.White,
                    fontSize = 15.sp,
                    lineHeight = 21.sp
                )

                Spacer(modifier = Modifier.height(4.dp))

                // Time and status
                Row(
                    modifier = Modifier.align(Alignment.End),
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.spacedBy(4.dp)
                ) {
                    if (message.isEdited) {
                        Text(
                            text = "ред.",
                            color = Color.White.copy(alpha = 0.5f),
                            fontSize = 11.sp
                        )
                    }
                    Text(
                        text = message.time,
                        color = Color.White.copy(alpha = 0.6f),
                        fontSize = 12.sp
                    )
                    if (message.isFromMe) {
                        Icon(
                            if (message.isRead) Icons.Filled.DoneAll else Icons.Filled.Done,
                            contentDescription = null,
                            tint = if (message.isRead) Color(0xFF68D391) else Color.White.copy(alpha = 0.6f),
                            modifier = Modifier.size(16.dp)
                        )
                    }
                }
            }
        }

        // Reactions
        if (message.reactions.isNotEmpty()) {
            Row(
                modifier = Modifier
                    .padding(top = 4.dp, start = 8.dp, end = 8.dp)
                    .shadow(2.dp, RoundedCornerShape(14.dp))
                    .background(
                        TelegramColors.Background.copy(alpha = 0.95f),
                        RoundedCornerShape(14.dp)
                    )
                    .padding(horizontal = 6.dp, vertical = 4.dp),
                horizontalArrangement = Arrangement.spacedBy(6.dp)
            ) {
                message.reactions.forEach { reaction ->
                    Row(
                        verticalAlignment = Alignment.CenterVertically,
                        modifier = Modifier
                            .clip(RoundedCornerShape(10.dp))
                            .background(
                                if (reaction.isSelected) TelegramColors.Accent.copy(alpha = 0.3f)
                                else Color.Transparent
                            )
                            .clickable { onReactionClick(reaction.emoji) }
                            .padding(horizontal = 8.dp, vertical = 4.dp)
                    ) {
                        Text(text = reaction.emoji, fontSize = 16.sp)
                        if (reaction.count > 1) {
                            Spacer(modifier = Modifier.width(4.dp))
                            Text(
                                text = reaction.count.toString(),
                                color = Color.White,
                                fontSize = 13.sp,
                                fontWeight = FontWeight.SemiBold
                            )
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun EmptyChatScreen(modifier: Modifier = Modifier) {
    Box(
        modifier = modifier
            .fillMaxHeight()
            .background(TelegramColors.ChatBackground),
        contentAlignment = Alignment.Center
    ) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Box(
                modifier = Modifier
                    .size(140.dp)
                    .shadow(16.dp, CircleShape, ambientColor = TelegramColors.Accent.copy(alpha = 0.3f))
                    .clip(CircleShape)
                    .background(
                        Brush.linearGradient(
                            colors = listOf(
                                TelegramColors.Accent.copy(alpha = 0.2f),
                                TelegramColors.Accent.copy(alpha = 0.1f)
                            )
                        )
                    ),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    Icons.Rounded.Chat,
                    contentDescription = null,
                    tint = TelegramColors.Accent,
                    modifier = Modifier.size(70.dp)
                )
            }
            Spacer(modifier = Modifier.height(28.dp))
            Text(
                text = "Выберите чат",
                color = Color.White,
                fontSize = 22.sp,
                fontWeight = FontWeight.SemiBold
            )
            Spacer(modifier = Modifier.height(8.dp))
            Text(
                text = "чтобы начать общение",
                color = Color(0xFF6C7883),
                fontSize = 15.sp
            )
        }
    }
}

// ============ ОСТАЛЬНЫЕ ЭКРАНЫ ============

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ProfileScreen(user: UserProfile, onBackClick: () -> Unit) {
    val scrollState = rememberScrollState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.DarkBackground)
    ) {
        // Улучшенная шапка
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .height(280.dp)
        ) {
            // Градиентный фон
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .background(
                        Brush.verticalGradient(
                            colors = listOf(
                                Color(0xFF667EEA),
                                Color(0xFF764BA2),
                                TelegramColors.DarkBackground
                            )
                        )
                    )
            )

            // Кнопки управления
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(top = 40.dp, start = 8.dp, end = 8.dp),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                IconButton(onClick = onBackClick) {
                    Icon(
                        Icons.Rounded.ArrowBack,
                        contentDescription = "Back",
                        tint = Color.White
                    )
                }
                Row {
                    IconButton(onClick = { }) {
                        Icon(
                            Icons.Rounded.Edit,
                            contentDescription = "Edit",
                            tint = Color.White
                        )
                    }
                    IconButton(onClick = { }) {
                        Icon(
                            Icons.Rounded.MoreVert,
                            contentDescription = "More",
                            tint = Color.White
                        )
                    }
                }
            }

            // Аватар и информация
            Column(
                modifier = Modifier
                    .align(Alignment.BottomCenter)
                    .padding(bottom = 20.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Box {
                    Box(
                        modifier = Modifier
                            .size(110.dp)
                            .shadow(12.dp, CircleShape)
                            .clip(CircleShape)
                            .background(
                                Brush.linearGradient(
                                    user.avatarGradient ?: listOf(user.avatarColor, user.avatarColor)
                                )
                            ),
                        contentAlignment = Alignment.Center
                    ) {
                        Text(
                            text = user.name.split(" ").take(2).map { it.firstOrNull() ?: "" }.joinToString("").uppercase(),
                            color = Color.White,
                            fontSize = 40.sp,
                            fontWeight = FontWeight.Bold
                        )
                    }
                    if (user.isPremium) {
                        Box(
                            modifier = Modifier
                                .align(Alignment.BottomEnd)
                                .offset(x = (-4).dp, y = (-4).dp)
                                .size(32.dp)
                                .shadow(4.dp, CircleShape)
                                .clip(CircleShape)
                                .background(Color(0xFFFFD700)),
                            contentAlignment = Alignment.Center
                        ) {
                            Icon(Icons.Filled.Star, contentDescription = null, tint = Color.White, modifier = Modifier.size(20.dp))
                        }
                    }
                }

                Spacer(modifier = Modifier.height(16.dp))

                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text(
                        text = user.name,
                        color = Color.White,
                        fontSize = 24.sp,
                        fontWeight = FontWeight.Bold
                    )
                    if (user.isPremium) {
                        Spacer(modifier = Modifier.width(8.dp))
                        Icon(
                            Icons.Filled.Verified,
                            contentDescription = "Premium",
                            tint = Color(0xFFFFD700),
                            modifier = Modifier.size(22.dp)
                        )
                    }
                }

                Text(
                    text = user.lastSeen,
                    color = TelegramColors.Online,
                    fontSize = 15.sp,
                    fontWeight = FontWeight.Medium
                )
            }
        }

        Column(
            modifier = Modifier
                .fillMaxSize()
                .verticalScroll(scrollState)
        ) {
            Spacer(modifier = Modifier.height(16.dp))

            // Информация об аккаунте
            Surface(
                modifier = Modifier.fillMaxWidth(),
                color = TelegramColors.Background,
                shape = RoundedCornerShape(0.dp)
            ) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Text(
                        text = "Аккаунт",
                        color = TelegramColors.Accent,
                        fontSize = 14.sp,
                        fontWeight = FontWeight.SemiBold,
                        modifier = Modifier.padding(bottom = 12.dp)
                    )

                    ProfileInfoItem(
                        icon = Icons.Rounded.Phone,
                        title = user.phone,
                        subtitle = "Телефон"
                    )

                    HorizontalDivider(color = TelegramColors.Divider.copy(alpha = 0.5f), modifier = Modifier.padding(vertical = 12.dp))

                    ProfileInfoItem(
                        icon = Icons.Rounded.AlternateEmail,
                        title = user.username,
                        subtitle = "Имя пользователя"
                    )

                    HorizontalDivider(color = TelegramColors.Divider.copy(alpha = 0.5f), modifier = Modifier.padding(vertical = 12.dp))

                    ProfileInfoItem(
                        icon = Icons.Rounded.Info,
                        title = user.bio,
                        subtitle = "О себе"
                    )
                }
            }

            Spacer(modifier = Modifier.height(16.dp))

            // Настройки
            Surface(
                modifier = Modifier.fillMaxWidth(),
                color = TelegramColors.Background
            ) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Text(
                        text = "Настройки",
                        color = TelegramColors.Accent,
                        fontSize = 14.sp,
                        fontWeight = FontWeight.SemiBold,
                        modifier = Modifier.padding(bottom = 12.dp)
                    )

                    ProfileSettingItem(
                        icon = Icons.Rounded.Notifications,
                        title = "Уведомления и звуки",
                        iconColor = Color(0xFFE53935)
                    )

                    ProfileSettingItem(
                        icon = Icons.Rounded.Lock,
                        title = "Конфиденциальность",
                        iconColor = Color(0xFF43A047)
                    )

                    ProfileSettingItem(
                        icon = Icons.Rounded.Storage,
                        title = "Данные и память",
                        iconColor = Color(0xFF1E88E5)
                    )

                    ProfileSettingItem(
                        icon = Icons.Rounded.Palette,
                        title = "Оформление",
                        iconColor = Color(0xFF8E24AA)
                    )

                    ProfileSettingItem(
                        icon = Icons.Rounded.Language,
                        title = "Язык",
                        subtitle = "Русский",
                        iconColor = Color(0xFFFF9800)
                    )

                    ProfileSettingItem(
                        icon = Icons.Rounded.Devices,
                        title = "Устройства",
                        subtitle = "3 устройства",
                        iconColor = Color(0xFF00ACC1)
                    )
                }
            }

            Spacer(modifier = Modifier.height(32.dp))
        }
    }
}

@Composable
fun ProfileInfoItem(
    icon: ImageVector,
    title: String,
    subtitle: String
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { }
            .padding(vertical = 8.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Icon(
            icon,
            contentDescription = null,
            tint = Color(0xFF6C7883),
            modifier = Modifier.size(24.dp)
        )
        Spacer(modifier = Modifier.width(20.dp))
        Column {
            Text(
                text = title,
                color = Color.White,
                fontSize = 16.sp
            )
            Text(
                text = subtitle,
                color = Color(0xFF6C7883),
                fontSize = 13.sp
            )
        }
    }
}

@Composable
fun ProfileSettingItem(
    icon: ImageVector,
    title: String,
    subtitle: String? = null,
    iconColor: Color = TelegramColors.IconTint
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { }
            .padding(vertical = 14.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Box(
            modifier = Modifier
                .size(42.dp)
                .clip(CircleShape)
                .background(iconColor.copy(alpha = 0.12f)),
            contentAlignment = Alignment.Center
        ) {
            Icon(
                icon,
                contentDescription = null,
                tint = iconColor,
                modifier = Modifier.size(22.dp)
            )
        }
        Spacer(modifier = Modifier.width(16.dp))
        Column(modifier = Modifier.weight(1f)) {
            Text(
                text = title,
                color = Color.White,
                fontSize = 16.sp
            )
            if (subtitle != null) {
                Text(
                    text = subtitle,
                    color = Color(0xFF6C7883),
                    fontSize = 13.sp
                )
            }
        }
        Icon(
            Icons.Rounded.ChevronRight,
            contentDescription = null,
            tint = Color(0xFF6C7883),
            modifier = Modifier.size(24.dp)
        )
    }
}

// Заглушки для остальных экранов
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen(user: UserProfile, onBackClick: () -> Unit) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.DarkBackground)
    ) {
        TopAppBar(
            title = { Text("Настройки", color = Color.White, fontWeight = FontWeight.SemiBold) },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Rounded.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )
        
        Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
            Text("Настройки", color = Color(0xFF6C7883), fontSize = 16.sp)
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ContactsScreen(onBackClick: () -> Unit) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.DarkBackground)
    ) {
        TopAppBar(
            title = { Text("Контакты", color = Color.White, fontWeight = FontWeight.SemiBold) },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Rounded.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )
        
        Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
            Text("Контакты", color = Color(0xFF6C7883), fontSize = 16.sp)
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CallsScreen(onBackClick: () -> Unit) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.DarkBackground)
    ) {
        TopAppBar(
            title = { Text("Звонки", color = Color.White, fontWeight = FontWeight.SemiBold) },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Rounded.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )
        
        Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
            Text("История звонков", color = Color(0xFF6C7883), fontSize = 16.sp)
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SavedMessagesScreen(onBackClick: () -> Unit) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.DarkBackground)
    ) {
        TopAppBar(
            title = { Text("Избранное", color = Color.White, fontWeight = FontWeight.SemiBold) },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Rounded.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )
        
        Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
            Column(horizontalAlignment = Alignment.CenterHorizontally) {
                Icon(
                    Icons.Rounded.Bookmark,
                    contentDescription = null,
                    tint = Color(0xFF6C7883),
                    modifier = Modifier.size(80.dp)
                )
                Spacer(modifier = Modifier.height(16.dp))
                Text(
                    "Пока нет сохранённых сообщений",
                    color = Color(0xFF6C7883),
                    fontSize = 16.sp
                )
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ArchivedChatsScreen(chats: List<ChatData>, onBackClick: () -> Unit) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.DarkBackground)
    ) {
        TopAppBar(
            title = { Text("Архив", color = Color.White, fontWeight = FontWeight.SemiBold) },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Rounded.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )
        
        Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
            Text("Архив пуст", color = Color(0xFF6C7883), fontSize = 16.sp)
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CreateGroupScreen(onBackClick: () -> Unit) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.DarkBackground)
    ) {
        TopAppBar(
            title = { Text("Новая группа", color = Color.White, fontWeight = FontWeight.SemiBold) },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Rounded.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )
        
        Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
            Text("Создание группы", color = Color(0xFF6C7883), fontSize = 16.sp)
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CreateChannelScreen(onBackClick: () -> Unit) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.DarkBackground)
    ) {
        TopAppBar(
            title = { Text("Новый канал", color = Color.White, fontWeight = FontWeight.SemiBold) },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Rounded.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )
        
        Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
            Text("Создание канала", color = Color(0xFF6C7883), fontSize = 16.sp)
        }
    }
}
