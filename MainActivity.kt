package com.kgeu.schedule

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Color
import android.graphics.LinearGradient
import android.graphics.Shader
import android.graphics.drawable.GradientDrawable
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.text.Editable
import android.text.Html
import android.text.TextWatcher
import android.util.TypedValue
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.view.inputmethod.EditorInfo
import android.view.inputmethod.InputMethodManager
import android.widget.*
import androidx.appcompat.app.AppCompatActivity
import androidx.cardview.widget.CardView
import androidx.core.content.ContextCompat
import androidx.core.widget.NestedScrollView
import org.json.JSONArray
import org.json.JSONObject
import java.io.BufferedReader
import java.io.InputStreamReader
import java.net.HttpURLConnection
import java.net.URL
import java.net.URLEncoder
import java.text.SimpleDateFormat
import java.util.*
import kotlin.concurrent.thread

// ════════════════════════════════════════════════════════════════════
// КГЭУ Расписание — Android (все в одном файле)
// ════════════════════════════════════════════════════════════════════

class MainActivity : AppCompatActivity() {

    // ─── Constants ───
    companion object {
        const val API_BASE = "https://kabinet.kgeu.ru"
        val CORS_PROXIES = listOf(
            "https://corsproxy.io/?",
            "https://api.allorigins.win/raw?url="
        )

        // Colors
        const val COLOR_BG_DARK = 0xFF0A1628.toInt()
        const val COLOR_BG_MID = 0xFF1A365D.toInt()
        const val COLOR_BG_CARD = 0x990F172A.toInt()
        const val COLOR_PRIMARY = 0xFF1E40AF.toInt()
        const val COLOR_PRIMARY_LIGHT = 0xFF3B82F6.toInt()
        const val COLOR_ACCENT = 0xFF60A5FA.toInt()
        const val COLOR_ACCENT_LIGHT = 0xFF93C5FD.toInt()
        const val COLOR_SUCCESS = 0xFF10B981.toInt()
        const val COLOR_SUCCESS_LIGHT = 0xFF34D399.toInt()
        const val COLOR_WARNING = 0xFFF59E0B.toInt()
        const val COLOR_PURPLE = 0xFF8B5CF6.toInt()
        const val COLOR_PURPLE_LIGHT = 0xFFC084FC.toInt()
        const val COLOR_DANGER = 0xFFEF4444.toInt()
        const val COLOR_TEXT_PRIMARY = 0xFFF1F5F9.toInt()
        const val COLOR_TEXT_SECONDARY = 0xFF94A3B8.toInt()
        const val COLOR_TEXT_MUTED = 0xFF64748B.toInt()
        const val COLOR_BORDER = 0x333B82F6.toInt()
        const val COLOR_CARD_BG = 0xFF0F172A.toInt()

        const val COLOR_LEC = 0xFF3B82F6.toInt()
        const val COLOR_PR = 0xFF10B981.toInt()
        const val COLOR_LAB = 0xFF8B5CF6.toInt()
        const val COLOR_SEM = 0xFFF59E0B.toInt()

        val DAY_NAMES = arrayOf("Воскресенье","Понедельник","Вторник","Среда","Четверг","Пятница","Суббота")
        val MONTH_NAMES = arrayOf("января","февраля","марта","апреля","мая","июня",
            "июля","августа","сентября","октября","ноября","декабря")
    }

    // ─── Data holders ───
    data class EntityItem(val id: Int, val name: String, val facul: String = "", val kurs: Int = 0, val yearName: String = "", val kaf: String = "")
    data class LessonItem(
        val date: String, val start: String, val end: String, val disc: String,
        val teacher: String, val group: String, val aud: String,
        val lessonNum: Int, val isReplace: Boolean
    )

    private var allGroups = mutableListOf<EntityItem>()
    private var allTeachers = mutableListOf<EntityItem>()
    private var allAuds = mutableListOf<EntityItem>()

    private var currentTab = "Group" // Group, Teacher, Aud
    private var activeFilter: String? = null
    private var currentEntityType = ""
    private var currentEntityId = 0
    private var currentEntityName = ""
    private var currentWeekStart: String? = null
    private var currentScheduleData: JSONObject? = null
    private var searchDropdownVisible = false

    private val handler = Handler(Looper.getMainLooper())
    private var debounceRunnable: Runnable? = null

    // ─── Views ───
    private lateinit var rootScroll: NestedScrollView
    private lateinit var mainContainer: LinearLayout
    private lateinit var headerBadge: TextView
    private lateinit var tabGroup: LinearLayout
    private lateinit var searchInput: EditText
    private lateinit var dropdownContainer: LinearLayout
    private lateinit var dropdownScroll: ScrollView
    private lateinit var filtersContainer: HorizontalScrollView
    private lateinit var filtersInner: LinearLayout
    private lateinit var contentArea: FrameLayout

    private lateinit var tabGroupBtn: TextView
    private lateinit var tabTeacherBtn: TextView
    private lateinit var tabAudBtn: TextView

    // ═══════════════════════════════════════════════════
    // onCreate
    // ═══════════════════════════════════════════════════
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.statusBarColor = COLOR_BG_DARK
        window.navigationBarColor = COLOR_BG_DARK
        buildUI()
        showWelcome()
        loadAllData()
    }

    // ═══════════════════════════════════════════════════
    // UI Construction (programmatic)
    // ═══════════════════════════════════════════════════
    @SuppressLint("SetTextI18n")
    private fun buildUI() {
        val root = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setBackgroundColor(COLOR_BG_DARK)
            layoutParams = ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT)
        }

        // ── HEADER ──
        val header = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            setPadding(dp(16), dp(12), dp(16), dp(12))
            setBackgroundColor(0xCC0F172A.toInt())
        }
        val logoText = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            layoutParams = LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f)
        }
        logoText.addView(TextView(this).apply {
            text = "КГЭУ. Образование"
            setTextColor(COLOR_ACCENT)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 20f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
        })
        logoText.addView(TextView(this).apply {
            text = "РАСПИСАНИЕ ЗАНЯТИЙ"
            setTextColor(COLOR_TEXT_MUTED)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 10f)
            letterSpacing = 0.15f
        })
        header.addView(logoText)

        headerBadge = TextView(this).apply {
            text = "Загрузка..."
            setTextColor(COLOR_ACCENT)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f)
            background = makeRoundRect(0x1F3B82F6, dp(16).toFloat())
            setPadding(dp(12), dp(6), dp(12), dp(6))
        }
        header.addView(headerBadge)
        root.addView(header)

        // ── Divider under header ──
        root.addView(View(this).apply {
            setBackgroundColor(COLOR_BORDER)
            layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, dp(1))
        })

        // ── SCROLLABLE BODY ──
        rootScroll = NestedScrollView(this).apply {
            layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0, 1f)
            setBackgroundColor(Color.TRANSPARENT)
            isNestedScrollingEnabled = true
        }

        mainContainer = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(dp(14), dp(16), dp(14), dp(24))
        }

        // ── SEARCH PANEL ──
        val searchPanel = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            background = makeRoundRect(0x4D1E3A8A, dp(16).toFloat())
            setPadding(dp(16), dp(16), dp(16), dp(16))
        }

        // ── TABS ──
        tabGroup = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            background = makeRoundRect(0x4D000000, dp(10).toFloat())
            setPadding(dp(4), dp(4), dp(4), dp(4))
        }
        tabGroupBtn = makeTabButton("Группа", true)
        tabTeacherBtn = makeTabButton("Преподав.", false)
        tabAudBtn = makeTabButton("Аудитория", false)

        tabGroupBtn.setOnClickListener { switchTab("Group") }
        tabTeacherBtn.setOnClickListener { switchTab("Teacher") }
        tabAudBtn.setOnClickListener { switchTab("Aud") }

        tabGroup.addView(tabGroupBtn)
        tabGroup.addView(tabTeacherBtn)
        tabGroup.addView(tabAudBtn)
        searchPanel.addView(tabGroup, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply {
            bottomMargin = dp(12)
        })

        // ── SEARCH INPUT ──
        searchInput = EditText(this).apply {
            hint = "Введите название группы..."
            setHintTextColor(COLOR_TEXT_MUTED)
            setTextColor(COLOR_TEXT_PRIMARY)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 15f)
            background = makeRoundRect(0xCC0F172A.toInt(), dp(10).toFloat())
            setPadding(dp(16), dp(14), dp(16), dp(14))
            isSingleLine = true
            imeOptions = EditorInfo.IME_ACTION_SEARCH
        }
        searchInput.addTextChangedListener(object : TextWatcher {
            override fun beforeTextChanged(s: CharSequence?, st: Int, c: Int, a: Int) {}
            override fun onTextChanged(s: CharSequence?, st: Int, b: Int, c: Int) {}
            override fun afterTextChanged(s: Editable?) {
                debounceRunnable?.let { handler.removeCallbacks(it) }
                debounceRunnable = Runnable { performSearch() }
                handler.postDelayed(debounceRunnable!!, 150)
            }
        })
        searchPanel.addView(searchInput, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT))

        // ── SEARCH DROPDOWN ──
        dropdownScroll = ScrollView(this).apply {
            layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, dp(300))
            visibility = View.GONE
            background = makeRoundRect(0xF50A1023.toInt(), dp(12).toFloat())
        }
        dropdownContainer = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
        }
        dropdownScroll.addView(dropdownContainer)
        searchPanel.addView(dropdownScroll, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply {
            topMargin = dp(6)
        })

        // ── FILTERS (horizontal scroll) ──
        filtersContainer = HorizontalScrollView(this).apply {
            isHorizontalScrollBarEnabled = false
            visibility = View.GONE
        }
        filtersInner = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            setPadding(0, dp(10), 0, 0)
        }
        filtersContainer.addView(filtersInner)
        searchPanel.addView(filtersContainer)

        mainContainer.addView(searchPanel, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply {
            bottomMargin = dp(20)
        })

        // ── CONTENT AREA ──
        contentArea = FrameLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT)
        }
        mainContainer.addView(contentArea)

        rootScroll.addView(mainContainer)
        root.addView(rootScroll)

        setContentView(root)
    }

    // ═══════════════════════════════════════════════════
    // Tab Button Helper
    // ═══════════════════════════════════════════════════
    private fun makeTabButton(text: String, active: Boolean): TextView {
        return TextView(this).apply {
            this.text = text
            gravity = Gravity.CENTER
            setTextColor(if (active) Color.WHITE else COLOR_TEXT_MUTED)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 13f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
            background = if (active) makeGradientRoundRect(COLOR_PRIMARY, COLOR_PRIMARY_LIGHT, dp(8).toFloat()) else null
            setPadding(dp(12), dp(11), dp(12), dp(11))
            layoutParams = LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f)
        }
    }

    private fun updateTabUI() {
        fun style(btn: TextView, active: Boolean) {
            btn.setTextColor(if (active) Color.WHITE else COLOR_TEXT_MUTED)
            btn.background = if (active) makeGradientRoundRect(COLOR_PRIMARY, COLOR_PRIMARY_LIGHT, dp(8).toFloat()) else null
        }
        style(tabGroupBtn, currentTab == "Group")
        style(tabTeacherBtn, currentTab == "Teacher")
        style(tabAudBtn, currentTab == "Aud")
    }

    // ═══════════════════════════════════════════════════
    // Switch Tab
    // ═══════════════════════════════════════════════════
    private fun switchTab(tab: String) {
        currentTab = tab
        activeFilter = null
        updateTabUI()
        val placeholders = mapOf(
            "Group" to "Введите название группы...",
            "Teacher" to "Введите ФИО преподавателя...",
            "Aud" to "Введите номер аудитории..."
        )
        searchInput.hint = placeholders[tab]
        searchInput.setText("")
        hideDropdown()
        buildFilters()
    }

    // ═══════════════════════════════════════════════════
    // Build Filters
    // ═══════════════════════════════════════════════════
    private fun buildFilters() {
        filtersInner.removeAllViews()
        if (currentTab != "Group" || allGroups.isEmpty()) {
            filtersContainer.visibility = View.GONE
            return
        }
        filtersContainer.visibility = View.VISIBLE
        val facs = mutableMapOf<String, Int>()
        allGroups.forEach { g ->
            if (g.facul.isNotEmpty()) facs[g.facul] = (facs[g.facul] ?: 0) + 1
        }

        // "Все" chip
        addFilterChip("Все", activeFilter == null) { setFilter(null) }

        facs.keys.sorted().forEach { f ->
            addFilterChip("$f (${facs[f]})", activeFilter == f) { setFilter(f) }
        }

        for (k in 1..5) {
            val cnt = allGroups.count { it.kurs == k }
            if (cnt > 0) {
                val key = "k$k"
                addFilterChip("$k курс", activeFilter == key) { setFilter(key) }
            }
        }
    }

    private fun addFilterChip(text: String, active: Boolean, onClick: () -> Unit) {
        val chip = TextView(this).apply {
            this.text = text
            setTextColor(if (active) COLOR_ACCENT else COLOR_TEXT_SECONDARY)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 12f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
            background = makeRoundRect(
                if (active) 0x263B82F6 else 0x33000000,
                dp(16).toFloat()
            )
            setPadding(dp(14), dp(7), dp(14), dp(7))
            setOnClickListener { onClick() }
            layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply {
                marginEnd = dp(8)
            }
        }
        filtersInner.addView(chip)
    }

    private fun setFilter(f: String?) {
        activeFilter = if (activeFilter == f) null else f
        buildFilters()
        performSearch()
    }

    // ═══════════════════════════════════════════════════
    // Search
    // ═══════════════════════════════════════════════════
    private fun performSearch() {
        val query = searchInput.text.toString().trim().lowercase()
        if (query.isEmpty() && activeFilter == null) {
            hideDropdown()
            return
        }

        var items = when (currentTab) {
            "Group" -> allGroups.toList()
            "Teacher" -> allTeachers.toList()
            "Aud" -> allAuds.toList()
            else -> emptyList()
        }

        // Apply filter
        if (currentTab == "Group" && activeFilter != null) {
            items = if (activeFilter!!.startsWith("k")) {
                val k = activeFilter!!.substring(1).toIntOrNull() ?: 0
                items.filter { it.kurs == k }
            } else {
                items.filter { it.facul == activeFilter }
            }
        }

        if (query.isNotEmpty()) {
            items = items.filter { it.name.lowercase().contains(query) }
        }

        val total = items.size
        val display = items.take(50)

        dropdownContainer.removeAllViews()

        // Count label
        val countLabel = TextView(this).apply {
            text = if (total == 0) "Ничего не найдено" else "Найдено: $total${if (total > 50) "+" else ""}"
            setTextColor(COLOR_TEXT_MUTED)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 12f)
            setPadding(dp(16), dp(10), dp(16), dp(10))
            setBackgroundColor(0x083B82F6)
        }
        dropdownContainer.addView(countLabel)

        display.forEach { item ->
            val row = makeResultItem(item)
            dropdownContainer.addView(row)
        }

        showDropdown()
    }

    @SuppressLint("SetTextI18n")
    private fun makeResultItem(item: EntityItem): LinearLayout {
        val row = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            setPadding(dp(16), dp(12), dp(16), dp(12))
            isClickable = true
            isFocusable = true
            setOnClickListener {
                selectItem(currentTab, item.id, item.name)
            }
        }

        // Icon
        val iconColor = when (currentTab) {
            "Group" -> COLOR_ACCENT
            "Teacher" -> COLOR_SUCCESS
            else -> COLOR_PURPLE
        }
        val iconBg = when (currentTab) {
            "Group" -> 0x263B82F6
            "Teacher" -> 0x2610B981
            else -> 0x268B5CF6
        }
        val iconLabel = when (currentTab) {
            "Group" -> "👥"
            "Teacher" -> "👨‍🏫"
            else -> "🏢"
        }
        val icon = TextView(this).apply {
            text = iconLabel
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 18f)
            gravity = Gravity.CENTER
            background = makeRoundRect(iconBg, dp(10).toFloat())
            layoutParams = LinearLayout.LayoutParams(dp(40), dp(40)).apply { marginEnd = dp(12) }
        }
        row.addView(icon)

        // Info
        val info = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            layoutParams = LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f)
        }
        info.addView(TextView(this).apply {
            text = item.name
            setTextColor(COLOR_TEXT_PRIMARY)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 14f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
        })
        val sub = when (currentTab) {
            "Group" -> listOf(item.facul, if (item.kurs > 0) "${item.kurs} курс" else "", item.yearName).filter { it.isNotEmpty() }.joinToString(" · ")
            "Teacher" -> item.kaf
            else -> ""
        }
        if (sub.isNotEmpty()) {
            info.addView(TextView(this).apply {
                text = sub
                setTextColor(COLOR_TEXT_MUTED)
                setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f)
            })
        }
        row.addView(info)

        // Arrow
        row.addView(TextView(this).apply {
            text = "›"
            setTextColor(COLOR_TEXT_MUTED)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 20f)
        })

        return row
    }

    private fun showDropdown() {
        dropdownScroll.visibility = View.VISIBLE
        searchDropdownVisible = true
    }

    private fun hideDropdown() {
        dropdownScroll.visibility = View.GONE
        searchDropdownVisible = false
    }

    // ═══════════════════════════════════════════════════
    // Select Item & Load Schedule
    // ═══════════════════════════════════════════════════
    private fun selectItem(type: String, id: Int, name: String) {
        hideDropdown()
        searchInput.setText(name)
        // Hide keyboard
        val imm = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        imm.hideSoftInputFromWindow(searchInput.windowToken, 0)
        currentEntityType = type
        currentEntityId = id
        currentEntityName = name
        currentWeekStart = null
        loadSchedule(type, id, name, null)
    }

    @SuppressLint("SetTextI18n")
    private fun loadSchedule(type: String, id: Int, name: String, sdate: String?) {
        contentArea.removeAllViews()
        // Loading
        val loading = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setPadding(dp(20), dp(60), dp(20), dp(60))
        }
        loading.addView(ProgressBar(this@MainActivity).apply {
            isIndeterminate = true
            indeterminateTintList = android.content.res.ColorStateList.valueOf(COLOR_PRIMARY_LIGHT)
        })
        loading.addView(TextView(this).apply {
            text = "Загружаем расписание для «$name»..."
            setTextColor(COLOR_TEXT_MUTED)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 14f)
            gravity = Gravity.CENTER
            setPadding(0, dp(16), 0, 0)
        })
        contentArea.addView(loading)

        val pk = when (type) {
            "Group" -> "idGroup"
            "Teacher" -> "idTeacher"
            else -> "idAudLine"
        }
        var url = "$API_BASE/api/Rasp?$pk=$id"
        if (sdate != null) url += "&sdate=$sdate"

        thread {
            val json = fetchJson(url)
            runOnUiThread {
                if (json == null || !json.has("data") || json.optInt("state", -1) < 0) {
                    showError(json?.optString("msg") ?: "Не удалось загрузить расписание. Попробуйте позже.")
                    return@runOnUiThread
                }
                currentScheduleData = json.getJSONObject("data")
                renderSchedule(type, id, name, sdate)
            }
        }
    }

    // ═══════════════════════════════════════════════════
    // Render Schedule
    // ═══════════════════════════════════════════════════
    @SuppressLint("SetTextI18n")
    private fun renderSchedule(type: String, id: Int, name: String, sdate: String?) {
        val data = currentScheduleData ?: return
        val rasp = data.optJSONArray("rasp") ?: JSONArray()
        val info = data.optJSONObject("info") ?: JSONObject()
        contentArea.removeAllViews()

        if (info.has("year")) {
            headerBadge.text = "${info.optString("year")} уч. год"
        }

        // Parse lessons
        val lessons = mutableListOf<LessonItem>()
        for (i in 0 until rasp.length()) {
            val item = rasp.getJSONObject(i)
            lessons.add(LessonItem(
                date = item.optString("дата", "").take(10),
                start = item.optString("начало", ""),
                end = item.optString("конец", ""),
                disc = item.optString("дисциплина", ""),
                teacher = item.optString("преподаватель", ""),
                group = item.optString("группа", ""),
                aud = item.optString("аудитория", ""),
                lessonNum = item.optInt("номерЗанятия", 0),
                isReplace = item.optBoolean("замена", false)
            ))
        }

        // Group by date
        val grouped = linkedMapOf<String, MutableList<LessonItem>>()
        lessons.forEach { l ->
            grouped.getOrPut(l.date) { mutableListOf() }.add(l)
        }
        grouped.values.forEach { list -> list.sortBy { it.start } }

        // Determine week
        val todayCal = Calendar.getInstance()
        val sdf = SimpleDateFormat("yyyy-MM-dd", Locale.getDefault())
        val todayStr = sdf.format(todayCal.time)

        val weekStart: Calendar = Calendar.getInstance()
        if (currentWeekStart != null) {
            weekStart.time = sdf.parse(currentWeekStart!!)!!
        } else if (sdate != null) {
            weekStart.time = sdf.parse(sdate)!!
            val dow = if (weekStart.get(Calendar.DAY_OF_WEEK) == Calendar.SUNDAY) 7 else weekStart.get(Calendar.DAY_OF_WEEK) - 1
            weekStart.add(Calendar.DAY_OF_MONTH, -(dow - 1))
        } else {
            val dow = if (todayCal.get(Calendar.DAY_OF_WEEK) == Calendar.SUNDAY) 7 else todayCal.get(Calendar.DAY_OF_WEEK) - 1
            weekStart.add(Calendar.DAY_OF_MONTH, -(dow - 1))
        }
        currentWeekStart = sdf.format(weekStart.time)

        val weekEnd = weekStart.clone() as Calendar
        weekEnd.add(Calendar.DAY_OF_MONTH, 6)

        val weekNum = info.optInt("curWeekNumber", 0)
        val dateUpd = info.optString("dateUploadingRasp", "")
        val wrs = "${weekStart.get(Calendar.DAY_OF_MONTH)} ${MONTH_NAMES[weekStart.get(Calendar.MONTH)]} — ${weekEnd.get(Calendar.DAY_OF_MONTH)} ${MONTH_NAMES[weekEnd.get(Calendar.MONTH)]} ${weekEnd.get(Calendar.YEAR)}"

        val container = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
        }

        // ── Schedule Header ──
        val badgeLabel = when (type) { "Group" -> "Группа"; "Teacher" -> "Преподаватель"; else -> "Аудитория" }
        val badgeColor = when (type) { "Group" -> COLOR_ACCENT; "Teacher" -> COLOR_SUCCESS_LIGHT; else -> COLOR_PURPLE_LIGHT }
        val badgeBg = when (type) { "Group" -> 0x263B82F6; "Teacher" -> 0x2610B981; else -> 0x268B5CF6 }

        // Title row
        val titleRow = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            setPadding(0, 0, 0, dp(4))
        }
        titleRow.addView(TextView(this).apply {
            text = name
            setTextColor(COLOR_TEXT_PRIMARY)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 22f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
            layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply { marginEnd = dp(10) }
        })
        titleRow.addView(TextView(this).apply {
            text = badgeLabel
            setTextColor(badgeColor)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
            background = makeRoundRect(badgeBg, dp(6).toFloat())
            setPadding(dp(10), dp(4), dp(10), dp(4))
        })
        container.addView(titleRow)

        // Meta
        val meta = TextView(this).apply {
            text = "📅 $wrs" +
                    (if (weekNum > 0) "  •  Неделя $weekNum" else "") +
                    (if (dateUpd.isNotEmpty()) "  •  Обн. ${dateUpd.take(10)}" else "")
            setTextColor(COLOR_TEXT_MUTED)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 12f)
            setPadding(0, dp(4), 0, dp(12))
        }
        container.addView(meta)

        // ── Navigation ──
        val navRow = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            setPadding(0, 0, 0, dp(16))
        }
        navRow.addView(makeNavButton("◀") { navWeek(-1) })
        navRow.addView(TextView(this).apply {
            text = "📅 Сегодня"
            setTextColor(COLOR_ACCENT)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 14f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
            background = makeRoundRect(0x1F3B82F6, dp(10).toFloat())
            setPadding(dp(20), dp(12), dp(20), dp(12))
            gravity = Gravity.CENTER
            layoutParams = LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f).apply {
                marginStart = dp(8); marginEnd = dp(8)
            }
            setOnClickListener { navToday() }
        })
        navRow.addView(makeNavButton("▶") { navWeek(1) })
        container.addView(navRow)

        // ── Days ──
        val dayCal = weekStart.clone() as Calendar
        for (i in 0..6) {
            val ds = sdf.format(dayCal.time)
            val dayLessons = grouped[ds] ?: emptyList()
            val isToday = ds == todayStr
            val dayNum = dayCal.get(Calendar.DAY_OF_MONTH)
            val dayName = DAY_NAMES[dayCal.get(Calendar.DAY_OF_WEEK) - 1]
            val dateStr = "$dayNum ${MONTH_NAMES[dayCal.get(Calendar.MONTH)]}"
            val shouldOpen = isToday || (dayLessons.isNotEmpty() && dayCal.get(Calendar.DAY_OF_WEEK) != Calendar.SUNDAY)

            container.addView(makeDayCard(ds, dayNum, dayName, dateStr, isToday, dayLessons, shouldOpen, type))

            dayCal.add(Calendar.DAY_OF_MONTH, 1)
        }

        contentArea.addView(container)
    }

    // ═══════════════════════════════════════════════════
    // Day Card
    // ═══════════════════════════════════════════════════
    @SuppressLint("SetTextI18n")
    private fun makeDayCard(
        ds: String, dayNum: Int, dayName: String, dateStr: String,
        isToday: Boolean, lessons: List<LessonItem>, shouldOpen: Boolean, type: String
    ): LinearLayout {
        val card = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            background = makeRoundRect(COLOR_BG_CARD, dp(16).toFloat())
            layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply {
                bottomMargin = dp(12)
            }
            if (isToday) {
                background = makeStrokeRoundRect(COLOR_BG_CARD, COLOR_SUCCESS, dp(16).toFloat(), dp(2))
            }
        }

        // ── Day Head ──
        val dayBody = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            visibility = if (shouldOpen) View.VISIBLE else View.GONE
        }

        val head = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            setPadding(dp(16), dp(14), dp(16), dp(14))
            isClickable = true
            isFocusable = true
            setOnClickListener {
                dayBody.visibility = if (dayBody.visibility == View.VISIBLE) View.GONE else View.VISIBLE
            }
        }

        // Day number circle
        val numBg = if (isToday) makeGradientRoundRect(0xFF059669.toInt(), COLOR_SUCCESS, dp(12).toFloat())
                    else makeGradientRoundRect(COLOR_PRIMARY, COLOR_PRIMARY_LIGHT, dp(12).toFloat())
        val numView = TextView(this).apply {
            text = "$dayNum"
            setTextColor(Color.WHITE)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 16f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
            gravity = Gravity.CENTER
            background = numBg
            layoutParams = LinearLayout.LayoutParams(dp(44), dp(44)).apply { marginEnd = dp(12) }
        }
        head.addView(numView)

        // Day name & date
        val dayInfo = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            layoutParams = LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f)
        }
        dayInfo.addView(LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            addView(TextView(this@MainActivity).apply {
                text = dayName
                setTextColor(COLOR_TEXT_PRIMARY)
                setTextSize(TypedValue.COMPLEX_UNIT_SP, 15f)
                typeface = android.graphics.Typeface.DEFAULT_BOLD
            })
            if (isToday) {
                addView(TextView(this@MainActivity).apply {
                    text = "СЕГОДНЯ"
                    setTextColor(COLOR_SUCCESS)
                    setTextSize(TypedValue.COMPLEX_UNIT_SP, 9f)
                    typeface = android.graphics.Typeface.DEFAULT_BOLD
                    background = makeRoundRect(0x3310B981, dp(4).toFloat())
                    setPadding(dp(6), dp(2), dp(6), dp(2))
                    layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply { marginStart = dp(8) }
                })
            }
        })
        dayInfo.addView(TextView(this).apply {
            text = dateStr
            setTextColor(COLOR_TEXT_MUTED)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 12f)
        })
        head.addView(dayInfo)

        // Lessons count
        head.addView(TextView(this).apply {
            text = if (lessons.isNotEmpty()) "${lessons.size} ${getLW(lessons.size)}" else "Нет пар"
            setTextColor(if (isToday) COLOR_SUCCESS else COLOR_ACCENT)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
            background = makeRoundRect(if (isToday) 0x1F10B981 else 0x1F3B82F6, dp(12).toFloat())
            setPadding(dp(10), dp(4), dp(10), dp(4))
            alpha = if (lessons.isEmpty()) 0.4f else 1f
        })

        card.addView(head)

        // ── Day Body ──
        if (lessons.isEmpty()) {
            dayBody.addView(LinearLayout(this).apply {
                gravity = Gravity.CENTER
                orientation = LinearLayout.VERTICAL
                setPadding(dp(16), dp(24), dp(16), dp(24))
                addView(TextView(this@MainActivity).apply {
                    text = "☕"
                    setTextSize(TypedValue.COMPLEX_UNIT_SP, 32f)
                    gravity = Gravity.CENTER
                })
                addView(TextView(this@MainActivity).apply {
                    text = "Занятий нет — можно отдохнуть"
                    setTextColor(COLOR_TEXT_MUTED)
                    setTextSize(TypedValue.COMPLEX_UNIT_SP, 13f)
                    gravity = Gravity.CENTER
                    setPadding(0, dp(8), 0, 0)
                })
            })
        } else {
            val lessonsContainer = LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                setPadding(dp(16), dp(4), dp(16), dp(16))
            }
            lessons.forEach { l ->
                lessonsContainer.addView(makeLessonView(l, type))
            }
            dayBody.addView(lessonsContainer)
        }

        card.addView(dayBody)
        return card
    }

    // ═══════════════════════════════════════════════════
    // Lesson View
    // ═══════════════════════════════════════════════════
    @SuppressLint("SetTextI18n")
    private fun makeLessonView(l: LessonItem, type: String): LinearLayout {
        val ti = getTypeInfo(l.disc)
        val cleanName = l.disc.replace(Regex("^(лек|пр|лаб|сем)\\s*", RegexOption.IGNORE_CASE), "")

        val row = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            background = makeRoundRect(0x331E3A8A, dp(10).toFloat())
            setPadding(dp(0), dp(14), dp(14), dp(14))
            layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply {
                bottomMargin = dp(8)
            }
        }

        // Left colored bar
        row.addView(View(this).apply {
            setBackgroundColor(ti.color)
            layoutParams = LinearLayout.LayoutParams(dp(4), ViewGroup.LayoutParams.MATCH_PARENT).apply {
                marginEnd = dp(12)
            }
        })

        // Time
        val timeCol = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            layoutParams = LinearLayout.LayoutParams(dp(60), ViewGroup.LayoutParams.WRAP_CONTENT).apply { marginEnd = dp(10) }
        }
        timeCol.addView(TextView(this).apply {
            text = l.start.replace("-", ":")
            setTextColor(COLOR_ACCENT)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 14f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
            gravity = Gravity.CENTER
        })
        timeCol.addView(TextView(this).apply {
            text = l.end.replace("-", ":")
            setTextColor(COLOR_TEXT_MUTED)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f)
            gravity = Gravity.CENTER
        })
        if (l.lessonNum > 0 && l.lessonNum != 10) {
            timeCol.addView(TextView(this).apply {
                text = "${l.lessonNum} пара"
                setTextColor(COLOR_TEXT_MUTED)
                setTextSize(TypedValue.COMPLEX_UNIT_SP, 9f)
                gravity = Gravity.CENTER
                background = makeRoundRect(0x1A3B82F6, dp(8).toFloat())
                setPadding(dp(6), dp(2), dp(6), dp(2))
                layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply { topMargin = dp(4) }
            })
        }
        row.addView(timeCol)

        // Lesson info
        val infoCol = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            layoutParams = LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f)
        }
        infoCol.addView(TextView(this).apply {
            text = cleanName.ifEmpty { l.disc }
            setTextColor(COLOR_TEXT_PRIMARY)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 14f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
            setPadding(0, 0, 0, dp(6))
        })

        // Tags row
        val tagsRow = FlowLayout(this).apply {
            setPadding(0, 0, 0, 0)
        }

        // Type badge
        tagsRow.addView(TextView(this).apply {
            text = ti.label
            setTextColor(ti.color)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 10f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
            background = makeRoundRect(ti.bgColor, dp(10).toFloat())
            setPadding(dp(8), dp(3), dp(8), dp(3))
            layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply { marginEnd = dp(6); bottomMargin = dp(4) }
        })

        if (l.teacher.isNotEmpty() && type != "Teacher") {
            tagsRow.addView(makeDetailTag("👨‍🏫 ${l.teacher}"))
        }
        if (l.group.isNotEmpty() && type != "Group") {
            tagsRow.addView(makeDetailTag("👥 ${l.group}"))
        }
        if (l.aud.isNotEmpty() && type != "Aud") {
            tagsRow.addView(makeDetailTag("🚪 ${l.aud}"))
        }
        if (l.isReplace) {
            tagsRow.addView(TextView(this).apply {
                text = "🔄 Замена"
                setTextColor(COLOR_WARNING)
                setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f)
                setPadding(dp(4), dp(2), dp(4), dp(2))
                layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply { marginEnd = dp(6); bottomMargin = dp(4) }
            })
        }

        infoCol.addView(tagsRow)
        row.addView(infoCol)
        return row
    }

    private fun makeDetailTag(text: String): TextView {
        return TextView(this).apply {
            this.text = text
            setTextColor(COLOR_TEXT_SECONDARY)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f)
            setPadding(dp(4), dp(2), dp(4), dp(2))
            layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply {
                marginEnd = dp(8)
                bottomMargin = dp(4)
            }
        }
    }

    // ═══════════════════════════════════════════════════
    // Navigation
    // ═══════════════════════════════════════════════════
    private fun makeNavButton(text: String, onClick: () -> Unit): TextView {
        return TextView(this).apply {
            this.text = text
            setTextColor(Color.WHITE)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 18f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
            gravity = Gravity.CENTER
            background = makeGradientRoundRect(COLOR_PRIMARY, COLOR_PRIMARY_LIGHT, dp(10).toFloat())
            layoutParams = LinearLayout.LayoutParams(dp(48), dp(48))
            setOnClickListener { onClick() }
        }
    }

    private fun navWeek(dir: Int) {
        if (currentEntityId == 0) return
        val sdf = SimpleDateFormat("yyyy-MM-dd", Locale.getDefault())
        val ws = Calendar.getInstance()
        ws.time = sdf.parse(currentWeekStart!!)!!
        ws.add(Calendar.DAY_OF_MONTH, dir * 7)
        currentWeekStart = sdf.format(ws.time)
        loadSchedule(currentEntityType, currentEntityId, currentEntityName, currentWeekStart)
    }

    private fun navToday() {
        if (currentEntityId == 0) return
        currentWeekStart = null
        loadSchedule(currentEntityType, currentEntityId, currentEntityName, null)
    }

    // ═══════════════════════════════════════════════════
    // Welcome screen
    // ═══════════════════════════════════════════════════
    @SuppressLint("SetTextI18n")
    private fun showWelcome() {
        contentArea.removeAllViews()
        val welcome = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER_HORIZONTAL
            setPadding(dp(20), dp(50), dp(20), dp(50))
        }

        // Icon
        welcome.addView(TextView(this).apply {
            text = "📅"
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 48f)
            gravity = Gravity.CENTER
            background = makeGradientRoundRect(COLOR_PRIMARY_LIGHT, COLOR_PRIMARY, dp(20).toFloat())
            setPadding(dp(20), dp(20), dp(20), dp(20))
            layoutParams = LinearLayout.LayoutParams(dp(90), dp(90)).apply { bottomMargin = dp(20) }
        })

        welcome.addView(TextView(this).apply {
            text = "Добро пожаловать!"
            setTextColor(COLOR_TEXT_PRIMARY)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 24f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
            gravity = Gravity.CENTER
            setPadding(0, 0, 0, dp(10))
        })

        welcome.addView(TextView(this).apply {
            text = "Выберите группу, преподавателя или аудиторию, чтобы просмотреть актуальное расписание КГЭУ"
            setTextColor(COLOR_TEXT_MUTED)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 14f)
            gravity = Gravity.CENTER
            lineHeight = dp(22)
        })

        // Stats
        val statsRow = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER
            setPadding(0, dp(28), 0, dp(20))
        }
        val gLen = if (allGroups.isNotEmpty()) allGroups.size else 668
        val tLen = if (allTeachers.isNotEmpty()) allTeachers.size else 575
        val aLen = if (allAuds.isNotEmpty()) allAuds.size else 247

        statsRow.addView(makeStatCard("👥", "$gLen", "Групп", 0x263B82F6))
        statsRow.addView(makeStatCard("👨‍🏫", "$tLen", "Преподавателей", 0x2610B981))
        statsRow.addView(makeStatCard("🏢", "$aLen", "Аудиторий", 0x268B5CF6))
        welcome.addView(statsRow)

        // Quick links
        val quickTitle = TextView(this).apply {
            text = "Быстрый доступ:"
            setTextColor(COLOR_TEXT_MUTED)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 12f)
            setPadding(0, dp(12), 0, dp(8))
            gravity = Gravity.CENTER
        }
        welcome.addView(quickTitle)

        val quickContainer = FlowLayout(this).apply {
            gravity = Gravity.CENTER
        }
        val links = if (allGroups.isNotEmpty()) {
            val targets = listOf("А-1-24", "А-1-25", "А-2-24", "А-2-25", "АВБ-1-24")
            val found = targets.mapNotNull { t -> allGroups.find { it.name == t } }.take(6)
            if (found.size < 4) allGroups.filter { it.kurs in 1..2 }.take(6) else found
        } else {
            listOf(
                EntityItem(12553, "А-1-24"), EntityItem(12556, "А-1-25"),
                EntityItem(12554, "А-2-24"), EntityItem(12557, "А-2-25"),
                EntityItem(12544, "АВБ-1-24")
            )
        }

        links.forEach { g ->
            quickContainer.addView(TextView(this).apply {
                text = "👥 ${g.name}"
                setTextColor(COLOR_TEXT_SECONDARY)
                setTextSize(TypedValue.COMPLEX_UNIT_SP, 13f)
                background = makeStrokeRoundRect(COLOR_BG_CARD, COLOR_BORDER, dp(10).toFloat(), dp(1))
                setPadding(dp(14), dp(10), dp(14), dp(10))
                layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply {
                    marginEnd = dp(8); bottomMargin = dp(8)
                }
                setOnClickListener { selectItem("Group", g.id, g.name) }
            })
        }
        welcome.addView(quickContainer)

        contentArea.addView(welcome)
    }

    @SuppressLint("SetTextI18n")
    private fun makeStatCard(icon: String, value: String, label: String, bgColor: Int): LinearLayout {
        return LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            background = makeStrokeRoundRect(COLOR_BG_CARD, COLOR_BORDER, dp(10).toFloat(), dp(1))
            setPadding(dp(12), dp(14), dp(12), dp(14))
            layoutParams = LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f).apply {
                marginStart = dp(4); marginEnd = dp(4)
            }
            addView(TextView(this@MainActivity).apply {
                text = icon
                setTextSize(TypedValue.COMPLEX_UNIT_SP, 22f)
                gravity = Gravity.CENTER
            })
            addView(TextView(this@MainActivity).apply {
                text = value
                setTextColor(COLOR_TEXT_PRIMARY)
                setTextSize(TypedValue.COMPLEX_UNIT_SP, 20f)
                typeface = android.graphics.Typeface.DEFAULT_BOLD
                gravity = Gravity.CENTER
            })
            addView(TextView(this@MainActivity).apply {
                text = label
                setTextColor(COLOR_TEXT_MUTED)
                setTextSize(TypedValue.COMPLEX_UNIT_SP, 10f)
                gravity = Gravity.CENTER
            })
        }
    }

    // ═══════════════════════════════════════════════════
    // Error screen
    // ═══════════════════════════════════════════════════
    @SuppressLint("SetTextI18n")
    private fun showError(msg: String) {
        contentArea.removeAllViews()
        val err = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setPadding(dp(20), dp(60), dp(20), dp(60))
        }
        err.addView(TextView(this).apply {
            text = "⚠️"
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 48f)
            gravity = Gravity.CENTER
        })
        err.addView(TextView(this).apply {
            text = "Ошибка"
            setTextColor(COLOR_TEXT_PRIMARY)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 22f)
            typeface = android.graphics.Typeface.DEFAULT_BOLD
            gravity = Gravity.CENTER
            setPadding(0, dp(12), 0, dp(8))
        })
        err.addView(TextView(this).apply {
            text = msg
            setTextColor(COLOR_TEXT_MUTED)
            setTextSize(TypedValue.COMPLEX_UNIT_SP, 14f)
            gravity = Gravity.CENTER
        })
        contentArea.addView(err)
    }

    // ═══════════════════════════════════════════════════
    // Data Loading
    // ═══════════════════════════════════════════════════
    private fun loadAllData() {
        thread {
            val g = fetchJson("$API_BASE/api/raspGrouplist")
            val t = fetchJson("$API_BASE/api/raspTeacherlist")
            val a = fetchJson("$API_BASE/api/raspAudlist")

            runOnUiThread {
                if (g != null && g.has("data")) {
                    val arr = g.getJSONArray("data")
                    for (i in 0 until arr.length()) {
                        val o = arr.getJSONObject(i)
                        allGroups.add(EntityItem(
                            id = o.optInt("id"),
                            name = o.optString("name", ""),
                            facul = o.optString("facul", ""),
                            kurs = o.optInt("kurs", 0),
                            yearName = o.optString("yearName", "")
                        ))
                    }
                }
                if (t != null && t.has("data")) {
                    val arr = t.getJSONArray("data")
                    val seen = mutableSetOf<String>()
                    for (i in 0 until arr.length()) {
                        val o = arr.getJSONObject(i)
                        val nm = o.optString("name", "")
                        if (seen.add(nm)) {
                            allTeachers.add(EntityItem(
                                id = o.optInt("id"),
                                name = nm,
                                kaf = o.optString("kaf", "")
                            ))
                        }
                    }
                }
                if (a != null && a.has("data")) {
                    val arr = a.getJSONArray("data")
                    for (i in 0 until arr.length()) {
                        val o = arr.getJSONObject(i)
                        allAuds.add(EntityItem(
                            id = o.optInt("id"),
                            name = o.optString("name", "")
                        ))
                    }
                }

                val yearText = if (allGroups.isNotEmpty()) {
                    "${allGroups[0].yearName.ifEmpty { "2025-2026" }} уч. год"
                } else "2025-2026 уч. год"
                headerBadge.text = yearText

                buildFilters()
                showWelcome()
            }
        }
    }

    // ═══════════════════════════════════════════════════
    // Network fetch with CORS proxies
    // ═══════════════════════════════════════════════════
    private fun fetchJson(originalUrl: String): JSONObject? {
        // Try direct first
        try {
            val text = httpGet(originalUrl)
            if (text != null) return JSONObject(text)
        } catch (_: Exception) {}

        // Try proxies
        for (proxy in CORS_PROXIES) {
            try {
                val proxyUrl = proxy + URLEncoder.encode(originalUrl, "UTF-8")
                val text = httpGet(proxyUrl)
                if (text != null) return JSONObject(text)
            } catch (_: Exception) {}
        }
        return null
    }

    private fun httpGet(url: String): String? {
        val conn = URL(url).openConnection() as HttpURLConnection
        conn.requestMethod = "GET"
        conn.setRequestProperty("Accept", "application/json")
        conn.connectTimeout = 10000
        conn.readTimeout = 15000
        return try {
            if (conn.responseCode == 200) {
                val reader = BufferedReader(InputStreamReader(conn.inputStream, "UTF-8"))
                val sb = StringBuilder()
                var line: String?
                while (reader.readLine().also { line = it } != null) sb.append(line)
                reader.close()
                sb.toString()
            } else null
        } catch (e: Exception) {
            null
        } finally {
            conn.disconnect()
        }
    }

    // ═══════════════════════════════════════════════════
    // Helpers
    // ═══════════════════════════════════════════════════
    data class TypeInfo(val cls: String, val label: String, val color: Int, val bgColor: Int)

    private fun getTypeInfo(disc: String): TypeInfo {
        val s = disc.lowercase().trim()
        return when {
            s.startsWith("лек") -> TypeInfo("lec", "Лекция", COLOR_LEC, 0x263B82F6)
            s.startsWith("пр") -> TypeInfo("pr", "Практика", COLOR_PR, 0x2610B981)
            s.startsWith("лаб") -> TypeInfo("lab", "Лаб. работа", COLOR_LAB, 0x268B5CF6)
            s.startsWith("сем") -> TypeInfo("sem", "Семинар", COLOR_SEM, 0x26F59E0B)
            else -> TypeInfo("lec", "Занятие", COLOR_ACCENT, 0x1A3B82F6)
        }
    }

    private fun getLW(n: Int): String {
        return when {
            n % 10 == 1 && n % 100 != 11 -> "пара"
            n % 10 in 2..4 && n % 100 !in 12..14 -> "пары"
            else -> "пар"
        }
    }

    private fun dp(value: Int): Int {
        return TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, value.toFloat(), resources.displayMetrics).toInt()
    }

    private fun makeRoundRect(color: Int, radius: Float): GradientDrawable {
        return GradientDrawable().apply {
            setColor(color)
            cornerRadius = radius
        }
    }

    private fun makeGradientRoundRect(startColor: Int, endColor: Int, radius: Float): GradientDrawable {
        return GradientDrawable(GradientDrawable.Orientation.TL_BR, intArrayOf(startColor, endColor)).apply {
            cornerRadius = radius
        }
    }

    private fun makeStrokeRoundRect(fillColor: Int, strokeColor: Int, radius: Float, strokeWidth: Int): GradientDrawable {
        return GradientDrawable().apply {
            setColor(fillColor)
            setStroke(strokeWidth, strokeColor)
            cornerRadius = radius
        }
    }

    // ═══════════════════════════════════════════════════
    // FlowLayout (simple wrap layout)
    // ═══════════════════════════════════════════════════
    class FlowLayout(context: Context) : ViewGroup(context) {

        override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
            val maxWidth = MeasureSpec.getSize(widthMeasureSpec)
            var currentWidth = 0
            var currentHeight = 0
            var lineHeight = 0

            for (i in 0 until childCount) {
                val child = getChildAt(i)
                measureChild(child, widthMeasureSpec, heightMeasureSpec)
                val lp = child.layoutParams as? MarginLayoutParams ?: MarginLayoutParams(child.layoutParams ?: LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT))
                val childWidth = child.measuredWidth + lp.leftMargin + lp.rightMargin
                val childHeight = child.measuredHeight + lp.topMargin + lp.bottomMargin

                if (currentWidth + childWidth > maxWidth) {
                    currentHeight += lineHeight
                    currentWidth = childWidth
                    lineHeight = childHeight
                } else {
                    currentWidth += childWidth
                    lineHeight = maxOf(lineHeight, childHeight)
                }
            }
            currentHeight += lineHeight
            setMeasuredDimension(maxWidth, currentHeight)
        }

        override fun onLayout(changed: Boolean, l: Int, t: Int, r: Int, b: Int) {
            val maxWidth = r - l
            var cx = 0
            var cy = 0
            var lineHeight = 0

            for (i in 0 until childCount) {
                val child = getChildAt(i)
                val lp = child.layoutParams as? MarginLayoutParams ?: MarginLayoutParams(child.layoutParams ?: LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT))
                val childWidth = child.measuredWidth + lp.leftMargin + lp.rightMargin
                val childHeight = child.measuredHeight + lp.topMargin + lp.bottomMargin

                if (cx + childWidth > maxWidth) {
                    cy += lineHeight
                    cx = 0
                    lineHeight = 0
                }
                child.layout(cx + lp.leftMargin, cy + lp.topMargin, cx + lp.leftMargin + child.measuredWidth, cy + lp.topMargin + child.measuredHeight)
                cx += childWidth
                lineHeight = maxOf(lineHeight, childHeight)
            }
        }

        override fun generateLayoutParams(attrs: android.util.AttributeSet?): LayoutParams {
            return MarginLayoutParams(context, attrs)
        }

        override fun generateDefaultLayoutParams(): LayoutParams {
            return MarginLayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT)
        }
    }
}
