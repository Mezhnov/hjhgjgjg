
package com.example.screencast

import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.widget.*
import android.view.View
import android.view.MotionEvent
import androidx.appcompat.app.AppCompatActivity
import java.io.*
import java.net.HttpURLConnection
import java.net.Socket
import java.net.URL
import java.util.concurrent.Executors
import android.util.Base64
import org.json.JSONObject

class MainActivity : AppCompatActivity() {
    
    // UI компоненты
    private lateinit var imageView: ImageView
    private lateinit var ipEditText: EditText
    private lateinit var portEditText: EditText
    private lateinit var connectButton: Button
    private lateinit var disconnectButton: Button
    private lateinit var statusText: TextView
    private lateinit var qualitySeekBar: SeekBar
    private lateinit var fpsText: TextView
    
    // Сетевые компоненты
    private var isConnected = false
    private var socket: Socket? = null
    private val executor = Executors.newSingleThreadExecutor()
    private val handler = Handler(Looper.getMainLooper())
    
    // Настройки
    private var serverIp = "192.168.1.100"
    private var serverPort = 5000
    private var quality = 50
    
    // FPS счетчик
    private var frameCount = 0
    private var lastFpsTime = System.currentTimeMillis()
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        // Создаем UI программно
        val mainLayout = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(16, 16, 16, 16)
        }
        
        // Панель настроек подключения
        val settingsLayout = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
        }
        
        // Поле ввода IP
        ipEditText = EditText(this).apply {
            hint = "IP адрес сервера"
            setText("192.168.1.100")
            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f)
        }
        
        // Поле ввода порта
        portEditText = EditText(this).apply {
            hint = "Порт"
            setText("5000")
            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 0.5f)
            inputType = android.text.InputType.TYPE_CLASS_NUMBER
        }
        
        settingsLayout.addView(ipEditText)
        settingsLayout.addView(portEditText)
        
        // Панель кнопок
        val buttonLayout = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
        }
        
        // Кнопка подключения
        connectButton = Button(this).apply {
            text = "Подключиться"
            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f)
            setOnClickListener { connect() }
        }
        
        // Кнопка отключения
        disconnectButton = Button(this).apply {
            text = "Отключиться"
            isEnabled = false
            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f)
            setOnClickListener { disconnect() }
        }
        
        buttonLayout.addView(connectButton)
        buttonLayout.addView(disconnectButton)
        
        // Текст статуса
        statusText = TextView(this).apply {
            text = "Статус: Отключено"
            textSize = 14f
        }
        
        // FPS текст
        fpsText = TextView(this).apply {
            text = "FPS: 0"
            textSize = 14f
        }
        
        // Настройка качества
        val qualityLayout = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
        }
        
        val qualityLabel = TextView(this).apply {
            text = "Качество: "
        }
        
        qualitySeekBar = SeekBar(this).apply {
            max = 100
            progress = 50
            layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f)
            setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
                override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                    quality = progress
                }
                override fun onStartTrackingTouch(seekBar: SeekBar?) {}
                override fun onStopTrackingTouch(seekBar: SeekBar?) {
                    sendQualityChange()
                }
            })
        }
        
        qualityLayout.addView(qualityLabel)
        qualityLayout.addView(qualitySeekBar)
        
        // Основной ImageView для отображения экрана
        imageView = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                0,
                1f
            )
            scaleType = ImageView.ScaleType.FIT_CENTER
            setBackgroundColor(android.graphics.Color.BLACK)
            
            // Обработка касаний для удаленного управления
            setOnTouchListener { v, event ->
                if (isConnected) {
                    handleTouch(event)
                }
                true
            }
        }
        
        // Добавляем все компоненты
        mainLayout.addView(settingsLayout)
        mainLayout.addView(buttonLayout)
        mainLayout.addView(statusText)
        mainLayout.addView(fpsText)
        mainLayout.addView(qualityLayout)
        mainLayout.addView(imageView)
        
        setContentView(mainLayout)
    }
    
    private fun connect() {
        serverIp = ipEditText.text.toString()
        serverPort = portEditText.text.toString().toIntOrNull() ?: 5000
        
        executor.execute {
            try {
                socket = Socket(serverIp, serverPort)
                isConnected = true
                
                handler.post {
                    statusText.text = "Статус: Подключено к $serverIp:$serverPort"
                    connectButton.isEnabled = false
                    disconnectButton.isEnabled = true
                }
                
                // Запускаем прием кадров
                receiveFrames()
                
            } catch (e: Exception) {
                handler.post {
                    statusText.text = "Ошибка: ${e.message}"
                    Toast.makeText(this, "Ошибка подключения: ${e.message}", Toast.LENGTH_LONG).show()
                }
            }
        }
    }
    
    private fun disconnect() {
        isConnected = false
        socket?.close()
        socket = null
        
        handler.post {
            statusText.text = "Статус: Отключено"
            connectButton.isEnabled = true
            disconnectButton.isEnabled = false
            imageView.setImageBitmap(null)
        }
    }
    
    private fun receiveFrames() {
        try {
            val inputStream = DataInputStream(socket!!.getInputStream())
            
            while (isConnected && socket?.isConnected == true) {
                // Читаем размер кадра (4 байта)
                val frameSize = inputStream.readInt()
                
                if (frameSize > 0 && frameSize < 10_000_000) { // Максимум 10 МБ
                    // Читаем данные кадра
                    val frameData = ByteArray(frameSize)
                    inputStream.readFully(frameData)
                    
                    // Декодируем изображение
                    val bitmap = BitmapFactory.decodeByteArray(frameData, 0, frameData.size)
                    
                    // Обновляем UI
                    handler.post {
                        bitmap?.let {
                            imageView.setImageBitmap(it)
                            updateFps()
                        }
                    }
                }
            }
        } catch (e: Exception) {
            if (isConnected) {
                handler.post {
                    statusText.text = "Соединение потеряно: ${e.message}"
                    disconnect()
                }
            }
        }
    }
    
    private fun updateFps() {
        frameCount++
        val currentTime = System.currentTimeMillis()
        val elapsed = currentTime - lastFpsTime
        
        if (elapsed >= 1000) {
            val fps = (frameCount * 1000.0 / elapsed).toInt()
            fpsText.text = "FPS: $fps"
            frameCount = 0
            lastFpsTime = currentTime
        }
    }
    
    private fun handleTouch(event: MotionEvent) {
        // Отправляем события касания на сервер для удаленного управления
        executor.execute {
            try {
                val outputStream = socket?.getOutputStream() ?: return@execute
                val writer = PrintWriter(outputStream, true)
                
                val imageWidth = imageView.width.toFloat()
                val imageHeight = imageView.height.toFloat()
                
                // Нормализуем координаты
                val normalizedX = event.x / imageWidth
                val normalizedY = event.y / imageHeight
                
                val action = when (event.action) {
                    MotionEvent.ACTION_DOWN -> "down"
                    MotionEvent.ACTION_UP -> "up"
                    MotionEvent.ACTION_MOVE -> "move"
                    else -> return@execute
                }
                
                // Формируем JSON сообщение
                val message = JSONObject().apply {
                    put("type", "touch")
                    put("action", action)
                    put("x", normalizedX)
                    put("y", normalizedY)
                }
                
                // Отправляем как отдельный пакет
                val data = message.toString().toByteArray()
                val dataOutputStream = DataOutputStream(outputStream)
                dataOutputStream.writeInt(-data.size) // Отрицательный размер = команда
                dataOutputStream.write(data)
                dataOutputStream.flush()
                
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }
    
    private fun sendQualityChange() {
        executor.execute {
            try {
                val outputStream = socket?.getOutputStream() ?: return@execute
                
                val message = JSONObject().apply {
                    put("type", "quality")
                    put("value", quality)
                }
                
                val data = message.toString().toByteArray()
                val dataOutputStream = DataOutputStream(outputStream)
                dataOutputStream.writeInt(-data.size)
                dataOutputStream.write(data)
                dataOutputStream.flush()
                
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }
    
    override fun onDestroy() {
        super.onDestroy()
        disconnect()
        executor.shutdown()
    }
}
