#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <shellapi.h>
#include <direct.h>
#include <tlhelp32.h>

#pragma comment(lib, "shell32.lib")

using namespace std;

// ===== НАСТРОЙКИ =====
const int DEFAULT_DELAY = 50;
const int PYCHARM_WAIT_SEC = 15;
const bool CLEAR_FILE_FIRST = true;

// ===== ОТПРАВКА КЛАВИШ =====
void sendUnicodeChar(wchar_t wc) {
    INPUT input[2] = {};
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wScan = wc;
    input[0].ki.dwFlags = KEYEVENTF_UNICODE;
    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wScan = wc;
    input[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
    SendInput(2, input, sizeof(INPUT));
}

void sendKey(WORD vk, bool extended = false) {
    INPUT input[2] = {};
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = vk;
    if (extended) input[0].ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = vk;
    input[1].ki.dwFlags = KEYEVENTF_KEYUP | (extended ? KEYEVENTF_EXTENDEDKEY : 0);
    SendInput(2, input, sizeof(INPUT));
}

void sendCtrl(char key) {
    INPUT inputs[4] = {};
    // Ctrl down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    // Key down
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = toupper(key);
    // Key up
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = toupper(key);
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    // Ctrl up
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(4, inputs, sizeof(INPUT));
}

void sendShiftCtrl(char key) {
    INPUT inputs[6] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_SHIFT;
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = toupper(key);
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = toupper(key);
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[4].type = INPUT_KEYBOARD;
    inputs[4].ki.wVk = VK_SHIFT;
    inputs[4].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[5].type = INPUT_KEYBOARD;
    inputs[5].ki.wVk = VK_CONTROL;
    inputs[5].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(6, inputs, sizeof(INPUT));
}

void sendEnter() { sendKey(VK_RETURN); }
void sendTab() { sendKey(VK_TAB); }
void sendEscape() { sendKey(VK_ESCAPE); }

// ===== ГЛОБАЛЬНАЯ ПЕРЕМЕННАЯ ДЛЯ ПОИСКА ОКНА =====
HWND g_foundHwnd = NULL;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    const wchar_t* searchText = (const wchar_t*)lParam;
    wchar_t title[512];
    GetWindowTextW(hwnd, title, 512);
    
    if (wcsstr(title, searchText) && IsWindowVisible(hwnd)) {
        g_foundHwnd = hwnd;
        return FALSE; // Останавливаем перечисление
    }
    return TRUE;
}

// ===== УПРАВЛЕНИЕ ОКНАМИ =====
HWND findWindowByPartialTitle(const wchar_t* titlePart) {
    g_foundHwnd = NULL;
    EnumWindows(EnumWindowsProc, (LPARAM)titlePart);
    return g_foundHwnd;
}

bool waitForWindow(const wchar_t* titlePart, int timeoutSec = 30) {
    cout << "[*] Ozhidanie okna: ";
    wcout << titlePart << endl;
    
    for (int i = 0; i < timeoutSec * 2; i++) {
        HWND hwnd = findWindowByPartialTitle(titlePart);
        if (hwnd) {
            SetForegroundWindow(hwnd);
            Sleep(200);
            SetForegroundWindow(hwnd); // Повторно для надёжности
            return true;
        }
        Sleep(500);
        cout << ".";
        cout.flush();
    }
    cout << endl;
    return false;
}

bool activatePyCharm() {
    cout << "[*] Poisk okna PyCharm... ";
    
    HWND hwnd = findWindowByPartialTitle(L"PyCharm");
    if (hwnd) {
        cout << "NAIDENO!\n";
        // Несколько попыток активации
        for (int i = 0; i < 3; i++) {
            SetForegroundWindow(hwnd);
            Sleep(200);
        }
        // Альтернативный способ
        ShowWindow(hwnd, SW_RESTORE);
        SetForegroundWindow(hwnd);
        Sleep(300);
        return true;
    }
    cout << "NE NAIDENO!\n";
    return false;
}

// ===== НАВИГАЦИЯ В PYCHARM =====
void navigateToFileInProject(const wstring& filename) {
    cout << "[*] Otkryvayu fail cherez Project view...\n";
    
    // Alt+1 - открыть панель Project
    INPUT inputs[4] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_MENU;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = '1';
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = '1';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_MENU;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(4, inputs, sizeof(INPUT));
    Sleep(800);
    
    // Печатаем имя файла для поиска в дереве
    for (wchar_t c : filename) {
        sendUnicodeChar(c);
        Sleep(50);
    }
    Sleep(500);
    
    sendEnter(); // Открыть файл
    Sleep(800);
}

void openFileWithGoto(const wstring& filename) {
    cout << "[*] Otkryvayu fail cherez Ctrl+Shift+N...\n";
    
    // Ctrl+Shift+N - Go to File
    sendShiftCtrl('N');
    Sleep(1000);
    
    // Печатаем имя файла
    for (wchar_t c : filename) {
        sendUnicodeChar(c);
        Sleep(50);
    }
    Sleep(600);
    
    sendEnter();
    Sleep(800);
    
    sendEscape();
    Sleep(300);
}

void doubleClickFile(const wstring& filename) {
    cout << "[*] Poidom dvoinym klikom po failu v dereve...\n";
    
    // Alt+1 - Project panel
    INPUT inputs[4] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_MENU;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = '1';
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = '1';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_MENU;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(4, inputs, sizeof(INPUT));
    Sleep(1000);
    
    // Стрелка вниз несколько раз чтобы выбрать файл
    for (int i = 0; i < 3; i++) {
        sendKey(VK_DOWN);
        Sleep(200);
    }
    
    // Enter для открытия
    sendEnter();
    Sleep(500);
    sendEnter();
    Sleep(800);
}

void clearFileContent() {
    cout << "[*] Ochishchayu fail...\n";
    sendCtrl('A');
    Sleep(300);
    sendKey(VK_DELETE);
    Sleep(400);
}

// ===== ПЕЧАТЬ С АНИМАЦИЕЙ =====
void typeWithAnimation(const wstring& text, int delayMs) {
    cout << "\n[>] NACHINAYU PECHAT! Smotri na PyCharm\n";
    cout << "[!] Nazhmi ESC dlya ostanovki\n\n";

    for (size_t i = 0; i < text.size(); i++) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            cout << "\n[!] Ostanovleno polzovatelem\n";
            break;
        }

        wchar_t wc = text[i];

        if (wc == L'\n') {
            sendEnter();
        }
        else if (wc == L'\t') {
            // Отправляем 4 пробела вместо таба (PyCharm может авто-форматировать)
            for (int j = 0; j < 4; j++) {
                sendUnicodeChar(L' ');
                Sleep(10);
            }
        }
        else if (wc == L'\r') {
            continue;
        }
        else {
            sendUnicodeChar(wc);
        }

        if (i % 20 == 0) {
            cout << "\r[>] Progress: " << i << "/" << text.size() << " (" << (i * 100 / text.size()) << "%)   ";
            cout.flush();
        }

        int randDelay = delayMs + (rand() % (delayMs / 3 + 1));
        Sleep(randDelay);
    }
    cout << "\n[OK] Pechat zavershena! (" << text.size() << " simvolov)\n";
}

// ===== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ =====
bool createFolderRecursive(const string& path) {
    string currentPath;
    for (char c : path) {
        currentPath += c;
        if (c == '\\' || c == '/') {
            _mkdir(currentPath.c_str());
        }
    }
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
}

bool saveEmptyFile(const string& filepath) {
    ofstream f(filepath, ios::out | ios::trunc);
    if (f.is_open()) {
        f.close();
        return true;
    }
    return false;
}

bool launchPyCharm(const string& projectPath, const string& pycharmExe = "") {
    string exePath;
    
    if (!pycharmExe.empty()) {
        exePath = pycharmExe;
    } else {
        // Типичные пути установки PyCharm
        const char* paths[] = {
            "C:\\Program Files\\JetBrains\\PyCharm Community Edition 2024.1\\bin\\pycharm64.exe",
            "C:\\Program Files\\JetBrains\\PyCharm Community Edition 2023.3\\bin\\pycharm64.exe",
            "C:\\Program Files\\JetBrains\\PyCharm Professional 2024.1\\bin\\pycharm64.exe",
            "C:\\Program Files\\JetBrains\\PyCharm 2024.1\\bin\\pycharm64.exe",
            nullptr
        };
        
        for (int i = 0; paths[i]; i++) {
            ifstream test(paths[i]);
            if (test.good()) {
                exePath = paths[i];
                break;
            }
        }
        
        if (exePath.empty()) {
            exePath = "pycharm64.exe"; // Надеемся что в PATH
        }
    }
    
    cout << "[*] Zapusk: " << exePath << endl;
    cout << "[*] Proekt: " << projectPath << endl;
    
    // Конвертируем пути в wide string
    wstring wExePath(exePath.begin(), exePath.end());
    wstring wProjectPath(projectPath.begin(), projectPath.end());
    
    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = L"open";
    sei.lpFile = wExePath.c_str();
    sei.lpParameters = wProjectPath.c_str();
    sei.nShow = SW_SHOWNORMAL;
    
    if (ShellExecuteExW(&sei)) {
        cout << "[OK] PyCharm zapushen!\n";
        return true;
    }
    
    cout << "[!] ShellExecute error: " << GetLastError() << endl;
    return false;
}

wstring stringToWstring(const string& str) {
    if (str.empty()) return wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wstring result(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
    return result;
}

// ===== MAIN =====
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    srand((unsigned)time(NULL));

    cout << "============================================\n";
    cout << "  AUTO TYPER v2.0 - Vizualnaya pechat\n";
    cout << "============================================\n\n";

    // === СБОР ДАННЫХ ===
    string projectName, basePath, filename, codeSourcePath;

    cout << ">>> Nazvanie proekta (naprimer: MyProject): ";
    getline(cin, projectName);
    if (projectName.empty()) projectName = "TestProject";

    cout << ">>> Papka dlya proektov (naprimer: C:\\Projects): ";
    getline(cin, basePath);
    if (basePath.empty()) basePath = "C:\\Projects";
    
    // Убираем trailing slash если есть
    while (!basePath.empty() && (basePath.back() == '\\' || basePath.back() == '/')) {
        basePath.pop_back();
    }
    
    string projectPath = basePath + "\\" + projectName;

    cout << ">>> Imya faila (naprimer: main.py): ";
    getline(cin, filename);
    if (filename.empty()) filename = "main.py";

    cout << "\n>>> Istochnik koda:\n";
    cout << "    1 - Vvesti vruchnuyu\n";
    cout << "    2 - Zagruzit iz faila\n";
    cout << "    Vybor [1/2]: ";
    
    int mode = 1;
    string modeStr;
    getline(cin, modeStr);
    if (!modeStr.empty()) mode = atoi(modeStr.c_str());

    string code;
    if (mode == 2) {
        cout << ">>> Polniy put k failu s kodom: ";
        getline(cin, codeSourcePath);
        
        ifstream f(codeSourcePath);
        if (!f) {
            cout << "[ERROR] Ne mogu otkryt fail: " << codeSourcePath << endl;
            cout << "Proverj:\n";
            cout << "  - Fail sushestvuet?\n";
            cout << "  - Pravilniy put? (ispolzui \\\\ ili /)\n";
            system("pause");
            return 1;
        }
        code = string((istreambuf_iterator<char>(f)), {});
        f.close();
        cout << "[OK] Kod zagruzhen: " << code.size() << " simvolov\n";
    }
    else {
        cout << "\n>>> Vvodi kod. Kogda zakonchish - napishi END:\n";
        cout << "----------------------------------------\n";
        string line;
        while (getline(cin, line)) {
            if (line == "END" || line == "end") break;
            code += line + "\n";
        }
    }

    if (code.empty()) {
        cout << "[ERROR] Kod pustoy!\n";
        system("pause");
        return 1;
    }

    cout << "\n>>> Zaderzhka mezhdu simvolami v ms [30-100, default 50]: ";
    string delayStr;
    getline(cin, delayStr);
    int delay = delayStr.empty() ? 50 : atoi(delayStr.c_str());
    if (delay < 20) delay = 30;
    if (delay > 200) delay = 100;

    cout << "\n>>> Put k pycharm64.exe (Enter = auto poisk): ";
    string pycharmPath;
    getline(cin, pycharmPath);

    // === ВЫВОД КОНФИГУРАЦИИ ===
    cout << "\n============================================\n";
    cout << "KONFIGURACIYA:\n";
    cout << "  Proekt:    " << projectPath << endl;
    cout << "  Fail:      " << filename << endl;
    cout << "  Kod:       " << code.size() << " simvolov\n";
    cout << "  Zaderzhka: " << delay << " ms\n";
    cout << "============================================\n\n";

    // === ПОДГОТОВКА ===
    cout << "[1/4] Sozdayu papku proekta...\n";
    if (!createFolderRecursive(projectPath)) {
        cout << "      Papka uzhe sushestvuet ili sozdana\n";
    }

    string fullFilepath = projectPath + "\\" + filename;
    cout << "[2/4] Sozdayu fail: " << fullFilepath << endl;
    if (!saveEmptyFile(fullFilepath)) {
        cout << "[ERROR] Ne mogu sozdat fail!\n";
        system("pause");
        return 1;
    }
    cout << "      OK!\n";

    cout << "[3/4] Zapuskayu PyCharm...\n";
    if (!launchPyCharm(projectPath, pycharmPath)) {
        cout << "[WARNING] Avtozapusk ne udalsa. Zapusti PyCharm vruchnuyu!\n";
    }

    cout << "\n[4/4] Ozhidanie zagruzki PyCharm...\n";
    cout << "      !!! NE SVORACHIVAI OKNO PYCHARM !!!\n\n";

    for (int i = PYCHARM_WAIT_SEC; i > 0; i--) {
        cout << "\r      Ostaloss: " << i << " sek...   ";
        cout.flush();
        Sleep(1000);
    }
    cout << "\n\n";

    // === АКТИВАЦИЯ И ОТКРЫТИЕ ФАЙЛА ===
    cout << ">>> Nazhmi Enter kogda PyCharm polnostyu zagruzitsya...\n";
    cin.get();

    if (!activatePyCharm()) {
        cout << "[!] PyCharm ne naiden. Aktivirui okno vruchnuyu i nazhmi Enter...\n";
        cin.get();
    }
    
    Sleep(500);
    
    // Пробуем открыть файл разными способами
    wstring wfilename = stringToWstring(filename);
    
    cout << "\n>>> Kak otkryt fail?\n";
    cout << "    1 - Ctrl+Shift+N (Go to File)\n";
    cout << "    2 - Alt+1 (Project panel)\n";
    cout << "    3 - Fail uzhe otkryt, prosto nachat pechat\n";
    cout << "    Vybor [1/2/3]: ";
    
    string openModeStr;
    getline(cin, openModeStr);
    int openMode = openModeStr.empty() ? 3 : atoi(openModeStr.c_str());
    
    activatePyCharm();
    Sleep(300);
    
    if (openMode == 1) {
        openFileWithGoto(wfilename);
    } else if (openMode == 2) {
        doubleClickFile(wfilename);
    }
    // Для режима 3 ничего не делаем
    
    Sleep(500);
    activatePyCharm();
    
    if (CLEAR_FILE_FIRST) {
        clearFileContent();
    }

    // === ПЕЧАТЬ ===
    wstring wcode = stringToWstring(code);

    cout << "\n============================================\n";
    cout << "  GOTOV K PECHATI!\n";
    cout << "  Ubedis chto kursor v okne PyCharm\n";
    cout << "  Pechat nachnyotsya cherez 5 sekund...\n";
    cout << "============================================\n";

    for (int i = 5; i > 0; i--) {
        cout << "\r>>> START cherez " << i << "...   ";
        cout.flush();
        Sleep(1000);
    }
    cout << "\n\n";

    // Финальная активация
    activatePyCharm();
    Sleep(200);
    
    // Клик в редактор (Escape чтобы закрыть popup'ы)
    sendEscape();
    Sleep(200);

    typeWithAnimation(wcode, delay);

    // Сохранение
    cout << "\n[*] Sohranayu fail (Ctrl+S)...\n";
    sendCtrl('S');
    Sleep(500);

    cout << "\n============================================\n";
    cout << "  GOTOVO! Kod napechatan v PyCharm.\n";
    cout << "============================================\n\n";

    system("pause");
    return 0;
}
