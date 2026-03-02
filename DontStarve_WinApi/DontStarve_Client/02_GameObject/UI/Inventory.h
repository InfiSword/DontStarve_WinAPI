#pragma once

class Sprite;
class Item;
class Player;
class UIButton;
class UIImage;
class UIText;

struct ItemSlot {
	Item* item;
	UINT count;

	ItemSlot() : item(nullptr), count(0) {}
	bool IsEmpty() const { return item == nullptr || count == 0; }
	void Clear();
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

	// 인벤토리 배경 UIImage
	UIImage* m_bgImage;

	// 슬롯당 UIButton (슬롯 배경) + UIImage (아이템 이미지) + UIText (개수 텍스트)
	std::vector<UIButton*> m_slotButtons;
	std::vector<UIImage*>  m_slotItemImages;
	std::vector<UIText*>   m_slotCountTexts;

public:
	Inventory(Player* owner);
	~Inventory();

	void Init();

	bool AddItem(Item* itemDef, UINT count = 1);
	bool RemoveItem(UINT slotIndex, UINT count = 1);
	bool ConsumeItems(const std::map<UINT, UINT>& requiredItems);

	UINT GetItemCount(UINT itemId) const;
	bool CheckHasEnoughItems(const std::map<UINT, UINT>& requiredItems) const;

	const ItemSlot& GetSlot(int index) const;
	Item* GetItem(int index) const {
		if (index < 0 || index >= (int)m_slots.size()) return nullptr;
		return m_slots[index].item;
	}

	void Render(int equippedSlotIndex);

	bool ContainsScreenPoint(float screenX, float screenY) const;
	bool HandleRightClick(float mouseScreenX, float mouseScreenY, Player* player);

private:
	int  FindFirstEmptySlot() const;
	int  FindExistingStack(UINT itemId) const;
	void HandleSlotClick(int slotIndex, Player* player);
	void UpdateSlotButton(int slotIndex);

	Gdiplus::RectF GetBgRect() const;
};
