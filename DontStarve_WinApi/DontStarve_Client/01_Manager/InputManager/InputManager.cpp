#include "../../99_Default/pch.h"
#include "InputManager.h"
#include "../CameraManager/CameraManager.h"

InputManager::InputManager() : s_lButtonState(false), s_lButtonPrevState(false), s_mousePos({0,0})
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

    for (int i = 0; i < 256; ++i) {
        s_keyState[i] = (GetAsyncKeyState(i) & 0x8000) != 0; 
    }

    s_lButtonState = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    s_rButtonState = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0; 

    GetCursorPos(&s_mousePos);
    if (g_hWnd) { 
        ScreenToClient(g_hWnd, &s_mousePos);
    }
}


void InputManager::LateUpdate() {}
void InputManager::Render() {}
void InputManager::Release() {}

bool InputManager::IsKeyDown(int vkCode) { return s_keyState[vkCode]; }
bool InputManager::IsKeyPressed(int vkCode) { return s_keyState[vkCode] && !s_keyPrevState[vkCode]; }
bool InputManager::IsKeyReleased(int vkCode) { return !s_keyState[vkCode] && s_keyPrevState[vkCode]; }
POINT InputManager::GetMousePos() { return s_mousePos; }
bool InputManager::IsLButtonDown() { return s_lButtonState; }
bool InputManager::IsLButtonClicked() { return s_lButtonState && !s_lButtonPrevState; }

bool InputManager::IsRButtonClicked() 
{ 
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
