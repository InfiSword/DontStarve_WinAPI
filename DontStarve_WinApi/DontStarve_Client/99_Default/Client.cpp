#include "99_Default/pch.h"
#include "framework.h"
#include "Client.h"
#include "../00_MainGame/DontStarve_MainGame.h"
#include "../01_Manager/InputManager/InputManager.h"
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

#define MAX_LOADSTRING 100

// 실행 파일 위치에서 Resource·MapData가 있는 프로젝트 루트로 작업 디렉터리 설정
static void EnsureResourceWorkingDirectory()
{
    wchar_t exePath[MAX_PATH];
    if (GetModuleFileNameW(NULL, exePath, MAX_PATH) == 0) return;
    std::wstring dir = exePath;
    size_t lastSlash = dir.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) dir.resize(lastSlash + 1);
    for (int level = 0; level < 5; ++level) {
        std::wstring resourceDir = dir + L"Resource";
        if (PathFileExistsW(resourceDir.c_str())) {
            if (SetCurrentDirectoryW(dir.c_str())) {
                break;
            }
        }
        size_t prev = dir.find_last_of(L"\\/", dir.length() - 2);
        if (prev == std::wstring::npos) break;
        dir.resize(prev + 1);
    }
}

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

HWND g_hWnd;           // 전역으로 쓸 window핸들
ULONG_PTR g_gdiplusToken; // 전역 GDI+ 토큰
static InputManager* g_inputManager = nullptr; // WndProc에서 사용할 InputManager 포인터

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    //메모리 누수 검사 플래그
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Resource/MapData 상대 경로가 맞도록 작업 디렉터리를 프로젝트 루트로 설정
    EnsureResourceWorkingDirectory();

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DONTSTARVE_CLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DONTSTARVE_CLIENT));

    MSG msg;
    msg.message = WM_NULL;

    // GDI+ 초기화
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

    DontStarve_MainGame* mainGame = new DontStarve_MainGame;
    mainGame->Init();
    mainGame->LateInit();

    // MainGame에서 생성된 InputManager를 WndProc에 전달
    g_inputManager = mainGame->GetInputManager();

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            mainGame->Update();
            mainGame->LateUpdate();
            mainGame->Render();
        }
    }

   if (mainGame) {
       mainGame->Release();
       Utils::SafeDelete(mainGame);
   }

   // Release 이후 WndProc이 해제된 InputManager에 접근하지 않도록 초기화
   g_inputManager = nullptr;

    // GDI+ 종료 (CRT 누수 덤프 이전에 호출하여 GDI+ 내부 블록 오탐 방지)
    GdiplusShutdown(g_gdiplusToken);

#ifdef _DEBUG
    //메모리릭 출력
    _CrtDumpMemoryLeaks();
#endif

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DONTSTARVE_CLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   RECT rc = { 0,0, WINCX, WINCY };
   AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
 
   if (!hWnd)
   {
       return FALSE;
   }

   g_hWnd = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // MainGame::Init()에서 설정된 g_inputManager를 사용 (싱글톤 직접 접근 제거)
    if (g_inputManager) {
        switch (message)
        {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
            g_inputManager->ProcessMouseMessage(message, wParam, lParam);
            break;
        case WM_KEYDOWN:
        case WM_KEYUP:
            g_inputManager->ProcessKeyMessage(message, wParam);
            break;
        }
    }

    switch (message)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            DestroyWindow(hWnd);
            break;
        }
       break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
