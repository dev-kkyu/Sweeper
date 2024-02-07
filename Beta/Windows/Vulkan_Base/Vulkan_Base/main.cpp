#include "GameFramework.h"

// 전역 변수
static int g_Width = 800;
static int g_Height = 600;

// 이벤트 콜백함수
static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

static void vulkanMain()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow* window = glfwCreateWindow(g_Width, g_Height, "Vulkan", nullptr, nullptr);

	// Todo. vulkan 생성


	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// Todo. frame 그리기
	}

	// Todo. vulkan 파괴

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

void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	g_Width = width;
	g_Height = height;
	// Todo. 스왑체인에 알려주기
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
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
