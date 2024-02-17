#include <iostream>

#include "GameFramework.h"

// 전역 변수
static int g_Width = 1600;
static int g_Height = 900;

static std::string Title = "Vulkan";
static GameFramework g_GameFramework{ Title, g_Width, g_Height };

// 이벤트 콜백함수
static void fullScreenToggle(GLFWwindow* window);
static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

static void vulkanMain()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow* window = glfwCreateWindow(g_Width, g_Height, Title.c_str(), nullptr, nullptr);
	glfwSetWindowAspectRatio(window, 16, 9);

	// 콜백함수 설정
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);

	// vulkan 생성
	g_GameFramework.initVulkan(window);

	// 메인루프
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// frame 그리기
		g_GameFramework.drawFrame();
	}

	// vulkan 파괴
	g_GameFramework.cleanup();

	glfwDestroyWindow(window);
	glfwTerminate();
}

int main() {

	try {
		vulkanMain();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void fullScreenToggle(GLFWwindow* window)
{
	static int xpos, ypos, width, height;
	GLFWmonitor* monitor = glfwGetWindowMonitor(window);

	// 기존에 전체화면 이었다면
	if (monitor) {
		if (0 == width or 0 == height) {	// 프로그램 시작시 창모드로 실행을 전제한다.
			throw std::runtime_error("failed to find saved monitor information!");
		}
		// 창모드로 변경
		glfwSetWindowMonitor(window, nullptr, xpos, ypos, width, height, GLFW_DONT_CARE);	// 2번째 인자가 nulllptr이면 창모드로 변경되며, 마지막 인자 무시
	}
	// 기존에 창모드 였다면
	else {
		// 기존 창의 위치와 크기를 백업해두고
		glfwGetWindowPos(window, &xpos, &ypos);
		glfwGetWindowSize(window, &width, &height);

		// 주 모니터에 전체화면으로 전환해준다.
		GLFWmonitor* primary = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(primary);
		glfwSetWindowMonitor(window, primary, 0, 0, mode->width, mode->height, mode->refreshRate);		// 2번째 인자가 지정되면 해당 모니터에 전체화면. 위치 무시
	}
}

void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	g_Width = width;
	g_Height = height;

	// 스왑체인에 알려주기
	g_GameFramework.setFramebufferResized();
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	g_GameFramework.processKeyboard(key, action, mods);

	switch (action)
	{
	case GLFW_PRESS:
		switch (key) {
		case GLFW_KEY_F9:
			fullScreenToggle(window);						// 전체 화면 전환
			break;
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);	// 창 닫기
			break;
		case GLFW_KEY_W:
			break;
		case GLFW_KEY_S:
			break;
		case GLFW_KEY_A:
			break;
		case GLFW_KEY_D:
			break;
		}
		break;
	case GLFW_RELEASE:
		break;
	case GLFW_REPEAT:
		break;
	}
	// mods 활용하여 shift, control, alt의 키가 눌려진 상태인지 파악할 수 있다. 비트연산 사용.
	// scancode - The system-specific scancode of the key.
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	g_GameFramework.processMouse(button, action, mods);

	switch (action)
	{
	case GLFW_PRESS:
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			break;
		case GLFW_MOUSE_BUTTON_4:
			break;
		case GLFW_MOUSE_BUTTON_5:
			break;
		}
		break;
	case GLFW_RELEASE:
		break;
	}
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	// 일반적으로 yoffset만 유효. 음수(down) or 양수(up). -1, 1이 나오는 것 같다
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	// 좌상단 [0, 0], 우하단 [width, height]
}
