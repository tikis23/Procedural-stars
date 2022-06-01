#pragma once

#include <iostream>
#include <string>
#include <chrono>

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
		std::cout << std::format("[TIMER] {} - {} ms\n", name, Elapsed());
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};

class ScopedTimer {
public:
	ScopedTimer(const std::string& name)
		: m_Name(name) {}
	~ScopedTimer() {
		float time = m_Timer.Elapsed();
		std::cout << std::format("[TIMER] {} - {} ms\n", m_Name, time);
	}
private:
	std::string m_Name;
	Timer m_Timer;
};