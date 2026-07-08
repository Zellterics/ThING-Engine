#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <print>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <source_location>

/*
C++23 header only light app logger
Zlog - 2026-7-6

https://github.com/Zellterics/Zlog

ZellTerics - zellterics@gmail.com
*/

struct packed_time_location{
    std::chrono::milliseconds time;
    std::string location;

    constexpr packed_time_location(std::chrono::milliseconds time, std::source_location location = std::source_location::current())
        : time(time), location((std::string)location.file_name() + ":" + std::to_string(location.line())){}
};

struct packed_maxTimes_location{
    uint32_t maxTimes;
    std::string location;

    constexpr packed_maxTimes_location(uint32_t maxTimes, std::source_location location = std::source_location::current())
        : maxTimes(maxTimes), location((std::string)location.file_name() + ":" + std::to_string(location.line())){}
};

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

    // ZLOG
    template<typename... _Args>
    inline void log(std::format_string<_Args...> fmt, _Args&&... _args){ baseLog("ZLOG", fmt, std::forward<_Args>(_args)...); }
    inline void log(std::string_view str){ baseLog("ZLOG", "{}", str); }

    template<typename... _Args>
    inline void logTimed(packed_time_location packedTimeLoc, std::format_string<_Args...> fmt, _Args&&... _args){
        timedImpl(packedTimeLoc.location, packedTimeLoc.time, "ZLOG", fmt, std::forward<_Args>(_args)...);
    }
    inline void logTimed(packed_time_location packedTimeLoc, std::string_view str){ 
        timedImpl(packedTimeLoc.location, packedTimeLoc.time, "ZLOG", "{}", str); 
    }

    template<typename... _Args>
    inline void logMaxed(packed_maxTimes_location packedMaxLoc, std::format_string<_Args...> fmt, _Args&&... _args){
        maxedImpl(packedMaxLoc.location, packedMaxLoc.maxTimes, "ZLOG", fmt, std::forward<_Args>(_args)...);
    }
    inline void logMaxed(packed_maxTimes_location packedMaxLoc, std::string_view str){
        maxedImpl(packedMaxLoc.location, packedMaxLoc.maxTimes, "ZLOG", "{}", str);
    }

    // WARN
    template<typename... _Args>
    inline void warn(std::format_string<_Args...> fmt, _Args&&... _args){ baseLog("WARN", fmt, std::forward<_Args>(_args)...); }
    inline void warn(std::string_view str){ baseLog("WARN", "{}", str); }

    template<typename... _Args>
    inline void warnTimed(packed_time_location packedTimeLoc, std::format_string<_Args...> fmt, _Args&&... _args){
        timedImpl(packedTimeLoc.location, packedTimeLoc.time, "WARN", fmt, std::forward<_Args>(_args)...);
    }
    inline void warnTimed(packed_time_location packedTimeLoc, std::string_view str){ 
        timedImpl(packedTimeLoc.location, packedTimeLoc.time, "WARN", "{}", str); 
    }

    template<typename... _Args>
    inline void warnMaxed(packed_maxTimes_location packedMaxLoc, std::format_string<_Args...> fmt, _Args&&... _args){
        maxedImpl(packedMaxLoc.location, packedMaxLoc.maxTimes, "WARN", fmt, std::forward<_Args>(_args)...);
    }
    inline void warnMaxed(packed_maxTimes_location packedMaxLoc, std::string_view str){
        maxedImpl(packedMaxLoc.location, packedMaxLoc.maxTimes, "WARN", "{}", str);
    }

    // ERRO
    template<typename... _Args>
    inline void err(std::format_string<_Args...> fmt, _Args&&... _args){ baseLog("ERRO", fmt, std::forward<_Args>(_args)...); }
    inline void err(std::string_view str){ baseLog("ERRO", "{}", str); }

    template<typename... _Args>
    inline void errTimed(packed_time_location packedTimeLoc, std::format_string<_Args...> fmt, _Args&&... _args){
        timedImpl(packedTimeLoc.location, packedTimeLoc.time, "ERRO", fmt, std::forward<_Args>(_args)...);
    }
    inline void errTimed(packed_time_location packedTimeLoc, std::string_view str){ 
        timedImpl(packedTimeLoc.location, packedTimeLoc.time, "ERRO", "{}", str); 
    }

    template<typename... _Args>
    inline void errMaxed(packed_maxTimes_location packedMaxLoc, std::format_string<_Args...> fmt, _Args&&... _args){
        maxedImpl(packedMaxLoc.location, packedMaxLoc.maxTimes, "ERRO", fmt, std::forward<_Args>(_args)...);
    }
    inline void errMaxed(packed_maxTimes_location packedMaxLoc, std::string_view str){
        maxedImpl(packedMaxLoc.location, packedMaxLoc.maxTimes, "ERRO", "{}", str);
    }

    // INFO
    template<typename... _Args>
    inline void info(std::format_string<_Args...> fmt, _Args&&... _args){ baseLog("INFO", fmt, std::forward<_Args>(_args)...); }
    inline void info(std::string_view str){ baseLog("INFO", "{}", str); }

    template<typename... _Args>
    inline void infoTimed(packed_time_location packedTimeLoc, std::format_string<_Args...> fmt, _Args&&... _args){
        timedImpl(packedTimeLoc.location, packedTimeLoc.time, "INFO", fmt, std::forward<_Args>(_args)...);
    }
    inline void infoTimed(packed_time_location packedTimeLoc, std::string_view str){ 
        timedImpl(packedTimeLoc.location, packedTimeLoc.time, "INFO", "{}", str); 
    }

    template<typename... _Args>
    inline void infoMaxed(packed_maxTimes_location packedMaxLoc, std::format_string<_Args...> fmt, _Args&&... _args){
        maxedImpl(packedMaxLoc.location, packedMaxLoc.maxTimes, "INFO", fmt, std::forward<_Args>(_args)...);
    }
    inline void infoMaxed(packed_maxTimes_location packedMaxLoc, std::string_view str){
        maxedImpl(packedMaxLoc.location, packedMaxLoc.maxTimes, "INFO", "{}", str);
    }

    // DEBG
    template<typename... _Args>
    inline void debug(std::format_string<_Args...> fmt, _Args&&... _args){ baseLog("DEBG", fmt, std::forward<_Args>(_args)...); }
    inline void debug(std::string_view str){ baseLog("DEBG", "{}", str); }

    template<typename... _Args>
    inline void debugTimed(packed_time_location packedTimeLoc, std::format_string<_Args...> fmt, _Args&&... _args){
        timedImpl(packedTimeLoc.location, packedTimeLoc.time, "DEBG", fmt, std::forward<_Args>(_args)...);
    }
    inline void debugTimed(packed_time_location packedTimeLoc, std::string_view str){ 
        timedImpl(packedTimeLoc.location, packedTimeLoc.time, "DEBG", "{}", str); 
    }

    template<typename... _Args>
    inline void debugMaxed(packed_maxTimes_location packedMaxLoc, std::format_string<_Args...> fmt, _Args&&... _args){
        maxedImpl(packedMaxLoc.location, packedMaxLoc.maxTimes, "DEBG", fmt, std::forward<_Args>(_args)...);
    }
    inline void debugMaxed(packed_maxTimes_location packedMaxLoc, std::string_view str){
        maxedImpl(packedMaxLoc.location, packedMaxLoc.maxTimes, "DEBG", "{}", str);
    }

    inline static const char* location(const char* location = std::source_location::current().function_name()){return location;}
private:
    template<typename... _Args>
    inline void timedImpl(std::string location, std::chrono::milliseconds time , std::string_view type, std::format_string<_Args...> fmt, _Args&&... __args){
        if(!openFlag) return;
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        static std::unordered_map<std::string , std::chrono::steady_clock::time_point> timeTable;
        if(!timeTable.contains(location)){
            timeTable[location] = now;
            baseLog(type, fmt, std::forward<_Args>(__args)...);
            return;
        }
        if((now - timeTable[location]) >= time){
            timeTable[location] = now;
            baseLog(type, fmt, std::forward<_Args>(__args)...);
        }
    }

    template<typename... _Args>
    inline void maxedImpl(std::string location, uint32_t maxTimes, std::string_view type, std::format_string<_Args...> fmt, _Args&&... _args){
        if(!openFlag) return;
        static std::unordered_map<std::string, uint32_t> counterTable;
        if(!counterTable.contains(location)){
            counterTable[location] = maxTimes;
            baseLog(type, fmt, std::forward<_Args>(_args)...);
            return;
        }
        if(counterTable[location] == 0){
            return;
        }
        counterTable[location]--;
        baseLog(type, fmt, std::forward<_Args>(_args)...);
    }

    template<typename... _Args>
    inline void baseLog(std::string_view type, std::format_string<_Args...> fmt, _Args&&... __args){
        if(!openFlag) return;
        std::print(file, "{} [{}]: ", std::chrono::system_clock::now(), type);
        std::println(file, fmt, std::forward<_Args>(__args)...);
    }
    std::ofstream file;
    bool openFlag;
};