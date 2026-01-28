#pragma once
#include <vector>
#include <memory>

class UIImage;
class UIButton;
class UIText;

class UIManager : public CSingleTon<UIManager>
{
	friend class CSingleTon<UIManager>;
public:
	UIManager();
	~UIManager();

	void Init();
	void LateInit();
	void Update(float deltaTime);
	void LateUpdate();
	void Render();
	void Release();

	// UI 오브젝트 관리
	void AddUIImage(UIImage* image);
	void AddUIButton(UIButton* button);
	void AddUIText(UIText* text);
	void RemoveUIImage(UIImage* image);
	void RemoveUIButton(UIButton* button);
	void RemoveUIText(UIText* text);
	void ClearAllUI();

	// 특정 UI 오브젝트 찾기
	UIImage* FindUIImage(GameObjectID id);
	UIButton* FindUIButton(GameObjectID id);
	UIText* FindUIText(GameObjectID id);

	// UI 상태 관리
	void SetUIVisibility(bool visible);
	bool IsUIVisible() const { return m_isUIVisible; }

private:
	std::vector<UIImage*> m_uiImages;
	std::vector<UIButton*> m_uiButtons;
	std::vector<UIText*> m_uiTexts;
	bool m_isUIVisible;
}; 
