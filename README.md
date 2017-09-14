# nubasic
nuBASIC is an implementation of an interpreter of the BASIC programming language

![nubasicide_gtk](https://user-images.githubusercontent.com/13032534/27808819-69aaa2c2-6042-11e7-9132-675d1c71d162.png)

nuBASIC has been designed and implemented mainly for educational purposes both for C++ developers that can deal with a non-trivial example of modern C++ programming and for beginners, which may get hooked on programming. 

It is suitable for simple games, educational or small business programs.

Anyone who has previously worked with other BASIC languages will quickly become accustomed to nuBASIC.
Large sections of the basic constructs of nuBASIC are compatible with other BASIC dialects.

nuBASIC is also very simple. It is an excellent tool for teaching programming to a complete beginner, despite this, it has all the features of a modern programming language.

It allows you to write both classic BASIC programs (which use line numbers and GoTo or GoSub control structures) and procedure oriented programs, based on procedural programming paradigm.

## Features
- Free and Open Source (distributed under GPLv2/MIT License)
- Easy to use and simple enough for 8-year-old-child to understand
- Fully-developed procedural programming language
- IDE for Windows and Linux (GTK+2)
- Multiplatform. Runs on 32-bit and 64-bit Linux/Windows x86/amd64
- Built-in help
- Documented (English and Italian Guides)
- Examples include Tetris, Mine Hunter, Breakout, Calculator, TicTacToe
- Tiny version is suitable for embedded systems

## IDE
nuBASIC IDE (Integrated Development Environment, for Windows and Linux/GTK) includes a syntax highlighting editor and debugger. 

IDE provides comprehensive facilities to programmers for software development, like the syntax highlighting, which is the ability to recognize keywords and display them in different colors.
Debugger lets you place breakpoints in your source code, add field watches, step through your code, run into procedures, take snapshots and monitor execution as it occurs.

## Windows Installation Prerequisites
To install successfully nuBASIC (32/64 bit - packages including editor) the following software component is required on the installation computer:
- Visual C++ Redistributable Packages are required.

## Linux Installation Prerequisites
To install nuBASIC (full version) the following software components are required on the installation system:
- X11 - X Windowing System (including xterm, xmessage)

You may compile nuBASIC either enabling or disabling SDL2 support (--enable-sdl2 configure switch is provided). SDL2 support is disabled for default.
In case of disabling SDL2 support, you need "aplay" external tool in order to exec PlaySound function.
The program aplay is generally part of most basic Linux distributions.

- To install SDL2 on Debian/Ubuntu use the following command:
``$ sudo apt-get install libsdl2-2.0-0``
- To install SDL2 on Fedora/Centos use the following command:
``$ yum install SDL2-devel``

## Other dependencies
- nuBASIC (full version for Linux) relies on stb_image.h part of STB library (https://github.com/nothings/stb)
- nuBASIC IDE relies on Scintilla

## Installers
You can download installers here: https://sourceforge.net/projects/nubasic/files/latest/download

![nubasic on android](https://sites.google.com/site/nubasiclanguageinterpreter/home/screenshot/nubasicide_on_android.png?attredirects=0)
