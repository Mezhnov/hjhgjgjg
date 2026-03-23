package com.example.elegantchat

import android.animation.AnimatorSet
import android.animation.ObjectAnimator
import android.content.Context
import android.graphics.*
import android.graphics.drawable.*
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.text.Editable
import android.text.TextWatcher
import android.util.TypedValue
import android.view.*
import android.view.animation.*
import android.view.inputmethod.EditorInfo
import android.widget.*
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.core.graphics.ColorUtils
import androidx.recyclerview.widget.DefaultItemAnimator
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import java.text.SimpleDateFormat
import java.util.*

// ══════════════════════════════════════════════════════════════════
//  ELEGANT MESSENGER — Telegram-Level Chat UI (Single File)
//  Android Studio · Kotlin · Material Design 3
// ══════════════════════════════════════════════════════════════════

// ─── Data Models ─────────────────────────────────────────────────

enum class MessageStatus { SENDING, SENT, DELIVERED, READ }
enum class MessageType { TEXT, IMAGE, VOICE, STICKER, DATE_HEADER }

data class ChatMessage(
    val id: String = UUID.randomUUID().toString(),
    val text: String,
    val time: Long = System.currentTimeMillis(),
    val isOutgoing: Boolean = true,
    val status: MessageStatus = MessageStatus.READ,
    val type: MessageType = MessageType.TEXT,
    val replyTo: String? = null,
    val senderName: String = "",
    val senderAvatar: Int = 0 // color seed
)

// ─── Color Palette ───────────────────────────────────────────────

object ChatColors {
    const val PRIMARY = 0xFF6C3AE1.toInt()          // Deep purple
    const val PRIMARY_DARK = 0xFF5A2DC5.toInt()
    const val PRIMARY_LIGHT = 0xFFEDE7FB.toInt()
    const val ACCENT = 0xFF7C4DFF.toInt()

    const val BG_CHAT = 0xFFF0EBF8.toInt()          // Light lavender bg
    const val BG_PATTERN = 0x12000000                // Subtle pattern overlay

    const val BUBBLE_OUT = 0xFF6C3AE1.toInt()        // Outgoing bubble
    const val BUBBLE_OUT_SHADOW = 0x306C3AE1         // Outgoing shadow
    const val BUBBLE_IN = 0xFFFFFFFF.toInt()          // Incoming bubble
    const val BUBBLE_IN_SHADOW = 0x18000000          // Incoming shadow

    const val TEXT_OUT = 0xFFFFFFFF.toInt()
    const val TEXT_IN = 0xFF1A1A2E.toInt()
    const val TIME_OUT = 0xBBFFFFFF.toInt()
    const val TIME_IN = 0xFF9E9EB8.toInt()

    const val HEADER_BG = 0xFFFFFFFF.toInt()
    const val HEADER_TITLE = 0xFF1A1A2E.toInt()
    const val HEADER_SUBTITLE = 0xFF7C4DFF.toInt()

    const val INPUT_BG = 0xFFFFFFFF.toInt()
    const val INPUT_FIELD_BG = 0xFFF5F3FA.toInt()
    const val INPUT_HINT = 0xFFB0A8C9.toInt()
    const val INPUT_TEXT = 0xFF1A1A2E.toInt()

    const val SEND_BTN = 0xFF6C3AE1.toInt()
    const val SEND_BTN_INACTIVE = 0xFFD0C8E6.toInt()
    const val ATTACH_ICON = 0xFF9E93C3.toInt()

    const val DATE_BADGE_BG = 0x22000000
    const val DATE_BADGE_TEXT = 0xFF6B6B8D.toInt()

    const val ONLINE_GREEN = 0xFF4CAF50.toInt()
    const val CHECK_COLOR = 0xBBFFFFFF.toInt()
    const val UNREAD_BADGE = 0xFFFF5252.toInt()

    val AVATAR_COLORS = intArrayOf(
        0xFFE91E63.toInt(), 0xFF9C27B0.toInt(), 0xFF673AB7.toInt(),
        0xFF3F51B5.toInt(), 0xFF2196F3.toInt(), 0xFF00BCD4.toInt(),
        0xFF009688.toInt(), 0xFF4CAF50.toInt(), 0xFFFF9800.toInt(),
        0xFFFF5722.toInt()
    )
}

// ─── Utility Extensions ──────────────────────────────────────────

fun Context.dp(value: Int): Int =
    TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, value.toFloat(), resources.displayMetrics).toInt()

fun Context.dpF(value: Float): Float =
    TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, value, resources.displayMetrics)

fun Context.sp(value: Float): Float =
    TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_SP, value, resources.displayMetrics)

// ─── Custom Drawables ────────────────────────────────────────────

class BubbleDrawable(
    private val color: Int,
    private val isOutgoing: Boolean,
    private val cornerRadius: Float,
    private val shadowColor: Int
) : Drawable() {

    private val paint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        this.color = this@BubbleDrawable.color
        style = Paint.Style.FILL
    }

    private val shadowPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        this.color = shadowColor
        maskFilter = BlurMaskFilter(8f, BlurMaskFilter.Blur.NORMAL)
    }

    private val tailSize = 8f

    override fun draw(canvas: Canvas) {
        val rect = RectF(bounds)
        val shadowRect = RectF(rect.left + 2, rect.top + 3, rect.right - 2, rect.bottom + 2)
        canvas.drawRoundRect(shadowRect, cornerRadius, cornerRadius, shadowPaint)

        if (isOutgoing) {
            rect.right -= tailSize
        } else {
            rect.left += tailSize
        }
        canvas.drawRoundRect(rect, cornerRadius, cornerRadius, paint)

        // Draw tail
        val tailPath = Path()
        if (isOutgoing) {
            val tx = rect.right
            val ty = rect.bottom - 16f
            tailPath.moveTo(tx, ty)
            tailPath.lineTo(tx + tailSize + 2, rect.bottom - 4f)
            tailPath.quadTo(tx + tailSize, rect.bottom, tx, rect.bottom)
            tailPath.close()
        } else {
            val tx = rect.left
            val ty = rect.bottom - 16f
            tailPath.moveTo(tx, ty)
            tailPath.lineTo(tx - tailSize - 2, rect.bottom - 4f)
            tailPath.quadTo(tx - tailSize, rect.bottom, tx, rect.bottom)
            tailPath.close()
        }
        canvas.drawPath(tailPath, paint)
    }

    override fun setAlpha(alpha: Int) { paint.alpha = alpha }
    override fun setColorFilter(cf: ColorFilter?) { paint.colorFilter = cf }
    override fun getOpacity(): Int = PixelFormat.TRANSLUCENT
}

class AvatarDrawable(
    private val letter: Char,
    private val bgColor: Int,
    private val size: Int
) : Drawable() {

    private val bgPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = bgColor
        style = Paint.Style.FILL
    }
    private val textPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.WHITE
        textSize = size * 0.42f
        textAlign = Paint.Align.CENTER
        typeface = Typeface.create(Typeface.DEFAULT, Typeface.BOLD)
    }

    override fun draw(canvas: Canvas) {
        val cx = bounds.exactCenterX()
        val cy = bounds.exactCenterY()
        canvas.drawCircle(cx, cy, bounds.width() / 2f, bgPaint)
        val textY = cy - (textPaint.descent() + textPaint.ascent()) / 2
        canvas.drawText(letter.toString(), cx, textY, textPaint)
    }

    override fun setAlpha(alpha: Int) {}
    override fun setColorFilter(cf: ColorFilter?) {}
    override fun getOpacity(): Int = PixelFormat.TRANSLUCENT
    override fun getIntrinsicWidth(): Int = size
    override fun getIntrinsicHeight(): Int = size
}

class CircleDrawable(private val color: Int) : Drawable() {
    private val paint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        this.color = this@CircleDrawable.color
    }
    override fun draw(canvas: Canvas) {
        canvas.drawCircle(bounds.exactCenterX(), bounds.exactCenterY(), bounds.width() / 2f, paint)
    }
    override fun setAlpha(alpha: Int) {}
    override fun setColorFilter(cf: ColorFilter?) {}
    override fun getOpacity(): Int = PixelFormat.TRANSLUCENT
}

class ChatPatternBackground(private val context: Context) : Drawable() {
    private val bgPaint = Paint().apply { color = ChatColors.BG_CHAT }
    private val dotPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = ChatColors.BG_PATTERN
        style = Paint.Style.FILL
    }
    override fun draw(canvas: Canvas) {
        canvas.drawRect(bounds, bgPaint)
        val spacing = context.dp(32)
        var y = 0
        var row = 0
        while (y < bounds.height()) {
            var x = if (row % 2 == 0) 0 else spacing / 2
            while (x < bounds.width()) {
                canvas.drawCircle(x.toFloat(), y.toFloat(), 2.5f, dotPaint)
                x += spacing
            }
            y += spacing
            row++
        }
    }
    override fun setAlpha(alpha: Int) {}
    override fun setColorFilter(cf: ColorFilter?) {}
    override fun getOpacity(): Int = PixelFormat.TRANSLUCENT
}

// ─── Ripple Helper ───────────────────────────────────────────────

fun createRipple(context: Context, normalColor: Int, rippleColor: Int, radius: Float): Drawable {
    val mask = GradientDrawable().apply {
        shape = GradientDrawable.RECTANGLE
        cornerRadius = radius
        setColor(Color.WHITE)
    }
    val content = GradientDrawable().apply {
        shape = GradientDrawable.RECTANGLE
        cornerRadius = radius
        setColor(normalColor)
    }
    return RippleDrawable(
        android.content.res.ColorStateList.valueOf(rippleColor),
        content, mask
    )
}

fun createCircleRipple(context: Context, normalColor: Int, rippleColor: Int): Drawable {
    val mask = GradientDrawable().apply {
        shape = GradientDrawable.OVAL
        setColor(Color.WHITE)
    }
    val content = GradientDrawable().apply {
        shape = GradientDrawable.OVAL
        setColor(normalColor)
    }
    return RippleDrawable(
        android.content.res.ColorStateList.valueOf(rippleColor),
        content, mask
    )
}

// ─── Check Marks Drawable ────────────────────────────────────────

class CheckMarksDrawable(
    private val status: MessageStatus,
    private val color: Int,
    private val size: Int
) : Drawable() {

    private val paint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        this.color = this@CheckMarksDrawable.color
        style = Paint.Style.STROKE
        strokeWidth = size * 0.12f
        strokeCap = Paint.Cap.ROUND
        strokeJoin = Paint.Join.ROUND
    }

    override fun draw(canvas: Canvas) {
        val s = size.toFloat()
        when (status) {
            MessageStatus.SENDING -> {
                // Clock icon
                val cx = s / 2; val cy = s / 2; val r = s * 0.35f
                canvas.drawCircle(cx, cy, r, paint)
                canvas.drawLine(cx, cy - r * 0.6f, cx, cy, paint)
                canvas.drawLine(cx, cy, cx + r * 0.4f, cy, paint)
            }
            MessageStatus.SENT -> {
                // Single check
                val path = Path()
                path.moveTo(s * 0.15f, s * 0.5f)
                path.lineTo(s * 0.4f, s * 0.75f)
                path.lineTo(s * 0.85f, s * 0.25f)
                canvas.drawPath(path, paint)
            }
            MessageStatus.DELIVERED -> {
                // Double check
                val path1 = Path()
                path1.moveTo(s * 0.05f, s * 0.5f)
                path1.lineTo(s * 0.3f, s * 0.75f)
                path1.lineTo(s * 0.75f, s * 0.25f)
                canvas.drawPath(path1, paint)
                val path2 = Path()
                path2.moveTo(s * 0.25f, s * 0.5f)
                path2.lineTo(s * 0.5f, s * 0.75f)
                path2.lineTo(s * 0.95f, s * 0.25f)
                canvas.drawPath(path2, paint)
            }
            MessageStatus.READ -> {
                // Double check (blue/colored)
                val path1 = Path()
                path1.moveTo(s * 0.05f, s * 0.5f)
                path1.lineTo(s * 0.3f, s * 0.75f)
                path1.lineTo(s * 0.75f, s * 0.25f)
                canvas.drawPath(path1, paint)
                val path2 = Path()
                path2.moveTo(s * 0.25f, s * 0.5f)
                path2.lineTo(s * 0.5f, s * 0.75f)
                path2.lineTo(s * 0.95f, s * 0.25f)
                canvas.drawPath(path2, paint)
            }
        }
    }

    override fun setAlpha(alpha: Int) {}
    override fun setColorFilter(cf: ColorFilter?) {}
    override fun getOpacity(): Int = PixelFormat.TRANSLUCENT
    override fun getIntrinsicWidth(): Int = size
    override fun getIntrinsicHeight(): Int = size
}

// ─── Chat Adapter ────────────────────────────────────────────────

class ChatAdapter(
    private val context: Context,
    private val messages: MutableList<ChatMessage>
) : RecyclerView.Adapter<RecyclerView.ViewHolder>() {

    companion object {
        const val TYPE_OUTGOING = 0
        const val TYPE_INCOMING = 1
        const val TYPE_DATE = 2
    }

    private val timeFormat = SimpleDateFormat("HH:mm", Locale.getDefault())

    override fun getItemViewType(position: Int): Int {
        val msg = messages[position]
        return when {
            msg.type == MessageType.DATE_HEADER -> TYPE_DATE
            msg.isOutgoing -> TYPE_OUTGOING
            else -> TYPE_INCOMING
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): RecyclerView.ViewHolder {
        return when (viewType) {
            TYPE_DATE -> DateViewHolder(createDateView())
            TYPE_OUTGOING -> OutgoingViewHolder(createOutgoingView())
            else -> IncomingViewHolder(createIncomingView())
        }
    }

    override fun onBindViewHolder(holder: RecyclerView.ViewHolder, position: Int) {
        val msg = messages[position]
        when (holder) {
            is DateViewHolder -> holder.bind(msg)
            is OutgoingViewHolder -> holder.bind(msg)
            is IncomingViewHolder -> holder.bind(msg)
        }
    }

    override fun getItemCount() = messages.size

    // ── Date Header ──

    private fun createDateView(): View {
        val container = FrameLayout(context).apply {
            layoutParams = ViewGroup.LayoutParams(-1, -2)
            setPadding(0, context.dp(12), 0, context.dp(8))
        }
        val badge = TextView(context).apply {
            id = 1001
            textSize = 13f
            setTextColor(ChatColors.DATE_BADGE_TEXT)
            typeface = Typeface.create("sans-serif-medium", Typeface.NORMAL)
            setPadding(context.dp(16), context.dp(6), context.dp(16), context.dp(6))
            background = GradientDrawable().apply {
                setColor(ChatColors.DATE_BADGE_BG)
                cornerRadius = context.dpF(16f)
            }
            val lp = FrameLayout.LayoutParams(-2, -2)
            lp.gravity = Gravity.CENTER_HORIZONTAL
            layoutParams = lp
        }
        container.addView(badge)
        return container
    }

    inner class DateViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        private val badge = view.findViewById<TextView>(1001)
        fun bind(msg: ChatMessage) { badge.text = msg.text }
    }

    // ── Outgoing Message ──

    private fun createOutgoingView(): View {
        val root = FrameLayout(context).apply {
            layoutParams = ViewGroup.LayoutParams(-1, -2)
            setPadding(context.dp(64), context.dp(2), context.dp(8), context.dp(2))
        }

        val bubble = LinearLayout(context).apply {
            id = 2001
            orientation = LinearLayout.VERTICAL
            val lp = FrameLayout.LayoutParams(-2, -2)
            lp.gravity = Gravity.END
            layoutParams = lp
            setPadding(context.dp(14), context.dp(9), context.dp(14), context.dp(8))
            background = BubbleDrawable(
                ChatColors.BUBBLE_OUT, true,
                context.dpF(18f), ChatColors.BUBBLE_OUT_SHADOW
            )
        }

        // Reply bar
        val replyBar = LinearLayout(context).apply {
            id = 2004
            orientation = LinearLayout.VERTICAL
            visibility = View.GONE
            setPadding(context.dp(10), context.dp(4), context.dp(8), context.dp(4))
            background = GradientDrawable().apply {
                setColor(0x33FFFFFF)
                cornerRadius = context.dpF(6f)
            }
            val lp = LinearLayout.LayoutParams(-1, -2)
            lp.bottomMargin = context.dp(6)
            layoutParams = lp
        }
        val replyName = TextView(context).apply {
            id = 2005
            textSize = 12f
            setTextColor(0xDDFFFFFF.toInt())
            typeface = Typeface.create("sans-serif-medium", Typeface.BOLD)
        }
        val replyText = TextView(context).apply {
            id = 2006
            textSize = 12f
            setTextColor(0xAAFFFFFF.toInt())
            maxLines = 1
        }
        replyBar.addView(replyName)
        replyBar.addView(replyText)
        bubble.addView(replyBar)

        val text = TextView(context).apply {
            id = 2002
            textSize = 15.5f
            setTextColor(ChatColors.TEXT_OUT)
            setLineSpacing(context.dpF(2f), 1f)
            typeface = Typeface.create("sans-serif", Typeface.NORMAL)
        }
        bubble.addView(text)

        val metaRow = LinearLayout(context).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.END or Gravity.CENTER_VERTICAL
            val lp = LinearLayout.LayoutParams(-2, -2)
            lp.gravity = Gravity.END
            lp.topMargin = context.dp(3)
            layoutParams = lp
        }

        val time = TextView(context).apply {
            id = 2003
            textSize = 11f
            setTextColor(ChatColors.TIME_OUT)
            typeface = Typeface.create("sans-serif", Typeface.NORMAL)
        }
        metaRow.addView(time)

        val checkView = ImageView(context).apply {
            id = 2007
            val sz = context.dp(14)
            val lp = LinearLayout.LayoutParams(sz, sz)
            lp.marginStart = context.dp(4)
            layoutParams = lp
        }
        metaRow.addView(checkView)

        bubble.addView(metaRow)
        root.addView(bubble)
        return root
    }

    inner class OutgoingViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        private val text = view.findViewById<TextView>(2002)
        private val time = view.findViewById<TextView>(2003)
        private val replyBar = view.findViewById<LinearLayout>(2004)
        private val replyName = view.findViewById<TextView>(2005)
        private val replyText = view.findViewById<TextView>(2006)
        private val checkView = view.findViewById<ImageView>(2007)

        fun bind(msg: ChatMessage) {
            text.text = msg.text
            time.text = timeFormat.format(Date(msg.time))

            val sz = context.dp(14)
            checkView.setImageDrawable(CheckMarksDrawable(msg.status, ChatColors.CHECK_COLOR, sz))

            if (msg.replyTo != null) {
                replyBar.visibility = View.VISIBLE
                replyName.text = "Алиса"
                replyText.text = msg.replyTo
            } else {
                replyBar.visibility = View.GONE
            }
        }
    }

    // ── Incoming Message ──

    private fun createIncomingView(): View {
        val root = LinearLayout(context).apply {
            orientation = LinearLayout.HORIZONTAL
            layoutParams = ViewGroup.LayoutParams(-1, -2)
            setPadding(context.dp(8), context.dp(2), context.dp(64), context.dp(2))
            gravity = Gravity.START or Gravity.BOTTOM
        }

        val avatar = ImageView(context).apply {
            id = 3005
            val sz = context.dp(36)
            val lp = LinearLayout.LayoutParams(sz, sz)
            lp.marginEnd = context.dp(6)
            lp.gravity = Gravity.BOTTOM
            lp.bottomMargin = context.dp(2)
            layoutParams = lp
        }
        root.addView(avatar)

        val bubble = LinearLayout(context).apply {
            id = 3001
            orientation = LinearLayout.VERTICAL
            val lp = LinearLayout.LayoutParams(-2, -2)
            layoutParams = lp
            setPadding(context.dp(14), context.dp(8), context.dp(14), context.dp(8))
            background = BubbleDrawable(
                ChatColors.BUBBLE_IN, false,
                context.dpF(18f), ChatColors.BUBBLE_IN_SHADOW
            )
        }

        val name = TextView(context).apply {
            id = 3004
            textSize = 13f
            typeface = Typeface.create("sans-serif-medium", Typeface.BOLD)
            val lp = LinearLayout.LayoutParams(-2, -2)
            lp.bottomMargin = context.dp(2)
            layoutParams = lp
        }
        bubble.addView(name)

        val text = TextView(context).apply {
            id = 3002
            textSize = 15.5f
            setTextColor(ChatColors.TEXT_IN)
            setLineSpacing(context.dpF(2f), 1f)
            typeface = Typeface.create("sans-serif", Typeface.NORMAL)
        }
        bubble.addView(text)

        val time = TextView(context).apply {
            id = 3003
            textSize = 11f
            setTextColor(ChatColors.TIME_IN)
            val lp = LinearLayout.LayoutParams(-2, -2)
            lp.gravity = Gravity.END
            lp.topMargin = context.dp(3)
            layoutParams = lp
        }
        bubble.addView(time)

        root.addView(bubble)
        return root
    }

    inner class IncomingViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        private val text = view.findViewById<TextView>(3002)
        private val time = view.findViewById<TextView>(3003)
        private val name = view.findViewById<TextView>(3004)
        private val avatar = view.findViewById<ImageView>(3005)

        fun bind(msg: ChatMessage) {
            text.text = msg.text
            time.text = timeFormat.format(Date(msg.time))

            val colorIndex = Math.abs(msg.senderName.hashCode()) % ChatColors.AVATAR_COLORS.size
            val avatarColor = ChatColors.AVATAR_COLORS[colorIndex]

            name.text = msg.senderName
            name.setTextColor(avatarColor)

            val letter = msg.senderName.firstOrNull()?.uppercaseChar() ?: 'U'
            avatar.setImageDrawable(AvatarDrawable(letter, avatarColor, context.dp(36)))
        }
    }
}

// ─── Main Activity ───────────────────────────────────────────────

class MainActivity : AppCompatActivity() {

    private lateinit var recyclerView: RecyclerView
    private lateinit var inputField: EditText
    private lateinit var sendButton: ImageView
    private lateinit var adapter: ChatAdapter
    private val messages = mutableListOf<ChatMessage>()
    private val handler = Handler(Looper.getMainLooper())

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        window.statusBarColor = ChatColors.PRIMARY_DARK
        window.navigationBarColor = ChatColors.INPUT_BG

        val rootLayout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setBackgroundColor(ChatColors.BG_CHAT)
        }

        // ── Header ──
        rootLayout.addView(createHeader())

        // ── RecyclerView ──
        recyclerView = RecyclerView(this).apply {
            layoutParams = LinearLayout.LayoutParams(-1, 0, 1f)
            background = ChatPatternBackground(this@MainActivity)
            clipToPadding = false
            setPadding(0, dp(8), 0, dp(8))
            overScrollMode = View.OVER_SCROLL_NEVER

            layoutManager = LinearLayoutManager(this@MainActivity).apply {
                stackFromEnd = true
            }
            itemAnimator = DefaultItemAnimator().apply {
                addDuration = 200
                removeDuration = 200
            }
        }
        rootLayout.addView(recyclerView)

        // ── Input Bar ──
        rootLayout.addView(createInputBar())

        setContentView(rootLayout)

        // ── Load demo messages ──
        loadDemoMessages()
        adapter = ChatAdapter(this, messages)
        recyclerView.adapter = adapter
        recyclerView.scrollToPosition(messages.size - 1)
    }

    // ── Header Bar ──

    private fun createHeader(): View {
        val header = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            setBackgroundColor(ChatColors.PRIMARY)
            setPadding(dp(6), dp(10), dp(12), dp(10))
            elevation = dpF(4f)
            val lp = LinearLayout.LayoutParams(-1, -2)
            layoutParams = lp
        }

        // Back button
        val backBtn = ImageView(this).apply {
            val sz = dp(40)
            val lp = LinearLayout.LayoutParams(sz, sz)
            lp.marginEnd = dp(4)
            layoutParams = lp
            setPadding(dp(8), dp(8), dp(8), dp(8))
            setImageDrawable(createBackArrow())
            background = createCircleRipple(this@MainActivity, Color.TRANSPARENT, 0x33FFFFFF)
            isClickable = true
            isFocusable = true
        }
        header.addView(backBtn)

        // Avatar with online indicator
        val avatarContainer = FrameLayout(this).apply {
            val sz = dp(42)
            val lp = LinearLayout.LayoutParams(sz, sz)
            lp.marginEnd = dp(12)
            layoutParams = lp
        }
        val avatarView = ImageView(this).apply {
            layoutParams = FrameLayout.LayoutParams(dp(42), dp(42))
            setImageDrawable(AvatarDrawable('А', ChatColors.AVATAR_COLORS[3], dp(42)))
        }
        avatarContainer.addView(avatarView)

        val onlineDot = View(this).apply {
            val sz = dp(13)
            val lp = FrameLayout.LayoutParams(sz, sz)
            lp.gravity = Gravity.BOTTOM or Gravity.END
            layoutParams = lp
            background = LayerDrawable(arrayOf(
                CircleDrawable(Color.WHITE),
                InsetDrawable(CircleDrawable(ChatColors.ONLINE_GREEN), dp(2))
            ))
        }
        avatarContainer.addView(onlineDot)
        header.addView(avatarContainer)

        // Name & status
        val infoLayout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            layoutParams = LinearLayout.LayoutParams(0, -2, 1f)
        }
        val nameText = TextView(this).apply {
            text = "Алиса Смирнова"
            textSize = 17f
            setTextColor(Color.WHITE)
            typeface = Typeface.create("sans-serif-medium", Typeface.BOLD)
        }
        infoLayout.addView(nameText)

        val statusText = TextView(this).apply {
            text = "в сети"
            textSize = 13f
            setTextColor(0xBBFFFFFF.toInt())
            typeface = Typeface.create("sans-serif", Typeface.NORMAL)
        }
        infoLayout.addView(statusText)
        header.addView(infoLayout)

        // Action buttons
        val searchBtn = createHeaderIcon(createSearchIcon())
        header.addView(searchBtn)

        val moreBtn = createHeaderIcon(createMoreIcon())
        header.addView(moreBtn)

        return header
    }

    private fun createHeaderIcon(drawable: Drawable): ImageView {
        return ImageView(this).apply {
            val sz = dp(40)
            val lp = LinearLayout.LayoutParams(sz, sz)
            lp.marginStart = dp(2)
            layoutParams = lp
            setPadding(dp(8), dp(8), dp(8), dp(8))
            setImageDrawable(drawable)
            background = createCircleRipple(this@MainActivity, Color.TRANSPARENT, 0x33FFFFFF)
            isClickable = true
            isFocusable = true
        }
    }

    // ── Input Bar ──

    private fun createInputBar(): View {
        val inputBar = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL or Gravity.BOTTOM
            setBackgroundColor(ChatColors.INPUT_BG)
            setPadding(dp(6), dp(6), dp(6), dp(6))
            elevation = dpF(8f)
        }

        // Emoji button
        val emojiBtn = ImageView(this).apply {
            val sz = dp(42)
            val lp = LinearLayout.LayoutParams(sz, sz)
            layoutParams = lp
            setPadding(dp(9), dp(9), dp(9), dp(9))
            setImageDrawable(createEmojiIcon())
            background = createCircleRipple(this@MainActivity, Color.TRANSPARENT, 0x11000000)
            isClickable = true
        }
        inputBar.addView(emojiBtn)

        // Text input
        inputField = EditText(this).apply {
            val lp = LinearLayout.LayoutParams(0, -2, 1f)
            lp.marginStart = dp(2)
            lp.marginEnd = dp(2)
            layoutParams = lp
            hint = "Сообщение"
            setHintTextColor(ChatColors.INPUT_HINT)
            setTextColor(ChatColors.INPUT_TEXT)
            textSize = 16f
            typeface = Typeface.create("sans-serif", Typeface.NORMAL)
            background = GradientDrawable().apply {
                setColor(ChatColors.INPUT_FIELD_BG)
                cornerRadius = dpF(24f)
            }
            setPadding(dp(18), dp(10), dp(18), dp(10))
            maxLines = 5
            imeOptions = EditorInfo.IME_ACTION_NONE
            inputType = android.text.InputType.TYPE_CLASS_TEXT or
                    android.text.InputType.TYPE_TEXT_FLAG_MULTI_LINE or
                    android.text.InputType.TYPE_TEXT_FLAG_CAP_SENTENCES
        }
        inputBar.addView(inputField)

        // Attach button
        val attachBtn = ImageView(this).apply {
            id = 5001
            val sz = dp(42)
            val lp = LinearLayout.LayoutParams(sz, sz)
            layoutParams = lp
            setPadding(dp(9), dp(9), dp(9), dp(9))
            setImageDrawable(createAttachIcon())
            background = createCircleRipple(this@MainActivity, Color.TRANSPARENT, 0x11000000)
            isClickable = true
        }
        inputBar.addView(attachBtn)

        // Send button
        sendButton = ImageView(this).apply {
            val sz = dp(46)
            val lp = LinearLayout.LayoutParams(sz, sz)
            lp.marginStart = dp(2)
            layoutParams = lp
            setPadding(dp(11), dp(11), dp(9), dp(11))
            setImageDrawable(createSendIcon())
            background = createCircleRipple(
                this@MainActivity, ChatColors.SEND_BTN, 0x33FFFFFF
            )
            elevation = dpF(3f)
            visibility = View.GONE
            isClickable = true
        }
        inputBar.addView(sendButton)

        // Mic button
        val micBtn = ImageView(this).apply {
            id = 5002
            val sz = dp(46)
            val lp = LinearLayout.LayoutParams(sz, sz)
            lp.marginStart = dp(2)
            layoutParams = lp
            setPadding(dp(11), dp(11), dp(11), dp(11))
            setImageDrawable(createMicIcon())
            background = createCircleRipple(
                this@MainActivity, ChatColors.SEND_BTN, 0x33FFFFFF
            )
            elevation = dpF(3f)
            isClickable = true
        }
        inputBar.addView(micBtn)

        // Input text watcher – toggle send/mic
        inputField.addTextChangedListener(object : TextWatcher {
            override fun beforeTextChanged(s: CharSequence?, st: Int, c: Int, a: Int) {}
            override fun onTextChanged(s: CharSequence?, st: Int, b: Int, c: Int) {}
            override fun afterTextChanged(s: Editable?) {
                val hasText = !s.isNullOrBlank()
                if (hasText) {
                    sendButton.visibility = View.VISIBLE
                    micBtn.visibility = View.GONE
                    attachBtn.visibility = View.GONE
                } else {
                    sendButton.visibility = View.GONE
                    micBtn.visibility = View.VISIBLE
                    attachBtn.visibility = View.VISIBLE
                }
            }
        })

        // Send click
        sendButton.setOnClickListener {
            val text = inputField.text.toString().trim()
            if (text.isNotEmpty()) {
                sendMessage(text)
                inputField.setText("")
            }
        }

        return inputBar
    }

    // ── Send Message Logic ──

    private fun sendMessage(text: String) {
        val msg = ChatMessage(
            text = text,
            isOutgoing = true,
            status = MessageStatus.SENDING,
            time = System.currentTimeMillis()
        )
        messages.add(msg)
        adapter.notifyItemInserted(messages.size - 1)
        recyclerView.smoothScrollToPosition(messages.size - 1)

        // Animate status changes
        handler.postDelayed({
            val idx = messages.indexOf(msg)
            if (idx >= 0) {
                messages[idx] = msg.copy(status = MessageStatus.SENT)
                adapter.notifyItemChanged(idx)
            }
        }, 500)

        handler.postDelayed({
            val idx = messages.indexOf(messages.find { it.id == msg.id })
            if (idx >= 0) {
                messages[idx] = msg.copy(status = MessageStatus.DELIVERED)
                adapter.notifyItemChanged(idx)
            }
        }, 1200)

        handler.postDelayed({
            val idx = messages.indexOf(messages.find { it.id == msg.id })
            if (idx >= 0) {
                messages[idx] = msg.copy(status = MessageStatus.READ)
                adapter.notifyItemChanged(idx)
            }
        }, 2500)

        // Auto-reply
        handler.postDelayed({
            simulateTypingAndReply()
        }, 3000)
    }

    private fun simulateTypingAndReply() {
        val replies = listOf(
            "Хорошо, поняла! 👍",
            "Отличная идея! Давай так и сделаем 🎉",
            "Секунду, сейчас проверю...",
            "Да, конечно! Без проблем 😊",
            "Ого, интересно! Расскажи подробнее",
            "Ладно, договорились! 🤝",
            "Хм, надо подумать... 🤔"
        )

        val reply = ChatMessage(
            text = replies.random(),
            isOutgoing = false,
            senderName = "Алиса Смирнова",
            time = System.currentTimeMillis()
        )
        messages.add(reply)
        adapter.notifyItemInserted(messages.size - 1)
        recyclerView.smoothScrollToPosition(messages.size - 1)
    }

    // ── Demo Data ──

    private fun loadDemoMessages() {
        val cal = Calendar.getInstance()
        val baseTime = cal.timeInMillis

        messages.add(ChatMessage(text = "Сегодня", type = MessageType.DATE_HEADER,
            time = baseTime - 7200000))

        messages.add(ChatMessage(
            text = "Привет! Как дела? 😊",
            isOutgoing = false, senderName = "Алиса Смирнова",
            time = baseTime - 7200000, status = MessageStatus.READ
        ))
        messages.add(ChatMessage(
            text = "Привет, Алиса! Всё отлично, спасибо! Как твои выходные прошли?",
            isOutgoing = true,
            time = baseTime - 7100000, status = MessageStatus.READ
        ))
        messages.add(ChatMessage(
            text = "Супер! Ездили за город, было классно 🌿☀️ Погода была просто идеальная!",
            isOutgoing = false, senderName = "Алиса Смирнова",
            time = baseTime - 7000000, status = MessageStatus.READ
        ))
        messages.add(ChatMessage(
            text = "Здорово! А мы ходили на выставку современного искусства",
            isOutgoing = true,
            time = baseTime - 6900000, status = MessageStatus.READ
        ))
        messages.add(ChatMessage(
            text = "О, какую? Я тоже давно хотела сходить! 🎨",
            isOutgoing = false, senderName = "Алиса Смирнова",
            time = baseTime - 6800000
        ))
        messages.add(ChatMessage(
            text = "В Третьяковку, там новая экспозиция открылась. Очень рекомендую, прям до мурашек!",
            isOutgoing = true,
            time = baseTime - 6700000, status = MessageStatus.READ
        ))
        messages.add(ChatMessage(
            text = "Кстати, ты не забыла про встречу завтра в 15:00?",
            isOutgoing = true,
            time = baseTime - 5000000, status = MessageStatus.READ
        ))
        messages.add(ChatMessage(
            text = "Да, конечно помню! Буду вовремя ✅",
            isOutgoing = false, senderName = "Алиса Смирнова",
            time = baseTime - 4800000
        ))
        messages.add(ChatMessage(
            text = "Может возьмёшь с собой те документы по проекту? Хочу ещё раз посмотреть перед презентацией",
            isOutgoing = true,
            time = baseTime - 4600000, status = MessageStatus.READ,
            replyTo = "Да, конечно помню! Буду вовремя ✅"
        ))
        messages.add(ChatMessage(
            text = "Без проблем, уже собрала всё в папку 📂",
            isOutgoing = false, senderName = "Алиса Смирнова",
            time = baseTime - 4400000
        ))
        messages.add(ChatMessage(
            text = "Спасибо огромное! Ты лучшая 🙏",
            isOutgoing = true,
            time = baseTime - 4200000, status = MessageStatus.DELIVERED
        ))
        messages.add(ChatMessage(
            text = "Хахаха, стараюсь 😄 До завтра тогда!",
            isOutgoing = false, senderName = "Алиса Смирнова",
            time = baseTime - 4000000
        ))
    }

    // ── Vector Icon Drawables ──

    private fun createBackArrow(): Drawable {
        return object : Drawable() {
            val paint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
                color = Color.WHITE; style = Paint.Style.STROKE
                strokeWidth = 5f; strokeCap = Paint.Cap.ROUND; strokeJoin = Paint.Join.ROUND
            }
            override fun draw(canvas: Canvas) {
                val cx = bounds.exactCenterX(); val cy = bounds.exactCenterY()
                val s = bounds.width() * 0.28f
                canvas.drawLine(cx + s * 0.3f, cy - s, cx - s * 0.5f, cy, paint)
                canvas.drawLine(cx - s * 0.5f, cy, cx + s * 0.3f, cy + s, paint)
            }
            override fun setAlpha(a: Int) {}
            override fun setColorFilter(cf: ColorFilter?) {}
            override fun getOpacity() = PixelFormat.TRANSLUCENT
        }
    }

    private fun createSearchIcon(): Drawable {
        return object : Drawable() {
            val paint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
                color = Color.WHITE; style = Paint.Style.STROKE
                strokeWidth = 4.5f; strokeCap = Paint.Cap.ROUND
            }
            override fun draw(canvas: Canvas) {
                val cx = bounds.exactCenterX() - 2; val cy = bounds.exactCenterY() - 2
                val r = bounds.width() * 0.2f
                canvas.drawCircle(cx, cy, r, paint)
                val offset = r * 0.7f
                canvas.drawLine(cx + offset, cy + offset, cx + r + offset, cy + r + offset, paint)
            }
            override fun setAlpha(a: Int) {}
            override fun setColorFilter(cf: ColorFilter?) {}
            override fun getOpacity() = PixelFormat.TRANSLUCENT
        }
    }

    private fun createMoreIcon(): Drawable {
        return object : Drawable() {
            val paint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
                color = Color.WHITE; style = Paint.Style.FILL
            }
            override fun draw(canvas: Canvas) {
                val cx = bounds.exactCenterX(); val cy = bounds.exactCenterY()
                val r = 3.5f; val gap = 11f
                canvas.drawCircle(cx, cy - gap, r, paint)
                canvas.drawCircle(cx, cy, r, paint)
                canvas.drawCircle(cx, cy + gap, r, paint)
            }
            override fun setAlpha(a: Int) {}
            override fun setColorFilter(cf: ColorFilter?) {}
            override fun getOpacity() = PixelFormat.TRANSLUCENT
        }
    }

    private fun createEmojiIcon(): Drawable {
        return object : Drawable() {
            val paint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
                color = ChatColors.ATTACH_ICON; style = Paint.Style.STROKE
                strokeWidth = 4f; strokeCap = Paint.Cap.ROUND
            }
            val fillPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
                color = ChatColors.ATTACH_ICON; style = Paint.Style.FILL
            }
            override fun draw(canvas: Canvas) {
                val cx = bounds.exactCenterX(); val cy = bounds.exactCenterY()
                val r = bounds.width() * 0.33f
                canvas.drawCircle(cx, cy, r, paint)
                canvas.drawCircle(cx - r * 0.35f, cy - r * 0.2f, 3f, fillPaint)
                canvas.drawCircle(cx + r * 0.35f, cy - r * 0.2f, 3f, fillPaint)
                val smileRect = RectF(cx - r * 0.45f, cy - r * 0.1f, cx + r * 0.45f, cy + r * 0.55f)
                canvas.drawArc(smileRect, 10f, 160f, false, paint)
            }
            override fun setAlpha(a: Int) {}
            override fun setColorFilter(cf: ColorFilter?) {}
            override fun getOpacity() = PixelFormat.TRANSLUCENT
        }
    }

    private fun createAttachIcon(): Drawable {
        return object : Drawable() {
            val paint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
                color = ChatColors.ATTACH_ICON; style = Paint.Style.STROKE
                strokeWidth = 4.5f; strokeCap = Paint.Cap.ROUND; strokeJoin = Paint.Join.ROUND
            }
            override fun draw(canvas: Canvas) {
                val cx = bounds.exactCenterX(); val cy = bounds.exactCenterY()
                val s = bounds.width() * 0.25f
                val path = Path()
                path.moveTo(cx + s * 0.6f, cy - s * 0.6f)
                path.lineTo(cx - s * 0.2f, cy + s * 0.5f)
                path.quadTo(cx - s * 1.1f, cy + s * 1.3f, cx - s * 0.5f, cy + s * 0.1f)
                path.lineTo(cx + s * 0.4f, cy - s * 0.8f)
                path.quadTo(cx + s * 1.2f, cy - s * 1.5f, cx + s * 0.8f, cy - s * 0.3f)
                canvas.drawPath(path, paint)
            }
            override fun setAlpha(a: Int) {}
            override fun setColorFilter(cf: ColorFilter?) {}
            override fun getOpacity() = PixelFormat.TRANSLUCENT
        }
    }

    private fun createSendIcon(): Drawable {
        return object : Drawable() {
            val paint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
                color = Color.WHITE; style = Paint.Style.FILL
            }
            override fun draw(canvas: Canvas) {
                val w = bounds.width().toFloat(); val h = bounds.height().toFloat()
                val path = Path()
                path.moveTo(w * 0.15f, h * 0.5f)
                path.lineTo(w * 0.85f, h * 0.5f)
                path.moveTo(w * 0.55f, h * 0.2f)
                path.lineTo(w * 0.85f, h * 0.5f)
                path.lineTo(w * 0.55f, h * 0.8f)
                val sPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
                    color = Color.WHITE; style = Paint.Style.STROKE
                    strokeWidth = 5f; strokeCap = Paint.Cap.ROUND; strokeJoin = Paint.Join.ROUND
                }
                canvas.drawPath(path, sPaint)
            }
            override fun setAlpha(a: Int) {}
            override fun setColorFilter(cf: ColorFilter?) {}
            override fun getOpacity() = PixelFormat.TRANSLUCENT
        }
    }

    private fun createMicIcon(): Drawable {
        return object : Drawable() {
            val paint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
                color = Color.WHITE; style = Paint.Style.STROKE
                strokeWidth = 4f; strokeCap = Paint.Cap.ROUND
            }
            val fillPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
                color = Color.WHITE; style = Paint.Style.FILL
            }
            override fun draw(canvas: Canvas) {
                val cx = bounds.exactCenterX(); val cy = bounds.exactCenterY()
                val w = bounds.width() * 0.15f; val h = bounds.height() * 0.22f
                val rect = RectF(cx - w, cy - h - 4, cx + w, cy + 2)
                canvas.drawRoundRect(rect, w, w, fillPaint)
                val arcRect = RectF(cx - w * 1.7f, cy - h - 4, cx + w * 1.7f, cy + h * 0.7f)
                canvas.drawArc(arcRect, 0f, 180f, false, paint)
                canvas.drawLine(cx, cy + h * 0.7f, cx, cy + h * 1.4f, paint)
                canvas.drawLine(cx - w * 0.8f, cy + h * 1.4f, cx + w * 0.8f, cy + h * 1.4f, paint)
            }
            override fun setAlpha(a: Int) {}
            override fun setColorFilter(cf: ColorFilter?) {}
            override fun getOpacity() = PixelFormat.TRANSLUCENT
        }
    }
}
