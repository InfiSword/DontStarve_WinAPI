#pragma once

class UIElement;
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

	// UI 오브젝트 관리 (모두 UIElement로 통합 보관)
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

	// 해당 스크린 좌표가 활성화된 UI 위에 있으면 true (클릭 차단 판정)
	bool IsScreenPointBlockedByUI(float screenX, float screenY) const;

	// UI 블록에서 제외할 영역 등록/해제 (인벤토리 등 자체 처리하는 UI)
	void RegisterBlockRegion(const Gdiplus::RectF& rect);
	void ClearBlockRegions();

private:
	std::vector<UIElement*> m_uiElements;
	std::vector<Gdiplus::RectF> m_blockRegions;
	bool m_isUIVisible;
};
