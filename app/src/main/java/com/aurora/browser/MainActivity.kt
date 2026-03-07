package com.aurora.browser

import android.Manifest
import android.annotation.SuppressLint
import android.app.Activity
import android.app.AlertDialog
import android.app.DownloadManager
import android.content.*
import android.content.pm.PackageManager
import android.graphics.*
import android.graphics.drawable.*
import android.net.Uri
import android.net.http.SslError
import android.os.*
import android.text.*
import android.util.*
import android.view.*
import android.view.animation.*
import android.view.inputmethod.*
import android.webkit.*
import android.widget.*
import androidx.annotation.RequiresApi
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout
import org.json.JSONArray
import org.json.JSONObject
import java.io.*
import java.net.HttpURLConnection
import java.net.URL
import java.text.SimpleDateFormat
import java.util.*
import kotlin.concurrent.thread

// ========================= DATA CLASSES =========================

data class BrowserTab(
    val id: String = UUID.randomUUID().toString(),
    var title: String = "Новая вкладка",
    var url: String = "",
    var favicon: Bitmap? = null,
    var webView: WebView? = null,
    var thumbnail: Bitmap? = null,
    var isHomePage: Boolean = true
)

data class BookmarkItem(
    val title: String,
    val url: String,
    val favicon: String = "",
    val timestamp: Long = System.currentTimeMillis()
)

data class HistoryItem(
    val title: String,
    val url: String,
    val timestamp: Long = System.currentTimeMillis()
)

data class SpeedDialItem(
    val title: String,
    val url: String,
    val iconColor: Int = Color.BLUE
)

data class NewsItem(
    val title: String,
    val source: String,
    val imageUrl: String,
    val link: String,
    val category: String
)

// ========================= MAIN ACTIVITY =========================

class MainActivity : Activity() {

    // --- State ---
    private val tabs = mutableListOf<BrowserTab>()
    private var currentTabIndex = 0
    private val bookmarks = mutableListOf<BookmarkItem>()
    private val history = mutableListOf<HistoryItem>()
    private val speedDials = mutableListOf<SpeedDialItem>()
    private var isIncognitoMode = false
    private var isDesktopMode = false
    private var currentNewsCategory = "all"

    // --- Views ---
    private lateinit var rootLayout: FrameLayout
    private lateinit var mainContainer: LinearLayout
    private lateinit var addressBarLayout: RelativeLayout
    private lateinit var addressBarInput: EditText
    private lateinit var progressBar: ProgressBar
    private lateinit var webViewContainer: FrameLayout
    private lateinit var homePageView: ScrollView
    private lateinit var bottomNavBar: LinearLayout
    private lateinit var swipeRefresh: SwipeRefreshLayout
    private lateinit var tabCountText: TextView
    private lateinit var sslIcon: ImageView
    private lateinit var menuButton: ImageView

    // --- Prefs ---
    private lateinit var prefs: SharedPreferences

    // --- News ---
    private val newsItems = mutableListOf<NewsItem>()
    private var newsContainer: LinearLayout? = null

    // ========================= LIFECYCLE =========================

    @SuppressLint("SetJavaScriptEnabled")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        window.statusBarColor = Color.TRANSPARENT
        window.decorView.systemUiVisibility = (
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
        )
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            window.attributes.layoutInDisplayCutoutMode =
                WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES
        }

        prefs = getSharedPreferences("aurora_browser", MODE_PRIVATE)
        loadData()
        initDefaultSpeedDials()

        buildUI()
        createNewTab()
        showHomePage()
    }

    override fun onBackPressed() {
        val tab = currentTab()
        if (tab?.webView?.canGoBack() == true) {
            tab.webView?.goBack()
        } else if (tab?.isHomePage == false) {
            showHomePage()
        } else {
            super.onBackPressed()
        }
    }

    override fun onPause() {
        super.onPause()
        saveData()
    }

    override fun onDestroy() {
        tabs.forEach { it.webView?.destroy() }
        super.onDestroy()
    }

    // ========================= DATA PERSISTENCE =========================

    private fun saveData() {
        val editor = prefs.edit()
        // Bookmarks
        val bArr = JSONArray()
        bookmarks.forEach { b ->
            val obj = JSONObject()
            obj.put("title", b.title)
            obj.put("url", b.url)
            obj.put("timestamp", b.timestamp)
            bArr.put(obj)
        }
        editor.putString("bookmarks", bArr.toString())

        // History
        val hArr = JSONArray()
        history.takeLast(500).forEach { h ->
            val obj = JSONObject()
            obj.put("title", h.title)
            obj.put("url", h.url)
            obj.put("timestamp", h.timestamp)
            hArr.put(obj)
        }
        editor.putString("history", hArr.toString())

        // Speed dials
        val sArr = JSONArray()
        speedDials.forEach { s ->
            val obj = JSONObject()
            obj.put("title", s.title)
            obj.put("url", s.url)
            obj.put("iconColor", s.iconColor)
            sArr.put(obj)
        }
        editor.putString("speed_dials", sArr.toString())
        editor.apply()
    }

    private fun loadData() {
        // Bookmarks
        try {
            val bStr = prefs.getString("bookmarks", "[]") ?: "[]"
            val bArr = JSONArray(bStr)
            for (i in 0 until bArr.length()) {
                val obj = bArr.getJSONObject(i)
                bookmarks.add(BookmarkItem(
                    obj.getString("title"),
                    obj.getString("url"),
                    "",
                    obj.optLong("timestamp", System.currentTimeMillis())
                ))
            }
        } catch (_: Exception) {}

        // History
        try {
            val hStr = prefs.getString("history", "[]") ?: "[]"
            val hArr = JSONArray(hStr)
            for (i in 0 until hArr.length()) {
                val obj = hArr.getJSONObject(i)
                history.add(HistoryItem(
                    obj.getString("title"),
                    obj.getString("url"),
                    obj.optLong("timestamp", System.currentTimeMillis())
                ))
            }
        } catch (_: Exception) {}

        // Speed dials
        try {
            val sStr = prefs.getString("speed_dials", "[]") ?: "[]"
            val sArr = JSONArray(sStr)
            for (i in 0 until sArr.length()) {
                val obj = sArr.getJSONObject(i)
                speedDials.add(SpeedDialItem(
                    obj.getString("title"),
                    obj.getString("url"),
                    obj.optInt("iconColor", Color.BLUE)
                ))
            }
        } catch (_: Exception) {}
    }

    private fun initDefaultSpeedDials() {
        if (speedDials.isEmpty()) {
            speedDials.addAll(listOf(
                SpeedDialItem("Google", "https://www.google.com", Color.parseColor("#4285F4")),
                SpeedDialItem("YouTube", "https://www.youtube.com", Color.parseColor("#FF0000")),
                SpeedDialItem("Wikipedia", "https://www.wikipedia.org", Color.parseColor("#636466")),
                SpeedDialItem("GitHub", "https://github.com", Color.parseColor("#333333")),
                SpeedDialItem("Reddit", "https://www.reddit.com", Color.parseColor("#FF4500")),
                SpeedDialItem("Habr", "https://habr.com", Color.parseColor("#77A2B6")),
                SpeedDialItem("VK", "https://vk.com", Color.parseColor("#0077FF")),
                SpeedDialItem("RBC", "https://www.rbc.ru", Color.parseColor("#50C878"))
            ))
        }
    }

    // ========================= UI BUILDER =========================

    private fun dp(value: Int): Int = (value * resources.displayMetrics.density).toInt()
    private fun sp(value: Float): Float = value * resources.displayMetrics.scaledDensity

    private fun currentTab(): BrowserTab? = tabs.getOrNull(currentTabIndex)

    @SuppressLint("ClickableViewAccessibility")
    private fun buildUI() {
        rootLayout = FrameLayout(this).apply {
            layoutParams = FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT
            )
            setBackgroundColor(Color.WHITE)
        }

        mainContainer = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.MATCH_PARENT
            )
        }

        // Status bar spacer
        val statusBarHeight = getStatusBarHeight()
        val statusSpacer = View(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, statusBarHeight
            )
            setBackgroundColor(Color.parseColor("#F8F9FA"))
        }
        mainContainer.addView(statusSpacer)

        // Address bar
        buildAddressBar()

        // Progress bar
        progressBar = ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, dp(3)
            )
            progressDrawable = GradientDrawable().apply {
                shape = GradientDrawable.RECTANGLE
                setColor(Color.parseColor("#6C5CE7"))
            }
            max = 100
            progress = 0
            visibility = View.GONE
        }
        mainContainer.addView(progressBar)

        // Content area (SwipeRefresh + WebView container + HomePage)
        swipeRefresh = SwipeRefreshLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, 0, 1f
            )
            setColorSchemeColors(
                Color.parseColor("#6C5CE7"),
                Color.parseColor("#A29BFE"),
                Color.parseColor("#FD79A8")
            )
            setOnRefreshListener {
                val tab = currentTab()
                if (tab?.isHomePage == true) {
                    refreshHomePage()
                    isRefreshing = false
                } else {
                    tab?.webView?.reload()
                }
            }
        }

        val contentFrame = FrameLayout(this).apply {
            layoutParams = FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT
            )
        }

        webViewContainer = FrameLayout(this).apply {
            layoutParams = FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT
            )
        }
        contentFrame.addView(webViewContainer)

        buildHomePage()
        contentFrame.addView(homePageView)

        swipeRefresh.addView(contentFrame)
        mainContainer.addView(swipeRefresh)

        // Bottom nav
        buildBottomNavBar()

        rootLayout.addView(mainContainer)
        setContentView(rootLayout)
    }

    private fun getStatusBarHeight(): Int {
        val resourceId = resources.getIdentifier("status_bar_height", "dimen", "android")
        return if (resourceId > 0) resources.getDimensionPixelSize(resourceId) else dp(24)
    }

    // ========================= ADDRESS BAR =========================

    @SuppressLint("ClickableViewAccessibility")
    private fun buildAddressBar() {
        addressBarLayout = RelativeLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                dp(52)
            ).apply { setMargins(dp(12), dp(6), dp(12), dp(6)) }

            val bgDrawable = GradientDrawable().apply {
                cornerRadius = dp(26).toFloat()
                setColor(Color.parseColor("#F1F3F4"))
                setStroke(dp(1), Color.parseColor("#E0E0E0"))
            }
            background = bgDrawable
            setPadding(dp(16), 0, dp(8), 0)
        }

        // SSL Icon
        sslIcon = ImageView(this).apply {
            id = View.generateViewId()
            layoutParams = RelativeLayout.LayoutParams(dp(20), dp(20)).apply {
                addRule(RelativeLayout.ALIGN_PARENT_START)
                addRule(RelativeLayout.CENTER_VERTICAL)
                marginStart = dp(4)
            }
            setColorFilter(Color.parseColor("#5F6368"))
            visibility = View.GONE
        }
        addressBarLayout.addView(sslIcon)

        // Menu button (3 dots)
        menuButton = ImageView(this).apply {
            id = View.generateViewId()
            layoutParams = RelativeLayout.LayoutParams(dp(36), dp(36)).apply {
                addRule(RelativeLayout.ALIGN_PARENT_END)
                addRule(RelativeLayout.CENTER_VERTICAL)
                marginEnd = dp(4)
            }
            setPadding(dp(6), dp(6), dp(6), dp(6))
            setColorFilter(Color.parseColor("#5F6368"))
            setOnClickListener { showMainMenu() }
        }
        // Draw 3 dots
        val dotsDrawable = createDotsDrawable()
        menuButton.setImageDrawable(dotsDrawable)
        addressBarLayout.addView(menuButton)

        // Address input
        addressBarInput = EditText(this).apply {
            id = View.generateViewId()
            layoutParams = RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.MATCH_PARENT,
                RelativeLayout.LayoutParams.MATCH_PARENT
            ).apply {
                addRule(RelativeLayout.END_OF, sslIcon.id)
                addRule(RelativeLayout.START_OF, menuButton.id)
                marginStart = dp(8)
                marginEnd = dp(4)
            }
            background = null
            hint = "Искать или ввести адрес"
            setHintTextColor(Color.parseColor("#9AA0A6"))
            setTextColor(Color.parseColor("#202124"))
            textSize = 15f
            maxLines = 1
            isSingleLine = true
            imeOptions = EditorInfo.IME_ACTION_GO
            inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_URI

            setOnEditorActionListener { _, actionId, _ ->
                if (actionId == EditorInfo.IME_ACTION_GO || actionId == EditorInfo.IME_ACTION_SEARCH) {
                    navigateTo(text.toString().trim())
                    hideKeyboard()
                    true
                } else false
            }

            setOnFocusChangeListener { _, hasFocus ->
                if (hasFocus) {
                    selectAll()
                    val bg = addressBarLayout.background as? GradientDrawable
                    bg?.setStroke(dp(2), Color.parseColor("#6C5CE7"))
                } else {
                    val bg = addressBarLayout.background as? GradientDrawable
                    bg?.setStroke(dp(1), Color.parseColor("#E0E0E0"))
                }
            }
        }
        addressBarLayout.addView(addressBarInput)

        mainContainer.addView(addressBarLayout)
    }

    private fun createDotsDrawable(): Drawable {
        return object : Drawable() {
            private val paint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
                color = Color.parseColor("#5F6368")
                style = Paint.Style.FILL
            }
            override fun draw(canvas: Canvas) {
                val cx = bounds.width() / 2f
                val cy = bounds.height() / 2f
                val r = bounds.width() * 0.06f
                val gap = bounds.height() * 0.2f
                canvas.drawCircle(cx, cy - gap, r, paint)
                canvas.drawCircle(cx, cy, r, paint)
                canvas.drawCircle(cx, cy + gap, r, paint)
            }
            override fun setAlpha(alpha: Int) { paint.alpha = alpha }
            override fun setColorFilter(cf: ColorFilter?) { paint.colorFilter = cf }
            override fun getOpacity(): Int = PixelFormat.TRANSLUCENT
        }
    }

    // ========================= HOME PAGE =========================

    private fun buildHomePage() {
        homePageView = ScrollView(this).apply {
            layoutParams = FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT
            )
            isVerticalScrollBarEnabled = false
        }

        val contentLayout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
        }

        // Gradient background
        val gradientBg = GradientDrawable(
            GradientDrawable.Orientation.TOP_BOTTOM,
            intArrayOf(
                Color.parseColor("#E8F0FE"),
                Color.parseColor("#F3E8FF"),
                Color.parseColor("#FFFFFF")
            )
        )
        contentLayout.background = gradientBg

        // === Search header ===
        val searchCard = buildSearchCard()
        contentLayout.addView(searchCard)

        // === Speed Dials ===
        val speedDialSection = buildSpeedDialSection()
        contentLayout.addView(speedDialSection)

        // === News categories tabs ===
        val newsCatTabs = buildNewsCategoryTabs()
        contentLayout.addView(newsCatTabs)

        // === News header ===
        val newsHeader = buildNewsHeader()
        contentLayout.addView(newsHeader)

        // === News cards container ===
        newsContainer = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply { setMargins(dp(12), 0, dp(12), dp(16)) }
        }
        contentLayout.addView(newsContainer)
        loadNewsItems()

        // === Bottom spacer ===
        val spacer = View(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, dp(20)
            )
        }
        contentLayout.addView(spacer)

        homePageView.addView(contentLayout)
    }

    private fun buildSearchCard(): LinearLayout {
        return LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, dp(56)
            ).apply { setMargins(dp(16), dp(16), dp(16), dp(8)) }

            val cardBg = GradientDrawable().apply {
                cornerRadius = dp(28).toFloat()
                setColor(Color.WHITE)
            }
            background = cardBg
            elevation = dp(4).toFloat()
            setPadding(dp(20), 0, dp(16), 0)

            // Search icon
            val searchIcon = TextView(this@MainActivity).apply {
                text = "🔍"
                textSize = 18f
                layoutParams = LinearLayout.LayoutParams(dp(32), dp(32)).apply {
                    gravity = Gravity.CENTER_VERTICAL
                }
                gravity = Gravity.CENTER
            }
            addView(searchIcon)

            // Hint text
            val hintText = TextView(this@MainActivity).apply {
                text = "Искать или задать вопрос"
                setTextColor(Color.parseColor("#9AA0A6"))
                textSize = 16f
                layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f).apply {
                    marginStart = dp(8)
                    gravity = Gravity.CENTER_VERTICAL
                }
            }
            addView(hintText)

            // QR icon
            val qrIcon = TextView(this@MainActivity).apply {
                text = "⊞"
                textSize = 22f
                setTextColor(Color.parseColor("#5F6368"))
                layoutParams = LinearLayout.LayoutParams(dp(32), dp(32))
                gravity = Gravity.CENTER
            }
            addView(qrIcon)

            setOnClickListener {
                addressBarInput.requestFocus()
                showKeyboard()
            }
        }
    }

    private fun buildSpeedDialSection(): LinearLayout {
        return LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply { setMargins(dp(8), dp(8), dp(8), dp(4)) }

            // Grid of speed dials
            val gridLayout = GridLayout(this@MainActivity).apply {
                columnCount = 4
                layoutParams = LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.MATCH_PARENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT
                )
                setPadding(dp(4), 0, dp(4), 0)
            }

            speedDials.take(8).forEachIndexed { index, dial ->
                val item = buildSpeedDialItem(dial)
                val params = GridLayout.LayoutParams().apply {
                    width = 0
                    height = GridLayout.LayoutParams.WRAP_CONTENT
                    columnSpec = GridLayout.spec(index % 4, 1f)
                    rowSpec = GridLayout.spec(index / 4)
                    setMargins(dp(4), dp(4), dp(4), dp(4))
                }
                item.layoutParams = params
                gridLayout.addView(item)
            }

            // Add button
            val addItem = buildAddSpeedDialItem()
            val addParams = GridLayout.LayoutParams().apply {
                width = 0
                height = GridLayout.LayoutParams.WRAP_CONTENT
                columnSpec = GridLayout.spec(speedDials.size % 4, 1f)
                rowSpec = GridLayout.spec(speedDials.size / 4)
                setMargins(dp(4), dp(4), dp(4), dp(4))
            }
            addItem.layoutParams = addParams
            gridLayout.addView(addItem)

            addView(gridLayout)
        }
    }

    private fun buildSpeedDialItem(dial: SpeedDialItem): LinearLayout {
        return LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER_HORIZONTAL
            setPadding(dp(4), dp(8), dp(4), dp(8))

            val iconBg = GradientDrawable().apply {
                cornerRadius = dp(14).toFloat()
                setColor(Color.WHITE)
            }

            val iconContainer = FrameLayout(this@MainActivity).apply {
                layoutParams = LinearLayout.LayoutParams(dp(52), dp(52))
                background = iconBg
                elevation = dp(2).toFloat()
            }

            val letter = TextView(this@MainActivity).apply {
                text = dial.title.take(1).uppercase()
                textSize = 22f
                setTextColor(dial.iconColor)
                typeface = Typeface.DEFAULT_BOLD
                gravity = Gravity.CENTER
                layoutParams = FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.MATCH_PARENT,
                    FrameLayout.LayoutParams.MATCH_PARENT
                )
            }
            iconContainer.addView(letter)
            addView(iconContainer)

            val title = TextView(this@MainActivity).apply {
                text = dial.title
                textSize = 11f
                setTextColor(Color.parseColor("#5F6368"))
                gravity = Gravity.CENTER
                maxLines = 1
                ellipsize = TextUtils.TruncateAt.END
                layoutParams = LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.MATCH_PARENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT
                ).apply { topMargin = dp(4) }
            }
            addView(title)

            setOnClickListener { navigateTo(dial.url) }
            setOnLongClickListener {
                showSpeedDialOptions(dial)
                true
            }
        }
    }

    private fun buildAddSpeedDialItem(): LinearLayout {
        return LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER_HORIZONTAL
            setPadding(dp(4), dp(8), dp(4), dp(8))

            val iconBg = GradientDrawable().apply {
                cornerRadius = dp(14).toFloat()
                setColor(Color.WHITE)
                setStroke(dp(1), Color.parseColor("#E0E0E0"))
            }

            val iconContainer = FrameLayout(this@MainActivity).apply {
                layoutParams = LinearLayout.LayoutParams(dp(52), dp(52))
                background = iconBg
                elevation = dp(1).toFloat()
            }

            val plus = TextView(this@MainActivity).apply {
                text = "+"
                textSize = 26f
                setTextColor(Color.parseColor("#9AA0A6"))
                gravity = Gravity.CENTER
                layoutParams = FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.MATCH_PARENT,
                    FrameLayout.LayoutParams.MATCH_PARENT
                )
            }
            iconContainer.addView(plus)
            addView(iconContainer)

            val title = TextView(this@MainActivity).apply {
                text = "Добавить"
                textSize = 11f
                setTextColor(Color.parseColor("#9AA0A6"))
                gravity = Gravity.CENTER
                layoutParams = LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.MATCH_PARENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT
                ).apply { topMargin = dp(4) }
            }
            addView(title)

            setOnClickListener { showAddSpeedDialDialog() }
        }
    }

    private fun buildNewsCategoryTabs(): HorizontalScrollView {
        val scrollView = HorizontalScrollView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply { setMargins(0, dp(12), 0, dp(4)) }
            isHorizontalScrollBarEnabled = false
        }

        val tabsRow = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            setPadding(dp(12), 0, dp(12), 0)
        }

        val categories = listOf(
            "all" to "Для вас",
            "politics" to "Политика",
            "sports" to "Спорт",
            "society" to "Общество",
            "tech" to "Технологии",
            "science" to "Наука"
        )

        categories.forEach { (key, label) ->
            val tab = TextView(this).apply {
                text = label
                textSize = 14f
                setPadding(dp(16), dp(10), dp(16), dp(10))
                
                if (key == currentNewsCategory) {
                    setTextColor(Color.parseColor("#202124"))
                    typeface = Typeface.DEFAULT_BOLD
                    val underline = GradientDrawable().apply {
                        setColor(Color.TRANSPARENT)
                    }
                    background = underline
                    paint.isUnderlineText = true
                } else {
                    setTextColor(Color.parseColor("#5F6368"))
                    typeface = Typeface.DEFAULT
                }

                setOnClickListener {
                    currentNewsCategory = key
                    refreshHomePage()
                }
            }
            tabsRow.addView(tab)
        }

        scrollView.addView(tabsRow)
        return scrollView
    }

    private fun buildNewsHeader(): RelativeLayout {
        return RelativeLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply { setMargins(dp(16), dp(8), dp(16), dp(8)) }

            // Icon + title
            val titleRow = LinearLayout(this@MainActivity).apply {
                id = View.generateViewId()
                orientation = LinearLayout.HORIZONTAL
                gravity = Gravity.CENTER_VERTICAL
                layoutParams = RelativeLayout.LayoutParams(
                    RelativeLayout.LayoutParams.WRAP_CONTENT,
                    RelativeLayout.LayoutParams.WRAP_CONTENT
                ).apply { addRule(RelativeLayout.ALIGN_PARENT_START) }
            }

            val newsIcon = TextView(this@MainActivity).apply {
                text = "📰"
                textSize = 20f
            }
            titleRow.addView(newsIcon)

            val newsTitle = TextView(this@MainActivity).apply {
                text = "  Сводка новостей"
                textSize = 16f
                setTextColor(Color.parseColor("#202124"))
                typeface = Typeface.DEFAULT_BOLD
            }
            titleRow.addView(newsTitle)
            addView(titleRow)

            // More button
            val moreBtn = TextView(this@MainActivity).apply {
                text = "Ещё"
                textSize = 13f
                setTextColor(Color.parseColor("#6C5CE7"))
                setPadding(dp(16), dp(6), dp(16), dp(6))
                val btnBg = GradientDrawable().apply {
                    cornerRadius = dp(16).toFloat()
                    setStroke(dp(1), Color.parseColor("#6C5CE7"))
                    setColor(Color.TRANSPARENT)
                }
                background = btnBg
                layoutParams = RelativeLayout.LayoutParams(
                    RelativeLayout.LayoutParams.WRAP_CONTENT,
                    RelativeLayout.LayoutParams.WRAP_CONTENT
                ).apply {
                    addRule(RelativeLayout.ALIGN_PARENT_END)
                    addRule(RelativeLayout.CENTER_VERTICAL)
                }
                setOnClickListener {
                    navigateTo("https://news.google.com")
                }
            }
            addView(moreBtn)
        }
    }

    // ========================= NEWS LOADING =========================

    private fun loadNewsItems() {
        // Static sample news for demonstration (in real app, fetch from API)
        newsItems.clear()
        newsItems.addAll(listOf(
            NewsItem(
                "Учёные обнаружили новый вид глубоководных рыб в Марианской впадине",
                "nauka.ru",
                "",
                "https://news.google.com",
                "science"
            ),
            NewsItem(
                "Технологический прорыв: квантовый компьютер решил задачу за секунды",
                "tech.news",
                "",
                "https://news.google.com",
                "tech"
            ),
            NewsItem(
                "Новый рекорд в лёгкой атлетике установлен на чемпионате мира",
                "sports.ru",
                "",
                "https://news.google.com",
                "sports"
            ),
            NewsItem(
                "Парламент утвердил новый закон о цифровых технологиях",
                "ria.ru",
                "",
                "https://news.google.com",
                "politics"
            ),
            NewsItem(
                "Глобальные инвестиции в возобновляемую энергетику достигли рекорда",
                "rbc.ru",
                "",
                "https://news.google.com",
                "society"
            ),
            NewsItem(
                "Новое исследование: регулярные прогулки снижают риск заболеваний на 30%",
                "health.info",
                "",
                "https://news.google.com",
                "all"
            ),
            NewsItem(
                "Искусственный интеллект научился предсказывать землетрясения",
                "science.daily",
                "",
                "https://news.google.com",
                "science"
            ),
            NewsItem(
                "Фондовый рынок продемонстрировал рекордный рост за квартал",
                "finance.ru",
                "",
                "https://news.google.com",
                "society"
            )
        ))
        populateNewsCards()

        // Also try to load real news from RSS
        thread {
            try {
                val conn = URL("https://news.google.com/rss?hl=ru&gl=RU&ceid=RU:ru").openConnection() as HttpURLConnection
                conn.connectTimeout = 5000
                conn.readTimeout = 5000
                val response = conn.inputStream.bufferedReader().readText()
                conn.disconnect()

                // Simple XML parsing for titles
                val titleRegex = Regex("<title><!\\[CDATA\\[(.+?)]]></title>|<title>(.+?)</title>")
                val sourceRegex = Regex("<source[^>]*>(.+?)</source>")
                val linkRegex = Regex("<link>(.+?)</link>")

                val matches = titleRegex.findAll(response).toList()
                val sources = sourceRegex.findAll(response).toList()
                val links = linkRegex.findAll(response).toList()

                if (matches.size > 2) {
                    val realNews = mutableListOf<NewsItem>()
                    for (i in 2 until minOf(matches.size, 12)) {
                        val title = matches[i].groupValues[1].ifEmpty { matches[i].groupValues[2] }
                        val source = if (i - 2 < sources.size) sources[i - 2].groupValues[1] else "Google News"
                        val link = if (i < links.size) links[i].groupValues[1] else "https://news.google.com"
                        realNews.add(NewsItem(title, source, "", link, "all"))
                    }
                    if (realNews.isNotEmpty()) {
                        runOnUiThread {
                            newsItems.clear()
                            newsItems.addAll(realNews)
                            populateNewsCards()
                        }
                    }
                }
            } catch (e: Exception) {
                // Keep static news
            }
        }
    }

    private fun populateNewsCards() {
        newsContainer?.removeAllViews()
        val filtered = if (currentNewsCategory == "all") newsItems
                       else newsItems.filter { it.category == currentNewsCategory || currentNewsCategory == "all" }

        if (filtered.isEmpty()) {
            newsItems.forEach { addNewsCard(it) }
        } else {
            filtered.forEach { addNewsCard(it) }
        }
    }

    private fun addNewsCard(news: NewsItem) {
        val cardColors = intArrayOf(
            Color.parseColor("#1A237E"),
            Color.parseColor("#004D40"),
            Color.parseColor("#BF360C"),
            Color.parseColor("#4A148C"),
            Color.parseColor("#263238"),
            Color.parseColor("#3E2723")
        )
        val randomColor = cardColors[newsItems.indexOf(news) % cardColors.size]

        val card = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply { setMargins(0, dp(6), 0, dp(6)) }

            val cardBg = GradientDrawable().apply {
                cornerRadius = dp(16).toFloat()
                val gradColors = intArrayOf(randomColor, Color.parseColor("#37474F"))
                colors = gradColors
                orientation = GradientDrawable.Orientation.BL_TR
            }
            background = cardBg
            elevation = dp(3).toFloat()
            setPadding(dp(16), dp(20), dp(16), dp(16))
            minimumHeight = dp(120)
        }

        // Source label
        val sourceLabel = TextView(this).apply {
            text = news.source
            textSize = 11f
            setTextColor(Color.parseColor("#B0BEC5"))
            setPadding(dp(8), dp(3), dp(8), dp(3))
            val labelBg = GradientDrawable().apply {
                cornerRadius = dp(4).toFloat()
                setColor(Color.parseColor("#40000000"))
            }
            background = labelBg
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply { bottomMargin = dp(8) }
        }
        card.addView(sourceLabel)

        // Spacer
        val spacer = View(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, 0, 1f
            )
        }
        card.addView(spacer)

        // Title
        val title = TextView(this).apply {
            text = news.title
            textSize = 16f
            setTextColor(Color.WHITE)
            typeface = Typeface.DEFAULT_BOLD
            maxLines = 3
            lineHeight = dp(22)
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
            setShadowLayer(2f, 1f, 1f, Color.parseColor("#80000000"))
        }
        card.addView(title)

        card.setOnClickListener {
            navigateTo(news.link)
        }

        newsContainer?.addView(card)
    }

    // ========================= WEB VIEW =========================

    @SuppressLint("SetJavaScriptEnabled")
    private fun createWebView(): WebView {
        return WebView(this).apply {
            layoutParams = FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT
            )
            settings.apply {
                javaScriptEnabled = true
                domStorageEnabled = true
                databaseEnabled = true
                setSupportZoom(true)
                builtInZoomControls = true
                displayZoomControls = false
                loadWithOverviewMode = true
                useWideViewPort = true
                allowContentAccess = true
                allowFileAccess = true
                setSupportMultipleWindows(true)
                javaScriptCanOpenWindowsAutomatically = true
                mediaPlaybackRequiresUserGesture = false
                mixedContentMode = WebSettings.MIXED_CONTENT_ALWAYS_ALLOW
                cacheMode = WebSettings.LOAD_DEFAULT
                
                if (isDesktopMode) {
                    userAgentString = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
                }

                if (isIncognitoMode) {
                    cacheMode = WebSettings.LOAD_NO_CACHE
                }
            }

            webViewClient = object : WebViewClient() {
                override fun onPageStarted(view: WebView?, url: String?, favicon: Bitmap?) {
                    super.onPageStarted(view, url, favicon)
                    progressBar.visibility = View.VISIBLE
                    progressBar.progress = 10
                    url?.let { addressBarInput.setText(it) }
                }

                override fun onPageFinished(view: WebView?, url: String?) {
                    super.onPageFinished(view, url)
                    progressBar.visibility = View.GONE
                    swipeRefresh.isRefreshing = false
                    
                    val tab = currentTab()
                    tab?.title = view?.title ?: "Без названия"
                    tab?.url = url ?: ""
                    url?.let { addressBarInput.setText(it) }

                    // Add to history
                    if (!isIncognitoMode && url != null && url.startsWith("http")) {
                        history.add(HistoryItem(
                            view?.title ?: url,
                            url
                        ))
                    }

                    // Update SSL icon
                    if (url?.startsWith("https") == true) {
                        sslIcon.visibility = View.VISIBLE
                        sslIcon.setColorFilter(Color.parseColor("#0F9D58"))
                    } else if (url?.startsWith("http") == true) {
                        sslIcon.visibility = View.VISIBLE
                        sslIcon.setColorFilter(Color.parseColor("#F4B400"))
                    } else {
                        sslIcon.visibility = View.GONE
                    }

                    updateTabCount()
                }

                override fun shouldOverrideUrlLoading(view: WebView?, request: WebResourceRequest?): Boolean {
                    val url = request?.url?.toString() ?: return false
                    if (url.startsWith("http://") || url.startsWith("https://")) {
                        return false
                    }
                    // Handle intents
                    try {
                        val intent = Intent(Intent.ACTION_VIEW, Uri.parse(url))
                        startActivity(intent)
                    } catch (_: Exception) {}
                    return true
                }

                override fun onReceivedSslError(view: WebView?, handler: SslErrorHandler?, error: SslError?) {
                    AlertDialog.Builder(this@MainActivity)
                        .setTitle("⚠️ Ошибка SSL")
                        .setMessage("Сертификат безопасности этого сайта недействителен. Продолжить?")
                        .setPositiveButton("Продолжить") { _, _ -> handler?.proceed() }
                        .setNegativeButton("Отмена") { _, _ -> handler?.cancel() }
                        .show()
                }
            }

            webChromeClient = object : WebChromeClient() {
                override fun onProgressChanged(view: WebView?, newProgress: Int) {
                    super.onProgressChanged(view, newProgress)
                    progressBar.progress = newProgress
                    if (newProgress >= 100) {
                        progressBar.visibility = View.GONE
                    }
                }

                override fun onReceivedTitle(view: WebView?, title: String?) {
                    super.onReceivedTitle(view, title)
                    currentTab()?.title = title ?: "Без названия"
                }

                override fun onReceivedIcon(view: WebView?, icon: Bitmap?) {
                    super.onReceivedIcon(view, icon)
                    currentTab()?.favicon = icon
                }

                override fun onCreateWindow(view: WebView?, isDialog: Boolean, isUserGesture: Boolean, resultMsg: Message?): Boolean {
                    val newTab = createNewTab()
                    val transport = resultMsg?.obj as? WebView.WebViewTransport
                    transport?.webView = newTab.webView
                    resultMsg?.sendToTarget()
                    return true
                }

                // File chooser
                override fun onShowFileChooser(
                    webView: WebView?,
                    filePathCallback: ValueCallback<Array<Uri>>?,
                    fileChooserParams: FileChooserParams?
                ): Boolean {
                    try {
                        val intent = fileChooserParams?.createIntent()
                        startActivityForResult(intent, 1001)
                    } catch (_: Exception) {}
                    return true
                }

                // Fullscreen video
                override fun onShowCustomView(view: View?, callback: CustomViewCallback?) {
                    super.onShowCustomView(view, callback)
                    rootLayout.addView(view)
                    mainContainer.visibility = View.GONE
                }

                override fun onHideCustomView() {
                    super.onHideCustomView()
                    mainContainer.visibility = View.VISIBLE
                    if (rootLayout.childCount > 1) {
                        rootLayout.removeViewAt(rootLayout.childCount - 1)
                    }
                }

                override fun onGeolocationPermissionsShowPrompt(
                    origin: String?,
                    callback: GeolocationPermissions.Callback?
                ) {
                    AlertDialog.Builder(this@MainActivity)
                        .setTitle("📍 Геолокация")
                        .setMessage("Сайт $origin запрашивает доступ к вашему местоположению")
                        .setPositiveButton("Разрешить") { _, _ -> callback?.invoke(origin, true, false) }
                        .setNegativeButton("Отклонить") { _, _ -> callback?.invoke(origin, false, false) }
                        .show()
                }
            }

            setDownloadListener { url, userAgent, contentDisposition, mimeType, contentLength ->
                try {
                    val request = DownloadManager.Request(Uri.parse(url))
                    request.setMimeType(mimeType)
                    request.addRequestHeader("User-Agent", userAgent)
                    request.setDescription("Загрузка файла...")
                    request.setTitle(URLUtil.guessFileName(url, contentDisposition, mimeType))
                    request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED)
                    request.setDestinationInExternalPublicDir(
                        Environment.DIRECTORY_DOWNLOADS,
                        URLUtil.guessFileName(url, contentDisposition, mimeType)
                    )
                    val dm = getSystemService(DOWNLOAD_SERVICE) as DownloadManager
                    dm.enqueue(request)
                    Toast.makeText(this@MainActivity, "⬇️ Загрузка начата", Toast.LENGTH_SHORT).show()
                } catch (e: Exception) {
                    Toast.makeText(this@MainActivity, "Ошибка загрузки: ${e.message}", Toast.LENGTH_SHORT).show()
                }
            }

            setOnLongClickListener {
                val hitResult = hitTestResult
                when (hitResult.type) {
                    WebView.HitTestResult.IMAGE_TYPE,
                    WebView.HitTestResult.SRC_IMAGE_ANCHOR_TYPE -> {
                        showImageContextMenu(hitResult.extra ?: "")
                        true
                    }
                    WebView.HitTestResult.SRC_ANCHOR_TYPE,
                    WebView.HitTestResult.ANCHOR_TYPE -> {
                        showLinkContextMenu(hitResult.extra ?: "")
                        true
                    }
                    else -> false
                }
            }
        }
    }

    // ========================= TAB MANAGEMENT =========================

    private fun createNewTab(url: String = ""): BrowserTab {
        val tab = BrowserTab()
        val webView = createWebView()
        tab.webView = webView
        tabs.add(tab)
        currentTabIndex = tabs.size - 1

        webViewContainer.removeAllViews()
        webViewContainer.addView(webView)

        if (url.isNotEmpty()) {
            navigateTo(url)
        } else {
            showHomePage()
        }

        updateTabCount()
        return tab
    }

    private fun switchToTab(index: Int) {
        if (index < 0 || index >= tabs.size) return
        currentTabIndex = index
        val tab = tabs[index]

        webViewContainer.removeAllViews()
        tab.webView?.let {
            (it.parent as? ViewGroup)?.removeView(it)
            webViewContainer.addView(it)
        }

        if (tab.isHomePage) {
            showHomePage()
        } else {
            homePageView.visibility = View.GONE
            webViewContainer.visibility = View.VISIBLE
            addressBarInput.setText(tab.url)
        }

        updateTabCount()
    }

    private fun closeTab(index: Int) {
        if (tabs.size <= 1) {
            tabs[0].webView?.loadUrl("about:blank")
            tabs[0].isHomePage = true
            tabs[0].title = "Новая вкладка"
            tabs[0].url = ""
            showHomePage()
            return
        }

        tabs[index].webView?.destroy()
        tabs.removeAt(index)
        
        if (currentTabIndex >= tabs.size) {
            currentTabIndex = tabs.size - 1
        }
        switchToTab(currentTabIndex)
    }

    private fun updateTabCount() {
        tabCountText.text = tabs.size.toString()
    }

    // ========================= NAVIGATION =========================

    private fun navigateTo(input: String) {
        var url = input.trim()
        if (url.isEmpty()) return

        currentTab()?.isHomePage = false
        homePageView.visibility = View.GONE
        webViewContainer.visibility = View.VISIBLE

        // Determine if URL or search query
        url = when {
            url.startsWith("http://") || url.startsWith("https://") -> url
            url.contains(".") && !url.contains(" ") -> "https://$url"
            else -> "https://www.google.com/search?q=${Uri.encode(url)}"
        }

        currentTab()?.url = url
        currentTab()?.webView?.loadUrl(url)
        addressBarInput.setText(url)
        addressBarInput.clearFocus()
    }

    private fun showHomePage() {
        currentTab()?.isHomePage = true
        homePageView.visibility = View.VISIBLE
        webViewContainer.visibility = View.GONE
        addressBarInput.setText("")
        addressBarInput.clearFocus()
        sslIcon.visibility = View.GONE
        progressBar.visibility = View.GONE

        val bg = addressBarLayout.background as? GradientDrawable
        bg?.setStroke(dp(1), Color.parseColor("#E0E0E0"))
    }

    private fun refreshHomePage() {
        // Rebuild home page content
        homePageView.visibility = View.GONE
        val parent = homePageView.parent as? ViewGroup
        parent?.removeView(homePageView)
        buildHomePage()
        parent?.addView(homePageView)
        homePageView.visibility = View.VISIBLE
        webViewContainer.visibility = View.GONE
    }

    // ========================= BOTTOM NAV BAR =========================

    private fun buildBottomNavBar() {
        bottomNavBar = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, dp(56)
            )
            setBackgroundColor(Color.WHITE)
            elevation = dp(8).toFloat()
            gravity = Gravity.CENTER_VERTICAL
        }

        // Back
        val backBtn = createNavButton("◀", "Назад") {
            val tab = currentTab()
            if (tab?.webView?.canGoBack() == true) {
                tab.webView?.goBack()
            }
        }
        bottomNavBar.addView(backBtn)

        // Forward / Refresh
        val refreshBtn = createNavButton("⟳", "Обновить") {
            val tab = currentTab()
            if (tab?.isHomePage == true) {
                refreshHomePage()
            } else {
                tab?.webView?.reload()
            }
        }
        bottomNavBar.addView(refreshBtn)

        // Search / Home
        val searchBtn = createNavButton("🏠", "Домой") {
            showHomePage()
        }
        bottomNavBar.addView(searchBtn)

        // Tabs
        val tabBtn = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.MATCH_PARENT, 1f)

            val tabIcon = FrameLayout(this@MainActivity).apply {
                layoutParams = LinearLayout.LayoutParams(dp(24), dp(24)).apply {
                    gravity = Gravity.CENTER_HORIZONTAL
                }
                val tabBorder = GradientDrawable().apply {
                    cornerRadius = dp(4).toFloat()
                    setStroke(dp(2), Color.parseColor("#5F6368"))
                    setColor(Color.TRANSPARENT)
                }
                background = tabBorder
            }

            tabCountText = TextView(this@MainActivity).apply {
                text = "1"
                textSize = 10f
                setTextColor(Color.parseColor("#5F6368"))
                gravity = Gravity.CENTER
                typeface = Typeface.DEFAULT_BOLD
                layoutParams = FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.MATCH_PARENT,
                    FrameLayout.LayoutParams.MATCH_PARENT
                )
            }
            tabIcon.addView(tabCountText)
            addView(tabIcon)

            val label = TextView(this@MainActivity).apply {
                text = "Вкладки"
                textSize = 10f
                setTextColor(Color.parseColor("#5F6368"))
                gravity = Gravity.CENTER
            }
            addView(label)

            setOnClickListener { showTabManager() }
        }
        bottomNavBar.addView(tabBtn)

        // Profile / Settings
        val profileBtn = createNavButton("⚙", "Ещё") {
            showMainMenu()
        }
        bottomNavBar.addView(profileBtn)

        mainContainer.addView(bottomNavBar)
    }

    private fun createNavButton(icon: String, label: String, onClick: () -> Unit): LinearLayout {
        return LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.MATCH_PARENT, 1f)

            val iconView = TextView(this@MainActivity).apply {
                text = icon
                textSize = 20f
                gravity = Gravity.CENTER
                layoutParams = LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT
                ).apply { gravity = Gravity.CENTER_HORIZONTAL }
            }
            addView(iconView)

            val labelView = TextView(this@MainActivity).apply {
                text = label
                textSize = 10f
                setTextColor(Color.parseColor("#5F6368"))
                gravity = Gravity.CENTER
            }
            addView(labelView)

            setOnClickListener { onClick() }
        }
    }

    // ========================= TAB MANAGER =========================

    private fun showTabManager() {
        val dialog = AlertDialog.Builder(this, android.R.style.Theme_Material_Light_NoActionBar_Fullscreen)
        
        val layout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setBackgroundColor(Color.parseColor("#F5F5F5"))
            setPadding(dp(16), dp(40), dp(16), dp(16))
        }

        // Header
        val header = RelativeLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, dp(48)
            )
        }

        val titleTv = TextView(this).apply {
            text = "Вкладки (${tabs.size})"
            textSize = 20f
            setTextColor(Color.parseColor("#202124"))
            typeface = Typeface.DEFAULT_BOLD
            layoutParams = RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT
            ).apply { addRule(RelativeLayout.CENTER_VERTICAL) }
        }
        header.addView(titleTv)

        val newTabBtn = TextView(this).apply {
            text = "+ Новая"
            textSize = 14f
            setTextColor(Color.parseColor("#6C5CE7"))
            typeface = Typeface.DEFAULT_BOLD
            layoutParams = RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                addRule(RelativeLayout.ALIGN_PARENT_END)
                addRule(RelativeLayout.CENTER_VERTICAL)
            }
            setPadding(dp(12), dp(8), dp(12), dp(8))
        }
        header.addView(newTabBtn)
        layout.addView(header)

        val scrollView = ScrollView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, 0, 1f
            )
        }
        val tabList = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
        }

        val alertDialog = AlertDialog.Builder(this)
            .setView(layout)
            .create()

        tabs.forEachIndexed { index, tab ->
            val tabCard = buildTabCard(tab, index, alertDialog)
            tabList.addView(tabCard)
        }

        scrollView.addView(tabList)
        layout.addView(scrollView)

        newTabBtn.setOnClickListener {
            createNewTab()
            alertDialog.dismiss()
        }

        alertDialog.show()
    }

    private fun buildTabCard(tab: BrowserTab, index: Int, dialog: AlertDialog): LinearLayout {
        return LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                dp(72)
            ).apply { setMargins(0, dp(4), 0, dp(4)) }

            val bg = GradientDrawable().apply {
                cornerRadius = dp(12).toFloat()
                setColor(if (index == currentTabIndex) Color.parseColor("#E8F0FE") else Color.WHITE)
            }
            background = bg
            elevation = dp(2).toFloat()
            setPadding(dp(12), dp(8), dp(8), dp(8))

            // Tab favicon / letter
            val iconView = TextView(this@MainActivity).apply {
                text = if (tab.isHomePage) "🏠" else tab.title.take(1).uppercase()
                textSize = 18f
                gravity = Gravity.CENTER
                layoutParams = LinearLayout.LayoutParams(dp(40), dp(40))
                if (!tab.isHomePage) {
                    setTextColor(Color.WHITE)
                    val circleBg = GradientDrawable().apply {
                        shape = GradientDrawable.OVAL
                        setColor(Color.parseColor("#6C5CE7"))
                    }
                    background = circleBg
                }
            }
            addView(iconView)

            // Title + URL
            val textCol = LinearLayout(this@MainActivity).apply {
                orientation = LinearLayout.VERTICAL
                layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f).apply {
                    marginStart = dp(12)
                }
            }

            val titleView = TextView(this@MainActivity).apply {
                text = if (tab.isHomePage) "Новая вкладка" else tab.title
                textSize = 14f
                setTextColor(Color.parseColor("#202124"))
                maxLines = 1
                ellipsize = TextUtils.TruncateAt.END
                typeface = Typeface.DEFAULT_BOLD
            }
            textCol.addView(titleView)

            if (!tab.isHomePage) {
                val urlView = TextView(this@MainActivity).apply {
                    text = tab.url
                    textSize = 12f
                    setTextColor(Color.parseColor("#5F6368"))
                    maxLines = 1
                    ellipsize = TextUtils.TruncateAt.END
                }
                textCol.addView(urlView)
            }
            addView(textCol)

            // Close button
            val closeBtn = TextView(this@MainActivity).apply {
                text = "✕"
                textSize = 18f
                setTextColor(Color.parseColor("#5F6368"))
                gravity = Gravity.CENTER
                layoutParams = LinearLayout.LayoutParams(dp(36), dp(36))
                setOnClickListener {
                    closeTab(index)
                    dialog.dismiss()
                    if (tabs.isNotEmpty()) showTabManager()
                }
            }
            addView(closeBtn)

            setOnClickListener {
                switchToTab(index)
                dialog.dismiss()
            }
        }
    }

    // ========================= MAIN MENU =========================

    private fun showMainMenu() {
        val menuItems = mutableListOf(
            "🔖 Закладки",
            "📋 История",
            "⬇️ Загрузки",
            "🔍 Найти на странице",
            "📱 → 🖥️ Версия для ПК",
            "🕶️ Инкогнито",
            "📤 Поделиться",
            "⭐ Добавить в закладки",
            "🔧 Настройки",
            "ℹ️ О браузере"
        )

        val dialog = AlertDialog.Builder(this)
            .setItems(menuItems.toTypedArray()) { _, which ->
                when (which) {
                    0 -> showBookmarks()
                    1 -> showHistory()
                    2 -> openDownloads()
                    3 -> showFindOnPage()
                    4 -> toggleDesktopMode()
                    5 -> toggleIncognitoMode()
                    6 -> sharePage()
                    7 -> addBookmark()
                    8 -> showSettings()
                    9 -> showAbout()
                }
            }
            .create()
        dialog.show()
    }

    // ========================= BOOKMARKS =========================

    private fun showBookmarks() {
        if (bookmarks.isEmpty()) {
            Toast.makeText(this, "Закладок пока нет", Toast.LENGTH_SHORT).show()
            return
        }

        val items = bookmarks.map { "${it.title}\n${it.url}" }.toTypedArray()
        AlertDialog.Builder(this)
            .setTitle("🔖 Закладки")
            .setItems(items) { _, which ->
                navigateTo(bookmarks[which].url)
            }
            .setNegativeButton("Закрыть", null)
            .show()
    }

    private fun addBookmark() {
        val tab = currentTab() ?: return
        if (tab.isHomePage) {
            Toast.makeText(this, "Нельзя добавить домашнюю страницу", Toast.LENGTH_SHORT).show()
            return
        }

        // Check if already bookmarked
        if (bookmarks.any { it.url == tab.url }) {
            Toast.makeText(this, "Уже в закладках", Toast.LENGTH_SHORT).show()
            return
        }

        bookmarks.add(BookmarkItem(tab.title, tab.url))
        saveData()
        Toast.makeText(this, "⭐ Добавлено в закладки", Toast.LENGTH_SHORT).show()
    }

    // ========================= HISTORY =========================

    private fun showHistory() {
        if (history.isEmpty()) {
            Toast.makeText(this, "История пуста", Toast.LENGTH_SHORT).show()
            return
        }

        val dateFormat = SimpleDateFormat("dd.MM HH:mm", Locale.getDefault())
        val items = history.reversed().take(50)
            .map { "${it.title}\n${dateFormat.format(Date(it.timestamp))}" }.toTypedArray()

        AlertDialog.Builder(this)
            .setTitle("📋 История")
            .setItems(items) { _, which ->
                navigateTo(history.reversed()[which].url)
            }
            .setNeutralButton("Очистить") { _, _ ->
                history.clear()
                saveData()
                Toast.makeText(this, "🗑️ История очищена", Toast.LENGTH_SHORT).show()
            }
            .setNegativeButton("Закрыть", null)
            .show()
    }

    // ========================= OTHER FEATURES =========================

    private fun openDownloads() {
        try {
            val intent = Intent(DownloadManager.ACTION_VIEW_DOWNLOADS)
            startActivity(intent)
        } catch (_: Exception) {
            Toast.makeText(this, "Не удалось открыть загрузки", Toast.LENGTH_SHORT).show()
        }
    }

    private fun showFindOnPage() {
        val tab = currentTab()
        if (tab?.isHomePage == true) return

        val input = EditText(this).apply {
            hint = "Текст для поиска"
            setPadding(dp(16), dp(12), dp(16), dp(12))
        }

        AlertDialog.Builder(this)
            .setTitle("🔍 Найти на странице")
            .setView(input)
            .setPositiveButton("Найти") { _, _ ->
                val query = input.text.toString()
                if (query.isNotEmpty()) {
                    tab?.webView?.findAllAsync(query)
                }
            }
            .setNeutralButton("Далее") { _, _ ->
                tab?.webView?.findNext(true)
            }
            .setNegativeButton("Закрыть") { _, _ ->
                tab?.webView?.clearMatches()
            }
            .show()
    }

    private fun toggleDesktopMode() {
        isDesktopMode = !isDesktopMode
        val tab = currentTab()
        tab?.webView?.settings?.apply {
            userAgentString = if (isDesktopMode) {
                "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"
            } else {
                WebSettings.getDefaultUserAgent(this@MainActivity)
            }
        }
        tab?.webView?.reload()
        Toast.makeText(this,
            if (isDesktopMode) "🖥️ Версия для ПК" else "📱 Мобильная версия",
            Toast.LENGTH_SHORT
        ).show()
    }

    private fun toggleIncognitoMode() {
        isIncognitoMode = !isIncognitoMode
        Toast.makeText(this,
            if (isIncognitoMode) "🕶️ Режим инкогнито включён" else "Режим инкогнито выключен",
            Toast.LENGTH_SHORT
        ).show()
        
        if (isIncognitoMode) {
            createNewTab()
            val bar = addressBarLayout.background as? GradientDrawable
            bar?.setColor(Color.parseColor("#303030"))
            addressBarInput.setTextColor(Color.WHITE)
            addressBarInput.setHintTextColor(Color.parseColor("#AAAAAA"))
        } else {
            val bar = addressBarLayout.background as? GradientDrawable
            bar?.setColor(Color.parseColor("#F1F3F4"))
            addressBarInput.setTextColor(Color.parseColor("#202124"))
            addressBarInput.setHintTextColor(Color.parseColor("#9AA0A6"))
        }
    }

    private fun sharePage() {
        val tab = currentTab() ?: return
        if (tab.isHomePage) return
        val intent = Intent(Intent.ACTION_SEND).apply {
            type = "text/plain"
            putExtra(Intent.EXTRA_TEXT, tab.url)
            putExtra(Intent.EXTRA_SUBJECT, tab.title)
        }
        startActivity(Intent.createChooser(intent, "Поделиться"))
    }

    private fun showSettings() {
        val options = arrayOf(
            "🗑️ Очистить кэш",
            "🍪 Очистить куки",
            "📋 Очистить историю",
            "🔖 Очистить закладки",
            "🏠 Сбросить экспресс-панель"
        )

        AlertDialog.Builder(this)
            .setTitle("🔧 Настройки")
            .setItems(options) { _, which ->
                when (which) {
                    0 -> {
                        currentTab()?.webView?.clearCache(true)
                        Toast.makeText(this, "Кэш очищен", Toast.LENGTH_SHORT).show()
                    }
                    1 -> {
                        CookieManager.getInstance().removeAllCookies(null)
                        Toast.makeText(this, "Куки очищены", Toast.LENGTH_SHORT).show()
                    }
                    2 -> {
                        history.clear()
                        saveData()
                        Toast.makeText(this, "История очищена", Toast.LENGTH_SHORT).show()
                    }
                    3 -> {
                        bookmarks.clear()
                        saveData()
                        Toast.makeText(this, "Закладки очищены", Toast.LENGTH_SHORT).show()
                    }
                    4 -> {
                        speedDials.clear()
                        initDefaultSpeedDials()
                        saveData()
                        refreshHomePage()
                        Toast.makeText(this, "Экспресс-панель сброшена", Toast.LENGTH_SHORT).show()
                    }
                }
            }
            .setNegativeButton("Закрыть", null)
            .show()
    }

    private fun showAbout() {
        AlertDialog.Builder(this)
            .setTitle("Aurora Browser")
            .setMessage("""
                🌐 Aurora Browser v1.0
                
                Быстрый и современный мобильный браузер
                
                ✨ Возможности:
                • Управление вкладками
                • Экспресс-панель с быстрым доступом
                • Новостная лента
                • Закладки и история
                • Режим инкогнито
                • Версия для ПК
                • Загрузка файлов
                • Поиск на странице
                • Безопасное соединение (SSL)
                • Pull-to-refresh
                • Полноэкранное видео
                
                Разработано с ❤️
            """.trimIndent())
            .setPositiveButton("OK", null)
            .show()
    }

    // ========================= CONTEXT MENUS =========================

    private fun showImageContextMenu(imageUrl: String) {
        val options = arrayOf(
            "🖼️ Открыть изображение",
            "💾 Сохранить изображение",
            "📤 Поделиться ссылкой"
        )
        AlertDialog.Builder(this)
            .setTitle("Изображение")
            .setItems(options) { _, which ->
                when (which) {
                    0 -> navigateTo(imageUrl)
                    1 -> {
                        try {
                            val request = DownloadManager.Request(Uri.parse(imageUrl))
                            request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED)
                            request.setDestinationInExternalPublicDir(
                                Environment.DIRECTORY_PICTURES, "aurora_${System.currentTimeMillis()}.jpg"
                            )
                            val dm = getSystemService(DOWNLOAD_SERVICE) as DownloadManager
                            dm.enqueue(request)
                            Toast.makeText(this, "⬇️ Загрузка начата", Toast.LENGTH_SHORT).show()
                        } catch (e: Exception) {
                            Toast.makeText(this, "Ошибка: ${e.message}", Toast.LENGTH_SHORT).show()
                        }
                    }
                    2 -> {
                        val intent = Intent(Intent.ACTION_SEND).apply {
                            type = "text/plain"
                            putExtra(Intent.EXTRA_TEXT, imageUrl)
                        }
                        startActivity(Intent.createChooser(intent, "Поделиться"))
                    }
                }
            }
            .show()
    }

    private fun showLinkContextMenu(linkUrl: String) {
        val options = arrayOf(
            "🔗 Открыть",
            "📑 Открыть в новой вкладке",
            "📋 Копировать ссылку",
            "📤 Поделиться"
        )
        AlertDialog.Builder(this)
            .setTitle("Ссылка")
            .setItems(options) { _, which ->
                when (which) {
                    0 -> navigateTo(linkUrl)
                    1 -> createNewTab(linkUrl)
                    2 -> {
                        val clipboard = getSystemService(CLIPBOARD_SERVICE) as ClipboardManager
                        clipboard.setPrimaryClip(ClipData.newPlainText("url", linkUrl))
                        Toast.makeText(this, "📋 Скопировано", Toast.LENGTH_SHORT).show()
                    }
                    3 -> {
                        val intent = Intent(Intent.ACTION_SEND).apply {
                            type = "text/plain"
                            putExtra(Intent.EXTRA_TEXT, linkUrl)
                        }
                        startActivity(Intent.createChooser(intent, "Поделиться"))
                    }
                }
            }
            .show()
    }

    // ========================= SPEED DIAL DIALOGS =========================

    private fun showAddSpeedDialDialog() {
        val layout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(dp(20), dp(16), dp(20), dp(8))
        }

        val titleInput = EditText(this).apply {
            hint = "Название"
            inputType = InputType.TYPE_CLASS_TEXT
        }
        layout.addView(titleInput)

        val urlInput = EditText(this).apply {
            hint = "URL (например: google.com)"
            inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_URI
        }
        layout.addView(urlInput)

        AlertDialog.Builder(this)
            .setTitle("➕ Добавить сайт")
            .setView(layout)
            .setPositiveButton("Добавить") { _, _ ->
                val title = titleInput.text.toString().trim()
                var url = urlInput.text.toString().trim()
                if (title.isNotEmpty() && url.isNotEmpty()) {
                    if (!url.startsWith("http")) url = "https://$url"
                    speedDials.add(SpeedDialItem(title, url))
                    saveData()
                    refreshHomePage()
                }
            }
            .setNegativeButton("Отмена", null)
            .show()
    }

    private fun showSpeedDialOptions(dial: SpeedDialItem) {
        val options = arrayOf("✏️ Редактировать", "🗑️ Удалить")
        AlertDialog.Builder(this)
            .setTitle(dial.title)
            .setItems(options) { _, which ->
                when (which) {
                    0 -> showEditSpeedDialDialog(dial)
                    1 -> {
                        speedDials.remove(dial)
                        saveData()
                        refreshHomePage()
                    }
                }
            }
            .show()
    }

    private fun showEditSpeedDialDialog(dial: SpeedDialItem) {
        val layout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(dp(20), dp(16), dp(20), dp(8))
        }

        val titleInput = EditText(this).apply {
            setText(dial.title)
        }
        layout.addView(titleInput)

        val urlInput = EditText(this).apply {
            setText(dial.url)
        }
        layout.addView(urlInput)

        AlertDialog.Builder(this)
            .setTitle("✏️ Редактировать")
            .setView(layout)
            .setPositiveButton("Сохранить") { _, _ ->
                val index = speedDials.indexOf(dial)
                if (index >= 0) {
                    speedDials[index] = SpeedDialItem(
                        titleInput.text.toString().trim(),
                        urlInput.text.toString().trim(),
                        dial.iconColor
                    )
                    saveData()
                    refreshHomePage()
                }
            }
            .setNegativeButton("Отмена", null)
            .show()
    }

    // ========================= HELPERS =========================

    private fun hideKeyboard() {
        val imm = getSystemService(INPUT_METHOD_SERVICE) as InputMethodManager
        imm.hideSoftInputFromWindow(addressBarInput.windowToken, 0)
    }

    private fun showKeyboard() {
        val imm = getSystemService(INPUT_METHOD_SERVICE) as InputMethodManager
        addressBarInput.requestFocus()
        imm.showSoftInput(addressBarInput, InputMethodManager.SHOW_IMPLICIT)
    }
}
