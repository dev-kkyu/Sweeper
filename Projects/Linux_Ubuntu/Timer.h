#pragma once

#include <chrono>
#include <string>

struct GLFWwindow;
class Timer {
	std::chrono::steady_clock::time_point last_time;		// 마지막으로 호출된 시간
	std::chrono::nanoseconds accm_time;
	int fps;
	int frame;

	std::string window_title;
	std::string gpu_name;
	int& winWidth;
	int& winHeight;
	GLFWwindow* window;

public:
	Timer(std::string title, int& width, int& height);

	void SetWindow(GLFWwindow* window);
	void SetGpuName(std::string name);

	float Tick(int fps_value);		// 경과 시간 리턴, 1 Frame에 단 한번 호출되어야 함
};


