#include "ConcentrationEngine.h"

#include "ConcentrationGraphics.h"

void ConcentrationEngine::Initialize(HINSTANCE hInstance, LPCWSTR gamename, unsigned int width, unsigned int height, bool screenresizeable, bool notitlebar, bool minimizable, bool maximizable)
{
	// 내가 쓸 윈도우를 등록
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc; // 프로시저
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = gamename;
	wcex.hIconSm = NULL;

	RegisterClassExW(&wcex);

	// 그 윈도우를 생성
	hWnd = CreateWindowW(
		gamename,
		gamename,
		screenresizeable ? WS_OVERLAPPEDWINDOW : (notitlebar ? (WS_POPUP) : (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | (minimizable ? WS_MINIMIZEBOX : 0L) | (maximizable ? WS_MAXIMIZEBOX : 0L))), // WS_EX_TOPMOST | WS_POPUP,
		0,
		0,
		width,
		height,
		NULL,
		NULL,
		hInstance,
		NULL);

	SetMenu(hWnd, NULL);

	//resolutionX = width;
	//resolutionY = height;
	//lastWidth = width;
	//lastHeight = height;

	//isFullScreenMode = false;
	//resizable = screenresizeable;

	ConcentrationGraphics::GetInstance().Initialize(width, height, true, hWnd, false);
}

void ConcentrationEngine::Finalize()
{

}

void ConcentrationEngine::Run(int nCmdShow)
{
	// 윈도우 핸들이 가장 중요
	if (!hWnd)
	{
		return;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// 무한루프 (안에 메시지 루프와 게임 루프 등이 포함됨)
	MSG msg;

	while (TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			DispatchMessage(&msg);
		}
		else
		{
			// 엔진 동작
			run();
		}
	}
}

void ConcentrationEngine::run()
{
	ConcentrationGraphics::GetInstance().BeginDraw();
	ConcentrationGraphics::GetInstance().EndDraw();
}

LRESULT ConcentrationEngine::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOUSEWHEEL:
	{
		//Input::GetInstance().updateWheelDelta(GET_WHEEL_DELTA_WPARAM(wParam));
		break;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_ENTERSIZEMOVE:
	{
		//TimeController::GetInstance().MoveWindow();

		break;
	}

	case WM_MOVE:
	{
		//TimeController::GetInstance().MoveWindow();

		break;
	}

	case WM_EXITSIZEMOVE:
	{
		//TimeController::GetInstance().MoveWindow();

		break;
	}

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		break;
	}

	//case WM_SYSCOMMAND:
	//{
	//	if ((wParam & 0xFFF0) == SC_KEYMENU)
	//	{
	//		return 0;
	//	}
	//	break;
	//}

	case WM_SYSKEYDOWN:
	{
		if (wParam == VK_MENU)
		{
			return 0;
		}

		if ((wParam & 0xFFF0) == SC_KEYMENU)
		{
			return 0;
		}

		break;
	}

	case WM_SIZE:
	{
		UINT width = LOWORD(lParam);
		UINT height = HIWORD(lParam);

		//GraphicsSystem::GetInstance().ResizeWindow(width, height);
	}

	default:
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	}
	return 0;
}

ConcentrationEngine& ConcentrationEngine::GetInstance()
{
	static ConcentrationEngine instance;
	return instance;
}

ConcentrationEngine::ConcentrationEngine() :
	hWnd(NULL)
{
}

ConcentrationEngine::~ConcentrationEngine()
{
}
