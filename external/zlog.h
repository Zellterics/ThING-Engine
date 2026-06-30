#pragma once

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <source_location>

/*
C++23 header only light app logger
Zlog - 2026-6-24

https://github.com/Zellterics/Zlog

ZellTerics - zellterics@gmail.com
*/

struct DisableLogger{};

class Zlog{
public:
    Zlog(DisableLogger DISABLE){
        openFlag = false;
        return;
    }

    Zlog() : Zlog("./Logs/Zlog.log"){}
    
    Zlog(std::filesystem::path path){
        std::filesystem::path base = path.string().substr(0, path.string().find_last_of('.'));
        auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
        std::chrono::year_month_day ymd{today};

        path = base.string() + "_" + 
            std::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now())) +
            "" + (std::string)path.extension();
        if(!path.parent_path().empty()){
            std::filesystem::create_directories(path.parent_path());
        }
        
        file = std::ofstream(path);
        if(!file.is_open()){ 
            openFlag = false;
        }
        openFlag = true;
    }
    
    ~Zlog(){
        if(openFlag){
            file.flush();
            file.close();
            file.clear();
        }
    }

    template<typename... _Args>
    inline void log(std::format_string<_Args...> fmt, _Args&&... _args){ baseLog("ZLOG", fmt, std::forward<_Args>(_args)...); }
    inline void log(std::string_view str){ baseLog("ZLOG", "{}", str); }

    template<typename... _Args>
    inline void warn(std::format_string<_Args...> fmt, _Args&&... _args){ baseLog("WARN", fmt, std::forward<_Args>(_args)...); }
    inline void warn(std::string_view str){ baseLog("WARN", "{}", str); }

    template<typename... _Args>
    inline void err(std::format_string<_Args...> fmt, _Args&&... _args){ baseLog("ERRO", fmt, std::forward<_Args>(_args)...); }
    inline void err(std::string_view str){ baseLog("ERRO", "{}", str); }

    template<typename... _Args>
    inline void info(std::format_string<_Args...> fmt, _Args&&... _args){ baseLog("INFO", fmt, std::forward<_Args>(_args)...); }
    inline void info(std::string_view str){ baseLog("INFO", "{}", str); }

    template<typename... _Args>
    inline void debug(std::format_string<_Args...> fmt, _Args&&... _args){ baseLog("DEBG", fmt, std::forward<_Args>(_args)...); }
    inline void debug(std::string_view str){ baseLog("DEBG", "{}", str); }

    inline static const char* location(const char* location = std::source_location::current().function_name()){return location;}
private:
    template<typename... _Args>
    inline void baseLog(std::string_view type, std::format_string<_Args...> fmt, _Args&&... __args){
        if(!openFlag) return;
        std::print(file, "{} [{}]: ", std::chrono::system_clock::now(), type);
        std::println(file, fmt, std::forward<_Args>(__args)...);
    }
    std::ofstream file;
    bool openFlag;
};