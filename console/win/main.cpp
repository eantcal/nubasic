//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "ConsoleWindow.h"
#include <chrono>
#include <thread>
#include <windows.h>

/* -------------------------------------------------------------------------- */

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;

    // Create console window
    console_window_t console(hInstance);

    // Configuration (optional - customize here)
    console_config_t config;
    config.cols = 80;
    config.rows = 25;
    config.scrollback_rows = 1000;
    config.font_name = L"Consolas";
    config.font_size = 16;
    config.text_color = RGB(192, 192, 192);
    config.background_color = RGB(0, 0, 0);
    config.cursor_color = RGB(255, 255, 255);
    config.cursor_blink = true;
    config.tab_width = 8;
    config.auto_scroll = true;

    console.set_config(config);

    // Create and show window
    if (!console.create(L"nuBASIC Console - Input Test")) {
        MessageBoxW(nullptr, L"Failed to create console window", L"Error",
            MB_ICONERROR);
        return 1;
    }

    console.show(nCmdShow);

    // Run tests in a separate thread
    std::thread test_thread([&console]() {
        console_buffer_t& buffer = console.get_buffer();

        buffer.write(L"nuBASIC Console v1.0 - Input Test\r\n");
        buffer.write(L"==================================\r\n\r\n");
        console.refresh();

        // Test 1: Blocking input (like INPUT or std::cin)
        buffer.write(L"[Test 1: Blocking Input]\r\n");
        buffer.write(L"What is your name? ");
        console.refresh();

        std::wstring name = console.read_line();

        buffer.write(L"Hello, ");
        buffer.write(name);
        buffer.write(L"!\r\n\r\n");
        console.refresh();

        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Test 2: Non-blocking input (like INKEY$)
        buffer.write(L"[Test 2: Non-blocking Input - INKEY$]\r\n");
        buffer.write(L"Press any key within 10 seconds...\r\n");
        buffer.write(L"(Press 'ESC' to quit this test)\r\n\r\n");
        console.refresh();

        auto start = std::chrono::steady_clock::now();
        bool quit = false;

        while (!quit) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed
                = std::chrono::duration_cast<std::chrono::seconds>(now - start)
                      .count();

            if (elapsed >= 10) {
                buffer.write(L"\r\nTime's up!\r\n\r\n");
                console.refresh();
                break;
            }

            if (console.is_key_available()) {
                std::wstring key = console.get_key();
                if (!key.empty()) {
                    if (key[0] == 27) { // ESC
                        quit = true;
                        buffer.write(L"\r\nQuitting test...\r\n\r\n");
                    } else {
                        buffer.write(L"Key pressed: '");
                        if (key[0] >= 32) {
                            buffer.write(key);
                        } else {
                            buffer.write(L"<ctrl>");
                        }
                        buffer.write(L"' (code: ");
                        wchar_t code_str[32];
                        swprintf(code_str, 32, L"%d", (int)key[0]);
                        buffer.write(code_str);
                        buffer.write(L")\r\n");
                    }
                    console.refresh();
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        // Test 3: Another blocking input
        buffer.write(L"[Test 3: Another Blocking Input]\r\n");
        buffer.write(L"Enter your age: ");
        console.refresh();

        std::wstring age = console.read_line();

        buffer.write(L"You are ");
        buffer.write(age);
        buffer.write(L" years old.\r\n\r\n");
        buffer.write(L"=== All tests completed! ===\r\n");
        console.refresh();
    });

    test_thread.detach();

    // Message loop
    return console.run();
}

/* -------------------------------------------------------------------------- */
