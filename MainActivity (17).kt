package com.example.advancedchat

import android.Manifest
import android.animation.Animator
import android.animation.AnimatorListenerAdapter
import android.animation.AnimatorSet
import android.animation.ObjectAnimator
import android.animation.ValueAnimator
import android.annotation.SuppressLint
import android.app.*
import android.content.*
import android.content.pm.PackageManager
import android.graphics.*
import android.graphics.drawable.*
import android.media.*
import android.net.Uri
import android.os.*
import android.provider.MediaStore
import android.text.*
import android.text.style.*
import android.util.*
import android.view.*
import android.view.animation.*
import android.view.inputmethod.InputMethodManager
import android.widget.*
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.cardview.widget.CardView
import androidx.core.app.ActivityCompat
import androidx.core.app.NotificationCompat
import androidx.core.content.ContextCompat
import androidx.core.view.GestureDetectorCompat
import androidx.core.widget.addTextChangedListener
import androidx.recyclerview.widget.*
import kotlinx.coroutines.*
import java.io.*
import java.text.SimpleDateFormat
import java.util.*
import java.util.concurrent.ConcurrentHashMap
import kotlin.math.*

/**
 * Advanced Chat Application - Telegram-level Implementation
 * Single-file MainActivity with 2000+ lines of code
 * 
 * Features:
 * - Real-time messaging with typing indicators
 * - Voice messages with waveform visualization
 * - Image/Video/File attachments
 * - Message reactions and replies
 * - Swipe to reply gesture
 * - Message editing and deletion
 * - Search functionality
 * - Read receipts and delivery status
 * - Emoji picker with recent emojis
 * - Link preview generation
 * - Message forwarding
 * - Pin messages
 * - Scheduled messages
 * - Voice/Video call UI
 * - Dark/Light theme support
 * - Message animations
 * - Pull to refresh for history
 * - Infinite scroll pagination
 * - Offline message queue
 * - End-to-end encryption indicators
 * - User presence status
 * - Group chat support
 * - Admin controls
 * - Media gallery
 * - Document sharing
 * - Location sharing
 * - Contact sharing
 * - Sticker support
 * - GIF support
 * - Poll creation
 * - Message statistics
 */

class MainActivity : AppCompatActivity(), CoroutineScope by MainScope() {

    // ==================== CONSTANTS ====================
    companion object {
        private const val TAG = "AdvancedChat"
        private const val CHANNEL_ID = "chat_notifications"
        private const val NOTIFICATION_ID = 1001
        
        // Message Types
        const val MSG_TYPE_TEXT = 0
        const val MSG_TYPE_IMAGE = 1
        const val MSG_TYPE_VIDEO = 2
        const val MSG_TYPE_VOICE = 3
        const val MSG_TYPE_FILE = 4
        const val MSG_TYPE_LOCATION = 5
        const val MSG_TYPE_CONTACT = 6
        const val MSG_TYPE_STICKER = 7
        const val MSG_TYPE_GIF = 8
        const val MSG_TYPE_POLL = 9
        const val MSG_TYPE_SYSTEM = 10
        const val MSG_TYPE_CALL = 11
        const val MSG_TYPE_LINK_PREVIEW = 12
        
        // Message Status
        const val STATUS_SENDING = 0
        const val STATUS_SENT = 1
        const val STATUS_DELIVERED = 2
        const val STATUS_READ = 3
        const val STATUS_FAILED = 4
        
        // View Types for RecyclerView
        const val VIEW_TYPE_INCOMING = 0
        const val VIEW_TYPE_OUTGOING = 1
        const val VIEW_TYPE_DATE_HEADER = 2
        const val VIEW_TYPE_SYSTEM = 3
        const val VIEW_TYPE_TYPING = 4
        
        // Permissions
        const val PERMISSION_AUDIO = 100
        const val PERMISSION_STORAGE = 101
        const val PERMISSION_CAMERA = 102
        const val PERMISSION_LOCATION = 103
        
        // Animation durations
        const val ANIM_DURATION_SHORT = 150L
        const val ANIM_DURATION_MEDIUM = 300L
        const val ANIM_DURATION_LONG = 500L
        
        // Pagination
        const val PAGE_SIZE = 50
        const val SCROLL_THRESHOLD = 10
        
        // Voice recording
        const val MAX_VOICE_DURATION = 300000L // 5 minutes
        const val VOICE_AMPLITUDE_UPDATE_INTERVAL = 50L
        
        // Typing indicator timeout
        const val TYPING_TIMEOUT = 3000L
        
        // Colors
        val COLOR_BUBBLE_OUTGOING = Color.parseColor("#E3F2FD")
        val COLOR_BUBBLE_INCOMING = Color.parseColor("#FFFFFF")
        val COLOR_ACCENT = Color.parseColor("#2196F3")
        val COLOR_ONLINE = Color.parseColor("#4CAF50")
        val COLOR_OFFLINE = Color.parseColor("#9E9E9E")
        val COLOR_TYPING = Color.parseColor("#FF9800")
        
        // Emoji categories
        val EMOJI_SMILEYS = listOf("😀", "😃", "😄", "😁", "😆", "😅", "🤣", "😂", "🙂", "🙃",
            "😉", "😊", "😇", "🥰", "😍", "🤩", "😘", "😗", "😚", "😙",
            "🥲", "😋", "😛", "😜", "🤪", "😝", "🤑", "🤗", "🤭", "🤫")
        
        val EMOJI_GESTURES = listOf("👍", "👎", "👌", "🤌", "🤏", "✌️", "🤞", "🤟", "🤘", "🤙",
            "👈", "👉", "👆", "👇", "☝️", "✋", "🤚", "🖐", "🖖", "👋")
        
        val EMOJI_HEARTS = listOf("❤️", "🧡", "💛", "💚", "💙", "💜", "🖤", "🤍", "🤎", "💔",
            "❣️", "💕", "💞", "💓", "💗", "💖", "💘", "💝", "💟", "♥️")
        
        val REACTIONS = listOf("👍", "👎", "❤️", "🔥", "🎉", "😢", "😮", "😂")
    }

    // ==================== DATA CLASSES ====================
    
    data class Message(
        val id: String = UUID.randomUUID().toString(),
        var text: String = "",
        var type: Int = MSG_TYPE_TEXT,
        var status: Int = STATUS_SENDING,
        val senderId: String = "",
        val senderName: String = "",
        val senderAvatar: String? = null,
        val timestamp: Long = System.currentTimeMillis(),
        var editedAt: Long? = null,
        var isEdited: Boolean = false,
        var isDeleted: Boolean = false,
        var isForwarded: Boolean = false,
        var forwardedFrom: String? = null,
        var replyToId: String? = null,
        var replyToText: String? = null,
        var replyToSender: String? = null,
        var mediaUrl: String? = null,
        var mediaThumbnail: String? = null,
        var mediaSize: Long = 0,
        var mediaDuration: Long = 0,
        var mediaWidth: Int = 0,
        var mediaHeight: Int = 0,
        var fileName: String? = null,
        var voiceWaveform: List<Int>? = null,
        var latitude: Double? = null,
        var longitude: Double? = null,
        var contactName: String? = null,
        var contactPhone: String? = null,
        var pollQuestion: String? = null,
        var pollOptions: List<PollOption>? = null,
        var linkPreview: LinkPreview? = null,
        var reactions: MutableMap<String, MutableList<String>> = mutableMapOf(),
        var isPinned: Boolean = false,
        var scheduledTime: Long? = null,
        var isEncrypted: Boolean = true,
        var readBy: MutableList<String> = mutableListOf(),
        var deliveredTo: MutableList<String> = mutableListOf()
    ) {
        fun getFormattedTime(): String {
            val sdf = SimpleDateFormat("HH:mm", Locale.getDefault())
            return sdf.format(Date(timestamp))
        }
        
        fun getFormattedDate(): String {
            val sdf = SimpleDateFormat("dd MMMM yyyy", Locale.getDefault())
            return sdf.format(Date(timestamp))
        }
        
        fun isOutgoing(currentUserId: String): Boolean = senderId == currentUserId
        
        fun getTotalReactions(): Int = reactions.values.sumOf { it.size }
    }
    
    data class PollOption(
        val id: String = UUID.randomUUID().toString(),
        val text: String,
        var votes: MutableList<String> = mutableListOf()
    ) {
        fun getPercentage(totalVoters: Int): Int {
            if (totalVoters == 0) return 0
            return (votes.size * 100) / totalVoters
        }
    }
    
    data class LinkPreview(
        val url: String,
        val title: String? = null,
        val description: String? = null,
        val imageUrl: String? = null,
        val siteName: String? = null
    )
    
    data class User(
        val id: String,
        val name: String,
        val avatar: String? = null,
        var isOnline: Boolean = false,
        var lastSeen: Long = 0,
        var isTyping: Boolean = false,
        var typingInChat: String? = null,
        var isBlocked: Boolean = false,
        var isMuted: Boolean = false,
        var customNotificationSound: String? = null,
        var wallpaper: String? = null
    ) {
        fun getLastSeenText(): String {
            if (isOnline) return "в сети"
            
            val now = System.currentTimeMillis()
            val diff = now - lastSeen
            
            return when {
                diff < 60000 -> "был(а) только что"
                diff < 3600000 -> "был(а) ${diff / 60000} мин. назад"
                diff < 86400000 -> "был(а) ${diff / 3600000} ч. назад"
                else -> {
                    val sdf = SimpleDateFormat("dd.MM.yy в HH:mm", Locale.getDefault())
                    "был(а) ${sdf.format(Date(lastSeen))}"
                }
            }
        }
    }
    
    data class Chat(
        val id: String,
        val name: String,
        val avatar: String? = null,
        var isGroup: Boolean = false,
        var participants: MutableList<User> = mutableListOf(),
        var admins: MutableList<String> = mutableListOf(),
        var creatorId: String = "",
        var lastMessage: Message? = null,
        var unreadCount: Int = 0,
        var isPinned: Boolean = false,
        var isMuted: Boolean = false,
        var muteUntil: Long? = null,
        var isArchived: Boolean = false,
        var draftMessage: String? = null,
        var pinnedMessages: MutableList<String> = mutableListOf(),
        var chatWallpaper: String? = null,
        var slowModeDelay: Int = 0,
        var description: String? = null,
        var inviteLink: String? = null
    ) {
        fun getOnlineCount(): Int = participants.count { it.isOnline }
        
        fun getTypingUsers(): List<User> = participants.filter { it.isTyping && it.typingInChat == id }
        
        fun getTypingText(): String? {
            val typingUsers = getTypingUsers()
            return when {
                typingUsers.isEmpty() -> null
                typingUsers.size == 1 -> "${typingUsers[0].name} печатает..."
                typingUsers.size == 2 -> "${typingUsers[0].name} и ${typingUsers[1].name} печатают..."
                else -> "${typingUsers.size} участников печатают..."
            }
        }
    }
    
    data class VoiceRecordingState(
        var isRecording: Boolean = false,
        var startTime: Long = 0,
        var amplitudes: MutableList<Int> = mutableListOf(),
        var filePath: String? = null,
        var isPaused: Boolean = false,
        var pausedDuration: Long = 0
    ) {
        fun getDuration(): Long {
            return if (isRecording) {
                System.currentTimeMillis() - startTime - pausedDuration
            } else 0
        }
        
        fun getFormattedDuration(): String {
            val duration = getDuration()
            val minutes = (duration / 1000) / 60
            val seconds = (duration / 1000) % 60
            return String.format("%02d:%02d", minutes, seconds)
        }
    }
    
    data class MessageSelection(
        val selectedIds: MutableSet<String> = mutableSetOf(),
        var isSelectionMode: Boolean = false
    ) {
        fun toggle(messageId: String) {
            if (selectedIds.contains(messageId)) {
                selectedIds.remove(messageId)
            } else {
                selectedIds.add(messageId)
            }
            isSelectionMode = selectedIds.isNotEmpty()
        }
        
        fun clear() {
            selectedIds.clear()
            isSelectionMode = false
        }
        
        fun selectAll(messageIds: List<String>) {
            selectedIds.addAll(messageIds)
            isSelectionMode = true
        }
    }

    // ==================== UI COMPONENTS ====================
    
    private lateinit var rootLayout: FrameLayout
    private lateinit var mainContainer: LinearLayout
    private lateinit var toolbar: RelativeLayout
    private lateinit var toolbarTitle: TextView
    private lateinit var toolbarSubtitle: TextView
    private lateinit var toolbarAvatar: ImageView
    private lateinit var toolbarOnlineIndicator: View
    private lateinit var toolbarBackButton: ImageView
    private lateinit var toolbarMenuButton: ImageView
    private lateinit var toolbarCallButton: ImageView
    private lateinit var toolbarVideoCallButton: ImageView
    private lateinit var toolbarSearchButton: ImageView
    
    private lateinit var recyclerView: RecyclerView
    private lateinit var messageAdapter: MessageAdapter
    private lateinit var layoutManager: LinearLayoutManager
    
    private lateinit var inputContainer: LinearLayout
    private lateinit var inputWrapper: LinearLayout
    private lateinit var messageInput: EditText
    private lateinit var sendButton: ImageView
    private lateinit var attachButton: ImageView
    private lateinit var emojiButton: ImageView
    private lateinit var voiceButton: ImageView
    private lateinit var cameraButton: ImageView
    
    private lateinit var replyContainer: LinearLayout
    private lateinit var replyText: TextView
    private lateinit var replySender: TextView
    private lateinit var replyCloseButton: ImageView
    private lateinit var replyIndicator: View
    
    private lateinit var editContainer: LinearLayout
    private lateinit var editText: TextView
    private lateinit var editCloseButton: ImageView
    
    private lateinit var voiceRecordingContainer: LinearLayout
    private lateinit var voiceRecordingTimer: TextView
    private lateinit var voiceRecordingWaveform: VoiceWaveformView
    private lateinit var voiceRecordingCancel: TextView
    private lateinit var voiceRecordingLock: ImageView
    private lateinit var voiceSendButton: ImageView
    private lateinit var voicePauseButton: ImageView
    
    private lateinit var emojiContainer: LinearLayout
    private lateinit var emojiPager: HorizontalScrollView
    private lateinit var emojiGrid: GridLayout
    private lateinit var recentEmojisRow: LinearLayout
    
    private lateinit var attachmentMenu: LinearLayout
    private lateinit var attachPhotoButton: LinearLayout
    private lateinit var attachVideoButton: LinearLayout
    private lateinit var attachFileButton: LinearLayout
    private lateinit var attachLocationButton: LinearLayout
    private lateinit var attachContactButton: LinearLayout
    private lateinit var attachPollButton: LinearLayout
    
    private lateinit var scrollToBottomButton: ImageView
    private lateinit var unreadBadge: TextView
    
    private lateinit var searchContainer: LinearLayout
    private lateinit var searchInput: EditText
    private lateinit var searchResultsCount: TextView
    private lateinit var searchPrevButton: ImageView
    private lateinit var searchNextButton: ImageView
    private lateinit var searchCloseButton: ImageView
    
    private lateinit var selectionToolbar: LinearLayout
    private lateinit var selectionCount: TextView
    private lateinit var selectionDeleteButton: ImageView
    private lateinit var selectionForwardButton: ImageView
    private lateinit var selectionCopyButton: ImageView
    private lateinit var selectionReplyButton: ImageView
    private lateinit var selectionCloseButton: ImageView
    
    private lateinit var pinnedMessageBar: LinearLayout
    private lateinit var pinnedMessageText: TextView
    private lateinit var pinnedMessageCount: TextView
    
    private lateinit var loadingOverlay: FrameLayout
    private lateinit var progressBar: ProgressBar
    
    // ==================== STATE VARIABLES ====================
    
    private var currentUserId: String = "user_001"
    private var currentUserName: String = "Текущий пользователь"
    private var currentChat: Chat? = null
    private var messages: MutableList<Message> = mutableListOf()
    private var filteredMessages: MutableList<Any> = mutableListOf() // Messages + DateHeaders
    
    private var replyToMessage: Message? = null
    private var editingMessage: Message? = null
    private var selection = MessageSelection()
    
    private var voiceRecordingState = VoiceRecordingState()
    private var mediaRecorder: MediaRecorder? = null
    private var mediaPlayer: MediaPlayer? = null
    private var playingVoiceMessageId: String? = null
    
    private var isEmojiKeyboardVisible = false
    private var isAttachmentMenuVisible = false
    private var isSearchVisible = false
    private var searchResults: List<Int> = emptyList()
    private var currentSearchIndex = 0
    
    private var recentEmojis: MutableList<String> = mutableListOf()
    private var typingJob: Job? = null
    private var lastTypingTime: Long = 0
    
    private var currentPage = 0
    private var isLoading = false
    private var hasMoreMessages = true
    
    private var isDarkTheme = false
    private var currentPinnedIndex = 0
    
    private val offlineMessageQueue: MutableList<Message> = mutableListOf()
    private val messageCache: ConcurrentHashMap<String, Message> = ConcurrentHashMap()
    
    // ==================== GESTURE DETECTION ====================
    
    private lateinit var swipeGestureDetector: GestureDetectorCompat
    private var swipedMessagePosition: Int = -1
    private var isSwipeInProgress = false
    
    // ==================== LIFECYCLE ====================
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        // Load preferences
        loadPreferences()
        
        // Apply theme
        applyTheme()
        
        // Create UI programmatically
        createUI()
        
        // Setup components
        setupToolbar()
        setupRecyclerView()
        setupInputArea()
        setupEmojiKeyboard()
        setupAttachmentMenu()
        setupSearch()
        setupSelectionToolbar()
        setupPinnedMessages()
        setupVoiceRecording()
        setupGestures()
        
        // Initialize data
        initializeMockData()
        
        // Create notification channel
        createNotificationChannel()
        
        // Request permissions
        requestRequiredPermissions()
    }
    
    override fun onResume() {
        super.onResume()
        markMessagesAsRead()
        simulateOnlineStatus(true)
    }
    
    override fun onPause() {
        super.onPause()
        saveDraft()
        simulateOnlineStatus(false)
        stopVoiceRecording(cancel = true)
    }
    
    override fun onDestroy() {
        super.onDestroy()
        cancel() // Cancel coroutine scope
        mediaPlayer?.release()
        mediaRecorder?.release()
    }
    
    override fun onBackPressed() {
        when {
            selection.isSelectionMode -> {
                selection.clear()
                updateSelectionUI()
            }
            isEmojiKeyboardVisible -> hideEmojiKeyboard()
            isAttachmentMenuVisible -> hideAttachmentMenu()
            isSearchVisible -> hideSearch()
            else -> super.onBackPressed()
        }
    }
    
    // ==================== UI CREATION ====================
    
    @SuppressLint("SetTextI18n")
    private fun createUI() {
        rootLayout = FrameLayout(this).apply {
            layoutParams = ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
            setBackgroundColor(if (isDarkTheme) Color.parseColor("#1A1A1A") else Color.parseColor("#E5DDD5"))
        }
        
        mainContainer = LinearLayout(this).apply {
            layoutParams = FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT
            )
            orientation = LinearLayout.VERTICAL
        }
        
        // Create all UI components
        createToolbar()
        createSearchBar()
        createPinnedMessageBar()
        createSelectionToolbar()
        createRecyclerView()
        createReplyContainer()
        createEditContainer()
        createInputArea()
        createEmojiKeyboard()
        createAttachmentMenu()
        createVoiceRecordingUI()
        createScrollToBottomButton()
        createLoadingOverlay()
        
        // Add components to containers
        mainContainer.addView(toolbar)
        mainContainer.addView(searchContainer)
        mainContainer.addView(pinnedMessageBar)
        mainContainer.addView(selectionToolbar)
        mainContainer.addView(recyclerView)
        mainContainer.addView(replyContainer)
        mainContainer.addView(editContainer)
        mainContainer.addView(voiceRecordingContainer)
        mainContainer.addView(inputContainer)
        mainContainer.addView(attachmentMenu)
        mainContainer.addView(emojiContainer)
        
        rootLayout.addView(mainContainer)
        rootLayout.addView(scrollToBottomButton)
        rootLayout.addView(loadingOverlay)
        
        setContentView(rootLayout)
        
        // Initial visibility states
        searchContainer.visibility = View.GONE
        selectionToolbar.visibility = View.GONE
        replyContainer.visibility = View.GONE
        editContainer.visibility = View.GONE
        voiceRecordingContainer.visibility = View.GONE
        attachmentMenu.visibility = View.GONE
        emojiContainer.visibility = View.GONE
        scrollToBottomButton.visibility = View.GONE
        pinnedMessageBar.visibility = View.GONE
        loadingOverlay.visibility = View.GONE
    }
    
    private fun createToolbar() {
        toolbar = RelativeLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                dp(56)
            )
            setBackgroundColor(if (isDarkTheme) Color.parseColor("#212121") else COLOR_ACCENT)
            elevation = dp(4).toFloat()
            setPadding(dp(8), 0, dp(8), 0)
        }
        
        toolbarBackButton = ImageView(this).apply {
            id = View.generateViewId()
            layoutParams = RelativeLayout.LayoutParams(dp(40), dp(40)).apply {
                addRule(RelativeLayout.ALIGN_PARENT_START)
                addRule(RelativeLayout.CENTER_VERTICAL)
            }
            setImageResource(android.R.drawable.ic_menu_revert)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { onBackPressed() }
        }
        
        toolbarAvatar = ImageView(this).apply {
            id = View.generateViewId()
            layoutParams = RelativeLayout.LayoutParams(dp(40), dp(40)).apply {
                addRule(RelativeLayout.END_OF, toolbarBackButton.id)
                addRule(RelativeLayout.CENTER_VERTICAL)
                marginStart = dp(8)
            }
            setImageResource(android.R.drawable.ic_menu_myplaces)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER_CROP
            background = createCircleDrawable(Color.parseColor("#64B5F6"))
        }
        
        toolbarOnlineIndicator = View(this).apply {
            layoutParams = RelativeLayout.LayoutParams(dp(12), dp(12)).apply {
                addRule(RelativeLayout.ALIGN_END, toolbarAvatar.id)
                addRule(RelativeLayout.ALIGN_BOTTOM, toolbarAvatar.id)
            }
            background = createCircleDrawable(COLOR_ONLINE, Color.WHITE, dp(2))
            visibility = View.GONE
        }
        
        val titleContainer = LinearLayout(this).apply {
            id = View.generateViewId()
            layoutParams = RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                addRule(RelativeLayout.END_OF, toolbarAvatar.id)
                addRule(RelativeLayout.CENTER_VERTICAL)
                marginStart = dp(12)
            }
            orientation = LinearLayout.VERTICAL
            setOnClickListener { showChatInfo() }
        }
        
        toolbarTitle = TextView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
            text = "Чат"
            setTextColor(Color.WHITE)
            textSize = 17f
            typeface = Typeface.DEFAULT_BOLD
            maxLines = 1
            ellipsize = TextUtils.TruncateAt.END
        }
        
        toolbarSubtitle = TextView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
            text = "в сети"
            setTextColor(Color.parseColor("#B3FFFFFF"))
            textSize = 13f
            maxLines = 1
            ellipsize = TextUtils.TruncateAt.END
        }
        
        titleContainer.addView(toolbarTitle)
        titleContainer.addView(toolbarSubtitle)
        
        // Right side buttons
        toolbarMenuButton = ImageView(this).apply {
            id = View.generateViewId()
            layoutParams = RelativeLayout.LayoutParams(dp(40), dp(40)).apply {
                addRule(RelativeLayout.ALIGN_PARENT_END)
                addRule(RelativeLayout.CENTER_VERTICAL)
            }
            setImageResource(android.R.drawable.ic_menu_more)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { showChatMenu() }
        }
        
        toolbarSearchButton = ImageView(this).apply {
            id = View.generateViewId()
            layoutParams = RelativeLayout.LayoutParams(dp(40), dp(40)).apply {
                addRule(RelativeLayout.START_OF, toolbarMenuButton.id)
                addRule(RelativeLayout.CENTER_VERTICAL)
            }
            setImageResource(android.R.drawable.ic_menu_search)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { showSearch() }
        }
        
        toolbarVideoCallButton = ImageView(this).apply {
            id = View.generateViewId()
            layoutParams = RelativeLayout.LayoutParams(dp(40), dp(40)).apply {
                addRule(RelativeLayout.START_OF, toolbarSearchButton.id)
                addRule(RelativeLayout.CENTER_VERTICAL)
            }
            setImageResource(android.R.drawable.ic_menu_camera)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { startVideoCall() }
        }
        
        toolbarCallButton = ImageView(this).apply {
            id = View.generateViewId()
            layoutParams = RelativeLayout.LayoutParams(dp(40), dp(40)).apply {
                addRule(RelativeLayout.START_OF, toolbarVideoCallButton.id)
                addRule(RelativeLayout.CENTER_VERTICAL)
            }
            setImageResource(android.R.drawable.ic_menu_call)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { startVoiceCall() }
        }
        
        toolbar.addView(toolbarBackButton)
        toolbar.addView(toolbarAvatar)
        toolbar.addView(toolbarOnlineIndicator)
        toolbar.addView(titleContainer)
        toolbar.addView(toolbarMenuButton)
        toolbar.addView(toolbarSearchButton)
        toolbar.addView(toolbarVideoCallButton)
        toolbar.addView(toolbarCallButton)
    }
    
    private fun createSearchBar() {
        searchContainer = LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                dp(56)
            )
            orientation = LinearLayout.HORIZONTAL
            setBackgroundColor(if (isDarkTheme) Color.parseColor("#212121") else Color.WHITE)
            elevation = dp(4).toFloat()
            gravity = Gravity.CENTER_VERTICAL
            setPadding(dp(8), 0, dp(8), 0)
        }
        
        searchCloseButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(40), dp(40))
            setImageResource(android.R.drawable.ic_menu_close_clear_cancel)
            setColorFilter(if (isDarkTheme) Color.WHITE else Color.GRAY)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { hideSearch() }
        }
        
        searchInput = EditText(this).apply {
            layoutParams = LinearLayout.LayoutParams(0, dp(40), 1f).apply {
                marginStart = dp(8)
                marginEnd = dp(8)
            }
            hint = "Поиск сообщений..."
            setHintTextColor(Color.GRAY)
            setTextColor(if (isDarkTheme) Color.WHITE else Color.BLACK)
            textSize = 16f
            background = null
            isSingleLine = true
            addTextChangedListener { performSearch(it.toString()) }
        }
        
        searchResultsCount = TextView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
            text = "0/0"
            setTextColor(Color.GRAY)
            textSize = 14f
        }
        
        searchPrevButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(36), dp(36))
            setImageResource(android.R.drawable.arrow_up_float)
            setColorFilter(if (isDarkTheme) Color.WHITE else Color.GRAY)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { navigateSearchResult(-1) }
        }
        
        searchNextButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(36), dp(36))
            setImageResource(android.R.drawable.arrow_down_float)
            setColorFilter(if (isDarkTheme) Color.WHITE else Color.GRAY)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { navigateSearchResult(1) }
        }
        
        searchContainer.addView(searchCloseButton)
        searchContainer.addView(searchInput)
        searchContainer.addView(searchResultsCount)
        searchContainer.addView(searchPrevButton)
        searchContainer.addView(searchNextButton)
    }
    
    private fun createPinnedMessageBar() {
        pinnedMessageBar = LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                dp(48)
            )
            orientation = LinearLayout.HORIZONTAL
            setBackgroundColor(if (isDarkTheme) Color.parseColor("#2D2D2D") else Color.parseColor("#F5F5F5"))
            gravity = Gravity.CENTER_VERTICAL
            setPadding(dp(16), 0, dp(8), 0)
            setOnClickListener { scrollToPinnedMessage() }
        }
        
        val pinnedIcon = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(20), dp(20))
            setImageResource(android.R.drawable.ic_menu_myplaces)
            setColorFilter(COLOR_ACCENT)
        }
        
        val pinnedTextContainer = LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f).apply {
                marginStart = dp(12)
            }
            orientation = LinearLayout.VERTICAL
        }
        
        pinnedMessageCount = TextView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
            text = "Закрепленное сообщение"
            setTextColor(COLOR_ACCENT)
            textSize = 12f
        }
        
        pinnedMessageText = TextView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
            setTextColor(if (isDarkTheme) Color.WHITE else Color.BLACK)
            textSize = 14f
            maxLines = 1
            ellipsize = TextUtils.TruncateAt.END
        }
        
        pinnedTextContainer.addView(pinnedMessageCount)
        pinnedTextContainer.addView(pinnedMessageText)
        
        val closeButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(36), dp(36))
            setImageResource(android.R.drawable.ic_menu_close_clear_cancel)
            setColorFilter(Color.GRAY)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { hidePinnedMessageBar() }
        }
        
        pinnedMessageBar.addView(pinnedIcon)
        pinnedMessageBar.addView(pinnedTextContainer)
        pinnedMessageBar.addView(closeButton)
    }
    
    private fun createSelectionToolbar() {
        selectionToolbar = LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                dp(56)
            )
            orientation = LinearLayout.HORIZONTAL
            setBackgroundColor(if (isDarkTheme) Color.parseColor("#212121") else COLOR_ACCENT)
            elevation = dp(4).toFloat()
            gravity = Gravity.CENTER_VERTICAL
            setPadding(dp(8), 0, dp(8), 0)
        }
        
        selectionCloseButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(40), dp(40))
            setImageResource(android.R.drawable.ic_menu_close_clear_cancel)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { 
                selection.clear()
                updateSelectionUI()
            }
        }
        
        selectionCount = TextView(this).apply {
            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f).apply {
                marginStart = dp(16)
            }
            setTextColor(Color.WHITE)
            textSize = 18f
            typeface = Typeface.DEFAULT_BOLD
        }
        
        selectionReplyButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(40), dp(40))
            setImageResource(android.R.drawable.ic_menu_revert)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { replyToSelectedMessages() }
        }
        
        selectionCopyButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(40), dp(40))
            setImageResource(android.R.drawable.ic_menu_edit)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { copySelectedMessages() }
        }
        
        selectionForwardButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(40), dp(40))
            setImageResource(android.R.drawable.ic_menu_share)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { forwardSelectedMessages() }
        }
        
        selectionDeleteButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(40), dp(40))
            setImageResource(android.R.drawable.ic_menu_delete)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { deleteSelectedMessages() }
        }
        
        selectionToolbar.addView(selectionCloseButton)
        selectionToolbar.addView(selectionCount)
        selectionToolbar.addView(selectionReplyButton)
        selectionToolbar.addView(selectionCopyButton)
        selectionToolbar.addView(selectionForwardButton)
        selectionToolbar.addView(selectionDeleteButton)
    }
    
    private fun createRecyclerView() {
        recyclerView = RecyclerView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                0,
                1f
            )
            clipToPadding = false
            setPadding(0, dp(8), 0, dp(8))
            overScrollMode = View.OVER_SCROLL_NEVER
        }
    }
    
    private fun createReplyContainer() {
        replyContainer = LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                dp(52)
            )
            orientation = LinearLayout.HORIZONTAL
            setBackgroundColor(if (isDarkTheme) Color.parseColor("#2D2D2D") else Color.WHITE)
            gravity = Gravity.CENTER_VERTICAL
            setPadding(dp(12), dp(8), dp(12), dp(8))
        }
        
        replyIndicator = View(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(3), LinearLayout.LayoutParams.MATCH_PARENT).apply {
                marginEnd = dp(12)
            }
            setBackgroundColor(COLOR_ACCENT)
        }
        
        val replyTextContainer = LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f)
            orientation = LinearLayout.VERTICAL
        }
        
        replySender = TextView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
            setTextColor(COLOR_ACCENT)
            textSize = 14f
            typeface = Typeface.DEFAULT_BOLD
        }
        
        replyText = TextView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
            setTextColor(if (isDarkTheme) Color.LTGRAY else Color.DKGRAY)
            textSize = 14f
            maxLines = 1
            ellipsize = TextUtils.TruncateAt.END
        }
        
        replyTextContainer.addView(replySender)
        replyTextContainer.addView(replyText)
        
        replyCloseButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(36), dp(36))
            setImageResource(android.R.drawable.ic_menu_close_clear_cancel)
            setColorFilter(Color.GRAY)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { cancelReply() }
        }
        
        replyContainer.addView(replyIndicator)
        replyContainer.addView(replyTextContainer)
        replyContainer.addView(replyCloseButton)
    }
    
    private fun createEditContainer() {
        editContainer = LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                dp(52)
            )
            orientation = LinearLayout.HORIZONTAL
            setBackgroundColor(if (isDarkTheme) Color.parseColor("#2D2D2D") else Color.WHITE)
            gravity = Gravity.CENTER_VERTICAL
            setPadding(dp(12), dp(8), dp(12), dp(8))
        }
        
        val editIcon = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(24), dp(24)).apply {
                marginEnd = dp(12)
            }
            setImageResource(android.R.drawable.ic_menu_edit)
            setColorFilter(COLOR_ACCENT)
        }
        
        val editLabel = TextView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                marginEnd = dp(8)
            }
            text = "Редактирование"
            setTextColor(COLOR_ACCENT)
            textSize = 14f
            typeface = Typeface.DEFAULT_BOLD
        }
        
        editText = TextView(this).apply {
            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f)
            setTextColor(if (isDarkTheme) Color.LTGRAY else Color.DKGRAY)
            textSize = 14f
            maxLines = 1
            ellipsize = TextUtils.TruncateAt.END
        }
        
        editCloseButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(36), dp(36))
            setImageResource(android.R.drawable.ic_menu_close_clear_cancel)
            setColorFilter(Color.GRAY)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { cancelEdit() }
        }
        
        editContainer.addView(editIcon)
        editContainer.addView(editLabel)
        editContainer.addView(editText)
        editContainer.addView(editCloseButton)
    }
    
    private fun createInputArea() {
        inputContainer = LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
            orientation = LinearLayout.HORIZONTAL
            setBackgroundColor(if (isDarkTheme) Color.parseColor("#212121") else Color.WHITE)
            gravity = Gravity.CENTER_VERTICAL
            setPadding(dp(4), dp(6), dp(4), dp(6))
        }
        
        emojiButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(44), dp(44))
            setImageResource(android.R.drawable.ic_menu_compass)
            setColorFilter(Color.GRAY)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { toggleEmojiKeyboard() }
        }
        
        inputWrapper = LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f).apply {
                marginStart = dp(4)
                marginEnd = dp(4)
            }
            orientation = LinearLayout.HORIZONTAL
            background = createRoundedDrawable(
                if (isDarkTheme) Color.parseColor("#333333") else Color.parseColor("#F0F0F0"),
                dp(24).toFloat()
            )
            gravity = Gravity.CENTER_VERTICAL
            setPadding(dp(12), dp(8), dp(12), dp(8))
        }
        
        messageInput = EditText(this).apply {
            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f)
            hint = "Сообщение"
            setHintTextColor(Color.GRAY)
            setTextColor(if (isDarkTheme) Color.WHITE else Color.BLACK)
            textSize = 16f
            background = null
            maxLines = 6
            inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_MULTI_LINE or InputType.TYPE_TEXT_FLAG_CAP_SENTENCES
            
            addTextChangedListener(object : TextWatcher {
                override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {}
                override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {
                    updateInputButtons()
                    sendTypingIndicator()
                }
                override fun afterTextChanged(s: Editable?) {}
            })
        }
        
        attachButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(36), dp(36))
            setImageResource(android.R.drawable.ic_input_add)
            setColorFilter(Color.GRAY)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { toggleAttachmentMenu() }
        }
        
        cameraButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(36), dp(36))
            setImageResource(android.R.drawable.ic_menu_camera)
            setColorFilter(Color.GRAY)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { openCamera() }
        }
        
        inputWrapper.addView(messageInput)
        inputWrapper.addView(attachButton)
        inputWrapper.addView(cameraButton)
        
        sendButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(48), dp(48))
            setImageResource(android.R.drawable.ic_menu_send)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER
            background = createCircleDrawable(COLOR_ACCENT)
            elevation = dp(2).toFloat()
            visibility = View.GONE
            setOnClickListener { sendMessage() }
        }
        
        voiceButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(48), dp(48))
            setImageResource(android.R.drawable.ic_btn_speak_now)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER
            background = createCircleDrawable(COLOR_ACCENT)
            elevation = dp(2).toFloat()
            setOnTouchListener { _, event -> handleVoiceButtonTouch(event) }
        }
        
        inputContainer.addView(emojiButton)
        inputContainer.addView(inputWrapper)
        inputContainer.addView(sendButton)
        inputContainer.addView(voiceButton)
    }
    
    private fun createEmojiKeyboard() {
        emojiContainer = LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                dp(250)
            )
            orientation = LinearLayout.VERTICAL
            setBackgroundColor(if (isDarkTheme) Color.parseColor("#1A1A1A") else Color.parseColor("#F5F5F5"))
        }
        
        // Category tabs
        val tabsContainer = LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                dp(44)
            )
            orientation = LinearLayout.HORIZONTAL
            setBackgroundColor(if (isDarkTheme) Color.parseColor("#212121") else Color.WHITE)
            gravity = Gravity.CENTER_VERTICAL
        }
        
        val categories = listOf("🕐", "😀", "👍", "❤️", "🎉", "🔥")
        categories.forEachIndexed { index, emoji ->
            val tab = TextView(this).apply {
                layoutParams = LinearLayout.LayoutParams(0, dp(44), 1f)
                text = emoji
                textSize = 20f
                gravity = Gravity.CENTER
                setBackgroundResource(selectableItemBackground())
                setOnClickListener { switchEmojiCategory(index) }
            }
            tabsContainer.addView(tab)
        }
        
        // Recent emojis row
        recentEmojisRow = LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                dp(44)
            )
            orientation = LinearLayout.HORIZONTAL
            setPadding(dp(8), 0, dp(8), 0)
            gravity = Gravity.CENTER_VERTICAL
        }
        
        // Emoji grid
        emojiPager = HorizontalScrollView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                0,
                1f
            )
            isHorizontalScrollBarEnabled = false
        }
        
        emojiGrid = GridLayout(this).apply {
            columnCount = 8
            setPadding(dp(8), dp(8), dp(8), dp(8))
        }
        
        populateEmojiGrid(EMOJI_SMILEYS + EMOJI_GESTURES + EMOJI_HEARTS)
        
        emojiPager.addView(emojiGrid)
        
        emojiContainer.addView(tabsContainer)
        emojiContainer.addView(recentEmojisRow)
        emojiContainer.addView(emojiPager)
    }
    
    private fun createAttachmentMenu() {
        attachmentMenu = LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
            orientation = LinearLayout.HORIZONTAL
            setBackgroundColor(if (isDarkTheme) Color.parseColor("#212121") else Color.WHITE)
            setPadding(dp(16), dp(16), dp(16), dp(16))
            gravity = Gravity.CENTER
        }
        
        val attachmentOptions = listOf(
            Triple("Фото", android.R.drawable.ic_menu_gallery, Color.parseColor("#9C27B0")),
            Triple("Видео", android.R.drawable.ic_menu_slideshow, Color.parseColor("#F44336")),
            Triple("Файл", android.R.drawable.ic_menu_agenda, Color.parseColor("#3F51B5")),
            Triple("Место", android.R.drawable.ic_menu_mylocation, Color.parseColor("#4CAF50")),
            Triple("Контакт", android.R.drawable.ic_menu_myplaces, Color.parseColor("#FF9800")),
            Triple("Опрос", android.R.drawable.ic_menu_more, Color.parseColor("#00BCD4"))
        )
        
        attachmentOptions.forEachIndexed { index, (label, icon, color) ->
            val button = createAttachmentButton(label, icon, color) {
                when (index) {
                    0 -> pickImage()
                    1 -> pickVideo()
                    2 -> pickFile()
                    3 -> shareLocation()
                    4 -> shareContact()
                    5 -> createPoll()
                }
                hideAttachmentMenu()
            }
            attachmentMenu.addView(button)
        }
    }
    
    private fun createAttachmentButton(label: String, iconRes: Int, bgColor: Int, onClick: () -> Unit): LinearLayout {
        return LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f)
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setPadding(dp(8), dp(8), dp(8), dp(8))
            setOnClickListener { onClick() }
            
            val icon = ImageView(context).apply {
                layoutParams = LinearLayout.LayoutParams(dp(48), dp(48))
                setImageResource(iconRes)
                setColorFilter(Color.WHITE)
                scaleType = ImageView.ScaleType.CENTER
                background = createCircleDrawable(bgColor)
            }
            
            val text = TextView(context).apply {
                layoutParams = LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT
                ).apply {
                    topMargin = dp(8)
                }
                this.text = label
                setTextColor(if (isDarkTheme) Color.WHITE else Color.BLACK)
                textSize = 12f
            }
            
            addView(icon)
            addView(text)
        }
    }
    
    private fun createVoiceRecordingUI() {
        voiceRecordingContainer = LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                dp(56)
            )
            orientation = LinearLayout.HORIZONTAL
            setBackgroundColor(if (isDarkTheme) Color.parseColor("#212121") else Color.WHITE)
            gravity = Gravity.CENTER_VERTICAL
            setPadding(dp(16), 0, dp(16), 0)
        }
        
        voiceRecordingCancel = TextView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
            text = "< Отмена"
            setTextColor(Color.RED)
            textSize = 14f
            setOnClickListener { stopVoiceRecording(cancel = true) }
        }
        
        voiceRecordingTimer = TextView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                marginStart = dp(16)
            }
            text = "00:00"
            setTextColor(Color.RED)
            textSize = 16f
            typeface = Typeface.MONOSPACE
        }
        
        voiceRecordingWaveform = VoiceWaveformView(this).apply {
            layoutParams = LinearLayout.LayoutParams(0, dp(32), 1f).apply {
                marginStart = dp(16)
                marginEnd = dp(16)
            }
        }
        
        voicePauseButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(40), dp(40))
            setImageResource(android.R.drawable.ic_media_pause)
            setColorFilter(Color.GRAY)
            scaleType = ImageView.ScaleType.CENTER
            setBackgroundResource(selectableItemBackgroundBorderless())
            setOnClickListener { toggleVoicePause() }
        }
        
        voiceSendButton = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(48), dp(48))
            setImageResource(android.R.drawable.ic_menu_send)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER
            background = createCircleDrawable(COLOR_ACCENT)
            setOnClickListener { stopVoiceRecording(cancel = false) }
        }
        
        voiceRecordingContainer.addView(voiceRecordingCancel)
        voiceRecordingContainer.addView(voiceRecordingTimer)
        voiceRecordingContainer.addView(voiceRecordingWaveform)
        voiceRecordingContainer.addView(voicePauseButton)
        voiceRecordingContainer.addView(voiceSendButton)
    }
    
    private fun createScrollToBottomButton() {
        scrollToBottomButton = ImageView(this).apply {
            layoutParams = FrameLayout.LayoutParams(dp(48), dp(48)).apply {
                gravity = Gravity.BOTTOM or Gravity.END
                marginEnd = dp(16)
                bottomMargin = dp(80)
            }
            setImageResource(android.R.drawable.arrow_down_float)
            setColorFilter(Color.WHITE)
            scaleType = ImageView.ScaleType.CENTER
            background = createCircleDrawable(COLOR_ACCENT)
            elevation = dp(4).toFloat()
            setOnClickListener { scrollToBottom(true) }
        }
        
        unreadBadge = TextView(this).apply {
            // This would be positioned on top of scrollToBottomButton
        }
    }
    
    private fun createLoadingOverlay() {
        loadingOverlay = FrameLayout(this).apply {
            layoutParams = FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT
            )
            setBackgroundColor(Color.parseColor("#80000000"))
            isClickable = true
        }
        
        progressBar = ProgressBar(this).apply {
            layoutParams = FrameLayout.LayoutParams(
                dp(48),
                dp(48)
            ).apply {
                gravity = Gravity.CENTER
            }
        }
        
        loadingOverlay.addView(progressBar)
    }

    // ==================== SETUP METHODS ====================
    
    private fun setupToolbar() {
        // Already set up in createToolbar()
    }
    
    private fun setupRecyclerView() {
        layoutManager = LinearLayoutManager(this).apply {
            stackFromEnd = true
            reverseLayout = false
        }
        
        messageAdapter = MessageAdapter()
        
        recyclerView.apply {
            this.layoutManager = this@MainActivity.layoutManager
            adapter = messageAdapter
            itemAnimator = DefaultItemAnimator().apply {
                addDuration = ANIM_DURATION_MEDIUM
                removeDuration = ANIM_DURATION_MEDIUM
                changeDuration = ANIM_DURATION_MEDIUM
            }
            
            addOnScrollListener(object : RecyclerView.OnScrollListener() {
                override fun onScrolled(recyclerView: RecyclerView, dx: Int, dy: Int) {
                    super.onScrolled(recyclerView, dx, dy)
                    handleScroll(dy)
                }
                
                override fun onScrollStateChanged(recyclerView: RecyclerView, newState: Int) {
                    super.onScrollStateChanged(recyclerView, newState)
                    if (newState == RecyclerView.SCROLL_STATE_IDLE) {
                        checkLoadMore()
                    }
                }
            })
            
            // Swipe gesture for reply
            val swipeCallback = object : ItemTouchHelper.SimpleCallback(0, ItemTouchHelper.RIGHT) {
                override fun onMove(rv: RecyclerView, vh: RecyclerView.ViewHolder, target: RecyclerView.ViewHolder) = false
                
                override fun onSwiped(viewHolder: RecyclerView.ViewHolder, direction: Int) {
                    val position = viewHolder.adapterPosition
                    val item = filteredMessages.getOrNull(position)
                    if (item is Message) {
                        replyTo(item)
                    }
                    messageAdapter.notifyItemChanged(position)
                }
                
                override fun getSwipeThreshold(viewHolder: RecyclerView.ViewHolder) = 0.3f
                
                override fun onChildDraw(
                    c: Canvas, recyclerView: RecyclerView, viewHolder: RecyclerView.ViewHolder,
                    dX: Float, dY: Float, actionState: Int, isCurrentlyActive: Boolean
                ) {
                    // Draw reply icon while swiping
                    if (dX > 0) {
                        val itemView = viewHolder.itemView
                        val replyIcon = ContextCompat.getDrawable(this@MainActivity, android.R.drawable.ic_menu_revert)
                        val iconMargin = dp(16)
                        val iconTop = itemView.top + (itemView.height - dp(24)) / 2
                        val iconBottom = iconTop + dp(24)
                        val iconLeft = itemView.left + iconMargin
                        val iconRight = iconLeft + dp(24)
                        
                        replyIcon?.setBounds(iconLeft, iconTop, iconRight, iconBottom)
                        replyIcon?.setTint(COLOR_ACCENT)
                        replyIcon?.draw(c)
                    }
                    
                    super.onChildDraw(c, recyclerView, viewHolder, dX / 3, dY, actionState, isCurrentlyActive)
                }
            }
            
            ItemTouchHelper(swipeCallback).attachToRecyclerView(this)
        }
    }
    
    private fun setupInputArea() {
        // Already set up in createInputArea()
    }
    
    private fun setupEmojiKeyboard() {
        loadRecentEmojis()
        updateRecentEmojisRow()
    }
    
    private fun setupAttachmentMenu() {
        // Already set up in createAttachmentMenu()
    }
    
    private fun setupSearch() {
        // Already set up in createSearchBar()
    }
    
    private fun setupSelectionToolbar() {
        // Already set up in createSelectionToolbar()
    }
    
    private fun setupPinnedMessages() {
        // Already set up in createPinnedMessageBar()
    }
    
    private fun setupVoiceRecording() {
        // Already set up in createVoiceRecordingUI()
    }
    
    private fun setupGestures() {
        // Gesture detection for various interactions
    }

    // ==================== MESSAGE ADAPTER ====================
    
    inner class MessageAdapter : RecyclerView.Adapter<RecyclerView.ViewHolder>() {
        
        override fun getItemCount(): Int = filteredMessages.size
        
        override fun getItemViewType(position: Int): Int {
            return when (val item = filteredMessages[position]) {
                is String -> VIEW_TYPE_DATE_HEADER
                is Message -> {
                    when {
                        item.type == MSG_TYPE_SYSTEM -> VIEW_TYPE_SYSTEM
                        item.isOutgoing(currentUserId) -> VIEW_TYPE_OUTGOING
                        else -> VIEW_TYPE_INCOMING
                    }
                }
                else -> VIEW_TYPE_OUTGOING
            }
        }
        
        override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): RecyclerView.ViewHolder {
            return when (viewType) {
                VIEW_TYPE_DATE_HEADER -> DateHeaderViewHolder(createDateHeaderView())
                VIEW_TYPE_SYSTEM -> SystemMessageViewHolder(createSystemMessageView())
                VIEW_TYPE_INCOMING -> MessageViewHolder(createMessageBubbleView(false))
                else -> MessageViewHolder(createMessageBubbleView(true))
            }
        }
        
        override fun onBindViewHolder(holder: RecyclerView.ViewHolder, position: Int) {
            when (holder) {
                is DateHeaderViewHolder -> {
                    val dateStr = filteredMessages[position] as String
                    holder.bind(dateStr)
                }
                is SystemMessageViewHolder -> {
                    val message = filteredMessages[position] as Message
                    holder.bind(message)
                }
                is MessageViewHolder -> {
                    val message = filteredMessages[position] as Message
                    holder.bind(message, position)
                }
            }
        }
        
        private fun createDateHeaderView(): View {
            return TextView(this@MainActivity).apply {
                layoutParams = RecyclerView.LayoutParams(
                    RecyclerView.LayoutParams.MATCH_PARENT,
                    RecyclerView.LayoutParams.WRAP_CONTENT
                ).apply {
                    topMargin = dp(8)
                    bottomMargin = dp(8)
                }
                gravity = Gravity.CENTER
                setPadding(dp(12), dp(6), dp(12), dp(6))
                textSize = 13f
                setTextColor(Color.WHITE)
                background = createRoundedDrawable(Color.parseColor("#80000000"), dp(12).toFloat())
            }
        }
        
        private fun createSystemMessageView(): View {
            return TextView(this@MainActivity).apply {
                layoutParams = RecyclerView.LayoutParams(
                    RecyclerView.LayoutParams.MATCH_PARENT,
                    RecyclerView.LayoutParams.WRAP_CONTENT
                ).apply {
                    topMargin = dp(8)
                    bottomMargin = dp(8)
                    marginStart = dp(32)
                    marginEnd = dp(32)
                }
                gravity = Gravity.CENTER
                setPadding(dp(16), dp(8), dp(16), dp(8))
                textSize = 13f
                setTextColor(if (isDarkTheme) Color.LTGRAY else Color.DKGRAY)
                background = createRoundedDrawable(
                    if (isDarkTheme) Color.parseColor("#2D2D2D") else Color.parseColor("#E0E0E0"),
                    dp(16).toFloat()
                )
            }
        }
        
        private fun createMessageBubbleView(isOutgoing: Boolean): View {
            val container = FrameLayout(this@MainActivity).apply {
                layoutParams = RecyclerView.LayoutParams(
                    RecyclerView.LayoutParams.MATCH_PARENT,
                    RecyclerView.LayoutParams.WRAP_CONTENT
                ).apply {
                    topMargin = dp(2)
                    bottomMargin = dp(2)
                }
            }
            
            val bubbleContainer = LinearLayout(this@MainActivity).apply {
                layoutParams = FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.WRAP_CONTENT,
                    FrameLayout.LayoutParams.WRAP_CONTENT
                ).apply {
                    gravity = if (isOutgoing) Gravity.END else Gravity.START
                    marginStart = if (isOutgoing) dp(48) else dp(8)
                    marginEnd = if (isOutgoing) dp(8) else dp(48)
                }
                orientation = LinearLayout.VERTICAL
            }
            
            // Reply preview (if replying)
            val replyPreview = LinearLayout(this@MainActivity).apply {
                orientation = LinearLayout.HORIZONTAL
                visibility = View.GONE
                setPadding(dp(8), dp(6), dp(8), dp(6))
                background = createRoundedDrawable(
                    Color.parseColor("#20000000"),
                    dp(8).toFloat()
                )
            }
            
            val replyBar = View(this@MainActivity).apply {
                layoutParams = LinearLayout.LayoutParams(dp(3), LinearLayout.LayoutParams.MATCH_PARENT)
                setBackgroundColor(COLOR_ACCENT)
            }
            
            val replyContent = LinearLayout(this@MainActivity).apply {
                layoutParams = LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT
                ).apply {
                    marginStart = dp(8)
                }
                orientation = LinearLayout.VERTICAL
            }
            
            val replySenderText = TextView(this@MainActivity).apply {
                setTextColor(COLOR_ACCENT)
                textSize = 12f
                typeface = Typeface.DEFAULT_BOLD
            }
            
            val replyMessageText = TextView(this@MainActivity).apply {
                setTextColor(if (isDarkTheme) Color.LTGRAY else Color.DKGRAY)
                textSize = 12f
                maxLines = 1
                ellipsize = TextUtils.TruncateAt.END
            }
            
            replyContent.addView(replySenderText)
            replyContent.addView(replyMessageText)
            replyPreview.addView(replyBar)
            replyPreview.addView(replyContent)
            
            // Forwarded label
            val forwardedLabel = TextView(this@MainActivity).apply {
                visibility = View.GONE
                text = "Переслано"
                setTextColor(COLOR_ACCENT)
                textSize = 12f
                setPadding(dp(8), dp(4), dp(8), 0)
            }
            
            // Bubble
            val bubble = LinearLayout(this@MainActivity).apply {
                orientation = LinearLayout.VERTICAL
                background = createBubbleDrawable(isOutgoing)
                setPadding(dp(10), dp(6), dp(10), dp(6))
            }
            
            // Message text
            val messageText = TextView(this@MainActivity).apply {
                setTextColor(if (isDarkTheme && !isOutgoing) Color.WHITE else Color.parseColor("#1A1A1A"))
                textSize = 15f
                setLineSpacing(dp(2).toFloat(), 1f)
                movementMethod = LinkMovementMethod.getInstance()
            }
            
            // Media container
            val mediaContainer = FrameLayout(this@MainActivity).apply {
                visibility = View.GONE
            }
            
            // Info row (time, status, edited)
            val infoRow = LinearLayout(this@MainActivity).apply {
                orientation = LinearLayout.HORIZONTAL
                gravity = Gravity.END or Gravity.CENTER_VERTICAL
                setPadding(0, dp(2), 0, 0)
            }
            
            val editedLabel = TextView(this@MainActivity).apply {
                text = "изм."
                setTextColor(Color.GRAY)
                textSize = 11f
                visibility = View.GONE
            }
            
            val timeText = TextView(this@MainActivity).apply {
                setTextColor(Color.GRAY)
                textSize = 11f
            }
            
            val statusIcon = ImageView(this@MainActivity).apply {
                layoutParams = LinearLayout.LayoutParams(dp(16), dp(16)).apply {
                    marginStart = dp(4)
                }
                visibility = if (isOutgoing) View.VISIBLE else View.GONE
            }
            
            infoRow.addView(editedLabel)
            infoRow.addView(timeText)
            infoRow.addView(statusIcon)
            
            // Reactions row
            val reactionsRow = LinearLayout(this@MainActivity).apply {
                orientation = LinearLayout.HORIZONTAL
                visibility = View.GONE
                setPadding(0, dp(4), 0, 0)
            }
            
            bubble.addView(replyPreview)
            bubble.addView(forwardedLabel)
            bubble.addView(mediaContainer)
            bubble.addView(messageText)
            bubble.addView(infoRow)
            bubble.addView(reactionsRow)
            
            bubbleContainer.addView(bubble)
            
            // Selection overlay
            val selectionOverlay = View(this@MainActivity).apply {
                layoutParams = FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.MATCH_PARENT,
                    FrameLayout.LayoutParams.MATCH_PARENT
                )
                setBackgroundColor(Color.parseColor("#302196F3"))
                visibility = View.GONE
            }
            
            container.addView(bubbleContainer)
            container.addView(selectionOverlay)
            
            // Store views as tags
            container.tag = mapOf(
                "bubble" to bubble,
                "messageText" to messageText,
                "timeText" to timeText,
                "statusIcon" to statusIcon,
                "editedLabel" to editedLabel,
                "mediaContainer" to mediaContainer,
                "reactionsRow" to reactionsRow,
                "replyPreview" to replyPreview,
                "replySenderText" to replySenderText,
                "replyMessageText" to replyMessageText,
                "forwardedLabel" to forwardedLabel,
                "selectionOverlay" to selectionOverlay,
                "bubbleContainer" to bubbleContainer
            )
            
            return container
        }
    }
    
    inner class DateHeaderViewHolder(private val view: View) : RecyclerView.ViewHolder(view) {
        fun bind(dateStr: String) {
            (view as TextView).text = dateStr
        }
    }
    
    inner class SystemMessageViewHolder(private val view: View) : RecyclerView.ViewHolder(view) {
        fun bind(message: Message) {
            (view as TextView).text = message.text
        }
    }
    
    inner class MessageViewHolder(private val view: View) : RecyclerView.ViewHolder(view) {
        @Suppress("UNCHECKED_CAST")
        fun bind(message: Message, position: Int) {
            val tags = view.tag as Map<String, View>
            
            val messageText = tags["messageText"] as TextView
            val timeText = tags["timeText"] as TextView
            val statusIcon = tags["statusIcon"] as ImageView
            val editedLabel = tags["editedLabel"] as TextView
            val mediaContainer = tags["mediaContainer"] as FrameLayout
            val reactionsRow = tags["reactionsRow"] as LinearLayout
            val replyPreview = tags["replyPreview"] as LinearLayout
            val replySenderText = tags["replySenderText"] as TextView
            val replyMessageText = tags["replyMessageText"] as TextView
            val forwardedLabel = tags["forwardedLabel"] as TextView
            val selectionOverlay = tags["selectionOverlay"] as View
            val bubble = tags["bubble"] as LinearLayout
            
            // Message text
            if (message.isDeleted) {
                messageText.text = "🚫 Сообщение удалено"
                messageText.setTypeface(null, Typeface.ITALIC)
                messageText.alpha = 0.6f
            } else {
                messageText.text = formatMessageText(message.text)
                messageText.setTypeface(null, Typeface.NORMAL)
                messageText.alpha = 1f
            }
            
            // Time
            timeText.text = message.getFormattedTime()
            
            // Edited label
            editedLabel.visibility = if (message.isEdited) View.VISIBLE else View.GONE
            
            // Status icon
            if (message.isOutgoing(currentUserId)) {
                statusIcon.visibility = View.VISIBLE
                when (message.status) {
                    STATUS_SENDING -> {
                        statusIcon.setImageResource(android.R.drawable.ic_menu_rotate)
                        statusIcon.setColorFilter(Color.GRAY)
                    }
                    STATUS_SENT -> {
                        statusIcon.setImageResource(android.R.drawable.checkbox_on_background)
                        statusIcon.setColorFilter(Color.GRAY)
                    }
                    STATUS_DELIVERED -> {
                        statusIcon.setImageResource(android.R.drawable.checkbox_on_background)
                        statusIcon.setColorFilter(COLOR_ACCENT)
                    }
                    STATUS_READ -> {
                        statusIcon.setImageResource(android.R.drawable.checkbox_on_background)
                        statusIcon.setColorFilter(COLOR_ONLINE)
                    }
                    STATUS_FAILED -> {
                        statusIcon.setImageResource(android.R.drawable.ic_delete)
                        statusIcon.setColorFilter(Color.RED)
                    }
                }
            } else {
                statusIcon.visibility = View.GONE
            }
            
            // Reply preview
            if (message.replyToId != null) {
                replyPreview.visibility = View.VISIBLE
                replySenderText.text = message.replyToSender ?: "Сообщение"
                replyMessageText.text = message.replyToText ?: ""
                replyPreview.setOnClickListener { scrollToMessage(message.replyToId!!) }
            } else {
                replyPreview.visibility = View.GONE
            }
            
            // Forwarded label
            forwardedLabel.visibility = if (message.isForwarded) View.VISIBLE else View.GONE
            if (message.isForwarded) {
                forwardedLabel.text = "Переслано от ${message.forwardedFrom ?: "пользователя"}"
            }
            
            // Media
            when (message.type) {
                MSG_TYPE_IMAGE, MSG_TYPE_VIDEO -> {
                    mediaContainer.visibility = View.VISIBLE
                    mediaContainer.removeAllViews()
                    val mediaView = createMediaView(message)
                    mediaContainer.addView(mediaView)
                }
                MSG_TYPE_VOICE -> {
                    mediaContainer.visibility = View.VISIBLE
                    mediaContainer.removeAllViews()
                    val voiceView = createVoiceMessageView(message)
                    mediaContainer.addView(voiceView)
                }
                MSG_TYPE_FILE -> {
                    mediaContainer.visibility = View.VISIBLE
                    mediaContainer.removeAllViews()
                    val fileView = createFileView(message)
                    mediaContainer.addView(fileView)
                }
                MSG_TYPE_LOCATION -> {
                    mediaContainer.visibility = View.VISIBLE
                    mediaContainer.removeAllViews()
                    val locationView = createLocationView(message)
                    mediaContainer.addView(locationView)
                }
                MSG_TYPE_POLL -> {
                    mediaContainer.visibility = View.VISIBLE
                    mediaContainer.removeAllViews()
                    val pollView = createPollView(message)
                    mediaContainer.addView(pollView)
                }
                else -> {
                    mediaContainer.visibility = View.GONE
                }
            }
            
            // Reactions
            if (message.reactions.isNotEmpty()) {
                reactionsRow.visibility = View.VISIBLE
                reactionsRow.removeAllViews()
                message.reactions.forEach { (emoji, users) ->
                    val reactionView = createReactionView(emoji, users.size, users.contains(currentUserId))
                    reactionView.setOnClickListener { toggleReaction(message, emoji) }
                    reactionsRow.addView(reactionView)
                }
            } else {
                reactionsRow.visibility = View.GONE
            }
            
            // Selection state
            selectionOverlay.visibility = if (selection.selectedIds.contains(message.id)) View.VISIBLE else View.GONE
            
            // Click listeners
            bubble.setOnClickListener {
                if (selection.isSelectionMode) {
                    selection.toggle(message.id)
                    updateSelectionUI()
                    notifyItemChanged(position)
                }
            }
            
            bubble.setOnLongClickListener {
                if (!selection.isSelectionMode) {
                    showMessageMenu(message, bubble)
                } else {
                    selection.toggle(message.id)
                    updateSelectionUI()
                    notifyItemChanged(position)
                }
                true
            }
            
            // Double tap for reaction
            bubble.setOnTouchListener(DoubleTapListener {
                showQuickReactions(message, bubble)
            })
        }
    }

    // ==================== MESSAGE OPERATIONS ====================
    
    private fun sendMessage() {
        val text = messageInput.text.toString().trim()
        if (text.isEmpty() && editingMessage == null) return
        
        if (editingMessage != null) {
            // Edit existing message
            val msg = editingMessage!!
            msg.text = text
            msg.isEdited = true
            msg.editedAt = System.currentTimeMillis()
            
            val index = messages.indexOfFirst { it.id == msg.id }
            if (index >= 0) {
                messages[index] = msg
                updateFilteredMessages()
                scrollToMessage(msg.id)
            }
            
            cancelEdit()
        } else {
            // Create new message
            val message = Message(
                text = text,
                type = MSG_TYPE_TEXT,
                senderId = currentUserId,
                senderName = currentUserName,
                replyToId = replyToMessage?.id,
                replyToText = replyToMessage?.text,
                replyToSender = replyToMessage?.senderName
            )
            
            addMessage(message)
            cancelReply()
            
            // Simulate sending
            simulateMessageSending(message)
        }
        
        messageInput.text.clear()
        hideEmojiKeyboard()
        hideAttachmentMenu()
    }
    
    private fun addMessage(message: Message) {
        messages.add(message)
        messageCache[message.id] = message
        updateFilteredMessages()
        scrollToBottom(true)
        
        // Add to offline queue if no network
        // offlineMessageQueue.add(message)
    }
    
    private fun updateFilteredMessages() {
        filteredMessages.clear()
        
        var lastDate: String? = null
        messages.sortedBy { it.timestamp }.forEach { message ->
            val date = message.getFormattedDate()
            if (date != lastDate) {
                filteredMessages.add(date)
                lastDate = date
            }
            filteredMessages.add(message)
        }
        
        messageAdapter.notifyDataSetChanged()
    }
    
    private fun simulateMessageSending(message: Message) {
        launch {
            delay(500)
            message.status = STATUS_SENT
            updateMessageInList(message)
            
            delay(1000)
            message.status = STATUS_DELIVERED
            updateMessageInList(message)
            
            delay(2000)
            message.status = STATUS_READ
            updateMessageInList(message)
        }
    }
    
    private fun updateMessageInList(message: Message) {
        val position = filteredMessages.indexOfFirst { it is Message && it.id == message.id }
        if (position >= 0) {
            messageAdapter.notifyItemChanged(position)
        }
    }
    
    private fun replyTo(message: Message) {
        replyToMessage = message
        replyContainer.visibility = View.VISIBLE
        replySender.text = message.senderName
        replyText.text = message.text
        
        messageInput.requestFocus()
        showKeyboard()
        
        animateViewIn(replyContainer)
    }
    
    private fun cancelReply() {
        replyToMessage = null
        replyContainer.visibility = View.GONE
    }
    
    private fun editMessage(message: Message) {
        editingMessage = message
        editContainer.visibility = View.VISIBLE
        editText.text = message.text
        messageInput.setText(message.text)
        messageInput.setSelection(message.text.length)
        messageInput.requestFocus()
        showKeyboard()
        
        animateViewIn(editContainer)
    }
    
    private fun cancelEdit() {
        editingMessage = null
        editContainer.visibility = View.GONE
        messageInput.text.clear()
    }
    
    private fun deleteMessage(message: Message, forEveryone: Boolean = false) {
        if (forEveryone) {
            message.isDeleted = true
            message.text = "Сообщение удалено"
            updateMessageInList(message)
        } else {
            messages.removeAll { it.id == message.id }
            messageCache.remove(message.id)
            updateFilteredMessages()
        }
    }
    
    private fun forwardMessage(message: Message) {
        // Show chat selection dialog
        Toast.makeText(this, "Выберите чат для пересылки", Toast.LENGTH_SHORT).show()
    }
    
    private fun copyMessage(message: Message) {
        val clipboard = getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
        val clip = ClipData.newPlainText("message", message.text)
        clipboard.setPrimaryClip(clip)
        Toast.makeText(this, "Скопировано", Toast.LENGTH_SHORT).show()
    }
    
    private fun pinMessage(message: Message) {
        message.isPinned = !message.isPinned
        if (message.isPinned) {
            currentChat?.pinnedMessages?.add(message.id)
            updatePinnedMessageBar()
        } else {
            currentChat?.pinnedMessages?.remove(message.id)
            updatePinnedMessageBar()
        }
        updateMessageInList(message)
    }
    
    private fun toggleReaction(message: Message, emoji: String) {
        val users = message.reactions.getOrPut(emoji) { mutableListOf() }
        if (users.contains(currentUserId)) {
            users.remove(currentUserId)
            if (users.isEmpty()) {
                message.reactions.remove(emoji)
            }
        } else {
            users.add(currentUserId)
        }
        updateMessageInList(message)
    }
    
    private fun scrollToMessage(messageId: String) {
        val position = filteredMessages.indexOfFirst { it is Message && it.id == messageId }
        if (position >= 0) {
            recyclerView.smoothScrollToPosition(position)
            // Highlight the message briefly
            launch {
                delay(300)
                messageAdapter.notifyItemChanged(position)
            }
        }
    }
    
    private fun scrollToBottom(animated: Boolean) {
        if (filteredMessages.isNotEmpty()) {
            if (animated) {
                recyclerView.smoothScrollToPosition(filteredMessages.size - 1)
            } else {
                recyclerView.scrollToPosition(filteredMessages.size - 1)
            }
        }
    }
    
    private fun markMessagesAsRead() {
        messages.filter { !it.isOutgoing(currentUserId) && it.status != STATUS_READ }.forEach {
            it.status = STATUS_READ
            it.readBy.add(currentUserId)
        }
    }

    // ==================== VOICE RECORDING ====================
    
    @SuppressLint("ClickableViewAccessibility")
    private fun handleVoiceButtonTouch(event: MotionEvent): Boolean {
        when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                if (checkAudioPermission()) {
                    startVoiceRecording()
                }
                return true
            }
            MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                if (voiceRecordingState.isRecording) {
                    val duration = voiceRecordingState.getDuration()
                    if (duration < 1000) {
                        stopVoiceRecording(cancel = true)
                        Toast.makeText(this, "Слишком короткое сообщение", Toast.LENGTH_SHORT).show()
                    } else {
                        stopVoiceRecording(cancel = false)
                    }
                }
                return true
            }
            MotionEvent.ACTION_MOVE -> {
                // Check if swiped left to cancel
                if (event.x < -dp(100)) {
                    stopVoiceRecording(cancel = true)
                }
                return true
            }
        }
        return false
    }
    
    private fun startVoiceRecording() {
        val fileName = "voice_${System.currentTimeMillis()}.m4a"
        val filePath = "${externalCacheDir?.absolutePath}/$fileName"
        
        try {
            mediaRecorder = MediaRecorder().apply {
                setAudioSource(MediaRecorder.AudioSource.MIC)
                setOutputFormat(MediaRecorder.OutputFormat.MPEG_4)
                setAudioEncoder(MediaRecorder.AudioEncoder.AAC)
                setAudioSamplingRate(44100)
                setAudioEncodingBitRate(96000)
                setOutputFile(filePath)
                prepare()
                start()
            }
            
            voiceRecordingState = VoiceRecordingState(
                isRecording = true,
                startTime = System.currentTimeMillis(),
                filePath = filePath
            )
            
            inputContainer.visibility = View.GONE
            voiceRecordingContainer.visibility = View.VISIBLE
            
            startRecordingTimer()
            startAmplitudeCapture()
            
            // Haptic feedback
            vibrateShort()
            
        } catch (e: Exception) {
            Log.e(TAG, "Failed to start recording", e)
            Toast.makeText(this, "Не удалось начать запись", Toast.LENGTH_SHORT).show()
        }
    }
    
    private fun stopVoiceRecording(cancel: Boolean) {
        try {
            mediaRecorder?.apply {
                stop()
                release()
            }
        } catch (e: Exception) {
            Log.e(TAG, "Error stopping recording", e)
        }
        mediaRecorder = null
        
        voiceRecordingContainer.visibility = View.GONE
        inputContainer.visibility = View.VISIBLE
        
        if (!cancel && voiceRecordingState.filePath != null) {
            sendVoiceMessage(
                voiceRecordingState.filePath!!,
                voiceRecordingState.getDuration(),
                voiceRecordingState.amplitudes
            )
        }
        
        voiceRecordingState = VoiceRecordingState()
    }
    
    private fun toggleVoicePause() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            if (voiceRecordingState.isPaused) {
                mediaRecorder?.resume()
                voiceRecordingState.isPaused = false
                voicePauseButton.setImageResource(android.R.drawable.ic_media_pause)
            } else {
                mediaRecorder?.pause()
                voiceRecordingState.isPaused = true
                voicePauseButton.setImageResource(android.R.drawable.ic_media_play)
            }
        }
    }
    
    private fun startRecordingTimer() {
        launch {
            while (voiceRecordingState.isRecording) {
                voiceRecordingTimer.text = voiceRecordingState.getFormattedDuration()
                delay(100)
            }
        }
    }
    
    private fun startAmplitudeCapture() {
        launch {
            while (voiceRecordingState.isRecording) {
                try {
                    val amplitude = mediaRecorder?.maxAmplitude ?: 0
                    voiceRecordingState.amplitudes.add(amplitude)
                    voiceRecordingWaveform.addAmplitude(amplitude)
                } catch (e: Exception) {
                    // Ignore
                }
                delay(VOICE_AMPLITUDE_UPDATE_INTERVAL)
            }
        }
    }
    
    private fun sendVoiceMessage(filePath: String, duration: Long, amplitudes: List<Int>) {
        val message = Message(
            type = MSG_TYPE_VOICE,
            senderId = currentUserId,
            senderName = currentUserName,
            mediaUrl = filePath,
            mediaDuration = duration,
            voiceWaveform = amplitudes.takeLast(50) // Normalize to 50 points
        )
        
        addMessage(message)
        simulateMessageSending(message)
    }

    // ==================== SEARCH ====================
    
    private fun showSearch() {
        isSearchVisible = true
        searchContainer.visibility = View.VISIBLE
        toolbar.visibility = View.GONE
        searchInput.requestFocus()
        showKeyboard()
    }
    
    private fun hideSearch() {
        isSearchVisible = false
        searchContainer.visibility = View.GONE
        toolbar.visibility = View.VISIBLE
        searchInput.text.clear()
        searchResults = emptyList()
        currentSearchIndex = 0
        hideKeyboard()
    }
    
    private fun performSearch(query: String) {
        if (query.length < 2) {
            searchResults = emptyList()
            searchResultsCount.text = "0/0"
            return
        }
        
        searchResults = messages
            .filter { it.text.contains(query, ignoreCase = true) }
            .mapNotNull { msg -> filteredMessages.indexOfFirst { it is Message && it.id == msg.id } }
            .filter { it >= 0 }
        
        currentSearchIndex = if (searchResults.isNotEmpty()) searchResults.size - 1 else 0
        updateSearchResultsUI()
        
        if (searchResults.isNotEmpty()) {
            recyclerView.scrollToPosition(searchResults[currentSearchIndex])
        }
    }
    
    private fun navigateSearchResult(direction: Int) {
        if (searchResults.isEmpty()) return
        
        currentSearchIndex = (currentSearchIndex + direction).coerceIn(0, searchResults.size - 1)
        updateSearchResultsUI()
        recyclerView.smoothScrollToPosition(searchResults[currentSearchIndex])
    }
    
    private fun updateSearchResultsUI() {
        if (searchResults.isEmpty()) {
            searchResultsCount.text = "0/0"
        } else {
            searchResultsCount.text = "${currentSearchIndex + 1}/${searchResults.size}"
        }
    }

    // ==================== UI HELPERS ====================
    
    private fun showMessageMenu(message: Message, anchor: View) {
        val popup = PopupMenu(this, anchor)
        popup.menu.apply {
            add(0, 1, 0, "Ответить")
            add(0, 2, 1, "Копировать")
            if (message.isOutgoing(currentUserId) && !message.isDeleted) {
                add(0, 3, 2, "Редактировать")
            }
            add(0, 4, 3, "Переслать")
            add(0, 5, 4, if (message.isPinned) "Открепить" else "Закрепить")
            add(0, 6, 5, "Удалить")
            add(0, 7, 6, "Выбрать")
        }
        
        popup.setOnMenuItemClickListener { item ->
            when (item.itemId) {
                1 -> replyTo(message)
                2 -> copyMessage(message)
                3 -> editMessage(message)
                4 -> forwardMessage(message)
                5 -> pinMessage(message)
                6 -> showDeleteDialog(message)
                7 -> {
                    selection.toggle(message.id)
                    updateSelectionUI()
                }
            }
            true
        }
        
        popup.show()
    }
    
    private fun showQuickReactions(message: Message, anchor: View) {
        val popup = PopupWindow(this)
        val container = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            setBackgroundColor(if (isDarkTheme) Color.parseColor("#333333") else Color.WHITE)
            elevation = dp(8).toFloat()
            setPadding(dp(8), dp(8), dp(8), dp(8))
        }
        
        REACTIONS.forEach { emoji ->
            val emojiView = TextView(this).apply {
                text = emoji
                textSize = 24f
                setPadding(dp(8), dp(4), dp(8), dp(4))
                setOnClickListener {
                    toggleReaction(message, emoji)
                    popup.dismiss()
                }
            }
            container.addView(emojiView)
        }
        
        popup.contentView = container
        popup.isOutsideTouchable = true
        popup.isFocusable = true
        popup.showAsDropDown(anchor, 0, -dp(60))
    }
    
    private fun showDeleteDialog(message: Message) {
        AlertDialog.Builder(this)
            .setTitle("Удалить сообщение?")
            .setItems(arrayOf("Удалить у меня", "Удалить у всех")) { _, which ->
                deleteMessage(message, forEveryone = which == 1)
            }
            .setNegativeButton("Отмена", null)
            .show()
    }
    
    private fun updateInputButtons() {
        val hasText = messageInput.text.isNotEmpty()
        sendButton.visibility = if (hasText) View.VISIBLE else View.GONE
        voiceButton.visibility = if (hasText) View.GONE else View.VISIBLE
        
        // Animate transition
        if (hasText) {
            animateViewIn(sendButton)
        }
    }
    
    private fun updateSelectionUI() {
        if (selection.isSelectionMode) {
            toolbar.visibility = View.GONE
            selectionToolbar.visibility = View.VISIBLE
            selectionCount.text = selection.selectedIds.size.toString()
        } else {
            toolbar.visibility = View.VISIBLE
            selectionToolbar.visibility = View.GONE
        }
        messageAdapter.notifyDataSetChanged()
    }
    
    private fun updatePinnedMessageBar() {
        val pinnedIds = currentChat?.pinnedMessages ?: emptyList()
        if (pinnedIds.isEmpty()) {
            pinnedMessageBar.visibility = View.GONE
            return
        }
        
        pinnedMessageBar.visibility = View.VISIBLE
        val pinnedMessage = messages.find { it.id == pinnedIds[currentPinnedIndex % pinnedIds.size] }
        pinnedMessageText.text = pinnedMessage?.text ?: ""
        pinnedMessageCount.text = if (pinnedIds.size > 1) {
            "Закрепленное сообщение ${currentPinnedIndex + 1} из ${pinnedIds.size}"
        } else {
            "Закрепленное сообщение"
        }
    }
    
    private fun scrollToPinnedMessage() {
        val pinnedIds = currentChat?.pinnedMessages ?: return
        if (pinnedIds.isEmpty()) return
        
        currentPinnedIndex = (currentPinnedIndex + 1) % pinnedIds.size
        updatePinnedMessageBar()
        scrollToMessage(pinnedIds[currentPinnedIndex])
    }
    
    private fun hidePinnedMessageBar() {
        pinnedMessageBar.visibility = View.GONE
    }
    
    private fun toggleEmojiKeyboard() {
        if (isEmojiKeyboardVisible) {
            hideEmojiKeyboard()
        } else {
            showEmojiKeyboard()
        }
    }
    
    private fun showEmojiKeyboard() {
        hideKeyboard()
        emojiContainer.visibility = View.VISIBLE
        isEmojiKeyboardVisible = true
        emojiButton.setColorFilter(COLOR_ACCENT)
    }
    
    private fun hideEmojiKeyboard() {
        emojiContainer.visibility = View.GONE
        isEmojiKeyboardVisible = false
        emojiButton.setColorFilter(Color.GRAY)
    }
    
    private fun toggleAttachmentMenu() {
        if (isAttachmentMenuVisible) {
            hideAttachmentMenu()
        } else {
            showAttachmentMenu()
        }
    }
    
    private fun showAttachmentMenu() {
        attachmentMenu.visibility = View.VISIBLE
        isAttachmentMenuVisible = true
        animateViewIn(attachmentMenu)
    }
    
    private fun hideAttachmentMenu() {
        attachmentMenu.visibility = View.GONE
        isAttachmentMenuVisible = false
    }
    
    private fun sendTypingIndicator() {
        val now = System.currentTimeMillis()
        if (now - lastTypingTime > 2000) {
            lastTypingTime = now
            // Send typing indicator to server
            // simulatePeerTyping()
        }
        
        typingJob?.cancel()
        typingJob = launch {
            delay(TYPING_TIMEOUT)
            // Stop typing indicator
        }
    }
    
    private fun handleScroll(dy: Int) {
        // Show/hide scroll to bottom button
        val lastVisible = layoutManager.findLastVisibleItemPosition()
        val totalItems = filteredMessages.size
        
        if (totalItems - lastVisible > SCROLL_THRESHOLD) {
            if (scrollToBottomButton.visibility != View.VISIBLE) {
                scrollToBottomButton.visibility = View.VISIBLE
                animateViewIn(scrollToBottomButton)
            }
        } else {
            scrollToBottomButton.visibility = View.GONE
        }
    }
    
    private fun checkLoadMore() {
        if (isLoading || !hasMoreMessages) return
        
        val firstVisible = layoutManager.findFirstVisibleItemPosition()
        if (firstVisible < 5) {
            loadMoreMessages()
        }
    }
    
    private fun loadMoreMessages() {
        isLoading = true
        // Simulate loading more messages
        launch {
            delay(1000)
            // Add older messages
            isLoading = false
        }
    }

    // ==================== VIEW FACTORIES ====================
    
    private fun createMediaView(message: Message): View {
        return FrameLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(200), dp(200))
            background = createRoundedDrawable(Color.parseColor("#E0E0E0"), dp(8).toFloat())
            
            val imageView = ImageView(context).apply {
                layoutParams = FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.MATCH_PARENT,
                    FrameLayout.LayoutParams.MATCH_PARENT
                )
                scaleType = ImageView.ScaleType.CENTER_CROP
                setImageResource(android.R.drawable.ic_menu_gallery)
                setColorFilter(Color.GRAY)
            }
            addView(imageView)
            
            if (message.type == MSG_TYPE_VIDEO) {
                val playIcon = ImageView(context).apply {
                    layoutParams = FrameLayout.LayoutParams(dp(48), dp(48)).apply {
                        gravity = Gravity.CENTER
                    }
                    setImageResource(android.R.drawable.ic_media_play)
                    setColorFilter(Color.WHITE)
                    background = createCircleDrawable(Color.parseColor("#80000000"))
                }
                addView(playIcon)
                
                val durationLabel = TextView(context).apply {
                    layoutParams = FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.WRAP_CONTENT,
                        FrameLayout.LayoutParams.WRAP_CONTENT
                    ).apply {
                        gravity = Gravity.BOTTOM or Gravity.END
                        marginEnd = dp(8)
                        bottomMargin = dp(8)
                    }
                    val minutes = (message.mediaDuration / 1000) / 60
                    val seconds = (message.mediaDuration / 1000) % 60
                    text = String.format("%d:%02d", minutes, seconds)
                    setTextColor(Color.WHITE)
                    textSize = 12f
                    setPadding(dp(6), dp(2), dp(6), dp(2))
                    background = createRoundedDrawable(Color.parseColor("#80000000"), dp(4).toFloat())
                }
                addView(durationLabel)
            }
        }
    }
    
    private fun createVoiceMessageView(message: Message): View {
        return LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(220), LinearLayout.LayoutParams.WRAP_CONTENT)
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            
            val playButton = ImageView(context).apply {
                layoutParams = LinearLayout.LayoutParams(dp(40), dp(40))
                setImageResource(android.R.drawable.ic_media_play)
                setColorFilter(COLOR_ACCENT)
                background = createCircleDrawable(Color.parseColor("#E3F2FD"))
                setOnClickListener { playVoiceMessage(message) }
            }
            
            val waveformContainer = LinearLayout(context).apply {
                layoutParams = LinearLayout.LayoutParams(0, dp(32), 1f).apply {
                    marginStart = dp(8)
                }
                orientation = LinearLayout.HORIZONTAL
                gravity = Gravity.CENTER_VERTICAL
                
                // Simplified waveform visualization
                message.voiceWaveform?.forEach { amp ->
                    val bar = View(context).apply {
                        val height = (amp.toFloat() / 32767 * dp(24)).toInt().coerceIn(dp(4), dp(24))
                        layoutParams = LinearLayout.LayoutParams(dp(3), height).apply {
                            marginEnd = dp(1)
                        }
                        setBackgroundColor(COLOR_ACCENT)
                    }
                    addView(bar)
                }
            }
            
            val durationText = TextView(context).apply {
                layoutParams = LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT
                ).apply {
                    marginStart = dp(8)
                }
                val minutes = (message.mediaDuration / 1000) / 60
                val seconds = (message.mediaDuration / 1000) % 60
                text = String.format("%d:%02d", minutes, seconds)
                setTextColor(Color.GRAY)
                textSize = 12f
            }
            
            addView(playButton)
            addView(waveformContainer)
            addView(durationText)
        }
    }
    
    private fun createFileView(message: Message): View {
        return LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            setPadding(dp(8), dp(8), dp(8), dp(8))
            background = createRoundedDrawable(Color.parseColor("#F0F0F0"), dp(8).toFloat())
            
            val fileIcon = ImageView(context).apply {
                layoutParams = LinearLayout.LayoutParams(dp(40), dp(40))
                setImageResource(android.R.drawable.ic_menu_agenda)
                setColorFilter(COLOR_ACCENT)
            }
            
            val textContainer = LinearLayout(context).apply {
                layoutParams = LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT
                ).apply {
                    marginStart = dp(12)
                }
                orientation = LinearLayout.VERTICAL
            }
            
            val fileName = TextView(context).apply {
                text = message.fileName ?: "Файл"
                setTextColor(Color.BLACK)
                textSize = 14f
                maxWidth = dp(180)
                maxLines = 1
                ellipsize = TextUtils.TruncateAt.MIDDLE
            }
            
            val fileSize = TextView(context).apply {
                text = formatFileSize(message.mediaSize)
                setTextColor(Color.GRAY)
                textSize = 12f
            }
            
            textContainer.addView(fileName)
            textContainer.addView(fileSize)
            
            addView(fileIcon)
            addView(textContainer)
            
            setOnClickListener { openFile(message) }
        }
    }
    
    private fun createLocationView(message: Message): View {
        return FrameLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(200), dp(120))
            background = createRoundedDrawable(Color.parseColor("#E0E0E0"), dp(8).toFloat())
            
            val mapPlaceholder = ImageView(context).apply {
                layoutParams = FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.MATCH_PARENT,
                    FrameLayout.LayoutParams.MATCH_PARENT
                )
                setImageResource(android.R.drawable.ic_menu_mapmode)
                setColorFilter(Color.GRAY)
                scaleType = ImageView.ScaleType.CENTER
            }
            
            val locationLabel = TextView(context).apply {
                layoutParams = FrameLayout.LayoutParams(
                    FrameLayout.LayoutParams.WRAP_CONTENT,
                    FrameLayout.LayoutParams.WRAP_CONTENT
                ).apply {
                    gravity = Gravity.BOTTOM or Gravity.CENTER_HORIZONTAL
                    bottomMargin = dp(8)
                }
                text = "📍 ${message.latitude?.format(4)}, ${message.longitude?.format(4)}"
                setTextColor(Color.WHITE)
                textSize = 11f
                setPadding(dp(6), dp(2), dp(6), dp(2))
                background = createRoundedDrawable(Color.parseColor("#80000000"), dp(4).toFloat())
            }
            
            addView(mapPlaceholder)
            addView(locationLabel)
            
            setOnClickListener { openLocation(message) }
        }
    }
    
    private fun createPollView(message: Message): View {
        return LinearLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(dp(250), LinearLayout.LayoutParams.WRAP_CONTENT)
            orientation = LinearLayout.VERTICAL
            setPadding(dp(4), dp(4), dp(4), dp(4))
            
            val question = TextView(context).apply {
                text = "📊 ${message.pollQuestion}"
                setTextColor(Color.BLACK)
                textSize = 15f
                typeface = Typeface.DEFAULT_BOLD
            }
            addView(question)
            
            val totalVoters = message.pollOptions?.sumOf { it.votes.size } ?: 0
            
            message.pollOptions?.forEach { option ->
                val optionView = LinearLayout(context).apply {
                    layoutParams = LinearLayout.LayoutParams(
                        LinearLayout.LayoutParams.MATCH_PARENT,
                        LinearLayout.LayoutParams.WRAP_CONTENT
                    ).apply {
                        topMargin = dp(8)
                    }
                    orientation = LinearLayout.VERTICAL
                    
                    val percentage = option.getPercentage(totalVoters)
                    val hasVoted = option.votes.contains(currentUserId)
                    
                    val optionContainer = LinearLayout(context).apply {
                        layoutParams = LinearLayout.LayoutParams(
                            LinearLayout.LayoutParams.MATCH_PARENT,
                            dp(36)
                        )
                        orientation = LinearLayout.HORIZONTAL
                        gravity = Gravity.CENTER_VERTICAL
                        background = createRoundedDrawable(
                            if (hasVoted) Color.parseColor("#E3F2FD") else Color.parseColor("#F5F5F5"),
                            dp(4).toFloat()
                        )
                        setPadding(dp(12), 0, dp(12), 0)
                        
                        val optionText = TextView(context).apply {
                            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f)
                            text = option.text
                            setTextColor(if (hasVoted) COLOR_ACCENT else Color.BLACK)
                            textSize = 14f
                        }
                        
                        val percentageText = TextView(context).apply {
                            text = "$percentage%"
                            setTextColor(Color.GRAY)
                            textSize = 12f
                        }
                        
                        addView(optionText)
                        addView(percentageText)
                    }
                    
                    addView(optionContainer)
                    
                    setOnClickListener { votePoll(message, option) }
                }
                addView(optionView)
            }
            
            val footer = TextView(context).apply {
                layoutParams = LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT
                ).apply {
                    topMargin = dp(8)
                }
                text = "$totalVoters голосов"
                setTextColor(Color.GRAY)
                textSize = 12f
            }
            addView(footer)
        }
    }
    
    private fun createReactionView(emoji: String, count: Int, isSelected: Boolean): View {
        return TextView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                marginEnd = dp(4)
            }
            text = "$emoji $count"
            textSize = 12f
            setTextColor(if (isSelected) COLOR_ACCENT else Color.GRAY)
            setPadding(dp(6), dp(2), dp(6), dp(2))
            background = createRoundedDrawable(
                if (isSelected) Color.parseColor("#E3F2FD") else Color.parseColor("#F0F0F0"),
                dp(12).toFloat()
            )
        }
    }

    // ==================== UTILITY METHODS ====================
    
    private fun dp(value: Int): Int = (value * resources.displayMetrics.density).toInt()
    
    private fun createCircleDrawable(color: Int, strokeColor: Int? = null, strokeWidth: Int = 0): GradientDrawable {
        return GradientDrawable().apply {
            shape = GradientDrawable.OVAL
            setColor(color)
            if (strokeColor != null && strokeWidth > 0) {
                setStroke(strokeWidth, strokeColor)
            }
        }
    }
    
    private fun createRoundedDrawable(color: Int, radius: Float): GradientDrawable {
        return GradientDrawable().apply {
            shape = GradientDrawable.RECTANGLE
            cornerRadius = radius
            setColor(color)
        }
    }
    
    private fun createBubbleDrawable(isOutgoing: Boolean): Drawable {
        val color = if (isOutgoing) COLOR_BUBBLE_OUTGOING else COLOR_BUBBLE_INCOMING
        return GradientDrawable().apply {
            shape = GradientDrawable.RECTANGLE
            cornerRadii = floatArrayOf(
                dp(16).toFloat(), dp(16).toFloat(), // top-left
                dp(16).toFloat(), dp(16).toFloat(), // top-right
                if (isOutgoing) dp(4).toFloat() else dp(16).toFloat(),
                if (isOutgoing) dp(4).toFloat() else dp(16).toFloat(), // bottom-right
                if (isOutgoing) dp(16).toFloat() else dp(4).toFloat(),
                if (isOutgoing) dp(16).toFloat() else dp(4).toFloat() // bottom-left
            )
            setColor(color)
        }
    }
    
    private fun selectableItemBackground(): Int = android.R.attr.selectableItemBackground
    
    private fun selectableItemBackgroundBorderless(): Int = android.R.attr.selectableItemBackgroundBorderless
    
    private fun formatMessageText(text: String): SpannableString {
        val spannable = SpannableString(text)
        
        // URL detection
        val urlPattern = Patterns.WEB_URL
        val matcher = urlPattern.matcher(text)
        while (matcher.find()) {
            val url = matcher.group()
            spannable.setSpan(
                URLSpan(url),
                matcher.start(),
                matcher.end(),
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE
            )
        }
        
        // Bold text **text**
        val boldPattern = "\*\*(.+?)\*\*".toRegex()
        boldPattern.findAll(text).forEach { result ->
            spannable.setSpan(
                StyleSpan(Typeface.BOLD),
                result.range.first,
                result.range.last + 1,
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE
            )
        }
        
        return spannable
    }
    
    private fun formatFileSize(bytes: Long): String {
        return when {
            bytes < 1024 -> "$bytes Б"
            bytes < 1024 * 1024 -> "${bytes / 1024} КБ"
            bytes < 1024 * 1024 * 1024 -> "${bytes / (1024 * 1024)} МБ"
            else -> "${bytes / (1024 * 1024 * 1024)} ГБ"
        }
    }
    
    private fun Double.format(digits: Int) = "%.${digits}f".format(this)
    
    private fun animateViewIn(view: View) {
        view.alpha = 0f
        view.animate()
            .alpha(1f)
            .setDuration(ANIM_DURATION_SHORT)
            .start()
    }
    
    private fun showKeyboard() {
        val imm = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        imm.showSoftInput(messageInput, InputMethodManager.SHOW_IMPLICIT)
    }
    
    private fun hideKeyboard() {
        val imm = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        imm.hideSoftInputFromWindow(messageInput.windowToken, 0)
    }
    
    private fun vibrateShort() {
        val vibrator = getSystemService(Context.VIBRATOR_SERVICE) as Vibrator
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            vibrator.vibrate(VibrationEffect.createOneShot(50, VibrationEffect.DEFAULT_AMPLITUDE))
        } else {
            @Suppress("DEPRECATION")
            vibrator.vibrate(50)
        }
    }

    // ==================== PERMISSIONS ====================
    
    private fun checkAudioPermission(): Boolean {
        return ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) == PackageManager.PERMISSION_GRANTED
    }
    
    private fun requestRequiredPermissions() {
        val permissions = arrayOf(
            Manifest.permission.RECORD_AUDIO,
            Manifest.permission.CAMERA,
            Manifest.permission.READ_EXTERNAL_STORAGE
        )
        
        val needed = permissions.filter {
            ContextCompat.checkSelfPermission(this, it) != PackageManager.PERMISSION_GRANTED
        }
        
        if (needed.isNotEmpty()) {
            ActivityCompat.requestPermissions(this, needed.toTypedArray(), PERMISSION_AUDIO)
        }
    }

    // ==================== MEDIA & FILES ====================
    
    private fun pickImage() {
        Toast.makeText(this, "Выбор изображения...", Toast.LENGTH_SHORT).show()
    }
    
    private fun pickVideo() {
        Toast.makeText(this, "Выбор видео...", Toast.LENGTH_SHORT).show()
    }
    
    private fun pickFile() {
        Toast.makeText(this, "Выбор файла...", Toast.LENGTH_SHORT).show()
    }
    
    private fun openCamera() {
        Toast.makeText(this, "Открытие камеры...", Toast.LENGTH_SHORT).show()
    }
    
    private fun shareLocation() {
        val message = Message(
            type = MSG_TYPE_LOCATION,
            senderId = currentUserId,
            senderName = currentUserName,
            latitude = 55.7558,
            longitude = 37.6173
        )
        addMessage(message)
        simulateMessageSending(message)
    }
    
    private fun shareContact() {
        val message = Message(
            type = MSG_TYPE_CONTACT,
            senderId = currentUserId,
            senderName = currentUserName,
            contactName = "Иван Иванов",
            contactPhone = "+7 999 123 4567"
        )
        addMessage(message)
        simulateMessageSending(message)
    }
    
    private fun createPoll() {
        val message = Message(
            type = MSG_TYPE_POLL,
            senderId = currentUserId,
            senderName = currentUserName,
            pollQuestion = "Какой ваш любимый язык программирования?",
            pollOptions = listOf(
                PollOption(text = "Kotlin"),
                PollOption(text = "Java"),
                PollOption(text = "Python"),
                PollOption(text = "JavaScript")
            )
        )
        addMessage(message)
        simulateMessageSending(message)
    }
    
    private fun playVoiceMessage(message: Message) {
        if (playingVoiceMessageId == message.id) {
            mediaPlayer?.pause()
            playingVoiceMessageId = null
            return
        }
        
        mediaPlayer?.release()
        
        try {
            mediaPlayer = MediaPlayer().apply {
                setDataSource(message.mediaUrl)
                prepare()
                start()
                setOnCompletionListener {
                    playingVoiceMessageId = null
                }
            }
            playingVoiceMessageId = message.id
        } catch (e: Exception) {
            Toast.makeText(this@MainActivity, "Не удалось воспроизвести", Toast.LENGTH_SHORT).show()
        }
    }
    
    private fun openFile(message: Message) {
        Toast.makeText(this, "Открытие файла: ${message.fileName}", Toast.LENGTH_SHORT).show()
    }
    
    private fun openLocation(message: Message) {
        Toast.makeText(this, "Открытие карты...", Toast.LENGTH_SHORT).show()
    }
    
    private fun votePoll(message: Message, option: PollOption) {
        // Remove vote from other options
        message.pollOptions?.forEach { opt ->
            opt.votes.remove(currentUserId)
        }
        // Add vote to selected option
        option.votes.add(currentUserId)
        updateMessageInList(message)
    }

    // ==================== SELECTION ACTIONS ====================
    
    private fun replyToSelectedMessages() {
        if (selection.selectedIds.size == 1) {
            val messageId = selection.selectedIds.first()
            val message = messages.find { it.id == messageId }
            if (message != null) {
                replyTo(message)
            }
        }
        selection.clear()
        updateSelectionUI()
    }
    
    private fun copySelectedMessages() {
        val selectedTexts = selection.selectedIds.mapNotNull { id ->
            messages.find { it.id == id }?.text
        }.joinToString("\n\n")
        
        val clipboard = getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
        val clip = ClipData.newPlainText("messages", selectedTexts)
        clipboard.setPrimaryClip(clip)
        
        Toast.makeText(this, "Скопировано ${selection.selectedIds.size} сообщений", Toast.LENGTH_SHORT).show()
        selection.clear()
        updateSelectionUI()
    }
    
    private fun forwardSelectedMessages() {
        Toast.makeText(this, "Пересылка ${selection.selectedIds.size} сообщений...", Toast.LENGTH_SHORT).show()
        selection.clear()
        updateSelectionUI()
    }
    
    private fun deleteSelectedMessages() {
        AlertDialog.Builder(this)
            .setTitle("Удалить ${selection.selectedIds.size} сообщений?")
            .setPositiveButton("Удалить") { _, _ ->
                selection.selectedIds.forEach { id ->
                    messages.removeAll { it.id == id }
                    messageCache.remove(id)
                }
                updateFilteredMessages()
                selection.clear()
                updateSelectionUI()
            }
            .setNegativeButton("Отмена", null)
            .show()
    }

    // ==================== EMOJI ====================
    
    private fun populateEmojiGrid(emojis: List<String>) {
        emojiGrid.removeAllViews()
        emojis.forEach { emoji ->
            val emojiView = TextView(this).apply {
                layoutParams = GridLayout.LayoutParams().apply {
                    width = dp(44)
                    height = dp(44)
                }
                text = emoji
                textSize = 24f
                gravity = Gravity.CENTER
                setBackgroundResource(selectableItemBackground())
                setOnClickListener { insertEmoji(emoji) }
            }
            emojiGrid.addView(emojiView)
        }
    }
    
    private fun insertEmoji(emoji: String) {
        val start = messageInput.selectionStart
        val end = messageInput.selectionEnd
        messageInput.text.replace(start, end, emoji)
        
        // Add to recent
        addRecentEmoji(emoji)
    }
    
    private fun addRecentEmoji(emoji: String) {
        recentEmojis.remove(emoji)
        recentEmojis.add(0, emoji)
        if (recentEmojis.size > 20) {
            recentEmojis.removeAt(recentEmojis.size - 1)
        }
        saveRecentEmojis()
        updateRecentEmojisRow()
    }
    
    private fun updateRecentEmojisRow() {
        recentEmojisRow.removeAllViews()
        recentEmojis.take(8).forEach { emoji ->
            val emojiView = TextView(this).apply {
                layoutParams = LinearLayout.LayoutParams(dp(40), dp(40))
                text = emoji
                textSize = 20f
                gravity = Gravity.CENTER
                setOnClickListener { insertEmoji(emoji) }
            }
            recentEmojisRow.addView(emojiView)
        }
    }
    
    private fun switchEmojiCategory(index: Int) {
        val emojis = when (index) {
            0 -> recentEmojis
            1 -> EMOJI_SMILEYS
            2 -> EMOJI_GESTURES
            3 -> EMOJI_HEARTS
            else -> EMOJI_SMILEYS
        }
        populateEmojiGrid(emojis)
    }
    
    private fun loadRecentEmojis() {
        val prefs = getSharedPreferences("chat_prefs", Context.MODE_PRIVATE)
        val saved = prefs.getString("recent_emojis", null)
        if (saved != null) {
            recentEmojis = saved.split(",").toMutableList()
        }
    }
    
    private fun saveRecentEmojis() {
        val prefs = getSharedPreferences("chat_prefs", Context.MODE_PRIVATE)
        prefs.edit().putString("recent_emojis", recentEmojis.joinToString(",")).apply()
    }

    // ==================== CALLS ====================
    
    private fun startVoiceCall() {
        Toast.makeText(this, "Начинаем голосовой звонок...", Toast.LENGTH_SHORT).show()
        addSystemMessage("Голосовой звонок (1:23)")
    }
    
    private fun startVideoCall() {
        Toast.makeText(this, "Начинаем видеозвонок...", Toast.LENGTH_SHORT).show()
        addSystemMessage("Видеозвонок (2:45)")
    }
    
    private fun addSystemMessage(text: String) {
        val message = Message(
            type = MSG_TYPE_SYSTEM,
            text = text,
            senderId = "system"
        )
        addMessage(message)
    }

    // ==================== MENU & DIALOGS ====================
    
    private fun showChatMenu() {
        val popup = PopupMenu(this, toolbarMenuButton)
        popup.menu.apply {
            add(0, 1, 0, "Очистить историю")
            add(0, 2, 1, "Отключить уведомления")
            add(0, 3, 2, "Поиск")
            add(0, 4, 3, "Медиафайлы")
            add(0, 5, 4, "Сменить тему")
            add(0, 6, 5, "Настройки")
        }
        
        popup.setOnMenuItemClickListener { item ->
            when (item.itemId) {
                1 -> clearHistory()
                2 -> toggleMute()
                3 -> showSearch()
                4 -> showMediaGallery()
                5 -> toggleTheme()
                6 -> showSettings()
            }
            true
        }
        
        popup.show()
    }
    
    private fun showChatInfo() {
        Toast.makeText(this, "Информация о чате", Toast.LENGTH_SHORT).show()
    }
    
    private fun clearHistory() {
        AlertDialog.Builder(this)
            .setTitle("Очистить историю?")
            .setMessage("Все сообщения будут удалены")
            .setPositiveButton("Очистить") { _, _ ->
                messages.clear()
                messageCache.clear()
                updateFilteredMessages()
            }
            .setNegativeButton("Отмена", null)
            .show()
    }
    
    private fun toggleMute() {
        currentChat?.isMuted = !(currentChat?.isMuted ?: false)
        Toast.makeText(this, if (currentChat?.isMuted == true) "Уведомления отключены" else "Уведомления включены", Toast.LENGTH_SHORT).show()
    }
    
    private fun showMediaGallery() {
        Toast.makeText(this, "Галерея медиафайлов", Toast.LENGTH_SHORT).show()
    }
    
    private fun toggleTheme() {
        isDarkTheme = !isDarkTheme
        savePreferences()
        recreate()
    }
    
    private fun showSettings() {
        Toast.makeText(this, "Настройки", Toast.LENGTH_SHORT).show()
    }

    // ==================== PREFERENCES ====================
    
    private fun loadPreferences() {
        val prefs = getSharedPreferences("chat_prefs", Context.MODE_PRIVATE)
        isDarkTheme = prefs.getBoolean("dark_theme", false)
    }
    
    private fun savePreferences() {
        val prefs = getSharedPreferences("chat_prefs", Context.MODE_PRIVATE)
        prefs.edit().putBoolean("dark_theme", isDarkTheme).apply()
    }
    
    private fun saveDraft() {
        val draft = messageInput.text.toString()
        currentChat?.draftMessage = if (draft.isNotEmpty()) draft else null
    }
    
    private fun applyTheme() {
        // Theme is applied when creating views
    }

    // ==================== NOTIFICATIONS ====================
    
    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val name = "Сообщения"
            val descriptionText = "Уведомления о новых сообщениях"
            val importance = NotificationManager.IMPORTANCE_HIGH
            val channel = NotificationChannel(CHANNEL_ID, name, importance).apply {
                description = descriptionText
            }
            
            val notificationManager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            notificationManager.createNotificationChannel(channel)
        }
    }
    
    private fun showNotification(message: Message) {
        val builder = NotificationCompat.Builder(this, CHANNEL_ID)
            .setSmallIcon(android.R.drawable.ic_dialog_email)
            .setContentTitle(message.senderName)
            .setContentText(message.text)
            .setPriority(NotificationCompat.PRIORITY_HIGH)
            .setAutoCancel(true)
        
        val notificationManager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        notificationManager.notify(NOTIFICATION_ID, builder.build())
    }

    // ==================== SIMULATION ====================
    
    private fun simulateOnlineStatus(isOnline: Boolean) {
        // Simulate sending online status to server
    }
    
    private fun initializeMockData() {
        // Create mock chat
        currentChat = Chat(
            id = "chat_001",
            name = "Алексей Петров",
            isGroup = false,
            participants = mutableListOf(
                User(
                    id = "user_002",
                    name = "Алексей Петров",
                    isOnline = true
                )
            )
        )
        
        toolbarTitle.text = currentChat?.name
        toolbarSubtitle.text = "в сети"
        toolbarOnlineIndicator.visibility = View.VISIBLE
        
        // Add mock messages
        val mockMessages = listOf(
            Message(
                text = "Привет! Как дела?",
                senderId = "user_002",
                senderName = "Алексей Петров",
                timestamp = System.currentTimeMillis() - 3600000,
                status = STATUS_READ
            ),
            Message(
                text = "Привет! Всё отлично, работаю над новым проектом 💻",
                senderId = currentUserId,
                senderName = currentUserName,
                timestamp = System.currentTimeMillis() - 3500000,
                status = STATUS_READ
            ),
            Message(
                text = "О, круто! Что за проект?",
                senderId = "user_002",
                senderName = "Алексей Петров",
                timestamp = System.currentTimeMillis() - 3400000,
                status = STATUS_READ
            ),
            Message(
                text = "Делаю приложение для чата, вроде Telegram 🚀",
                senderId = currentUserId,
                senderName = currentUserName,
                timestamp = System.currentTimeMillis() - 3300000,
                status = STATUS_READ
            ),
            Message(
                text = "Вау! Звучит интересно! Покажешь когда будет готово?",
                senderId = "user_002",
                senderName = "Алексей Петров",
                timestamp = System.currentTimeMillis() - 3200000,
                status = STATUS_READ
            ),
            Message(
                text = "Конечно! Уже почти закончил основной функционал 👍",
                senderId = currentUserId,
                senderName = currentUserName,
                timestamp = System.currentTimeMillis() - 3100000,
                status = STATUS_DELIVERED
            )
        )
        
        messages.addAll(mockMessages)
        updateFilteredMessages()
    }

    // ==================== CUSTOM VIEWS ====================
    
    inner class VoiceWaveformView(context: Context) : View(context) {
        private val amplitudes = mutableListOf<Int>()
        private val paint = Paint().apply {
            color = Color.RED
            strokeWidth = dp(3).toFloat()
            strokeCap = Paint.Cap.ROUND
        }
        
        fun addAmplitude(amplitude: Int) {
            amplitudes.add(amplitude)
            if (amplitudes.size > 50) {
                amplitudes.removeAt(0)
            }
            invalidate()
        }
        
        override fun onDraw(canvas: Canvas) {
            super.onDraw(canvas)
            
            if (amplitudes.isEmpty()) return
            
            val barWidth = width.toFloat() / 50
            val maxHeight = height.toFloat()
            
            amplitudes.forEachIndexed { index, amp ->
                val normalizedHeight = (amp.toFloat() / 32767 * maxHeight).coerceIn(dp(4).toFloat(), maxHeight)
                val x = index * barWidth + barWidth / 2
                val top = (height - normalizedHeight) / 2
                val bottom = top + normalizedHeight
                
                canvas.drawLine(x, top, x, bottom, paint)
            }
        }
    }
    
    inner class DoubleTapListener(private val onDoubleTap: () -> Unit) : View.OnTouchListener {
        private var lastTapTime: Long = 0
        
        @SuppressLint("ClickableViewAccessibility")
        override fun onTouch(v: View?, event: MotionEvent?): Boolean {
            if (event?.action == MotionEvent.ACTION_DOWN) {
                val currentTime = System.currentTimeMillis()
                if (currentTime - lastTapTime < 300) {
                    onDoubleTap()
                    lastTapTime = 0
                    return true
                }
                lastTapTime = currentTime
            }
            return false
        }
    }
}
