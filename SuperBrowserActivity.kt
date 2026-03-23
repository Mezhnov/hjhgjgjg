package com.example.superbrowser

import android.annotation.SuppressLint
import android.app.DownloadManager
import android.content.Context
import android.content.Intent
import android.graphics.Bitmap
import android.net.Uri
import android.os.Bundle
import android.os.Environment
import android.util.Patterns
import android.view.ViewGroup
import android.webkit.*
import android.widget.FrameLayout
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.BackHandler
import androidx.activity.compose.setContent
import androidx.compose.animation.*
import androidx.compose.animation.core.*
import androidx.compose.foundation.*
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.focus.FocusRequester
import androidx.compose.ui.focus.focusRequester
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalFocusManager
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.compose.ui.window.Dialog
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewmodel.compose.viewModel
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import java.io.ByteArrayInputStream
import java.net.URLDecoder
import java.net.URLEncoder
import java.text.SimpleDateFormat
import java.util.*


// ==========================================
// DATA MODELS & ENUMS
// ==========================================

data class TabState(
    val id: String = UUID.randomUUID().toString(),
    var url: String = "about:blank",
    var title: String = "New Tab",
    var favicon: Bitmap? = null,
    var isLoading: Boolean = false,
    var loadProgress: Int = 0,
    var canGoBack: Boolean = false,
    var canGoForward: Boolean = false,
    var isIncognito: Boolean = false,
    var webView: WebView? = null
)

data class Bookmark(
    val id: String = UUID.randomUUID().toString(),
    val title: String,
    val url: String,
    val timestamp: Long = System.currentTimeMillis()
)

data class HistoryItem(
    val id: String = UUID.randomUUID().toString(),
    val title: String,
    val url: String,
    val timestamp: Long = System.currentTimeMillis()
)

enum class SearchEngine(val engineName: String, val searchUrl: String) {
    GOOGLE("Google", "https://www.google.com/search?q="),
    DUCKDUCKGO("DuckDuckGo", "https://duckduckgo.com/?q="),
    BING("Bing", "https://www.bing.com/search?q="),
    YANDEX("Yandex", "https://yandex.ru/search/?text=")
}

enum class BrowserScreen {
    BROWSER, TABS, SETTINGS, BOOKMARKS, HISTORY
}


// ==========================================
// VIEW MODEL (STATE MANAGEMENT)
// ==========================================

class BrowserViewModel : ViewModel() {
    // Current UI Screen
    var currentScreen by mutableStateOf(BrowserScreen.BROWSER)
    
    // Tabs Management
    var tabs = mutableStateListOf<TabState>()
    var currentTabId by mutableStateOf<String?>(null)
    
    // Core Features
    var bookmarks = mutableStateListOf<Bookmark>()
    var history = mutableStateListOf<HistoryItem>()
    
    // Settings
    var searchEngine by mutableStateOf(SearchEngine.GOOGLE)
    var isAdBlockEnabled by mutableStateOf(true)
    var isJavaScriptEnabled by mutableStateOf(true)
    var isDesktopMode by mutableStateOf(false)
    var isDarkModeEnabled by mutableStateOf(false)
    
    // UI States
    var showAddressBar by mutableStateOf(true)
    var isOmniboxFocused by mutableStateOf(false)
    var omniboxText by mutableStateOf("")

    init {
        // Create initial tab
        createNewTab("https://www.google.com")
        
        // Add some mock bookmarks for demonstration
        bookmarks.add(Bookmark(title = "GitHub", url = "https://github.com"))
        bookmarks.add(Bookmark(title = "StackOverflow", url = "https://stackoverflow.com"))
        bookmarks.add(Bookmark(title = "Kotlin", url = "https://kotlinlang.org"))
    }

    val currentTab: TabState?
        get() = tabs.find { it.id == currentTabId }

    fun createNewTab(url: String = "https://www.google.com", isIncognito: Boolean = false) {
        val newTab = TabState(url = url, isIncognito = isIncognito)
        tabs.add(newTab)
        currentTabId = newTab.id
        currentScreen = BrowserScreen.BROWSER
    }

    fun closeTab(tabId: String) {
        val index = tabs.indexOfFirst { it.id == tabId }
        if (index != -1) {
            val tab = tabs[index]
            tab.webView?.destroy() 
            tabs.removeAt(index)
            
            if (currentTabId == tabId) {
                if (tabs.isNotEmpty()) {
                    currentTabId = tabs[Math.max(0, index - 1)].id
                } else {
                    currentTabId = null
                    // Automatically create a new tab if closed the last one
                    createNewTab()
                }
            }
        }
    }

    fun switchTab(tabId: String) {
        currentTabId = tabId
        currentScreen = BrowserScreen.BROWSER
    }

    fun toggleBookmark(url: String, title: String) {
        val existing = bookmarks.find { it.url == url }
        if (existing != null) {
            bookmarks.remove(existing)
        } else {
            bookmarks.add(Bookmark(title = title, url = url))
        }
    }

    fun isBookmarked(url: String): Boolean {
        return bookmarks.any { it.url == url }
    }

    fun addToHistory(url: String, title: String) {
        // Avoid adding duplicates in a row
        if (history.isNotEmpty() && history.first().url == url) return
        
        // If it's an incognito tab, don't record history
        if (currentTab?.isIncognito == true) return
        
        history.add(0, HistoryItem(title = title, url = url))
        // Keep only last 1000 items
        if (history.size > 1000) {
            history.removeAt(history.lastIndex)
        }
    }
    
    fun clearHistory() {
        history.clear()
    }
    
    fun loadUrlInCurrentTab(rawInput: String) {
        val url = formatUrl(rawInput)
        val tab = currentTab
        if (tab != null) {
            tab.url = url
            tab.webView?.loadUrl(url)
            omniboxText = url
        }
    }
    
    private fun formatUrl(input: String): String {
        return if (Patterns.WEB_URL.matcher(input).matches()) {
            if (input.startsWith("http://") || input.startsWith("https://")) {
                input
            } else {
                "https://$input"
            }
        } else {
            searchEngine.searchUrl + URLEncoder.encode(input, "UTF-8")
        }
    }
}


// ==========================================
// WEBVIEW CLIENTS & CORE LOGIC
// ==========================================

class CustomWebViewClient(
    private val viewModel: BrowserViewModel,
    private val tabId: String,
    private val adBlocker: AdBlocker
) : WebViewClient() {

    override fun shouldOverrideUrlLoading(view: WebView?, request: WebResourceRequest?): Boolean {
        val url = request?.url.toString()
        if (url.startsWith("http://") || url.startsWith("https://")) {
            return false // Let WebView handle it
        }
        // Handle intents (e.g. mailto, tel, deep links)
        return try {
            val intent = Intent(Intent.ACTION_VIEW, Uri.parse(url))
            view?.context?.startActivity(intent)
            true
        } catch (e: Exception) {
            true // Cannot handle, but consumed
        }
    }

    override fun onPageStarted(view: WebView?, url: String?, favicon: Bitmap?) {
        super.onPageStarted(view, url, favicon)
        val tab = viewModel.tabs.find { it.id == tabId }
        tab?.let {
            it.isLoading = true
            it.url = url ?: "about:blank"
            if (viewModel.currentTabId == tabId) {
                viewModel.omniboxText = it.url
            }
        }
    }

    override fun onPageFinished(view: WebView?, url: String?) {
        super.onPageFinished(view, url)
        val tab = viewModel.tabs.find { it.id == tabId }
        tab?.let {
            it.isLoading = false
            it.title = view?.title ?: url ?: "New Tab"
            it.canGoBack = view?.canGoBack() ?: false
            it.canGoForward = view?.canGoForward() ?: false
            
            // Inject dark mode CSS if enabled
            if (viewModel.isDarkModeEnabled) {
                injectDarkMode(view)
            }
        }
        
        if (url != null && view?.title != null) {
            viewModel.addToHistory(url, view.title!!)
        }
    }

    override fun shouldInterceptRequest(view: WebView?, request: WebResourceRequest?): WebResourceResponse? {
        if (viewModel.isAdBlockEnabled && request != null) {
            val url = request.url.toString()
            if (adBlocker.isAd(url)) {
                // Return empty response to block the resource
                return WebResourceResponse("text/plain", "UTF-8", ByteArrayInputStream(ByteArray(0)))
            }
        }
        return super.shouldInterceptRequest(view, request)
    }
    
    private fun injectDarkMode(view: WebView?) {
        val css = "html, body { background-color: #121212 !important; color: #E0E0E0 !important; }"
        val js = "var style = document.createElement('style'); style.innerHTML = '$css'; document.head.appendChild(style);"
        view?.evaluateJavascript(js, null)
    }
}

class CustomWebChromeClient(
    private val viewModel: BrowserViewModel,
    private val tabId: String
) : WebChromeClient() {

    override fun onProgressChanged(view: WebView?, newProgress: Int) {
        super.onProgressChanged(view, newProgress)
        val tab = viewModel.tabs.find { it.id == tabId }
        tab?.loadProgress = newProgress
    }

    override fun onReceivedTitle(view: WebView?, title: String?) {
        super.onReceivedTitle(view, title)
        val tab = viewModel.tabs.find { it.id == tabId }
        tab?.title = title ?: tab?.url ?: "New Tab"
    }

    override fun onReceivedIcon(view: WebView?, icon: Bitmap?) {
        super.onReceivedIcon(view, icon)
        val tab = viewModel.tabs.find { it.id == tabId }
        tab?.favicon = icon
    }
    
    // Support for full screen videos
    private var customView: android.view.View? = null
    private var customViewCallback: CustomViewCallback? = null

    override fun onShowCustomView(view: android.view.View?, callback: CustomViewCallback?) {
        if (customView != null) {
            callback?.onCustomViewHidden()
            return
        }
        customView = view
        customViewCallback = callback
        // In a real app, you would add `customView` to the Window DecorView
    }

    override fun onHideCustomView() {
        super.onHideCustomView()
        if (customView == null) return
        // Remove from DecorView here
        customView = null
        customViewCallback?.onCustomViewHidden()
    }
}


// ==========================================
// AD BLOCKER LOGIC & DATA
// ==========================================

class AdBlocker {
    private val blockedDomains = HashSet<String>()

    init {
        loadRules()
    }

    fun isAd(url: String): Boolean {
        try {
            val host = Uri.parse(url).host ?: return false
            var currentHost = host
            // Check domain and super-domains
            while (currentHost.contains(".")) {
                if (blockedDomains.contains(currentHost)) {
                    return true
                }
                val firstDot = currentHost.indexOf('.')
                if (firstDot != -1 && firstDot + 1 < currentHost.length) {
                    currentHost = currentHost.substring(firstDot + 1)
                } else {
                    break
                }
            }
        } catch (e: Exception) {
            // Ignore parse exceptions
        }
        return false
    }

    private fun loadRules() {
        val rules = listOf(
            "doubleclick.net",
            "admob.com",
            "googleadservices.com",
            "googlesyndication.com",
            "facebook.com/tr",
            "analytics.yahoo.com",
            "adnexus.com",
            "scorecardresearch.com",
            "quantserve.com",
            "moatads.com",
            "taboola.com",
            "outbrain.com",
            "criteo.com",
            "adnxs.com",
            "rubiconproject.com",
            "pubmatic.com",
            "indexexchange.com",
            "openx.net",
            "smartadserver.com",
            "yieldmo.com",
            "teads.tv",
            "appnexus.com",
            "ad-server-mock-0.com",
            "tracker-analytics-0.net",
            "banner-ads-hosting-0.org",
            "ad-server-mock-1.com",
            "tracker-analytics-1.net",
            "banner-ads-hosting-1.org",
            "ad-server-mock-2.com",
            "tracker-analytics-2.net",
            "banner-ads-hosting-2.org",
            "ad-server-mock-3.com",
            "tracker-analytics-3.net",
            "banner-ads-hosting-3.org",
            "ad-server-mock-4.com",
            "tracker-analytics-4.net",
            "banner-ads-hosting-4.org",
            "ad-server-mock-5.com",
            "tracker-analytics-5.net",
            "banner-ads-hosting-5.org",
            "ad-server-mock-6.com",
            "tracker-analytics-6.net",
            "banner-ads-hosting-6.org",
            "ad-server-mock-7.com",
            "tracker-analytics-7.net",
            "banner-ads-hosting-7.org",
            "ad-server-mock-8.com",
            "tracker-analytics-8.net",
            "banner-ads-hosting-8.org",
            "ad-server-mock-9.com",
            "tracker-analytics-9.net",
            "banner-ads-hosting-9.org",
            "ad-server-mock-10.com",
            "tracker-analytics-10.net",
            "banner-ads-hosting-10.org",
            "ad-server-mock-11.com",
            "tracker-analytics-11.net",
            "banner-ads-hosting-11.org",
            "ad-server-mock-12.com",
            "tracker-analytics-12.net",
            "banner-ads-hosting-12.org",
            "ad-server-mock-13.com",
            "tracker-analytics-13.net",
            "banner-ads-hosting-13.org",
            "ad-server-mock-14.com",
            "tracker-analytics-14.net",
            "banner-ads-hosting-14.org",
            "ad-server-mock-15.com",
            "tracker-analytics-15.net",
            "banner-ads-hosting-15.org",
            "ad-server-mock-16.com",
            "tracker-analytics-16.net",
            "banner-ads-hosting-16.org",
            "ad-server-mock-17.com",
            "tracker-analytics-17.net",
            "banner-ads-hosting-17.org",
            "ad-server-mock-18.com",
            "tracker-analytics-18.net",
            "banner-ads-hosting-18.org",
            "ad-server-mock-19.com",
            "tracker-analytics-19.net",
            "banner-ads-hosting-19.org",
            "ad-server-mock-20.com",
            "tracker-analytics-20.net",
            "banner-ads-hosting-20.org",
            "ad-server-mock-21.com",
            "tracker-analytics-21.net",
            "banner-ads-hosting-21.org",
            "ad-server-mock-22.com",
            "tracker-analytics-22.net",
            "banner-ads-hosting-22.org",
            "ad-server-mock-23.com",
            "tracker-analytics-23.net",
            "banner-ads-hosting-23.org",
            "ad-server-mock-24.com",
            "tracker-analytics-24.net",
            "banner-ads-hosting-24.org",
            "ad-server-mock-25.com",
            "tracker-analytics-25.net",
            "banner-ads-hosting-25.org",
            "ad-server-mock-26.com",
            "tracker-analytics-26.net",
            "banner-ads-hosting-26.org",
            "ad-server-mock-27.com",
            "tracker-analytics-27.net",
            "banner-ads-hosting-27.org",
            "ad-server-mock-28.com",
            "tracker-analytics-28.net",
            "banner-ads-hosting-28.org",
            "ad-server-mock-29.com",
            "tracker-analytics-29.net",
            "banner-ads-hosting-29.org",
            "ad-server-mock-30.com",
            "tracker-analytics-30.net",
            "banner-ads-hosting-30.org",
            "ad-server-mock-31.com",
            "tracker-analytics-31.net",
            "banner-ads-hosting-31.org",
            "ad-server-mock-32.com",
            "tracker-analytics-32.net",
            "banner-ads-hosting-32.org",
            "ad-server-mock-33.com",
            "tracker-analytics-33.net",
            "banner-ads-hosting-33.org",
            "ad-server-mock-34.com",
            "tracker-analytics-34.net",
            "banner-ads-hosting-34.org",
            "ad-server-mock-35.com",
            "tracker-analytics-35.net",
            "banner-ads-hosting-35.org",
            "ad-server-mock-36.com",
            "tracker-analytics-36.net",
            "banner-ads-hosting-36.org",
            "ad-server-mock-37.com",
            "tracker-analytics-37.net",
            "banner-ads-hosting-37.org",
            "ad-server-mock-38.com",
            "tracker-analytics-38.net",
            "banner-ads-hosting-38.org",
            "ad-server-mock-39.com",
            "tracker-analytics-39.net",
            "banner-ads-hosting-39.org",
            "ad-server-mock-40.com",
            "tracker-analytics-40.net",
            "banner-ads-hosting-40.org",
            "ad-server-mock-41.com",
            "tracker-analytics-41.net",
            "banner-ads-hosting-41.org",
            "ad-server-mock-42.com",
            "tracker-analytics-42.net",
            "banner-ads-hosting-42.org",
            "ad-server-mock-43.com",
            "tracker-analytics-43.net",
            "banner-ads-hosting-43.org",
            "ad-server-mock-44.com",
            "tracker-analytics-44.net",
            "banner-ads-hosting-44.org",
            "ad-server-mock-45.com",
            "tracker-analytics-45.net",
            "banner-ads-hosting-45.org",
            "ad-server-mock-46.com",
            "tracker-analytics-46.net",
            "banner-ads-hosting-46.org",
            "ad-server-mock-47.com",
            "tracker-analytics-47.net",
            "banner-ads-hosting-47.org",
            "ad-server-mock-48.com",
            "tracker-analytics-48.net",
            "banner-ads-hosting-48.org",
            "ad-server-mock-49.com",
            "tracker-analytics-49.net",
            "banner-ads-hosting-49.org",
            "ad-server-mock-50.com",
            "tracker-analytics-50.net",
            "banner-ads-hosting-50.org",
            "ad-server-mock-51.com",
            "tracker-analytics-51.net",
            "banner-ads-hosting-51.org",
            "ad-server-mock-52.com",
            "tracker-analytics-52.net",
            "banner-ads-hosting-52.org",
            "ad-server-mock-53.com",
            "tracker-analytics-53.net",
            "banner-ads-hosting-53.org",
            "ad-server-mock-54.com",
            "tracker-analytics-54.net",
            "banner-ads-hosting-54.org",
            "ad-server-mock-55.com",
            "tracker-analytics-55.net",
            "banner-ads-hosting-55.org",
            "ad-server-mock-56.com",
            "tracker-analytics-56.net",
            "banner-ads-hosting-56.org",
            "ad-server-mock-57.com",
            "tracker-analytics-57.net",
            "banner-ads-hosting-57.org",
            "ad-server-mock-58.com",
            "tracker-analytics-58.net",
            "banner-ads-hosting-58.org",
            "ad-server-mock-59.com",
            "tracker-analytics-59.net",
            "banner-ads-hosting-59.org",
            "ad-server-mock-60.com",
            "tracker-analytics-60.net",
            "banner-ads-hosting-60.org",
            "ad-server-mock-61.com",
            "tracker-analytics-61.net",
            "banner-ads-hosting-61.org",
            "ad-server-mock-62.com",
            "tracker-analytics-62.net",
            "banner-ads-hosting-62.org",
            "ad-server-mock-63.com",
            "tracker-analytics-63.net",
            "banner-ads-hosting-63.org",
            "ad-server-mock-64.com",
            "tracker-analytics-64.net",
            "banner-ads-hosting-64.org",
            "ad-server-mock-65.com",
            "tracker-analytics-65.net",
            "banner-ads-hosting-65.org",
            "ad-server-mock-66.com",
            "tracker-analytics-66.net",
            "banner-ads-hosting-66.org",
            "ad-server-mock-67.com",
            "tracker-analytics-67.net",
            "banner-ads-hosting-67.org",
            "ad-server-mock-68.com",
            "tracker-analytics-68.net",
            "banner-ads-hosting-68.org",
            "ad-server-mock-69.com",
            "tracker-analytics-69.net",
            "banner-ads-hosting-69.org",
            "ad-server-mock-70.com",
            "tracker-analytics-70.net",
            "banner-ads-hosting-70.org",
            "ad-server-mock-71.com",
            "tracker-analytics-71.net",
            "banner-ads-hosting-71.org",
            "ad-server-mock-72.com",
            "tracker-analytics-72.net",
            "banner-ads-hosting-72.org",
            "ad-server-mock-73.com",
            "tracker-analytics-73.net",
            "banner-ads-hosting-73.org",
            "ad-server-mock-74.com",
            "tracker-analytics-74.net",
            "banner-ads-hosting-74.org",
            "ad-server-mock-75.com",
            "tracker-analytics-75.net",
            "banner-ads-hosting-75.org",
            "ad-server-mock-76.com",
            "tracker-analytics-76.net",
            "banner-ads-hosting-76.org",
            "ad-server-mock-77.com",
            "tracker-analytics-77.net",
            "banner-ads-hosting-77.org",
            "ad-server-mock-78.com",
            "tracker-analytics-78.net",
            "banner-ads-hosting-78.org",
            "ad-server-mock-79.com",
            "tracker-analytics-79.net",
            "banner-ads-hosting-79.org",
            "ad-server-mock-80.com",
            "tracker-analytics-80.net",
            "banner-ads-hosting-80.org",
            "ad-server-mock-81.com",
            "tracker-analytics-81.net",
            "banner-ads-hosting-81.org",
            "ad-server-mock-82.com",
            "tracker-analytics-82.net",
            "banner-ads-hosting-82.org",
            "ad-server-mock-83.com",
            "tracker-analytics-83.net",
            "banner-ads-hosting-83.org",
            "ad-server-mock-84.com",
            "tracker-analytics-84.net",
            "banner-ads-hosting-84.org",
            "ad-server-mock-85.com",
            "tracker-analytics-85.net",
            "banner-ads-hosting-85.org",
            "ad-server-mock-86.com",
            "tracker-analytics-86.net",
            "banner-ads-hosting-86.org",
            "ad-server-mock-87.com",
            "tracker-analytics-87.net",
            "banner-ads-hosting-87.org",
            "ad-server-mock-88.com",
            "tracker-analytics-88.net",
            "banner-ads-hosting-88.org",
            "ad-server-mock-89.com",
            "tracker-analytics-89.net",
            "banner-ads-hosting-89.org",
            "ad-server-mock-90.com",
            "tracker-analytics-90.net",
            "banner-ads-hosting-90.org",
            "ad-server-mock-91.com",
            "tracker-analytics-91.net",
            "banner-ads-hosting-91.org",
            "ad-server-mock-92.com",
            "tracker-analytics-92.net",
            "banner-ads-hosting-92.org",
            "ad-server-mock-93.com",
            "tracker-analytics-93.net",
            "banner-ads-hosting-93.org",
            "ad-server-mock-94.com",
            "tracker-analytics-94.net",
            "banner-ads-hosting-94.org",
            "ad-server-mock-95.com",
            "tracker-analytics-95.net",
            "banner-ads-hosting-95.org",
            "ad-server-mock-96.com",
            "tracker-analytics-96.net",
            "banner-ads-hosting-96.org",
            "ad-server-mock-97.com",
            "tracker-analytics-97.net",
            "banner-ads-hosting-97.org",
            "ad-server-mock-98.com",
            "tracker-analytics-98.net",
            "banner-ads-hosting-98.org",
            "ad-server-mock-99.com",
            "tracker-analytics-99.net",
            "banner-ads-hosting-99.org",
            "ad-server-mock-100.com",
            "tracker-analytics-100.net",
            "banner-ads-hosting-100.org",
            "ad-server-mock-101.com",
            "tracker-analytics-101.net",
            "banner-ads-hosting-101.org",
            "ad-server-mock-102.com",
            "tracker-analytics-102.net",
            "banner-ads-hosting-102.org",
            "ad-server-mock-103.com",
            "tracker-analytics-103.net",
            "banner-ads-hosting-103.org",
            "ad-server-mock-104.com",
            "tracker-analytics-104.net",
            "banner-ads-hosting-104.org",
            "ad-server-mock-105.com",
            "tracker-analytics-105.net",
            "banner-ads-hosting-105.org",
            "ad-server-mock-106.com",
            "tracker-analytics-106.net",
            "banner-ads-hosting-106.org",
            "ad-server-mock-107.com",
            "tracker-analytics-107.net",
            "banner-ads-hosting-107.org",
            "ad-server-mock-108.com",
            "tracker-analytics-108.net",
            "banner-ads-hosting-108.org",
            "ad-server-mock-109.com",
            "tracker-analytics-109.net",
            "banner-ads-hosting-109.org",
            "ad-server-mock-110.com",
            "tracker-analytics-110.net",
            "banner-ads-hosting-110.org",
            "ad-server-mock-111.com",
            "tracker-analytics-111.net",
            "banner-ads-hosting-111.org",
            "ad-server-mock-112.com",
            "tracker-analytics-112.net",
            "banner-ads-hosting-112.org",
            "ad-server-mock-113.com",
            "tracker-analytics-113.net",
            "banner-ads-hosting-113.org",
            "ad-server-mock-114.com",
            "tracker-analytics-114.net",
            "banner-ads-hosting-114.org",
            "ad-server-mock-115.com",
            "tracker-analytics-115.net",
            "banner-ads-hosting-115.org",
            "ad-server-mock-116.com",
            "tracker-analytics-116.net",
            "banner-ads-hosting-116.org",
            "ad-server-mock-117.com",
            "tracker-analytics-117.net",
            "banner-ads-hosting-117.org",
            "ad-server-mock-118.com",
            "tracker-analytics-118.net",
            "banner-ads-hosting-118.org",
            "ad-server-mock-119.com",
            "tracker-analytics-119.net",
            "banner-ads-hosting-119.org",
            "ad-server-mock-120.com",
            "tracker-analytics-120.net",
            "banner-ads-hosting-120.org",
            "ad-server-mock-121.com",
            "tracker-analytics-121.net",
            "banner-ads-hosting-121.org",
            "ad-server-mock-122.com",
            "tracker-analytics-122.net",
            "banner-ads-hosting-122.org",
            "ad-server-mock-123.com",
            "tracker-analytics-123.net",
            "banner-ads-hosting-123.org",
            "ad-server-mock-124.com",
            "tracker-analytics-124.net",
            "banner-ads-hosting-124.org",
            "ad-server-mock-125.com",
            "tracker-analytics-125.net",
            "banner-ads-hosting-125.org",
            "ad-server-mock-126.com",
            "tracker-analytics-126.net",
            "banner-ads-hosting-126.org",
            "ad-server-mock-127.com",
            "tracker-analytics-127.net",
            "banner-ads-hosting-127.org",
            "ad-server-mock-128.com",
            "tracker-analytics-128.net",
            "banner-ads-hosting-128.org",
            "ad-server-mock-129.com",
            "tracker-analytics-129.net",
            "banner-ads-hosting-129.org",
            "ad-server-mock-130.com",
            "tracker-analytics-130.net",
            "banner-ads-hosting-130.org",
            "ad-server-mock-131.com",
            "tracker-analytics-131.net",
            "banner-ads-hosting-131.org",
            "ad-server-mock-132.com",
            "tracker-analytics-132.net",
            "banner-ads-hosting-132.org",
            "ad-server-mock-133.com",
            "tracker-analytics-133.net",
            "banner-ads-hosting-133.org",
            "ad-server-mock-134.com",
            "tracker-analytics-134.net",
            "banner-ads-hosting-134.org",
            "ad-server-mock-135.com",
            "tracker-analytics-135.net",
            "banner-ads-hosting-135.org",
            "ad-server-mock-136.com",
            "tracker-analytics-136.net",
            "banner-ads-hosting-136.org",
            "ad-server-mock-137.com",
            "tracker-analytics-137.net",
            "banner-ads-hosting-137.org",
            "ad-server-mock-138.com",
            "tracker-analytics-138.net",
            "banner-ads-hosting-138.org",
            "ad-server-mock-139.com",
            "tracker-analytics-139.net",
            "banner-ads-hosting-139.org",
            "ad-server-mock-140.com",
            "tracker-analytics-140.net",
            "banner-ads-hosting-140.org",
            "ad-server-mock-141.com",
            "tracker-analytics-141.net",
            "banner-ads-hosting-141.org",
            "ad-server-mock-142.com",
            "tracker-analytics-142.net",
            "banner-ads-hosting-142.org",
            "ad-server-mock-143.com",
            "tracker-analytics-143.net",
            "banner-ads-hosting-143.org",
            "ad-server-mock-144.com",
            "tracker-analytics-144.net",
            "banner-ads-hosting-144.org",
            "ad-server-mock-145.com",
            "tracker-analytics-145.net",
            "banner-ads-hosting-145.org",
            "ad-server-mock-146.com",
            "tracker-analytics-146.net",
            "banner-ads-hosting-146.org",
            "ad-server-mock-147.com",
            "tracker-analytics-147.net",
            "banner-ads-hosting-147.org",
            "ad-server-mock-148.com",
            "tracker-analytics-148.net",
            "banner-ads-hosting-148.org",
            "ad-server-mock-149.com",
            "tracker-analytics-149.net",
            "banner-ads-hosting-149.org",
            "ad-server-mock-150.com",
            "tracker-analytics-150.net",
            "banner-ads-hosting-150.org",
            "ad-server-mock-151.com",
            "tracker-analytics-151.net",
            "banner-ads-hosting-151.org",
            "ad-server-mock-152.com",
            "tracker-analytics-152.net",
            "banner-ads-hosting-152.org",
            "ad-server-mock-153.com",
            "tracker-analytics-153.net",
            "banner-ads-hosting-153.org",
            "ad-server-mock-154.com",
            "tracker-analytics-154.net",
            "banner-ads-hosting-154.org",
            "ad-server-mock-155.com",
            "tracker-analytics-155.net",
            "banner-ads-hosting-155.org",
            "ad-server-mock-156.com",
            "tracker-analytics-156.net",
            "banner-ads-hosting-156.org",
            "ad-server-mock-157.com",
            "tracker-analytics-157.net",
            "banner-ads-hosting-157.org",
            "ad-server-mock-158.com",
            "tracker-analytics-158.net",
            "banner-ads-hosting-158.org",
            "ad-server-mock-159.com",
            "tracker-analytics-159.net",
            "banner-ads-hosting-159.org",
            "ad-server-mock-160.com",
            "tracker-analytics-160.net",
            "banner-ads-hosting-160.org",
            "ad-server-mock-161.com",
            "tracker-analytics-161.net",
            "banner-ads-hosting-161.org",
            "ad-server-mock-162.com",
            "tracker-analytics-162.net",
            "banner-ads-hosting-162.org",
            "ad-server-mock-163.com",
            "tracker-analytics-163.net",
            "banner-ads-hosting-163.org",
            "ad-server-mock-164.com",
            "tracker-analytics-164.net",
            "banner-ads-hosting-164.org",
            "ad-server-mock-165.com",
            "tracker-analytics-165.net",
            "banner-ads-hosting-165.org",
            "ad-server-mock-166.com",
            "tracker-analytics-166.net",
            "banner-ads-hosting-166.org",
            "ad-server-mock-167.com",
            "tracker-analytics-167.net",
            "banner-ads-hosting-167.org",
            "ad-server-mock-168.com",
            "tracker-analytics-168.net",
            "banner-ads-hosting-168.org",
            "ad-server-mock-169.com",
            "tracker-analytics-169.net",
            "banner-ads-hosting-169.org",
            "ad-server-mock-170.com",
            "tracker-analytics-170.net",
            "banner-ads-hosting-170.org",
            "ad-server-mock-171.com",
            "tracker-analytics-171.net",
            "banner-ads-hosting-171.org",
            "ad-server-mock-172.com",
            "tracker-analytics-172.net",
            "banner-ads-hosting-172.org",
            "ad-server-mock-173.com",
            "tracker-analytics-173.net",
            "banner-ads-hosting-173.org",
            "ad-server-mock-174.com",
            "tracker-analytics-174.net",
            "banner-ads-hosting-174.org",
            "ad-server-mock-175.com",
            "tracker-analytics-175.net",
            "banner-ads-hosting-175.org",
            "ad-server-mock-176.com",
            "tracker-analytics-176.net",
            "banner-ads-hosting-176.org",
            "ad-server-mock-177.com",
            "tracker-analytics-177.net",
            "banner-ads-hosting-177.org",
            "ad-server-mock-178.com",
            "tracker-analytics-178.net",
            "banner-ads-hosting-178.org",
            "ad-server-mock-179.com",
            "tracker-analytics-179.net",
            "banner-ads-hosting-179.org",
            "ad-server-mock-180.com",
            "tracker-analytics-180.net",
            "banner-ads-hosting-180.org",
            "ad-server-mock-181.com",
            "tracker-analytics-181.net",
            "banner-ads-hosting-181.org",
            "ad-server-mock-182.com",
            "tracker-analytics-182.net",
            "banner-ads-hosting-182.org",
            "ad-server-mock-183.com",
            "tracker-analytics-183.net",
            "banner-ads-hosting-183.org",
            "ad-server-mock-184.com",
            "tracker-analytics-184.net",
            "banner-ads-hosting-184.org",
            "ad-server-mock-185.com",
            "tracker-analytics-185.net",
            "banner-ads-hosting-185.org",
            "ad-server-mock-186.com",
            "tracker-analytics-186.net",
            "banner-ads-hosting-186.org",
            "ad-server-mock-187.com",
            "tracker-analytics-187.net",
            "banner-ads-hosting-187.org",
            "ad-server-mock-188.com",
            "tracker-analytics-188.net",
            "banner-ads-hosting-188.org",
            "ad-server-mock-189.com",
            "tracker-analytics-189.net",
            "banner-ads-hosting-189.org",
            "ad-server-mock-190.com",
            "tracker-analytics-190.net",
            "banner-ads-hosting-190.org",
            "ad-server-mock-191.com",
            "tracker-analytics-191.net",
            "banner-ads-hosting-191.org",
            "ad-server-mock-192.com",
            "tracker-analytics-192.net",
            "banner-ads-hosting-192.org",
            "ad-server-mock-193.com",
            "tracker-analytics-193.net",
            "banner-ads-hosting-193.org",
            "ad-server-mock-194.com",
            "tracker-analytics-194.net",
            "banner-ads-hosting-194.org",
            "ad-server-mock-195.com",
            "tracker-analytics-195.net",
            "banner-ads-hosting-195.org",
            "ad-server-mock-196.com",
            "tracker-analytics-196.net",
            "banner-ads-hosting-196.org",
            "ad-server-mock-197.com",
            "tracker-analytics-197.net",
            "banner-ads-hosting-197.org",
            "ad-server-mock-198.com",
            "tracker-analytics-198.net",
            "banner-ads-hosting-198.org",
            "ad-server-mock-199.com",
            "tracker-analytics-199.net",
            "banner-ads-hosting-199.org",
            "ad-server-mock-200.com",
            "tracker-analytics-200.net",
            "banner-ads-hosting-200.org",
            "ad-server-mock-201.com",
            "tracker-analytics-201.net",
            "banner-ads-hosting-201.org",
            "ad-server-mock-202.com",
            "tracker-analytics-202.net",
            "banner-ads-hosting-202.org",
            "ad-server-mock-203.com",
            "tracker-analytics-203.net",
            "banner-ads-hosting-203.org",
            "ad-server-mock-204.com",
            "tracker-analytics-204.net",
            "banner-ads-hosting-204.org",
            "ad-server-mock-205.com",
            "tracker-analytics-205.net",
            "banner-ads-hosting-205.org",
            "ad-server-mock-206.com",
            "tracker-analytics-206.net",
            "banner-ads-hosting-206.org",
            "ad-server-mock-207.com",
            "tracker-analytics-207.net",
            "banner-ads-hosting-207.org",
            "ad-server-mock-208.com",
            "tracker-analytics-208.net",
            "banner-ads-hosting-208.org",
            "ad-server-mock-209.com",
            "tracker-analytics-209.net",
            "banner-ads-hosting-209.org",
            "ad-server-mock-210.com",
            "tracker-analytics-210.net",
            "banner-ads-hosting-210.org",
            "ad-server-mock-211.com",
            "tracker-analytics-211.net",
            "banner-ads-hosting-211.org",
            "ad-server-mock-212.com",
            "tracker-analytics-212.net",
            "banner-ads-hosting-212.org",
            "ad-server-mock-213.com",
            "tracker-analytics-213.net",
            "banner-ads-hosting-213.org",
            "ad-server-mock-214.com",
            "tracker-analytics-214.net",
            "banner-ads-hosting-214.org",
            "ad-server-mock-215.com",
            "tracker-analytics-215.net",
            "banner-ads-hosting-215.org",
            "ad-server-mock-216.com",
            "tracker-analytics-216.net",
            "banner-ads-hosting-216.org",
            "ad-server-mock-217.com",
            "tracker-analytics-217.net",
            "banner-ads-hosting-217.org",
            "ad-server-mock-218.com",
            "tracker-analytics-218.net",
            "banner-ads-hosting-218.org",
            "ad-server-mock-219.com",
            "tracker-analytics-219.net",
            "banner-ads-hosting-219.org",
            "ad-server-mock-220.com",
            "tracker-analytics-220.net",
            "banner-ads-hosting-220.org",
            "ad-server-mock-221.com",
            "tracker-analytics-221.net",
            "banner-ads-hosting-221.org",
            "ad-server-mock-222.com",
            "tracker-analytics-222.net",
            "banner-ads-hosting-222.org",
            "ad-server-mock-223.com",
            "tracker-analytics-223.net",
            "banner-ads-hosting-223.org",
            "ad-server-mock-224.com",
            "tracker-analytics-224.net",
            "banner-ads-hosting-224.org",
            "ad-server-mock-225.com",
            "tracker-analytics-225.net",
            "banner-ads-hosting-225.org",
            "ad-server-mock-226.com",
            "tracker-analytics-226.net",
            "banner-ads-hosting-226.org",
            "ad-server-mock-227.com",
            "tracker-analytics-227.net",
            "banner-ads-hosting-227.org",
            "ad-server-mock-228.com",
            "tracker-analytics-228.net",
            "banner-ads-hosting-228.org",
            "ad-server-mock-229.com",
            "tracker-analytics-229.net",
            "banner-ads-hosting-229.org",
            "ad-server-mock-230.com",
            "tracker-analytics-230.net",
            "banner-ads-hosting-230.org",
            "ad-server-mock-231.com",
            "tracker-analytics-231.net",
            "banner-ads-hosting-231.org",
            "ad-server-mock-232.com",
            "tracker-analytics-232.net",
            "banner-ads-hosting-232.org",
            "ad-server-mock-233.com",
            "tracker-analytics-233.net",
            "banner-ads-hosting-233.org",
            "ad-server-mock-234.com",
            "tracker-analytics-234.net",
            "banner-ads-hosting-234.org",
            "ad-server-mock-235.com",
            "tracker-analytics-235.net",
            "banner-ads-hosting-235.org",
            "ad-server-mock-236.com",
            "tracker-analytics-236.net",
            "banner-ads-hosting-236.org",
            "ad-server-mock-237.com",
            "tracker-analytics-237.net",
            "banner-ads-hosting-237.org",
            "ad-server-mock-238.com",
            "tracker-analytics-238.net",
            "banner-ads-hosting-238.org",
            "ad-server-mock-239.com",
            "tracker-analytics-239.net",
            "banner-ads-hosting-239.org",
            "ad-server-mock-240.com",
            "tracker-analytics-240.net",
            "banner-ads-hosting-240.org",
            "ad-server-mock-241.com",
            "tracker-analytics-241.net",
            "banner-ads-hosting-241.org",
            "ad-server-mock-242.com",
            "tracker-analytics-242.net",
            "banner-ads-hosting-242.org",
            "ad-server-mock-243.com",
            "tracker-analytics-243.net",
            "banner-ads-hosting-243.org",
            "ad-server-mock-244.com",
            "tracker-analytics-244.net",
            "banner-ads-hosting-244.org",
            "ad-server-mock-245.com",
            "tracker-analytics-245.net",
            "banner-ads-hosting-245.org",
            "ad-server-mock-246.com",
            "tracker-analytics-246.net",
            "banner-ads-hosting-246.org",
            "ad-server-mock-247.com",
            "tracker-analytics-247.net",
            "banner-ads-hosting-247.org",
            "ad-server-mock-248.com",
            "tracker-analytics-248.net",
            "banner-ads-hosting-248.org",
            "ad-server-mock-249.com",
            "tracker-analytics-249.net",
            "banner-ads-hosting-249.org",
            "ad-server-mock-250.com",
            "tracker-analytics-250.net",
            "banner-ads-hosting-250.org",
            "ad-server-mock-251.com",
            "tracker-analytics-251.net",
            "banner-ads-hosting-251.org",
            "ad-server-mock-252.com",
            "tracker-analytics-252.net",
            "banner-ads-hosting-252.org",
            "ad-server-mock-253.com",
            "tracker-analytics-253.net",
            "banner-ads-hosting-253.org",
            "ad-server-mock-254.com",
            "tracker-analytics-254.net",
            "banner-ads-hosting-254.org",
            "ad-server-mock-255.com",
            "tracker-analytics-255.net",
            "banner-ads-hosting-255.org",
            "ad-server-mock-256.com",
            "tracker-analytics-256.net",
            "banner-ads-hosting-256.org",
            "ad-server-mock-257.com",
            "tracker-analytics-257.net",
            "banner-ads-hosting-257.org",
            "ad-server-mock-258.com",
            "tracker-analytics-258.net",
            "banner-ads-hosting-258.org",
            "ad-server-mock-259.com",
            "tracker-analytics-259.net",
            "banner-ads-hosting-259.org",
            "ad-server-mock-260.com",
            "tracker-analytics-260.net",
            "banner-ads-hosting-260.org",
            "ad-server-mock-261.com",
            "tracker-analytics-261.net",
            "banner-ads-hosting-261.org",
            "ad-server-mock-262.com",
            "tracker-analytics-262.net",
            "banner-ads-hosting-262.org",
            "ad-server-mock-263.com",
            "tracker-analytics-263.net",
            "banner-ads-hosting-263.org",
            "ad-server-mock-264.com",
            "tracker-analytics-264.net",
            "banner-ads-hosting-264.org",
            "ad-server-mock-265.com",
            "tracker-analytics-265.net",
            "banner-ads-hosting-265.org",
            "ad-server-mock-266.com",
            "tracker-analytics-266.net",
            "banner-ads-hosting-266.org",
            "ad-server-mock-267.com",
            "tracker-analytics-267.net",
            "banner-ads-hosting-267.org",
            "ad-server-mock-268.com",
            "tracker-analytics-268.net",
            "banner-ads-hosting-268.org",
            "ad-server-mock-269.com",
            "tracker-analytics-269.net",
            "banner-ads-hosting-269.org",
            "ad-server-mock-270.com",
            "tracker-analytics-270.net",
            "banner-ads-hosting-270.org",
            "ad-server-mock-271.com",
            "tracker-analytics-271.net",
            "banner-ads-hosting-271.org",
            "ad-server-mock-272.com",
            "tracker-analytics-272.net",
            "banner-ads-hosting-272.org",
            "ad-server-mock-273.com",
            "tracker-analytics-273.net",
            "banner-ads-hosting-273.org",
            "ad-server-mock-274.com",
            "tracker-analytics-274.net",
            "banner-ads-hosting-274.org",
            "ad-server-mock-275.com",
            "tracker-analytics-275.net",
            "banner-ads-hosting-275.org",
            "ad-server-mock-276.com",
            "tracker-analytics-276.net",
            "banner-ads-hosting-276.org",
            "ad-server-mock-277.com",
            "tracker-analytics-277.net",
            "banner-ads-hosting-277.org",
            "ad-server-mock-278.com",
            "tracker-analytics-278.net",
            "banner-ads-hosting-278.org",
            "ad-server-mock-279.com",
            "tracker-analytics-279.net",
            "banner-ads-hosting-279.org",
            "ad-server-mock-280.com",
            "tracker-analytics-280.net",
            "banner-ads-hosting-280.org",
            "ad-server-mock-281.com",
            "tracker-analytics-281.net",
            "banner-ads-hosting-281.org",
            "ad-server-mock-282.com",
            "tracker-analytics-282.net",
            "banner-ads-hosting-282.org",
            "ad-server-mock-283.com",
            "tracker-analytics-283.net",
            "banner-ads-hosting-283.org",
            "ad-server-mock-284.com",
            "tracker-analytics-284.net",
            "banner-ads-hosting-284.org",
            "ad-server-mock-285.com",
            "tracker-analytics-285.net",
            "banner-ads-hosting-285.org",
            "ad-server-mock-286.com",
            "tracker-analytics-286.net",
            "banner-ads-hosting-286.org",
            "ad-server-mock-287.com",
            "tracker-analytics-287.net",
            "banner-ads-hosting-287.org",
            "ad-server-mock-288.com",
            "tracker-analytics-288.net",
            "banner-ads-hosting-288.org",
            "ad-server-mock-289.com",
            "tracker-analytics-289.net",
            "banner-ads-hosting-289.org",
            "ad-server-mock-290.com",
            "tracker-analytics-290.net",
            "banner-ads-hosting-290.org",
            "ad-server-mock-291.com",
            "tracker-analytics-291.net",
            "banner-ads-hosting-291.org",
            "ad-server-mock-292.com",
            "tracker-analytics-292.net",
            "banner-ads-hosting-292.org",
            "ad-server-mock-293.com",
            "tracker-analytics-293.net",
            "banner-ads-hosting-293.org",
            "ad-server-mock-294.com",
            "tracker-analytics-294.net",
            "banner-ads-hosting-294.org",
            "ad-server-mock-295.com",
            "tracker-analytics-295.net",
            "banner-ads-hosting-295.org",
            "ad-server-mock-296.com",
            "tracker-analytics-296.net",
            "banner-ads-hosting-296.org",
            "ad-server-mock-297.com",
            "tracker-analytics-297.net",
            "banner-ads-hosting-297.org",
            "ad-server-mock-298.com",
            "tracker-analytics-298.net",
            "banner-ads-hosting-298.org",
            "ad-server-mock-299.com",
            "tracker-analytics-299.net",
            "banner-ads-hosting-299.org",
            "ad-server-mock-300.com",
            "tracker-analytics-300.net",
            "banner-ads-hosting-300.org",
            "ad-server-mock-301.com",
            "tracker-analytics-301.net",
            "banner-ads-hosting-301.org",
            "ad-server-mock-302.com",
            "tracker-analytics-302.net",
            "banner-ads-hosting-302.org",
            "ad-server-mock-303.com",
            "tracker-analytics-303.net",
            "banner-ads-hosting-303.org",
            "ad-server-mock-304.com",
            "tracker-analytics-304.net",
            "banner-ads-hosting-304.org",
            "ad-server-mock-305.com",
            "tracker-analytics-305.net",
            "banner-ads-hosting-305.org",
            "ad-server-mock-306.com",
            "tracker-analytics-306.net",
            "banner-ads-hosting-306.org",
            "ad-server-mock-307.com",
            "tracker-analytics-307.net",
            "banner-ads-hosting-307.org",
            "ad-server-mock-308.com",
            "tracker-analytics-308.net",
            "banner-ads-hosting-308.org",
            "ad-server-mock-309.com",
            "tracker-analytics-309.net",
            "banner-ads-hosting-309.org",
            "ad-server-mock-310.com",
            "tracker-analytics-310.net",
            "banner-ads-hosting-310.org",
            "ad-server-mock-311.com",
            "tracker-analytics-311.net",
            "banner-ads-hosting-311.org",
            "ad-server-mock-312.com",
            "tracker-analytics-312.net",
            "banner-ads-hosting-312.org",
            "ad-server-mock-313.com",
            "tracker-analytics-313.net",
            "banner-ads-hosting-313.org",
            "ad-server-mock-314.com",
            "tracker-analytics-314.net",
            "banner-ads-hosting-314.org",
            "ad-server-mock-315.com",
            "tracker-analytics-315.net",
            "banner-ads-hosting-315.org",
            "ad-server-mock-316.com",
            "tracker-analytics-316.net",
            "banner-ads-hosting-316.org",
            "ad-server-mock-317.com",
            "tracker-analytics-317.net",
            "banner-ads-hosting-317.org",
            "ad-server-mock-318.com",
            "tracker-analytics-318.net",
            "banner-ads-hosting-318.org",
            "ad-server-mock-319.com",
            "tracker-analytics-319.net",
            "banner-ads-hosting-319.org",
            "ad-server-mock-320.com",
            "tracker-analytics-320.net",
            "banner-ads-hosting-320.org",
            "ad-server-mock-321.com",
            "tracker-analytics-321.net",
            "banner-ads-hosting-321.org",
            "ad-server-mock-322.com",
            "tracker-analytics-322.net",
            "banner-ads-hosting-322.org",
            "ad-server-mock-323.com",
            "tracker-analytics-323.net",
            "banner-ads-hosting-323.org",
            "ad-server-mock-324.com",
            "tracker-analytics-324.net",
            "banner-ads-hosting-324.org",
            "ad-server-mock-325.com",
            "tracker-analytics-325.net",
            "banner-ads-hosting-325.org",
            "ad-server-mock-326.com",
            "tracker-analytics-326.net",
            "banner-ads-hosting-326.org",
            "ad-server-mock-327.com",
            "tracker-analytics-327.net",
            "banner-ads-hosting-327.org",
            "ad-server-mock-328.com",
            "tracker-analytics-328.net",
            "banner-ads-hosting-328.org",
            "ad-server-mock-329.com",
            "tracker-analytics-329.net",
            "banner-ads-hosting-329.org",
            "ad-server-mock-330.com",
            "tracker-analytics-330.net",
            "banner-ads-hosting-330.org",
            "ad-server-mock-331.com",
            "tracker-analytics-331.net",
            "banner-ads-hosting-331.org",
            "ad-server-mock-332.com",
            "tracker-analytics-332.net",
            "banner-ads-hosting-332.org",
            "ad-server-mock-333.com",
            "tracker-analytics-333.net",
            "banner-ads-hosting-333.org",
            "ad-server-mock-334.com",
            "tracker-analytics-334.net",
            "banner-ads-hosting-334.org",
            "ad-server-mock-335.com",
            "tracker-analytics-335.net",
            "banner-ads-hosting-335.org",
            "ad-server-mock-336.com",
            "tracker-analytics-336.net",
            "banner-ads-hosting-336.org",
            "ad-server-mock-337.com",
            "tracker-analytics-337.net",
            "banner-ads-hosting-337.org",
            "ad-server-mock-338.com",
            "tracker-analytics-338.net",
            "banner-ads-hosting-338.org",
            "ad-server-mock-339.com",
            "tracker-analytics-339.net",
            "banner-ads-hosting-339.org",
            "ad-server-mock-340.com",
            "tracker-analytics-340.net",
            "banner-ads-hosting-340.org",
            "ad-server-mock-341.com",
            "tracker-analytics-341.net",
            "banner-ads-hosting-341.org",
            "ad-server-mock-342.com",
            "tracker-analytics-342.net",
            "banner-ads-hosting-342.org",
            "ad-server-mock-343.com",
            "tracker-analytics-343.net",
            "banner-ads-hosting-343.org",
            "ad-server-mock-344.com",
            "tracker-analytics-344.net",
            "banner-ads-hosting-344.org",
            "ad-server-mock-345.com",
            "tracker-analytics-345.net",
            "banner-ads-hosting-345.org",
            "ad-server-mock-346.com",
            "tracker-analytics-346.net",
            "banner-ads-hosting-346.org",
            "ad-server-mock-347.com",
            "tracker-analytics-347.net",
            "banner-ads-hosting-347.org",
            "ad-server-mock-348.com",
            "tracker-analytics-348.net",
            "banner-ads-hosting-348.org",
            "ad-server-mock-349.com",
            "tracker-analytics-349.net",
            "banner-ads-hosting-349.org",
            "ad-server-mock-350.com",
            "tracker-analytics-350.net",
            "banner-ads-hosting-350.org",
            "ad-server-mock-351.com",
            "tracker-analytics-351.net",
            "banner-ads-hosting-351.org",
            "ad-server-mock-352.com",
            "tracker-analytics-352.net",
            "banner-ads-hosting-352.org",
            "ad-server-mock-353.com",
            "tracker-analytics-353.net",
            "banner-ads-hosting-353.org",
            "ad-server-mock-354.com",
            "tracker-analytics-354.net",
            "banner-ads-hosting-354.org",
            "ad-server-mock-355.com",
            "tracker-analytics-355.net",
            "banner-ads-hosting-355.org",
            "ad-server-mock-356.com",
            "tracker-analytics-356.net",
            "banner-ads-hosting-356.org",
            "ad-server-mock-357.com",
            "tracker-analytics-357.net",
            "banner-ads-hosting-357.org",
            "ad-server-mock-358.com",
            "tracker-analytics-358.net",
            "banner-ads-hosting-358.org",
            "ad-server-mock-359.com",
            "tracker-analytics-359.net",
            "banner-ads-hosting-359.org",
            "ad-server-mock-360.com",
            "tracker-analytics-360.net",
            "banner-ads-hosting-360.org",
            "ad-server-mock-361.com",
            "tracker-analytics-361.net",
            "banner-ads-hosting-361.org",
            "ad-server-mock-362.com",
            "tracker-analytics-362.net",
            "banner-ads-hosting-362.org",
            "ad-server-mock-363.com",
            "tracker-analytics-363.net",
            "banner-ads-hosting-363.org",
            "ad-server-mock-364.com",
            "tracker-analytics-364.net",
            "banner-ads-hosting-364.org",
            "ad-server-mock-365.com",
            "tracker-analytics-365.net",
            "banner-ads-hosting-365.org",
            "ad-server-mock-366.com",
            "tracker-analytics-366.net",
            "banner-ads-hosting-366.org",
            "ad-server-mock-367.com",
            "tracker-analytics-367.net",
            "banner-ads-hosting-367.org",
            "ad-server-mock-368.com",
            "tracker-analytics-368.net",
            "banner-ads-hosting-368.org",
            "ad-server-mock-369.com",
            "tracker-analytics-369.net",
            "banner-ads-hosting-369.org",
            "ad-server-mock-370.com",
            "tracker-analytics-370.net",
            "banner-ads-hosting-370.org",
            "ad-server-mock-371.com",
            "tracker-analytics-371.net",
            "banner-ads-hosting-371.org",
            "ad-server-mock-372.com",
            "tracker-analytics-372.net",
            "banner-ads-hosting-372.org",
            "ad-server-mock-373.com",
            "tracker-analytics-373.net",
            "banner-ads-hosting-373.org",
            "ad-server-mock-374.com",
            "tracker-analytics-374.net",
            "banner-ads-hosting-374.org",
            "ad-server-mock-375.com",
            "tracker-analytics-375.net",
            "banner-ads-hosting-375.org",
            "ad-server-mock-376.com",
            "tracker-analytics-376.net",
            "banner-ads-hosting-376.org",
            "ad-server-mock-377.com",
            "tracker-analytics-377.net",
            "banner-ads-hosting-377.org",
            "ad-server-mock-378.com",
            "tracker-analytics-378.net",
            "banner-ads-hosting-378.org",
            "ad-server-mock-379.com",
            "tracker-analytics-379.net",
            "banner-ads-hosting-379.org",
            "ad-server-mock-380.com",
            "tracker-analytics-380.net",
            "banner-ads-hosting-380.org",
            "ad-server-mock-381.com",
            "tracker-analytics-381.net",
            "banner-ads-hosting-381.org",
            "ad-server-mock-382.com",
            "tracker-analytics-382.net",
            "banner-ads-hosting-382.org",
            "ad-server-mock-383.com",
            "tracker-analytics-383.net",
            "banner-ads-hosting-383.org",
            "ad-server-mock-384.com",
            "tracker-analytics-384.net",
            "banner-ads-hosting-384.org",
            "ad-server-mock-385.com",
            "tracker-analytics-385.net",
            "banner-ads-hosting-385.org",
            "ad-server-mock-386.com",
            "tracker-analytics-386.net",
            "banner-ads-hosting-386.org",
            "ad-server-mock-387.com",
            "tracker-analytics-387.net",
            "banner-ads-hosting-387.org",
            "ad-server-mock-388.com",
            "tracker-analytics-388.net",
            "banner-ads-hosting-388.org",
            "ad-server-mock-389.com",
            "tracker-analytics-389.net",
            "banner-ads-hosting-389.org",
            "ad-server-mock-390.com",
            "tracker-analytics-390.net",
            "banner-ads-hosting-390.org",
            "ad-server-mock-391.com",
            "tracker-analytics-391.net",
            "banner-ads-hosting-391.org",
            "ad-server-mock-392.com",
            "tracker-analytics-392.net",
            "banner-ads-hosting-392.org",
            "ad-server-mock-393.com",
            "tracker-analytics-393.net",
            "banner-ads-hosting-393.org",
            "ad-server-mock-394.com",
            "tracker-analytics-394.net",
            "banner-ads-hosting-394.org",
            "ad-server-mock-395.com",
            "tracker-analytics-395.net",
            "banner-ads-hosting-395.org",
            "ad-server-mock-396.com",
            "tracker-analytics-396.net",
            "banner-ads-hosting-396.org",
            "ad-server-mock-397.com",
            "tracker-analytics-397.net",
            "banner-ads-hosting-397.org",
            "ad-server-mock-398.com",
            "tracker-analytics-398.net",
            "banner-ads-hosting-398.org",
            "ad-server-mock-399.com",
            "tracker-analytics-399.net",
            "banner-ads-hosting-399.org",
            "ad-server-mock-400.com",
            "tracker-analytics-400.net",
            "banner-ads-hosting-400.org",
            "ad-server-mock-401.com",
            "tracker-analytics-401.net",
            "banner-ads-hosting-401.org",
            "ad-server-mock-402.com",
            "tracker-analytics-402.net",
            "banner-ads-hosting-402.org",
            "ad-server-mock-403.com",
            "tracker-analytics-403.net",
            "banner-ads-hosting-403.org",
            "ad-server-mock-404.com",
            "tracker-analytics-404.net",
            "banner-ads-hosting-404.org",
            "ad-server-mock-405.com",
            "tracker-analytics-405.net",
            "banner-ads-hosting-405.org",
            "ad-server-mock-406.com",
            "tracker-analytics-406.net",
            "banner-ads-hosting-406.org",
            "ad-server-mock-407.com",
            "tracker-analytics-407.net",
            "banner-ads-hosting-407.org",
            "ad-server-mock-408.com",
            "tracker-analytics-408.net",
            "banner-ads-hosting-408.org",
            "ad-server-mock-409.com",
            "tracker-analytics-409.net",
            "banner-ads-hosting-409.org",
            "ad-server-mock-410.com",
            "tracker-analytics-410.net",
            "banner-ads-hosting-410.org",
            "ad-server-mock-411.com",
            "tracker-analytics-411.net",
            "banner-ads-hosting-411.org",
            "ad-server-mock-412.com",
            "tracker-analytics-412.net",
            "banner-ads-hosting-412.org",
            "ad-server-mock-413.com",
            "tracker-analytics-413.net",
            "banner-ads-hosting-413.org",
            "ad-server-mock-414.com",
            "tracker-analytics-414.net",
            "banner-ads-hosting-414.org",
            "ad-server-mock-415.com",
            "tracker-analytics-415.net",
            "banner-ads-hosting-415.org",
            "ad-server-mock-416.com",
            "tracker-analytics-416.net",
            "banner-ads-hosting-416.org",
            "ad-server-mock-417.com",
            "tracker-analytics-417.net",
            "banner-ads-hosting-417.org",
            "ad-server-mock-418.com",
            "tracker-analytics-418.net",
            "banner-ads-hosting-418.org",
            "ad-server-mock-419.com",
            "tracker-analytics-419.net",
            "banner-ads-hosting-419.org",
            "ad-server-mock-420.com",
            "tracker-analytics-420.net",
            "banner-ads-hosting-420.org",
            "ad-server-mock-421.com",
            "tracker-analytics-421.net",
            "banner-ads-hosting-421.org",
            "ad-server-mock-422.com",
            "tracker-analytics-422.net",
            "banner-ads-hosting-422.org",
            "ad-server-mock-423.com",
            "tracker-analytics-423.net",
            "banner-ads-hosting-423.org",
            "ad-server-mock-424.com",
            "tracker-analytics-424.net",
            "banner-ads-hosting-424.org",
            "ad-server-mock-425.com",
            "tracker-analytics-425.net",
            "banner-ads-hosting-425.org",
            "ad-server-mock-426.com",
            "tracker-analytics-426.net",
            "banner-ads-hosting-426.org",
            "ad-server-mock-427.com",
            "tracker-analytics-427.net",
            "banner-ads-hosting-427.org",
            "ad-server-mock-428.com",
            "tracker-analytics-428.net",
            "banner-ads-hosting-428.org",
            "ad-server-mock-429.com",
            "tracker-analytics-429.net",
            "banner-ads-hosting-429.org",
            "ad-server-mock-430.com",
            "tracker-analytics-430.net",
            "banner-ads-hosting-430.org",
            "ad-server-mock-431.com",
            "tracker-analytics-431.net",
            "banner-ads-hosting-431.org",
            "ad-server-mock-432.com",
            "tracker-analytics-432.net",
            "banner-ads-hosting-432.org",
            "ad-server-mock-433.com",
            "tracker-analytics-433.net",
            "banner-ads-hosting-433.org",
            "ad-server-mock-434.com",
            "tracker-analytics-434.net",
            "banner-ads-hosting-434.org",
            "ad-server-mock-435.com",
            "tracker-analytics-435.net",
            "banner-ads-hosting-435.org",
            "ad-server-mock-436.com",
            "tracker-analytics-436.net",
            "banner-ads-hosting-436.org",
            "ad-server-mock-437.com",
            "tracker-analytics-437.net",
            "banner-ads-hosting-437.org",
            "ad-server-mock-438.com",
            "tracker-analytics-438.net",
            "banner-ads-hosting-438.org",
            "ad-server-mock-439.com",
            "tracker-analytics-439.net",
            "banner-ads-hosting-439.org",
            "ad-server-mock-440.com",
            "tracker-analytics-440.net",
            "banner-ads-hosting-440.org",
            "ad-server-mock-441.com",
            "tracker-analytics-441.net",
            "banner-ads-hosting-441.org",
            "ad-server-mock-442.com",
            "tracker-analytics-442.net",
            "banner-ads-hosting-442.org",
            "ad-server-mock-443.com",
            "tracker-analytics-443.net",
            "banner-ads-hosting-443.org",
            "ad-server-mock-444.com",
            "tracker-analytics-444.net",
            "banner-ads-hosting-444.org",
            "ad-server-mock-445.com",
            "tracker-analytics-445.net",
            "banner-ads-hosting-445.org",
            "ad-server-mock-446.com",
            "tracker-analytics-446.net",
            "banner-ads-hosting-446.org",
            "ad-server-mock-447.com",
            "tracker-analytics-447.net",
            "banner-ads-hosting-447.org",
            "ad-server-mock-448.com",
            "tracker-analytics-448.net",
            "banner-ads-hosting-448.org",
            "ad-server-mock-449.com",
            "tracker-analytics-449.net",
            "banner-ads-hosting-449.org",
            "ad-server-mock-450.com",
            "tracker-analytics-450.net",
            "banner-ads-hosting-450.org",
            "ad-server-mock-451.com",
            "tracker-analytics-451.net",
            "banner-ads-hosting-451.org",
            "ad-server-mock-452.com",
            "tracker-analytics-452.net",
            "banner-ads-hosting-452.org",
            "ad-server-mock-453.com",
            "tracker-analytics-453.net",
            "banner-ads-hosting-453.org",
            "ad-server-mock-454.com",
            "tracker-analytics-454.net",
            "banner-ads-hosting-454.org",
            "ad-server-mock-455.com",
            "tracker-analytics-455.net",
            "banner-ads-hosting-455.org",
            "ad-server-mock-456.com",
            "tracker-analytics-456.net",
            "banner-ads-hosting-456.org",
            "ad-server-mock-457.com",
            "tracker-analytics-457.net",
            "banner-ads-hosting-457.org",
            "ad-server-mock-458.com",
            "tracker-analytics-458.net",
            "banner-ads-hosting-458.org",
            "ad-server-mock-459.com",
            "tracker-analytics-459.net",
            "banner-ads-hosting-459.org",
            "ad-server-mock-460.com",
            "tracker-analytics-460.net",
            "banner-ads-hosting-460.org",
            "ad-server-mock-461.com",
            "tracker-analytics-461.net",
            "banner-ads-hosting-461.org",
            "ad-server-mock-462.com",
            "tracker-analytics-462.net",
            "banner-ads-hosting-462.org",
            "ad-server-mock-463.com",
            "tracker-analytics-463.net",
            "banner-ads-hosting-463.org",
            "ad-server-mock-464.com",
            "tracker-analytics-464.net",
            "banner-ads-hosting-464.org",
            "ad-server-mock-465.com",
            "tracker-analytics-465.net",
            "banner-ads-hosting-465.org",
            "ad-server-mock-466.com",
            "tracker-analytics-466.net",
            "banner-ads-hosting-466.org",
            "ad-server-mock-467.com",
            "tracker-analytics-467.net",
            "banner-ads-hosting-467.org",
            "ad-server-mock-468.com",
            "tracker-analytics-468.net",
            "banner-ads-hosting-468.org",
            "ad-server-mock-469.com",
            "tracker-analytics-469.net",
            "banner-ads-hosting-469.org",
            "ad-server-mock-470.com",
            "tracker-analytics-470.net",
            "banner-ads-hosting-470.org",
            "ad-server-mock-471.com",
            "tracker-analytics-471.net",
            "banner-ads-hosting-471.org",
            "ad-server-mock-472.com",
            "tracker-analytics-472.net",
            "banner-ads-hosting-472.org",
            "ad-server-mock-473.com",
            "tracker-analytics-473.net",
            "banner-ads-hosting-473.org",
            "ad-server-mock-474.com",
            "tracker-analytics-474.net",
            "banner-ads-hosting-474.org",
            "ad-server-mock-475.com",
            "tracker-analytics-475.net",
            "banner-ads-hosting-475.org",
            "ad-server-mock-476.com",
            "tracker-analytics-476.net",
            "banner-ads-hosting-476.org",
            "ad-server-mock-477.com",
            "tracker-analytics-477.net",
            "banner-ads-hosting-477.org",
            "ad-server-mock-478.com",
            "tracker-analytics-478.net",
            "banner-ads-hosting-478.org",
            "ad-server-mock-479.com",
            "tracker-analytics-479.net",
            "banner-ads-hosting-479.org",
            "ad-server-mock-480.com",
            "tracker-analytics-480.net",
            "banner-ads-hosting-480.org",
            "ad-server-mock-481.com",
            "tracker-analytics-481.net",
            "banner-ads-hosting-481.org",
            "ad-server-mock-482.com",
            "tracker-analytics-482.net",
            "banner-ads-hosting-482.org",
            "ad-server-mock-483.com",
            "tracker-analytics-483.net",
            "banner-ads-hosting-483.org",
            "ad-server-mock-484.com",
            "tracker-analytics-484.net",
            "banner-ads-hosting-484.org",
            "ad-server-mock-485.com",
            "tracker-analytics-485.net",
            "banner-ads-hosting-485.org",
            "ad-server-mock-486.com",
            "tracker-analytics-486.net",
            "banner-ads-hosting-486.org",
            "ad-server-mock-487.com",
            "tracker-analytics-487.net",
            "banner-ads-hosting-487.org",
            "ad-server-mock-488.com",
            "tracker-analytics-488.net",
            "banner-ads-hosting-488.org",
            "ad-server-mock-489.com",
            "tracker-analytics-489.net",
            "banner-ads-hosting-489.org",
            "ad-server-mock-490.com",
            "tracker-analytics-490.net",
            "banner-ads-hosting-490.org",
            "ad-server-mock-491.com",
            "tracker-analytics-491.net",
            "banner-ads-hosting-491.org",
            "ad-server-mock-492.com",
            "tracker-analytics-492.net",
            "banner-ads-hosting-492.org",
            "ad-server-mock-493.com",
            "tracker-analytics-493.net",
            "banner-ads-hosting-493.org",
            "ad-server-mock-494.com",
            "tracker-analytics-494.net",
            "banner-ads-hosting-494.org",
            "ad-server-mock-495.com",
            "tracker-analytics-495.net",
            "banner-ads-hosting-495.org",
            "ad-server-mock-496.com",
            "tracker-analytics-496.net",
            "banner-ads-hosting-496.org",
            "ad-server-mock-497.com",
            "tracker-analytics-497.net",
            "banner-ads-hosting-497.org",
            "ad-server-mock-498.com",
            "tracker-analytics-498.net",
            "banner-ads-hosting-498.org",
            "ad-server-mock-499.com",
            "tracker-analytics-499.net",
            "banner-ads-hosting-499.org",
            "ad-server-mock-500.com",
            "tracker-analytics-500.net",
            "banner-ads-hosting-500.org",
            "ad-server-mock-501.com",
            "tracker-analytics-501.net",
            "banner-ads-hosting-501.org",
            "ad-server-mock-502.com",
            "tracker-analytics-502.net",
            "banner-ads-hosting-502.org",
            "ad-server-mock-503.com",
            "tracker-analytics-503.net",
            "banner-ads-hosting-503.org",
            "ad-server-mock-504.com",
            "tracker-analytics-504.net",
            "banner-ads-hosting-504.org",
            "ad-server-mock-505.com",
            "tracker-analytics-505.net",
            "banner-ads-hosting-505.org",
            "ad-server-mock-506.com",
            "tracker-analytics-506.net",
            "banner-ads-hosting-506.org",
            "ad-server-mock-507.com",
            "tracker-analytics-507.net",
            "banner-ads-hosting-507.org",
            "ad-server-mock-508.com",
            "tracker-analytics-508.net",
            "banner-ads-hosting-508.org",
            "ad-server-mock-509.com",
            "tracker-analytics-509.net",
            "banner-ads-hosting-509.org",
            "ad-server-mock-510.com",
            "tracker-analytics-510.net",
            "banner-ads-hosting-510.org",
            "ad-server-mock-511.com",
            "tracker-analytics-511.net",
            "banner-ads-hosting-511.org",
            "ad-server-mock-512.com",
            "tracker-analytics-512.net",
            "banner-ads-hosting-512.org",
            "ad-server-mock-513.com",
            "tracker-analytics-513.net",
            "banner-ads-hosting-513.org",
            "ad-server-mock-514.com",
            "tracker-analytics-514.net",
            "banner-ads-hosting-514.org",
            "ad-server-mock-515.com",
            "tracker-analytics-515.net",
            "banner-ads-hosting-515.org",
            "ad-server-mock-516.com",
            "tracker-analytics-516.net",
            "banner-ads-hosting-516.org",
            "ad-server-mock-517.com",
            "tracker-analytics-517.net",
            "banner-ads-hosting-517.org",
            "ad-server-mock-518.com",
            "tracker-analytics-518.net",
            "banner-ads-hosting-518.org",
            "ad-server-mock-519.com",
            "tracker-analytics-519.net",
            "banner-ads-hosting-519.org",
            "ad-server-mock-520.com",
            "tracker-analytics-520.net",
            "banner-ads-hosting-520.org",
            "ad-server-mock-521.com",
            "tracker-analytics-521.net",
            "banner-ads-hosting-521.org",
            "ad-server-mock-522.com",
            "tracker-analytics-522.net",
            "banner-ads-hosting-522.org",
            "ad-server-mock-523.com",
            "tracker-analytics-523.net",
            "banner-ads-hosting-523.org",
            "ad-server-mock-524.com",
            "tracker-analytics-524.net",
            "banner-ads-hosting-524.org",
            "ad-server-mock-525.com",
            "tracker-analytics-525.net",
            "banner-ads-hosting-525.org",
            "ad-server-mock-526.com",
            "tracker-analytics-526.net",
            "banner-ads-hosting-526.org",
            "ad-server-mock-527.com",
            "tracker-analytics-527.net",
            "banner-ads-hosting-527.org",
            "ad-server-mock-528.com",
            "tracker-analytics-528.net",
            "banner-ads-hosting-528.org",
            "ad-server-mock-529.com",
            "tracker-analytics-529.net",
            "banner-ads-hosting-529.org",
            "ad-server-mock-530.com",
            "tracker-analytics-530.net",
            "banner-ads-hosting-530.org",
            "ad-server-mock-531.com",
            "tracker-analytics-531.net",
            "banner-ads-hosting-531.org",
            "ad-server-mock-532.com",
            "tracker-analytics-532.net",
            "banner-ads-hosting-532.org",
            "ad-server-mock-533.com",
            "tracker-analytics-533.net",
            "banner-ads-hosting-533.org",
            "ad-server-mock-534.com",
            "tracker-analytics-534.net",
            "banner-ads-hosting-534.org",
            "ad-server-mock-535.com",
            "tracker-analytics-535.net",
            "banner-ads-hosting-535.org",
            "ad-server-mock-536.com",
            "tracker-analytics-536.net",
            "banner-ads-hosting-536.org",
            "ad-server-mock-537.com",
            "tracker-analytics-537.net",
            "banner-ads-hosting-537.org",
            "ad-server-mock-538.com",
            "tracker-analytics-538.net",
            "banner-ads-hosting-538.org",
            "ad-server-mock-539.com",
            "tracker-analytics-539.net",
            "banner-ads-hosting-539.org",
            "ad-server-mock-540.com",
            "tracker-analytics-540.net",
            "banner-ads-hosting-540.org",
            "ad-server-mock-541.com",
            "tracker-analytics-541.net",
            "banner-ads-hosting-541.org",
            "ad-server-mock-542.com",
            "tracker-analytics-542.net",
            "banner-ads-hosting-542.org",
            "ad-server-mock-543.com",
            "tracker-analytics-543.net",
            "banner-ads-hosting-543.org",
            "ad-server-mock-544.com",
            "tracker-analytics-544.net",
            "banner-ads-hosting-544.org",
            "ad-server-mock-545.com",
            "tracker-analytics-545.net",
            "banner-ads-hosting-545.org",
            "ad-server-mock-546.com",
            "tracker-analytics-546.net",
            "banner-ads-hosting-546.org",
            "ad-server-mock-547.com",
            "tracker-analytics-547.net",
            "banner-ads-hosting-547.org",
            "ad-server-mock-548.com",
            "tracker-analytics-548.net",
            "banner-ads-hosting-548.org",
            "ad-server-mock-549.com",
            "tracker-analytics-549.net",
            "banner-ads-hosting-549.org",
            "ad-server-mock-550.com",
            "tracker-analytics-550.net",
            "banner-ads-hosting-550.org",
            "ad-server-mock-551.com",
            "tracker-analytics-551.net",
            "banner-ads-hosting-551.org",
            "ad-server-mock-552.com",
            "tracker-analytics-552.net",
            "banner-ads-hosting-552.org",
            "ad-server-mock-553.com",
            "tracker-analytics-553.net",
            "banner-ads-hosting-553.org",
            "ad-server-mock-554.com",
            "tracker-analytics-554.net",
            "banner-ads-hosting-554.org",
            "ad-server-mock-555.com",
            "tracker-analytics-555.net",
            "banner-ads-hosting-555.org",
            "ad-server-mock-556.com",
            "tracker-analytics-556.net",
            "banner-ads-hosting-556.org",
            "ad-server-mock-557.com",
            "tracker-analytics-557.net",
            "banner-ads-hosting-557.org",
            "ad-server-mock-558.com",
            "tracker-analytics-558.net",
            "banner-ads-hosting-558.org",
            "ad-server-mock-559.com",
            "tracker-analytics-559.net",
            "banner-ads-hosting-559.org",
            "ad-server-mock-560.com",
            "tracker-analytics-560.net",
            "banner-ads-hosting-560.org",
            "ad-server-mock-561.com",
            "tracker-analytics-561.net",
            "banner-ads-hosting-561.org",
            "ad-server-mock-562.com",
            "tracker-analytics-562.net",
            "banner-ads-hosting-562.org",
            "ad-server-mock-563.com",
            "tracker-analytics-563.net",
            "banner-ads-hosting-563.org",
            "ad-server-mock-564.com",
            "tracker-analytics-564.net",
            "banner-ads-hosting-564.org",
            "ad-server-mock-565.com",
            "tracker-analytics-565.net",
            "banner-ads-hosting-565.org",
            "ad-server-mock-566.com",
            "tracker-analytics-566.net",
            "banner-ads-hosting-566.org",
            "ad-server-mock-567.com",
            "tracker-analytics-567.net",
            "banner-ads-hosting-567.org",
            "ad-server-mock-568.com",
            "tracker-analytics-568.net",
            "banner-ads-hosting-568.org",
            "ad-server-mock-569.com",
            "tracker-analytics-569.net",
            "banner-ads-hosting-569.org",
            "ad-server-mock-570.com",
            "tracker-analytics-570.net",
            "banner-ads-hosting-570.org",
            "ad-server-mock-571.com",
            "tracker-analytics-571.net",
            "banner-ads-hosting-571.org",
            "ad-server-mock-572.com",
            "tracker-analytics-572.net",
            "banner-ads-hosting-572.org",
            "ad-server-mock-573.com",
            "tracker-analytics-573.net",
            "banner-ads-hosting-573.org",
            "ad-server-mock-574.com",
            "tracker-analytics-574.net",
            "banner-ads-hosting-574.org",
            "ad-server-mock-575.com",
            "tracker-analytics-575.net",
            "banner-ads-hosting-575.org",
            "ad-server-mock-576.com",
            "tracker-analytics-576.net",
            "banner-ads-hosting-576.org",
            "ad-server-mock-577.com",
            "tracker-analytics-577.net",
            "banner-ads-hosting-577.org",
            "ad-server-mock-578.com",
            "tracker-analytics-578.net",
            "banner-ads-hosting-578.org",
            "ad-server-mock-579.com",
            "tracker-analytics-579.net",
            "banner-ads-hosting-579.org",
            "ad-server-mock-580.com",
            "tracker-analytics-580.net",
            "banner-ads-hosting-580.org",
            "ad-server-mock-581.com",
            "tracker-analytics-581.net",
            "banner-ads-hosting-581.org",
            "ad-server-mock-582.com",
            "tracker-analytics-582.net",
            "banner-ads-hosting-582.org",
            "ad-server-mock-583.com",
            "tracker-analytics-583.net",
            "banner-ads-hosting-583.org",
            "ad-server-mock-584.com",
            "tracker-analytics-584.net",
            "banner-ads-hosting-584.org",
            "ad-server-mock-585.com",
            "tracker-analytics-585.net",
            "banner-ads-hosting-585.org",
            "ad-server-mock-586.com",
            "tracker-analytics-586.net",
            "banner-ads-hosting-586.org",
            "ad-server-mock-587.com",
            "tracker-analytics-587.net",
            "banner-ads-hosting-587.org",
            "ad-server-mock-588.com",
            "tracker-analytics-588.net",
            "banner-ads-hosting-588.org",
            "ad-server-mock-589.com",
            "tracker-analytics-589.net",
            "banner-ads-hosting-589.org",
            "ad-server-mock-590.com",
            "tracker-analytics-590.net",
            "banner-ads-hosting-590.org",
            "ad-server-mock-591.com",
            "tracker-analytics-591.net",
            "banner-ads-hosting-591.org",
            "ad-server-mock-592.com",
            "tracker-analytics-592.net",
            "banner-ads-hosting-592.org",
            "ad-server-mock-593.com",
            "tracker-analytics-593.net",
            "banner-ads-hosting-593.org",
            "ad-server-mock-594.com",
            "tracker-analytics-594.net",
            "banner-ads-hosting-594.org",
            "ad-server-mock-595.com",
            "tracker-analytics-595.net",
            "banner-ads-hosting-595.org",
            "ad-server-mock-596.com",
            "tracker-analytics-596.net",
            "banner-ads-hosting-596.org",
            "ad-server-mock-597.com",
            "tracker-analytics-597.net",
            "banner-ads-hosting-597.org",
            "ad-server-mock-598.com",
            "tracker-analytics-598.net",
            "banner-ads-hosting-598.org",
            "ad-server-mock-599.com",
            "tracker-analytics-599.net",
            "banner-ads-hosting-599.org",
            "ad-server-mock-600.com",
            "tracker-analytics-600.net",
            "banner-ads-hosting-600.org",
            "ad-server-mock-601.com",
            "tracker-analytics-601.net",
            "banner-ads-hosting-601.org",
            "ad-server-mock-602.com",
            "tracker-analytics-602.net",
            "banner-ads-hosting-602.org",
            "ad-server-mock-603.com",
            "tracker-analytics-603.net",
            "banner-ads-hosting-603.org",
            "ad-server-mock-604.com",
            "tracker-analytics-604.net",
            "banner-ads-hosting-604.org",
            "ad-server-mock-605.com",
            "tracker-analytics-605.net",
            "banner-ads-hosting-605.org",
            "ad-server-mock-606.com",
            "tracker-analytics-606.net",
            "banner-ads-hosting-606.org",
            "ad-server-mock-607.com",
            "tracker-analytics-607.net",
            "banner-ads-hosting-607.org",
            "ad-server-mock-608.com",
            "tracker-analytics-608.net",
            "banner-ads-hosting-608.org",
            "ad-server-mock-609.com",
            "tracker-analytics-609.net",
            "banner-ads-hosting-609.org",
            "ad-server-mock-610.com",
            "tracker-analytics-610.net",
            "banner-ads-hosting-610.org",
            "ad-server-mock-611.com",
            "tracker-analytics-611.net",
            "banner-ads-hosting-611.org",
            "ad-server-mock-612.com",
            "tracker-analytics-612.net",
            "banner-ads-hosting-612.org",
            "ad-server-mock-613.com",
            "tracker-analytics-613.net",
            "banner-ads-hosting-613.org",
            "ad-server-mock-614.com",
            "tracker-analytics-614.net",
            "banner-ads-hosting-614.org",
            "ad-server-mock-615.com",
            "tracker-analytics-615.net",
            "banner-ads-hosting-615.org",
            "ad-server-mock-616.com",
            "tracker-analytics-616.net",
            "banner-ads-hosting-616.org",
            "ad-server-mock-617.com",
            "tracker-analytics-617.net",
            "banner-ads-hosting-617.org",
            "ad-server-mock-618.com",
            "tracker-analytics-618.net",
            "banner-ads-hosting-618.org",
            "ad-server-mock-619.com",
            "tracker-analytics-619.net",
            "banner-ads-hosting-619.org",
            "ad-server-mock-620.com",
            "tracker-analytics-620.net",
            "banner-ads-hosting-620.org",
            "ad-server-mock-621.com",
            "tracker-analytics-621.net",
            "banner-ads-hosting-621.org",
            "ad-server-mock-622.com",
            "tracker-analytics-622.net",
            "banner-ads-hosting-622.org",
            "ad-server-mock-623.com",
            "tracker-analytics-623.net",
            "banner-ads-hosting-623.org",
            "ad-server-mock-624.com",
            "tracker-analytics-624.net",
            "banner-ads-hosting-624.org",
            "ad-server-mock-625.com",
            "tracker-analytics-625.net",
            "banner-ads-hosting-625.org",
            "ad-server-mock-626.com",
            "tracker-analytics-626.net",
            "banner-ads-hosting-626.org",
            "ad-server-mock-627.com",
            "tracker-analytics-627.net",
            "banner-ads-hosting-627.org",
            "ad-server-mock-628.com",
            "tracker-analytics-628.net",
            "banner-ads-hosting-628.org",
            "ad-server-mock-629.com",
            "tracker-analytics-629.net",
            "banner-ads-hosting-629.org",
            "ad-server-mock-630.com",
            "tracker-analytics-630.net",
            "banner-ads-hosting-630.org",
            "ad-server-mock-631.com",
            "tracker-analytics-631.net",
            "banner-ads-hosting-631.org",
            "ad-server-mock-632.com",
            "tracker-analytics-632.net",
            "banner-ads-hosting-632.org",
            "ad-server-mock-633.com",
            "tracker-analytics-633.net",
            "banner-ads-hosting-633.org",
            "ad-server-mock-634.com",
            "tracker-analytics-634.net",
            "banner-ads-hosting-634.org",
            "ad-server-mock-635.com",
            "tracker-analytics-635.net",
            "banner-ads-hosting-635.org",
            "ad-server-mock-636.com",
            "tracker-analytics-636.net",
            "banner-ads-hosting-636.org",
            "ad-server-mock-637.com",
            "tracker-analytics-637.net",
            "banner-ads-hosting-637.org",
            "ad-server-mock-638.com",
            "tracker-analytics-638.net",
            "banner-ads-hosting-638.org",
            "ad-server-mock-639.com",
            "tracker-analytics-639.net",
            "banner-ads-hosting-639.org",
            "ad-server-mock-640.com",
            "tracker-analytics-640.net",
            "banner-ads-hosting-640.org",
            "ad-server-mock-641.com",
            "tracker-analytics-641.net",
            "banner-ads-hosting-641.org",
            "ad-server-mock-642.com",
            "tracker-analytics-642.net",
            "banner-ads-hosting-642.org",
            "ad-server-mock-643.com",
            "tracker-analytics-643.net",
            "banner-ads-hosting-643.org",
            "ad-server-mock-644.com",
            "tracker-analytics-644.net",
            "banner-ads-hosting-644.org",
            "ad-server-mock-645.com",
            "tracker-analytics-645.net",
            "banner-ads-hosting-645.org",
            "ad-server-mock-646.com",
            "tracker-analytics-646.net",
            "banner-ads-hosting-646.org",
            "ad-server-mock-647.com",
            "tracker-analytics-647.net",
            "banner-ads-hosting-647.org",
            "ad-server-mock-648.com",
            "tracker-analytics-648.net",
            "banner-ads-hosting-648.org",
            "ad-server-mock-649.com",
            "tracker-analytics-649.net",
            "banner-ads-hosting-649.org",
            "ad-server-mock-650.com",
            "tracker-analytics-650.net",
            "banner-ads-hosting-650.org",
            "ad-server-mock-651.com",
            "tracker-analytics-651.net",
            "banner-ads-hosting-651.org",
            "ad-server-mock-652.com",
            "tracker-analytics-652.net",
            "banner-ads-hosting-652.org",
            "ad-server-mock-653.com",
            "tracker-analytics-653.net",
            "banner-ads-hosting-653.org",
            "ad-server-mock-654.com",
            "tracker-analytics-654.net",
            "banner-ads-hosting-654.org",
            "ad-server-mock-655.com",
            "tracker-analytics-655.net",
            "banner-ads-hosting-655.org",
            "ad-server-mock-656.com",
            "tracker-analytics-656.net",
            "banner-ads-hosting-656.org",
            "ad-server-mock-657.com",
            "tracker-analytics-657.net",
            "banner-ads-hosting-657.org",
            "ad-server-mock-658.com",
            "tracker-analytics-658.net",
            "banner-ads-hosting-658.org",
            "ad-server-mock-659.com",
            "tracker-analytics-659.net",
            "banner-ads-hosting-659.org",
            "ad-server-mock-660.com",
            "tracker-analytics-660.net",
            "banner-ads-hosting-660.org",
            "ad-server-mock-661.com",
            "tracker-analytics-661.net",
            "banner-ads-hosting-661.org",
            "ad-server-mock-662.com",
            "tracker-analytics-662.net",
            "banner-ads-hosting-662.org",
            "ad-server-mock-663.com",
            "tracker-analytics-663.net",
            "banner-ads-hosting-663.org",
            "ad-server-mock-664.com",
            "tracker-analytics-664.net",
            "banner-ads-hosting-664.org",
            "ad-server-mock-665.com",
            "tracker-analytics-665.net",
            "banner-ads-hosting-665.org",
            "ad-server-mock-666.com",
            "tracker-analytics-666.net",
            "banner-ads-hosting-666.org",
            "ad-server-mock-667.com",
            "tracker-analytics-667.net",
            "banner-ads-hosting-667.org",
            "ad-server-mock-668.com",
            "tracker-analytics-668.net",
            "banner-ads-hosting-668.org",
            "ad-server-mock-669.com",
            "tracker-analytics-669.net",
            "banner-ads-hosting-669.org",
            "ad-server-mock-670.com",
            "tracker-analytics-670.net",
            "banner-ads-hosting-670.org",
            "ad-server-mock-671.com",
            "tracker-analytics-671.net",
            "banner-ads-hosting-671.org",
            "ad-server-mock-672.com",
            "tracker-analytics-672.net",
            "banner-ads-hosting-672.org",
            "ad-server-mock-673.com",
            "tracker-analytics-673.net",
            "banner-ads-hosting-673.org",
            "ad-server-mock-674.com",
            "tracker-analytics-674.net",
            "banner-ads-hosting-674.org",
            "ad-server-mock-675.com",
            "tracker-analytics-675.net",
            "banner-ads-hosting-675.org",
            "ad-server-mock-676.com",
            "tracker-analytics-676.net",
            "banner-ads-hosting-676.org",
            "ad-server-mock-677.com",
            "tracker-analytics-677.net",
            "banner-ads-hosting-677.org",
            "ad-server-mock-678.com",
            "tracker-analytics-678.net",
            "banner-ads-hosting-678.org",
            "ad-server-mock-679.com",
            "tracker-analytics-679.net",
            "banner-ads-hosting-679.org",
            "ad-server-mock-680.com",
            "tracker-analytics-680.net",
            "banner-ads-hosting-680.org",
            "ad-server-mock-681.com",
            "tracker-analytics-681.net",
            "banner-ads-hosting-681.org",
            "ad-server-mock-682.com",
            "tracker-analytics-682.net",
            "banner-ads-hosting-682.org",
            "ad-server-mock-683.com",
            "tracker-analytics-683.net",
            "banner-ads-hosting-683.org",
            "ad-server-mock-684.com",
            "tracker-analytics-684.net",
            "banner-ads-hosting-684.org",
            "ad-server-mock-685.com",
            "tracker-analytics-685.net",
            "banner-ads-hosting-685.org",
            "ad-server-mock-686.com",
            "tracker-analytics-686.net",
            "banner-ads-hosting-686.org",
            "ad-server-mock-687.com",
            "tracker-analytics-687.net",
            "banner-ads-hosting-687.org",
            "ad-server-mock-688.com",
            "tracker-analytics-688.net",
            "banner-ads-hosting-688.org",
            "ad-server-mock-689.com",
            "tracker-analytics-689.net",
            "banner-ads-hosting-689.org",
            "ad-server-mock-690.com",
            "tracker-analytics-690.net",
            "banner-ads-hosting-690.org",
            "ad-server-mock-691.com",
            "tracker-analytics-691.net",
            "banner-ads-hosting-691.org",
            "ad-server-mock-692.com",
            "tracker-analytics-692.net",
            "banner-ads-hosting-692.org",
            "ad-server-mock-693.com",
            "tracker-analytics-693.net",
            "banner-ads-hosting-693.org",
            "ad-server-mock-694.com",
            "tracker-analytics-694.net",
            "banner-ads-hosting-694.org",
            "ad-server-mock-695.com",
            "tracker-analytics-695.net",
            "banner-ads-hosting-695.org",
            "ad-server-mock-696.com",
            "tracker-analytics-696.net",
            "banner-ads-hosting-696.org",
            "ad-server-mock-697.com",
            "tracker-analytics-697.net",
            "banner-ads-hosting-697.org",
            "ad-server-mock-698.com",
            "tracker-analytics-698.net",
            "banner-ads-hosting-698.org",
            "ad-server-mock-699.com",
            "tracker-analytics-699.net",
            "banner-ads-hosting-699.org",
            "ad-server-mock-700.com",
            "tracker-analytics-700.net",
            "banner-ads-hosting-700.org",
            "ad-server-mock-701.com",
            "tracker-analytics-701.net",
            "banner-ads-hosting-701.org",
            "ad-server-mock-702.com",
            "tracker-analytics-702.net",
            "banner-ads-hosting-702.org",
            "ad-server-mock-703.com",
            "tracker-analytics-703.net",
            "banner-ads-hosting-703.org",
            "ad-server-mock-704.com",
            "tracker-analytics-704.net",
            "banner-ads-hosting-704.org",
            "ad-server-mock-705.com",
            "tracker-analytics-705.net",
            "banner-ads-hosting-705.org",
            "ad-server-mock-706.com",
            "tracker-analytics-706.net",
            "banner-ads-hosting-706.org",
            "ad-server-mock-707.com",
            "tracker-analytics-707.net",
            "banner-ads-hosting-707.org",
            "ad-server-mock-708.com",
            "tracker-analytics-708.net",
            "banner-ads-hosting-708.org",
            "ad-server-mock-709.com",
            "tracker-analytics-709.net",
            "banner-ads-hosting-709.org",
            "ad-server-mock-710.com",
            "tracker-analytics-710.net",
            "banner-ads-hosting-710.org",
            "ad-server-mock-711.com",
            "tracker-analytics-711.net",
            "banner-ads-hosting-711.org",
            "ad-server-mock-712.com",
            "tracker-analytics-712.net",
            "banner-ads-hosting-712.org",
            "ad-server-mock-713.com",
            "tracker-analytics-713.net",
            "banner-ads-hosting-713.org",
            "ad-server-mock-714.com",
            "tracker-analytics-714.net",
            "banner-ads-hosting-714.org",
            "ad-server-mock-715.com",
            "tracker-analytics-715.net",
            "banner-ads-hosting-715.org",
            "ad-server-mock-716.com",
            "tracker-analytics-716.net",
            "banner-ads-hosting-716.org",
            "ad-server-mock-717.com",
            "tracker-analytics-717.net",
            "banner-ads-hosting-717.org",
            "ad-server-mock-718.com",
            "tracker-analytics-718.net",
            "banner-ads-hosting-718.org",
            "ad-server-mock-719.com",
            "tracker-analytics-719.net",
            "banner-ads-hosting-719.org",
            "ad-server-mock-720.com",
            "tracker-analytics-720.net",
            "banner-ads-hosting-720.org",
            "ad-server-mock-721.com",
            "tracker-analytics-721.net",
            "banner-ads-hosting-721.org",
            "ad-server-mock-722.com",
            "tracker-analytics-722.net",
            "banner-ads-hosting-722.org",
            "ad-server-mock-723.com",
            "tracker-analytics-723.net",
            "banner-ads-hosting-723.org",
            "ad-server-mock-724.com",
            "tracker-analytics-724.net",
            "banner-ads-hosting-724.org",
            "ad-server-mock-725.com",
            "tracker-analytics-725.net",
            "banner-ads-hosting-725.org",
            "ad-server-mock-726.com",
            "tracker-analytics-726.net",
            "banner-ads-hosting-726.org",
            "ad-server-mock-727.com",
            "tracker-analytics-727.net",
            "banner-ads-hosting-727.org",
            "ad-server-mock-728.com",
            "tracker-analytics-728.net",
            "banner-ads-hosting-728.org",
            "ad-server-mock-729.com",
            "tracker-analytics-729.net",
            "banner-ads-hosting-729.org",
            "ad-server-mock-730.com",
            "tracker-analytics-730.net",
            "banner-ads-hosting-730.org",
            "ad-server-mock-731.com",
            "tracker-analytics-731.net",
            "banner-ads-hosting-731.org",
            "ad-server-mock-732.com",
            "tracker-analytics-732.net",
            "banner-ads-hosting-732.org",
            "ad-server-mock-733.com",
            "tracker-analytics-733.net",
            "banner-ads-hosting-733.org",
            "ad-server-mock-734.com",
            "tracker-analytics-734.net",
            "banner-ads-hosting-734.org",
            "ad-server-mock-735.com",
            "tracker-analytics-735.net",
            "banner-ads-hosting-735.org",
            "ad-server-mock-736.com",
            "tracker-analytics-736.net",
            "banner-ads-hosting-736.org",
            "ad-server-mock-737.com",
            "tracker-analytics-737.net",
            "banner-ads-hosting-737.org",
            "ad-server-mock-738.com",
            "tracker-analytics-738.net",
            "banner-ads-hosting-738.org",
            "ad-server-mock-739.com",
            "tracker-analytics-739.net",
            "banner-ads-hosting-739.org",
            "ad-server-mock-740.com",
            "tracker-analytics-740.net",
            "banner-ads-hosting-740.org",
            "ad-server-mock-741.com",
            "tracker-analytics-741.net",
            "banner-ads-hosting-741.org",
            "ad-server-mock-742.com",
            "tracker-analytics-742.net",
            "banner-ads-hosting-742.org",
            "ad-server-mock-743.com",
            "tracker-analytics-743.net",
            "banner-ads-hosting-743.org",
            "ad-server-mock-744.com",
            "tracker-analytics-744.net",
            "banner-ads-hosting-744.org",
            "ad-server-mock-745.com",
            "tracker-analytics-745.net",
            "banner-ads-hosting-745.org",
            "ad-server-mock-746.com",
            "tracker-analytics-746.net",
            "banner-ads-hosting-746.org",
            "ad-server-mock-747.com",
            "tracker-analytics-747.net",
            "banner-ads-hosting-747.org",
            "ad-server-mock-748.com",
            "tracker-analytics-748.net",
            "banner-ads-hosting-748.org",
            "ad-server-mock-749.com",
            "tracker-analytics-749.net",
            "banner-ads-hosting-749.org",
            "ad-server-mock-750.com",
            "tracker-analytics-750.net",
            "banner-ads-hosting-750.org",
            "ad-server-mock-751.com",
            "tracker-analytics-751.net",
            "banner-ads-hosting-751.org",
            "ad-server-mock-752.com",
            "tracker-analytics-752.net",
            "banner-ads-hosting-752.org",
            "ad-server-mock-753.com",
            "tracker-analytics-753.net",
            "banner-ads-hosting-753.org",
            "ad-server-mock-754.com",
            "tracker-analytics-754.net",
            "banner-ads-hosting-754.org",
            "ad-server-mock-755.com",
            "tracker-analytics-755.net",
            "banner-ads-hosting-755.org",
            "ad-server-mock-756.com",
            "tracker-analytics-756.net",
            "banner-ads-hosting-756.org",
            "ad-server-mock-757.com",
            "tracker-analytics-757.net",
            "banner-ads-hosting-757.org",
            "ad-server-mock-758.com",
            "tracker-analytics-758.net",
            "banner-ads-hosting-758.org",
            "ad-server-mock-759.com",
            "tracker-analytics-759.net",
            "banner-ads-hosting-759.org",
            "ad-server-mock-760.com",
            "tracker-analytics-760.net",
            "banner-ads-hosting-760.org",
            "ad-server-mock-761.com",
            "tracker-analytics-761.net",
            "banner-ads-hosting-761.org",
            "ad-server-mock-762.com",
            "tracker-analytics-762.net",
            "banner-ads-hosting-762.org",
            "ad-server-mock-763.com",
            "tracker-analytics-763.net",
            "banner-ads-hosting-763.org",
            "ad-server-mock-764.com",
            "tracker-analytics-764.net",
            "banner-ads-hosting-764.org",
            "ad-server-mock-765.com",
            "tracker-analytics-765.net",
            "banner-ads-hosting-765.org",
            "ad-server-mock-766.com",
            "tracker-analytics-766.net",
            "banner-ads-hosting-766.org",
            "ad-server-mock-767.com",
            "tracker-analytics-767.net",
            "banner-ads-hosting-767.org",
            "ad-server-mock-768.com",
            "tracker-analytics-768.net",
            "banner-ads-hosting-768.org",
            "ad-server-mock-769.com",
            "tracker-analytics-769.net",
            "banner-ads-hosting-769.org",
            "ad-server-mock-770.com",
            "tracker-analytics-770.net",
            "banner-ads-hosting-770.org",
            "ad-server-mock-771.com",
            "tracker-analytics-771.net",
            "banner-ads-hosting-771.org",
            "ad-server-mock-772.com",
            "tracker-analytics-772.net",
            "banner-ads-hosting-772.org",
            "ad-server-mock-773.com",
            "tracker-analytics-773.net",
            "banner-ads-hosting-773.org",
            "ad-server-mock-774.com",
            "tracker-analytics-774.net",
            "banner-ads-hosting-774.org",
            "ad-server-mock-775.com",
            "tracker-analytics-775.net",
            "banner-ads-hosting-775.org",
            "ad-server-mock-776.com",
            "tracker-analytics-776.net",
            "banner-ads-hosting-776.org",
            "ad-server-mock-777.com",
            "tracker-analytics-777.net",
            "banner-ads-hosting-777.org",
            "ad-server-mock-778.com",
            "tracker-analytics-778.net",
            "banner-ads-hosting-778.org",
            "ad-server-mock-779.com",
            "tracker-analytics-779.net",
            "banner-ads-hosting-779.org",
            "ad-server-mock-780.com",
            "tracker-analytics-780.net",
            "banner-ads-hosting-780.org",
            "ad-server-mock-781.com",
            "tracker-analytics-781.net",
            "banner-ads-hosting-781.org",
            "ad-server-mock-782.com",
            "tracker-analytics-782.net",
            "banner-ads-hosting-782.org",
            "ad-server-mock-783.com",
            "tracker-analytics-783.net",
            "banner-ads-hosting-783.org",
            "ad-server-mock-784.com",
            "tracker-analytics-784.net",
            "banner-ads-hosting-784.org",
            "ad-server-mock-785.com",
            "tracker-analytics-785.net",
            "banner-ads-hosting-785.org",
            "ad-server-mock-786.com",
            "tracker-analytics-786.net",
            "banner-ads-hosting-786.org",
            "ad-server-mock-787.com",
            "tracker-analytics-787.net",
            "banner-ads-hosting-787.org",
            "ad-server-mock-788.com",
            "tracker-analytics-788.net",
            "banner-ads-hosting-788.org",
            "ad-server-mock-789.com",
            "tracker-analytics-789.net",
            "banner-ads-hosting-789.org",
            "ad-server-mock-790.com",
            "tracker-analytics-790.net",
            "banner-ads-hosting-790.org",
            "ad-server-mock-791.com",
            "tracker-analytics-791.net",
            "banner-ads-hosting-791.org",
            "ad-server-mock-792.com",
            "tracker-analytics-792.net",
            "banner-ads-hosting-792.org",
            "ad-server-mock-793.com",
            "tracker-analytics-793.net",
            "banner-ads-hosting-793.org",
            "ad-server-mock-794.com",
            "tracker-analytics-794.net",
            "banner-ads-hosting-794.org",
            "ad-server-mock-795.com",
            "tracker-analytics-795.net",
            "banner-ads-hosting-795.org",
            "ad-server-mock-796.com",
            "tracker-analytics-796.net",
            "banner-ads-hosting-796.org",
            "ad-server-mock-797.com",
            "tracker-analytics-797.net",
            "banner-ads-hosting-797.org",
            "ad-server-mock-798.com",
            "tracker-analytics-798.net",
            "banner-ads-hosting-798.org",
            "ad-server-mock-799.com",
            "tracker-analytics-799.net",
            "banner-ads-hosting-799.org",
            "ad-server-mock-800.com",
            "tracker-analytics-800.net",
            "banner-ads-hosting-800.org",
            "ad-server-mock-801.com",
            "tracker-analytics-801.net",
            "banner-ads-hosting-801.org",
            "ad-server-mock-802.com",
            "tracker-analytics-802.net",
            "banner-ads-hosting-802.org",
            "ad-server-mock-803.com",
            "tracker-analytics-803.net",
            "banner-ads-hosting-803.org",
            "ad-server-mock-804.com",
            "tracker-analytics-804.net",
            "banner-ads-hosting-804.org",
            "ad-server-mock-805.com",
            "tracker-analytics-805.net",
            "banner-ads-hosting-805.org",
            "ad-server-mock-806.com",
            "tracker-analytics-806.net",
            "banner-ads-hosting-806.org",
            "ad-server-mock-807.com",
            "tracker-analytics-807.net",
            "banner-ads-hosting-807.org",
            "ad-server-mock-808.com",
            "tracker-analytics-808.net",
            "banner-ads-hosting-808.org",
            "ad-server-mock-809.com",
            "tracker-analytics-809.net",
            "banner-ads-hosting-809.org",
            "ad-server-mock-810.com",
            "tracker-analytics-810.net",
            "banner-ads-hosting-810.org",
            "ad-server-mock-811.com",
            "tracker-analytics-811.net",
            "banner-ads-hosting-811.org",
            "ad-server-mock-812.com",
            "tracker-analytics-812.net",
            "banner-ads-hosting-812.org",
            "ad-server-mock-813.com",
            "tracker-analytics-813.net",
            "banner-ads-hosting-813.org",
            "ad-server-mock-814.com",
            "tracker-analytics-814.net",
            "banner-ads-hosting-814.org",
            "ad-server-mock-815.com",
            "tracker-analytics-815.net",
            "banner-ads-hosting-815.org",
            "ad-server-mock-816.com",
            "tracker-analytics-816.net",
            "banner-ads-hosting-816.org",
            "ad-server-mock-817.com",
            "tracker-analytics-817.net",
            "banner-ads-hosting-817.org",
            "ad-server-mock-818.com",
            "tracker-analytics-818.net",
            "banner-ads-hosting-818.org",
            "ad-server-mock-819.com",
            "tracker-analytics-819.net",
            "banner-ads-hosting-819.org",
            "ad-server-mock-820.com",
            "tracker-analytics-820.net",
            "banner-ads-hosting-820.org",
            "ad-server-mock-821.com",
            "tracker-analytics-821.net",
            "banner-ads-hosting-821.org",
            "ad-server-mock-822.com",
            "tracker-analytics-822.net",
            "banner-ads-hosting-822.org",
            "ad-server-mock-823.com",
            "tracker-analytics-823.net",
            "banner-ads-hosting-823.org",
            "ad-server-mock-824.com",
            "tracker-analytics-824.net",
            "banner-ads-hosting-824.org",
            "ad-server-mock-825.com",
            "tracker-analytics-825.net",
            "banner-ads-hosting-825.org",
            "ad-server-mock-826.com",
            "tracker-analytics-826.net",
            "banner-ads-hosting-826.org",
            "ad-server-mock-827.com",
            "tracker-analytics-827.net",
            "banner-ads-hosting-827.org",
            "ad-server-mock-828.com",
            "tracker-analytics-828.net",
            "banner-ads-hosting-828.org",
            "ad-server-mock-829.com",
            "tracker-analytics-829.net",
            "banner-ads-hosting-829.org",
            "ad-server-mock-830.com",
            "tracker-analytics-830.net",
            "banner-ads-hosting-830.org",
            "ad-server-mock-831.com",
            "tracker-analytics-831.net",
            "banner-ads-hosting-831.org",
            "ad-server-mock-832.com",
            "tracker-analytics-832.net",
            "banner-ads-hosting-832.org",
            "ad-server-mock-833.com",
            "tracker-analytics-833.net",
            "banner-ads-hosting-833.org",
            "ad-server-mock-834.com",
            "tracker-analytics-834.net",
            "banner-ads-hosting-834.org",
            "ad-server-mock-835.com",
            "tracker-analytics-835.net",
            "banner-ads-hosting-835.org",
            "ad-server-mock-836.com",
            "tracker-analytics-836.net",
            "banner-ads-hosting-836.org",
            "ad-server-mock-837.com",
            "tracker-analytics-837.net",
            "banner-ads-hosting-837.org",
            "ad-server-mock-838.com",
            "tracker-analytics-838.net",
            "banner-ads-hosting-838.org",
            "ad-server-mock-839.com",
            "tracker-analytics-839.net",
            "banner-ads-hosting-839.org",
            "ad-server-mock-840.com",
            "tracker-analytics-840.net",
            "banner-ads-hosting-840.org",
            "ad-server-mock-841.com",
            "tracker-analytics-841.net",
            "banner-ads-hosting-841.org",
            "ad-server-mock-842.com",
            "tracker-analytics-842.net",
            "banner-ads-hosting-842.org",
            "ad-server-mock-843.com",
            "tracker-analytics-843.net",
            "banner-ads-hosting-843.org",
            "ad-server-mock-844.com",
            "tracker-analytics-844.net",
            "banner-ads-hosting-844.org",
            "ad-server-mock-845.com",
            "tracker-analytics-845.net",
            "banner-ads-hosting-845.org",
            "ad-server-mock-846.com",
            "tracker-analytics-846.net",
            "banner-ads-hosting-846.org",
            "ad-server-mock-847.com",
            "tracker-analytics-847.net",
            "banner-ads-hosting-847.org",
            "ad-server-mock-848.com",
            "tracker-analytics-848.net",
            "banner-ads-hosting-848.org",
            "ad-server-mock-849.com",
            "tracker-analytics-849.net",
            "banner-ads-hosting-849.org",
            "ad-server-mock-850.com",
            "tracker-analytics-850.net",
            "banner-ads-hosting-850.org",
            "ad-server-mock-851.com",
            "tracker-analytics-851.net",
            "banner-ads-hosting-851.org",
            "ad-server-mock-852.com",
            "tracker-analytics-852.net",
            "banner-ads-hosting-852.org",
            "ad-server-mock-853.com",
            "tracker-analytics-853.net",
            "banner-ads-hosting-853.org",
            "ad-server-mock-854.com",
            "tracker-analytics-854.net",
            "banner-ads-hosting-854.org",
            "ad-server-mock-855.com",
            "tracker-analytics-855.net",
            "banner-ads-hosting-855.org",
            "ad-server-mock-856.com",
            "tracker-analytics-856.net",
            "banner-ads-hosting-856.org",
            "ad-server-mock-857.com",
            "tracker-analytics-857.net",
            "banner-ads-hosting-857.org",
            "ad-server-mock-858.com",
            "tracker-analytics-858.net",
            "banner-ads-hosting-858.org",
            "ad-server-mock-859.com",
            "tracker-analytics-859.net",
            "banner-ads-hosting-859.org",
            "ad-server-mock-860.com",
            "tracker-analytics-860.net",
            "banner-ads-hosting-860.org",
            "ad-server-mock-861.com",
            "tracker-analytics-861.net",
            "banner-ads-hosting-861.org",
            "ad-server-mock-862.com",
            "tracker-analytics-862.net",
            "banner-ads-hosting-862.org",
            "ad-server-mock-863.com",
            "tracker-analytics-863.net",
            "banner-ads-hosting-863.org",
            "ad-server-mock-864.com",
            "tracker-analytics-864.net",
            "banner-ads-hosting-864.org",
            "ad-server-mock-865.com",
            "tracker-analytics-865.net",
            "banner-ads-hosting-865.org",
            "ad-server-mock-866.com",
            "tracker-analytics-866.net",
            "banner-ads-hosting-866.org",
            "ad-server-mock-867.com",
            "tracker-analytics-867.net",
            "banner-ads-hosting-867.org",
            "ad-server-mock-868.com",
            "tracker-analytics-868.net",
            "banner-ads-hosting-868.org",
            "ad-server-mock-869.com",
            "tracker-analytics-869.net",
            "banner-ads-hosting-869.org",
            "ad-server-mock-870.com",
            "tracker-analytics-870.net",
            "banner-ads-hosting-870.org",
            "ad-server-mock-871.com",
            "tracker-analytics-871.net",
            "banner-ads-hosting-871.org",
            "ad-server-mock-872.com",
            "tracker-analytics-872.net",
            "banner-ads-hosting-872.org",
            "ad-server-mock-873.com",
            "tracker-analytics-873.net",
            "banner-ads-hosting-873.org",
            "ad-server-mock-874.com",
            "tracker-analytics-874.net",
            "banner-ads-hosting-874.org",
            "ad-server-mock-875.com",
            "tracker-analytics-875.net",
            "banner-ads-hosting-875.org",
            "ad-server-mock-876.com",
            "tracker-analytics-876.net",
            "banner-ads-hosting-876.org",
            "ad-server-mock-877.com",
            "tracker-analytics-877.net",
            "banner-ads-hosting-877.org",
            "ad-server-mock-878.com",
            "tracker-analytics-878.net",
            "banner-ads-hosting-878.org",
            "ad-server-mock-879.com",
            "tracker-analytics-879.net",
            "banner-ads-hosting-879.org",
            "ad-server-mock-880.com",
            "tracker-analytics-880.net",
            "banner-ads-hosting-880.org",
            "ad-server-mock-881.com",
            "tracker-analytics-881.net",
            "banner-ads-hosting-881.org",
            "ad-server-mock-882.com",
            "tracker-analytics-882.net",
            "banner-ads-hosting-882.org",
            "ad-server-mock-883.com",
            "tracker-analytics-883.net",
            "banner-ads-hosting-883.org",
            "ad-server-mock-884.com",
            "tracker-analytics-884.net",
            "banner-ads-hosting-884.org",
            "ad-server-mock-885.com",
            "tracker-analytics-885.net",
            "banner-ads-hosting-885.org",
            "ad-server-mock-886.com",
            "tracker-analytics-886.net",
            "banner-ads-hosting-886.org",
            "ad-server-mock-887.com",
            "tracker-analytics-887.net",
            "banner-ads-hosting-887.org",
            "ad-server-mock-888.com",
            "tracker-analytics-888.net",
            "banner-ads-hosting-888.org",
            "ad-server-mock-889.com",
            "tracker-analytics-889.net",
            "banner-ads-hosting-889.org",
            "ad-server-mock-890.com",
            "tracker-analytics-890.net",
            "banner-ads-hosting-890.org",
            "ad-server-mock-891.com",
            "tracker-analytics-891.net",
            "banner-ads-hosting-891.org",
            "ad-server-mock-892.com",
            "tracker-analytics-892.net",
            "banner-ads-hosting-892.org",
            "ad-server-mock-893.com",
            "tracker-analytics-893.net",
            "banner-ads-hosting-893.org",
            "ad-server-mock-894.com",
            "tracker-analytics-894.net",
            "banner-ads-hosting-894.org",
            "ad-server-mock-895.com",
            "tracker-analytics-895.net",
            "banner-ads-hosting-895.org",
            "ad-server-mock-896.com",
            "tracker-analytics-896.net",
            "banner-ads-hosting-896.org",
            "ad-server-mock-897.com",
            "tracker-analytics-897.net",
            "banner-ads-hosting-897.org",
            "ad-server-mock-898.com",
            "tracker-analytics-898.net",
            "banner-ads-hosting-898.org",
            "ad-server-mock-899.com",
            "tracker-analytics-899.net",
            "banner-ads-hosting-899.org",
            "ad-server-mock-900.com",
            "tracker-analytics-900.net",
            "banner-ads-hosting-900.org",
            "ad-server-mock-901.com",
            "tracker-analytics-901.net",
            "banner-ads-hosting-901.org",
            "ad-server-mock-902.com",
            "tracker-analytics-902.net",
            "banner-ads-hosting-902.org",
            "ad-server-mock-903.com",
            "tracker-analytics-903.net",
            "banner-ads-hosting-903.org",
            "ad-server-mock-904.com",
            "tracker-analytics-904.net",
            "banner-ads-hosting-904.org",
            "ad-server-mock-905.com",
            "tracker-analytics-905.net",
            "banner-ads-hosting-905.org",
            "ad-server-mock-906.com",
            "tracker-analytics-906.net",
            "banner-ads-hosting-906.org",
            "ad-server-mock-907.com",
            "tracker-analytics-907.net",
            "banner-ads-hosting-907.org",
            "ad-server-mock-908.com",
            "tracker-analytics-908.net",
            "banner-ads-hosting-908.org",
            "ad-server-mock-909.com",
            "tracker-analytics-909.net",
            "banner-ads-hosting-909.org",
            "ad-server-mock-910.com",
            "tracker-analytics-910.net",
            "banner-ads-hosting-910.org",
            "ad-server-mock-911.com",
            "tracker-analytics-911.net",
            "banner-ads-hosting-911.org",
            "ad-server-mock-912.com",
            "tracker-analytics-912.net",
            "banner-ads-hosting-912.org",
            "ad-server-mock-913.com",
            "tracker-analytics-913.net",
            "banner-ads-hosting-913.org",
            "ad-server-mock-914.com",
            "tracker-analytics-914.net",
            "banner-ads-hosting-914.org",
            "ad-server-mock-915.com",
            "tracker-analytics-915.net",
            "banner-ads-hosting-915.org",
            "ad-server-mock-916.com",
            "tracker-analytics-916.net",
            "banner-ads-hosting-916.org",
            "ad-server-mock-917.com",
            "tracker-analytics-917.net",
            "banner-ads-hosting-917.org",
            "ad-server-mock-918.com",
            "tracker-analytics-918.net",
            "banner-ads-hosting-918.org",
            "ad-server-mock-919.com",
            "tracker-analytics-919.net",
            "banner-ads-hosting-919.org",
            "ad-server-mock-920.com",
            "tracker-analytics-920.net",
            "banner-ads-hosting-920.org",
            "ad-server-mock-921.com",
            "tracker-analytics-921.net",
            "banner-ads-hosting-921.org",
            "ad-server-mock-922.com",
            "tracker-analytics-922.net",
            "banner-ads-hosting-922.org",
            "ad-server-mock-923.com",
            "tracker-analytics-923.net",
            "banner-ads-hosting-923.org",
            "ad-server-mock-924.com",
            "tracker-analytics-924.net",
            "banner-ads-hosting-924.org",
            "ad-server-mock-925.com",
            "tracker-analytics-925.net",
            "banner-ads-hosting-925.org",
            "ad-server-mock-926.com",
            "tracker-analytics-926.net",
            "banner-ads-hosting-926.org",
            "ad-server-mock-927.com",
            "tracker-analytics-927.net",
            "banner-ads-hosting-927.org",
            "ad-server-mock-928.com",
            "tracker-analytics-928.net",
            "banner-ads-hosting-928.org",
            "ad-server-mock-929.com",
            "tracker-analytics-929.net",
            "banner-ads-hosting-929.org",
            "ad-server-mock-930.com",
            "tracker-analytics-930.net",
            "banner-ads-hosting-930.org",
            "ad-server-mock-931.com",
            "tracker-analytics-931.net",
            "banner-ads-hosting-931.org",
            "ad-server-mock-932.com",
            "tracker-analytics-932.net",
            "banner-ads-hosting-932.org",
            "ad-server-mock-933.com",
            "tracker-analytics-933.net",
            "banner-ads-hosting-933.org",
            "ad-server-mock-934.com",
            "tracker-analytics-934.net",
            "banner-ads-hosting-934.org",
            "ad-server-mock-935.com",
            "tracker-analytics-935.net",
            "banner-ads-hosting-935.org",
            "ad-server-mock-936.com",
            "tracker-analytics-936.net",
            "banner-ads-hosting-936.org",
            "ad-server-mock-937.com",
            "tracker-analytics-937.net",
            "banner-ads-hosting-937.org",
            "ad-server-mock-938.com",
            "tracker-analytics-938.net",
            "banner-ads-hosting-938.org",
            "ad-server-mock-939.com",
            "tracker-analytics-939.net",
            "banner-ads-hosting-939.org",
            "ad-server-mock-940.com",
            "tracker-analytics-940.net",
            "banner-ads-hosting-940.org",
            "ad-server-mock-941.com",
            "tracker-analytics-941.net",
            "banner-ads-hosting-941.org",
            "ad-server-mock-942.com",
            "tracker-analytics-942.net",
            "banner-ads-hosting-942.org",
            "ad-server-mock-943.com",
            "tracker-analytics-943.net",
            "banner-ads-hosting-943.org",
            "ad-server-mock-944.com",
            "tracker-analytics-944.net",
            "banner-ads-hosting-944.org",
            "ad-server-mock-945.com",
            "tracker-analytics-945.net",
            "banner-ads-hosting-945.org",
            "ad-server-mock-946.com",
            "tracker-analytics-946.net",
            "banner-ads-hosting-946.org",
            "ad-server-mock-947.com",
            "tracker-analytics-947.net",
            "banner-ads-hosting-947.org",
            "ad-server-mock-948.com",
            "tracker-analytics-948.net",
            "banner-ads-hosting-948.org",
            "ad-server-mock-949.com",
            "tracker-analytics-949.net",
            "banner-ads-hosting-949.org",
            "ad-server-mock-950.com",
            "tracker-analytics-950.net",
            "banner-ads-hosting-950.org",
            "ad-server-mock-951.com",
            "tracker-analytics-951.net",
            "banner-ads-hosting-951.org",
            "ad-server-mock-952.com",
            "tracker-analytics-952.net",
            "banner-ads-hosting-952.org",
            "ad-server-mock-953.com",
            "tracker-analytics-953.net",
            "banner-ads-hosting-953.org",
            "ad-server-mock-954.com",
            "tracker-analytics-954.net",
            "banner-ads-hosting-954.org",
            "ad-server-mock-955.com",
            "tracker-analytics-955.net",
            "banner-ads-hosting-955.org",
            "ad-server-mock-956.com",
            "tracker-analytics-956.net",
            "banner-ads-hosting-956.org",
            "ad-server-mock-957.com",
            "tracker-analytics-957.net",
            "banner-ads-hosting-957.org",
            "ad-server-mock-958.com",
            "tracker-analytics-958.net",
            "banner-ads-hosting-958.org",
            "ad-server-mock-959.com",
            "tracker-analytics-959.net",
            "banner-ads-hosting-959.org",
            "ad-server-mock-960.com",
            "tracker-analytics-960.net",
            "banner-ads-hosting-960.org",
            "ad-server-mock-961.com",
            "tracker-analytics-961.net",
            "banner-ads-hosting-961.org",
            "ad-server-mock-962.com",
            "tracker-analytics-962.net",
            "banner-ads-hosting-962.org",
            "ad-server-mock-963.com",
            "tracker-analytics-963.net",
            "banner-ads-hosting-963.org",
            "ad-server-mock-964.com",
            "tracker-analytics-964.net",
            "banner-ads-hosting-964.org",
            "ad-server-mock-965.com",
            "tracker-analytics-965.net",
            "banner-ads-hosting-965.org",
            "ad-server-mock-966.com",
            "tracker-analytics-966.net",
            "banner-ads-hosting-966.org",
            "ad-server-mock-967.com",
            "tracker-analytics-967.net",
            "banner-ads-hosting-967.org",
            "ad-server-mock-968.com",
            "tracker-analytics-968.net",
            "banner-ads-hosting-968.org",
            "ad-server-mock-969.com",
            "tracker-analytics-969.net",
            "banner-ads-hosting-969.org",
            "ad-server-mock-970.com",
            "tracker-analytics-970.net",
            "banner-ads-hosting-970.org",
            "ad-server-mock-971.com",
            "tracker-analytics-971.net",
            "banner-ads-hosting-971.org",
            "ad-server-mock-972.com",
            "tracker-analytics-972.net",
            "banner-ads-hosting-972.org",
            "ad-server-mock-973.com",
            "tracker-analytics-973.net",
            "banner-ads-hosting-973.org",
            "ad-server-mock-974.com",
            "tracker-analytics-974.net",
            "banner-ads-hosting-974.org",
            "ad-server-mock-975.com",
            "tracker-analytics-975.net",
            "banner-ads-hosting-975.org",
            "ad-server-mock-976.com",
            "tracker-analytics-976.net",
            "banner-ads-hosting-976.org",
            "ad-server-mock-977.com",
            "tracker-analytics-977.net",
            "banner-ads-hosting-977.org",
            "ad-server-mock-978.com",
            "tracker-analytics-978.net",
            "banner-ads-hosting-978.org",
            "ad-server-mock-979.com",
            "tracker-analytics-979.net",
            "banner-ads-hosting-979.org",
            "ad-server-mock-980.com",
            "tracker-analytics-980.net",
            "banner-ads-hosting-980.org",
            "ad-server-mock-981.com",
            "tracker-analytics-981.net",
            "banner-ads-hosting-981.org",
            "ad-server-mock-982.com",
            "tracker-analytics-982.net",
            "banner-ads-hosting-982.org",
            "ad-server-mock-983.com",
            "tracker-analytics-983.net",
            "banner-ads-hosting-983.org",
            "ad-server-mock-984.com",
            "tracker-analytics-984.net",
            "banner-ads-hosting-984.org",
            "ad-server-mock-985.com",
            "tracker-analytics-985.net",
            "banner-ads-hosting-985.org",
            "ad-server-mock-986.com",
            "tracker-analytics-986.net",
            "banner-ads-hosting-986.org",
            "ad-server-mock-987.com",
            "tracker-analytics-987.net",
            "banner-ads-hosting-987.org",
            "ad-server-mock-988.com",
            "tracker-analytics-988.net",
            "banner-ads-hosting-988.org",
            "ad-server-mock-989.com",
            "tracker-analytics-989.net",
            "banner-ads-hosting-989.org",
            "ad-server-mock-990.com",
            "tracker-analytics-990.net",
            "banner-ads-hosting-990.org",
            "ad-server-mock-991.com",
            "tracker-analytics-991.net",
            "banner-ads-hosting-991.org",
            "ad-server-mock-992.com",
            "tracker-analytics-992.net",
            "banner-ads-hosting-992.org",
            "ad-server-mock-993.com",
            "tracker-analytics-993.net",
            "banner-ads-hosting-993.org",
            "ad-server-mock-994.com",
            "tracker-analytics-994.net",
            "banner-ads-hosting-994.org",
            "ad-server-mock-995.com",
            "tracker-analytics-995.net",
            "banner-ads-hosting-995.org",
            "ad-server-mock-996.com",
            "tracker-analytics-996.net",
            "banner-ads-hosting-996.org",
            "ad-server-mock-997.com",
            "tracker-analytics-997.net",
            "banner-ads-hosting-997.org",
            "ad-server-mock-998.com",
            "tracker-analytics-998.net",
            "banner-ads-hosting-998.org",
            "ad-server-mock-999.com",
            "tracker-analytics-999.net",
            "banner-ads-hosting-999.org",
            "ad-server-mock-1000.com",
            "tracker-analytics-1000.net",
            "banner-ads-hosting-1000.org",
            "ad-server-mock-1001.com",
            "tracker-analytics-1001.net",
            "banner-ads-hosting-1001.org",
            "ad-server-mock-1002.com",
            "tracker-analytics-1002.net",
            "banner-ads-hosting-1002.org",
            "ad-server-mock-1003.com",
            "tracker-analytics-1003.net",
            "banner-ads-hosting-1003.org",
            "ad-server-mock-1004.com",
            "tracker-analytics-1004.net",
            "banner-ads-hosting-1004.org",
            "ad-server-mock-1005.com",
            "tracker-analytics-1005.net",
            "banner-ads-hosting-1005.org",
            "ad-server-mock-1006.com",
            "tracker-analytics-1006.net",
            "banner-ads-hosting-1006.org",
            "ad-server-mock-1007.com",
            "tracker-analytics-1007.net",
            "banner-ads-hosting-1007.org",
            "ad-server-mock-1008.com",
            "tracker-analytics-1008.net",
            "banner-ads-hosting-1008.org",
            "ad-server-mock-1009.com",
            "tracker-analytics-1009.net",
            "banner-ads-hosting-1009.org",
            "ad-server-mock-1010.com",
            "tracker-analytics-1010.net",
            "banner-ads-hosting-1010.org",
            "ad-server-mock-1011.com",
            "tracker-analytics-1011.net",
            "banner-ads-hosting-1011.org",
            "ad-server-mock-1012.com",
            "tracker-analytics-1012.net",
            "banner-ads-hosting-1012.org",
            "ad-server-mock-1013.com",
            "tracker-analytics-1013.net",
            "banner-ads-hosting-1013.org",
            "ad-server-mock-1014.com",
            "tracker-analytics-1014.net",
            "banner-ads-hosting-1014.org",
            "ad-server-mock-1015.com",
            "tracker-analytics-1015.net",
            "banner-ads-hosting-1015.org",
            "ad-server-mock-1016.com",
            "tracker-analytics-1016.net",
            "banner-ads-hosting-1016.org",
            "ad-server-mock-1017.com",
            "tracker-analytics-1017.net",
            "banner-ads-hosting-1017.org",
            "ad-server-mock-1018.com",
            "tracker-analytics-1018.net",
            "banner-ads-hosting-1018.org",
            "ad-server-mock-1019.com",
            "tracker-analytics-1019.net",
            "banner-ads-hosting-1019.org",
            "ad-server-mock-1020.com",
            "tracker-analytics-1020.net",
            "banner-ads-hosting-1020.org",
            "ad-server-mock-1021.com",
            "tracker-analytics-1021.net",
            "banner-ads-hosting-1021.org",
            "ad-server-mock-1022.com",
            "tracker-analytics-1022.net",
            "banner-ads-hosting-1022.org",
            "ad-server-mock-1023.com",
            "tracker-analytics-1023.net",
            "banner-ads-hosting-1023.org",
            "ad-server-mock-1024.com",
            "tracker-analytics-1024.net",
            "banner-ads-hosting-1024.org",
            "ad-server-mock-1025.com",
            "tracker-analytics-1025.net",
            "banner-ads-hosting-1025.org",
            "ad-server-mock-1026.com",
            "tracker-analytics-1026.net",
            "banner-ads-hosting-1026.org",
            "ad-server-mock-1027.com",
            "tracker-analytics-1027.net",
            "banner-ads-hosting-1027.org",
            "ad-server-mock-1028.com",
            "tracker-analytics-1028.net",
            "banner-ads-hosting-1028.org",
            "ad-server-mock-1029.com",
            "tracker-analytics-1029.net",
            "banner-ads-hosting-1029.org",
            "ad-server-mock-1030.com",
            "tracker-analytics-1030.net",
            "banner-ads-hosting-1030.org",
            "ad-server-mock-1031.com",
            "tracker-analytics-1031.net",
            "banner-ads-hosting-1031.org",
            "ad-server-mock-1032.com",
            "tracker-analytics-1032.net",
            "banner-ads-hosting-1032.org",
            "ad-server-mock-1033.com",
            "tracker-analytics-1033.net",
            "banner-ads-hosting-1033.org",
            "ad-server-mock-1034.com",
            "tracker-analytics-1034.net",
            "banner-ads-hosting-1034.org",
            "ad-server-mock-1035.com",
            "tracker-analytics-1035.net",
            "banner-ads-hosting-1035.org",
            "ad-server-mock-1036.com",
            "tracker-analytics-1036.net",
            "banner-ads-hosting-1036.org",
            "ad-server-mock-1037.com",
            "tracker-analytics-1037.net",
            "banner-ads-hosting-1037.org",
            "ad-server-mock-1038.com",
            "tracker-analytics-1038.net",
            "banner-ads-hosting-1038.org",
            "ad-server-mock-1039.com",
            "tracker-analytics-1039.net",
            "banner-ads-hosting-1039.org",
            "ad-server-mock-1040.com",
            "tracker-analytics-1040.net",
            "banner-ads-hosting-1040.org",
            "ad-server-mock-1041.com",
            "tracker-analytics-1041.net",
            "banner-ads-hosting-1041.org",
            "ad-server-mock-1042.com",
            "tracker-analytics-1042.net",
            "banner-ads-hosting-1042.org",
            "ad-server-mock-1043.com",
            "tracker-analytics-1043.net",
            "banner-ads-hosting-1043.org",
            "ad-server-mock-1044.com",
            "tracker-analytics-1044.net",
            "banner-ads-hosting-1044.org",
            "ad-server-mock-1045.com",
            "tracker-analytics-1045.net",
            "banner-ads-hosting-1045.org",
            "ad-server-mock-1046.com",
            "tracker-analytics-1046.net",
            "banner-ads-hosting-1046.org",
            "ad-server-mock-1047.com",
            "tracker-analytics-1047.net",
            "banner-ads-hosting-1047.org",
            "ad-server-mock-1048.com",
            "tracker-analytics-1048.net",
            "banner-ads-hosting-1048.org",
            "ad-server-mock-1049.com",
            "tracker-analytics-1049.net",
            "banner-ads-hosting-1049.org",
            "ad-server-mock-1050.com",
            "tracker-analytics-1050.net",
            "banner-ads-hosting-1050.org",
            "ad-server-mock-1051.com",
            "tracker-analytics-1051.net",
            "banner-ads-hosting-1051.org",
            "ad-server-mock-1052.com",
            "tracker-analytics-1052.net",
            "banner-ads-hosting-1052.org",
            "ad-server-mock-1053.com",
            "tracker-analytics-1053.net",
            "banner-ads-hosting-1053.org",
            "ad-server-mock-1054.com",
            "tracker-analytics-1054.net",
            "banner-ads-hosting-1054.org",
            "ad-server-mock-1055.com",
            "tracker-analytics-1055.net",
            "banner-ads-hosting-1055.org",
            "ad-server-mock-1056.com",
            "tracker-analytics-1056.net",
            "banner-ads-hosting-1056.org",
            "ad-server-mock-1057.com",
            "tracker-analytics-1057.net",
            "banner-ads-hosting-1057.org",
            "ad-server-mock-1058.com",
            "tracker-analytics-1058.net",
            "banner-ads-hosting-1058.org",
            "ad-server-mock-1059.com",
            "tracker-analytics-1059.net",
            "banner-ads-hosting-1059.org",
            "ad-server-mock-1060.com",
            "tracker-analytics-1060.net",
            "banner-ads-hosting-1060.org",
            "ad-server-mock-1061.com",
            "tracker-analytics-1061.net",
            "banner-ads-hosting-1061.org",
            "ad-server-mock-1062.com",
            "tracker-analytics-1062.net",
            "banner-ads-hosting-1062.org",
            "ad-server-mock-1063.com",
            "tracker-analytics-1063.net",
            "banner-ads-hosting-1063.org",
            "ad-server-mock-1064.com",
            "tracker-analytics-1064.net",
            "banner-ads-hosting-1064.org",
            "ad-server-mock-1065.com",
            "tracker-analytics-1065.net",
            "banner-ads-hosting-1065.org",
            "ad-server-mock-1066.com",
            "tracker-analytics-1066.net",
            "banner-ads-hosting-1066.org",
            "ad-server-mock-1067.com",
            "tracker-analytics-1067.net",
            "banner-ads-hosting-1067.org",
            "ad-server-mock-1068.com",
            "tracker-analytics-1068.net",
            "banner-ads-hosting-1068.org",
            "ad-server-mock-1069.com",
            "tracker-analytics-1069.net",
            "banner-ads-hosting-1069.org",
            "ad-server-mock-1070.com",
            "tracker-analytics-1070.net",
            "banner-ads-hosting-1070.org",
            "ad-server-mock-1071.com",
            "tracker-analytics-1071.net",
            "banner-ads-hosting-1071.org",
            "ad-server-mock-1072.com",
            "tracker-analytics-1072.net",
            "banner-ads-hosting-1072.org",
            "ad-server-mock-1073.com",
            "tracker-analytics-1073.net",
            "banner-ads-hosting-1073.org",
            "ad-server-mock-1074.com",
            "tracker-analytics-1074.net",
            "banner-ads-hosting-1074.org",
            "ad-server-mock-1075.com",
            "tracker-analytics-1075.net",
            "banner-ads-hosting-1075.org",
            "ad-server-mock-1076.com",
            "tracker-analytics-1076.net",
            "banner-ads-hosting-1076.org",
            "ad-server-mock-1077.com",
            "tracker-analytics-1077.net",
            "banner-ads-hosting-1077.org",
            "ad-server-mock-1078.com",
            "tracker-analytics-1078.net",
            "banner-ads-hosting-1078.org",
            "ad-server-mock-1079.com",
            "tracker-analytics-1079.net",
            "banner-ads-hosting-1079.org",
            "ad-server-mock-1080.com",
            "tracker-analytics-1080.net",
            "banner-ads-hosting-1080.org",
            "ad-server-mock-1081.com",
            "tracker-analytics-1081.net",
            "banner-ads-hosting-1081.org",
            "ad-server-mock-1082.com",
            "tracker-analytics-1082.net",
            "banner-ads-hosting-1082.org",
            "ad-server-mock-1083.com",
            "tracker-analytics-1083.net",
            "banner-ads-hosting-1083.org",
            "ad-server-mock-1084.com",
            "tracker-analytics-1084.net",
            "banner-ads-hosting-1084.org",
            "ad-server-mock-1085.com",
            "tracker-analytics-1085.net",
            "banner-ads-hosting-1085.org",
            "ad-server-mock-1086.com",
            "tracker-analytics-1086.net",
            "banner-ads-hosting-1086.org",
            "ad-server-mock-1087.com",
            "tracker-analytics-1087.net",
            "banner-ads-hosting-1087.org",
            "ad-server-mock-1088.com",
            "tracker-analytics-1088.net",
            "banner-ads-hosting-1088.org",
            "ad-server-mock-1089.com",
            "tracker-analytics-1089.net",
            "banner-ads-hosting-1089.org",
            "ad-server-mock-1090.com",
            "tracker-analytics-1090.net",
            "banner-ads-hosting-1090.org",
            "ad-server-mock-1091.com",
            "tracker-analytics-1091.net",
            "banner-ads-hosting-1091.org",
            "ad-server-mock-1092.com",
            "tracker-analytics-1092.net",
            "banner-ads-hosting-1092.org",
            "ad-server-mock-1093.com",
            "tracker-analytics-1093.net",
            "banner-ads-hosting-1093.org",
            "ad-server-mock-1094.com",
            "tracker-analytics-1094.net",
            "banner-ads-hosting-1094.org",
            "ad-server-mock-1095.com",
            "tracker-analytics-1095.net",
            "banner-ads-hosting-1095.org",
            "ad-server-mock-1096.com",
            "tracker-analytics-1096.net",
            "banner-ads-hosting-1096.org",
            "ad-server-mock-1097.com",
            "tracker-analytics-1097.net",
            "banner-ads-hosting-1097.org",
            "ad-server-mock-1098.com",
            "tracker-analytics-1098.net",
            "banner-ads-hosting-1098.org",
            "ad-server-mock-1099.com",
            "tracker-analytics-1099.net",
            "banner-ads-hosting-1099.org",
            "ad-server-mock-1100.com",
            "tracker-analytics-1100.net",
            "banner-ads-hosting-1100.org",
            "ad-server-mock-1101.com",
            "tracker-analytics-1101.net",
            "banner-ads-hosting-1101.org",
            "ad-server-mock-1102.com",
            "tracker-analytics-1102.net",
            "banner-ads-hosting-1102.org",
            "ad-server-mock-1103.com",
            "tracker-analytics-1103.net",
            "banner-ads-hosting-1103.org",
            "ad-server-mock-1104.com",
            "tracker-analytics-1104.net",
            "banner-ads-hosting-1104.org",
            "ad-server-mock-1105.com",
            "tracker-analytics-1105.net",
            "banner-ads-hosting-1105.org",
            "ad-server-mock-1106.com",
            "tracker-analytics-1106.net",
            "banner-ads-hosting-1106.org",
            "ad-server-mock-1107.com",
            "tracker-analytics-1107.net",
            "banner-ads-hosting-1107.org",
            "ad-server-mock-1108.com",
            "tracker-analytics-1108.net",
            "banner-ads-hosting-1108.org",
            "ad-server-mock-1109.com",
            "tracker-analytics-1109.net",
            "banner-ads-hosting-1109.org",
            "ad-server-mock-1110.com",
            "tracker-analytics-1110.net",
            "banner-ads-hosting-1110.org",
            "ad-server-mock-1111.com",
            "tracker-analytics-1111.net",
            "banner-ads-hosting-1111.org",
            "ad-server-mock-1112.com",
            "tracker-analytics-1112.net",
            "banner-ads-hosting-1112.org",
            "ad-server-mock-1113.com",
            "tracker-analytics-1113.net",
            "banner-ads-hosting-1113.org",
            "ad-server-mock-1114.com",
            "tracker-analytics-1114.net",
            "banner-ads-hosting-1114.org",
            "ad-server-mock-1115.com",
            "tracker-analytics-1115.net",
            "banner-ads-hosting-1115.org",
            "ad-server-mock-1116.com",
            "tracker-analytics-1116.net",
            "banner-ads-hosting-1116.org",
            "ad-server-mock-1117.com",
            "tracker-analytics-1117.net",
            "banner-ads-hosting-1117.org",
            "ad-server-mock-1118.com",
            "tracker-analytics-1118.net",
            "banner-ads-hosting-1118.org",
            "ad-server-mock-1119.com",
            "tracker-analytics-1119.net",
            "banner-ads-hosting-1119.org",
            "ad-server-mock-1120.com",
            "tracker-analytics-1120.net",
            "banner-ads-hosting-1120.org",
            "ad-server-mock-1121.com",
            "tracker-analytics-1121.net",
            "banner-ads-hosting-1121.org",
            "ad-server-mock-1122.com",
            "tracker-analytics-1122.net",
            "banner-ads-hosting-1122.org",
            "ad-server-mock-1123.com",
            "tracker-analytics-1123.net",
            "banner-ads-hosting-1123.org",
            "ad-server-mock-1124.com",
            "tracker-analytics-1124.net",
            "banner-ads-hosting-1124.org",
            "ad-server-mock-1125.com",
            "tracker-analytics-1125.net",
            "banner-ads-hosting-1125.org",
            "ad-server-mock-1126.com",
            "tracker-analytics-1126.net",
            "banner-ads-hosting-1126.org",
            "ad-server-mock-1127.com",
            "tracker-analytics-1127.net",
            "banner-ads-hosting-1127.org",
            "ad-server-mock-1128.com",
            "tracker-analytics-1128.net",
            "banner-ads-hosting-1128.org",
            "ad-server-mock-1129.com",
            "tracker-analytics-1129.net",
            "banner-ads-hosting-1129.org",
            "ad-server-mock-1130.com",
            "tracker-analytics-1130.net",
            "banner-ads-hosting-1130.org",
            "ad-server-mock-1131.com",
            "tracker-analytics-1131.net",
            "banner-ads-hosting-1131.org",
            "ad-server-mock-1132.com",
            "tracker-analytics-1132.net",
            "banner-ads-hosting-1132.org",
            "ad-server-mock-1133.com",
            "tracker-analytics-1133.net",
            "banner-ads-hosting-1133.org",
            "ad-server-mock-1134.com",
            "tracker-analytics-1134.net",
            "banner-ads-hosting-1134.org",
            "ad-server-mock-1135.com",
            "tracker-analytics-1135.net",
            "banner-ads-hosting-1135.org",
            "ad-server-mock-1136.com",
            "tracker-analytics-1136.net",
            "banner-ads-hosting-1136.org",
            "ad-server-mock-1137.com",
            "tracker-analytics-1137.net",
            "banner-ads-hosting-1137.org",
            "ad-server-mock-1138.com",
            "tracker-analytics-1138.net",
            "banner-ads-hosting-1138.org",
            "ad-server-mock-1139.com",
            "tracker-analytics-1139.net",
            "banner-ads-hosting-1139.org",
            "ad-server-mock-1140.com",
            "tracker-analytics-1140.net",
            "banner-ads-hosting-1140.org",
            "ad-server-mock-1141.com",
            "tracker-analytics-1141.net",
            "banner-ads-hosting-1141.org",
            "ad-server-mock-1142.com",
            "tracker-analytics-1142.net",
            "banner-ads-hosting-1142.org",
            "ad-server-mock-1143.com",
            "tracker-analytics-1143.net",
            "banner-ads-hosting-1143.org",
            "ad-server-mock-1144.com",
            "tracker-analytics-1144.net",
            "banner-ads-hosting-1144.org",
            "ad-server-mock-1145.com",
            "tracker-analytics-1145.net",
            "banner-ads-hosting-1145.org",
            "ad-server-mock-1146.com",
            "tracker-analytics-1146.net",
            "banner-ads-hosting-1146.org",
            "ad-server-mock-1147.com",
            "tracker-analytics-1147.net",
            "banner-ads-hosting-1147.org",
            "ad-server-mock-1148.com",
            "tracker-analytics-1148.net",
            "banner-ads-hosting-1148.org",
            "ad-server-mock-1149.com",
            "tracker-analytics-1149.net",
            "banner-ads-hosting-1149.org",
            "ad-server-mock-1150.com",
            "tracker-analytics-1150.net",
            "banner-ads-hosting-1150.org",
            "ad-server-mock-1151.com",
            "tracker-analytics-1151.net",
            "banner-ads-hosting-1151.org",
            "ad-server-mock-1152.com",
            "tracker-analytics-1152.net",
            "banner-ads-hosting-1152.org",
            "ad-server-mock-1153.com",
            "tracker-analytics-1153.net",
            "banner-ads-hosting-1153.org",
            "ad-server-mock-1154.com",
            "tracker-analytics-1154.net",
            "banner-ads-hosting-1154.org",
            "ad-server-mock-1155.com",
            "tracker-analytics-1155.net",
            "banner-ads-hosting-1155.org",
            "ad-server-mock-1156.com",
            "tracker-analytics-1156.net",
            "banner-ads-hosting-1156.org",
            "ad-server-mock-1157.com",
            "tracker-analytics-1157.net",
            "banner-ads-hosting-1157.org",
            "ad-server-mock-1158.com",
            "tracker-analytics-1158.net",
            "banner-ads-hosting-1158.org",
            "ad-server-mock-1159.com",
            "tracker-analytics-1159.net",
            "banner-ads-hosting-1159.org",
            "ad-server-mock-1160.com",
            "tracker-analytics-1160.net",
            "banner-ads-hosting-1160.org",
            "ad-server-mock-1161.com",
            "tracker-analytics-1161.net",
            "banner-ads-hosting-1161.org",
            "ad-server-mock-1162.com",
            "tracker-analytics-1162.net",
            "banner-ads-hosting-1162.org",
            "ad-server-mock-1163.com",
            "tracker-analytics-1163.net",
            "banner-ads-hosting-1163.org",
            "ad-server-mock-1164.com",
            "tracker-analytics-1164.net",
            "banner-ads-hosting-1164.org",
            "ad-server-mock-1165.com",
            "tracker-analytics-1165.net",
            "banner-ads-hosting-1165.org",
            "ad-server-mock-1166.com",
            "tracker-analytics-1166.net",
            "banner-ads-hosting-1166.org",
            "ad-server-mock-1167.com",
            "tracker-analytics-1167.net",
            "banner-ads-hosting-1167.org",
            "ad-server-mock-1168.com",
            "tracker-analytics-1168.net",
            "banner-ads-hosting-1168.org",
            "ad-server-mock-1169.com",
            "tracker-analytics-1169.net",
            "banner-ads-hosting-1169.org",
            "ad-server-mock-1170.com",
            "tracker-analytics-1170.net",
            "banner-ads-hosting-1170.org",
            "ad-server-mock-1171.com",
            "tracker-analytics-1171.net",
            "banner-ads-hosting-1171.org",
            "ad-server-mock-1172.com",
            "tracker-analytics-1172.net",
            "banner-ads-hosting-1172.org",
            "ad-server-mock-1173.com",
            "tracker-analytics-1173.net",
            "banner-ads-hosting-1173.org",
            "ad-server-mock-1174.com",
            "tracker-analytics-1174.net",
            "banner-ads-hosting-1174.org",
            "ad-server-mock-1175.com",
            "tracker-analytics-1175.net",
            "banner-ads-hosting-1175.org",
            "ad-server-mock-1176.com",
            "tracker-analytics-1176.net",
            "banner-ads-hosting-1176.org",
            "ad-server-mock-1177.com",
            "tracker-analytics-1177.net",
            "banner-ads-hosting-1177.org",
            "ad-server-mock-1178.com",
            "tracker-analytics-1178.net",
            "banner-ads-hosting-1178.org",
            "ad-server-mock-1179.com",
            "tracker-analytics-1179.net",
            "banner-ads-hosting-1179.org",
            "ad-server-mock-1180.com",
            "tracker-analytics-1180.net",
            "banner-ads-hosting-1180.org",
            "ad-server-mock-1181.com",
            "tracker-analytics-1181.net",
            "banner-ads-hosting-1181.org",
            "ad-server-mock-1182.com",
            "tracker-analytics-1182.net",
            "banner-ads-hosting-1182.org",
            "ad-server-mock-1183.com",
            "tracker-analytics-1183.net",
            "banner-ads-hosting-1183.org",
            "ad-server-mock-1184.com",
            "tracker-analytics-1184.net",
            "banner-ads-hosting-1184.org",
            "ad-server-mock-1185.com",
            "tracker-analytics-1185.net",
            "banner-ads-hosting-1185.org",
            "ad-server-mock-1186.com",
            "tracker-analytics-1186.net",
            "banner-ads-hosting-1186.org",
            "ad-server-mock-1187.com",
            "tracker-analytics-1187.net",
            "banner-ads-hosting-1187.org",
            "ad-server-mock-1188.com",
            "tracker-analytics-1188.net",
            "banner-ads-hosting-1188.org",
            "ad-server-mock-1189.com",
            "tracker-analytics-1189.net",
            "banner-ads-hosting-1189.org",
            "ad-server-mock-1190.com",
            "tracker-analytics-1190.net",
            "banner-ads-hosting-1190.org",
            "ad-server-mock-1191.com",
            "tracker-analytics-1191.net",
            "banner-ads-hosting-1191.org",
            "ad-server-mock-1192.com",
            "tracker-analytics-1192.net",
            "banner-ads-hosting-1192.org",
            "ad-server-mock-1193.com",
            "tracker-analytics-1193.net",
            "banner-ads-hosting-1193.org",
            "ad-server-mock-1194.com",
            "tracker-analytics-1194.net",
            "banner-ads-hosting-1194.org",
            "ad-server-mock-1195.com",
            "tracker-analytics-1195.net",
            "banner-ads-hosting-1195.org",
            "ad-server-mock-1196.com",
            "tracker-analytics-1196.net",
            "banner-ads-hosting-1196.org",
            "ad-server-mock-1197.com",
            "tracker-analytics-1197.net",
            "banner-ads-hosting-1197.org",
            "ad-server-mock-1198.com",
            "tracker-analytics-1198.net",
            "banner-ads-hosting-1198.org",
            "ad-server-mock-1199.com",
            "tracker-analytics-1199.net",
            "banner-ads-hosting-1199.org"
        )
        blockedDomains.addAll(rules)
    }
}

// ==========================================
// MAIN UI COMPONENTS (JETPACK COMPOSE)
// ==========================================

@SuppressLint("SetJavaScriptEnabled")
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun BrowserApp(viewModel: BrowserViewModel = viewModel()) {
    val currentScreen = viewModel.currentScreen
    val context = LocalContext.current
    
    // We instantiate the AdBlocker once
    val adBlocker = remember { AdBlocker() }

    MaterialTheme(
        colorScheme = if (viewModel.isDarkModeEnabled) darkColorScheme() else lightColorScheme()
    ) {
        Surface(modifier = Modifier.fillMaxSize()) {
            Crossfade(targetState = currentScreen, label = "ScreenTransition") { screen ->
                when (screen) {
                    BrowserScreen.BROWSER -> BrowserScreenUI(viewModel, adBlocker)
                    BrowserScreen.TABS -> TabsScreenUI(viewModel)
                    BrowserScreen.SETTINGS -> SettingsScreenUI(viewModel)
                    BrowserScreen.BOOKMARKS -> BookmarksScreenUI(viewModel)
                    BrowserScreen.HISTORY -> HistoryScreenUI(viewModel)
                }
            }
        }
    }
    
    // Handle system back button
    BackHandler(enabled = true) {
        when (viewModel.currentScreen) {
            BrowserScreen.BROWSER -> {
                val tab = viewModel.currentTab
                if (tab?.canGoBack == true) {
                    tab.webView?.goBack()
                } else {
                    // Prompt app close or go to previous tab
                    val activity = context as? ComponentActivity
                    activity?.moveTaskToBack(true)
                }
            }
            else -> viewModel.currentScreen = BrowserScreen.BROWSER
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun BrowserScreenUI(viewModel: BrowserViewModel, adBlocker: AdBlocker) {
    val currentTab = viewModel.currentTab

    Scaffold(
        topBar = {
            if (viewModel.showAddressBar) {
                AddressBar(viewModel)
            }
        },
        bottomBar = {
            BottomNavigationBar(viewModel)
        }
    ) { paddingValues ->
        Box(
            modifier = Modifier
                .fillMaxSize()
                .padding(paddingValues)
        ) {
            if (currentTab != null) {
                WebViewComponent(viewModel, currentTab, adBlocker)
                
                // Progress Bar Overlay
                if (currentTab.isLoading) {
                    LinearProgressIndicator(
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(2.dp)
                            .align(Alignment.TopCenter),
                        progress = { currentTab.loadProgress / 100f }
                    )
                }
            } else {
                // Empty State
                Column(
                    modifier = Modifier.fillMaxSize(),
                    verticalArrangement = Arrangement.Center,
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    Icon(
                        imageVector = Icons.Default.Public,
                        contentDescription = "No Tabs",
                        modifier = Modifier.size(64.dp),
                        tint = Color.Gray
                    )
                    Spacer(modifier = Modifier.height(16.dp))
                    Text("No Active Tabs", style = MaterialTheme.typography.titleLarge)
                    Spacer(modifier = Modifier.height(16.dp))
                    Button(onClick = { viewModel.createNewTab() }) {
                        Text("Open New Tab")
                    }
                }
            }
        }
    }
}

@Composable
fun AddressBar(viewModel: BrowserViewModel) {
    val focusManager = LocalFocusManager.current
    val currentTab = viewModel.currentTab
    
    Surface(
        shadowElevation = 4.dp,
        modifier = Modifier.fillMaxWidth(),
        color = MaterialTheme.colorScheme.surface
    ) {
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 8.dp, vertical = 6.dp)
        ) {
            // SSL Icon or Incognito Icon
            Icon(
                imageVector = if (currentTab?.isIncognito == true) Icons.Default.PrivacyTip else Icons.Default.Lock,
                contentDescription = "Security",
                tint = if (currentTab?.isIncognito == true) Color.Magenta else MaterialTheme.colorScheme.primary,
                modifier = Modifier.padding(8.dp)
            )

            OutlinedTextField(
                value = viewModel.omniboxText,
                onValueChange = { viewModel.omniboxText = it },
                modifier = Modifier
                    .weight(1f)
                    .heightIn(min = 48.dp),
                shape = RoundedCornerShape(24.dp),
                colors = OutlinedTextFieldDefaults.colors(
                    focusedBorderColor = MaterialTheme.colorScheme.primary,
                    unfocusedBorderColor = Color.Transparent,
                    unfocusedContainerColor = MaterialTheme.colorScheme.surfaceVariant
                ),
                singleLine = true,
                keyboardOptions = KeyboardOptions(
                    keyboardType = KeyboardType.Uri,
                    imeAction = ImeAction.Go
                ),
                keyboardActions = KeyboardActions(
                    onGo = {
                        viewModel.loadUrlInCurrentTab(viewModel.omniboxText)
                        focusManager.clearFocus()
                    }
                ),
                placeholder = { Text("Search or type URL") },
                trailingIcon = {
                    if (viewModel.omniboxText.isNotEmpty()) {
                        IconButton(onClick = { viewModel.omniboxText = "" }) {
                            Icon(Icons.Default.Clear, contentDescription = "Clear text")
                        }
                    } else if (currentTab?.isLoading == true) {
                        IconButton(onClick = { currentTab.webView?.stopLoading() }) {
                            Icon(Icons.Default.Close, contentDescription = "Stop Loading")
                        }
                    } else {
                        IconButton(onClick = { currentTab?.webView?.reload() }) {
                            Icon(Icons.Default.Refresh, contentDescription = "Refresh")
                        }
                    }
                }
            )
            
            // Bookmark Star
            if (currentTab != null && !currentTab.isIncognito) {
                val isBookmarked = viewModel.isBookmarked(currentTab.url)
                IconButton(onClick = {
                    viewModel.toggleBookmark(currentTab.url, currentTab.title)
                }) {
                    Icon(
                        imageVector = if (isBookmarked) Icons.Default.Star else Icons.Default.StarOutline,
                        contentDescription = "Bookmark",
                        tint = if (isBookmarked) Color.Yellow else MaterialTheme.colorScheme.onSurface
                    )
                }
            }
        }
    }
}

@Composable
fun BottomNavigationBar(viewModel: BrowserViewModel) {
    val currentTab = viewModel.currentTab
    
    BottomAppBar(
        containerColor = MaterialTheme.colorScheme.surface,
        contentColor = MaterialTheme.colorScheme.onSurface,
        tonalElevation = 8.dp
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceEvenly,
            verticalAlignment = Alignment.CenterVertically
        ) {
            IconButton(
                onClick = { currentTab?.webView?.goBack() },
                enabled = currentTab?.canGoBack == true
            ) {
                Icon(Icons.Default.ArrowBack, contentDescription = "Back")
            }
            
            IconButton(
                onClick = { currentTab?.webView?.goForward() },
                enabled = currentTab?.canGoForward == true
            ) {
                Icon(Icons.Default.ArrowForward, contentDescription = "Forward")
            }
            
            IconButton(onClick = { viewModel.currentScreen = BrowserScreen.TABS }) {
                Box(contentAlignment = Alignment.Center) {
                    Icon(Icons.Default.FilterNone, contentDescription = "Tabs")
                    Text(
                        text = viewModel.tabs.size.toString(),
                        fontSize = 10.sp,
                        fontWeight = FontWeight.Bold,
                        modifier = Modifier.padding(top = 2.dp)
                    )
                }
            }
            
            IconButton(onClick = { viewModel.loadUrlInCurrentTab(viewModel.searchEngine.searchUrl) }) {
                Icon(Icons.Default.Home, contentDescription = "Home")
            }
            
            var showMenu by remember { mutableStateOf(false) }
            
            Box {
                IconButton(onClick = { showMenu = true }) {
                    Icon(Icons.Default.Menu, contentDescription = "Menu")
                }
                
                DropdownMenu(
                    expanded = showMenu,
                    onDismissRequest = { showMenu = false }
                ) {
                    DropdownMenuItem(
                        text = { Text("New Incognito Tab") },
                        onClick = {
                            viewModel.createNewTab(isIncognito = true)
                            showMenu = false
                        },
                        leadingIcon = { Icon(Icons.Default.VisibilityOff, null) }
                    )
                    DropdownMenuItem(
                        text = { Text("Bookmarks") },
                        onClick = {
                            viewModel.currentScreen = BrowserScreen.BOOKMARKS
                            showMenu = false
                        },
                        leadingIcon = { Icon(Icons.Default.Bookmarks, null) }
                    )
                    DropdownMenuItem(
                        text = { Text("History") },
                        onClick = {
                            viewModel.currentScreen = BrowserScreen.HISTORY
                            showMenu = false
                        },
                        leadingIcon = { Icon(Icons.Default.History, null) }
                    )
                    HorizontalDivider()
                    DropdownMenuItem(
                        text = { Text("Settings") },
                        onClick = {
                            viewModel.currentScreen = BrowserScreen.SETTINGS
                            showMenu = false
                        },
                        leadingIcon = { Icon(Icons.Default.Settings, null) }
                    )
                    DropdownMenuItem(
                        text = { Text("Desktop Site") },
                        onClick = {
                            viewModel.isDesktopMode = !viewModel.isDesktopMode
                            currentTab?.webView?.settings?.userAgentString = if (viewModel.isDesktopMode) {
                                "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36"
                            } else {
                                null // Default mobile user agent
                            }
                            currentTab?.webView?.reload()
                            showMenu = false
                        },
                        trailingIcon = {
                            Checkbox(checked = viewModel.isDesktopMode, onCheckedChange = null)
                        }
                    )
                }
            }
        }
    }
}


// ==========================================
// WEBVIEW COMPOSABLE & LOGIC
// ==========================================

@SuppressLint("SetJavaScriptEnabled")
@Composable
fun WebViewComponent(
    viewModel: BrowserViewModel,
    tab: TabState,
    adBlocker: AdBlocker
) {
    val context = LocalContext.current
    
    // Using AndroidView to wrap standard Android WebView
    AndroidView(
        factory = { ctx ->
            WebView(ctx).apply {
                layoutParams = ViewGroup.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT
                )
                
                // WebView Settings
                settings.apply {
                    javaScriptEnabled = viewModel.isJavaScriptEnabled
                    domStorageEnabled = true
                    databaseEnabled = true
                    loadsImagesAutomatically = true
                    mixedContentMode = WebSettings.MIXED_CONTENT_COMPATIBILITY_MODE
                    setSupportZoom(true)
                    builtInZoomControls = true
                    displayZoomControls = false
                    useWideViewPort = true
                    loadWithOverviewMode = true
                    
                    // Desktop mode check
                    if (viewModel.isDesktopMode) {
                        userAgentString = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36"
                    }
                    
                    // Incognito setup
                    if (tab.isIncognito) {
                        cacheMode = WebSettings.LOAD_NO_CACHE
                        CookieManager.getInstance().setAcceptCookie(false)
                        clearHistory()
                        clearCache(true)
                        clearFormData()
                    } else {
                        cacheMode = WebSettings.LOAD_DEFAULT
                        CookieManager.getInstance().setAcceptCookie(true)
                    }
                }
                
                // Attach custom clients
                webViewClient = CustomWebViewClient(viewModel, tab.id, adBlocker)
                webChromeClient = CustomWebChromeClient(viewModel, tab.id)
                
                // Download manager setup
                setDownloadListener { url, userAgent, contentDisposition, mimetype, contentLength ->
                    val request = DownloadManager.Request(Uri.parse(url)).apply {
                        setMimeType(mimetype)
                        addRequestHeader("cookie", CookieManager.getInstance().getCookie(url))
                        addRequestHeader("User-Agent", userAgent)
                        setDescription("Downloading file...")
                        setTitle(URLUtil.guessFileName(url, contentDisposition, mimetype))
                        allowScanningByMediaScanner()
                        setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED)
                        setDestinationInExternalPublicDir(Environment.DIRECTORY_DOWNLOADS, URLUtil.guessFileName(url, contentDisposition, mimetype))
                    }
                    val dm = context.getSystemService(Context.DOWNLOAD_SERVICE) as DownloadManager
                    dm.enqueue(request)
                    Toast.makeText(context, "Download started...", Toast.LENGTH_SHORT).show()
                }

                tab.webView = this // Persist reference to viewmodel
                loadUrl(tab.url)
            }
        },
        update = { webView ->
            // Update logic if needed when compose state changes, e.g. toggling JS dynamically
            webView.settings.javaScriptEnabled = viewModel.isJavaScriptEnabled
        },
        modifier = Modifier.fillMaxSize()
    )
}


// ==========================================
// SECONDARY SCREENS (TABS, SETTINGS, ETC)
// ==========================================

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TabsScreenUI(viewModel: BrowserViewModel) {
    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Open Tabs (${viewModel.tabs.size})") },
                navigationIcon = {
                    IconButton(onClick = { viewModel.currentScreen = BrowserScreen.BROWSER }) {
                        Icon(Icons.Default.ArrowBack, contentDescription = "Back")
                    }
                },
                actions = {
                    IconButton(onClick = { viewModel.createNewTab() }) {
                        Icon(Icons.Default.Add, contentDescription = "New Tab")
                    }
                    IconButton(onClick = { viewModel.createNewTab(isIncognito = true) }) {
                        Icon(Icons.Default.VisibilityOff, contentDescription = "New Incognito Tab")
                    }
                }
            )
        }
    ) { padding ->
        LazyVerticalGrid(
            columns = GridCells.Fixed(2),
            modifier = Modifier
                .padding(padding)
                .fillMaxSize(),
            contentPadding = PaddingValues(8.dp),
            horizontalArrangement = Arrangement.spacedBy(8.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            items(viewModel.tabs) { tab ->
                Card(
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(180.dp)
                        .clickable { viewModel.switchTab(tab.id) },
                    border = if (viewModel.currentTabId == tab.id) 
                             BorderStroke(2.dp, MaterialTheme.colorScheme.primary) 
                             else null,
                    colors = CardDefaults.cardColors(
                        containerColor = if (tab.isIncognito) Color(0xFF2A2A2A) else MaterialTheme.colorScheme.surfaceVariant,
                        contentColor = if (tab.isIncognito) Color.White else MaterialTheme.colorScheme.onSurfaceVariant
                    )
                ) {
                    Column(
                        modifier = Modifier.fillMaxSize()
                    ) {
                        Row(
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(8.dp),
                            horizontalArrangement = Arrangement.SpaceBetween,
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Text(
                                text = tab.title,
                                maxLines = 1,
                                overflow = TextOverflow.Ellipsis,
                                modifier = Modifier.weight(1f),
                                fontWeight = FontWeight.Bold,
                                fontSize = 14.sp
                            )
                            IconButton(
                                onClick = { viewModel.closeTab(tab.id) },
                                modifier = Modifier.size(24.dp)
                            ) {
                                Icon(Icons.Default.Close, contentDescription = "Close", modifier = Modifier.size(16.dp))
                            }
                        }
                        HorizontalDivider()
                        // A placeholder for Web Content Thumbnail
                        Box(
                            modifier = Modifier
                                .fillMaxSize()
                                .background(if (tab.isIncognito) Color.DarkGray else Color.White),
                            contentAlignment = Alignment.Center
                        ) {
                            if (tab.isIncognito) {
                                Icon(Icons.Default.VisibilityOff, contentDescription = null, modifier = Modifier.size(48.dp), tint = Color.Gray)
                            } else {
                                Text(tab.url, fontSize = 10.sp, color = Color.Gray, textAlign = TextAlign.Center, modifier = Modifier.padding(16.dp))
                            }
                        }
                    }
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreenUI(viewModel: BrowserViewModel) {
    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Settings") },
                navigationIcon = {
                    IconButton(onClick = { viewModel.currentScreen = BrowserScreen.BROWSER }) {
                        Icon(Icons.Default.ArrowBack, contentDescription = "Back")
                    }
                }
            )
        }
    ) { padding ->
        LazyColumn(
            modifier = Modifier
                .padding(padding)
                .fillMaxSize()
        ) {
            item {
                SettingsCategory("Search Engine")
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(16.dp),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text("Default Search Engine")
                    
                    var expanded by remember { mutableStateOf(false) }
                    Box {
                        TextButton(onClick = { expanded = true }) {
                            Text(viewModel.searchEngine.engineName)
                        }
                        DropdownMenu(expanded = expanded, onDismissRequest = { expanded = false }) {
                            SearchEngine.values().forEach { engine ->
                                DropdownMenuItem(
                                    text = { Text(engine.engineName) },
                                    onClick = {
                                        viewModel.searchEngine = engine
                                        expanded = false
                                    }
                                )
                            }
                        }
                    }
                }
                HorizontalDivider()
            }
            
            item {
                SettingsCategory("Privacy & Security")
                SettingsSwitch(
                    title = "Ad Blocker (EasyList)",
                    subtitle = "Blocks intrusive ads and trackers",
                    checked = viewModel.isAdBlockEnabled,
                    onCheckedChange = { viewModel.isAdBlockEnabled = it }
                )
                HorizontalDivider()
            }
            
            item {
                SettingsCategory("Site Settings")
                SettingsSwitch(
                    title = "Enable JavaScript",
                    subtitle = "Allows sites to run interactive scripts",
                    checked = viewModel.isJavaScriptEnabled,
                    onCheckedChange = { viewModel.isJavaScriptEnabled = it }
                )
                HorizontalDivider()
            }
            
            item {
                SettingsCategory("Appearance")
                SettingsSwitch(
                    title = "Dark Mode Override",
                    subtitle = "Forces dark styling on websites",
                    checked = viewModel.isDarkModeEnabled,
                    onCheckedChange = { viewModel.isDarkModeEnabled = it }
                )
                SettingsSwitch(
                    title = "Desktop Site by Default",
                    subtitle = "Requests desktop version of websites",
                    checked = viewModel.isDesktopMode,
                    onCheckedChange = { viewModel.isDesktopMode = it }
                )
                HorizontalDivider()
            }
            
            item {
                SettingsCategory("About")
                ListItem(
                    headlineContent = { Text("SuperBrowser Version 1.0.0") },
                    supportingContent = { Text("Built with Jetpack Compose & WebKit") },
                    leadingContent = { Icon(Icons.Default.Info, contentDescription = null) }
                )
            }
        }
    }
}

@Composable
fun SettingsCategory(title: String) {
    Text(
        text = title,
        color = MaterialTheme.colorScheme.primary,
        fontWeight = FontWeight.Bold,
        modifier = Modifier.padding(start = 16.dp, top = 24.dp, bottom = 8.dp, end = 16.dp)
    )
}

@Composable
fun SettingsSwitch(title: String, subtitle: String, checked: Boolean, onCheckedChange: (Boolean) -> Unit) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { onCheckedChange(!checked) }
            .padding(16.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column(modifier = Modifier.weight(1f)) {
            Text(title, style = MaterialTheme.typography.bodyLarge)
            Text(subtitle, style = MaterialTheme.typography.bodyMedium, color = Color.Gray)
        }
        Switch(checked = checked, onCheckedChange = onCheckedChange)
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun BookmarksScreenUI(viewModel: BrowserViewModel) {
    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Bookmarks") },
                navigationIcon = {
                    IconButton(onClick = { viewModel.currentScreen = BrowserScreen.BROWSER }) {
                        Icon(Icons.Default.ArrowBack, contentDescription = "Back")
                    }
                }
            )
        }
    ) { padding ->
        if (viewModel.bookmarks.isEmpty()) {
            Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                Text("No bookmarks yet.", color = Color.Gray)
            }
        } else {
            LazyColumn(modifier = Modifier.padding(padding)) {
                items(viewModel.bookmarks) { bookmark ->
                    ListItem(
                        headlineContent = { Text(bookmark.title, maxLines = 1, overflow = TextOverflow.Ellipsis) },
                        supportingContent = { Text(bookmark.url, maxLines = 1, overflow = TextOverflow.Ellipsis, color = Color.Gray) },
                        leadingContent = { Icon(Icons.Default.Bookmark, contentDescription = null, tint = MaterialTheme.colorScheme.primary) },
                        trailingContent = {
                            IconButton(onClick = { viewModel.bookmarks.remove(bookmark) }) {
                                Icon(Icons.Default.Delete, contentDescription = "Delete")
                            }
                        },
                        modifier = Modifier.clickable {
                            viewModel.loadUrlInCurrentTab(bookmark.url)
                            viewModel.currentScreen = BrowserScreen.BROWSER
                        }
                    )
                    HorizontalDivider()
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HistoryScreenUI(viewModel: BrowserViewModel) {
    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("History") },
                navigationIcon = {
                    IconButton(onClick = { viewModel.currentScreen = BrowserScreen.BROWSER }) {
                        Icon(Icons.Default.ArrowBack, contentDescription = "Back")
                    }
                },
                actions = {
                    IconButton(onClick = { viewModel.clearHistory() }) {
                        Icon(Icons.Default.DeleteSweep, contentDescription = "Clear History")
                    }
                }
            )
        }
    ) { padding ->
        if (viewModel.history.isEmpty()) {
            Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                Text("Your history is empty.", color = Color.Gray)
            }
        } else {
            val dateFormat = remember { SimpleDateFormat("MMM dd, yyyy HH:mm", Locale.getDefault()) }
            
            LazyColumn(modifier = Modifier.padding(padding)) {
                items(viewModel.history) { historyItem ->
                    val dateString = dateFormat.format(Date(historyItem.timestamp))
                    ListItem(
                        headlineContent = { Text(historyItem.title, maxLines = 1, overflow = TextOverflow.Ellipsis) },
                        supportingContent = { 
                            Column {
                                Text(historyItem.url, maxLines = 1, overflow = TextOverflow.Ellipsis, color = Color.Blue)
                                Text(dateString, style = MaterialTheme.typography.labelSmall, color = Color.Gray)
                            }
                        },
                        leadingContent = { Icon(Icons.Default.History, contentDescription = null) },
                        modifier = Modifier.clickable {
                            viewModel.loadUrlInCurrentTab(historyItem.url)
                            viewModel.currentScreen = BrowserScreen.BROWSER
                        }
                    )
                    HorizontalDivider()
                }
            }
        }
    }
}


// ==========================================
// ANDROID COMPONENT ACTIVITY
// ==========================================

class SuperBrowserActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        // This is the entry point of our Compose Application
        setContent {
            BrowserApp()
        }
    }
    
    // Graceful handling of back press overrides when UI states change
    @Deprecated("Deprecated in Java", ReplaceWith("super.onBackPressed()"))
    override fun onBackPressed() {
        super.onBackPressed()
        // Compose BackHandler manages internal navigation.
        // Activity will close if there's no backstack left.
    }
}
