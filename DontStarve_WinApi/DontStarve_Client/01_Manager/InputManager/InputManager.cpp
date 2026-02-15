#include "99_Default/pch.h"
#include "InputManager.h"
#include "../CameraManager/CameraManager.h"
#include "../UIManager/UIManager.h"

InputManager::InputManager() 
	: s_lButtonState(false), s_lButtonPrevState(false), s_mousePos({0,0})
	, s_lButtonClickedThisFrame(false), s_rButtonClickedThisFrame(false)
{
    memset(s_keyState, 0, sizeof(s_keyState));
    memset(s_keyPrevState, 0, sizeof(s_keyPrevState)); 
}

InputManager::~InputManager()
{
}

void InputManager::Init()
{
    ZeroMemory(s_keyState, sizeof(s_keyState));
    ZeroMemory(s_keyPrevState, sizeof(s_keyPrevState));
    s_mousePos = { 0, 0 };
    s_lButtonState = false;
    s_lButtonPrevState = false;
	s_lButtonClickedThisFrame = false;
	s_rButtonClickedThisFrame = false;
}

void InputManager::LateInit()
{
    
}

void InputManager::Update(float deltaTime)
{
    // 이전 프레임의 키 상태를 현재 프레임의 이전 상태로 복사
    memcpy(s_keyPrevState, s_keyState, sizeof(s_keyState));
    s_lButtonPrevState = s_lButtonState;
    s_rButtonPrevState = s_rButtonState;

    // Windows 메시지로 처리되지 않은 경우를 대비해 GetAsyncKeyState로 폴백
    for (int i = 0; i < 256; ++i) {
        s_keyState[i] = (GetAsyncKeyState(i) & 0x8000) != 0; 
    }

    // 마우스 버튼 상태 업데이트 (메시지로 처리되지 않은 경우 폴백)
    if (!s_lButtonClickedThisFrame) {
        s_lButtonState = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    }
    if (!s_rButtonClickedThisFrame) {
        s_rButtonState = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    }

    // 마우스 위치 업데이트 (메시지로 처리되지 않은 경우 폴백)
    if (g_hWnd) {
        GetCursorPos(&s_mousePos);
        ScreenToClient(g_hWnd, &s_mousePos);
    }
}


void InputManager::LateUpdate() 
{
	// 프레임 끝에 클릭 플래그 초기화 (다음 프레임을 위해)
	s_lButtonClickedThisFrame = false;
	s_rButtonClickedThisFrame = false;
}
void InputManager::Render() {}
void InputManager::Release() {}

bool InputManager::IsKeyDown(int vkCode) { return s_keyState[vkCode]; }
bool InputManager::IsKeyPressed(int vkCode) { return s_keyState[vkCode] && !s_keyPrevState[vkCode]; }
bool InputManager::IsKeyReleased(int vkCode) { return !s_keyState[vkCode] && s_keyPrevState[vkCode]; }
POINT InputManager::GetMousePos() { return s_mousePos; }
bool InputManager::IsLButtonDown() { return s_lButtonState; }
bool InputManager::IsLButtonClicked() 
{ 
	// Windows 메시지로 즉시 감지된 클릭이 있으면 우선 반환
	if (s_lButtonClickedThisFrame) {
		return true;
	}
	// 그렇지 않으면 기존 방식 (한 프레임 지연 가능)
	return s_lButtonState && !s_lButtonPrevState; 
}

bool InputManager::IsRButtonClicked() 
{ 
	// Windows 메시지로 즉시 감지된 클릭이 있으면 우선 반환
	if (s_rButtonClickedThisFrame) {
		return true;
	}
	// 그렇지 않으면 기존 방식 (한 프레임 지연 가능)
	return s_rButtonState && !s_rButtonPrevState;
}

Gdiplus::PointF InputManager::GetMouseClickScreenPos() const {
    return Gdiplus::PointF(static_cast<float>(s_mousePos.x), static_cast<float>(s_mousePos.y));
}

// 마우스 클릭한 월드 좌표
Gdiplus::PointF InputManager::GetMouseClickWorldPos() const {
    Gdiplus::PointF screenPos = GetMouseClickScreenPos();
    return CameraManager::GetInstance()->ScreenToWorld(screenPos.X, screenPos.Y);
}

// Windows 메시지 직접 처리 (즉시 반응을 위해)
void InputManager::ProcessMouseMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	UIManager* uiManager = UIManager::GetInstance();
	switch (message)
	{
	case WM_MOUSEMOVE:
		s_mousePos.x = LOWORD(lParam);
		s_mousePos.y = HIWORD(lParam);
				
		if (uiManager) {
			uiManager->UpdateButtonHoverStates();
		}
		break;
		
	case WM_LBUTTONDOWN:
		// 왼쪽 버튼 클릭 즉시 감지
		s_lButtonPrevState = s_lButtonState;
		s_lButtonState = true;
		s_lButtonClickedThisFrame = true;  // 이 프레임에 클릭 발생
		s_mousePos.x = LOWORD(lParam);
		s_mousePos.y = HIWORD(lParam);
		break;
		
	case WM_LBUTTONUP:
		// 왼쪽 버튼 떼기
		s_lButtonPrevState = s_lButtonState;
		s_lButtonState = false;
		s_mousePos.x = LOWORD(lParam);
		s_mousePos.y = HIWORD(lParam);
		break;
		
	case WM_RBUTTONDOWN:
		// 오른쪽 버튼 클릭 즉시 감지
		s_rButtonPrevState = s_rButtonState;
		s_rButtonState = true;
		s_rButtonClickedThisFrame = true;  // 이 프레임에 클릭 발생
		s_mousePos.x = LOWORD(lParam);
		s_mousePos.y = HIWORD(lParam);
		break;
		
	case WM_RBUTTONUP:
		// 오른쪽 버튼 떼기
		s_rButtonPrevState = s_rButtonState;
		s_rButtonState = false;
		s_mousePos.x = LOWORD(lParam);
		s_mousePos.y = HIWORD(lParam);
		break;
	}
}

void InputManager::ProcessKeyMessage(UINT message, WPARAM wParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		if (wParam >= 0 && wParam < 256) {
			s_keyPrevState[wParam] = s_keyState[wParam];
			s_keyState[wParam] = true;
		}
		break;
		
	case WM_KEYUP:
		if (wParam >= 0 && wParam < 256) {
			s_keyPrevState[wParam] = s_keyState[wParam];
			s_keyState[wParam] = false;
		}
		break;
	}
}
