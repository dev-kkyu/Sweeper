#include <iostream>

#include "GameFramework.h"
#include "NetworkManager.h"

// 전역 변수
static int g_Width = 1600;
static int g_Height = 900;

static std::string Title = "Sweeper";
static GameFramework g_GameFramework{ Title, g_Width, g_Height };

// 이벤트 콜백함수
static void fullScreenToggle(GLFWwindow* window);
static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

// 네트워크 패킷 처리 콜백함수
static void packetCallback(unsigned char* packet);

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

	// 네트워크 연결
	NetworkManager::getInstance().connectServer("127.0.0.1");
	NetworkManager::getInstance().setPacketReceivedCallback(packetCallback);	// Recv된 데이터 처리할 함수 설정
	NetworkManager::getInstance().start();										// Recv 시작

	// 메인루프
	while (!glfwWindowShouldClose(window)) {
		// window 이벤트 받기
		glfwPollEvents();

		// 비동기 서버의 완료된 작업 실행
		NetworkManager::getInstance().poll();

		// frame 그리기
		g_GameFramework.drawFrame();
	}

	// 네트워크 연결 해제
	NetworkManager::getInstance().stop();

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
	GLFWmonitor* currMonitor = glfwGetWindowMonitor(window);

	// 기존에 전체화면 이었다면
	if (currMonitor) {
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
		GLFWmonitor* priMonitor = glfwGetPrimaryMonitor();							// 주 모니터 얻어오기
		const GLFWvidmode* currMode = glfwGetVideoMode(priMonitor);					// 현재 비디오 모드 얻기
		// 16:9 비율 만들어주기
		int setWidth = currMode->width;
		int setHeight = currMode->height;
		int setRefreshRate = currMode->refreshRate;
		if (setWidth / 16 * 9 != setHeight) {										// 만약 기존 모드가 16:9가 아니었다면
			// 사용 가능한 모니터 모드 로드
			int modeCount;
			const GLFWvidmode* modes = glfwGetVideoModes(priMonitor, &modeCount);	// 모든 비디오 모드 얻기
			std::vector<int> candidateIdx;
			for (int i = 0; i < modeCount; ++i) {
				if (modes[i].width / 16 * 9 == modes[i].height)						// 16:9 모드 찾기
					candidateIdx.push_back(i);
			}
			if (candidateIdx.empty()) {												// 없으면 전체화면을 사용하지 않는다.
				std::cerr << "Can't find any available monitor modes." << std::endl;
				return;
			}
			int selectIdx = *std::max_element(candidateIdx.begin(), candidateIdx.end(), [&modes](const int a, const int b) {
				if (modes[a].width != modes[b].width)
					return modes[a].width < modes[b].width;							// 가능한 모드중에 제일 높은 해상도 선택
				return modes[a].refreshRate < modes[b].refreshRate;					// 같은 해상도 중에서는 높은 주사율 선택
				});
			setWidth = modes[selectIdx].width;
			setHeight = modes[selectIdx].height;
			setRefreshRate = modes[selectIdx].refreshRate;
		}
		glfwSetWindowMonitor(window, priMonitor, 0, 0, setWidth, setHeight, setRefreshRate);		// 2번째 인자가 지정되면 해당 모니터에 전체화면. 위치 무시
		std::cout << "Full Screen Info : (" << setWidth << " * " << setHeight << " " << setRefreshRate << "Hz)" << std::endl;
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
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	xpos = xpos / g_Width * 2. - 1.;
	ypos = static_cast<double>(g_Height - ypos) / g_Height * 2. - 1.;
	g_GameFramework.processMouseButton(button, action, mods, float(xpos), float(ypos));

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

	xpos = xpos / g_Width * 2. - 1.;
	ypos = static_cast<double>(g_Height - ypos) / g_Height * 2. - 1.;
	g_GameFramework.processMouseCursor(float(xpos), float(ypos));
}

void packetCallback(unsigned char* packet)
{
	g_GameFramework.processPacket(packet);
}
