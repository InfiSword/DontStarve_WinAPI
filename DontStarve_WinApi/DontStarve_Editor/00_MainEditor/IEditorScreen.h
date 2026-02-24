#pragma once

#include <windows.h>

// 에디터 화면 종류 (런처 / 맵 에디터 / 오브젝트 에디터)
enum class EditorScreenType {
	Launcher,
	MapEditor,
	ObjectEditor
};

// 전환 요청 (None = 전환 없음, BackToLauncher = 에디터에서 메인으로)
enum class EditorScreenSwitch {
	None,
	MapEditor,
	ObjectEditor,
	BackToLauncher
};

// 공용 에디터 화면 인터페이스 (런처, MapEditor, ObjectEditor)
class IEditorScreen {
public:
	virtual ~IEditorScreen() = default;
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void Release() = 0;
	virtual LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;

	// 런처에서 버튼 클릭 시 전환 요청 (호출 후 None으로 리셋됨)
	virtual EditorScreenSwitch GetRequestedSwitch() { return EditorScreenSwitch::None; }

	// 창 제목용: FPS (없으면 -1), 메모리 MB (없으면 -1)
	virtual float GetCurrentFPS() const { return -1.0f; }
	virtual float GetLayerMemoryUsageMB() const { return -1.0f; }
	virtual void SetCurrentFPS(float) {}
};
