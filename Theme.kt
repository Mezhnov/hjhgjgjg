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
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.rotate
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.myapplication.ui.theme.MyApplicationTheme
import com.example.myapplication.ui.theme.TelegramColors
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.*

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
    val lastMessageSender: String? = null,
    val isTyping: Boolean = false,
    val draft: String? = null
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
    val imageUrl: String? = null
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
            isPremium = true
        )
    }

    // Список чатов
    val chats = remember {
        listOf(
            ChatData(1, "Алексей Петров", "Привет! Как дела? 👋", "12:45", 2, true, true, false, false, false, false, false, Color(0xFF5C6BC0)),
            ChatData(2, "Команда проекта", "Андрей: Встреча в 15:00 📅", "11:30", 5, false, true, false, false, false, true, false, Color(0xFF26A69A), "Андрей"),
            ChatData(3, "Мария Иванова", "Спасибо за помощь! 🙏", "10:15", 0, true, false, false, false, false, false, false, Color(0xFFEF5350), isTyping = true),
            ChatData(4, "Telegram", "Добро пожаловать в Telegram! ✅", "Вчера", 0, false, false, false, true, true, false, false, Color(0xFF5B8DEF)),
            ChatData(5, "Дмитрий Сидоров", "Документы отправил 📄", "Вчера", 1, false, false, false, false, false, false, false, Color(0xFF42A5F5)),
            ChatData(6, "Анна Козлова", "Отличная идея! 💡", "Пн", 0, true, false, false, false, false, false, false, Color(0xFFAB47BC)),
            ChatData(7, "Бот помощник", "Чем могу помочь? 🤖", "Пн", 0, false, false, false, true, false, false, true, Color(0xFFFF9800)),
            ChatData(8, "Семья", "Мама: Фото с праздника 📷🎉", "Вс", 0, false, false, false, false, false, true, false, Color(0xFFFFCA28), "Мама"),
            ChatData(9, "Новости IT", "Срочные новости технологий! 📰", "Вс", 12, false, false, true, true, true, false, false, Color(0xFF78909C)),
            ChatData(10, "Спортзал", "Тренировка в 18:00 💪", "Сб", 0, false, false, false, false, false, false, false, Color(0xFFFF5722), draft = "Буду в 18:")
        )
    }

    // Пункты меню
    val menuItems = listOf(
        DrawerMenuItem(Icons.Outlined.Person, "Мой профиль", screen = Screen.Profile),
        DrawerMenuItem(Icons.Outlined.Group, "Создать группу", screen = Screen.CreateGroup),
        DrawerMenuItem(Icons.Outlined.Campaign, "Создать канал", screen = Screen.NewChannel),
        DrawerMenuItem(Icons.Outlined.PersonAdd, "Контакты", screen = Screen.Contacts),
        DrawerMenuItem(Icons.Outlined.Call, "Звонки", badge = "3", screen = Screen.Calls),
        DrawerMenuItem(Icons.Outlined.Bookmark, "Избранное", screen = Screen.SavedMessages),
        DrawerMenuItem(Icons.Outlined.Settings, "Настройки", screen = Screen.Settings)
    )

    val bottomMenuItems = listOf(
        DrawerMenuItem(Icons.Outlined.Archive, "Архив", badge = "24", badgeColor = Color(0xFF6C7883), screen = Screen.ArchivedChats),
        DrawerMenuItem(Icons.Outlined.PersonAdd, "Пригласить друзей")
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
                drawerContainerColor = TelegramColors.DrawerBackground
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
                                slideInHorizontally { it } togetherWith slideOutHorizontally { -it }
                            } else {
                                slideInHorizontally { -it } togetherWith slideOutHorizontally { it }
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
        animationSpec = tween(300),
        label = "arrow_rotation"
    )

    Column(modifier = Modifier.fillMaxSize()) {
        // Шапка с профилем
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .height(190.dp)
                .background(
                    Brush.verticalGradient(
                        colors = listOf(
                            Color(0xFF5B8DEF),
                            Color(0xFF4A7BD4)
                        )
                    )
                )
                .clickable { isAccountExpanded = !isAccountExpanded }
                .padding(16.dp)
        ) {
            Column(
                modifier = Modifier.align(Alignment.BottomStart)
            ) {
                Box {
                    Box(
                        modifier = Modifier
                            .size(70.dp)
                            .clip(CircleShape)
                            .background(user.avatarColor),
                        contentAlignment = Alignment.Center
                    ) {
                        Text(
                            text = user.name.split(" ").take(2).map { it.firstOrNull() ?: "" }.joinToString("").uppercase(),
                            color = Color.White,
                            fontSize = 26.sp,
                            fontWeight = FontWeight.Bold
                        )
                    }
                    // Premium badge
                    if (user.isPremium) {
                        Box(
                            modifier = Modifier
                                .align(Alignment.BottomEnd)
                                .size(24.dp)
                                .clip(CircleShape)
                                .background(Color(0xFFFFD700)),
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

                Spacer(modifier = Modifier.height(14.dp))

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

                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text(
                        text = user.phone,
                        color = Color.White.copy(alpha = 0.85f),
                        fontSize = 14.sp
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Box(
                        modifier = Modifier
                            .size(8.dp)
                            .clip(CircleShape)
                            .background(Color(0xFF4CAF50))
                    )
                }
            }

            // Стрелка для переключения аккаунтов
            Icon(
                Icons.Filled.KeyboardArrowDown,
                contentDescription = null,
                tint = Color.White,
                modifier = Modifier
                    .align(Alignment.BottomEnd)
                    .padding(bottom = 8.dp)
                    .rotate(rotationAngle)
            )

            // Ночной режим
            IconButton(
                onClick = { },
                modifier = Modifier.align(Alignment.TopEnd)
            ) {
                Icon(
                    Icons.Outlined.DarkMode,
                    contentDescription = "Night Mode",
                    tint = Color.White.copy(alpha = 0.8f)
                )
            }
        }

        // Выбор аккаунта (если раскрыт)
        AnimatedVisibility(
            visible = isAccountExpanded,
            enter = expandVertically() + fadeIn(),
            exit = shrinkVertically() + fadeOut()
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
                HorizontalDivider(color = TelegramColors.Divider, thickness = 0.5.dp)
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clickable { }
                        .padding(16.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Icon(
                        Icons.Filled.Add,
                        contentDescription = null,
                        tint = Color(0xFF5B8DEF),
                        modifier = Modifier.size(24.dp)
                    )
                    Spacer(modifier = Modifier.width(32.dp))
                    Text(
                        "Добавить аккаунт",
                        color = Color(0xFF5B8DEF),
                        fontSize = 15.sp
                    )
                }
                HorizontalDivider(color = TelegramColors.Divider, thickness = 0.5.dp)
            }
        }

        Spacer(modifier = Modifier.height(8.dp))

        // Основные пункты меню
        menuItems.forEach { item ->
            DrawerMenuItemRow(item = item, onClick = { onItemClick(item.screen) })
        }

        Spacer(modifier = Modifier.weight(1f))

        HorizontalDivider(color = TelegramColors.Divider, thickness = 0.5.dp)

        // Нижние пункты меню
        bottomMenuItems.forEach { item ->
            DrawerMenuItemRow(item = item, onClick = { onItemClick(item.screen) })
        }

        // Версия приложения
        Text(
            text = "Telegram Clone v2.0",
            color = Color.Gray,
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
                .background(Color(0xFF5B8DEF)),
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
                Icons.Filled.Check,
                contentDescription = null,
                tint = Color(0xFF5B8DEF),
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
            tint = Color(0xFF8E99A4),
            modifier = Modifier.size(24.dp)
        )

        Spacer(modifier = Modifier.width(24.dp))

        Text(
            text = item.title,
            color = Color.White,
            fontSize = 15.sp,
            modifier = Modifier.weight(1f)
        )

        if (item.badge != null) {
            Box(
                modifier = Modifier
                    .background(item.badgeColor, CircleShape)
                    .padding(horizontal = 8.dp, vertical = 2.dp),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = item.badge,
                    color = Color.White,
                    fontSize = 12.sp,
                    fontWeight = FontWeight.Medium
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
        // Верхняя панель
        TopAppBar(
            title = {
                if (isSearchActive) {
                    TextField(
                        value = searchQuery,
                        onValueChange = { searchQuery = it },
                        placeholder = { Text("Поиск", color = Color.Gray) },
                        colors = TextFieldDefaults.colors(
                            focusedContainerColor = Color.Transparent,
                            unfocusedContainerColor = Color.Transparent,
                            focusedTextColor = Color.White,
                            cursorColor = TelegramColors.Accent,
                            focusedIndicatorColor = Color.Transparent,
                            unfocusedIndicatorColor = Color.Transparent
                        ),
                        singleLine = true,
                        modifier = Modifier.fillMaxWidth()
                    )
                } else {
                    Text(
                        "Telegram",
                        color = Color.White,
                        fontWeight = FontWeight.SemiBold,
                        fontSize = 20.sp
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
                        if (isSearchActive) Icons.Filled.ArrowBack else Icons.Filled.Menu,
                        contentDescription = "Menu",
                        tint = TelegramColors.IconTint
                    )
                }
            },
            actions = {
                IconButton(onClick = { isSearchActive = !isSearchActive }) {
                    Icon(
                        Icons.Filled.Search,
                        contentDescription = "Search",
                        tint = TelegramColors.IconTint
                    )
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(
                containerColor = TelegramColors.Background
            )
        )

        HorizontalDivider(color = TelegramColors.DarkBackground, thickness = 1.dp)

        // Фильтр по папкам
        ScrollableTabRow(
            selectedTabIndex = 0,
            containerColor = TelegramColors.Background,
            contentColor = Color.White,
            edgePadding = 0.dp,
            divider = {}
        ) {
            listOf("Все", "Личные", "Каналы", "Группы", "Боты").forEachIndexed { index, title ->
                Tab(
                    selected = index == 0,
                    onClick = { },
                    text = {
                        Text(
                            title,
                            color = if (index == 0) TelegramColors.Accent else Color(0xFF6C7883),
                            fontWeight = if (index == 0) FontWeight.Medium else FontWeight.Normal
                        )
                    }
                )
            }
        }

        HorizontalDivider(color = TelegramColors.DarkBackground, thickness = 1.dp)

        // Список чатов
        LazyColumn(modifier = Modifier.fillMaxSize()) {
            val pinnedChats = chats.filter { it.isPinned }
            val regularChats = chats.filter { !it.isPinned }

            items(pinnedChats) { chat ->
                ChatListItem(
                    chat = chat,
                    isSelected = chat == selectedChat,
                    onClick = { onChatSelected(chat) }
                )
            }

            if (pinnedChats.isNotEmpty() && regularChats.isNotEmpty()) {
                item {
                    HorizontalDivider(
                        color = TelegramColors.Divider,
                        modifier = Modifier.padding(vertical = 4.dp)
                    )
                }
            }

            items(regularChats) { chat ->
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
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .background(if (isSelected) TelegramColors.SelectedChat else Color.Transparent)
            .clickable(onClick = onClick)
            .padding(horizontal = 16.dp, vertical = 10.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        // Аватар
        Box(contentAlignment = Alignment.BottomEnd) {
            Box(
                modifier = Modifier
                    .size(56.dp)
                    .clip(CircleShape)
                    .background(chat.avatarColor),
                contentAlignment = Alignment.Center
            ) {
                if (chat.isChannel) {
                    Icon(
                        Icons.Filled.Campaign,
                        contentDescription = null,
                        tint = Color.White,
                        modifier = Modifier.size(28.dp)
                    )
                } else if (chat.isGroup) {
                    Icon(
                        Icons.Filled.Group,
                        contentDescription = null,
                        tint = Color.White,
                        modifier = Modifier.size(28.dp)
                    )
                } else if (chat.isBot) {
                    Icon(
                        Icons.Filled.SmartToy,
                        contentDescription = null,
                        tint = Color.White,
                        modifier = Modifier.size(28.dp)
                    )
                } else {
                    Text(
                        text = chat.name.split(" ").take(2).map { it.firstOrNull() ?: "" }.joinToString("").uppercase(),
                        color = Color.White,
                        fontSize = 20.sp,
                        fontWeight = FontWeight.Medium
                    )
                }
            }
            if (chat.isOnline && !chat.isChannel && !chat.isGroup) {
                Box(
                    modifier = Modifier
                        .size(16.dp)
                        .offset(x = 2.dp, y = 2.dp)
                        .clip(CircleShape)
                        .background(TelegramColors.Background)
                        .padding(2.dp)
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
                            Icons.Filled.VolumeOff,
                            contentDescription = null,
                            tint = Color.Gray,
                            modifier = Modifier.size(16.dp)
                        )
                    }
                }
                Row(verticalAlignment = Alignment.CenterVertically) {
                    if (chat.isPinned) {
                        Icon(
                            Icons.Filled.PushPin,
                            contentDescription = null,
                            tint = Color.Gray,
                            modifier = Modifier.size(14.dp)
                        )
                        Spacer(modifier = Modifier.width(4.dp))
                    }
                    Text(
                        text = chat.time,
                        color = if (chat.unreadCount > 0) TelegramColors.Accent else TelegramColors.SecondaryText,
                        fontSize = 13.sp
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
                            color = TelegramColors.SecondaryText,
                            fontSize = 15.sp,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                    }
                } else if (chat.isTyping) {
                    TypingIndicator()
                } else {
                    Text(
                        text = chat.lastMessage,
                        color = TelegramColors.SecondaryText,
                        fontSize = 15.sp,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis,
                        modifier = Modifier.weight(1f)
                    )
                }

                if (chat.unreadCount > 0) {
                    Spacer(modifier = Modifier.width(8.dp))
                    Box(
                        modifier = Modifier
                            .defaultMinSize(minWidth = 24.dp)
                            .height(24.dp)
                            .clip(CircleShape)
                            .background(if (chat.isMuted) TelegramColors.SecondaryText else TelegramColors.Accent)
                            .padding(horizontal = 6.dp),
                        contentAlignment = Alignment.Center
                    ) {
                        Text(
                            text = if (chat.unreadCount > 99) "99+" else chat.unreadCount.toString(),
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

@Composable
fun TypingIndicator() {
    val infiniteTransition = rememberInfiniteTransition(label = "typing")

    Row(verticalAlignment = Alignment.CenterVertically) {
        Text(
            text = "печатает",
            color = TelegramColors.Accent,
            fontSize = 15.sp
        )
        Spacer(modifier = Modifier.width(4.dp))
        repeat(3) { index ->
            val delay = index * 200
            val alpha by infiniteTransition.animateFloat(
                initialValue = 0.3f,
                targetValue = 1f,
                animationSpec = infiniteRepeatable(
                    animation = tween(600, delayMillis = delay),
                    repeatMode = RepeatMode.Reverse
                ),
                label = "dot_$index"
            )
            Box(
                modifier = Modifier
                    .padding(horizontal = 1.dp)
                    .size(4.dp)
                    .clip(CircleShape)
                    .background(TelegramColors.Accent.copy(alpha = alpha))
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

    Column(
        modifier = modifier
            .fillMaxHeight()
            .background(TelegramColors.DarkBackground)
    ) {
        // Шапка чата
        TopAppBar(
            title = {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    modifier = Modifier.clickable { }
                ) {
                    Box(
                        modifier = Modifier
                            .size(42.dp)
                            .clip(CircleShape)
                            .background(chat.avatarColor),
                        contentAlignment = Alignment.Center
                    ) {
                        if (chat.isChannel) {
                            Icon(Icons.Filled.Campaign, contentDescription = null, tint = Color.White, modifier = Modifier.size(22.dp))
                        } else if (chat.isGroup) {
                            Icon(Icons.Filled.Group, contentDescription = null, tint = Color.White, modifier = Modifier.size(22.dp))
                        } else if (chat.isBot) {
                            Icon(Icons.Filled.SmartToy, contentDescription = null, tint = Color.White, modifier = Modifier.size(22.dp))
                        } else {
                            Text(
                                text = chat.name.split(" ").take(2).map { it.firstOrNull() ?: "" }.joinToString("").uppercase(),
                                color = Color.White,
                                fontSize = 16.sp,
                                fontWeight = FontWeight.Medium
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
                        Text(
                            text = when {
                                chat.isTyping -> "печатает..."
                                chat.isChannel -> "канал"
                                chat.isGroup -> "группа"
                                chat.isBot -> "бот"
                                chat.isOnline -> "в сети"
                                else -> "был(а) недавно"
                            },
                            color = when {
                                chat.isTyping -> TelegramColors.Accent
                                chat.isOnline -> TelegramColors.Online
                                else -> TelegramColors.SecondaryText
                            },
                            fontSize = 13.sp
                        )
                    }
                }
            },
            navigationIcon = {
                if (showBackButton) {
                    IconButton(onClick = onBackClick) {
                        Icon(Icons.Filled.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                    }
                }
            },
            actions = {
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.Videocam, contentDescription = "Video Call", tint = TelegramColors.IconTint)
                }
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.Call, contentDescription = "Call", tint = TelegramColors.IconTint)
                }
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.MoreVert, contentDescription = "More", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(
                containerColor = TelegramColors.Background
            )
        )

        // Сообщения
        Box(modifier = Modifier.weight(1f)) {
            LazyColumn(
                state = listState,
                modifier = Modifier
                    .fillMaxSize()
                    .padding(horizontal = 8.dp),
                reverseLayout = true,
                contentPadding = PaddingValues(vertical = 8.dp)
            ) {
                items(messages.reversed()) { message ->
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

        // Поле ввода
        Surface(
            color = TelegramColors.Background,
            shadowElevation = 8.dp
        ) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(8.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                IconButton(onClick = { showAttachMenu = !showAttachMenu }) {
                    Icon(
                        Icons.Filled.AttachFile,
                        contentDescription = "Attach",
                        tint = if (showAttachMenu) TelegramColors.Accent else TelegramColors.IconTint
                    )
                }

                TextField(
                    value = messageText,
                    onValueChange = { messageText = it },
                    placeholder = { Text("Сообщение", color = TelegramColors.SecondaryText) },
                    colors = TextFieldDefaults.colors(
                        focusedContainerColor = TelegramColors.InputBackground,
                        unfocusedContainerColor = TelegramColors.InputBackground,
                        focusedTextColor = Color.White,
                        cursorColor = TelegramColors.Accent,
                        focusedIndicatorColor = Color.Transparent,
                        unfocusedIndicatorColor = Color.Transparent
                    ),
                    shape = RoundedCornerShape(24.dp),
                    modifier = Modifier
                        .weight(1f)
                        .heightIn(min = 48.dp),
                    maxLines = 4
                )

                IconButton(onClick = { showEmojiPicker = !showEmojiPicker }) {
                    Icon(
                        Icons.Filled.EmojiEmotions,
                        contentDescription = "Emoji",
                        tint = if (showEmojiPicker) TelegramColors.Accent else TelegramColors.IconTint
                    )
                }

                IconButton(onClick = { }) {
                    Icon(
                        if (messageText.isNotEmpty()) Icons.Filled.Send else Icons.Filled.Mic,
                        contentDescription = "Send",
                        tint = TelegramColors.Accent
                    )
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
                .background(
                    TelegramColors.Background.copy(alpha = 0.8f),
                    RoundedCornerShape(12.dp)
                )
                .padding(horizontal = 12.dp, vertical = 4.dp)
        ) {
            Text(
                text = date,
                color = Color.White,
                fontSize = 13.sp
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
                .padding(horizontal = 16.dp, vertical = 8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Box(
                modifier = Modifier
                    .width(3.dp)
                    .height(36.dp)
                    .background(TelegramColors.Accent, RoundedCornerShape(2.dp))
            )

            Spacer(modifier = Modifier.width(12.dp))

            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = "Ответ",
                    color = TelegramColors.Accent,
                    fontSize = 13.sp,
                    fontWeight = FontWeight.Medium
                )
                Text(
                    text = message.text,
                    color = Color.White,
                    fontSize = 14.sp,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
            }

            IconButton(onClick = onCancel) {
                Icon(
                    Icons.Filled.Close,
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
        Triple(Icons.Filled.Image, "Галерея", Color(0xFF7C4DFF)),
        Triple(Icons.Filled.InsertDriveFile, "Файл", Color(0xFF00BFA5)),
        Triple(Icons.Filled.LocationOn, "Геолокация", Color(0xFFFF5252)),
        Triple(Icons.Filled.Poll, "Опрос", Color(0xFFFFAB00)),
        Triple(Icons.Filled.Person, "Контакт", Color(0xFF2196F3)),
        Triple(Icons.Filled.MusicNote, "Музыка", Color(0xFFE91E63))
    )

    Surface(color = TelegramColors.Background) {
        LazyVerticalGrid(
            columns = GridCells.Fixed(4),
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalArrangement = Arrangement.spacedBy(8.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            items(attachOptions) { (icon, title, color) ->
                Column(
                    horizontalAlignment = Alignment.CenterHorizontally,
                    modifier = Modifier.clickable { onDismiss() }
                ) {
                    Box(
                        modifier = Modifier
                            .size(56.dp)
                            .clip(CircleShape)
                            .background(color),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(icon, contentDescription = title, tint = Color.White, modifier = Modifier.size(26.dp))
                    }
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = title,
                        color = TelegramColors.SecondaryText,
                        fontSize = 12.sp,
                        textAlign = TextAlign.Center
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
            .height(280.dp),
        color = TelegramColors.InputBackground
    ) {
        Column {
            // Категории эмодзи
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .background(TelegramColors.Background)
                    .padding(4.dp),
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
                            .padding(8.dp)
                    ) {
                        Text(text = emoji, fontSize = 22.sp)
                    }
                }
            }

            // Сетка эмодзи
            LazyVerticalGrid(
                columns = GridCells.Fixed(8),
                modifier = Modifier
                    .fillMaxSize()
                    .padding(4.dp),
                contentPadding = PaddingValues(4.dp)
            ) {
                items(emojiList) { emoji ->
                    Box(
                        modifier = Modifier
                            .clickable { onEmojiSelected(emoji) }
                            .padding(4.dp),
                        contentAlignment = Alignment.Center
                    ) {
                        Text(text = emoji, fontSize = 26.sp)
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

    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 2.dp),
        horizontalAlignment = if (message.isFromMe) Alignment.End else Alignment.Start
    ) {
        // Quick reactions
        AnimatedVisibility(
            visible = showReactions,
            enter = scaleIn() + fadeIn(),
            exit = scaleOut() + fadeOut()
        ) {
            Row(
                modifier = Modifier
                    .background(TelegramColors.Background, RoundedCornerShape(20.dp))
                    .padding(horizontal = 8.dp, vertical = 4.dp)
            ) {
                quickReactions.forEach { emoji ->
                    Text(
                        text = emoji,
                        fontSize = 22.sp,
                        modifier = Modifier
                            .clickable {
                                onReactionClick(emoji)
                                showReactions = false
                            }
                            .padding(4.dp)
                    )
                }
            }
        }

        Box(
            modifier = Modifier
                .widthIn(max = 320.dp)
                .clip(RoundedCornerShape(
                    topStart = 18.dp,
                    topEnd = 18.dp,
                    bottomStart = if (message.isFromMe) 18.dp else 4.dp,
                    bottomEnd = if (message.isFromMe) 4.dp else 18.dp
                ))
                .background(
                    if (message.isFromMe) TelegramColors.MyMessageBubble else TelegramColors.OtherMessageBubble
                )
                .combinedClickable(
                    onClick = { },
                    onLongClick = {
                        showReactions = !showReactions
                        onLongPress()
                    }
                )
                .padding(horizontal = 14.dp, vertical = 8.dp)
        ) {
            Column {
                // Reply preview
                if (message.replyTo != null) {
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(bottom = 6.dp)
                    ) {
                        Box(
                            modifier = Modifier
                                .width(2.dp)
                                .height(32.dp)
                                .background(TelegramColors.Accent, RoundedCornerShape(1.dp))
                        )
                        Spacer(modifier = Modifier.width(8.dp))
                        Column {
                            Text(
                                text = if (message.replyTo.isFromMe) "Вы" else "Собеседник",
                                color = TelegramColors.Accent,
                                fontSize = 12.sp,
                                fontWeight = FontWeight.Medium
                            )
                            Text(
                                text = message.replyTo.text,
                                color = Color.White.copy(alpha = 0.7f),
                                fontSize = 13.sp,
                                maxLines = 1,
                                overflow = TextOverflow.Ellipsis
                            )
                        }
                    }
                }

                // Forwarded from
                if (message.forwardedFrom != null) {
                    Row(
                        modifier = Modifier.padding(bottom = 4.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Icon(
                            Icons.Filled.Reply,
                            contentDescription = null,
                            tint = TelegramColors.Accent,
                            modifier = Modifier.size(14.dp)
                        )
                        Spacer(modifier = Modifier.width(4.dp))
                        Text(
                            text = "Переслано от ${message.forwardedFrom}",
                            color = TelegramColors.Accent,
                            fontSize = 12.sp
                        )
                    }
                }

                Text(
                    text = message.text,
                    color = Color.White,
                    fontSize = 15.sp,
                    lineHeight = 20.sp
                )

                Spacer(modifier = Modifier.height(2.dp))

                Row(
                    modifier = Modifier.align(Alignment.End),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    if (message.isEdited) {
                        Text(
                            text = "ред.",
                            color = TelegramColors.SecondaryText,
                            fontSize = 11.sp
                        )
                        Spacer(modifier = Modifier.width(4.dp))
                    }
                    Text(
                        text = message.time,
                        color = TelegramColors.SecondaryText,
                        fontSize = 12.sp
                    )
                    if (message.isFromMe) {
                        Spacer(modifier = Modifier.width(4.dp))
                        Icon(
                            if (message.isRead) Icons.Filled.DoneAll else Icons.Filled.Done,
                            contentDescription = null,
                            tint = if (message.isRead) TelegramColors.Accent else TelegramColors.SecondaryText,
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
                    .padding(top = 4.dp)
                    .background(
                        TelegramColors.Background.copy(alpha = 0.8f),
                        RoundedCornerShape(12.dp)
                    )
                    .padding(horizontal = 8.dp, vertical = 4.dp),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                message.reactions.forEach { reaction ->
                    Row(
                        verticalAlignment = Alignment.CenterVertically,
                        modifier = Modifier
                            .background(
                                if (reaction.isSelected) TelegramColors.Accent.copy(alpha = 0.3f)
                                else Color.Transparent,
                                RoundedCornerShape(8.dp)
                            )
                            .clickable { onReactionClick(reaction.emoji) }
                            .padding(horizontal = 6.dp, vertical = 2.dp)
                    ) {
                        Text(text = reaction.emoji, fontSize = 14.sp)
                        if (reaction.count > 1) {
                            Spacer(modifier = Modifier.width(4.dp))
                            Text(
                                text = reaction.count.toString(),
                                color = Color.White,
                                fontSize = 12.sp
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
            .background(TelegramColors.DarkBackground),
        contentAlignment = Alignment.Center
    ) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Box(
                modifier = Modifier
                    .size(120.dp)
                    .clip(CircleShape)
                    .background(TelegramColors.Background),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    Icons.Filled.Chat,
                    contentDescription = null,
                    tint = TelegramColors.Accent,
                    modifier = Modifier.size(60.dp)
                )
            }
            Spacer(modifier = Modifier.height(24.dp))
            Text(
                text = "Выберите чат",
                color = Color.White,
                fontSize = 20.sp,
                fontWeight = FontWeight.SemiBold
            )
            Spacer(modifier = Modifier.height(8.dp))
            Text(
                text = "чтобы начать общение",
                color = TelegramColors.SecondaryText,
                fontSize = 15.sp
            )
        }
    }
}

// Экран профиля
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ProfileScreen(user: UserProfile, onBackClick: () -> Unit) {
    val scrollState = rememberScrollState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.DarkBackground)
    ) {
        TopAppBar(
            title = { Text("Профиль", color = Color.White) },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Filled.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            actions = {
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.Edit, contentDescription = "Edit", tint = TelegramColors.IconTint)
                }
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.MoreVert, contentDescription = "More", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )

        Column(
            modifier = Modifier
                .fillMaxSize()
                .verticalScroll(scrollState)
        ) {
            // Аватар и основная информация
            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .background(TelegramColors.Background)
                    .padding(24.dp),
                contentAlignment = Alignment.Center
            ) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Box {
                        Box(
                            modifier = Modifier
                                .size(100.dp)
                                .clip(CircleShape)
                                .background(user.avatarColor),
                            contentAlignment = Alignment.Center
                        ) {
                            Text(
                                text = user.name.split(" ").take(2).map { it.firstOrNull() ?: "" }.joinToString("").uppercase(),
                                color = Color.White,
                                fontSize = 36.sp,
                                fontWeight = FontWeight.Bold
                            )
                        }
                        if (user.isPremium) {
                            Box(
                                modifier = Modifier
                                    .align(Alignment.BottomEnd)
                                    .size(32.dp)
                                    .clip(CircleShape)
                                    .background(Color(0xFFFFD700)),
                                contentAlignment = Alignment.Center
                            ) {
                                Icon(Icons.Filled.Star, contentDescription = null, tint = Color.White, modifier = Modifier.size(20.dp))
                            }
                        }
                        // Кнопка изменения фото
                        Box(
                            modifier = Modifier
                                .align(Alignment.BottomEnd)
                                .offset(x = 8.dp, y = 8.dp)
                                .size(36.dp)
                                .clip(CircleShape)
                                .background(TelegramColors.Accent)
                                .clickable { },
                            contentAlignment = Alignment.Center
                        ) {
                            Icon(Icons.Filled.CameraAlt, contentDescription = null, tint = Color.White, modifier = Modifier.size(20.dp))
                        }
                    }

                    Spacer(modifier = Modifier.height(16.dp))

                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Text(
                            text = user.name,
                            color = Color.White,
                            fontSize = 22.sp,
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
                        fontSize = 14.sp
                    )
                }
            }

            Spacer(modifier = Modifier.height(16.dp))

            // Информация об аккаунте
            Surface(
                modifier = Modifier.fillMaxWidth(),
                color = TelegramColors.Background
            ) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Text(
                        text = "Аккаунт",
                        color = TelegramColors.Accent,
                        fontSize = 14.sp,
                        fontWeight = FontWeight.Medium,
                        modifier = Modifier.padding(bottom = 8.dp)
                    )

                    ProfileInfoItem(
                        icon = Icons.Filled.Phone,
                        title = user.phone,
                        subtitle = "Телефон",
                        onClick = { }
                    )

                    HorizontalDivider(color = TelegramColors.Divider, modifier = Modifier.padding(vertical = 8.dp))

                    ProfileInfoItem(
                        icon = Icons.Filled.AlternateEmail,
                        title = user.username,
                        subtitle = "Имя пользователя",
                        onClick = { }
                    )

                    HorizontalDivider(color = TelegramColors.Divider, modifier = Modifier.padding(vertical = 8.dp))

                    ProfileInfoItem(
                        icon = Icons.Filled.Info,
                        title = user.bio,
                        subtitle = "О себе",
                        onClick = { }
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
                        fontWeight = FontWeight.Medium,
                        modifier = Modifier.padding(bottom = 8.dp)
                    )

                    ProfileSettingItem(
                        icon = Icons.Filled.Notifications,
                        title = "Уведомления и звуки",
                        iconColor = Color(0xFFE53935)
                    )

                    ProfileSettingItem(
                        icon = Icons.Filled.Lock,
                        title = "Конфиденциальность",
                        iconColor = Color(0xFF43A047)
                    )

                    ProfileSettingItem(
                        icon = Icons.Filled.Storage,
                        title = "Данные и память",
                        iconColor = Color(0xFF1E88E5)
                    )

                    ProfileSettingItem(
                        icon = Icons.Filled.Palette,
                        title = "Оформление",
                        iconColor = Color(0xFF8E24AA)
                    )

                    ProfileSettingItem(
                        icon = Icons.Filled.Language,
                        title = "Язык",
                        subtitle = "Русский",
                        iconColor = Color(0xFFFF9800)
                    )

                    ProfileSettingItem(
                        icon = Icons.Filled.Devices,
                        title = "Устройства",
                        subtitle = "3 устройства",
                        iconColor = Color(0xFF00ACC1)
                    )
                }
            }

            Spacer(modifier = Modifier.height(16.dp))

            // Premium
            if (!user.isPremium) {
                Surface(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 16.dp),
                    color = Color(0xFF2B3E50),
                    shape = RoundedCornerShape(12.dp)
                ) {
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .clickable { }
                            .padding(16.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Icon(
                            Icons.Filled.Star,
                            contentDescription = null,
                            tint = Color(0xFFFFD700),
                            modifier = Modifier.size(32.dp)
                        )
                        Spacer(modifier = Modifier.width(16.dp))
                        Column(modifier = Modifier.weight(1f)) {
                            Text(
                                text = "Telegram Premium",
                                color = Color.White,
                                fontSize = 16.sp,
                                fontWeight = FontWeight.SemiBold
                            )
                            Text(
                                text = "Удвоенные лимиты, эксклюзивные стикеры и многое другое",
                                color = TelegramColors.SecondaryText,
                                fontSize = 13.sp
                            )
                        }
                    }
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
    subtitle: String,
    onClick: () -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
            .padding(vertical = 8.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Icon(
            icon,
            contentDescription = null,
            tint = TelegramColors.SecondaryText,
            modifier = Modifier.size(24.dp)
        )
        Spacer(modifier = Modifier.width(16.dp))
        Column {
            Text(
                text = title,
                color = Color.White,
                fontSize = 16.sp
            )
            Text(
                text = subtitle,
                color = TelegramColors.SecondaryText,
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
            .padding(vertical = 12.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Box(
            modifier = Modifier
                .size(40.dp)
                .clip(CircleShape)
                .background(iconColor.copy(alpha = 0.15f)),
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
                    color = TelegramColors.SecondaryText,
                    fontSize = 13.sp
                )
            }
        }
        Icon(
            Icons.Filled.ChevronRight,
            contentDescription = null,
            tint = TelegramColors.SecondaryText,
            modifier = Modifier.size(24.dp)
        )
    }
}

// Экран настроек
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen(user: UserProfile, onBackClick: () -> Unit) {
    val scrollState = rememberScrollState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.DarkBackground)
    ) {
        TopAppBar(
            title = { Text("Настройки", color = Color.White) },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Filled.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            actions = {
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.Search, contentDescription = "Search", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )

        Column(
            modifier = Modifier
                .fillMaxSize()
                .verticalScroll(scrollState)
        ) {
            // Профиль пользователя
            Surface(
                modifier = Modifier.fillMaxWidth(),
                color = TelegramColors.Background
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
                            .size(64.dp)
                            .clip(CircleShape)
                            .background(user.avatarColor),
                        contentAlignment = Alignment.Center
                    ) {
                        Text(
                            text = user.name.split(" ").take(2).map { it.firstOrNull() ?: "" }.joinToString("").uppercase(),
                            color = Color.White,
                            fontSize = 24.sp,
                            fontWeight = FontWeight.Bold
                        )
                    }
                    Spacer(modifier = Modifier.width(16.dp))
                    Column(modifier = Modifier.weight(1f)) {
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
                                    contentDescription = null,
                                    tint = Color(0xFFFFD700),
                                    modifier = Modifier.size(18.dp)
                                )
                            }
                        }
                        Text(
                            text = user.phone,
                            color = TelegramColors.SecondaryText,
                            fontSize = 14.sp
                        )
                        Text(
                            text = user.username,
                            color = TelegramColors.Accent,
                            fontSize = 14.sp
                        )
                    }
                    Icon(
                        Icons.Filled.QrCode,
                        contentDescription = "QR Code",
                        tint = TelegramColors.Accent,
                        modifier = Modifier.size(28.dp)
                    )
                }
            }

            Spacer(modifier = Modifier.height(16.dp))

            // Premium (если нет)
            if (!user.isPremium) {
                Surface(
                    modifier = Modifier.fillMaxWidth(),
                    color = TelegramColors.Background
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
                                .size(40.dp)
                                .clip(CircleShape)
                                .background(
                                    Brush.linearGradient(
                                        colors = listOf(Color(0xFFFFD700), Color(0xFFFFA000))
                                    )
                                ),
                            contentAlignment = Alignment.Center
                        ) {
                            Icon(Icons.Filled.Star, contentDescription = null, tint = Color.White, modifier = Modifier.size(24.dp))
                        }
                        Spacer(modifier = Modifier.width(16.dp))
                        Column(modifier = Modifier.weight(1f)) {
                            Text(
                                text = "Telegram Premium",
                                color = Color.White,
                                fontSize = 16.sp,
                                fontWeight = FontWeight.Medium
                            )
                            Text(
                                text = "Новые функции и возможности",
                                color = TelegramColors.SecondaryText,
                                fontSize = 13.sp
                            )
                        }
                    }
                }
                Spacer(modifier = Modifier.height(16.dp))
            }

            // Основные настройки
            Surface(
                modifier = Modifier.fillMaxWidth(),
                color = TelegramColors.Background
            ) {
                Column {
                    SettingsItem(
                        icon = Icons.Filled.Notifications,
                        title = "Уведомления и звуки",
                        iconColor = Color(0xFFE53935)
                    )
                    SettingsItem(
                        icon = Icons.Filled.Lock,
                        title = "Конфиденциальность",
                        iconColor = Color(0xFF43A047)
                    )
                    SettingsItem(
                        icon = Icons.Filled.DataUsage,
                        title = "Данные и память",
                        iconColor = Color(0xFF1E88E5)
                    )
                    SettingsItem(
                        icon = Icons.Filled.Palette,
                        title = "Оформление",
                        iconColor = Color(0xFF8E24AA)
                    )
                    SettingsItem(
                        icon = Icons.Filled.BatteryChargingFull,
                        title = "Энергосбережение",
                        iconColor = Color(0xFF00BCD4)
                    )
                    SettingsItem(
                        icon = Icons.Filled.Language,
                        title = "Язык",
                        subtitle = "Русский",
                        iconColor = Color(0xFFFF9800)
                    )
                }
            }

            Spacer(modifier = Modifier.height(16.dp))

            // Дополнительные настройки
            Surface(
                modifier = Modifier.fillMaxWidth(),
                color = TelegramColors.Background
            ) {
                Column {
                    SettingsItem(
                        icon = Icons.Filled.Folder,
                        title = "Папки с чатами",
                        iconColor = Color(0xFF7B1FA2)
                    )
                    SettingsItem(
                        icon = Icons.Filled.Devices,
                        title = "Устройства",
                        subtitle = "3 устройства",
                        iconColor = Color(0xFF00ACC1)
                    )
                }
            }

            Spacer(modifier = Modifier.height(16.dp))

            // Помощь
            Surface(
                modifier = Modifier.fillMaxWidth(),
                color = TelegramColors.Background
            ) {
                Column {
                    SettingsItem(
                        icon = Icons.Filled.Help,
                        title = "Вопросы о Telegram",
                        iconColor = Color(0xFF5B8DEF)
                    )
                    SettingsItem(
                        icon = Icons.Filled.QuestionAnswer,
                        title = "Задать вопрос",
                        iconColor = Color(0xFF26A69A)
                    )
                }
            }

            Spacer(modifier = Modifier.height(32.dp))

            Text(
                text = "Telegram Clone for Android",
                color = TelegramColors.SecondaryText,
                fontSize = 14.sp,
                modifier = Modifier.fillMaxWidth(),
                textAlign = TextAlign.Center
            )
            Text(
                text = "Версия 2.0.0 (1234)",
                color = TelegramColors.SecondaryText,
                fontSize = 14.sp,
                modifier = Modifier.fillMaxWidth(),
                textAlign = TextAlign.Center
            )

            Spacer(modifier = Modifier.height(32.dp))
        }
    }
}

@Composable
fun SettingsItem(
    icon: ImageVector,
    title: String,
    subtitle: String? = null,
    iconColor: Color = TelegramColors.IconTint,
    onClick: () -> Unit = {}
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
            .padding(horizontal = 16.dp, vertical = 14.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Box(
            modifier = Modifier
                .size(40.dp)
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
                    color = TelegramColors.SecondaryText,
                    fontSize = 13.sp
                )
            }
        }
    }
}

// Остальные экраны (заглушки)
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ContactsScreen(onBackClick: () -> Unit) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.DarkBackground)
    ) {
        TopAppBar(
            title = { Text("Контакты", color = Color.White) },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Filled.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            actions = {
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.Search, contentDescription = "Search", tint = TelegramColors.IconTint)
                }
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.PersonAdd, contentDescription = "Add", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )

        LazyColumn(modifier = Modifier.fillMaxSize()) {
            item {
                ContactItem(name = "Приглашение друзей", icon = Icons.Filled.PersonAdd, isAction = true)
                ContactItem(name = "Найти людей рядом", icon = Icons.Filled.LocationOn, isAction = true)
            }

            item {
                Text(
                    text = "Контакты",
                    color = TelegramColors.Accent,
                    fontSize = 14.sp,
                    fontWeight = FontWeight.Medium,
                    modifier = Modifier.padding(16.dp)
                )
            }

            items(10) { index ->
                ContactItem(
                    name = listOf("Алексей", "Мария", "Дмитрий", "Анна", "Сергей", "Елена", "Павел", "Ольга", "Андрей", "Наталья")[index],
                    status = if (index % 3 == 0) "в сети" else "был(а) недавно",
                    isOnline = index % 3 == 0
                )
            }
        }
    }
}

@Composable
fun ContactItem(
    name: String,
    icon: ImageVector? = null,
    status: String? = null,
    isAction: Boolean = false,
    isOnline: Boolean = false
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { }
            .padding(horizontal = 16.dp, vertical = 12.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        if (isAction && icon != null) {
            Box(
                modifier = Modifier
                    .size(44.dp)
                    .clip(CircleShape)
                    .background(TelegramColors.Accent),
                contentAlignment = Alignment.Center
            ) {
                Icon(icon, contentDescription = null, tint = Color.White, modifier = Modifier.size(24.dp))
            }
        } else {
            Box(
                modifier = Modifier
                    .size(44.dp)
                    .clip(CircleShape)
                    .background(Color(0xFF5C6BC0)),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = name.take(1).uppercase(),
                    color = Color.White,
                    fontSize = 18.sp,
                    fontWeight = FontWeight.Medium
                )
            }
        }

        Spacer(modifier = Modifier.width(16.dp))

        Column {
            Text(
                text = name,
                color = if (isAction) TelegramColors.Accent else Color.White,
                fontSize = 16.sp,
                fontWeight = FontWeight.Medium
            )
            if (status != null) {
                Text(
                    text = status,
                    color = if (isOnline) TelegramColors.Online else TelegramColors.SecondaryText,
                    fontSize = 14.sp
                )
            }
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
            title = { Text("Звонки", color = Color.White) },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Filled.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )

        LazyColumn(modifier = Modifier.fillMaxSize()) {
            items(8) { index ->
                CallItem(
                    name = listOf("Алексей", "Мария", "Дмитрий", "Анна", "Сергей", "Елена", "Павел", "Ольга")[index],
                    time = listOf("Вчера, 18:45", "Вчера, 12:30", "23 фев, 10:15", "22 фев, 20:00", "20 фев, 15:30", "19 фев, 09:00", "18 фев, 21:45", "17 фев, 14:20")[index],
                    isIncoming = index % 2 == 0,
                    isMissed = index == 2 || index == 5,
                    isVideo = index % 3 == 0
                )
            }
        }
    }
}

@Composable
fun CallItem(
    name: String,
    time: String,
    isIncoming: Boolean,
    isMissed: Boolean,
    isVideo: Boolean
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { }
            .padding(horizontal = 16.dp, vertical = 12.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Box(
            modifier = Modifier
                .size(48.dp)
                .clip(CircleShape)
                .background(Color(0xFF5C6BC0)),
            contentAlignment = Alignment.Center
        ) {
            Text(
                text = name.take(1).uppercase(),
                color = Color.White,
                fontSize = 20.sp,
                fontWeight = FontWeight.Medium
            )
        }

        Spacer(modifier = Modifier.width(16.dp))

        Column(modifier = Modifier.weight(1f)) {
            Text(
                text = name,
                color = Color.White,
                fontSize = 16.sp,
                fontWeight = FontWeight.Medium
            )
            Row(verticalAlignment = Alignment.CenterVertically) {
                Icon(
                    if (isIncoming) Icons.Filled.CallReceived else Icons.Filled.CallMade,
                    contentDescription = null,
                    tint = if (isMissed) Color(0xFFE53935) else TelegramColors.Online,
                    modifier = Modifier.size(16.dp)
                )
                Spacer(modifier = Modifier.width(4.dp))
                Text(
                    text = time,
                    color = TelegramColors.SecondaryText,
                    fontSize = 14.sp
                )
            }
        }

        IconButton(onClick = { }) {
            Icon(
                if (isVideo) Icons.Filled.Videocam else Icons.Filled.Call,
                contentDescription = null,
                tint = TelegramColors.Accent
            )
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
            title = {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Box(
                        modifier = Modifier
                            .size(40.dp)
                            .clip(CircleShape)
                            .background(TelegramColors.Accent),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(Icons.Filled.Bookmark, contentDescription = null, tint = Color.White, modifier = Modifier.size(22.dp))
                    }
                    Spacer(modifier = Modifier.width(12.dp))
                    Text("Избранное", color = Color.White)
                }
            },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Filled.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )

        Box(
            modifier = Modifier.fillMaxSize(),
            contentAlignment = Alignment.Center
        ) {
            Column(horizontalAlignment = Alignment.CenterHorizontally) {
                Icon(
                    Icons.Filled.Bookmark,
                    contentDescription = null,
                    tint = TelegramColors.SecondaryText,
                    modifier = Modifier.size(80.dp)
                )
                Spacer(modifier = Modifier.height(16.dp))
                Text(
                    text = "Пока нет сохранённых сообщений",
                    color = TelegramColors.SecondaryText,
                    fontSize = 16.sp
                )
                Spacer(modifier = Modifier.height(8.dp))
                Text(
                    text = "Пересылайте сюда сообщения, чтобы\nсохранить их для быстрого доступа",
                    color = TelegramColors.SecondaryText.copy(alpha = 0.7f),
                    fontSize = 14.sp,
                    textAlign = TextAlign.Center
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
            title = { Text("Архив", color = Color.White) },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Filled.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )

        if (chats.isEmpty()) {
            Box(
                modifier = Modifier.fillMaxSize(),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = "Архив пуст",
                    color = TelegramColors.SecondaryText,
                    fontSize = 16.sp
                )
            }
        } else {
            LazyColumn(modifier = Modifier.fillMaxSize()) {
                items(chats) { chat ->
                    ChatListItem(chat = chat, isSelected = false, onClick = { })
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CreateGroupScreen(onBackClick: () -> Unit) {
    var groupName by remember { mutableStateOf("") }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.DarkBackground)
    ) {
        TopAppBar(
            title = { Text("Новая группа", color = Color.White) },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Filled.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )

        Surface(
            modifier = Modifier.fillMaxWidth(),
            color = TelegramColors.Background
        ) {
            Row(
                modifier = Modifier.padding(16.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Box(
                    modifier = Modifier
                        .size(64.dp)
                        .clip(CircleShape)
                        .background(TelegramColors.Accent),
                    contentAlignment = Alignment.Center
                ) {
                    Icon(Icons.Filled.CameraAlt, contentDescription = null, tint = Color.White, modifier = Modifier.size(32.dp))
                }

                Spacer(modifier = Modifier.width(16.dp))

                TextField(
                    value = groupName,
                    onValueChange = { groupName = it },
                    placeholder = { Text("Введите название группы", color = TelegramColors.SecondaryText) },
                    colors = TextFieldDefaults.colors(
                        focusedContainerColor = Color.Transparent,
                        unfocusedContainerColor = Color.Transparent,
                        focusedTextColor = Color.White,
                        cursorColor = TelegramColors.Accent,
                        focusedIndicatorColor = TelegramColors.Accent,
                        unfocusedIndicatorColor = TelegramColors.Divider
                    ),
                    modifier = Modifier.weight(1f)
                )
            }
        }

        Spacer(modifier = Modifier.height(16.dp))

        Text(
            text = "Участники",
            color = TelegramColors.Accent,
            fontSize = 14.sp,
            fontWeight = FontWeight.Medium,
            modifier = Modifier.padding(16.dp)
        )

        LazyColumn(modifier = Modifier.fillMaxSize()) {
            items(5) { index ->
                ContactItem(
                    name = listOf("Алексей", "Мария", "Дмитрий", "Анна", "Сергей")[index],
                    status = "был(а) недавно"
                )
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CreateChannelScreen(onBackClick: () -> Unit) {
    var channelName by remember { mutableStateOf("") }
    var channelDescription by remember { mutableStateOf("") }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(TelegramColors.DarkBackground)
    ) {
        TopAppBar(
            title = { Text("Новый канал", color = Color.White) },
            navigationIcon = {
                IconButton(onClick = onBackClick) {
                    Icon(Icons.Filled.ArrowBack, contentDescription = "Back", tint = TelegramColors.IconTint)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(containerColor = TelegramColors.Background)
        )

        Surface(
            modifier = Modifier.fillMaxWidth(),
            color = TelegramColors.Background
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Box(
                        modifier = Modifier
                            .size(64.dp)
                            .clip(CircleShape)
                            .background(TelegramColors.Accent),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(Icons.Filled.CameraAlt, contentDescription = null, tint = Color.White, modifier = Modifier.size(32.dp))
                    }

                    Spacer(modifier = Modifier.width(16.dp))

                    TextField(
                        value = channelName,
                        onValueChange = { channelName = it },
                        placeholder = { Text("Название канала", color = TelegramColors.SecondaryText) },
                        colors = TextFieldDefaults.colors(
                            focusedContainerColor = Color.Transparent,
                            unfocusedContainerColor = Color.Transparent,
                            focusedTextColor = Color.White,
                            cursorColor = TelegramColors.Accent,
                            focusedIndicatorColor = TelegramColors.Accent,
                            unfocusedIndicatorColor = TelegramColors.Divider
                        ),
                        modifier = Modifier.weight(1f)
                    )
                }

                Spacer(modifier = Modifier.height(16.dp))

                TextField(
                    value = channelDescription,
                    onValueChange = { channelDescription = it },
                    placeholder = { Text("Описание (необязательно)", color = TelegramColors.SecondaryText) },
                    colors = TextFieldDefaults.colors(
                        focusedContainerColor = Color.Transparent,
                        unfocusedContainerColor = Color.Transparent,
                        focusedTextColor = Color.White,
                        cursorColor = TelegramColors.Accent,
                        focusedIndicatorColor = TelegramColors.Accent,
                        unfocusedIndicatorColor = TelegramColors.Divider
                    ),
                    modifier = Modifier.fillMaxWidth(),
                    maxLines = 3
                )
            }
        }

        Spacer(modifier = Modifier.height(16.dp))

        Text(
            text = "Вы можете добавить описание для своего канала.",
            color = TelegramColors.SecondaryText,
            fontSize = 14.sp,
            modifier = Modifier.padding(horizontal = 16.dp)
        )

        Spacer(modifier = Modifier.height(24.dp))

        Text(
            text = "Тип канала",
            color = TelegramColors.Accent,
            fontSize = 14.sp,
            fontWeight = FontWeight.Medium,
            modifier = Modifier.padding(horizontal = 16.dp, vertical = 8.dp)
        )

        Surface(
            modifier = Modifier.fillMaxWidth(),
            color = TelegramColors.Background
        ) {
            Column {
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clickable { }
                        .padding(16.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    RadioButton(
                        selected = true,
                        onClick = { },
                        colors = RadioButtonDefaults.colors(selectedColor = TelegramColors.Accent)
                    )
                    Spacer(modifier = Modifier.width(16.dp))
                    Column {
                        Text("Публичный канал", color = Color.White, fontSize = 16.sp)
                        Text("Любой может найти и подписаться", color = TelegramColors.SecondaryText, fontSize = 14.sp)
                    }
                }

                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clickable { }
                        .padding(16.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    RadioButton(
                        selected = false,
                        onClick = { },
                        colors = RadioButtonDefaults.colors(selectedColor = TelegramColors.Accent)
                    )
                    Spacer(modifier = Modifier.width(16.dp))
                    Column {
                        Text("Приватный канал", color = Color.White, fontSize = 16.sp)
                        Text("Подписка только по ссылке-приглашению", color = TelegramColors.SecondaryText, fontSize = 14.sp)
                    }
                }
            }
        }
    }
}

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
