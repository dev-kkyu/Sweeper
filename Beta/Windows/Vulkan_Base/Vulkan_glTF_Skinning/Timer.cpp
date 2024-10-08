﻿#include "Timer.h"
#include <sstream>
#include <thread>
#include <cmath>

#include <GLFW/glfw3.h>

Timer::Timer(std::string title, int& width, int& height)
	: last_time{ std::chrono::steady_clock::now() }, accm_time{ std::chrono::seconds{ 0 } }, fps{ 0 }, frame{ 0 },
	winWidth{ width }, winHeight{ height }, window_title{ title }, window{ nullptr }
{
}

void Timer::SetWindow(GLFWwindow* window)
{
	this->window = window;
}

void Timer::SetGpuName(std::string name)
{
	gpu_name = name;
}

float Timer::Tick(int fps_value)		// 경과 시간 리턴, 1 Frame에 단 한번 호출되어야 함
{
	if (fps_value > 0 and accm_time.count() / 1'000'000'000. * fps_value < frame)		// 프레임 제한
		std::this_thread::sleep_until(last_time + std::chrono::nanoseconds{ static_cast<int>(1. / fps_value * 1'000'000'000.) });

	++frame;
	auto now_time = std::chrono::steady_clock::now();
	auto elapsed_time = now_time - last_time;
	accm_time += elapsed_time;
	last_time = now_time;
	if (accm_time >= std::chrono::seconds{ 1 }) {			// 초당 한번씩 FPS 표시
		float over_time = accm_time.count() / 1'000'000'000.f;
		fps = static_cast<int>(std::round(frame * (1.f / over_time)));
		frame -= fps;
		accm_time -= std::chrono::seconds{ 1 };

		// 창 이름에 FPS 표시
		if (window) {
			std::stringstream title;
			title << window_title << " : " << gpu_name << " [" << winWidth << " * " << winHeight << "] (" << fps << "FPS)";
			glfwSetWindowTitle(window, title.str().c_str());
		}
	}

	return elapsed_time.count() / 1'000'000'000.f;		// 나노초를 초로 바꿔준다.
}
