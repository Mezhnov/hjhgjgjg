
package com.investor.appstore

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.animation.*
import androidx.compose.foundation.*
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.*
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
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.navigation.NavHostController
import androidx.navigation.compose.*
import coil.compose.AsyncImage

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            AppStoreTheme {
                MainApp()
            }
        }
    }
}

// ==================== THEME ====================
@Composable
fun AppStoreTheme(content: @Composable () -> Unit) {
    val colorScheme = darkColorScheme(
        primary = Color(0xFF00C853),
        secondary = Color(0xFF03DAC6),
        background = Color(0xFF0F0F0F),
        surface = Color(0xFF1A1A1A),
        surfaceVariant = Color(0xFF252525),
        onPrimary = Color.Black,
        onBackground = Color.White,
        onSurface = Color.White
    )
    
    MaterialTheme(
        colorScheme = colorScheme,
        content = content
    )
}

// ==================== DATA MODELS ====================
data class AppItem(
    val id: Int,
    val name: String,
    val developer: String,
    val rating: Float,
    val downloads: String,
    val iconUrl: String,
    val category: String,
    val size: String,
    val description: String,
    val screenshots: List<String>,
    val isEditorChoice: Boolean = false,
    val price: String = "Бесплатно"
)

// ==================== SAMPLE DATA ====================
object SampleData {
    val apps = listOf(
        AppItem(
            id = 1,
            name = "TikTok",
            developer = "TikTok Pte. Ltd.",
            rating = 4.5f,
            downloads = "1 млрд+",
            iconUrl = "https://upload.wikimedia.org/wikipedia/en/thumb/a/a9/TikTok_logo.svg/800px-TikTok_logo.svg.png",
            category = "Социальные",
            size = "287 МБ",
            description = "TikTok — это место для коротких видео на мобильных устройствах. Наша миссия — вдохновлять на творчество и приносить радость.",
            screenshots = listOf(
                "https://play-lh.googleusercontent.com/QWzqjC6Dc5aDJPnqKcl82nYEKl_fZ9EVMKnKd1O7yJTn5YVskPw0qT4fGg7QXFPH_Q=w526-h296",
                "https://play-lh.googleusercontent.com/jAHiKnlDQlPxkL4MN6dNnGn0A9XjKczUGpAoEVIhJJPT-TGfLQCNKu7mVEKa3Qxnfw=w526-h296"
            ),
            isEditorChoice = true
        ),
        AppItem(
            id = 2,
            name = "Telegram",
            developer = "Telegram FZ-LLC",
            rating = 4.6f,
            downloads = "1 млрд+",
            iconUrl = "https://upload.wikimedia.org/wikipedia/commons/thumb/8/82/Telegram_logo.svg/512px-Telegram_logo.svg.png",
            category = "Общение",
            size = "98 МБ",
            description = "Telegram — это облачное приложение для обмена сообщениями с возможностью синхронизации на всех устройствах.",
            screenshots = listOf(
                "https://play-lh.googleusercontent.com/5mP1QOYVhVQdz8A3BEa6I9n5NVbhv7VFb5H4u4pNzJOvCWGMbJH2xJj2vkPB1_2BrDM=w526-h296"
            ),
            isEditorChoice = true
        ),
        AppItem(
            id = 3,
            name = "Instagram",
            developer = "Meta Platforms, Inc.",
            rating = 4.4f,
            downloads = "5 млрд+",
            iconUrl = "https://upload.wikimedia.org/wikipedia/commons/thumb/e/e7/Instagram_logo_2016.svg/768px-Instagram_logo_2016.svg.png",
            category = "Социальные",
            size = "267 МБ",
            description = "Instagram позволяет делиться моментами жизни с друзьями и семьей по всему миру.",
            screenshots = listOf(
                "https://play-lh.googleusercontent.com/7YHuczOMXsO2nJqGkCpNMx_CHRfF_CxDaXYn3tMJQnPTCVU5LXKM8bC9lDGHaXVMwg=w526-h296"
            )
        ),
        AppItem(
            id = 4,
            name = "Spotify",
            developer = "Spotify AB",
            rating = 4.3f,
            downloads = "1 млрд+",
            iconUrl = "https://upload.wikimedia.org/wikipedia/commons/thumb/1/19/Spotify_logo_without_text.svg/768px-Spotify_logo_without_text.svg.png",
            category = "Музыка",
            size = "150 МБ",
            description = "Spotify — это цифровой музыкальный сервис, который дает вам доступ к миллионам песен.",
            screenshots = listOf(),
            price = "Бесплатно"
        ),
        AppItem(
            id = 5,
            name = "WhatsApp",
            developer = "Meta Platforms, Inc.",
            rating = 4.2f,
            downloads = "5 млрд+",
            iconUrl = "https://upload.wikimedia.org/wikipedia/commons/thumb/6/6b/WhatsApp.svg/767px-WhatsApp.svg.png",
            category = "Общение",
            size = "89 МБ",
            description = "Простой, надежный, бесплатный обмен сообщениями и видеозвонки.",
            screenshots = listOf()
        ),
        AppItem(
            id = 6,
            name = "YouTube",
            developer = "Google LLC",
            rating = 4.5f,
            downloads = "10 млрд+",
            iconUrl = "https://upload.wikimedia.org/wikipedia/commons/thumb/0/09/YouTube_full-color_icon_%282017%29.svg/800px-YouTube_full-color_icon_%282017%29.svg.png",
            category = "Видео",
            size = "145 МБ",
            description = "Смотрите любимые видео и слушайте музыку бесплатно.",
            screenshots = listOf(),
            isEditorChoice = true
        ),
        AppItem(
            id = 7,
            name = "Netflix",
            developer = "Netflix, Inc.",
            rating = 4.4f,
            downloads = "1 млрд+",
            iconUrl = "https://upload.wikimedia.org/wikipedia/commons/thumb/0/08/Netflix_2015_logo.svg/1920px-Netflix_2015_logo.svg.png",
            category = "Развлечения",
            size = "178 МБ",
            description = "Смотрите фильмы и сериалы Netflix на своем устройстве.",
            screenshots = listOf(),
            price = "Подписка"
        ),
        AppItem(
            id = 8,
            name = "Zoom",
            developer = "Zoom Video Communications",
            rating = 4.1f,
            downloads = "500 млн+",
            iconUrl = "https://upload.wikimedia.org/wikipedia/commons/thumb/1/11/Zoom_Logo_2022.svg/800px-Zoom_Logo_2022.svg.png",
            category = "Бизнес",
            size = "198 МБ",
            description = "Zoom — платформа для видеоконференций и онлайн-встреч.",
            screenshots = listOf()
        )
    )
    
    val categories = listOf(
        "Для вас", "Топ чартов", "Дети", "Категории", "Выбор редакции"
    )
    
    val gameApps = listOf(
        AppItem(
            id = 101,
            name = "Genshin Impact",
            developer = "miHoYo Limited",
            rating = 4.7f,
            downloads = "50 млн+",
            iconUrl = "https://upload.wikimedia.org/wikipedia/en/5/5d/Genshin_Impact_logo.png",
            category = "Игры",
            size = "3.5 ГБ",
            description = "Погрузитесь в фэнтезийный мир Тейват.",
            screenshots = listOf(),
            isEditorChoice = true
        ),
        AppItem(
            id = 102,
            name = "PUBG Mobile",
            developer = "Level Infinite",
            rating = 4.3f,
            downloads = "1 млрд+",
            iconUrl = "https://upload.wikimedia.org/wikipedia/en/9/91/PUBG_Mobile_Logo.png",
            category = "Игры",
            size = "2.1 ГБ",
            description = "Официальная мобильная версия PUBG.",
            screenshots = listOf()
        )
    )
}

// ==================== NAVIGATION ====================
sealed class Screen(val route: String) {
    object Home : Screen("home")
    object Games : Screen("games")
    object AppDetail : Screen("app_detail/{appId}") {
        fun createRoute(appId: Int) = "app_detail/$appId"
    }
    object Search : Screen("search")
    object Updates : Screen("updates")
    object Books : Screen("books")
}

@Composable
fun MainApp() {
    val navController = rememberNavController()
    var selectedTab by remember { mutableIntStateOf(0) }
    
    Scaffold(
        bottomBar = {
            NavigationBar(
                containerColor = MaterialTheme.colorScheme.surface,
                tonalElevation = 0.dp
            ) {
                val items = listOf(
                    Triple("Игры", Icons.Filled.SportsEsports, 0),
                    Triple("Приложения", Icons.Filled.Apps, 1),
                    Triple("Книги", Icons.Filled.Book, 2)
                )
                items.forEach { (title, icon, index) ->
                    NavigationBarItem(
                        icon = { Icon(icon, contentDescription = title) },
                        label = { Text(title, fontSize = 12.sp) },
                        selected = selectedTab == index,
                        onClick = { 
                            selectedTab = index
                            when(index) {
                                0 -> navController.navigate(Screen.Games.route)
                                1 -> navController.navigate(Screen.Home.route)
                                2 -> navController.navigate(Screen.Books.route)
                            }
                        },
                        colors = NavigationBarItemDefaults.colors(
                            selectedIconColor = MaterialTheme.colorScheme.primary,
                            selectedTextColor = MaterialTheme.colorScheme.primary,
                            indicatorColor = MaterialTheme.colorScheme.surfaceVariant
                        )
                    )
                }
            }
        }
    ) { paddingValues ->
        NavHost(
            navController = navController,
            startDestination = Screen.Home.route,
            modifier = Modifier.padding(paddingValues)
        ) {
            composable(Screen.Home.route) {
                selectedTab = 1
                HomeScreen(navController)
            }
            composable(Screen.Games.route) {
                selectedTab = 0
                GamesScreen(navController)
            }
            composable(Screen.Books.route) {
                selectedTab = 2
                BooksScreen(navController)
            }
            composable(Screen.AppDetail.route) { backStackEntry ->
                val appId = backStackEntry.arguments?.getString("appId")?.toIntOrNull() ?: 1
                AppDetailScreen(navController, appId)
            }
            composable(Screen.Search.route) {
                SearchScreen(navController)
            }
        }
    }
}

// ==================== HOME SCREEN ====================
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HomeScreen(navController: NavHostController) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background)
    ) {
        // Top Bar
        TopAppBar(
            title = {
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    modifier = Modifier.fillMaxWidth()
                ) {
                    AsyncImage(
                        model = "https://upload.wikimedia.org/wikipedia/commons/thumb/7/78/Google_Play_Store_badge_EN.svg/1200px-Google_Play_Store_badge_EN.svg.png",
                        contentDescription = "Logo",
                        modifier = Modifier
                            .height(30.dp)
                            .clip(RoundedCornerShape(4.dp))
                    )
                }
            },
            actions = {
                IconButton(onClick = { navController.navigate(Screen.Search.route) }) {
                    Icon(Icons.Filled.Search, "Поиск", tint = Color.White)
                }
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.AccountCircle, "Профиль", tint = Color.White)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(
                containerColor = MaterialTheme.colorScheme.background
            )
        )
        
        // Category Tabs
        ScrollableTabRow(
            selectedTabIndex = 0,
            edgePadding = 16.dp,
            containerColor = Color.Transparent,
            indicator = { },
            divider = { }
        ) {
            SampleData.categories.forEachIndexed { index, category ->
                Tab(
                    selected = index == 0,
                    onClick = { },
                    modifier = Modifier.padding(horizontal = 4.dp)
                ) {
                    Surface(
                        shape = RoundedCornerShape(20.dp),
                        color = if (index == 0) MaterialTheme.colorScheme.primary.copy(alpha = 0.2f)
                               else Color.Transparent,
                        border = if (index == 0) null 
                                else BorderStroke(1.dp, Color.Gray.copy(alpha = 0.5f))
                    ) {
                        Text(
                            text = category,
                            modifier = Modifier.padding(horizontal = 16.dp, vertical = 8.dp),
                            color = if (index == 0) MaterialTheme.colorScheme.primary else Color.Gray,
                            fontSize = 14.sp
                        )
                    }
                }
            }
        }
        
        LazyColumn(
            modifier = Modifier.fillMaxSize(),
            contentPadding = PaddingValues(bottom = 16.dp)
        ) {
            // Featured Banner
            item {
                FeaturedBanner(navController)
            }
            
            // Editor's Choice Section
            item {
                SectionHeader("Выбор редакции", "Смотреть все")
                LazyRow(
                    contentPadding = PaddingValues(horizontal = 16.dp),
                    horizontalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    items(SampleData.apps.filter { it.isEditorChoice }) { app ->
                        EditorChoiceCard(app, navController)
                    }
                }
            }
            
            // Recommended Section
            item {
                Spacer(modifier = Modifier.height(24.dp))
                SectionHeader("Рекомендуем", "Смотреть все")
            }
            
            items(SampleData.apps.take(5)) { app ->
                AppListItem(app, navController)
            }
            
            // Top Charts
            item {
                Spacer(modifier = Modifier.height(24.dp))
                SectionHeader("Топ бесплатных приложений", "Смотреть все")
            }
            
            items(SampleData.apps.drop(3).take(4)) { app ->
                AppListItem(app, navController, showRank = true)
            }
        }
    }
}

// ==================== GAMES SCREEN ====================
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun GamesScreen(navController: NavHostController) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background)
    ) {
        TopAppBar(
            title = { Text("Игры", color = Color.White) },
            actions = {
                IconButton(onClick = { navController.navigate(Screen.Search.route) }) {
                    Icon(Icons.Filled.Search, "Поиск", tint = Color.White)
                }
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.AccountCircle, "Профиль", tint = Color.White)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(
                containerColor = MaterialTheme.colorScheme.background
            )
        )
        
        LazyColumn(
            modifier = Modifier.fillMaxSize(),
            contentPadding = PaddingValues(bottom = 16.dp)
        ) {
            item {
                // Game Banner
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(16.dp)
                        .height(180.dp)
                        .clip(RoundedCornerShape(16.dp))
                        .background(
                            Brush.horizontalGradient(
                                colors = listOf(
                                    Color(0xFF1E3A5F),
                                    Color(0xFF0D1B2A)
                                )
                            )
                        )
                ) {
                    Column(
                        modifier = Modifier
                            .padding(20.dp)
                            .align(Alignment.CenterStart)
                    ) {
                        Text(
                            "Новые игры недели",
                            color = Color.White,
                            fontSize = 22.sp,
                            fontWeight = FontWeight.Bold
                        )
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(
                            "Откройте лучшие новинки",
                            color = Color.Gray,
                            fontSize = 14.sp
                        )
                        Spacer(modifier = Modifier.height(16.dp))
                        Button(
                            onClick = { },
                            colors = ButtonDefaults.buttonColors(
                                containerColor = MaterialTheme.colorScheme.primary
                            )
                        ) {
                            Text("Смотреть", color = Color.Black)
                        }
                    }
                }
            }
            
            item {
                SectionHeader("Популярные игры", "Смотреть все")
            }
            
            items(SampleData.gameApps) { app ->
                AppListItem(app, navController)
            }
            
            item {
                Spacer(modifier = Modifier.height(24.dp))
                SectionHeader("Казуальные игры", "Смотреть все")
                LazyRow(
                    contentPadding = PaddingValues(horizontal = 16.dp),
                    horizontalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    items(SampleData.gameApps) { app ->
                        GameCard(app, navController)
                    }
                }
            }
        }
    }
}

// ==================== BOOKS SCREEN ====================
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun BooksScreen(navController: NavHostController) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background)
    ) {
        TopAppBar(
            title = { Text("Книги", color = Color.White) },
            actions = {
                IconButton(onClick = { navController.navigate(Screen.Search.route) }) {
                    Icon(Icons.Filled.Search, "Поиск", tint = Color.White)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(
                containerColor = MaterialTheme.colorScheme.background
            )
        )
        
        Box(
            modifier = Modifier.fillMaxSize(),
            contentAlignment = Alignment.Center
        ) {
            Column(horizontalAlignment = Alignment.CenterHorizontally) {
                Icon(
                    Icons.Filled.Book,
                    contentDescription = null,
                    tint = Color.Gray,
                    modifier = Modifier.size(80.dp)
                )
                Spacer(modifier = Modifier.height(16.dp))
                Text(
                    "Раздел в разработке",
                    color = Color.Gray,
                    fontSize = 18.sp
                )
            }
        }
    }
}

// ==================== APP DETAIL SCREEN ====================
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AppDetailScreen(navController: NavHostController, appId: Int) {
    val app = (SampleData.apps + SampleData.gameApps).find { it.id == appId } ?: SampleData.apps[0]
    
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background)
            .verticalScroll(rememberScrollState())
    ) {
        // Top Bar
        TopAppBar(
            title = { },
            navigationIcon = {
                IconButton(onClick = { navController.popBackStack() }) {
                    Icon(Icons.Filled.ArrowBack, "Назад", tint = Color.White)
                }
            },
            actions = {
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.Search, "Поиск", tint = Color.White)
                }
                IconButton(onClick = { }) {
                    Icon(Icons.Filled.MoreVert, "Меню", tint = Color.White)
                }
            },
            colors = TopAppBarDefaults.topAppBarColors(
                containerColor = Color.Transparent
            )
        )
        
        // App Header
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp),
            verticalAlignment = Alignment.Top
        ) {
            AsyncImage(
                model = app.iconUrl,
                contentDescription = app.name,
                modifier = Modifier
                    .size(80.dp)
                    .clip(RoundedCornerShape(16.dp))
                    .background(Color.White.copy(alpha = 0.1f)),
                contentScale = ContentScale.Fit
            )
            
            Spacer(modifier = Modifier.width(16.dp))
            
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    app.name,
                    color = Color.White,
                    fontSize = 24.sp,
                    fontWeight = FontWeight.Bold
                )
                Text(
                    app.developer,
                    color = MaterialTheme.colorScheme.primary,
                    fontSize = 14.sp
                )
                
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    modifier = Modifier.padding(top = 4.dp)
                ) {
                    if (app.isEditorChoice) {
                        Surface(
                            shape = RoundedCornerShape(4.dp),
                            color = MaterialTheme.colorScheme.primary.copy(alpha = 0.2f)
                        ) {
                            Text(
                                "Выбор редакции",
                                modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp),
                                color = MaterialTheme.colorScheme.primary,
                                fontSize = 10.sp
                            )
                        }
                    }
                }
            }
        }
        
        // Stats Row
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalArrangement = Arrangement.SpaceEvenly
        ) {
            StatItem("${app.rating}", Icons.Filled.Star, "${(app.rating * 100).toInt()}K отзывов")
            VerticalDivider()
            StatItem(app.downloads, null, "Скачиваний")
            VerticalDivider()
            StatItem(app.size, null, "Размер")
            VerticalDivider()
            StatItem("12+", null, "Возраст")
        }
        
        // Install Button
        Button(
            onClick = { },
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp)
                .height(48.dp),
            shape = RoundedCornerShape(24.dp),
            colors = ButtonDefaults.buttonColors(
                containerColor = MaterialTheme.colorScheme.primary
            )
        ) {
            Text(
                "Установить",
                color = Color.Black,
                fontWeight = FontWeight.Bold,
                fontSize = 16.sp
            )
        }
        
        // Share & Wishlist buttons
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalArrangement = Arrangement.Center
        ) {
            OutlinedButton(
                onClick = { },
                modifier = Modifier.weight(1f),
                shape = RoundedCornerShape(24.dp),
                border = BorderStroke(1.dp, Color.Gray)
            ) {
                Icon(Icons.Outlined.Share, "Поделиться", tint = Color.White)
                Spacer(modifier = Modifier.width(8.dp))
                Text("Поделиться", color = Color.White)
            }
            
            Spacer(modifier = Modifier.width(12.dp))
            
            OutlinedButton(
                onClick = { },
                modifier = Modifier.weight(1f),
                shape = RoundedCornerShape(24.dp),
                border = BorderStroke(1.dp, Color.Gray)
            ) {
                Icon(Icons.Outlined.BookmarkBorder, "В список", tint = Color.White)
                Spacer(modifier = Modifier.width(8.dp))
                Text("В список", color = Color.White)
            }
        }
        
        // Screenshots
        if (app.screenshots.isNotEmpty()) {
            Text(
                "Скриншоты",
                color = Color.White,
                fontSize = 18.sp,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.padding(horizontal = 16.dp, vertical = 8.dp)
            )
            
            LazyRow(
                contentPadding = PaddingValues(horizontal = 16.dp),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                items(app.screenshots) { screenshot ->
                    AsyncImage(
                        model = screenshot,
                        contentDescription = "Screenshot",
                        modifier = Modifier
                            .height(200.dp)
                            .clip(RoundedCornerShape(12.dp)),
                        contentScale = ContentScale.FillHeight
                    )
                }
            }
        }
        
        // About Section
        Spacer(modifier = Modifier.height(24.dp))
        
        Text(
            "Об этом приложении",
            color = Color.White,
            fontSize = 18.sp,
            fontWeight = FontWeight.Bold,
            modifier = Modifier.padding(horizontal = 16.dp)
        )
        
        Text(
            app.description,
            color = Color.Gray,
            fontSize = 14.sp,
            modifier = Modifier.padding(16.dp)
        )
        
        // Info Cards
        Surface(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            shape = RoundedCornerShape(12.dp),
            color = MaterialTheme.colorScheme.surfaceVariant
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                InfoRow("Версия", "10.2.1")
                InfoRow("Обновлено", "15 февраля 2026")
                InfoRow("Совместимость", "Android 8.0+")
                InfoRow("Категория", app.category)
            }
        }
        
        // Reviews Section
        Spacer(modifier = Modifier.height(16.dp))
        
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                "Оценки и отзывы",
                color = Color.White,
                fontSize = 18.sp,
                fontWeight = FontWeight.Bold
            )
            Icon(
                Icons.Filled.ArrowForward,
                contentDescription = null,
                tint = Color.Gray
            )
        }
        
        // Rating Display
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(horizontalAlignment = Alignment.CenterHorizontally) {
                Text(
                    app.rating.toString(),
                    color = Color.White,
                    fontSize = 48.sp,
                    fontWeight = FontWeight.Bold
                )
                Row {
                    repeat(5) { index ->
                        Icon(
                            Icons.Filled.Star,
                            contentDescription = null,
                            tint = if (index < app.rating.toInt()) 
                                MaterialTheme.colorScheme.primary 
                            else Color.Gray,
                            modifier = Modifier.size(16.dp)
                        )
                    }
                }
                Text(
                    "1.2M отзывов",
                    color = Color.Gray,
                    fontSize = 12.sp
                )
            }
            
            Spacer(modifier = Modifier.width(32.dp))
            
            Column(modifier = Modifier.weight(1f)) {
                RatingBar(5, 0.7f)
                RatingBar(4, 0.15f)
                RatingBar(3, 0.08f)
                RatingBar(2, 0.04f)
                RatingBar(1, 0.03f)
            }
        }
        
        // Sample Review
        ReviewCard(
            userName = "Алексей",
            date = "10 февраля 2026",
            rating = 5,
            text = "Отличное приложение! Пользуюсь каждый день, очень удобный интерфейс."
        )
        
        Spacer(modifier = Modifier.height(100.dp))
    }
}

// ==================== SEARCH SCREEN ====================
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SearchScreen(navController: NavHostController) {
    var searchQuery by remember { mutableStateOf("") }
    
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            IconButton(onClick = { navController.popBackStack() }) {
                Icon(Icons.Filled.ArrowBack, "Назад", tint = Color.White)
            }
            
            TextField(
                value = searchQuery,
                onValueChange = { searchQuery = it },
                placeholder = { Text("Поиск приложений и игр", color = Color.Gray) },
                modifier = Modifier
                    .weight(1f)
                    .clip(RoundedCornerShape(24.dp)),
                colors = TextFieldDefaults.colors(
                    focusedContainerColor = MaterialTheme.colorScheme.surfaceVariant,
                    unfocusedContainerColor = MaterialTheme.colorScheme.surfaceVariant,
                    focusedIndicatorColor = Color.Transparent,
                    unfocusedIndicatorColor = Color.Transparent,
                    cursorColor = MaterialTheme.colorScheme.primary,
                    focusedTextColor = Color.White
                ),
                singleLine = true,
                trailingIcon = {
                    if (searchQuery.isNotEmpty()) {
                        IconButton(onClick = { searchQuery = "" }) {
                            Icon(Icons.Filled.Close, "Очистить", tint = Color.Gray)
                        }
                    }
                }
            )
        }
        
        if (searchQuery.isEmpty()) {
            // Recent Searches
            Text(
                "Недавние поиски",
                color = Color.White,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.padding(16.dp)
            )
            
            listOf("TikTok", "Instagram", "Игры").forEach { recent ->
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clickable { searchQuery = recent }
                        .padding(horizontal = 16.dp, vertical = 12.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Icon(Icons.Filled.History, "История", tint = Color.Gray)
                    Spacer(modifier = Modifier.width(16.dp))
                    Text(recent, color = Color.White)
                }
            }
            
            Divider(color = Color.Gray.copy(alpha = 0.3f), modifier = Modifier.padding(vertical = 8.dp))
            
            // Trending Searches
            Text(
                "Популярные запросы",
                color = Color.White,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.padding(16.dp)
            )
            
            listOf("Социальные сети", "Фоторедакторы", "Мессенджеры", "Игры").forEach { trending ->
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .clickable { searchQuery = trending }
                        .padding(horizontal = 16.dp, vertical = 12.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Icon(Icons.Filled.TrendingUp, "Тренд", tint = MaterialTheme.colorScheme.primary)
                    Spacer(modifier = Modifier.width(16.dp))
                    Text(trending, color = Color.White)
                }
            }
        } else {
            // Search Results
            val results = SampleData.apps.filter { 
                it.name.contains(searchQuery, ignoreCase = true) ||
                it.category.contains(searchQuery, ignoreCase = true)
            }
            
            LazyColumn {
                items(results) { app ->
                    AppListItem(app, navController)
                }
            }
        }
    }
}

// ==================== UI COMPONENTS ====================
@Composable
fun FeaturedBanner(navController: NavHostController) {
    LazyRow(
        contentPadding = PaddingValues(horizontal = 16.dp, vertical = 16.dp),
        horizontalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        items(SampleData.apps.take(3)) { app ->
            Box(
                modifier = Modifier
                    .width(320.dp)
                    .height(180.dp)
                    .clip(RoundedCornerShape(16.dp))
                    .background(
                        Brush.horizontalGradient(
                            colors = listOf(
                                Color(0xFF2D1B4E),
                                Color(0xFF1A1A2E)
                            )
                        )
                    )
                    .clickable { navController.navigate(Screen.AppDetail.createRoute(app.id)) }
            ) {
                Row(
                    modifier = Modifier
                        .fillMaxSize()
                        .padding(16.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Column(modifier = Modifier.weight(1f)) {
                        Surface(
                            shape = RoundedCornerShape(4.dp),
                            color = MaterialTheme.colorScheme.primary.copy(alpha = 0.2f)
                        ) {
                            Text(
                                "Рекомендуем",
                                modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp),
                                color = MaterialTheme.colorScheme.primary,
                                fontSize = 10.sp
                            )
                        }
                        Spacer(modifier = Modifier.height(12.dp))
                        Text(
                            app.name,
                            color = Color.White,
                            fontSize = 20.sp,
                            fontWeight = FontWeight.Bold
                        )
                        Text(
                            app.category,
                            color = Color.Gray,
                            fontSize = 12.sp
                        )
                        Spacer(modifier = Modifier.height(8.dp))
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Icon(
                                Icons.Filled.Star,
                                contentDescription = null,
                                tint = MaterialTheme.colorScheme.primary,
                                modifier = Modifier.size(14.dp)
                            )
                            Text(
                                " ${app.rating}",
                                color = Color.White,
                                fontSize = 12.sp
                            )
                        }
                    }
                    
                    AsyncImage(
                        model = app.iconUrl,
                        contentDescription = app.name,
                        modifier = Modifier
                            .size(100.dp)
                            .clip(RoundedCornerShape(20.dp))
                            .background(Color.White.copy(alpha = 0.1f)),
                        contentScale = ContentScale.Fit
                    )
                }
            }
        }
    }
}

@Composable
fun SectionHeader(title: String, actionText: String) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 16.dp, vertical = 8.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            title,
            color = Color.White,
            fontSize = 18.sp,
            fontWeight = FontWeight.Bold
        )
        Text(
            actionText,
            color = MaterialTheme.colorScheme.primary,
            fontSize = 14.sp
        )
    }
}

@Composable
fun EditorChoiceCard(app: AppItem, navController: NavHostController) {
    Card(
        modifier = Modifier
            .width(280.dp)
            .clickable { navController.navigate(Screen.AppDetail.createRoute(app.id)) },
        shape = RoundedCornerShape(12.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant
        )
    ) {
        Column {
            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .height(140.dp)
                    .background(
                        Brush.verticalGradient(
                            colors = listOf(
                                Color(0xFF1E3A5F),
                                Color(0xFF0D1B2A)
                            )
                        )
                    ),
                contentAlignment = Alignment.Center
            ) {
                AsyncImage(
                    model = app.iconUrl,
                    contentDescription = app.name,
                    modifier = Modifier
                        .size(80.dp)
                        .clip(RoundedCornerShape(16.dp)),
                    contentScale = ContentScale.Fit
                )
            }
            
            Column(modifier = Modifier.padding(12.dp)) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(
                        Icons.Filled.EmojiEvents,
                        contentDescription = null,
                        tint = MaterialTheme.colorScheme.primary,
                        modifier = Modifier.size(16.dp)
                    )
                    Spacer(modifier = Modifier.width(4.dp))
                    Text(
                        "Выбор редакции",
                        color = MaterialTheme.colorScheme.primary,
                        fontSize = 10.sp
                    )
                }
                Spacer(modifier = Modifier.height(4.dp))
                Text(
                    app.name,
                    color = Color.White,
                    fontSize = 14.sp,
                    fontWeight = FontWeight.Medium
                )
                Text(
                    app.category,
                    color = Color.Gray,
                    fontSize = 12.sp
                )
                Row(
                    verticalAlignment = Alignment.CenterVertically,
                    modifier = Modifier.padding(top = 4.dp)
                ) {
                    Text(
                        app.rating.toString(),
                        color = Color.White,
                        fontSize = 12.sp
                    )
                    Icon(
                        Icons.Filled.Star,
                        contentDescription = null,
                        tint = Color.White,
                        modifier = Modifier.size(12.dp)
                    )
                }
            }
        }
    }
}

@Composable
fun AppListItem(
    app: AppItem, 
    navController: NavHostController,
    showRank: Boolean = false
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { navController.navigate(Screen.AppDetail.createRoute(app.id)) }
            .padding(horizontal = 16.dp, vertical = 12.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        if (showRank) {
            Text(
                "${SampleData.apps.indexOf(app) + 1}",
                color = Color.Gray,
                fontSize = 14.sp,
                modifier = Modifier.width(24.dp)
            )
        }
        
        AsyncImage(
            model = app.iconUrl,
            contentDescription = app.name,
            modifier = Modifier
                .size(56.dp)
                .clip(RoundedCornerShape(12.dp))
                .background(Color.White.copy(alpha = 0.1f)),
            contentScale = ContentScale.Fit
        )
        
        Spacer(modifier = Modifier.width(16.dp))
        
        Column(modifier = Modifier.weight(1f)) {
            Text(
                app.name,
                color = Color.White,
                fontSize = 16.sp,
                fontWeight = FontWeight.Medium,
                maxLines = 1,
                overflow = TextOverflow.Ellipsis
            )
            Text(
                app.category,
                color = Color.Gray,
                fontSize = 12.sp
            )
            Row(verticalAlignment = Alignment.CenterVertically) {
                Text(
                    app.rating.toString(),
                    color = Color.Gray,
                    fontSize = 12.sp
                )
                Icon(
                    Icons.Filled.Star,
                    contentDescription = null,
                    tint = Color.Gray,
                    modifier = Modifier.size(12.dp)
                )
                Text(
                    " • ${app.size}",
                    color = Color.Gray,
                    fontSize = 12.sp
                )
            }
        }
    }
}

@Composable
fun GameCard(app: AppItem, navController: NavHostController) {
    Card(
        modifier = Modifier
            .width(160.dp)
            .clickable { navController.navigate(Screen.AppDetail.createRoute(app.id)) },
        shape = RoundedCornerShape(12.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant
        )
    ) {
        Column {
            AsyncImage(
                model = app.iconUrl,
                contentDescription = app.name,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(100.dp)
                    .clip(RoundedCornerShape(topStart = 12.dp, topEnd = 12.dp)),
                contentScale = ContentScale.Crop
            )
            
            Column(modifier = Modifier.padding(12.dp)) {
                Text(
                    app.name,
                    color = Color.White,
                    fontSize = 14.sp,
                    fontWeight = FontWeight.Medium,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text(
                        app.rating.toString(),
                        color = Color.Gray,
                        fontSize = 12.sp
                    )
                    Icon(
                        Icons.Filled.Star,
                        contentDescription = null,
                        tint = Color.Gray,
                        modifier = Modifier.size(12.dp)
                    )
                }
            }
        }
    }
}

@Composable
fun StatItem(value: String, icon: ImageVector?, label: String) {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        modifier = Modifier.padding(horizontal = 8.dp)
    ) {
        Row(verticalAlignment = Alignment.CenterVertically) {
            Text(
                value,
                color = Color.White,
                fontSize = 16.sp,
                fontWeight = FontWeight.Bold
            )
            if (icon != null) {
                Icon(
                    icon,
                    contentDescription = null,
                    tint = Color.White,
                    modifier = Modifier.size(16.dp)
                )
            }
        }
        Text(
            label,
            color = Color.Gray,
            fontSize = 11.sp,
            maxLines = 1
        )
    }
}

@Composable
fun VerticalDivider() {
    Box(
        modifier = Modifier
            .width(1.dp)
            .height(40.dp)
            .background(Color.Gray.copy(alpha = 0.3f))
    )
}

@Composable
fun InfoRow(label: String, value: String) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 8.dp),
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        Text(label, color = Color.Gray, fontSize = 14.sp)
        Text(value, color = Color.White, fontSize = 14.sp)
    }
}

@Composable
fun RatingBar(stars: Int, percentage: Float) {
    Row(
        verticalAlignment = Alignment.CenterVertically,
        modifier = Modifier.padding(vertical = 2.dp)
    ) {
        Text(
            stars.toString(),
            color = Color.Gray,
            fontSize = 12.sp,
            modifier = Modifier.width(16.dp)
        )
        Spacer(modifier = Modifier.width(8.dp))
        Box(
            modifier = Modifier
                .weight(1f)
                .height(8.dp)
                .clip(RoundedCornerShape(4.dp))
                .background(Color.Gray.copy(alpha = 0.3f))
        ) {
            Box(
                modifier = Modifier
                    .fillMaxHeight()
                    .fillMaxWidth(percentage)
                    .clip(RoundedCornerShape(4.dp))
                    .background(MaterialTheme.colorScheme.primary)
            )
        }
    }
}

@Composable
fun ReviewCard(userName: String, date: String, rating: Int, text: String) {
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp),
        shape = RoundedCornerShape(12.dp),
        color = MaterialTheme.colorScheme.surfaceVariant
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Box(
                    modifier = Modifier
                        .size(40.dp)
                        .clip(CircleShape)
                        .background(MaterialTheme.colorScheme.primary),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        userName.first().toString(),
                        color = Color.Black,
                        fontWeight = FontWeight.Bold
                    )
                }
                Spacer(modifier = Modifier.width(12.dp))
                Column {
                    Text(userName, color = Color.White, fontWeight = FontWeight.Medium)
                    Text(date, color = Color.Gray, fontSize = 12.sp)
                }
            }
            
            Row(modifier = Modifier.padding(vertical = 8.dp)) {
                repeat(5) { index ->
                    Icon(
                        Icons.Filled.Star,
                        contentDescription = null,
                        tint = if (index < rating) MaterialTheme.colorScheme.primary else Color.Gray,
                        modifier = Modifier.size(14.dp)
                    )
                }
            }
            
            Text(text, color = Color.White, fontSize = 14.sp)
            
            Row(
                modifier = Modifier.padding(top = 12.dp),
                horizontalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                Text("Полезно?", color = Color.Gray, fontSize = 12.sp)
                Text("Да", color = Color.Gray, fontSize = 12.sp)
                Text("Нет", color = Color.Gray, fontSize = 12.sp)
            }
        }
    }
}
