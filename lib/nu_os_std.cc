//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_os_std.h"
#include <ctime>
#include <stdlib.h>

#include <stdlib.h>


/* -------------------------------------------------------------------------- */

#if defined(__MINGW32__)
// For unknown reason putenv is not declared (but it is implemented)
extern "C" int putenv(const char*);
#endif


/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */

int _os_get_time()
{
    time_t t = time(NULL);
    return int(t);
}


/* -------------------------------------------------------------------------- */

static inline tm* _get_local_tm()
{
    time_t t = time(NULL);
    return localtime(&t);
}


/* -------------------------------------------------------------------------- */

int _os_get_month() 
{ 
    return _get_local_tm()->tm_mon + 1; 
}


/* -------------------------------------------------------------------------- */

int _os_get_day() 
{ 
    return _get_local_tm()->tm_mday; 
}


/* -------------------------------------------------------------------------- */

int _os_get_wday() 
{ 
    return _get_local_tm()->tm_wday; 
}


/* -------------------------------------------------------------------------- */

int _os_get_yday() 
{ 
    return _get_local_tm()->tm_yday; 
}


/* -------------------------------------------------------------------------- */

int _os_get_year() 
{ 
    return _get_local_tm()->tm_year + 1900; 
}


/* -------------------------------------------------------------------------- */

int _os_get_hour() 
{ 
    return _get_local_tm()->tm_hour; 
}


/* -------------------------------------------------------------------------- */

int _os_get_min() 
{ 
    return _get_local_tm()->tm_min; 
}


/* -------------------------------------------------------------------------- */

int _os_get_sec() 
{ 
    return _get_local_tm()->tm_sec; 
}


/* -------------------------------------------------------------------------- */

std::string _os_get_systime()
{
    std::string s = asctime(_get_local_tm());

    // Remove \n or \r at the end of the string
    while (!s.empty()) {
        auto c = s.c_str()[s.size() - 1];

        if (c == '\n' || c == '\r')
            s = s.substr(0, s.size() - 1);
        else
            break;
    }

    return s;
}


/* -------------------------------------------------------------------------- */

void _os_beep() 
{ 
    putc(7, stdout); 
}


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#ifdef _WIN32


/* -------------------------------------------------------------------------- */

#include <stdio.h>
#include <time.h>
#include <windows.h>


/* -------------------------------------------------------------------------- */


namespace nu {


/* -------------------------------------------------------------------------- */

int _os_get_vkey() 
{
    if (! kbhit() {
        return -1;
    }

    int key = _getch();
    if (key == 224) {
        key = _getch();
        switch (key) {
        case 71:
            return vk_Home;
        case 72:
            return vk_Up;
        case 73:
            return vk_PageUp;
        case 75:
            return vk_Left;
        case 77:
            return vk_Right;
        case 79:
            return vk_End;
        case 80:
            return vk_Down;
        case 81:
            return vk_PageDown;
        case 82:
            return vk_Insert;
        case 83:
            return vk_Delete;
        }
    }

    return key;
}



/* -------------------------------------------------------------------------- */

std::string _os_get_app_path()
{
    char buffer[1024] = { 0 };
    GetModuleFileNameA(NULL, buffer, 1024);

    return buffer;
}


/* -------------------------------------------------------------------------- */

const __int64 DELTA_EPOCH_IN_MICROSECS = 11644473600000000;


/* -------------------------------------------------------------------------- */

struct timeval2 {
    __int32 tv_sec; /* seconds */
    __int32 tv_usec; /* microseconds */
};


/* -------------------------------------------------------------------------- */

static int gettimeofday(struct timeval2* tv)
{
    FILETIME ft = { 0 };
    __int64 tmpres = 0;
    int rez = 0;

    GetSystemTimeAsFileTime(&ft);

    tmpres = ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /*converting file time to unix epoch*/
    tmpres /= 10; /*convert into microseconds*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS;
    tv->tv_sec = (__int32)(tmpres * 0.000001);
    tv->tv_usec = (tmpres % 1000000);

    return 0;
}


/* -------------------------------------------------------------------------- */

void _os_randomize()
{
    struct timeval2 t1 = { 0 };
    gettimeofday(&t1);
    srand(t1.tv_usec * t1.tv_sec);
}


/* -------------------------------------------------------------------------- */

void _os_delay(int s) 
{ 
    ::Sleep(s * 1000); 
}


/* -------------------------------------------------------------------------- */

void _os_mdelay(int s) 
{ 
    ::Sleep(s); 
}


/* -------------------------------------------------------------------------- */

bool _os_change_dir(const std::string& dir)
{
    return FALSE != SetCurrentDirectory(dir.c_str());
}


/* -------------------------------------------------------------------------- */

std::string _os_get_working_dir()
{
    char buf[MAX_PATH + 1] = { 0 };
    ::GetCurrentDirectory(MAX_PATH, buf);
    return buf;
}


/* -------------------------------------------------------------------------- */

#if defined(__MINGW32__)


int _os_setenv(const char* var, const char* val)
{
    std::string s = var;
    s += "=";
    s += val;
    return putenv(s.c_str());
}


/* -------------------------------------------------------------------------- */

int _os_unsetenv(const char* var)
{
    std::string s = var;
    s += "=";
    return putenv(s.c_str());
}


/* -------------------------------------------------------------------------- */

#endif


/* -------------------------------------------------------------------------- */

int _os_erase_file(const std::string& filepath)
{
    return ::DeleteFile(filepath.c_str()) ? 0 : -1;
}


/* -------------------------------------------------------------------------- */

int _os_make_dir(const std::string& filepath)
{
    return ::CreateDirectory(filepath.c_str(), NULL) ? 0 : -1;
}


/* -------------------------------------------------------------------------- */

int _os_erase_dir(const std::string& filepath)
{
    return ::RemoveDirectory(filepath.c_str()) ? 0 : -1;
}


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#else // LINUX -----------------------------------------------------------------


/* -------------------------------------------------------------------------- */

#include <fcntl.h>
#ifdef __linux__
#include <linux/kd.h>
#include <linux/limits.h>
#endif
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

static int _kbhit()
{
    struct timeval tv = { 0 };

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);

    return select(1, &fds, NULL, NULL, &tv);
}


/* -------------------------------------------------------------------------- */

int _os_get_vkey() 
{
    struct no_echo_t {
        struct termios _oldt;

        no_echo_t() noexcept {
            tcgetattr(STDIN_FILENO, &_oldt); 
            struct termios newt;
            tcgetattr(STDIN_FILENO, &newt);
            newt.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        }

        ~no_echo_t() {
            tcsetattr(STDIN_FILENO, TCSANOW, &_oldt);
        }
    } no_echo;

    if (! _kbhit()) {
        return -1;
    }

    int key = getchar();

    if (key == 0x9) {
        return vk_Tab;
    }
    else if (key == 0xa) {
        return vk_Return;
    }
    else if (key == 127) {
        return vk_BackSpace;
    }
    else if (key == 0x1b) {

        key = getchar();
        if (key < 0) {
            return vk_Escape;
        }

        if (key==0x5b) {
            key = getchar();

            if (key==0x32) {
                key=getchar();
                if (key==0x7e) {
                    return vk_Insert;
                }
            }
            else if (key==0x41) {
                return vk_Up;
            }
            else if (key==0x42) {
                return vk_Down;
            }
            else if (key==0x43) {
                return vk_Right;
            }
            else if (key==0x44) {
                return vk_Left;
            }
            else if (key==0x46) {
                return vk_End;
            }
            else if (key==0x48) {
                return vk_Home;
            }
            else if (key==0x35) {
                key=getchar();
                if (key==0x7e) {
                    return vk_PageUp;
                }
            }
            else if (key==0x36) {
                key=getchar();
                if (key==0x7e) {
                    return vk_PageDown;
                }
            }
            else if (key==0x33) {
                key=getchar();
                if (key==0x7e) {
                    return vk_Delete;
                }
            }
        }
    }

    return key;
}


/* -------------------------------------------------------------------------- */

void _os_randomize()
{
    struct timeval t1 = { 0 };
    gettimeofday(&t1, NULL);
    srand(t1.tv_usec * t1.tv_sec);
}


/* -------------------------------------------------------------------------- */

void _os_delay(int s) 
{ 
    sleep(s); 
}


/* -------------------------------------------------------------------------- */

void _os_mdelay(int s) 
{
    usleep(s * 1000); 
}


/* -------------------------------------------------------------------------- */

bool _os_change_dir(const std::string& dir) { return 0 == chdir(dir.c_str()); }


/* -------------------------------------------------------------------------- */

std::string _os_get_working_dir()
{
    char buf[PATH_MAX + 1] = { 0 };

    if (getcwd(buf, PATH_MAX) != nullptr) {
        return buf;
    }

    return "";
}


/* -------------------------------------------------------------------------- */

int _os_erase_file(const std::string& filepath)
{
    return ::unlink(filepath.c_str()) ? 0 : -1;
}


/* -------------------------------------------------------------------------- */

int _os_erase_dir(const std::string& filepath)
{
    return ::rmdir(filepath.c_str()) ? 0 : -1;
}


/* -------------------------------------------------------------------------- */

int _os_make_dir(const std::string& filepath)
{
    return ::mkdir(filepath.c_str(), 0777 /* & ~umask */);
}


/* -------------------------------------------------------------------------- */

std::string _os_get_app_path()
{
    char exepath[ 1024 ] = { 0 };

    auto ret = ::readlink("/proc/self/exe", exepath, sizeof(exepath) - 1);
    (void) ret;
    return std::string(exepath);
}


} // namespace nu


/* -------------------------------------------------------------------------- */

#endif
