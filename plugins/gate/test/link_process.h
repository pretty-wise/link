/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <functional>

class Parser {
private:
	const int kInputEnd = 0;
	const int kOutputEnd = 1;
	const int kStdOut = 1;
	pid_t m_pid;
	int m_channel[2];
	std::string m_output;
public:
	Parser();
	bool Run(const char* executable, char* const argv[]); 
	void Wait();
	void Read(int timeout_ms);
	void Kill();
	void ForEachLine(std::function<void (const std::string&)> func);
};
