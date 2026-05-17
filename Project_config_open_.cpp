#include <iostream>
#include <cstdlib>
#include <string>
#include <filesystem>
#include <windows.h>
#include <conio.h>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;


fs::path get_executable_dir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return fs::path(buffer).parent_path();
}

const fs::path CONFIG_FILE = get_executable_dir() / "config.txt";

struct Bind
{
    char key1;
    char key2;
    std::string path;
};

void open_program(const fs::path& path) {
    
    std::string path_str = "\"" + path.string() + "\"";

#if defined(_WIN32) || defined(_WIN64)
    
    std::string command = "start \"\" " + path_str;
#elif defined(__APPLE__)
    std::string command = "open " + path_str;
#elif defined(__linux__)
    std::string command = "xdg-open " + path_str;
#else
#error "Операционная system не поддерживается!"
#endif

    std::system(command.c_str());
}


void save_config(const std::vector<Bind>&vec) {
    std::ofstream file(CONFIG_FILE);
    if (file.is_open()) {
        for (const auto& i : vec)
        {
            file << i.key1 << " " << i.key2 << " " << i.path << "\n";
        }
        file.close();
    }
}

std::vector<Bind> load_config() {
    std::vector<Bind>return_vec;
    std::ifstream file(CONFIG_FILE);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file,line))
        {
            if (line.empty()) continue;

            std::stringstream ss(line);
            Bind bind;

            ss >> bind.key1 >> bind.key2;

            ss.ignore();
            std::getline(ss, bind.path);
            return_vec.push_back(bind);
        }
        file.close();
    }
    return return_vec;
}
int char_to_vk(char ch) {
    return VkKeyScanA(ch) & 0xFF;
}
int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    std::vector<Bind> active_binds = load_config();


    if (!active_binds.empty()) {
        std::cout << "=== Загруженные бинды ===\n";
        for (size_t i = 0; i < active_binds.size(); ++i) {
            std::cout << i + 1 << ". [" << active_binds[i].key1 << "] + ["
                << active_binds[i].key2 << "] -> " << active_binds[i].path << "\n";
        }
        std::cout << "=========================\n\n";
    }
    else {
        std::cout << "Сохраненных биндов пока нет.\n\n";
    }

    std::cout << "Хотите добавить новый бинд? (y/n): ";
    char choice = _getch();
    std::cout << choice << "\n";

    if (choice == 'y' || choice == 'Y' || choice == 'л' || choice == 'Л') {
        Bind new_bind;
        std::cout << "1. Введите ссылку или путь: ";
        std::getline(std::cin, new_bind.path);

        std::cout << "2. Нажмите ПЕРВУЮ клавишу: ";
        new_bind.key1 = _getch();
        std::cout << "[" << new_bind.key1 << "]\n";

        std::cout << "3. Нажмите ВТОРУЮ клавишу: ";
        new_bind.key2 = _getch();
        std::cout << "[" << new_bind.key2 << "]\n";

        active_binds.push_back(new_bind);
        save_config(active_binds);
        std::cout << "\nНовый бинд успешно добавлен и сохранен!\n\n";
    }

    if (active_binds.empty()) {
        std::cout << "Нет активных биндов для отслеживания. Программа завершает работу.\n";
        return 0;
    }
    
    std::cout << "Режим ожидания. Нажимайте ваши комбинации клавиш...\n";
    Sleep(2000);
    HWND window = GetConsoleWindow();
    ShowWindow(window, SW_HIDE); // SW_HIDE = 0
    

    while (true) {
        for (const auto& b : active_binds) {
            int first = char_to_vk(std::tolower(b.key1));
            int second = char_to_vk(std::tolower(b.key2));

            if (GetAsyncKeyState(first)&0x8000 && GetAsyncKeyState(second) & 0x8000)
            {
                std::cout << "\nСработала комбинация [" << b.key1 << "]+[" << b.key2 << "]. Открываю: " << b.path << "\n";
                open_program(b.path);
                Sleep(1000);
            }
        }
        Sleep(10);
    }
    
    return 0;
}
