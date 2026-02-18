#pragma once

class Item;
class Player;

struct ItemSlot {
	Item* item;
	UINT count;

	ItemSlot() : item(nullptr), count(0) {}
	bool IsEmpty() const { return item == nullptr || count == 0; }
	void Clear();  // cpp 파일에서 구현 (Item의 완전한 정의 필요)
};

class Inventory {
private:
	std::vector<ItemSlot> m_slots;
	std::vector<Gdiplus::RectF> m_slotRects;
	const UINT ITEM_STACK_MAX = 99;

public:
	Inventory();
	~Inventory();

	void Init(std::vector<Gdiplus::RectF>& slotRects);

	bool AddItem(Item* itemDef, UINT count = 1);
	bool RemoveItem(UINT slotIndex, UINT count = 1);
	bool ConsumeItems(const std::map<UINT, UINT>& requiredItems);

	UINT GetItemCount(UINT itemId) const;
	bool CheckHasEnoughItems(const std::map<UINT, UINT>& requiredItems) const;

	const ItemSlot& GetSlot(int index) const;

	void Render(int equippedSlotIndex);

	void LoadUIBitmaps();
	void ReleaseUIBitmaps();

	bool HandleMouseClick(float mouseScreenX, float mouseScreenY, Player* player);
	bool HandleRightClick(float mouseScreenX, float mouseScreenY, Player* player);

private:
	int FindFirstEmptySlot() const;
	int FindExistingStack(UINT itemId) const;

	// UI 이미지 캐시
	Gdiplus::Bitmap* m_inventoryBgBitmap;
	Gdiplus::Bitmap* m_slotBgBitmap;
	Gdiplus::Font* m_font;
	Gdiplus::SolidBrush* m_solidBrush;
	Gdiplus::SolidBrush* m_shadowBrush;
	Gdiplus::StringFormat* m_stringFormat;
};

