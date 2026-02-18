package com.example.probrowser

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
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalFocusManager
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.compose.ui.zIndex
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.*

// ============== DATA CLASSES ==============

data class Tab(
    val id: String = UUID.randomUUID().toString(),
    var url: String = "https://www.google.com",
    var title: String = "Новая вкладка",
    var favicon: Bitmap? = null,
    var isIncognito: Boolean = false,
    var webView: WebView? = null
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

enum class DownloadStatus { DOWNLOADING, COMPLETED, FAILED, PAUSED }

// ============== BROWSER STATE ==============

class BrowserState {
    var tabs = mutableStateListOf<Tab>()
    var currentTabIndex by mutableStateOf(0)
    var history = mutableStateListOf<HistoryItem>()
    var bookmarks = mutableStateListOf<Bookmark>()
    var downloads = mutableStateListOf<DownloadItem>()
    
    // Settings
    var isDarkMode by mutableStateOf(false)
    var isDesktopMode by mutableStateOf(false)
    var isAdBlockEnabled by mutableStateOf(true)
    var isJavaScriptEnabled by mutableStateOf(true)
    var fontSize by mutableStateOf(100) // percentage
    var searchEngine by mutableStateOf("Google")
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
        if (currentTab?.isIncognito == false) {
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
}

// ============== MAIN ACTIVITY ==============

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            val browserState = remember { BrowserState() }
            ProBrowserTheme(darkTheme = browserState.isDarkMode) {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    ProBrowser(browserState)
                }
            }
        }
    }
}

// ============== THEME ==============

@Composable
fun ProBrowserTheme(
    darkTheme: Boolean = false,
    content: @Composable () -> Unit
) {
    val colorScheme = if (darkTheme) {
        darkColorScheme(
            primary = Color(0xFF8AB4F8),
            secondary = Color(0xFF81C995),
            background = Color(0xFF202124),
            surface = Color(0xFF303134),
            onPrimary = Color.Black,
            onSecondary = Color.Black,
            onBackground = Color.White,
            onSurface = Color.White
        )
    } else {
        lightColorScheme(
            primary = Color(0xFF1A73E8),
            secondary = Color(0xFF34A853),
            background = Color.White,
            surface = Color(0xFFF1F3F4),
            onPrimary = Color.White,
            onSecondary = Color.White,
            onBackground = Color.Black,
            onSurface = Color.Black
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
fun ProBrowser(state: BrowserState) {
    val context = LocalContext.current
    val scope = rememberCoroutineScope()
    val focusManager = LocalFocusManager.current
    
    var url by remember { mutableStateOf("") }
    var isLoading by remember { mutableStateOf(false) }
    var loadProgress by remember { mutableStateOf(0) }
    var canGoBack by remember { mutableStateOf(false) }
    var canGoForward by remember { mutableStateOf(false) }
    var pageTitle by remember { mutableStateOf("") }
    var isSecure by remember { mutableStateOf(false) }
    
    // UI States
    var showMenu by remember { mutableStateOf(false) }
    var showTabs by remember { mutableStateOf(false) }
    var showHistory by remember { mutableStateOf(false) }
    var showBookmarks by remember { mutableStateOf(false) }
    var showDownloads by remember { mutableStateOf(false) }
    var showSettings by remember { mutableStateOf(false) }
    var showFindInPage by remember { mutableStateOf(false) }
    var findQuery by remember { mutableStateOf("") }
    var showShareSheet by remember { mutableStateOf(false) }
    
    // Voice search
    val voiceLauncher = rememberLauncherForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        val data = result.data?.getStringArrayListExtra(RecognizerIntent.EXTRA_RESULTS)
        data?.firstOrNull()?.let { query ->
            url = query
            state.currentTab?.webView?.loadUrl(getSearchUrl(query, state.searchEngine))
        }
    }
    
    // Swipe gestures
    var swipeOffset by remember { mutableStateOf(0f) }
    
    Box(modifier = Modifier.fillMaxSize()) {
        Column(modifier = Modifier.fillMaxSize()) {
            
            // ===== TOP BAR =====
            TopAppBar(state, url, isLoading, loadProgress, isSecure, canGoBack,
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
            
            // ===== FIND IN PAGE BAR =====
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
            
            // ===== WEB VIEW =====
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
                        progress = loadProgress / 100f,
                        modifier = Modifier
                            .fillMaxWidth()
                            .zIndex(10f),
                        color = MaterialTheme.colorScheme.primary
                    )
                }
            }
            
            // ===== BOTTOM BAR =====
            BottomNavigationBar(
                canGoBack = canGoBack,
                canGoForward = canGoForward,
                isBookmarked = state.isBookmarked(state.currentTab?.url ?: ""),
                onBack = { state.currentTab?.webView?.goBack() },
                onForward = { state.currentTab?.webView?.goForward() },
                onHome = { 
                    state.currentTab?.webView?.loadUrl("https://www.google.com")
                },
                onBookmark = {
                    val currentUrl = state.currentTab?.url ?: return@BottomNavigationBar
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
                    showMenu = false
                },
                onIncognitoTab = {
                    state.addTab(isIncognito = true)
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
                    showTabs = false
                },
                onTabClose = { index ->
                    state.closeTab(index)
                },
                onNewTab = {
                    state.addTab()
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
    }
}

// ============== TOP APP BAR ==============

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TopAppBar(
    state: BrowserState,
    url: String,
    isLoading: Boolean,
    progress: Int,
    isSecure: Boolean,
    canGoBack: Boolean,
    onUrlChange: (String) -> Unit,
    onNavigate: (String) -> Unit,
    onVoiceSearch: () -> Unit,
    onShowTabs: () -> Unit,
    onShowMenu: () -> Unit
) {
    val isIncognito = state.currentTab?.isIncognito == true
    
    Surface(
        modifier = Modifier.fillMaxWidth(),
        color = if (isIncognito) Color(0xFF303134) else MaterialTheme.colorScheme.surface,
        shadowElevation = 4.dp
    ) {
        Column {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(8.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                // Incognito indicator
                if (isIncognito) {
                    Icon(
                        imageVector = Icons.Default.VisibilityOff,
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
                    color = if (isIncognito) Color(0xFF202124) else MaterialTheme.colorScheme.surface
                ) {
                    Row(
                        modifier = Modifier
                            .fillMaxSize()
                            .padding(horizontal = 12.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        // Security icon
                        Icon(
                            imageVector = if (isSecure) Icons.Default.Lock else Icons.Default.LockOpen,
                            contentDescription = if (isSecure) "Secure" else "Not Secure",
                            tint = if (isSecure) Color(0xFF34A853) else Color(0xFFEA4335),
                            modifier = Modifier.size(16.dp)
                        )
                        
                        Spacer(modifier = Modifier.width(8.dp))
                        
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
                                color = if (isIncognito) Color.White else Color.Black
                            ),
                            placeholder = {
                                Text(
                                    "Поиск или введите URL",
                                    color = Color.Gray,
                                    fontSize = 14.sp
                                )
                            },
                            keyboardOptions = KeyboardOptions(imeAction = ImeAction.Go),
                            keyboardActions = KeyboardActions(onGo = { onNavigate(url) }),
                            singleLine = true
                        )
                        
                        // Voice search
                        IconButton(
                            onClick = onVoiceSearch,
                            modifier = Modifier.size(32.dp)
                        ) {
                            Icon(
                                imageVector = Icons.Default.Mic,
                                contentDescription = "Voice Search",
                                tint = Color.Gray,
                                modifier = Modifier.size(20.dp)
                            )
                        }
                    }
                }
                
                Spacer(modifier = Modifier.width(4.dp))
                
                // Tabs button
                BadgedBox(
                    badge = {
                        Badge(containerColor = MaterialTheme.colorScheme.primary) {
                            Text(
                                state.tabs.size.toString(),
                                fontSize = 10.sp
                            )
                        }
                    }
                ) {
                    IconButton(onClick = onShowTabs) {
                        Icon(
                            imageVector = Icons.Default.Tab,
                            contentDescription = "Tabs",
                            tint = if (isIncognito) Color.White else MaterialTheme.colorScheme.onSurface
                        )
                    }
                }
                
                // Menu button
                IconButton(onClick = onShowMenu) {
                    Icon(
                        imageVector = Icons.Default.MoreVert,
                        contentDescription = "Menu",
                        tint = if (isIncognito) Color.White else MaterialTheme.colorScheme.onSurface
                    )
                }
            }
        }
    }
}

// ============== BOTTOM NAVIGATION BAR ==============

@Composable
fun BottomNavigationBar(
    canGoBack: Boolean,
    canGoForward: Boolean,
    isBookmarked: Boolean,
    onBack: () -> Unit,
    onForward: () -> Unit,
    onHome: () -> Unit,
    onBookmark: () -> Unit,
    onTabs: () -> Unit
) {
    Surface(
        modifier = Modifier.fillMaxWidth(),
        color = MaterialTheme.colorScheme.surface,
        shadowElevation = 8.dp
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(vertical = 4.dp),
            horizontalArrangement = Arrangement.SpaceEvenly,
            verticalAlignment = Alignment.CenterVertically
        ) {
            NavButton(Icons.Default.ArrowBack, "Back", canGoBack, onBack)
            NavButton(Icons.Default.ArrowForward, "Forward", canGoForward, onForward)
            NavButton(Icons.Default.Home, "Home", true, onHome)
            NavButton(
                if (isBookmarked) Icons.Default.Bookmark else Icons.Default.BookmarkBorder,
                "Bookmark",
                true,
                onBookmark
            )
            NavButton(Icons.Default.Layers, "Tabs", true, onTabs)
        }
    }
}

@Composable
fun NavButton(
    icon: ImageVector,
    description: String,
    enabled: Boolean,
    onClick: () -> Unit
) {
    IconButton(onClick = onClick, enabled = enabled) {
        Icon(
            imageVector = icon,
            contentDescription = description,
            tint = if (enabled) MaterialTheme.colorScheme.onSurface else Color.LightGray,
            modifier = Modifier.size(24.dp)
        )
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
                .padding(8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            TextField(
                value = query,
                onValueChange = onQueryChange,
                modifier = Modifier.weight(1f),
                placeholder = { Text("Найти на странице") },
                colors = TextFieldDefaults.colors(
                    focusedContainerColor = Color.Transparent,
                    unfocusedContainerColor = Color.Transparent
                ),
                singleLine = true
            )
            IconButton(onClick = onPrevious) {
                Icon(Icons.Default.KeyboardArrowUp, "Previous")
            }
            IconButton(onClick = onNext) {
                Icon(Icons.Default.KeyboardArrowDown, "Next")
            }
            IconButton(onClick = onClose) {
                Icon(Icons.Default.Close, "Close")
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
                    
                    // Desktop mode
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
                        
                        // Handle special URLs
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
                    
                    // Simple ad blocking
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
                
                // Download listener
                setDownloadListener { url, userAgent, contentDisposition, mimetype, contentLength ->
                    val request = DownloadManager.Request(Uri.parse(url)).apply {
                        setMimeType(mimetype)
                        addRequestHeader("User-Agent", userAgent)
                        setDescription("Downloading file...")
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
                loadUrl(tab.url)
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
        containerColor = MaterialTheme.colorScheme.surface
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp)
        ) {
            Text(
                "Меню",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.padding(bottom = 16.dp)
            )
            
            // Quick actions row
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceEvenly
            ) {
                QuickActionButton(Icons.Default.Add, "Новая\nвкладка", onNewTab)
                QuickActionButton(Icons.Default.VisibilityOff, "Инкогнито", onIncognitoTab)
                QuickActionButton(Icons.Default.Refresh, "Обновить", onReload)
                QuickActionButton(Icons.Default.Share, "Поделиться", onShare)
            }
            
            Spacer(modifier = Modifier.height(16.dp))
            HorizontalDivider()
            Spacer(modifier = Modifier.height(8.dp))
            
            // Menu items
            MenuItem(Icons.Default.History, "История", onHistory)
            MenuItem(Icons.Default.Bookmarks, "Закладки", onBookmarks)
            MenuItem(Icons.Default.Download, "Загрузки", onDownloads)
            MenuItem(Icons.Default.Search, "Найти на странице", onFindInPage)
            
            // Desktop mode toggle
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { onDesktopMode() }
                    .padding(vertical = 12.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Icon(
                    Icons.Default.DesktopWindows,
                    contentDescription = "Desktop Mode",
                    modifier = Modifier.padding(end = 16.dp)
                )
                Text("Версия для ПК", modifier = Modifier.weight(1f))
                Switch(
                    checked = state.isDesktopMode,
                    onCheckedChange = { onDesktopMode() }
                )
            }
            
            MenuItem(Icons.Default.Settings, "Настройки", onSettings)
            
            Spacer(modifier = Modifier.height(32.dp))
        }
    }
}

@Composable
fun QuickActionButton(icon: ImageVector, label: String, onClick: () -> Unit) {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        modifier = Modifier
            .clip(RoundedCornerShape(8.dp))
            .clickable { onClick() }
            .padding(12.dp)
    ) {
        Surface(
            shape = CircleShape,
            color = MaterialTheme.colorScheme.primary.copy(alpha = 0.1f),
            modifier = Modifier.size(48.dp)
        ) {
            Box(contentAlignment = Alignment.Center) {
                Icon(
                    icon,
                    contentDescription = label,
                    tint = MaterialTheme.colorScheme.primary
                )
            }
        }
        Spacer(modifier = Modifier.height(4.dp))
        Text(
            label.replace("\n", "\n"),
            fontSize = 11.sp,
            color = MaterialTheme.colorScheme.onSurface
        )
    }
}

@Composable
fun MenuItem(icon: ImageVector, title: String, onClick: () -> Unit) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { onClick() }
            .padding(vertical = 12.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Icon(icon, contentDescription = title, modifier = Modifier.padding(end = 16.dp))
        Text(title)
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
        containerColor = MaterialTheme.colorScheme.surface
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .fillMaxHeight(0.8f)
        ) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    "Вкладки (${state.tabs.size})",
                    style = MaterialTheme.typography.titleLarge,
                    fontWeight = FontWeight.Bold
                )
                FilledTonalButton(onClick = onNewTab) {
                    Icon(Icons.Default.Add, "New Tab")
                    Spacer(modifier = Modifier.width(4.dp))
                    Text("Новая")
                }
            }
            
            LazyVerticalGrid(
                columns = GridCells.Fixed(2),
                contentPadding = PaddingValues(16.dp),
                horizontalArrangement = Arrangement.spacedBy(12.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                itemsIndexed(state.tabs) { index, tab ->
                    TabCard(
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
fun TabCard(
    tab: Tab,
    isSelected: Boolean,
    onClick: () -> Unit,
    onClose: () -> Unit
) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .height(160.dp)
            .clickable { onClick() },
        shape = RoundedCornerShape(12.dp),
        border = if (isSelected) BorderStroke(2.dp, MaterialTheme.colorScheme.primary) else null,
        colors = CardDefaults.cardColors(
            containerColor = if (tab.isIncognito) Color(0xFF303134) else MaterialTheme.colorScheme.surface
        )
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
                            Icons.Default.VisibilityOff,
                            "Incognito",
                            tint = Color.Gray,
                            modifier = Modifier.size(16.dp)
                        )
                    }
                    Spacer(modifier = Modifier.weight(1f))
                    IconButton(
                        onClick = onClose,
                        modifier = Modifier.size(24.dp)
                    ) {
                        Icon(
                            Icons.Default.Close,
                            "Close",
                            modifier = Modifier.size(16.dp),
                            tint = if (tab.isIncognito) Color.Gray else MaterialTheme.colorScheme.onSurface
                        )
                    }
                }
                
                Spacer(modifier = Modifier.weight(1f))
                
                Text(
                    tab.title,
                    maxLines = 2,
                    overflow = TextOverflow.Ellipsis,
                    style = MaterialTheme.typography.bodyMedium,
                    fontWeight = FontWeight.Medium,
                    color = if (tab.isIncognito) Color.White else MaterialTheme.colorScheme.onSurface
                )
                
                Text(
                    tab.url.removePrefix("https://").removePrefix("www.").take(30),
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
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
        containerColor = MaterialTheme.colorScheme.surface
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .fillMaxHeight(0.8f)
        ) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    "История",
                    style = MaterialTheme.typography.titleLarge,
                    fontWeight = FontWeight.Bold
                )
                TextButton(onClick = onClearHistory) {
                    Text("Очистить")
                }
            }
            
            if (history.isEmpty()) {
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(32.dp),
                    contentAlignment = Alignment.Center
                ) {
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        Icon(
                            Icons.Default.History,
                            "Empty history",
                            modifier = Modifier.size(64.dp),
                            tint = Color.Gray
                        )
                        Spacer(modifier = Modifier.height(16.dp))
                        Text("История пуста", color = Color.Gray)
                    }
                }
            } else {
                LazyColumn {
                    items(history) { item ->
                        ListItem(
                            modifier = Modifier.clickable { onItemClick(item) },
                            headlineContent = {
                                Text(
                                    item.title.ifEmpty { item.url },
                                    maxLines = 1,
                                    overflow = TextOverflow.Ellipsis
                                )
                            },
                            supportingContent = {
                                Text(
                                    item.url,
                                    maxLines = 1,
                                    overflow = TextOverflow.Ellipsis,
                                    color = Color.Gray,
                                    fontSize = 12.sp
                                )
                            },
                            trailingContent = {
                                Text(
                                    dateFormat.format(Date(item.timestamp)),
                                    fontSize = 11.sp,
                                    color = Color.Gray
                                )
                            },
                            leadingContent = {
                                Icon(
                                    Icons.Default.Public,
                                    "Website",
                                    tint = MaterialTheme.colorScheme.primary
                                )
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
        containerColor = MaterialTheme.colorScheme.surface
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .fillMaxHeight(0.8f)
        ) {
            Text(
                "Закладки",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.padding(16.dp)
            )
            
            if (bookmarks.isEmpty()) {
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(32.dp),
                    contentAlignment = Alignment.Center
                ) {
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        Icon(
                            Icons.Default.BookmarkBorder,
                            "Empty bookmarks",
                            modifier = Modifier.size(64.dp),
                            tint = Color.Gray
                        )
                        Spacer(modifier = Modifier.height(16.dp))
                        Text("Нет закладок", color = Color.Gray)
                    }
                }
            } else {
                LazyColumn {
                    items(bookmarks) { bookmark ->
                        ListItem(
                            modifier = Modifier.clickable { onItemClick(bookmark) },
                            headlineContent = {
                                Text(
                                    bookmark.title,
                                    maxLines = 1,
                                    overflow = TextOverflow.Ellipsis
                                )
                            },
                            supportingContent = {
                                Text(
                                    bookmark.url,
                                    maxLines = 1,
                                    overflow = TextOverflow.Ellipsis,
                                    color = Color.Gray,
                                    fontSize = 12.sp
                                )
                            },
                            leadingContent = {
                                Icon(
                                    Icons.Default.Bookmark,
                                    "Bookmark",
                                    tint = MaterialTheme.colorScheme.primary
                                )
                            },
                            trailingContent = {
                                IconButton(onClick = { onDelete(bookmark) }) {
                                    Icon(Icons.Default.Delete, "Delete", tint = Color.Gray)
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
        containerColor = MaterialTheme.colorScheme.surface
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .fillMaxHeight(0.6f)
        ) {
            Text(
                "Загрузки",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.padding(16.dp)
            )
            
            if (downloads.isEmpty()) {
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(32.dp),
                    contentAlignment = Alignment.Center
                ) {
                    Column(horizontalAlignment = Alignment.CenterHorizontally) {
                        Icon(
                            Icons.Default.Download,
                            "Empty downloads",
                            modifier = Modifier.size(64.dp),
                            tint = Color.Gray
                        )
                        Spacer(modifier = Modifier.height(16.dp))
                        Text("Нет загрузок", color = Color.Gray)
                    }
                }
            } else {
                LazyColumn {
                    items(downloads) { download ->
                        ListItem(
                            headlineContent = { Text(download.fileName) },
                            supportingContent = {
                                when (download.status) {
                                    DownloadStatus.DOWNLOADING -> LinearProgressIndicator(
                                        progress = download.progress / 100f,
                                        modifier = Modifier.fillMaxWidth()
                                    )
                                    DownloadStatus.COMPLETED -> Text("Завершено", color = Color(0xFF34A853))
                                    DownloadStatus.FAILED -> Text("Ошибка", color = Color(0xFFEA4335))
                                    DownloadStatus.PAUSED -> Text("Пауза", color = Color(0xFFFBBC04))
                                }
                            },
                            leadingContent = {
                                Icon(
                                    when (download.status) {
                                        DownloadStatus.COMPLETED -> Icons.Default.CheckCircle
                                        DownloadStatus.FAILED -> Icons.Default.Error
                                        else -> Icons.Default.Download
                                    },
                                    "Download status",
                                    tint = when (download.status) {
                                        DownloadStatus.COMPLETED -> Color(0xFF34A853)
                                        DownloadStatus.FAILED -> Color(0xFFEA4335)
                                        else -> MaterialTheme.colorScheme.primary
                                    }
                                )
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
        containerColor = MaterialTheme.colorScheme.surface
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .fillMaxHeight(0.9f)
                .verticalScroll(rememberScrollState())
                .padding(16.dp)
        ) {
            Text(
                "Настройки",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.padding(bottom = 16.dp)
            )
            
            // Appearance section
            Text(
                "Внешний вид",
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary,
                modifier = Modifier.padding(vertical = 8.dp)
            )
            
            SettingsToggle(
                title = "Тёмная тема",
                subtitle = "Включить тёмный режим",
                icon = Icons.Default.DarkMode,
                checked = state.isDarkMode,
                onCheckedChange = { state.isDarkMode = it }
            )
            
            // Font size
            Column(modifier = Modifier.padding(vertical = 8.dp)) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween
                ) {
                    Text("Размер шрифта")
                    Text("${state.fontSize}%")
                }
                Slider(
                    value = state.fontSize.toFloat(),
                    onValueChange = { state.fontSize = it.toInt() },
                    valueRange = 50f..200f,
                    steps = 14
                )
            }
            
            HorizontalDivider(modifier = Modifier.padding(vertical = 8.dp))
            
            // Privacy section
            Text(
                "Конфиденциальность",
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary,
                modifier = Modifier.padding(vertical = 8.dp)
            )
            
            SettingsToggle(
                title = "Блокировка рекламы",
                subtitle = "Блокировать рекламу и трекеры",
                icon = Icons.Default.Block,
                checked = state.isAdBlockEnabled,
                onCheckedChange = { state.isAdBlockEnabled = it }
            )
            
            SettingsToggle(
                title = "JavaScript",
                subtitle = "Разрешить выполнение JavaScript",
                icon = Icons.Default.Code,
                checked = state.isJavaScriptEnabled,
                onCheckedChange = { state.isJavaScriptEnabled = it }
            )
            
            HorizontalDivider(modifier = Modifier.padding(vertical = 8.dp))
            
            // Data section
            Text(
                "Данные",
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary,
                modifier = Modifier.padding(vertical = 8.dp)
            )
            
            SettingsToggle(
                title = "Экономия трафика",
                subtitle = "Использовать кэш для экономии данных",
                icon = Icons.Default.DataSaverOn,
                checked = state.isDataSaverEnabled,
                onCheckedChange = { state.isDataSaverEnabled = it }
            )
            
            // Search engine selector
            var expanded by remember { mutableStateOf(false) }
            
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { expanded = true }
                    .padding(vertical = 12.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(
                        Icons.Default.Search,
                        "Search engine",
                        modifier = Modifier.padding(end = 16.dp)
                    )
                    Column {
                        Text("Поисковая система")
                        Text(
                            state.searchEngine,
                            style = MaterialTheme.typography.bodySmall,
                            color = Color.Gray
                        )
                    }
                }
                Icon(Icons.Default.ArrowDropDown, "Select")
                
                DropdownMenu(
                    expanded = expanded,
                    onDismissRequest = { expanded = false }
                ) {
                    listOf("Google", "Яндекс", "DuckDuckGo", "Bing").forEach { engine ->
                        DropdownMenuItem(
                            text = { Text(engine) },
                            onClick = {
                                state.searchEngine = engine
                                expanded = false
                            }
                        )
                    }
                }
            }
            
            Spacer(modifier = Modifier.height(32.dp))
            
            // Version info
            Text(
                "ProBrowser v1.0.0",
                style = MaterialTheme.typography.bodySmall,
                color = Color.Gray,
                modifier = Modifier.align(Alignment.CenterHorizontally)
            )
            
            Spacer(modifier = Modifier.height(32.dp))
        }
    }
}

@Composable
fun SettingsToggle(
    title: String,
    subtitle: String,
    icon: ImageVector,
    checked: Boolean,
    onCheckedChange: (Boolean) -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { onCheckedChange(!checked) }
            .padding(vertical = 8.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Row(
            modifier = Modifier.weight(1f),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Icon(icon, title, modifier = Modifier.padding(end = 16.dp))
            Column {
                Text(title)
                Text(
                    subtitle,
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
            }
        }
        Switch(checked = checked, onCheckedChange = onCheckedChange)
    }
}

// ============== UTILITY FUNCTIONS ==============

fun processUrl(input: String, searchEngine: String): String {
    val trimmed = input.trim()
    
    // Check if it's a valid URL
    if (trimmed.contains(".") && !trimmed.contains(" ")) {
        return if (trimmed.startsWith("http://") || trimmed.startsWith("https://")) {
            trimmed
        } else {
            "https://$trimmed"
        }
    }
    
    // Otherwise, treat it as a search query
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
