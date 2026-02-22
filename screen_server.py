#!/usr/bin/env python3
"""
Сервер для трансляции экрана ПК на Android устройство
Запуск: python screen_server.py
"""

import socket
import struct
import threading
import json
from io import BytesIO
from PIL import ImageGrab
import pyautogui

class ScreenServer:
    def __init__(self, host='0.0.0.0', port=5000):
        self.host = host
        self.port = port
        self.quality = 50
        self.running = False
        self.clients = []
        self.screen_width, self.screen_height = pyautogui.size()
        
    def start(self):
        self.running = True
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_socket.bind((self.host, self.port))
        self.server_socket.listen(5)
        
        print(f"🖥️  Сервер трансляции экрана запущен")
        print(f"📡 Адрес: {self.get_local_ip()}:{self.port}")
        print(f"⏳ Ожидание подключений...")
        
        while self.running:
            try:
                client_socket, address = self.server_socket.accept()
                print(f"✅ Подключен клиент: {address}")
                
                client_thread = threading.Thread(
                    target=self.handle_client,
                    args=(client_socket, address)
                )
                client_thread.daemon = True
                client_thread.start()
                
            except Exception as e:
                if self.running:
                    print(f"Ошибка: {e}")
                    
    def get_local_ip(self):
        """Получение локального IP адреса"""
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            s.connect(('8.8.8.8', 80))
            ip = s.getsockname()[0]
        except:
            ip = '127.0.0.1'
        finally:
            s.close()
        return ip
    
    def handle_client(self, client_socket, address):
        """Обработка клиента"""
        self.clients.append(client_socket)
        
        # Запускаем поток для приема команд
        command_thread = threading.Thread(
            target=self.receive_commands,
            args=(client_socket,)
        )
        command_thread.daemon = True
        command_thread.start()
        
        try:
            while self.running:
                # Захват экрана
                screenshot = ImageGrab.grab()
                
                # Уменьшаем размер для лучшей производительности
                max_width = 1280
                if screenshot.width > max_width:
                    ratio = max_width / screenshot.width
                    new_size = (max_width, int(screenshot.height * ratio))
                    screenshot = screenshot.resize(new_size)
                
                # Сжимаем в JPEG
                buffer = BytesIO()
                screenshot.save(buffer, format='JPEG', quality=self.quality)
                frame_data = buffer.getvalue()
                
                # Отправляем размер кадра (4 байта) + данные
                size_data = struct.pack('>I', len(frame_data))
                client_socket.sendall(size_data + frame_data)
                
        except Exception as e:
            print(f"❌ Клиент отключен {address}: {e}")
        finally:
            if client_socket in self.clients:
                self.clients.remove(client_socket)
            client_socket.close()
            
    def receive_commands(self, client_socket):
        """Прием команд от клиента"""
        try:
            while self.running:
                # Читаем размер команды
                size_data = client_socket.recv(4)
                if not size_data:
                    break
                    
                size = struct.unpack('>I', size_data)[0]
                
                # Отрицательный размер = команда
                if size > 0x80000000:  # Signed negative
                    size = 0xFFFFFFFF - size + 1
                    command_data = client_socket.recv(size)
                    command = json.loads(command_data.decode('utf-8'))
                    self.process_command(command)
                    
        except Exception as e:
            pass
            
    def process_command(self, command):
        """Обработка команды от клиента"""
        cmd_type = command.get('type')
        
        if cmd_type == 'touch':
            # Обработка касания (эмуляция мыши)
            action = command.get('action')
            x = command.get('x', 0) * self.screen_width
            y = command.get('y', 0) * self.screen_height
            
            if action == 'down':
                pyautogui.moveTo(x, y)
                pyautogui.mouseDown()
            elif action == 'up':
                pyautogui.mouseUp()
            elif action == 'move':
                pyautogui.moveTo(x, y)
                
        elif cmd_type == 'quality':
            # Изменение качества
            self.quality = command.get('value', 50)
            print(f"📊 Качество изменено на: {self.quality}")
            
    def stop(self):
        self.running = False
        self.server_socket.close()
        for client in self.clients:
            client.close()

if __name__ == '__main__':
    server = ScreenServer()
    try:
        server.start()
    except KeyboardInterrupt:
        print("\n🛑 Сервер остановлен")
        server.stop()
