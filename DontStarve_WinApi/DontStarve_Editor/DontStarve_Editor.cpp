// DontStarve_Editor.cpp : 애플리케이션에 대한 진입점을 정의합니다.

#include "pch.h"
#include "framework.h"
#include "DontStarve_Editor.h"
#include "00_MainEditor/IEditorScreen.h"
#include "00_MainEditor/EditorLauncher.h"
#include "00_MainEditor/MapEditor.h"
#include "00_MainEditor/ObjectEditor.h"
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

#define MAX_LOADSTRING 100

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

HWND g_hWnd;
ULONG_PTR g_gdiplusToken;

// 현재 화면 (런처 / 맵 에디터 / 오브젝트 에디터). 메뉴 IDM_* 는 해당 에디터 포인터 사용.
IEditorScreen* g_currentScreen = nullptr;
MapEditor* mapEditor = nullptr;      // g_currentScreen이 MapEditor일 때만 유효
ObjectEditor* objectEditor = nullptr; // g_currentScreen이 ObjectEditor일 때만 유효

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Resource·GameData 등 상대 경로가 맞도록 작업 디렉터리를 프로젝트 루트로 설정
    EnsureResourceWorkingDirectory();

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DONTSTARVEEDITOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DONTSTARVEEDITOR));

    MSG msg;
    msg.message = WM_NULL;

    // GDI+ 초기화
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

    // 초기 화면: 런처 (MapEditor / ObjectEditor 버튼)
    g_currentScreen = new(std::nothrow) EditorLauncher();
    if (!g_currentScreen) {
        MessageBox(nullptr, L"메모리 부족으로 런처를 초기화할 수 없습니다.", L"초기화 오류", MB_OK | MB_ICONERROR);
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
        return FALSE;
    }
    mapEditor = nullptr;
    try {
        g_currentScreen->Initialize();
    }
    catch (...) {
        MessageBox(nullptr, L"런처 초기화 중 오류가 발생했습니다.", L"초기화 오류", MB_OK | MB_ICONERROR);
        if (g_currentScreen) { g_currentScreen->Release(); delete g_currentScreen; g_currentScreen = nullptr; }
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
        return FALSE;
    }
    SetWindowText(g_hWnd, L"DontStarve Editor - Select Mode");

    // 성능 상수 정의
    constexpr ULONGLONG TARGET_FRAME_TIME = 16;     // 60 FPS (1000ms / 60)
    constexpr ULONGLONG INFO_UPDATE_INTERVAL = 1000; // 1초마다 FPS/제목 업데이트
    constexpr ULONGLONG CPU_YIELD_TIME = 1;          // CPU 점유율 최소화 (1ms)
    constexpr int WINDOW_TITLE_MAX_LENGTH = 128;     // 창 제목 최대 길이

    ULONGLONG lastFrameTime = GetTickCount64();
    ULONGLONG lastInfoUpdateTime = lastFrameTime;
    ULONGLONG currentTime = 0;

    TCHAR windowTitle[WINDOW_TITLE_MAX_LENGTH] = {};
    int frameCount = 0;
    float currentFPS = 0.0f;

    // 최적화된 메시지 루프
    while (msg.message != WM_QUIT)
    {
        // 메시지 처리
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            continue; // 메시지 처리 후 다음 루프로
        }

        // 현재 시간 (한 번만 호출)
        currentTime = GetTickCount64();

        // 프레임 제한 체크 (60 FPS)
        if (currentTime - lastFrameTime >= TARGET_FRAME_TIME)
        {
            lastFrameTime = currentTime;
            frameCount++;

            if (g_currentScreen) {
                g_currentScreen->Update();
                g_currentScreen->Render();
            }

            // 화면 전환 요청 처리 (런처 버튼 클릭 / Back to main)
            EditorScreenSwitch sw = g_currentScreen ? g_currentScreen->GetRequestedSwitch() : EditorScreenSwitch::None;
            if (sw != EditorScreenSwitch::None) {
                if (g_currentScreen) {
                    g_currentScreen->Release();
                    delete g_currentScreen;
                    g_currentScreen = nullptr;
                }
                mapEditor = nullptr;
                objectEditor = nullptr;
                if (sw == EditorScreenSwitch::MapEditor) {
                    g_currentScreen = new(std::nothrow) MapEditor();
                    if (g_currentScreen) {
                        try {
                            g_currentScreen->Initialize();
                            mapEditor = static_cast<MapEditor*>(g_currentScreen);
                        }
                        catch (...) {
                            g_currentScreen->Release();
                            delete g_currentScreen;
                            g_currentScreen = nullptr;
                        }
                    }
                }
                else if (sw == EditorScreenSwitch::ObjectEditor) {
                    g_currentScreen = new(std::nothrow) ObjectEditor();
                    if (g_currentScreen) {
                        try {
                            g_currentScreen->Initialize();
                            objectEditor = static_cast<ObjectEditor*>(g_currentScreen);
                        }
                        catch (...) {
                            g_currentScreen->Release();
                            delete g_currentScreen;
                            g_currentScreen = nullptr;
                        }
                    }
                }
                else if (sw == EditorScreenSwitch::BackToLauncher) {
                    g_currentScreen = new(std::nothrow) EditorLauncher();
                    if (g_currentScreen) {
                        try { g_currentScreen->Initialize(); }
                        catch (...) {
                            g_currentScreen->Release();
                            delete g_currentScreen;
                            g_currentScreen = nullptr;
                        }
                    }
                    SetWindowText(g_hWnd, L"DontStarve Editor - Select Mode");
                }
            }

            // FPS/정보 업데이트 (1초마다)
            if (currentTime - lastInfoUpdateTime >= INFO_UPDATE_INTERVAL && g_currentScreen)
            {
                lastInfoUpdateTime = currentTime;
                currentFPS = (float)frameCount / (INFO_UPDATE_INTERVAL / 1000.0f);
                frameCount = 0;
                g_currentScreen->SetCurrentFPS(currentFPS);
                float memoryMB = g_currentScreen->GetLayerMemoryUsageMB();
                if (memoryMB >= 0 && currentFPS >= 0)
                    swprintf_s(windowTitle, L"DontStarve Editor - FPS: %.1f | Memory: %.1fMB", currentFPS, memoryMB);
                else if (mapEditor)
                    swprintf_s(windowTitle, L"DontStarve Editor - Map Editor");
                else
                    swprintf_s(windowTitle, L"DontStarve Editor - Object Editor");
                SetWindowText(g_hWnd, windowTitle);
            }
        }
        else
        {
            // CPU 점유율 최소화
            Sleep(CPU_YIELD_TIME);
        }
    }

    if (g_currentScreen) {
        g_currentScreen->Release();
        delete g_currentScreen;
        g_currentScreen = nullptr;
    }
    mapEditor = nullptr;
    objectEditor = nullptr;
    Gdiplus::GdiplusShutdown(g_gdiplusToken);     // GDI+ 종료
    return (int)msg.wParam;
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

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DONTSTARVEEDITOR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DONTSTARVEEDITOR);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // WM_PAINT 메시지는 EditMain의 Render()에서 처리하므로 여기서 직접 그리지 않습니다.
    // WM_DESTROY 및 WM_KEYDOWN (Escape)은 여기서 처리합니다.
    // 그 외 모든 메시지는 g_currentScreen으로 전달하여 처리합니다.
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        // Handle menu commands
        switch (wmId)
        {
        case IDM_NEW:
        {
            if (mapEditor) {
                int result = MessageBox(hWnd,
                    L"현재 작업 중인 맵이 초기화됩니다.\n저장하지 않은 변경사항은 손실됩니다.\n계속하시겠습니까?",
                    L"새 맵", MB_YESNO | MB_ICONQUESTION);

                if (result == IDYES) {
                    mapEditor->NewMap();
                    MessageBox(hWnd, L"새 맵이 생성되었습니다.\n플레이어 스폰 포인트가 맵 중앙으로 설정되었습니다.", L"새 맵 완료", MB_OK | MB_ICONINFORMATION);
                    InvalidateRect(hWnd, NULL, FALSE);
                }
            }
        }
        break;

        case IDM_SAVE:
        {
            if (objectEditor) {
                if (objectEditor->SaveObjects()) {
                    InvalidateRect(hWnd, NULL, FALSE);
                }
                else {
                    MessageBox(hWnd, L"오브젝트 리소스 저장에 실패했습니다.", L"저장 오류", MB_OK | MB_ICONERROR);
                }
            }
            else if (mapEditor) {
                WCHAR fileName[MAX_PATH] = {};
                if (mapEditor->ShowSaveFileDialog(fileName, MAX_PATH)) {
                    if (mapEditor->SaveMap(fileName)) {
                        MessageBox(hWnd, L"맵이 성공적으로 저장되었습니다.", L"저장 완료", MB_OK | MB_ICONINFORMATION);
                    }
                    else {
                        MessageBox(hWnd, L"맵 저장에 실패했습니다.", L"저장 오류", MB_OK | MB_ICONERROR);
                    }
                }
            }
        }
        break;

        case IDM_LOAD:
        {
            if (mapEditor) {
                WCHAR fileName[MAX_PATH] = {};
                if (mapEditor->ShowOpenFileDialog(fileName, MAX_PATH)) {
                    if (mapEditor->LoadMap(fileName)) {
                        MessageBox(hWnd, L"맵이 성공적으로 불러와졌습니다.", L"불러오기 완료", MB_OK | MB_ICONINFORMATION);
                        InvalidateRect(hWnd, NULL, FALSE);
                    }
                    else {
                        MessageBox(hWnd, L"맵 불러오기에 실패했습니다.", L"불러오기 오류", MB_OK | MB_ICONERROR);
                    }
                }
            }
        }
        break;

        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            if (g_currentScreen) {
                LRESULT r = g_currentScreen->HandleMessage(hWnd, message, wParam, lParam);
                if (r == 0) return 0;
            }
            DestroyWindow(hWnd);
        }
        // Ctrl 조합 단축키 처리
        else if (GetKeyState(VK_CONTROL) & 0x8000)
        {
            // Ctrl+N: 새 맵
            if (wParam == 'N' || wParam == 'n')
            {
                SendMessage(hWnd, WM_COMMAND, IDM_NEW, 0);
                return 0;
            }
            // Ctrl+O: 맵 불러오기
            else if (wParam == 'O' || wParam == 'o')
            {
                SendMessage(hWnd, WM_COMMAND, IDM_LOAD, 0);
                return 0;
            }
            // Ctrl+S: 맵 저장
            else if (wParam == 'S' || wParam == 's')
            {
                SendMessage(hWnd, WM_COMMAND, IDM_SAVE, 0);
                return 0;
            }
            // Ctrl+M: 맵 크기 설정 (MapEditor만)
            else if (wParam == 'M' || wParam == 'm')
            {
                if (mapEditor) {
                    mapEditor->ShowMapSizeDialog(hWnd);
                    InvalidateRect(hWnd, NULL, FALSE);
                }
                return 0;
            }
        }
        // F1~F12: 현재 화면으로 전달
        else if (wParam >= VK_F1 && wParam <= VK_F12)
        {
            if (g_currentScreen != nullptr) {
                return g_currentScreen->HandleMessage(hWnd, message, wParam, lParam);
            }
        }
    default:
        if (g_currentScreen != nullptr) {
            return g_currentScreen->HandleMessage(hWnd, message, wParam, lParam);
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}