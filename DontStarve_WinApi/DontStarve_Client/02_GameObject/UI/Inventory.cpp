#include "99_Default/pch.h"
#include "Inventory.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UIText.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../01_Manager/InventoryManager/InventoryManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"
#include "../../02_GameObject/Item/Item.h"
#include "../../02_GameObject/Component/Sprite/SpriteRenderer.h"
#include "../../02_GameObject/Component/Transform/RectTransform.h"

// 슬롯 소유의 Item을 해제하고 비움 (count==0일 때 RemoveItem/ConsumeItems에서 호출)
void ItemSlot::Clear() {
	if (item) { delete item; item = nullptr; }
	count = 0;
}

Inventory::Inventory(Player* owner)
	:m_player(owner)
	, m_slots(INVENTORY_SLOT_COUNT)
	, SLOT_WIDTH(64.0f)
	, SLOT_HEIGHT(64.0f)
	, SLOT_PADDING(10.0f)
	, SLOT_STRIDE(64.0f + 10.0f)
	, m_bgImage(nullptr)
{
}

Inventory::~Inventory() {
	for (auto& slot : m_slots) {
		if (slot.item) { delete slot.item; slot.item = nullptr; }
	}

	if (m_bgImage) { m_bgImage->Release(); delete m_bgImage; m_bgImage = nullptr; }
	for (UIButton* btn : m_slotButtons) { if (btn) { btn->Release(); delete btn; } }
	for (UIImage* img : m_slotItemImages) { if (img) { img->Release(); delete img; } }
	for (UIText* txt : m_slotCountTexts) { if (txt) { txt->Release(); delete txt; } }
	m_slotButtons.clear();
	m_slotItemImages.clear();
	m_slotCountTexts.clear();
}

void Inventory::Init()
{
	// ── 인벤토리 배경 UIImage ────────────────────────────────────────────
	{
		auto bgSprite = ResourceManager::GetInstance()->LoadSprite(L"Resource\\UI\\Inven.png");
		float bgW = static_cast<float>(bgSprite->bitmap->GetWidth());
		float bgH = static_cast<float>(bgSprite->bitmap->GetHeight());

		float bgCx = WINCX * 0.5f;
		float bgCy = WINCY - bgH * 0.5f - 5.0f;

		m_bgImage = new UIImage(
			GOID_NONE, bgW, bgH,
			LAYER_UI_BACKGROUND, L"Resource\\UI\\Inven.png", 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, bgCx, bgCy
		);
		m_bgImage->Init();
	}

	// ── 슬롯 UIButton / UIImage / UIText 생성 ───────────────────────────
	float totalSlotsWidth = SLOT_STRIDE * INVENTORY_SLOT_COUNT - SLOT_PADDING;
	float startX = WINCX * 0.5f - totalSlotsWidth * 0.5f;
	float startY = WINCY - SLOT_HEIGHT - 30.0f;

	std::shared_ptr<Sprite> slotBgSprite =
		ResourceManager::GetInstance()->LoadSprite(L"Resource\\UI\\slot.png");

	m_slotButtons.resize(INVENTORY_SLOT_COUNT, nullptr);
	m_slotItemImages.resize(INVENTORY_SLOT_COUNT, nullptr);
	m_slotCountTexts.resize(INVENTORY_SLOT_COUNT, nullptr);

	for (int i = 0; i < INVENTORY_SLOT_COUNT; ++i) {
		float cx = startX + i * SLOT_STRIDE + SLOT_WIDTH * 0.5f;
		float cy = startY + SLOT_HEIGHT * 0.5f;

		UIButton* btn = new UIButton(
			GOID_NONE, SLOT_WIDTH, SLOT_HEIGHT,
			slotBgSprite, slotBgSprite,
			0.0f, 0.0f, 0.0f, 0.0f, cx, cy
		);
		btn->SetHoverColor(Gdiplus::Color(255, 255, 255, 255));
		btn->SetClickedColor(Gdiplus::Color(255, 255, 255, 255));
		btn->Init();
		m_slotButtons[i] = btn;

		UIImage* img = new UIImage(
			GOID_NONE, SLOT_WIDTH, SLOT_HEIGHT,
			LAYER_UI_FOREGROUND, L"",
			2.0f + (float)i * 0.001f,
			0.0f, 0.0f, 0.0f, 0.0f, cx, cy
		);
		img->Init();
		img->SetActive(false);
		m_slotItemImages[i] = img;

		float textX = cx - SLOT_WIDTH * 0.5f;
		float textY = cy - SLOT_HEIGHT * 0.5f;
		UIText* txt = new UIText(
			GOID_NONE, SLOT_WIDTH, SLOT_HEIGHT,
			L"", Gdiplus::Color(255, 255, 255, 255),
			LAYER_UI_FOREGROUND,
			3.0f + (float)i * 0.001f,
			L"Arial", 18.0f,
			Gdiplus::StringAlignmentFar, Gdiplus::StringAlignmentFar,
			0.0f, 0.0f, 0.0f, 0.0f, textX, textY
		);
		txt->Init();
		txt->SetActive(false);
		m_slotCountTexts[i] = txt;
	}
}

// ── 슬롯 이미지 상태 동기화 ─────────────────────────────────────────────
void Inventory::UpdateSlotButton(int slotIndex)
{
	if (slotIndex < 0 || slotIndex >= INVENTORY_SLOT_COUNT) return;

	UIImage* img = m_slotItemImages[slotIndex];
	UIText* txt = m_slotCountTexts[slotIndex];

	const ItemSlot& slot = m_slots[slotIndex];
	if (slot.IsEmpty()) {
		if (img) img->SetActive(false);
		if (txt) txt->SetActive(false);
		return;
	}

	// ── 아이템 이미지 갱신 ──────────────────────────────────────
	SpriteRenderer* sr = slot.item->GetComponent<SpriteRenderer>();
	std::shared_ptr<Sprite> spriteHandle = sr ? sr->GetSpriteHandle() : nullptr;
	if (spriteHandle) {
		float bw = spriteHandle->sourceRect.Width;
		float bh = spriteHandle->sourceRect.Height;
		if (bw > 0.0f && bh > 0.0f) {
			float scale = (std::min)(SLOT_WIDTH / bw, SLOT_HEIGHT / bh);
			RectTransform* rt = img->GetRectTransform();
			rt->SetScale(scale, scale);
		}
		img->SetSprite(spriteHandle);
		img->SetActive(true);
	}
	else {
		img->SetActive(false);
	}


	// ── 개수 텍스트 갱신 ────────────────────────────────────────
	if (slot.count >= 1) {
		wchar_t countStr[16];
		swprintf_s(countStr, 16, L"%u", slot.count);
		txt->SetText(countStr);
		txt->SetActive(true);
	}
	else {
		txt->SetActive(false);
	}

}

// ── 아이템 추가 ─────────────────────────────────────────────────────────
bool Inventory::AddItem(Item* itemDef, UINT count) {
	if (!itemDef) return false;

	int existingSlotIndex = FindExistingStack(itemDef->GetID());
	if (existingSlotIndex != -1 && m_slots[existingSlotIndex].count < ITEM_STACK_MAX) {
		UINT canAdd = ITEM_STACK_MAX - m_slots[existingSlotIndex].count;
		UINT actualAdd = min(count, canAdd);
		m_slots[existingSlotIndex].count += actualAdd;
		count -= actualAdd;
		UpdateSlotButton(existingSlotIndex);
		if (count == 0) { delete itemDef; return true; }
	}

	int emptySlotIndex = FindFirstEmptySlot();
	if (emptySlotIndex == -1) return false;

	m_slots[emptySlotIndex].item = itemDef;
	m_slots[emptySlotIndex].count = min(count, ITEM_STACK_MAX);

	UpdateSlotButton(emptySlotIndex);
	return true;
}

// ── 아이템 제거 ─────────────────────────────────────────────────────────
bool Inventory::RemoveItem(UINT slotIndex, UINT count) {
	if (slotIndex >= INVENTORY_SLOT_COUNT || m_slots[slotIndex].IsEmpty() || m_slots[slotIndex].count < count)
		return false;

	m_slots[slotIndex].count -= count;
	if (m_slots[slotIndex].count == 0) m_slots[slotIndex].Clear();

	UpdateSlotButton((int)slotIndex);
	return true;
}

// ── 아이템 소모 ─────────────────────────────────────────────────────────
bool Inventory::ConsumeItems(const std::map<UINT, UINT>& requiredItems) {
	if (!CheckHasEnoughItems(requiredItems)) return false;

	std::map<UINT, UINT> tempRequired = requiredItems;
	for (int i = INVENTORY_SLOT_COUNT - 1; i >= 0; --i) {
		ItemSlot& slot = m_slots[i];
		if (slot.IsEmpty() || tempRequired.count(slot.item->GetID()) == 0) continue;

		UINT& neededCount = tempRequired.at(slot.item->GetID());
		if (neededCount > 0) {
			UINT consume = min(slot.count, neededCount);
			slot.count -= consume;
			neededCount -= consume;
			if (slot.count == 0) slot.Clear();
			UpdateSlotButton(i);
		}
	}
	return true;
}

void Inventory::ClearAllItems()
{
	for (auto& slot : m_slots) {
		slot.Clear();
	}
	for (int i = 0; i < INVENTORY_SLOT_COUNT; ++i) {
		UpdateSlotButton(i);
	}
}

bool Inventory::AddItemByID(GameObjectID itemID, UINT count)
{
	GameObject* itemObj = ObjectManager::GetInstance()->CreateGameObject(itemID, 0.0f, 0.0f, nullptr, false);
	Item* item = dynamic_cast<Item*>(itemObj);
	if (!item) {
		if (itemObj) delete itemObj;
		return false;
	}
	bool result = AddItem(item, count);
	if (!result) delete item;
	return result;
}

std::vector<std::pair<GameObjectID, UINT>> Inventory::GetAllItemsSnapshot() const
{
	std::vector<std::pair<GameObjectID, UINT>> result;
	for (const ItemSlot& slot : m_slots) {
		if (!slot.IsEmpty()) {
			result.emplace_back(slot.item->GetID(), slot.count);
		}
	}
	return result;
}

UINT Inventory::GetItemCount(UINT itemId) const {
	UINT total = 0;
	for (const auto& slot : m_slots)
		if (!slot.IsEmpty() && slot.item->GetID() == itemId)
			total += slot.count;
	return total;
}

const ItemSlot& Inventory::GetSlot(int index) const {
	static ItemSlot emptySlot;
	if (index < 0 || index >= INVENTORY_SLOT_COUNT) return emptySlot;
	return m_slots[index];
}

// ── GetBgRect (배경 이미지 RectTransform 기준 — 이미지 크기 그대로) ──
Gdiplus::RectF Inventory::GetBgRect() const {
	if (!m_bgImage) return Gdiplus::RectF(0, 0, 0, 0);
	RectTransform* rt = m_bgImage->GetRectTransform();
	if (!rt) return Gdiplus::RectF(0, 0, 0, 0);
	float bw = (rt->GetWidth() - 80.f) * rt->GetScaleX();
	float bh = (rt->GetHeight() - 30.f) * rt->GetScaleY();
	return Gdiplus::RectF(rt->GetX() - bw * 0.5f, rt->GetY() - bh * 0.5f, bw, bh);
}

// ── 렌더링 (순서: BG → 슬롯 버튼 → 슬롯 아이템/텍스트 → 장착 하이라이트, 슬롯은 BG 안에 배치) ──
void Inventory::Render(int equippedSlotIndex)
{
	RenderManager* pRM = RenderManager::GetInstance();
	if (!pRM) return;

	if (m_bgImage) m_bgImage->Render();

	for (int i = 0; i < INVENTORY_SLOT_COUNT; ++i) {
		if (m_slotButtons[i])
			m_slotButtons[i]->Render();

		if (m_slotItemImages[i] && m_slotItemImages[i]->IsEnabled())
			m_slotItemImages[i]->Render();

		if (m_slotCountTexts[i] && m_slotCountTexts[i]->IsEnabled())
			m_slotCountTexts[i]->Render();
	}

	// 장착 슬롯 하이라이트
	if (equippedSlotIndex >= 0 && equippedSlotIndex < INVENTORY_SLOT_COUNT
		&& m_slotButtons[equippedSlotIndex]) {
		RectTransform* rt = m_slotButtons[equippedSlotIndex]->GetRectTransform();
		if (rt) {
			float bw = rt->GetWidth() * rt->GetScaleX();
			float bh = rt->GetHeight() * rt->GetScaleY();
			Gdiplus::RectF highlightRect(rt->GetX() - bw * 0.5f, rt->GetY() - bh * 0.5f, bw, bh);
			pRM->AddDrawRectCommand(highlightRect, Gdiplus::Color(255, 255, 0, 0), 3.0f, LAYER_UI_FOREGROUND, 3.2f);
		}
	}
}

int Inventory::FindFirstEmptySlot() const {
	for (int i = 0; i < INVENTORY_SLOT_COUNT; ++i)
		if (m_slots[i].IsEmpty()) return i;
	return -1;
}

int Inventory::FindExistingStack(UINT itemId) const {
	for (int i = 0; i < INVENTORY_SLOT_COUNT; ++i)
		if (!m_slots[i].IsEmpty() && m_slots[i].item->GetID() == itemId && m_slots[i].count < ITEM_STACK_MAX)
			return i;
	return -1;
}

bool Inventory::CheckHasEnoughItems(const std::map<UINT, UINT>& requiredItems) const {
	for (const auto& req : requiredItems)
		if (GetItemCount(req.first) < req.second) return false;
	return true;
}

bool Inventory::ContainsScreenPoint(float screenX, float screenY) const {
	Gdiplus::RectF bgRect = GetBgRect();
	return (bgRect.Width > 0 && bgRect.Height > 0 && bgRect.Contains(screenX, screenY));
}

void Inventory::HandleSlotClick(int slotIndex, Player* player) {
		InventoryManager::GetInstance()->TryUseItem(player, slotIndex);
}

bool Inventory::HandleRightClick(float mouseScreenX, float mouseScreenY, Player* player) {
	if (!player) return false;

	float totalSlotsWidth = SLOT_STRIDE * INVENTORY_SLOT_COUNT - SLOT_PADDING;
	float startX = WINCX * 0.5f - totalSlotsWidth * 0.5f;
	float startY = WINCY - SLOT_HEIGHT - 30.0f;

	// 1) 슬롯 클릭 검사 → 슬롯 위면 처리 후 true
	bool inSlotRow = (mouseScreenY >= startY && mouseScreenY < startY + SLOT_HEIGHT);
	if (inSlotRow) {
		float localX = mouseScreenX - startX;
		int col = static_cast<int>(localX / SLOT_STRIDE);
		float slotLocalX = localX - col * SLOT_STRIDE;
		if (col >= 0 && col < INVENTORY_SLOT_COUNT && slotLocalX >= 0.0f && slotLocalX < SLOT_WIDTH) {
			HandleSlotClick(col, player);
			return true;
		}
	}

	// 2) 슬롯이 아니면 인벤토리 영역(배경 Rect) 검사 → 안이면 이동만 막고 true
	if (ContainsScreenPoint(mouseScreenX, mouseScreenY))
		return true;

	return false;
}
