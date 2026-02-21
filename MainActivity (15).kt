package com.example.oregabrowser

import androidx.compose.material3.TabRowDefaults.tabIndicatorOffset
import androidx.compose.foundation.lazy.grid.*
import android.Manifest
import android.annotation.SuppressLint
import android.app.DownloadManager
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.content.Intent
import android.graphics.Bitmap
import android.net.Uri
import android.os.Bundle
import android.os.Environment
import android.speech.RecognizerIntent
import android.webkit.*
import android.widget.LinearLayout
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.animation.*
import androidx.compose.foundation.*
import androidx.compose.foundation.gestures.detectHorizontalDragGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material.icons.outlined.*
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.blur
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalFocusManager
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.compose.ui.zIndex
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.*
import coil.compose.AsyncImage
import coil.request.ImageRequest

// ============== COLORS ==============

object OregaColors {
    val GradientStart = Color(0xFFE8F4FD)
    val GradientMiddle = Color(0xFFE5E0F7)
    val GradientEnd = Color(0xFFF5E6F0)
    val Primary = Color(0xFF6B4EFF)
    val PrimaryLight = Color(0xFF8B7AFF)
    val Secondary = Color(0xFF00D4AA)
    val Surface = Color(0xFFFFFFFF)
    val SurfaceVariant = Color(0xFFF8F9FA)
    val CardBackground = Color(0xFFFFFFFF)
    val TextPrimary = Color(0xFF1A1A2E)
    val TextSecondary = Color(0xFF6B7280)
    val Accent = Color(0xFFFF6B6B)
    val SearchBar = Color(0xFFFFFFFF)
    val YandexRed = Color(0xFFFF0000)

    // Dark theme
    val DarkBackground = Color(0xFF0F0F23)
    val DarkSurface = Color(0xFF1A1A2E)
    val DarkCard = Color(0xFF252542)
    val DarkGradientStart = Color(0xFF1A1A2E)
    val DarkGradientEnd = Color(0xFF2D1B4E)
}

// ============== DATA CLASSES ==============

data class Tab(
    val id: String = UUID.randomUUID().toString(),
    var url: String = "",
    var title: String = "Новая вкладка",
    var favicon: Bitmap? = null,
    var isIncognito: Boolean = false,
    var webView: WebView? = null,
    var isHomePage: Boolean = true
)

data class HistoryItem(
    val id: String = UUID.randomUUID().toString(),
    val url: String,
    val title: String,
    val timestamp: Long = System.currentTimeMillis(),
    val favicon: Bitmap? = null
)

data class Bookmark(
    val id: String = UUID.randomUUID().toString(),
    val url: String,
    val title: String,
    val folder: String = "Общие",
    val timestamp: Long = System.currentTimeMillis()
)

data class DownloadItem(
    val id: String = UUID.randomUUID().toString(),
    val fileName: String,
    val url: String,
    val progress: Int = 0,
    val status: DownloadStatus = DownloadStatus.DOWNLOADING
)

data class QuickLink(
    val title: String,
    val url: String,
    val icon: ImageVector,
    val color: Color,
    val iconUrl: String? = null
)

data class NewsItem(
    val id: String = UUID.randomUUID().toString(),
    val title: String,
    val source: String,
    val imageUrl: String?,
    val url: String,
    val category: String = "Для вас",
    val timestamp: Long = System.currentTimeMillis()
)

enum class DownloadStatus { DOWNLOADING, COMPLETED, FAILED, PAUSED }

// ============== BROWSER STATE ==============

class BrowserState {
    var tabs = mutableStateListOf<Tab>()
    var currentTabIndex by mutableStateOf(0)
    var history = mutableStateListOf<HistoryItem>()
    var bookmarks = mutableStateListOf<Bookmark>()
    var downloads = mutableStateListOf<DownloadItem>()
    var quickLinks = mutableStateListOf(
        QuickLink("YouTube", "https://youtube.com", Icons.Rounded.PlayCircle, Color(0xFFFF0000)),
        QuickLink("Google", "https://google.com", Icons.Rounded.Search, Color(0xFF4285F4)),
        QuickLink("VK", "https://vk.com", Icons.Rounded.People, Color(0xFF0077FF)),
        QuickLink("Telegram", "https://web.telegram.org", Icons.Rounded.Send, Color(0xFF0088CC)),
        QuickLink("GitHub", "https://github.com", Icons.Rounded.Code, Color(0xFF333333)),
        QuickLink("Reddit", "https://reddit.com", Icons.Rounded.Forum, Color(0xFFFF4500))
    )
    
    // News items
    var newsItems = mutableStateListOf(
        NewsItem(
            title = "Трехдневный траур объявили в округе Приамурья из-за страшной катастрофы",
            source = "lenta.ru",
            imageUrl = "https://images.unsplash.com/photo-1516738901171-8eb4fc13bd20?w=800",
            url = "https://lenta.ru",
            category = "Для вас"
        ),
        NewsItem(
            title = "Haqqin: ракеты «Фламинго» поразили завод",
            source = "haqqin.az",
            imageUrl = "https://images.unsplash.com/photo-1569982175971-d92b01cf8694?w=400",
            url = "https://haqqin.az",
            category = "Политика"
        ),
        NewsItem(
            title = "Новые технологии изменят мир в ближайшие 10 лет",
            source = "techcrunch.com",
            imageUrl = "https://images.unsplash.com/photo-1518770660439-4636190af475?w=800",
            url = "https://techcrunch.com",
            category = "Технологии"
        ),
        NewsItem(
            title = "Финал чемпионата мира: сенсационная победа",
            source = "sports.ru",
            imageUrl = "https://images.unsplash.com/photo-1461896836934- voices-of-justice-bfcc74?w=800",
            url = "https://sports.ru",
            category = "Спорт"
        ),
        NewsItem(
            title = "Учёные обнаружили новую планету в обитаемой зоне",
            source = "science.org",
            imageUrl = "https://images.unsplash.com/photo-1446776811953-b23d57bd21aa?w=800",
            url = "https://science.org",
            category = "Наука"
        )
    )

    // Settings
    var isDarkMode by mutableStateOf(false)
    var isDesktopMode by mutableStateOf(false)
    var isAdBlockEnabled by mutableStateOf(true)
    var isJavaScriptEnabled by mutableStateOf(true)
    var fontSize by mutableStateOf(100)
    var searchEngine by mutableStateOf("Яндекс")
    var isDataSaverEnabled by mutableStateOf(false)

    val currentTab: Tab?
        get() = tabs.getOrNull(currentTabIndex)

    init {
        tabs.add(Tab())
    }

    fun addTab(isIncognito: Boolean = false) {
        tabs.add(Tab(isIncognito = isIncognito))
        currentTabIndex = tabs.size - 1
    }

    fun closeTab(index: Int) {
        if (tabs.size > 1) {
            tabs[index].webView?.destroy()
            tabs.removeAt(index)
            if (currentTabIndex >= tabs.size) {
                currentTabIndex = tabs.size - 1
            }
        }
    }

    fun addToHistory(url: String, title: String, favicon: Bitmap? = null) {
        if (currentTab?.isIncognito == false && url.isNotEmpty()) {
            history.add(0, HistoryItem(url = url, title = title, favicon = favicon))
            if (history.size > 1000) history.removeLast()
        }
    }

    fun addBookmark(url: String, title: String) {
        if (!bookmarks.any { it.url == url }) {
            bookmarks.add(Bookmark(url = url, title = title))
        }
    }

    fun isBookmarked(url: String): Boolean = bookmarks.any { it.url == url }

    fun removeBookmark(url: String) {
        bookmarks.removeAll { it.url == url }
    }

    fun addQuickLink(title: String, url: String) {
        quickLinks.add(QuickLink(title, url, Icons.Rounded.Link, OregaColors.Primary))
    }
    
    fun getNewsByCategory(category: String): List<NewsItem> {
        return if (category == "Для вас") newsItems else newsItems.filter { it.category == category }
    }
}

// ============== MAIN ACTIVITY ==============

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            val browserState = remember { BrowserState() }
            OregaBrowserTheme(darkTheme = browserState.isDarkMode) {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    OregaBrowser(browserState)
                }
            }
        }
    }
}

// ============== THEME ==============

@Composable
fun OregaBrowserTheme(
    darkTheme: Boolean = false,
    content: @Composable () -> Unit
) {
    val colorScheme = if (darkTheme) {
        darkColorScheme(
            primary = OregaColors.PrimaryLight,
            secondary = OregaColors.Secondary,
            background = OregaColors.DarkBackground,
            surface = OregaColors.DarkSurface,
            surfaceVariant = OregaColors.DarkCard,
            onPrimary = Color.White,
            onSecondary = Color.White,
            onBackground = Color.White,
            onSurface = Color.White
        )
    } else {
        lightColorScheme(
            primary = OregaColors.Primary,
            secondary = OregaColors.Secondary,
            background = Color.White,
            surface = OregaColors.Surface,
            surfaceVariant = OregaColors.SurfaceVariant,
            onPrimary = Color.White,
            onSecondary = Color.White,
            onBackground = OregaColors.TextPrimary,
            onSurface = OregaColors.TextPrimary
        )
    }

    MaterialTheme(
        colorScheme = colorScheme,
        content = content
    )
}

// ============== MAIN BROWSER COMPOSABLE ==============

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun OregaBrowser(state: BrowserState) {
    val context = LocalContext.current
    val scope = rememberCoroutineScope()
    val focusManager = LocalFocusManager.current

    var url by remember { mutableStateOf("") }
    var searchQuery by remember { mutableStateOf("") }
    var isLoading by remember { mutableStateOf(false) }
    var loadProgress by remember { mutableStateOf(0) }
    var canGoBack by remember { mutableStateOf(false) }
    var canGoForward by remember { mutableStateOf(false) }
    var pageTitle by remember { mutableStateOf("") }
    var isSecure by remember { mutableStateOf(false) }
    var showWebView by remember { mutableStateOf(false) }

    // UI States
    var showMenu by remember { mutableStateOf(false) }
    var showTabs by remember { mutableStateOf(false) }
    var showHistory by remember { mutableStateOf(false) }
    var showBookmarks by remember { mutableStateOf(false) }
    var showDownloads by remember { mutableStateOf(false) }
    var showSettings by remember { mutableStateOf(false) }
    var showFindInPage by remember { mutableStateOf(false) }
    var findQuery by remember { mutableStateOf("") }
    var showAddQuickLink by remember { mutableStateOf(false) }
    var showQRScanner by remember { mutableStateOf(false) }

    // Voice search
    val voiceLauncher = rememberLauncherForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        val data = result.data?.getStringArrayListExtra(RecognizerIntent.EXTRA_RESULTS)
        data?.firstOrNull()?.let { query ->
            searchQuery = query
            val finalUrl = processUrl(query, state.searchEngine)
            state.currentTab?.webView?.loadUrl(finalUrl)
            state.currentTab?.isHomePage = false
            showWebView = true
        }
    }

    // Swipe gestures
    var swipeOffset by remember { mutableStateOf(0f) }

    // Check if showing home page
    val isHomePage = state.currentTab?.isHomePage ?: true

    Box(modifier = Modifier.fillMaxSize()) {
        Column(modifier = Modifier.fillMaxSize()) {

            // ===== CONTENT =====
            Box(
                modifier = Modifier
                    .weight(1f)
                    .pointerInput(Unit) {
                        detectHorizontalDragGestures(
                            onDragEnd = {
                                if (swipeOffset > 100 && canGoBack) {
                                    state.currentTab?.webView?.goBack()
                                } else if (swipeOffset < -100 && canGoForward) {
                                    state.currentTab?.webView?.goForward()
                                }
                                swipeOffset = 0f
                            },
                            onHorizontalDrag = { _, dragAmount ->
                                swipeOffset += dragAmount
                            }
                        )
                    }
            ) {
                if (isHomePage && !showWebView) {
                    // Home Page
                    HomePage(
                        state = state,
                        searchQuery = searchQuery,
                        onSearchQueryChange = { searchQuery = it },
                        onSearch = { query ->
                            val finalUrl = processUrl(query, state.searchEngine)
                            state.currentTab?.webView?.loadUrl(finalUrl)
                            state.currentTab?.isHomePage = false
                            showWebView = true
                        },
                        onVoiceSearch = {
                            val intent = Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH).apply {
                                putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL, RecognizerIntent.LANGUAGE_MODEL_FREE_FORM)
                                putExtra(RecognizerIntent.EXTRA_LANGUAGE, Locale.getDefault())
                                putExtra(RecognizerIntent.EXTRA_PROMPT, "Голосовой поиск")
                            }
                            voiceLauncher.launch(intent)
                        },
                        onQuickLinkClick = { link ->
                            state.currentTab?.webView?.loadUrl(link.url)
                            state.currentTab?.isHomePage = false
                            showWebView = true
                        },
                        onShowMenu = { showMenu = true },
                        onAddQuickLink = { showAddQuickLink = true },
                        onQRScan = { showQRScanner = true },
                        onNewsClick = { newsItem ->
                            state.currentTab?.webView?.loadUrl(newsItem.url)
                            state.currentTab?.isHomePage = false
                            showWebView = true
                        }
                    )
                } else {
                    // Web View with URL bar
                    Column {
                        // URL Bar for web view
                        WebViewTopBar(
                            state = state,
                            url = url,
                            isLoading = isLoading,
                            isSecure = isSecure,
                            onUrlChange = { url = it },
                            onNavigate = { newUrl ->
                                val finalUrl = processUrl(newUrl, state.searchEngine)
                                state.currentTab?.webView?.loadUrl(finalUrl)
                                focusManager.clearFocus()
                            },
                            onVoiceSearch = {
                                val intent = Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH).apply {
                                    putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL, RecognizerIntent.LANGUAGE_MODEL_FREE_FORM)
                                    putExtra(RecognizerIntent.EXTRA_LANGUAGE, Locale.getDefault())
                                    putExtra(RecognizerIntent.EXTRA_PROMPT, "Голосовой поиск")
                                }
                                voiceLauncher.launch(intent)
                            },
                            onShowTabs = { showTabs = true },
                            onShowMenu = { showMenu = true }
                        )

                        // Find in page bar
                        AnimatedVisibility(visible = showFindInPage) {
                            FindInPageBar(
                                query = findQuery,
                                onQueryChange = {
                                    findQuery = it
                                    state.currentTab?.webView?.findAllAsync(it)
                                },
                                onNext = { state.currentTab?.webView?.findNext(true) },
                                onPrevious = { state.currentTab?.webView?.findNext(false) },
                                onClose = {
                                    showFindInPage = false
                                    state.currentTab?.webView?.clearMatches()
                                }
                            )
                        }

                        // Web View
                        Box(modifier = Modifier.weight(1f)) {
                            state.currentTab?.let { tab ->
                                WebViewContainer(
                                    tab = tab,
                                    state = state,
                                    onUrlChange = { url = it },
                                    onTitleChange = { pageTitle = it },
                                    onLoadingChange = { isLoading = it },
                                    onProgressChange = { loadProgress = it },
                                    onNavigationChange = { back, forward ->
                                        canGoBack = back
                                        canGoForward = forward
                                    },
                                    onSecureChange = { isSecure = it },
                                    context = context
                                )
                            }

                            // Loading indicator
                            if (isLoading) {
                                LinearProgressIndicator(
                                    progress = { loadProgress / 100f },
                                    modifier = Modifier
                                        .fillMaxWidth()
                                        .height(3.dp)
                                        .zIndex(10f),
                                    color = OregaColors.Primary,
                                    trackColor = OregaColors.Primary.copy(alpha = 0.2f)
                                )
                            }
                        }
                    }
                }
            }

            // ===== BOTTOM BAR =====
            ModernBottomBar(
                canGoBack = canGoBack || showWebView,
                canGoForward = canGoForward,
                isBookmarked = state.isBookmarked(state.currentTab?.url ?: ""),
                tabCount = state.tabs.size,
                onBack = {
                    if (state.currentTab?.webView?.canGoBack() == true) {
                        state.currentTab?.webView?.goBack()
                    } else {
                        state.currentTab?.isHomePage = true
                        showWebView = false
                    }
                },
                onForward = { state.currentTab?.webView?.goForward() },
                onRefresh = {
                    if (showWebView) {
                        state.currentTab?.webView?.reload()
                    }
                },
                onHome = {
                    state.currentTab?.isHomePage = true
                    showWebView = false
                },
                onBookmark = {
                    val currentUrl = state.currentTab?.url ?: return@ModernBottomBar
                    if (state.isBookmarked(currentUrl)) {
                        state.removeBookmark(currentUrl)
                        Toast.makeText(context, "Закладка удалена", Toast.LENGTH_SHORT).show()
                    } else {
                        state.addBookmark(currentUrl, pageTitle)
                        Toast.makeText(context, "Добавлено в закладки", Toast.LENGTH_SHORT).show()
                    }
                },
                onTabs = { showTabs = true }
            )
        }

        // ===== DIALOGS & SHEETS =====

        // Main Menu
        if (showMenu) {
            MainMenuDialog(
                state = state,
                onDismiss = { showMenu = false },
                onNewTab = {
                    state.addTab()
                    showWebView = false
                    showMenu = false
                },
                onIncognitoTab = {
                    state.addTab(isIncognito = true)
                    showWebView = false
                    showMenu = false
                },
                onHistory = {
                    showHistory = true
                    showMenu = false
                },
                onBookmarks = {
                    showBookmarks = true
                    showMenu = false
                },
                onDownloads = {
                    showDownloads = true
                    showMenu = false
                },
                onFindInPage = {
                    showFindInPage = true
                    showMenu = false
                },
                onShare = {
                    val sendIntent = Intent().apply {
                        action = Intent.ACTION_SEND
                        putExtra(Intent.EXTRA_TEXT, state.currentTab?.url)
                        type = "text/plain"
                    }
                    context.startActivity(Intent.createChooser(sendIntent, "Поделиться"))
                    showMenu = false
                },
                onDesktopMode = {
                    state.isDesktopMode = !state.isDesktopMode
                    state.currentTab?.webView?.settings?.apply {
                        userAgentString = if (state.isDesktopMode) {
                            "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"
                        } else null
                        useWideViewPort = state.isDesktopMode
                    }
                    state.currentTab?.webView?.reload()
                },
                onSettings = {
                    showSettings = true
                    showMenu = false
                },
                onReload = {
                    state.currentTab?.webView?.reload()
                    showMenu = false
                },
                context = context
            )
        }

        // Tabs Sheet
        if (showTabs) {
            TabsSheet(
                state = state,
                onDismiss = { showTabs = false },
                onTabSelect = { index ->
                    state.currentTabIndex = index
                    showWebView = !state.currentTab!!.isHomePage
                    showTabs = false
                },
                onTabClose = { index ->
                    state.closeTab(index)
                },
                onNewTab = {
                    state.addTab()
                    showWebView = false
                    showTabs = false
                }
            )
        }

        // History Sheet
        if (showHistory) {
            HistorySheet(
                history = state.history,
                onDismiss = { showHistory = false },
                onItemClick = { item ->
                    state.currentTab?.webView?.loadUrl(item.url)
                    state.currentTab?.isHomePage = false
                    showWebView = true
                    showHistory = false
                },
                onClearHistory = {
                    state.history.clear()
                }
            )
        }

        // Bookmarks Sheet
        if (showBookmarks) {
            BookmarksSheet(
                bookmarks = state.bookmarks,
                onDismiss = { showBookmarks = false },
                onItemClick = { bookmark ->
                    state.currentTab?.webView?.loadUrl(bookmark.url)
                    state.currentTab?.isHomePage = false
                    showWebView = true
                    showBookmarks = false
                },
                onDelete = { bookmark ->
                    state.bookmarks.remove(bookmark)
                }
            )
        }

        // Downloads Sheet
        if (showDownloads) {
            DownloadsSheet(
                downloads = state.downloads,
                onDismiss = { showDownloads = false }
            )
        }

        // Settings Sheet
        if (showSettings) {
            SettingsSheet(
                state = state,
                onDismiss = { showSettings = false }
            )
        }
        
        // Add Quick Link Dialog
        if (showAddQuickLink) {
            AddQuickLinkDialog(
                onDismiss = { showAddQuickLink = false },
                onAdd = { title, urlStr ->
                    state.addQuickLink(title, urlStr)
                    showAddQuickLink = false
                }
            )
        }
        
        // QR Scanner Dialog
        if (showQRScanner) {
            QRScannerDialog(
                onDismiss = { showQRScanner = false },
                onScanResult = { result ->
                    showQRScanner = false
                    val finalUrl = processUrl(result, state.searchEngine)
                    state.currentTab?.webView?.loadUrl(finalUrl)
                    state.currentTab?.isHomePage = false
                    showWebView = true
                }
            )
        }
    }
}

// ============== HOME PAGE ==============

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HomePage(
    state: BrowserState,
    searchQuery: String,
    onSearchQueryChange: (String) -> Unit,
    onSearch: (String) -> Unit,
    onVoiceSearch: () -> Unit,
    onQuickLinkClick: (QuickLink) -> Unit,
    onShowMenu: () -> Unit,
    onAddQuickLink: () -> Unit,
    onQRScan: () -> Unit,
    onNewsClick: (NewsItem) -> Unit
) {
    val isDark = state.isDarkMode

    val gradientColors = if (isDark) {
        listOf(OregaColors.DarkGradientStart, OregaColors.DarkGradientEnd, OregaColors.DarkBackground)
    } else {
        listOf(
            Color(0xFFE8F4FD),
            Color(0xFFE5E0F7),
            Color(0xFFF0E8F5),
            Color.White
        )
    }

    var selectedCategory by remember { mutableStateOf("Для вас") }
    val categories = listOf("Для вас", "Политика", "Спорт", "Общество", "Технологии", "Наука")

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(
                brush = Brush.verticalGradient(
                    colors = gradientColors,
                    startY = 0f,
                    endY = 1200f
                )
            )
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .verticalScroll(rememberScrollState())
        ) {
            // Top Settings Button
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp),
                horizontalArrangement = Arrangement.End
            ) {
                IconButton(
                    onClick = onShowMenu,
                    modifier = Modifier
                        .size(40.dp)
                        .background(
                            color = if (isDark) OregaColors.DarkCard.copy(alpha = 0.5f)
                            else Color.White.copy(alpha = 0.7f),
                            shape = RoundedCornerShape(12.dp)
                        )
                ) {
                    Icon(
                        Icons.Rounded.Tune,
                        contentDescription = "Настройки",
                        tint = if (isDark) Color.White else OregaColors.TextPrimary
                    )
                }
            }

            Spacer(modifier = Modifier.height(20.dp))

            // Search Bar - Yandex style
            YandexSearchBar(
                query = searchQuery,
                onQueryChange = onSearchQueryChange,
                onSearch = onSearch,
                onQRScan = onQRScan,
                isDark = isDark,
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 20.dp)
            )

            Spacer(modifier = Modifier.height(40.dp))

            // Quick Links Section - Only add button like in screenshot
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 20.dp),
                horizontalArrangement = Arrangement.Start
            ) {
                AddQuickLinkButton(
                    isDark = isDark,
                    onClick = onAddQuickLink
                )
                
                Spacer(modifier = Modifier.width(12.dp))
                
                // Show quick links if any
                LazyRow(
                    horizontalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    items(state.quickLinks.take(5)) { link ->
                        QuickLinkCard(
                            link = link,
                            isDark = isDark,
                            onClick = { onQuickLinkClick(link) }
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(32.dp))

            // News Section
            Surface(
                modifier = Modifier.fillMaxWidth(),
                color = if (isDark) OregaColors.DarkSurface else Color.White,
                shape = RoundedCornerShape(topStart = 28.dp, topEnd = 28.dp)
            ) {
                Column(
                    modifier = Modifier.padding(top = 20.dp)
                ) {
                    // Categories with more options
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(horizontal = 8.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        ScrollableTabRow(
                            selectedTabIndex = categories.indexOf(selectedCategory),
                            containerColor = Color.Transparent,
                            contentColor = if (isDark) Color.White else OregaColors.TextPrimary,
                            edgePadding = 12.dp,
                            modifier = Modifier.weight(1f),
                            indicator = { tabPositions ->
                                if (categories.indexOf(selectedCategory) < tabPositions.size) {
                                    Box(
                                        Modifier
                                            .tabIndicatorOffset(tabPositions[categories.indexOf(selectedCategory)])
                                            .height(3.dp)
                                            .padding(horizontal = 16.dp)
                                            .background(
                                                OregaColors.TextPrimary,
                                                RoundedCornerShape(topStart = 3.dp, topEnd = 3.dp)
                                            )
                                    )
                                }
                            },
                            divider = {}
                        ) {
                            categories.forEach { category ->
                                Tab(
                                    selected = category == selectedCategory,
                                    onClick = { selectedCategory = category },
                                    text = {
                                        Text(
                                            category,
                                            fontWeight = if (category == selectedCategory)
                                                FontWeight.SemiBold else FontWeight.Normal,
                                            fontSize = 14.sp
                                        )
                                    }
                                )
                            }
                        }
                        
                        // More options button
                        IconButton(onClick = { /* Show more categories */ }) {
                            Icon(
                                Icons.Rounded.MoreVert,
                                contentDescription = "Ещё",
                                tint = OregaColors.TextSecondary
                            )
                        }
                    }

                    Spacer(modifier = Modifier.height(16.dp))

                    // News Header
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(horizontal = 20.dp),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Surface(
                                shape = CircleShape,
                                color = OregaColors.Primary.copy(alpha = 0.1f),
                                modifier = Modifier.size(32.dp)
                            ) {
                                Box(contentAlignment = Alignment.Center) {
                                    Icon(
                                        Icons.Rounded.Autorenew,
                                        contentDescription = null,
                                        tint = OregaColors.Primary,
                                        modifier = Modifier.size(20.dp)
                                    )
                                }
                            }
                            Spacer(modifier = Modifier.width(10.dp))
                            Text(
                                "Сводка новостей",
                                style = MaterialTheme.typography.titleMedium,
                                fontWeight = FontWeight.SemiBold
                            )
                        }

                        OutlinedButton(
                            onClick = { /* Show more news */ },
                            shape = RoundedCornerShape(20.dp),
                            border = BorderStroke(1.dp, OregaColors.Primary),
                            contentPadding = PaddingValues(horizontal = 16.dp, vertical = 4.dp)
                        ) {
                            Text(
                                "Ещё",
                                color = OregaColors.Primary,
                                fontWeight = FontWeight.Medium,
                                fontSize = 14.sp
                            )
                        }
                    }

                    Spacer(modifier = Modifier.height(16.dp))

                    // News Cards
                    val filteredNews = state.getNewsByCategory(selectedCategory)
                    
                    // Large News Card (horizontal scroll)
                    LazyRow(
                        contentPadding = PaddingValues(horizontal = 20.dp),
                        horizontalArrangement = Arrangement.spacedBy(12.dp)
                    ) {
                        items(filteredNews.take(3)) { news ->
                            NewsCardLarge(
                                newsItem = news,
                                isDark = isDark,
                                onClick = { onNewsClick(news) },
                                modifier = Modifier.width(340.dp)
                            )
                        }
                    }

                    Spacer(modifier = Modifier.height(16.dp))

                    // Small News Cards
                    Column(
                        modifier = Modifier.padding(horizontal = 20.dp),
                        verticalArrangement = Arrangement.spacedBy(12.dp)
                    ) {
                        filteredNews.drop(1).take(5).forEach { news ->
                            NewsCardSmall(
                                newsItem = news,
                                isDark = isDark,
                                onClick = { onNewsClick(news) }
                            )
                        }
                    }

                    Spacer(modifier = Modifier.height(100.dp))
                }
            }
        }
    }
}

// ============== YANDEX STYLE SEARCH BAR ==============

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun YandexSearchBar(
    query: String,
    onQueryChange: (String) -> Unit,
    onSearch: (String) -> Unit,
    onQRScan: () -> Unit,
    isDark: Boolean,
    modifier: Modifier = Modifier
) {
    Surface(
        modifier = modifier
            .height(56.dp)
            .shadow(
                elevation = 8.dp,
                shape = RoundedCornerShape(28.dp),
                ambientColor = Color.Black.copy(alpha = 0.1f),
                spotColor = Color.Black.copy(alpha = 0.1f)
            ),
        shape = RoundedCornerShape(28.dp),
        color = if (isDark) OregaColors.DarkCard else Color.White
    ) {
        Row(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 6.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            // Yandex Logo
            Surface(
                modifier = Modifier
                    .size(44.dp)
                    .padding(4.dp),
                shape = RoundedCornerShape(12.dp),
                color = OregaColors.YandexRed
            ) {
                Box(contentAlignment = Alignment.Center) {
                    Text(
                        "Я",
                        color = Color.White,
                        fontWeight = FontWeight.Bold,
                        fontSize = 22.sp
                    )
                }
            }

            // Search TextField
            TextField(
                value = query,
                onValueChange = onQueryChange,
                modifier = Modifier.weight(1f),
                colors = TextFieldDefaults.colors(
                    focusedContainerColor = Color.Transparent,
                    unfocusedContainerColor = Color.Transparent,
                    focusedIndicatorColor = Color.Transparent,
                    unfocusedIndicatorColor = Color.Transparent,
                    cursorColor = OregaColors.Primary
                ),
                textStyle = LocalTextStyle.current.copy(
                    fontSize = 16.sp,
                    color = if (isDark) Color.White else OregaColors.TextPrimary
                ),
                placeholder = {
                    Text(
                        "Искать или задать вопрос",
                        color = OregaColors.TextSecondary,
                        fontSize = 16.sp
                    )
                },
                keyboardOptions = KeyboardOptions(imeAction = ImeAction.Search),
                keyboardActions = KeyboardActions(onSearch = { onSearch(query) }),
                singleLine = true
            )

            // QR Code Button
            IconButton(
                onClick = onQRScan,
                modifier = Modifier.size(44.dp)
            ) {
                Icon(
                    Icons.Rounded.QrCodeScanner,
                    contentDescription = "Сканировать QR",
                    tint = OregaColors.TextSecondary,
                    modifier = Modifier.size(24.dp)
                )
            }
        }
    }
}

// ============== QUICK LINK CARDS ==============

@Composable
fun AddQuickLinkButton(
    isDark: Boolean,
    onClick: () -> Unit
) {
    Surface(
        modifier = Modifier
            .size(56.dp)
            .clickable { onClick() },
        shape = RoundedCornerShape(16.dp),
        color = if (isDark) OregaColors.DarkCard.copy(alpha = 0.5f)
        else Color.White,
        shadowElevation = 2.dp
    ) {
        Box(contentAlignment = Alignment.Center) {
            Icon(
                Icons.Rounded.Add,
                contentDescription = "Добавить",
                tint = if (isDark) Color.White.copy(alpha = 0.7f)
                else OregaColors.TextPrimary,
                modifier = Modifier.size(28.dp)
            )
        }
    }
}

@Composable
fun QuickLinkCard(
    link: QuickLink,
    isDark: Boolean,
    onClick: () -> Unit
) {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        modifier = Modifier.width(56.dp)
    ) {
        Surface(
            modifier = Modifier
                .size(56.dp)
                .clickable { onClick() },
            shape = RoundedCornerShape(16.dp),
            color = if (isDark) OregaColors.DarkCard else Color.White,
            shadowElevation = 2.dp
        ) {
            Box(contentAlignment = Alignment.Center) {
                Icon(
                    link.icon,
                    contentDescription = link.title,
                    tint = link.color,
                    modifier = Modifier.size(28.dp)
                )
            }
        }

        Spacer(modifier = Modifier.height(4.dp))

        Text(
            link.title,
            style = MaterialTheme.typography.labelSmall,
            color = if (isDark) Color.White.copy(alpha = 0.8f)
            else OregaColors.TextSecondary,
            maxLines = 1,
            overflow = TextOverflow.Ellipsis,
            textAlign = TextAlign.Center,
            fontSize = 10.sp
        )
    }
}

// ============== NEWS CARDS ==============

@Composable
fun NewsCardLarge(
    newsItem: NewsItem,
    isDark: Boolean,
    onClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    Card(
        modifier = modifier
            .height(220.dp)
            .clickable { onClick() },
        shape = RoundedCornerShape(20.dp),
        colors = CardDefaults.cardColors(
            containerColor = if (isDark) OregaColors.DarkCard else Color.Gray.copy(alpha = 0.1f)
        )
    ) {
        Box(modifier = Modifier.fillMaxSize()) {
            // Background Image
            if (newsItem.imageUrl != null) {
                AsyncImage(
                    model = ImageRequest.Builder(LocalContext.current)
                        .data(newsItem.imageUrl)
                        .crossfade(true)
                        .build(),
                    contentDescription = null,
                    contentScale = ContentScale.Crop,
                    modifier = Modifier.fillMaxSize()
                )
            }
            
            // Gradient overlay
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .background(
                        Brush.verticalGradient(
                            colors = listOf(
                                Color.Transparent,
                                Color.Black.copy(alpha = 0.3f),
                                Color.Black.copy(alpha = 0.8f)
                            ),
                            startY = 0f
                        )
                    )
            )

            // Content
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(16.dp),
                verticalArrangement = Arrangement.SpaceBetween
            ) {
                Spacer(modifier = Modifier.weight(1f))
                
                // Source badge
                Surface(
                    shape = RoundedCornerShape(6.dp),
                    color = Color.Black.copy(alpha = 0.6f)
                ) {
                    Text(
                        newsItem.source,
                        modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp),
                        color = Color.White,
                        fontSize = 11.sp,
                        fontWeight = FontWeight.Medium
                    )
                }
                
                Spacer(modifier = Modifier.height(8.dp))

                // Title
                Text(
                    newsItem.title,
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold,
                    color = Color.White,
                    maxLines = 3,
                    overflow = TextOverflow.Ellipsis,
                    lineHeight = 22.sp
                )
            }
        }
    }
}

@Composable
fun NewsCardSmall(
    newsItem: NewsItem,
    isDark: Boolean,
    onClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    Card(
        modifier = modifier
            .fillMaxWidth()
            .clickable { onClick() },
        shape = RoundedCornerShape(16.dp),
        colors = CardDefaults.cardColors(
            containerColor = if (isDark) OregaColors.DarkCard else Color.White
        ),
        elevation = CardDefaults.cardElevation(defaultElevation = 2.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    newsItem.source,
                    style = MaterialTheme.typography.labelSmall,
                    color = OregaColors.TextSecondary,
                    fontSize = 11.sp
                )
                Spacer(modifier = Modifier.height(4.dp))
                Text(
                    newsItem.title,
                    style = MaterialTheme.typography.bodyMedium,
                    fontWeight = FontWeight.Medium,
                    maxLines = 2,
                    overflow = TextOverflow.Ellipsis,
                    lineHeight = 20.sp
                )
            }

            Spacer(modifier = Modifier.width(12.dp))

            // Image
            if (newsItem.imageUrl != null) {
                Card(
                    modifier = Modifier.size(80.dp, 60.dp),
                    shape = RoundedCornerShape(12.dp)
                ) {
                    AsyncImage(
                        model = ImageRequest.Builder(LocalContext.current)
                            .data(newsItem.imageUrl)
                            .crossfade(true)
                            .build(),
                        contentDescription = null,
                        contentScale = ContentScale.Crop,
                        modifier = Modifier.fillMaxSize()
                    )
                }
            } else {
                Surface(
                    modifier = Modifier.size(80.dp, 60.dp),
                    shape = RoundedCornerShape(12.dp),
                    color = OregaColors.Primary.copy(alpha = 0.1f)
                ) {
                    Box(contentAlignment = Alignment.Center) {
                        Icon(
                            Icons.Rounded.Article,
                            contentDescription = null,
                            tint = OregaColors.Primary,
                            modifier = Modifier.size(24.dp)
                        )
                    }
                }
            }
        }
    }
}

// ============== ADD QUICK LINK DIALOG ==============

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AddQuickLinkDialog(
    onDismiss: () -> Unit,
    onAdd: (String, String) -> Unit
) {
    var title by remember { mutableStateOf("") }
    var url by remember { mutableStateOf("") }
    
    AlertDialog(
        onDismissRequest = onDismiss,
        title = {
            Text(
                "Добавить ссылку",
                fontWeight = FontWeight.Bold
            )
        },
        text = {
            Column {
                OutlinedTextField(
                    value = title,
                    onValueChange = { title = it },
                    label = { Text("Название") },
                    singleLine = true,
                    modifier = Modifier.fillMaxWidth(),
                    shape = RoundedCornerShape(12.dp)
                )
                Spacer(modifier = Modifier.height(12.dp))
                OutlinedTextField(
                    value = url,
                    onValueChange = { url = it },
                    label = { Text("URL") },
                    singleLine = true,
                    modifier = Modifier.fillMaxWidth(),
                    shape = RoundedCornerShape(12.dp),
                    placeholder = { Text("https://") }
                )
            }
        },
        confirmButton = {
            Button(
                onClick = { 
                    if (title.isNotBlank() && url.isNotBlank()) {
                        onAdd(title, url)
                    }
                },
                shape = RoundedCornerShape(12.dp),
                colors = ButtonDefaults.buttonColors(
                    containerColor = OregaColors.Primary
                )
            ) {
                Text("Добавить")
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text("Отмена")
            }
        },
        shape = RoundedCornerShape(20.dp)
    )
}

// ============== QR SCANNER DIALOG ==============

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun QRScannerDialog(
    onDismiss: () -> Unit,
    onScanResult: (String) -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Icon(
                    Icons.Rounded.QrCodeScanner,
                    contentDescription = null,
                    tint = OregaColors.Primary,
                    modifier = Modifier.size(28.dp)
                )
                Spacer(modifier = Modifier.width(12.dp))
                Text(
                    "Сканер QR-кода",
                    fontWeight = FontWeight.Bold
                )
            }
        },
        text = {
            Column(
                horizontalAlignment = Alignment.CenterHorizontally,
                modifier = Modifier.fillMaxWidth()
            ) {
                // Placeholder for QR scanner
                Surface(
                    modifier = Modifier
                        .size(200.dp)
                        .clip(RoundedCornerShape(16.dp)),
                    color = OregaColors.TextSecondary.copy(alpha = 0.1f)
                ) {
                    Box(contentAlignment = Alignment.Center) {
                        Column(horizontalAlignment = Alignment.CenterHorizontally) {
                            Icon(
                                Icons.Rounded.CameraAlt,
                                contentDescription = null,
                                tint = OregaColors.TextSecondary,
                                modifier = Modifier.size(48.dp)
                            )
                            Spacer(modifier = Modifier.height(8.dp))
                            Text(
                                "Наведите камеру на QR-код",
                                color = OregaColors.TextSecondary,
                                fontSize = 12.sp,
                                textAlign = TextAlign.Center
                            )
                        }
                    }
                }
                
                Spacer(modifier = Modifier.height(16.dp))
                
                Text(
                    "Для работы сканера требуется разрешение камеры",
                    color = OregaColors.TextSecondary,
                    fontSize = 12.sp,
                    textAlign = TextAlign.Center
                )
            }
        },
        confirmButton = {
            Button(
                onClick = onDismiss,
                shape = RoundedCornerShape(12.dp),
                colors = ButtonDefaults.buttonColors(
                    containerColor = OregaColors.Primary
                )
            ) {
                Text("Закрыть")
            }
        },
        shape = RoundedCornerShape(20.dp)
    )
}

// ============== WEB VIEW TOP BAR ==============

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun WebViewTopBar(
    state: BrowserState,
    url: String,
    isLoading: Boolean,
    isSecure: Boolean,
    onUrlChange: (String) -> Unit,
    onNavigate: (String) -> Unit,
    onVoiceSearch: () -> Unit,
    onShowTabs: () -> Unit,
    onShowMenu: () -> Unit
) {
    val isIncognito = state.currentTab?.isIncognito == true
    val isDark = state.isDarkMode

    Surface(
        modifier = Modifier.fillMaxWidth(),
        color = if (isIncognito) OregaColors.DarkSurface
        else if (isDark) OregaColors.DarkSurface
        else Color.White,
        shadowElevation = 4.dp
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 12.dp, vertical = 8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            // Incognito indicator
            if (isIncognito) {
                Icon(
                    imageVector = Icons.Rounded.VisibilityOff,
                    contentDescription = "Incognito",
                    tint = Color.Gray,
                    modifier = Modifier
                        .padding(end = 8.dp)
                        .size(20.dp)
                )
            }

            // URL Bar
            Surface(
                modifier = Modifier
                    .weight(1f)
                    .height(44.dp),
                shape = RoundedCornerShape(22.dp),
                color = if (isIncognito) OregaColors.DarkCard.copy(alpha = 0.5f)
                else if (isDark) OregaColors.DarkCard
                else OregaColors.SurfaceVariant
            ) {
                Row(
                    modifier = Modifier
                        .fillMaxSize()
                        .padding(horizontal = 14.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    // Security icon
                    Icon(
                        imageVector = if (isSecure) Icons.Rounded.Lock else Icons.Rounded.LockOpen,
                        contentDescription = if (isSecure) "Защищено" else "Не защищено",
                        tint = if (isSecure) OregaColors.Secondary else OregaColors.Accent,
                        modifier = Modifier.size(16.dp)
                    )

                    Spacer(modifier = Modifier.width(10.dp))

                    // URL TextField
                    TextField(
                        value = url,
                        onValueChange = onUrlChange,
                        modifier = Modifier.weight(1f),
                        colors = TextFieldDefaults.colors(
                            focusedContainerColor = Color.Transparent,
                            unfocusedContainerColor = Color.Transparent,
                            focusedIndicatorColor = Color.Transparent,
                            unfocusedIndicatorColor = Color.Transparent
                        ),
                        textStyle = LocalTextStyle.current.copy(
                            fontSize = 14.sp,
                            color = if (isIncognito || isDark) Color.White else OregaColors.TextPrimary
                        ),
                        placeholder = {
                            Text(
                                "Искать или ввести адрес",
                                color = OregaColors.TextSecondary,
                                fontSize = 14.sp
                            )
                        },
                        keyboardOptions = KeyboardOptions(imeAction = ImeAction.Go),
                        keyboardActions = KeyboardActions(onGo = { onNavigate(url) }),
                        singleLine = true
                    )

                    // Refresh/Stop button
                    if (isLoading) {
                        IconButton(
                            onClick = { state.currentTab?.webView?.stopLoading() },
                            modifier = Modifier.size(32.dp)
                        ) {
                            Icon(
                                Icons.Rounded.Close,
                                contentDescription = "Остановить",
                                tint = OregaColors.TextSecondary,
                                modifier = Modifier.size(18.dp)
                            )
                        }
                    }
                }
            }

            Spacer(modifier = Modifier.width(6.dp))

            // Tabs button
            BadgedBox(
                badge = {
                    Badge(
                        containerColor = OregaColors.Primary,
                        contentColor = Color.White
                    ) {
                        Text(
                            state.tabs.size.toString(),
                            fontSize = 10.sp
                        )
                    }
                }
            ) {
                IconButton(onClick = onShowTabs) {
                    Icon(
                        imageVector = Icons.Rounded.Layers,
                        contentDescription = "Вкладки",
                        tint = if (isIncognito || isDark) Color.White
                        else OregaColors.TextPrimary
                    )
                }
            }

            // Menu button
            IconButton(onClick = onShowMenu) {
                Icon(
                    imageVector = Icons.Rounded.MoreVert,
                    contentDescription = "Меню",
                    tint = if (isIncognito || isDark) Color.White
                    else OregaColors.TextPrimary
                )
            }
        }
    }
}

// ============== MODERN BOTTOM BAR ==============

@Composable
fun ModernBottomBar(
    canGoBack: Boolean,
    canGoForward: Boolean,
    isBookmarked: Boolean,
    tabCount: Int,
    onBack: () -> Unit,
    onForward: () -> Unit,
    onRefresh: () -> Unit,
    onHome: () -> Unit,
    onBookmark: () -> Unit,
    onTabs: () -> Unit
) {
    Surface(
        modifier = Modifier.fillMaxWidth(),
        color = MaterialTheme.colorScheme.surface,
        shadowElevation = 12.dp
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 8.dp, vertical = 8.dp),
            horizontalArrangement = Arrangement.SpaceEvenly,
            verticalAlignment = Alignment.CenterVertically
        ) {
            // Back button
            ModernNavButton(
                icon = Icons.Rounded.ArrowBack,
                description = "Назад",
                enabled = canGoBack,
                onClick = onBack
            )
            
            // Refresh button
            ModernNavButton(
                icon = Icons.Rounded.Refresh,
                description = "Обновить",
                enabled = true,
                onClick = onRefresh
            )
            
            // Search/Home button (center, highlighted)
            ModernNavButton(
                icon = Icons.Rounded.Search,
                description = "Поиск",
                enabled = true,
                isHighlighted = false,
                onClick = onHome
            )

            // Tabs button with badge
            Box {
                Surface(
                    modifier = Modifier.size(48.dp),
                    shape = RoundedCornerShape(12.dp),
                    color = Color.Transparent,
                    onClick = onTabs
                ) {
                    Box(contentAlignment = Alignment.Center) {
                        // Tab count in a box
                        Surface(
                            modifier = Modifier.size(24.dp),
                            shape = RoundedCornerShape(6.dp),
                            border = BorderStroke(2.dp, MaterialTheme.colorScheme.onSurface)
                        ) {
                            Box(contentAlignment = Alignment.Center) {
                                Text(
                                    tabCount.toString(),
                                    fontSize = 12.sp,
                                    fontWeight = FontWeight.Bold,
                                    color = MaterialTheme.colorScheme.onSurface
                                )
                            }
                        }
                    }
                }
            }

            // Profile button
            ModernNavButton(
                icon = Icons.Rounded.AccountCircle,
                description = "Профиль",
                enabled = true,
                onClick = onBookmark
            )
        }
    }
}

@Composable
fun ModernNavButton(
    icon: ImageVector,
    description: String,
    enabled: Boolean,
    isHighlighted: Boolean = false,
    onClick: () -> Unit
) {
    val backgroundColor = if (isHighlighted) OregaColors.Primary else Color.Transparent
    val iconColor = when {
        isHighlighted -> Color.White
        enabled -> MaterialTheme.colorScheme.onSurface
        else -> MaterialTheme.colorScheme.onSurface.copy(alpha = 0.38f)
    }

    Surface(
        modifier = Modifier.size(48.dp),
        shape = CircleShape,
        color = backgroundColor,
        onClick = onClick
    ) {
        Box(contentAlignment = Alignment.Center) {
            Icon(
                imageVector = icon,
                contentDescription = description,
                tint = iconColor,
                modifier = Modifier.size(24.dp)
            )
        }
    }
}

// ============== FIND IN PAGE ==============

@Composable
fun FindInPageBar(
    query: String,
    onQueryChange: (String) -> Unit,
    onNext: () -> Unit,
    onPrevious: () -> Unit,
    onClose: () -> Unit
) {
    Surface(
        modifier = Modifier.fillMaxWidth(),
        color = MaterialTheme.colorScheme.surface,
        shadowElevation = 4.dp
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp, vertical = 8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Surface(
                modifier = Modifier
                    .weight(1f)
                    .height(40.dp),
                shape = RoundedCornerShape(20.dp),
                color = MaterialTheme.colorScheme.surfaceVariant
            ) {
                TextField(
                    value = query,
                    onValueChange = onQueryChange,
                    modifier = Modifier.fillMaxSize(),
                    placeholder = { Text("Найти на странице", fontSize = 14.sp) },
                    colors = TextFieldDefaults.colors(
                        focusedContainerColor = Color.Transparent,
                        unfocusedContainerColor = Color.Transparent,
                        focusedIndicatorColor = Color.Transparent,
                        unfocusedIndicatorColor = Color.Transparent
                    ),
                    textStyle = LocalTextStyle.current.copy(fontSize = 14.sp),
                    singleLine = true
                )
            }

            IconButton(onClick = onPrevious) {
                Icon(Icons.Rounded.KeyboardArrowUp, "Предыдущий")
            }
            IconButton(onClick = onNext) {
                Icon(Icons.Rounded.KeyboardArrowDown, "Следующий")
            }
            IconButton(onClick = onClose) {
                Icon(Icons.Rounded.Close, "Закрыть")
            }
        }
    }
}

// ============== WEB VIEW CONTAINER ==============

@SuppressLint("SetJavaScriptEnabled")
@Composable
fun WebViewContainer(
    tab: Tab,
    state: BrowserState,
    onUrlChange: (String) -> Unit,
    onTitleChange: (String) -> Unit,
    onLoadingChange: (Boolean) -> Unit,
    onProgressChange: (Int) -> Unit,
    onNavigationChange: (Boolean, Boolean) -> Unit,
    onSecureChange: (Boolean) -> Unit,
    context: Context
) {
    val startUrl = if (tab.url.isEmpty()) "https://www.google.com" else tab.url

    AndroidView(
        factory = { ctx ->
            WebView(ctx).apply {
                layoutParams = LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.MATCH_PARENT,
                    LinearLayout.LayoutParams.MATCH_PARENT
                )

                settings.apply {
                    javaScriptEnabled = state.isJavaScriptEnabled
                    domStorageEnabled = true
                    loadWithOverviewMode = true
                    useWideViewPort = true
                    builtInZoomControls = true
                    displayZoomControls = false
                    setSupportZoom(true)
                    cacheMode = if (state.isDataSaverEnabled)
                        WebSettings.LOAD_CACHE_ELSE_NETWORK
                    else
                        WebSettings.LOAD_DEFAULT
                    mixedContentMode = WebSettings.MIXED_CONTENT_COMPATIBILITY_MODE
                    allowFileAccess = true
                    allowContentAccess = true
                    databaseEnabled = true
                    textZoom = state.fontSize

                    if (state.isDesktopMode) {
                        userAgentString = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36"
                    }
                }

                webViewClient = object : WebViewClient() {
                    override fun onPageStarted(view: WebView?, url: String?, favicon: Bitmap?) {
                        onLoadingChange(true)
                        url?.let {
                            onUrlChange(it)
                            onSecureChange(it.startsWith("https"))
                        }
                    }

                    override fun onPageFinished(view: WebView?, url: String?) {
                        onLoadingChange(false)
                        onNavigationChange(view?.canGoBack() ?: false, view?.canGoForward() ?: false)
                        url?.let {
                            tab.url = it
                            tab.isHomePage = false
                            onUrlChange(it)
                            state.addToHistory(it, view?.title ?: "")
                        }
                        view?.title?.let {
                            tab.title = it
                            onTitleChange(it)
                        }
                    }

                    override fun shouldOverrideUrlLoading(view: WebView?, request: WebResourceRequest?): Boolean {
                        val url = request?.url?.toString() ?: return false

                        if (url.startsWith("tel:") || url.startsWith("mailto:") || url.startsWith("intent:")) {
                            try {
                                context.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)))
                            } catch (e: Exception) {
                                e.printStackTrace()
                            }
                            return true
                        }

                        return false
                    }

                    override fun shouldInterceptRequest(view: WebView?, request: WebResourceRequest?): WebResourceResponse? {
                        if (state.isAdBlockEnabled) {
                            val url = request?.url?.toString() ?: ""
                            val adDomains = listOf(
                                "doubleclick.net", "googlesyndication.com", "adservice",
                                "ads.", "tracking.", "analytics."
                            )
                            if (adDomains.any { url.contains(it) }) {
                                return WebResourceResponse("text/plain", "utf-8", null)
                            }
                        }
                        return super.shouldInterceptRequest(view, request)
                    }
                }

                webChromeClient = object : WebChromeClient() {
                    override fun onProgressChanged(view: WebView?, newProgress: Int) {
                        onProgressChange(newProgress)
                    }

                    override fun onReceivedTitle(view: WebView?, title: String?) {
                        title?.let {
                            tab.title = it
                            onTitleChange(it)
                        }
                    }

                    override fun onReceivedIcon(view: WebView?, icon: Bitmap?) {
                        tab.favicon = icon
                    }
                }

                setDownloadListener { url, userAgent, contentDisposition, mimetype, contentLength ->
                    val request = DownloadManager.Request(Uri.parse(url)).apply {
                        setMimeType(mimetype)
                        addRequestHeader("User-Agent", userAgent)
                        setDescription("Загрузка файла...")
                        setTitle(URLUtil.guessFileName(url, contentDisposition, mimetype))
                        setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED)
                        setDestinationInExternalPublicDir(
                            Environment.DIRECTORY_DOWNLOADS,
                            URLUtil.guessFileName(url, contentDisposition, mimetype)
                        )
                    }

                    val dm = context.getSystemService(Context.DOWNLOAD_SERVICE) as DownloadManager
                    dm.enqueue(request)

                    state.downloads.add(
                        DownloadItem(
                            fileName = URLUtil.guessFileName(url, contentDisposition, mimetype),
                            url = url,
                            status = DownloadStatus.DOWNLOADING
                        )
                    )

                    Toast.makeText(context, "Загрузка начата", Toast.LENGTH_SHORT).show()
                }

                tab.webView = this
                loadUrl(startUrl)
            }
        },
        update = { webView ->
            webView.settings.javaScriptEnabled = state.isJavaScriptEnabled
            webView.settings.textZoom = state.fontSize
        },
        modifier = Modifier.fillMaxSize()
    )
}

// ============== DIALOGS & SHEETS ==============

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun MainMenuDialog(
    state: BrowserState,
    onDismiss: () -> Unit,
    onNewTab: () -> Unit,
    onIncognitoTab: () -> Unit,
    onHistory: () -> Unit,
    onBookmarks: () -> Unit,
    onDownloads: () -> Unit,
    onFindInPage: () -> Unit,
    onShare: () -> Unit,
    onDesktopMode: () -> Unit,
    onSettings: () -> Unit,
    onReload: () -> Unit,
    context: Context
) {
    ModalBottomSheet(
        onDismissRequest = onDismiss,
        containerColor = MaterialTheme.colorScheme.surface,
        shape = RoundedCornerShape(topStart = 24.dp, topEnd = 24.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(20.dp)
        ) {
            // Header
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    "Orega Browser",
                    style = MaterialTheme.typography.titleLarge,
                    fontWeight = FontWeight.Bold,
                    color = OregaColors.Primary
                )
                Text(
                    "v1.0",
                    style = MaterialTheme.typography.bodySmall,
                    color = OregaColors.TextSecondary
                )
            }

            Spacer(modifier = Modifier.height(20.dp))

            // Quick actions grid
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceEvenly
            ) {
                ModernQuickAction(Icons.Rounded.Add, "Новая вкладка", onNewTab)
                ModernQuickAction(Icons.Rounded.VisibilityOff, "Инкогнито", onIncognitoTab)
                ModernQuickAction(Icons.Rounded.Refresh, "Обновить", onReload)
                ModernQuickAction(Icons.Rounded.Share, "Поделиться", onShare)
            }

            Spacer(modifier = Modifier.height(20.dp))
            HorizontalDivider(color = OregaColors.TextSecondary.copy(alpha = 0.2f))
            Spacer(modifier = Modifier.height(12.dp))

            // Menu items
            ModernMenuItem(Icons.Rounded.History, "История", onHistory)
            ModernMenuItem(Icons.Rounded.Bookmarks, "Закладки", onBookmarks)
            ModernMenuItem(Icons.Rounded.Download, "Загрузки", onDownloads)
            ModernMenuItem(Icons.Rounded.Search, "Найти на странице", onFindInPage)

            // Desktop mode toggle
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(12.dp))
                    .clickable { onDesktopMode() }
                    .padding(vertical = 14.dp, horizontal = 4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Surface(
                    shape = RoundedCornerShape(10.dp),
                    color = OregaColors.Primary.copy(alpha = 0.1f),
                    modifier = Modifier.size(40.dp)
                ) {
                    Box(contentAlignment = Alignment.Center) {
                        Icon(
                            Icons.Rounded.DesktopWindows,
                            contentDescription = null,
                            tint = OregaColors.Primary
                        )
                    }
                }
                Spacer(modifier = Modifier.width(16.dp))
                Text("Версия для ПК", modifier = Modifier.weight(1f))
                Switch(
                    checked = state.isDesktopMode,
                    onCheckedChange = { onDesktopMode() },
                    colors = SwitchDefaults.colors(
                        checkedThumbColor = Color.White,
                        checkedTrackColor = OregaColors.Primary
                    )
                )
            }

            ModernMenuItem(Icons.Rounded.Settings, "Настройки", onSettings)

            Spacer(modifier = Modifier.height(24.dp))
        }
    }
}

@Composable
fun ModernQuickAction(icon: ImageVector, label: String, onClick: () -> Unit) {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        modifier = Modifier
            .clip(RoundedCornerShape(12.dp))
            .clickable { onClick() }
            .padding(12.dp)
    ) {
        Surface(
            shape = RoundedCornerShape(14.dp),
            color = OregaColors.Primary.copy(alpha = 0.1f),
            modifier = Modifier.size(52.dp)
        ) {
            Box(contentAlignment = Alignment.Center) {
                Icon(
                    icon,
                    contentDescription = label,
                    tint = OregaColors.Primary,
                    modifier = Modifier.size(24.dp)
                )
            }
        }
        Spacer(modifier = Modifier.height(8.dp))
        Text(
            label,
            fontSize = 11.sp,
            color = MaterialTheme.colorScheme.onSurface,
            textAlign = TextAlign.Center,
            lineHeight = 14.sp
        )
    }
}

@Composable
fun ModernMenuItem(icon: ImageVector, title: String, onClick: () -> Unit) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(12.dp))
            .clickable { onClick() }
            .padding(vertical = 14.dp, horizontal = 4.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Surface(
            shape = RoundedCornerShape(10.dp),
            color = OregaColors.Primary.copy(alpha = 0.1f),
            modifier = Modifier.size(40.dp)
        ) {
            Box(contentAlignment = Alignment.Center) {
                Icon(icon, contentDescription = title, tint = OregaColors.Primary)
            }
        }
        Spacer(modifier = Modifier.width(16.dp))
        Text(title, style = MaterialTheme.typography.bodyLarge)
    }
}

// ============== TABS SHEET ==============

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TabsSheet(
    state: BrowserState,
    onDismiss: () -> Unit,
    onTabSelect: (Int) -> Unit,
    onTabClose: (Int) -> Unit,
    onNewTab: () -> Unit
) {
    ModalBottomSheet(
        onDismissRequest = onDismiss,
        containerColor = MaterialTheme.colorScheme.surface,
        shape = RoundedCornerShape(topStart = 24.dp, topEnd = 24.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .fillMaxHeight(0.85f)
        ) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(20.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    "Вкладки (${state.tabs.size})",
                    style = MaterialTheme.typography.titleLarge,
                    fontWeight = FontWeight.Bold
                )
                FilledTonalButton(
                    onClick = onNewTab,
                    shape = RoundedCornerShape(12.dp),
                    colors = ButtonDefaults.filledTonalButtonColors(
                        containerColor = OregaColors.Primary.copy(alpha = 0.1f),
                        contentColor = OregaColors.Primary
                    )
                ) {
                    Icon(Icons.Rounded.Add, "Новая вкладка", modifier = Modifier.size(18.dp))
                    Spacer(modifier = Modifier.width(6.dp))
                    Text("Новая", fontWeight = FontWeight.Medium)
                }
            }

            LazyVerticalGrid(
                columns = GridCells.Fixed(2),
                contentPadding = PaddingValues(horizontal = 16.dp, vertical = 8.dp),
                horizontalArrangement = Arrangement.spacedBy(12.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                itemsIndexed(state.tabs) { index, tab ->
                    ModernTabCard(
                        tab = tab,
                        isSelected = index == state.currentTabIndex,
                        onClick = { onTabSelect(index) },
                        onClose = { onTabClose(index) }
                    )
                }
            }
        }
    }
}

@Composable
fun ModernTabCard(
    tab: Tab,
    isSelected: Boolean,
    onClick: () -> Unit,
    onClose: () -> Unit
) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .height(140.dp)
            .clickable { onClick() },
        shape = RoundedCornerShape(16.dp),
        border = if (isSelected) BorderStroke(2.dp, OregaColors.Primary) else null,
        colors = CardDefaults.cardColors(
            containerColor = if (tab.isIncognito) OregaColors.DarkCard
            else MaterialTheme.colorScheme.surfaceVariant
        ),
        elevation = CardDefaults.cardElevation(defaultElevation = if (isSelected) 4.dp else 1.dp)
    ) {
        Box(modifier = Modifier.fillMaxSize()) {
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(12.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    if (tab.isIncognito) {
                        Icon(
                            Icons.Rounded.VisibilityOff,
                            "Incognito",
                            tint = Color.Gray,
                            modifier = Modifier.size(16.dp)
                        )
                    } else {
                        Spacer(modifier = Modifier.size(16.dp))
                    }

                    Surface(
                        onClick = onClose,
                        shape = CircleShape,
                        color = Color.Black.copy(alpha = 0.1f),
                        modifier = Modifier.size(24.dp)
                    ) {
                        Box(contentAlignment = Alignment.Center) {
                            Icon(
                                Icons.Rounded.Close,
                                "Закрыть",
                                modifier = Modifier.size(14.dp),
                                tint = if (tab.isIncognito) Color.Gray
                                else MaterialTheme.colorScheme.onSurface.copy(alpha = 0.6f)
                            )
                        }
                    }
                }

                Spacer(modifier = Modifier.weight(1f))

                Text(
                    tab.title,
                    maxLines = 2,
                    overflow = TextOverflow.Ellipsis,
                    style = MaterialTheme.typography.bodyMedium,
                    fontWeight = FontWeight.Medium,
                    color = if (tab.isIncognito) Color.White
                    else MaterialTheme.colorScheme.onSurface
                )

                Spacer(modifier = Modifier.height(4.dp))

                Text(
                    if (tab.isHomePage) "Главная"
                    else tab.url.removePrefix("https://").removePrefix("www.").take(25),
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                    style = MaterialTheme.typography.bodySmall,
                    color = OregaColors.TextSecondary
                )
            }
        }
    }
}

// ============== HISTORY SHEET ==============

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HistorySheet(
    history: List<HistoryItem>,
    onDismiss: () -> Unit,
    onItemClick: (HistoryItem) -> Unit,
    onClearHistory: () -> Unit
) {
    val dateFormat = remember { SimpleDateFormat("dd MMM, HH:mm", Locale.getDefault()) }

    ModalBottomSheet(
        onDismissRequest = onDismiss,
        containerColor = MaterialTheme.colorScheme.surface,
        shape = RoundedCornerShape(topStart = 24.dp, topEnd = 24.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .fillMaxHeight(0.85f)
        ) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(20.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    "История",
                    style = MaterialTheme.typography.titleLarge,
                    fontWeight = FontWeight.Bold
                )
                TextButton(
                    onClick = onClearHistory,
                    colors = ButtonDefaults.textButtonColors(
                        contentColor = OregaColors.Accent
                    )
                ) {
                    Text("Очистить", fontWeight = FontWeight.Medium)
                }
            }

            if (history.isEmpty()) {
                EmptyState(
                    icon = Icons.Rounded.History,
                    message = "История пуста"
                )
            } else {
                LazyColumn {
                    items(history) { item ->
                        ListItem(
                            modifier = Modifier
                                .clickable { onItemClick(item) }
                                .padding(horizontal = 8.dp),
                            headlineContent = {
                                Text(
                                    item.title.ifEmpty { item.url },
                                    maxLines = 1,
                                    overflow = TextOverflow.Ellipsis,
                                    fontWeight = FontWeight.Medium
                                )
                            },
                            supportingContent = {
                                Text(
                                    item.url,
                                    maxLines = 1,
                                    overflow = TextOverflow.Ellipsis,
                                    color = OregaColors.TextSecondary,
                                    fontSize = 12.sp
                                )
                            },
                            trailingContent = {
                                Text(
                                    dateFormat.format(Date(item.timestamp)),
                                    fontSize = 11.sp,
                                    color = OregaColors.TextSecondary
                                )
                            },
                            leadingContent = {
                                Surface(
                                    shape = RoundedCornerShape(8.dp),
                                    color = OregaColors.Primary.copy(alpha = 0.1f),
                                    modifier = Modifier.size(40.dp)
                                ) {
                                    Box(contentAlignment = Alignment.Center) {
                                        Icon(
                                            Icons.Rounded.Public,
                                            "Website",
                                            tint = OregaColors.Primary
                                        )
                                    }
                                }
                            }
                        )
                    }
                }
            }
        }
    }
}

// ============== BOOKMARKS SHEET ==============

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun BookmarksSheet(
    bookmarks: List<Bookmark>,
    onDismiss: () -> Unit,
    onItemClick: (Bookmark) -> Unit,
    onDelete: (Bookmark) -> Unit
) {
    ModalBottomSheet(
        onDismissRequest = onDismiss,
        containerColor = MaterialTheme.colorScheme.surface,
        shape = RoundedCornerShape(topStart = 24.dp, topEnd = 24.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .fillMaxHeight(0.85f)
        ) {
            Text(
                "Закладки",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.padding(20.dp)
            )

            if (bookmarks.isEmpty()) {
                EmptyState(
                    icon = Icons.Rounded.BookmarkBorder,
                    message = "Нет закладок"
                )
            } else {
                LazyColumn {
                    items(bookmarks) { bookmark ->
                        ListItem(
                            modifier = Modifier
                                .clickable { onItemClick(bookmark) }
                                .padding(horizontal = 8.dp),
                            headlineContent = {
                                Text(
                                    bookmark.title,
                                    maxLines = 1,
                                    overflow = TextOverflow.Ellipsis,
                                    fontWeight = FontWeight.Medium
                                )
                            },
                            supportingContent = {
                                Text(
                                    bookmark.url,
                                    maxLines = 1,
                                    overflow = TextOverflow.Ellipsis,
                                    color = OregaColors.TextSecondary,
                                    fontSize = 12.sp
                                )
                            },
                            leadingContent = {
                                Surface(
                                    shape = RoundedCornerShape(8.dp),
                                    color = OregaColors.Secondary.copy(alpha = 0.1f),
                                    modifier = Modifier.size(40.dp)
                                ) {
                                    Box(contentAlignment = Alignment.Center) {
                                        Icon(
                                            Icons.Rounded.Bookmark,
                                            "Bookmark",
                                            tint = OregaColors.Secondary
                                        )
                                    }
                                }
                            },
                            trailingContent = {
                                IconButton(onClick = { onDelete(bookmark) }) {
                                    Icon(
                                        Icons.Rounded.Delete,
                                        "Удалить",
                                        tint = OregaColors.TextSecondary
                                    )
                                }
                            }
                        )
                    }
                }
            }
        }
    }
}

// ============== DOWNLOADS SHEET ==============

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DownloadsSheet(
    downloads: List<DownloadItem>,
    onDismiss: () -> Unit
) {
    ModalBottomSheet(
        onDismissRequest = onDismiss,
        containerColor = MaterialTheme.colorScheme.surface,
        shape = RoundedCornerShape(topStart = 24.dp, topEnd = 24.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .fillMaxHeight(0.65f)
        ) {
            Text(
                "Загрузки",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.padding(20.dp)
            )

            if (downloads.isEmpty()) {
                EmptyState(
                    icon = Icons.Rounded.Download,
                    message = "Нет загрузок"
                )
            } else {
                LazyColumn {
                    items(downloads) { download ->
                        ListItem(
                            headlineContent = {
                                Text(
                                    download.fileName,
                                    fontWeight = FontWeight.Medium
                                )
                            },
                            supportingContent = {
                                when (download.status) {
                                    DownloadStatus.DOWNLOADING -> {
                                        LinearProgressIndicator(
                                            progress = { download.progress / 100f },
                                            modifier = Modifier
                                                .fillMaxWidth()
                                                .padding(top = 8.dp)
                                                .height(4.dp)
                                                .clip(RoundedCornerShape(2.dp)),
                                            color = OregaColors.Primary,
                                            trackColor = OregaColors.Primary.copy(alpha = 0.2f)
                                        )
                                    }
                                    DownloadStatus.COMPLETED -> Text(
                                        "Завершено",
                                        color = OregaColors.Secondary,
                                        fontWeight = FontWeight.Medium
                                    )
                                    DownloadStatus.FAILED -> Text(
                                        "Ошибка",
                                        color = OregaColors.Accent,
                                        fontWeight = FontWeight.Medium
                                    )
                                    DownloadStatus.PAUSED -> Text(
                                        "Пауза",
                                        color = Color(0xFFFBBC04),
                                        fontWeight = FontWeight.Medium
                                    )
                                }
                            },
                            leadingContent = {
                                Surface(
                                    shape = RoundedCornerShape(8.dp),
                                    color = when (download.status) {
                                        DownloadStatus.COMPLETED -> OregaColors.Secondary.copy(alpha = 0.1f)
                                        DownloadStatus.FAILED -> OregaColors.Accent.copy(alpha = 0.1f)
                                        else -> OregaColors.Primary.copy(alpha = 0.1f)
                                    },
                                    modifier = Modifier.size(40.dp)
                                ) {
                                    Box(contentAlignment = Alignment.Center) {
                                        Icon(
                                            when (download.status) {
                                                DownloadStatus.COMPLETED -> Icons.Rounded.CheckCircle
                                                DownloadStatus.FAILED -> Icons.Rounded.Error
                                                else -> Icons.Rounded.Download
                                            },
                                            "Download status",
                                            tint = when (download.status) {
                                                DownloadStatus.COMPLETED -> OregaColors.Secondary
                                                DownloadStatus.FAILED -> OregaColors.Accent
                                                else -> OregaColors.Primary
                                            }
                                        )
                                    }
                                }
                            }
                        )
                    }
                }
            }
        }
    }
}

// ============== SETTINGS SHEET ==============

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsSheet(
    state: BrowserState,
    onDismiss: () -> Unit
) {
    ModalBottomSheet(
        onDismissRequest = onDismiss,
        containerColor = MaterialTheme.colorScheme.surface,
        shape = RoundedCornerShape(topStart = 24.dp, topEnd = 24.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .fillMaxHeight(0.9f)
                .verticalScroll(rememberScrollState())
                .padding(20.dp)
        ) {
            Text(
                "Настройки",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.padding(bottom = 20.dp)
            )

            // Appearance section
            SettingsSectionHeader("Внешний вид")

            ModernSettingsToggle(
                title = "Тёмная тема",
                subtitle = "Включить тёмный режим",
                icon = Icons.Rounded.DarkMode,
                checked = state.isDarkMode,
                onCheckedChange = { state.isDarkMode = it }
            )

            // Font size
            Column(modifier = Modifier.padding(vertical = 12.dp)) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween
                ) {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Surface(
                            shape = RoundedCornerShape(10.dp),
                            color = OregaColors.Primary.copy(alpha = 0.1f),
                            modifier = Modifier.size(40.dp)
                        ) {
                            Box(contentAlignment = Alignment.Center) {
                                Icon(
                                    Icons.Rounded.TextFields,
                                    "Font size",
                                    tint = OregaColors.Primary
                                )
                            }
                        }
                        Spacer(modifier = Modifier.width(16.dp))
                        Text("Размер шрифта")
                    }
                    Text(
                        "${state.fontSize}%",
                        color = OregaColors.Primary,
                        fontWeight = FontWeight.Medium
                    )
                }
                Slider(
                    value = state.fontSize.toFloat(),
                    onValueChange = { state.fontSize = it.toInt() },
                    valueRange = 50f..200f,
                    steps = 14,
                    modifier = Modifier.padding(top = 8.dp),
                    colors = SliderDefaults.colors(
                        thumbColor = OregaColors.Primary,
                        activeTrackColor = OregaColors.Primary
                    )
                )
            }

            HorizontalDivider(
                modifier = Modifier.padding(vertical = 12.dp),
                color = OregaColors.TextSecondary.copy(alpha = 0.2f)
            )

            // Privacy section
            SettingsSectionHeader("Конфиденциальность")

            ModernSettingsToggle(
                title = "Блокировка рекламы",
                subtitle = "Блокировать рекламу и трекеры",
                icon = Icons.Rounded.Block,
                checked = state.isAdBlockEnabled,
                onCheckedChange = { state.isAdBlockEnabled = it }
            )

            ModernSettingsToggle(
                title = "JavaScript",
                subtitle = "Разрешить выполнение JavaScript",
                icon = Icons.Rounded.Code,
                checked = state.isJavaScriptEnabled,
                onCheckedChange = { state.isJavaScriptEnabled = it }
            )

            HorizontalDivider(
                modifier = Modifier.padding(vertical = 12.dp),
                color = OregaColors.TextSecondary.copy(alpha = 0.2f)
            )

            // Data section
            SettingsSectionHeader("Данные")

            ModernSettingsToggle(
                title = "Экономия трафика",
                subtitle = "Использовать кэш для экономии данных",
                icon = Icons.Rounded.DataSaverOn,
                checked = state.isDataSaverEnabled,
                onCheckedChange = { state.isDataSaverEnabled = it }
            )

            // Search engine selector
            var expanded by remember { mutableStateOf(false) }

            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(RoundedCornerShape(12.dp))
                    .clickable { expanded = true }
                    .padding(vertical = 12.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Surface(
                        shape = RoundedCornerShape(10.dp),
                        color = OregaColors.Primary.copy(alpha = 0.1f),
                        modifier = Modifier.size(40.dp)
                    ) {
                        Box(contentAlignment = Alignment.Center) {
                            Icon(
                                Icons.Rounded.Search,
                                "Search engine",
                                tint = OregaColors.Primary
                            )
                        }
                    }
                    Spacer(modifier = Modifier.width(16.dp))
                    Column {
                        Text("Поисковая система")
                        Text(
                            state.searchEngine,
                            style = MaterialTheme.typography.bodySmall,
                            color = OregaColors.Primary
                        )
                    }
                }
                Icon(
                    Icons.Rounded.ArrowDropDown,
                    "Выбрать",
                    tint = OregaColors.TextSecondary
                )

                DropdownMenu(
                    expanded = expanded,
                    onDismissRequest = { expanded = false }
                ) {
                    listOf("Google", "Яндекс", "DuckDuckGo", "Bing").forEach { engine ->
                        DropdownMenuItem(
                            text = {
                                Text(
                                    engine,
                                    fontWeight = if (engine == state.searchEngine)
                                        FontWeight.Medium else FontWeight.Normal
                                )
                            },
                            onClick = {
                                state.searchEngine = engine
                                expanded = false
                            },
                            leadingIcon = {
                                if (engine == state.searchEngine) {
                                    Icon(
                                        Icons.Rounded.Check,
                                        null,
                                        tint = OregaColors.Primary
                                    )
                                }
                            }
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(32.dp))

            // Version info
            Column(
                modifier = Modifier.fillMaxWidth(),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    "O",
                    fontSize = 32.sp,
                    fontWeight = FontWeight.Bold,
                    color = OregaColors.Primary
                )
                Spacer(modifier = Modifier.height(4.dp))
                Text(
                    "Orega Browser",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Medium
                )
                Text(
                    "Версия 1.0.0",
                    style = MaterialTheme.typography.bodySmall,
                    color = OregaColors.TextSecondary
                )
            }

            Spacer(modifier = Modifier.height(32.dp))
        }
    }
}

@Composable
fun SettingsSectionHeader(title: String) {
    Text(
        title,
        style = MaterialTheme.typography.titleSmall,
        color = OregaColors.Primary,
        fontWeight = FontWeight.SemiBold,
        modifier = Modifier.padding(vertical = 8.dp)
    )
}

@Composable
fun ModernSettingsToggle(
    title: String,
    subtitle: String,
    icon: ImageVector,
    checked: Boolean,
    onCheckedChange: (Boolean) -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(12.dp))
            .clickable { onCheckedChange(!checked) }
            .padding(vertical = 10.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Row(
            modifier = Modifier.weight(1f),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Surface(
                shape = RoundedCornerShape(10.dp),
                color = OregaColors.Primary.copy(alpha = 0.1f),
                modifier = Modifier.size(40.dp)
            ) {
                Box(contentAlignment = Alignment.Center) {
                    Icon(icon, title, tint = OregaColors.Primary)
                }
            }
            Spacer(modifier = Modifier.width(16.dp))
            Column {
                Text(title)
                Text(
                    subtitle,
                    style = MaterialTheme.typography.bodySmall,
                    color = OregaColors.TextSecondary
                )
            }
        }
        Switch(
            checked = checked,
            onCheckedChange = onCheckedChange,
            colors = SwitchDefaults.colors(
                checkedThumbColor = Color.White,
                checkedTrackColor = OregaColors.Primary,
                uncheckedThumbColor = Color.White,
                uncheckedTrackColor = OregaColors.TextSecondary.copy(alpha = 0.3f)
            )
        )
    }
}

// ============== EMPTY STATE ==============

@Composable
fun EmptyState(
    icon: ImageVector,
    message: String
) {
    Box(
        modifier = Modifier
            .fillMaxWidth()
            .padding(48.dp),
        contentAlignment = Alignment.Center
    ) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Surface(
                shape = CircleShape,
                color = OregaColors.Primary.copy(alpha = 0.1f),
                modifier = Modifier.size(80.dp)
            ) {
                Box(contentAlignment = Alignment.Center) {
                    Icon(
                        icon,
                        message,
                        modifier = Modifier.size(40.dp),
                        tint = OregaColors.Primary.copy(alpha = 0.5f)
                    )
                }
            }
            Spacer(modifier = Modifier.height(16.dp))
            Text(
                message,
                color = OregaColors.TextSecondary,
                style = MaterialTheme.typography.bodyLarge
            )
        }
    }
}

// ============== UTILITY FUNCTIONS ==============

fun processUrl(input: String, searchEngine: String): String {
    val trimmed = input.trim()

    if (trimmed.contains(".") && !trimmed.contains(" ")) {
        return if (trimmed.startsWith("http://") || trimmed.startsWith("https://")) {
            trimmed
        } else {
            "https://$trimmed"
        }
    }

    return getSearchUrl(trimmed, searchEngine)
}

fun getSearchUrl(query: String, engine: String): String {
    val encoded = java.net.URLEncoder.encode(query, "UTF-8")
    return when (engine) {
        "Google" -> "https://www.google.com/search?q=$encoded"
        "Яндекс" -> "https://yandex.ru/search/?text=$encoded"
        "DuckDuckGo" -> "https://duckduckgo.com/?q=$encoded"
        "Bing" -> "https://www.bing.com/search?q=$encoded"
        else -> "https://www.google.com/search?q=$encoded"
    }
}
