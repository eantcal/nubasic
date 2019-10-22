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
- Multiplatform. Runs on 32-bit and 64-bit Linux/Windows/MacOS
- Built-in help
- Documented (English and Italian Guides)
- Examples include Tetris, Mine Hunter, Breakout, Calculator, TicTacToe
- Tiny version is suitable for embedded systems

## IDE
nuBASIC IDE (Integrated Development Environment, for Windows and Linux/GTK+2) includes a syntax highlighting editor and debugger. 

IDE provides comprehensive facilities to programmers for software development, like the syntax highlighting, which is the ability to recognize keywords and display them in different colors.
Debugger lets you place breakpoints in your source code, add field watches, step through your code, run into procedures, take snapshots and monitor execution as it occurs.

## C++ Compiler Prerequisites
To compile nuBASIC you will need a compiler supporting modern C++ 
(C++17 extensions have been enabled and any further refactoring will be replacing the existing C++11/14 code by using new language features, where applicable).
Tested on latest versions GCC, Microsoft Visual C++, (Apple) Clang.

## Windows Installation Prerequisites
To install successfully nuBASIC (32/64 bit - packages including editor) the following software component is required on the installation computer:
- Visual C++ Redistributable Packages are required.

## Linux Installation Prerequisites
To install nuBASIC (full version) the following software components are required on the installation system:
- X11 - X Windowing System (including xterm, xmessage)
- Gtk+ 2.0
- aplay (which is generally part of most basic Linux distributions).

## Installers
You can download installers here: https://sourceforge.net/projects/nubasic/files/latest/download

## Android
A nuBASIC version for Android (ARM) system is available at https://github.com/eantcal/nubasic/tree/termux (it includes a pre-built binary).
This is version is not stable and relies on Termux (https://termux.com)

![nubasic on android](https://sites.google.com/site/nubasiclanguageinterpreter/home/screenshot/nubasicide_on_android.png?attredirects=0)

(see also README file)
