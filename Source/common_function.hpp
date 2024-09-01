# pragma once
#ifndef common_function_hpp

#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>

//共通関数
void OutputLogFile(const std::string str) {
	std::time_t now = std::time(nullptr);
	// tm構造体に変換
	std::tm localTime;
	localtime_s(&localTime, &now);
	std::ostringstream oss;
	oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");

    std::ofstream file("ShoutWars.log", std::ios::app);
    file << "[" << oss.str() << "]" << str << std::endl;
    file.close();
    return;
}

#endif
#define common_function_hpp
