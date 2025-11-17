// DontStarve_Editor.cpp : 애플리케이션에 대한 진입점을 정의합니다.

#include "pch.h"
#include "framework.h"
#include "DontStarve_Editor.h"
#include "00_MainEditor/DontStarve_EditorMain.h"
#include "Resource.h"
#include <commdlg.h>

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

HWND g_hWnd;
ULONG_PTR g_gdiplusToken;

DontStarve_EditorMain* mainEditor = nullptr;

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

    // TODO: 여기에 코드를 입력합니다.

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

    // EditMain 초기화 (에러 처리 포함)
    mainEditor = new(std::nothrow) DontStarve_EditorMain();
    if (!mainEditor) {
        MessageBox(nullptr, L"메모리 부족으로 에디터를 초기화할 수 없습니다.", L"초기화 오류", MB_OK | MB_ICONERROR);
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
        return FALSE;
    }

    try {
        mainEditor->Initialize();
    }
    catch (...) {
        MessageBox(nullptr, L"에디터 초기화 중 오류가 발생했습니다.", L"초기화 오류", MB_OK | MB_ICONERROR);
        SafeDelete(mainEditor);
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
        return FALSE;
    }

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

            // 게임 업데이트 및 렌더링
            mainEditor->Update();
            mainEditor->Render();

            // FPS/정보 업데이트 (1초마다)
            if (currentTime - lastInfoUpdateTime >= INFO_UPDATE_INTERVAL)
            {
                lastInfoUpdateTime = currentTime;

                // FPS 계산
                currentFPS = (float)frameCount / (INFO_UPDATE_INTERVAL / 1000.0f);
                frameCount = 0;

                // EditMain에 FPS 전달
                mainEditor->SetCurrentFPS(currentFPS);

                // 창 제목 업데이트 (메모리 정보 포함)
                float memoryMB = mainEditor->GetLayerMemoryUsageMB();
                swprintf_s(windowTitle, L"DontStarve Editor - FPS: %.1f | Memory: %.1fMB",
                    currentFPS, memoryMB);
                SetWindowText(g_hWnd, windowTitle);
            }
        }
        else
        {
            // CPU 점유율 최소화
            Sleep(CPU_YIELD_TIME);
        }
    }

    if (mainEditor) {
        mainEditor->Release();
        SafeDelete(mainEditor);
    }
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
    // 그 외 모든 메시지는 g_mainEditor로 전달하여 처리합니다.
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
            if (mainEditor) {
                int result = MessageBox(hWnd,
                    L"현재 작업 중인 맵이 초기화됩니다.\n저장하지 않은 변경사항은 손실됩니다.\n계속하시겠습니까?",
                    L"새 맵", MB_YESNO | MB_ICONQUESTION);

                if (result == IDYES) {
                    mainEditor->NewMap();
                    MessageBox(hWnd, L"새 맵이 생성되었습니다.\n플레이어 스폰 포인트가 맵 중앙으로 설정되었습니다.", L"새 맵 완료", MB_OK | MB_ICONINFORMATION);
                    InvalidateRect(hWnd, NULL, FALSE); // 화면 갱신
                }
            }
        }
        break;

        case IDM_SAVE:
        {
            if (mainEditor) {
                WCHAR fileName[MAX_PATH] = {};
                if (mainEditor->ShowSaveFileDialog(fileName, MAX_PATH)) {
                    if (mainEditor->SaveMap(fileName)) {
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
            if (mainEditor) {
                WCHAR fileName[MAX_PATH] = {};
                if (mainEditor->ShowOpenFileDialog(fileName, MAX_PATH)) {
                    if (mainEditor->LoadMap(fileName)) {
                        MessageBox(hWnd, L"맵이 성공적으로 불러와졌습니다.", L"불러오기 완료", MB_OK | MB_ICONINFORMATION);
                        InvalidateRect(hWnd, NULL, FALSE); // 화면 갱신
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
            DestroyWindow(hWnd);
        }
        // F1~F12 등 특수 키는 EditMain으로 전달
        else if (wParam >= VK_F1 && wParam <= VK_F12)
        {
            if (mainEditor != nullptr) {
                return mainEditor->HandleMessage(hWnd, message, wParam, lParam);
            }
        }
    default:
        if (mainEditor != nullptr) {
            // EditMain의 메시지 처리 함수로 메시지 전달
            return mainEditor->HandleMessage(hWnd, message, wParam, lParam);
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