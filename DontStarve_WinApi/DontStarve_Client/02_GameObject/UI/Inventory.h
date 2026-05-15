#pragma once

class Sprite;
class Item;
class Player;
class UIButton;
class UIImage;
class UIText;

struct ItemSlot {
	GameObjectID id;
	UINT count;
	std::wstring countStr; // 렌더링용 캐싱 문자열
	std::shared_ptr<Sprite> cachedSprite; // 렌더링용 캐싱 스프라이트 (수명 보장)

	ItemSlot() : id(GOID_NONE), count(0), countStr(L""), cachedSprite(nullptr) {}
	bool IsEmpty() const { return id == GOID_NONE || count == 0; }
	void Clear();
	void UpdateCount(UINT newCount) {
		count = newCount;
		countStr = (count > 1) ? std::to_wstring(count) : L"";
	}
};

class Inventory {
private:
	Player* m_player;  
	std::vector<ItemSlot> m_slots;
	const UINT  ITEM_STACK_MAX = 99;

	// 슬롯 배치 상수
	const float SLOT_WIDTH;
	const float SLOT_HEIGHT;
	const float SLOT_PADDING;
	const float SLOT_STRIDE;   // SLOT_WIDTH + SLOT_PADDING

	// 인벤토리 배경 및 슬롯 리소스
	std::shared_ptr<Sprite> m_bgSprite;
	std::shared_ptr<Sprite> m_slotSprite;

	// 인벤토리 위치/크기 (Init에서 계산)
	float m_bgX, m_bgY;
	float m_bgW, m_bgH;
	float m_slotStartX, m_slotStartY;

	// 텍스트 렌더링용 폰트/브러시
	Gdiplus::Font* m_font;
	Gdiplus::SolidBrush* m_textBrush;
	Gdiplus::StringFormat* m_stringFormat;

public:
	Inventory(Player* owner);
	~Inventory();

	void Init();
	void Update(float deltaTime);
	void Render(int equippedSlotIndex);

	bool ContainsScreenPoint(float screenX, float screenY) const;
	bool HandleRightClick(float mouseScreenX, float mouseScreenY, Player* player);

	// 인벤토리 아이템 조작 인터페이스

	bool AddItem(GameObjectID itemID, UINT count = 1);
	bool RemoveItem(UINT slotIndex, UINT count = 1);
	bool ConsumeItems(const std::map<UINT, UINT>& requiredItems);
	
	// 인벤토리 아이템 조회 인터페이스
	UINT GetItemCount(UINT itemId) const;
	UINT GetAvailableSpace(UINT itemId) const;
	const ItemSlot& GetSlot(int index) const;
	bool CheckHasEnoughItems(const std::map<UINT, UINT>& requiredItems) const;

	// 상태 저장/복원용 메서드
	void ClearAllItems();
	std::vector<std::pair<GameObjectID, UINT>> GetAllItemsSnapshot() const;

private:
	int  FindFirstEmptySlot() const;
	int  FindExistingStack(UINT itemId) const;
	void HandleSlotClick(int slotIndex, Player* player);
	void UpdateSlotButton(int slotIndex);

	Gdiplus::RectF GetBgRect() const;
};
