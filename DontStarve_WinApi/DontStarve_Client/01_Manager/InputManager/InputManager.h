#pragma once

class InputManager : public CSingleTon<InputManager>
{
	friend class CSingleTon<InputManager>;
public:
	InputManager();
	~InputManager();

public:
	void Init();
	void LateInit();
	void Update(float deltaTime);
	void LateUpdate();
	void Render();
	void Release();

	bool IsKeyDown(int vkCode);
	// 특정 키가 현재 눌려있는지
	bool IsKeyPressed(int vkCode);
	// 특정 키가 이번 프레임에 눌렸는지
	bool IsKeyReleased(int vkCode);
	// 특정 키가 이번 프레임에 떼어진지
	POINT GetMousePos();
	// 현재 마우스 커서의 화면 좌표 
	bool IsLButtonDown();
	// 마우스 왼쪽 버튼이 현재 눌려있는지
	bool IsLButtonClicked();
	// 마우스 왼쪽 버튼이 클릭되었는지
	bool IsRButtonClicked();

	// Windows 메시지 직접 처리 (즉시 반응을 위한 함수)
	void ProcessMouseMessage(UINT message, WPARAM wParam, LPARAM lParam);
	void ProcessKeyMessage(UINT message, WPARAM wParam);

	Gdiplus::PointF GetMouseClickWorldPos() const; 
	Gdiplus::PointF GetMouseClickScreenPos() const;

private:
	bool s_keyState[256];
	bool s_keyPrevState[256];
	POINT s_mousePos;
	bool s_lButtonState;
	bool s_lButtonPrevState;
	bool s_rButtonState;     
	bool s_rButtonPrevState;
	
	// 클릭 즉시 감지를 위한 플래그 (Windows 메시지에서 설정)
	bool s_lButtonClickedThisFrame;
	bool s_rButtonClickedThisFrame; 
};
