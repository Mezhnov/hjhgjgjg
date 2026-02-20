package com.example.myapplication

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.compose.BackHandler
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
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
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog
import com.example.myapplication.ui.theme.MyApplicationTheme
import kotlinx.coroutines.launch

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

// Модель данных для чата
data class Chat(
    val id: Int,
    val name: String,
    val lastMessage: String,
    val time: String,
    val unreadCount: Int = 0,
    val isOnline: Boolean = false,
    val isPinned: Boolean = false,
    val isMuted: Boolean = false,
    val avatarColor: Color
)

// Модель данных для сообщения
data class Message(
    val id: Int,
    val text: String,
    val time: String,
    val isFromMe: Boolean,
    val isRead: Boolean = true
)

// Пункты меню боковой панели
data class DrawerMenuItem(
    val icon: ImageVector,
    val title: String,
    val badge: String? = null
)

// Список популярных эмодзи
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

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TelegramChatApp() {
    val drawerState = rememberDrawerState(initialValue = DrawerValue.Closed)
    val scope = rememberCoroutineScope()
    var selectedChat by remember { mutableStateOf<Chat?>(null) }
    
    // Определяем размер экрана
    val configuration = LocalConfiguration.current
    val isTablet = configuration.screenWidthDp >= 600

    // Список чатов
    val chats = remember {
        listOf(
            Chat(1, "Алексей Петров", "Привет! Как дела? 👋", "12:45", 2, true, true, false, Color(0xFF5C6BC0)),
            Chat(2, "Команда проекта", "Встреча в 15:00 📅", "11:30", 5, false, true, false, Color(0xFF26A69A)),
            Chat(3, "Мария Иванова", "Спасибо за помощь! 🙏", "10:15", 0, true, false, false, Color(0xFFEF5350)),
            Chat(4, "Техподдержка", "Ваш запрос обработан ✅", "Вчера", 0, false, false, true, Color(0xFFFF7043)),
            Chat(5, "Дмитрий Сидоров", "Документы отправил 📄", "Вчера", 1, false, false, false, Color(0xFF42A5F5)),
            Chat(6, "Анна Козлова", "Отличная идея! 💡", "Пн", 0, true, false, false, Color(0xFFAB47BC)),
            Chat(7, "Работа", "Отчет готов к отправке 📊", "Пн", 3, false, false, true, Color(0xFF66BB6A)),
            Chat(8, "Семья", "Фото с праздника 📷🎉", "Вс", 0, false, false, false, Color(0xFFFFCA28)),
            Chat(9, "Новости канал", "Срочные новости! 📰", "Вс", 12, false, false, true, Color(0xFF78909C)),
            Chat(10, "Спортзал", "Тренировка в 18:00 💪", "Сб", 0, false, false, false, Color(0xFFFF5722))
        )
    }

    // Пункты меню
    val menuItems = listOf(
        DrawerMenuItem(Icons.Outlined.Person, "Мой профиль"),
        DrawerMenuItem(Icons.Outlined.Group, "Создать группу"),
        DrawerMenuItem(Icons.Outlined.PersonAdd, "Контакты"),
        DrawerMenuItem(Icons.Outlined.Call, "Звонки"),
        DrawerMenuItem(Icons.Outlined.Bookmark, "Избранное"),
        DrawerMenuItem(Icons.Outlined.Settings, "Настройки"),
        DrawerMenuItem(Icons.Outlined.NightlightRound, "Ночной режим"),
        DrawerMenuItem(Icons.Outlined.Help, "Помощь")
    )

    // Обработка кнопки "Назад" на телефоне
    if (!isTablet && selectedChat != null) {
        BackHandler {
            selectedChat = null
        }
    }

    ModalNavigationDrawer(
        drawerState = drawerState,
        gesturesEnabled = if (isTablet) true else selectedChat == null,
        drawerContent = {
            ModalDrawerSheet(
                modifier = Modifier.width(300.dp),
                drawerContainerColor = Color(0xFF17212B)
            ) {
                DrawerContent(menuItems)
            }
        }
    ) {
        if (isTablet) {
            // Планшетный layout: список чатов + чат рядом
            Row(modifier = Modifier.fillMaxSize()) {
                ChatListPanel(
                    chats = chats,
                    selectedChat = selectedChat,
                    onChatSelected = { selectedChat = it },
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
            // Телефонный layout: только список ИЛИ только чат
            if (selectedChat == null) {
                ChatListPanel(
                    chats = chats,
                    selectedChat = selectedChat,
                    onChatSelected = { selectedChat = it },
                    onMenuClick = { scope.launch { drawerState.open() } },
                    modifier = Modifier.fillMaxWidth()
                )
            } else {
                ChatScreen(
                    chat = selectedChat!!,
                    onBackClick = { selectedChat = null },
                    showBackButton = true,
                    modifier = Modifier.fillMaxWidth()
                )
            }
        }
    }
}

@Composable
fun DrawerContent(menuItems: List<DrawerMenuItem>) {
    Column(modifier = Modifier.fillMaxSize()) {
        // Шапка с профилем
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .height(180.dp)
                .background(Color(0xFF5B8DEF))
                .padding(16.dp)
        ) {
            Column(
                modifier = Modifier.align(Alignment.BottomStart)
            ) {
                Box(
                    modifier = Modifier
                        .size(64.dp)
                        .clip(CircleShape)
                        .background(Color(0xFF3D5AFE)),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        text = "ИП",
                        color = Color.White,
                        fontSize = 24.sp,
                        fontWeight = FontWeight.Bold
                    )
                }
                Spacer(modifier = Modifier.height(12.dp))
                Text(
                    text = "Иван Петров",
                    color = Color.White,
                    fontSize = 18.sp,
                    fontWeight = FontWeight.SemiBold
                )
                Text(
                    text = "+7 999 123-45-67",
                    color = Color.White.copy(alpha = 0.8f),
                    fontSize = 14.sp
                )
            }
            // Стрелка для переключения аккаунтов
            Icon(
                Icons.Filled.KeyboardArrowDown,
                contentDescription = null,
                tint = Color.White,
                modifier = Modifier
                    .align(Alignment.BottomEnd)
                    .padding(bottom = 8.dp)
            )
        }

        Spacer(modifier = Modifier.height(8.dp))

        // Пункты меню
        menuItems.forEach { item ->
            NavigationDrawerItem(
                icon = { 
                    Icon(
                        item.icon, 
                        contentDescription = null, 
                        tint = Color(0xFF8E99A4),
                        modifier = Modifier.size(24.dp)
                    ) 
                },
                label = { 
                    Text(
                        item.title, 
                        color = Color.White,
                        fontSize = 15.sp
                    ) 
                },
                selected = false,
                onClick = { },
                colors = NavigationDrawerItemDefaults.colors(
                    unselectedContainerColor = Color.Transparent
                ),
                modifier = Modifier.padding(horizontal = 12.dp, vertical = 0.dp)
            )
        }

        Spacer(modifier = Modifier.weight(1f))
        
        HorizontalDivider(color = Color(0xFF2B3E50), thickness = 0.5.dp)
        
        // Пригласить друзей
        NavigationDrawerItem(
            icon = { 
                Icon(
                    Icons.Outlined.PersonAdd, 
                    contentDescription = null, 
                    tint = Color(0xFF8E99A4)
                ) 
            },
            label = { Text("Пригласить друзей", color = Color.White) },
            selected = false,
            onClick = { },
            colors = NavigationDrawerItemDefaults.colors(
                unselectedContainerColor = Color.Transparent
            ),
            modifier = Modifier.padding(horizontal = 12.dp)
        )

        // Версия приложения
        Text(
            text = "Telegram Clone v1.0",
            color = Color.Gray,
            fontSize = 12.sp,
            modifier = Modifier.padding(16.dp)
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ChatListPanel(
    chats: List<Chat>,
    selectedChat: Chat?,
    onChatSelected: (Chat) -> Unit,
    onMenuClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    var searchQuery by remember { mutableStateOf("") }
    var isSearchActive by remember { mutableStateOf(false) }
    
    Column(
        modifier = modifier
            .fillMaxHeight()
            .background(Color(0xFF17212B))
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
                            unfocusedTextColor = Color.White,
                            cursorColor = Color(0xFF5B8DEF),
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
                        tint = Color(0xFF8E99A4)
                    )
                }
            },
            actions = {
                IconButton(onClick = { isSearchActive = !isSearchActive }) {
                    Icon(
                        Icons.Filled.Search,
                        contentDescription = "Search",
                        tint = Color(0xFF8E99A4)
                    )
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(
                containerColor = Color(0xFF17212B)
            )
        )
        
        HorizontalDivider(color = Color(0xFF0E1621), thickness = 1.dp)

        // Список чатов
        LazyColumn(modifier = Modifier.fillMaxSize()) {
            // Закрепленные чаты
            val pinnedChats = chats.filter { it.isPinned }
            val regularChats = chats.filter { !it.isPinned }
            
            items(pinnedChats) { chat ->
                ChatListItem(
                    chat = chat,
                    isSelected = chat == selectedChat,
                    onClick = { onChatSelected(chat) }
                )
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
    chat: Chat,
    isSelected: Boolean,
    onClick: () -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .background(if (isSelected) Color(0xFF2B5278) else Color.Transparent)
            .clickable(onClick = onClick)
            .padding(horizontal = 16.dp, vertical = 12.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        // Аватар
        Box(contentAlignment = Alignment.BottomEnd) {
            Box(
                modifier = Modifier
                    .size(54.dp)
                    .clip(CircleShape)
                    .background(chat.avatarColor),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = chat.name.split(" ").take(2).map { it.firstOrNull() ?: "" }.joinToString("").uppercase(),
                    color = Color.White,
                    fontSize = 20.sp,
                    fontWeight = FontWeight.Medium
                )
            }
            if (chat.isOnline) {
                Box(
                    modifier = Modifier
                        .size(16.dp)
                        .offset(x = 2.dp, y = 2.dp)
                        .clip(CircleShape)
                        .background(Color(0xFF17212B))
                        .padding(2.dp)
                ) {
                    Box(
                        modifier = Modifier
                            .fillMaxSize()
                            .clip(CircleShape)
                            .background(Color(0xFF4CAF50))
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
                            modifier = Modifier
                                .size(16.dp)
                                .padding(end = 4.dp)
                        )
                    }
                    Text(
                        text = chat.time,
                        color = if (chat.unreadCount > 0) Color(0xFF5B8DEF) else Color(0xFF6C7883),
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
                Text(
                    text = chat.lastMessage,
                    color = Color(0xFF6C7883),
                    fontSize = 15.sp,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                    modifier = Modifier.weight(1f)
                )
                if (chat.unreadCount > 0) {
                    Spacer(modifier = Modifier.width(8.dp))
                    Box(
                        modifier = Modifier
                            .defaultMinSize(minWidth = 24.dp)
                            .height(24.dp)
                            .clip(CircleShape)
                            .background(if (chat.isMuted) Color(0xFF6C7883) else Color(0xFF5B8DEF))
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

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ChatScreen(
    chat: Chat,
    onBackClick: () -> Unit,
    showBackButton: Boolean,
    modifier: Modifier = Modifier
) {
    val messages = remember {
        listOf(
            Message(1, "Привет! Как дела? 👋", "12:30", false, true),
            Message(2, "Привет! Все отлично, спасибо! 😊", "12:32", true, true),
            Message(3, "Что нового?", "12:33", false, true),
            Message(4, "Работаю над новым проектом. Очень интересно! 💻", "12:35", true, true),
            Message(5, "Звучит здорово! Расскажи подробнее 🤔", "12:40", false, true),
            Message(6, "Это приложение-мессенджер в стиле Telegram 😎✨", "12:42", true, true),
            Message(7, "Вау, круто! Покажешь когда закончишь? 🚀", "12:45", false, true)
        )
    }

    var messageText by remember { mutableStateOf("") }
    var showEmojiPicker by remember { mutableStateOf(false) }

    Column(
        modifier = modifier
            .fillMaxHeight()
            .background(Color(0xFF0E1621))
    ) {
        // Шапка чата
        TopAppBar(
            title = {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    modifier = Modifier.clickable { /* открыть профиль */ }
                ) {
                    Box(
                        modifier = Modifier
                            .size(42.dp)
                            .clip(CircleShape)
                            .background(chat.avatarColor),
                        contentAlignment = Alignment.Center
                    ) {
                        Text(
                            text = chat.name.split(" ").take(2).map { it.firstOrNull() ?: "" }.joinToString("").uppercase(),
                            color = Color.White,
                            fontSize = 16.sp,
                            fontWeight = FontWeight.Medium
                        )
                    }
                    Spacer(modifier = Modifier.width(12.dp))
                    Column {
                        Text(
                            text = chat.name,
                            color = Color.White,
                            fontSize = 17.sp,
                            fontWeight = FontWeight.SemiBold
                        )
                        Text(
                            text = if (chat.isOnline) "в сети" else "был(а) недавно",
                            color = if (chat.isOnline) Color(0xFF4CAF50) else Color(0xFF6C7883),
                            fontSize = 13.sp
                        )
                    }
                }
            },
            navigationIcon = {
                if (showBackButton) {
                    IconButton(onClick = onBackClick) {
                        Icon(
                            Icons.Filled.ArrowBack,
                            contentDescription = "Back",
                            tint = Color(0xFF8E99A4)
                        )
                    }
                }
            },
            actions = {
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.Videocam, contentDescription = "Video Call", tint = Color(0xFF8E99A4))
                }
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.Call, contentDescription = "Call", tint = Color(0xFF8E99A4))
                }
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.MoreVert, contentDescription = "More", tint = Color(0xFF8E99A4))
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(
                containerColor = Color(0xFF17212B)
            )
        )

        // Сообщения
        LazyColumn(
            modifier = Modifier
                .weight(1f)
                .fillMaxWidth()
                .padding(horizontal = 8.dp),
            reverseLayout = true
        ) {
            items(messages.reversed()) { message ->
                MessageBubble(message = message)
            }
        }
        
        // Emoji picker
        if (showEmojiPicker) {
            EmojiPicker(
                onEmojiSelected = { emoji ->
                    messageText += emoji
                },
                onDismiss = { showEmojiPicker = false }
            )
        }

        // Поле ввода
        Surface(
            color = Color(0xFF17212B),
            shadowElevation = 8.dp
        ) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(8.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.AttachFile, contentDescription = "Attach", tint = Color(0xFF8E99A4))
                }

                TextField(
                    value = messageText,
                    onValueChange = { messageText = it },
                    placeholder = { Text("Сообщение", color = Color(0xFF6C7883)) },
                    colors = TextFieldDefaults.colors(
                        focusedContainerColor = Color(0xFF242F3D),
                        unfocusedContainerColor = Color(0xFF242F3D),
                        focusedTextColor = Color.White,
                        unfocusedTextColor = Color.White,
                        cursorColor = Color(0xFF5B8DEF),
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
                        tint = if (showEmojiPicker) Color(0xFF5B8DEF) else Color(0xFF8E99A4)
                    )
                }

                IconButton(onClick = { }) {
                    Icon(
                        if (messageText.isNotEmpty()) Icons.Filled.Send else Icons.Filled.Mic,
                        contentDescription = "Send",
                        tint = Color(0xFF5B8DEF)
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
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .height(250.dp),
        color = Color(0xFF1E2933)
    ) {
        Column {
            // Категории эмодзи
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .background(Color(0xFF17212B))
                    .padding(8.dp),
                horizontalArrangement = Arrangement.SpaceEvenly
            ) {
                listOf("😀", "🐱", "🍔", "⚽", "🚗", "💡", "❤️", "🏳️").forEach { emoji ->
                    Text(
                        text = emoji,
                        fontSize = 20.sp,
                        modifier = Modifier
                            .clickable { }
                            .padding(8.dp)
                    )
                }
            }
            
            // Сетка эмодзи
            LazyVerticalGrid(
                columns = GridCells.Fixed(8),
                modifier = Modifier
                    .fillMaxSize()
                    .padding(4.dp)
            ) {
                items(emojiList) { emoji ->
                    Text(
                        text = emoji,
                        fontSize = 24.sp,
                        modifier = Modifier
                            .clickable { onEmojiSelected(emoji) }
                            .padding(8.dp)
                    )
                }
            }
        }
    }
}

@Composable
fun MessageBubble(message: Message) {
    Box(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 2.dp),
        contentAlignment = if (message.isFromMe) Alignment.CenterEnd else Alignment.CenterStart
    ) {
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
                    if (message.isFromMe) Color(0xFF2B5278) else Color(0xFF182533)
                )
                .padding(horizontal = 14.dp, vertical = 8.dp)
        ) {
            Column {
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
                    Text(
                        text = message.time,
                        color = Color(0xFF6C7883),
                        fontSize = 12.sp
                    )
                    if (message.isFromMe) {
                        Spacer(modifier = Modifier.width(4.dp))
                        Icon(
                            if (message.isRead) Icons.Filled.DoneAll else Icons.Filled.Done,
                            contentDescription = null,
                            tint = if (message.isRead) Color(0xFF5B8DEF) else Color(0xFF6C7883),
                            modifier = Modifier.size(16.dp)
                        )
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
            .background(Color(0xFF0E1621)),
        contentAlignment = Alignment.Center
    ) {
        Column(
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Box(
                modifier = Modifier
                    .size(120.dp)
                    .clip(CircleShape)
                    .background(Color(0xFF17212B)),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    Icons.Filled.Chat,
                    contentDescription = null,
                    tint = Color(0xFF5B8DEF),
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
                color = Color(0xFF6C7883),
                fontSize = 15.sp
            )
        }
    }
}
