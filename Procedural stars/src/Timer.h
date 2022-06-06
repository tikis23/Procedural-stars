#pragma once

#include <iostream>
#include <string>
#include <chrono>

extern bool TIMER_ENABLE_PRINT;

class Timer {
public:
	Timer() {
		Reset();
	}

	void Reset() {
		m_Start = std::chrono::high_resolution_clock::now();
	}

	float ElapsedNano() {
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f * 0.001f;
	}

	float Elapsed() {
		return ElapsedNano() * 1000.0f;
	}

	void PrintElapsed(const std::string& name) {
		if (TIMER_ENABLE_PRINT)
			std::cout << std::format("[{}] {}ms\n", name, Elapsed());
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};

class ScopedTimer {
public:
	ScopedTimer(const std::string& name)
		: m_Name(name) {}
	~ScopedTimer() {
		if (TIMER_ENABLE_PRINT) {
			float time = m_Timer.Elapsed();
			std::cout << std::format("[{}] {}ms\n", m_Name, time);
		}
	}
private:
	std::string m_Name;
	Timer m_Timer;
};