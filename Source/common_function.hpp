# pragma once
#ifndef common_function_hpp
#define common_function_hpp

#include <vector>

//共通関数
void OutputLogFile(const std::string str);

/// @brief count... の idx 番目の値を返す。
/// @param idx 取得する引数のインデックス
/// @param count 引数
template<typename T, typename... Args>
T what(int idx, Args... args) {
	if (idx < 0 || idx >= sizeof...(args)) {
		throw std::out_of_range("what; Index out of range");
	}
	std::vector<T> arguments = { args... };
    return arguments.at(idx);
}
#endif

