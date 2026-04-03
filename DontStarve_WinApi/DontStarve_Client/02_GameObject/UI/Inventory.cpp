#include "99_Default/pch.h"
#include "Inventory.h"
#include "../../01_Manager/InventoryManager/InventoryManager.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../01_Manager/InputManager/InputManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"
#include "../../01_Manager/DataManager/DataManager.h"
#include "../../02_GameObject/Item/Item.h"
#include "../../02_GameObject/Component/Sprite/SpriteRenderer.h"
#include "../../02_GameObject/Component/Transform/RectTransform.h"

// 슬롯 비움
void ItemSlot::Clear() {
	id = GOID_NONE;
	cachedSprite = nullptr;
	UpdateCount(0);
}

Inventory::Inventory(Player* owner)
	: m_player(owner)
	, m_slots(INVENTORY_SLOT_COUNT)
	, SLOT_WIDTH(64.0f)
	, SLOT_HEIGHT(64.0f)
	, SLOT_PADDING(10.0f)
	, SLOT_STRIDE(64.0f + 10.0f)
	, m_bgX(0), m_bgY(0), m_bgW(0), m_bgH(0)
	, m_slotStartX(0), m_slotStartY(0)
	, m_font(nullptr), m_textBrush(nullptr), m_stringFormat(nullptr)
{
}

Inventory::~Inventory() {
	Utils::SafeDelete(m_font);
	Utils::SafeDelete(m_textBrush);
	Utils::SafeDelete(m_stringFormat);
}

void Inventory::Init()
{
	auto* resMgr = ResourceManager::GetInstance();

	// ── 리소스 로드 ────────────────────────────────────────────
	m_bgSprite = resMgr->LoadSprite(L"Resource\\UI\\Inven.png");
	m_slotSprite = resMgr->LoadSprite(L"Resource\\UI\\slot.png");

	// ── 위치/크기 계산 ──────────────────────────────────────────
	if (m_bgSprite) {
		m_bgW = static_cast<float>(m_bgSprite->bitmap->GetWidth());
		m_bgH = static_cast<float>(m_bgSprite->bitmap->GetHeight());
		m_bgX = WINCX * 0.5f;
		m_bgY = WINCY - m_bgH * 0.5f - 5.0f;
	}

	float totalSlotsWidth = SLOT_STRIDE * INVENTORY_SLOT_COUNT - SLOT_PADDING;
	m_slotStartX = WINCX * 0.5f - totalSlotsWidth * 0.5f + SLOT_WIDTH * 0.5f;
	m_slotStartY = WINCY - SLOT_HEIGHT - 30.0f + SLOT_HEIGHT * 0.5f;

	// ── 텍스트 렌더링 리소스 생성 ──────────────────────────────────
	m_font = new Gdiplus::Font(L"Arial", 14.0f, Gdiplus::FontStyleBold);
	m_textBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 255, 255, 255));
	m_stringFormat = new Gdiplus::StringFormat();
	m_stringFormat->SetAlignment(Gdiplus::StringAlignmentFar);
	m_stringFormat->SetLineAlignment(Gdiplus::StringAlignmentFar);
}

void Inventory::Update(float deltaTime)
{
	if (!m_player || m_player->GetHp() <= 0) return;

	InputManager* input = InputManager::GetInstance();
	if (input->IsLButtonClicked()) {
		POINT mousePos = input->GetMousePos();
		float mx = (float)mousePos.x;
		float my = (float)mousePos.y;

		// 슬롯 클릭 감지
		for (int i = 0; i < INVENTORY_SLOT_COUNT; ++i) {
			float cx = m_slotStartX + i * SLOT_STRIDE;
			float cy = m_slotStartY;
			Gdiplus::RectF slotRect(cx - SLOT_WIDTH * 0.5f, cy - SLOT_HEIGHT * 0.5f, SLOT_WIDTH, SLOT_HEIGHT);
			
			if (slotRect.Contains(mx, my)) {
				// Shift 키가 눌려있으면 아이템 버리기
				if (input->IsKeyPressed(VK_SHIFT)) {
					InventoryManager::GetInstance()->TryDropItem(m_player, i);
				}
				else {
					HandleSlotClick(i, m_player);
				}
				break;
			}
		}
	}
}

void Inventory::Render(int equippedSlotIndex)
{
	RenderManager* pRM = RenderManager::GetInstance();
	if (!pRM) return;

	// 1. 배경 렌더링
	if (m_bgSprite) {
		pRM->AddUICommand(m_bgSprite->bitmap.get(), m_bgSprite->sourceRect,
			m_bgX, m_bgY, 1.0f, 1.0f, 0.5f, 0.5f, LAYER_UI_BACKGROUND, 0.0f);
	}

	// 2. 슬롯 및 아이템 렌더링
	auto* resMgr = ResourceManager::GetInstance();
	auto* dataMgr = DataManager::GetInstance();

	for (int i = 0; i < INVENTORY_SLOT_COUNT; ++i) {
		float cx = m_slotStartX + i * SLOT_STRIDE;
		float cy = m_slotStartY;

		// 2-1. 슬롯 배경
		if (m_slotSprite) {
			pRM->AddUICommand(m_slotSprite->bitmap.get(), m_slotSprite->sourceRect,
				cx, cy, 1.0f, 1.0f, 0.5f, 0.5f, LAYER_UI_FOREGROUND, 1.0f);
		}

		// 2-2. 장착 하이라이트
		if (i == equippedSlotIndex) {
			Gdiplus::RectF highlightRect(cx - SLOT_WIDTH * 0.5f, cy - SLOT_HEIGHT * 0.5f, SLOT_WIDTH, SLOT_HEIGHT);
			pRM->AddDrawRectCommand(highlightRect, Gdiplus::Color(255, 255, 255, 0), 3.0f, LAYER_UI_FOREGROUND, 1.1f);
		}
// 2-3. 아이템 이미지 및 개수
ItemSlot& slot = m_slots[i];
if (!slot.IsEmpty()) {
	// 캐싱된 스프라이트가 없으면 로드
	if (!slot.cachedSprite) {
		const auto* data = dataMgr->GetObjectResourceInfo(slot.id);
		if (data) {
			std::wstring path = ResourcePathUtils::BuildResourcePath(data->baseDir, data->imageName);
			slot.cachedSprite = resMgr->LoadSprite(path, { data->pivotX, data->pivotY });
		}
	}

	if (slot.cachedSprite) {
		float bw = slot.cachedSprite->sourceRect.Width;
		float bh = slot.cachedSprite->sourceRect.Height;
		float scale = (std::min)(SLOT_WIDTH / bw, SLOT_HEIGHT / bh) * 0.8f;

		// 💡 UI 슬롯에서는 아이템의 월드 피벗을 무시하고 중앙(0.5, 0.5)에 강제 배치
		pRM->AddUICommand(slot.cachedSprite->bitmap.get(), slot.cachedSprite->sourceRect,
			cx, cy, scale, scale, 0.5f, 0.5f,
			LAYER_UI_FOREGROUND, 1.2f);
	}
	if (slot.count > 1) {
		Gdiplus::RectF textRect(cx - SLOT_WIDTH * 0.5f, cy - SLOT_HEIGHT * 0.5f, SLOT_WIDTH, SLOT_HEIGHT);
		pRM->AddTextCommand(&slot.countStr, m_font, m_textBrush, m_stringFormat, textRect, LAYER_UI_FOREGROUND, 1.3f);
	}
}
}
}


void Inventory::UpdateSlotButton(int slotIndex)
{
}

bool Inventory::AddItem(Item* itemDef, UINT count) {
	if (!itemDef || count == 0) return false;
	GameObjectID id = itemDef->GetID();

	// 트랜잭션: 모든 슬롯의 여유 공간을 합쳐서 요청 수량을 수용할 수 있는지 먼저 확인
	if (GetAvailableSpace(id) < count) {
		return false;
	}

	UINT remaining = count;

	// 1. 기존에 같은 아이템이 있는 스택들에 우선적으로 채움
	for (auto& slot : m_slots) {
		if (!slot.IsEmpty() && slot.id == id && slot.count < ITEM_STACK_MAX) {
			UINT canAdd = ITEM_STACK_MAX - slot.count;
			UINT actualAdd = (std::min)(remaining, canAdd);
			slot.UpdateCount(slot.count + actualAdd);
			remaining -= actualAdd;
			if (remaining == 0) break;
		}
	}

	// 2. 남은 수량이 있다면 빈 슬롯들을 찾아 순차적으로 채움 (Overflow 처리)
	while (remaining > 0) {
		int emptySlotIndex = FindFirstEmptySlot();
		if (emptySlotIndex == -1) break; 

		m_slots[emptySlotIndex].id = id;
		UINT actualAdd = (std::min)(remaining, ITEM_STACK_MAX);
		m_slots[emptySlotIndex].UpdateCount(actualAdd);
		remaining -= actualAdd;
	}

	// 모든 수량이 인벤토리에 데이터로 들어갔으므로, 월드 객체(itemDef)는 무조건 파괴
	if (remaining == 0) {
		ObjectManager::GetInstance()->RemoveGameObject(itemDef);
		return true;
	}

	return false;
}

bool Inventory::RemoveItem(UINT slotIndex, UINT count) {
	if (slotIndex >= (UINT)m_slots.size() || m_slots[slotIndex].IsEmpty() || m_slots[slotIndex].count < count)
		return false;

	m_slots[slotIndex].UpdateCount(m_slots[slotIndex].count - count);
	if (m_slots[slotIndex].count == 0) m_slots[slotIndex].Clear();
	return true;
}

bool Inventory::ConsumeItems(const std::map<UINT, UINT>& requiredItems) {
	if (!CheckHasEnoughItems(requiredItems)) return false;

	std::map<UINT, UINT> tempRequired = requiredItems;
	for (int i = (int)m_slots.size() - 1; i >= 0; --i) {
		ItemSlot& slot = m_slots[i];
		if (slot.IsEmpty() || tempRequired.count(slot.id) == 0) continue;

		UINT& neededCount = tempRequired.at(slot.id);
		if (neededCount > 0) {
			UINT consume = min(slot.count, neededCount);
			slot.UpdateCount(slot.count - consume);
			neededCount -= consume;
			if (slot.count == 0) slot.Clear();
		}
	}
	return true;
}

void Inventory::ClearAllItems() {
	for (auto& slot : m_slots) slot.Clear();
}

bool Inventory::AddItemByID(GameObjectID itemID, UINT count) {
	// 데이터 중심이므로 itemID와 count를 바로 AddItem에 넘길 수 있도록 오버로딩하거나 내부 로직 수정
	// 여기서는 기존 AddItem이 Item*을 받으므로 임시 객체를 생성해서 넘기고, AddItem 내부에서 삭제되도록 함
	Item* item = ObjectManager::GetInstance()->CreateItem(itemID, 0.0f, 0.0f);
	if (!item) return false;
	
	// AddItem 내부에서 성공 시 item을 RemoveGameObject 처리하므로 별도 해제 불필요
	return AddItem(item, count);
}

std::vector<std::pair<GameObjectID, UINT>> Inventory::GetAllItemsSnapshot() const {
	std::vector<std::pair<GameObjectID, UINT>> result;
	for (const ItemSlot& slot : m_slots) {
		if (!slot.IsEmpty()) result.emplace_back(slot.id, slot.count);
	}
	return result;
}

UINT Inventory::GetAvailableSpace(UINT itemId) const {
	UINT totalSpace = 0;
	for (const auto& slot : m_slots) {
		if (slot.IsEmpty()) {
			totalSpace += ITEM_STACK_MAX;
		}
		else if (slot.id == itemId) {
			totalSpace += (ITEM_STACK_MAX - slot.count);
		}
	}
	return totalSpace;
}

UINT Inventory::GetItemCount(UINT itemId) const {
	UINT total = 0;
	for (const auto& slot : m_slots)
		if (!slot.IsEmpty() && slot.id == itemId) total += slot.count;
	return total;
}

const ItemSlot& Inventory::GetSlot(int index) const {
	static ItemSlot emptySlot;
	if (index < 0 || index >= (int)m_slots.size()) return emptySlot;
	return m_slots[index];
}

Gdiplus::RectF Inventory::GetBgRect() const {
	return Gdiplus::RectF(m_bgX - m_bgW * 0.5f, m_bgY - m_bgH * 0.5f, m_bgW, m_bgH);
}

int Inventory::FindFirstEmptySlot() const {
	for (int i = 0; i < (int)m_slots.size(); ++i)
		if (m_slots[i].IsEmpty()) return i;
	return -1;
}

int Inventory::FindExistingStack(UINT itemId) const {
	for (int i = 0; i < (int)m_slots.size(); ++i)
		if (!m_slots[i].IsEmpty() && m_slots[i].id == itemId && m_slots[i].count < ITEM_STACK_MAX)
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

	// 슬롯 클릭 검사
	for (int i = 0; i < INVENTORY_SLOT_COUNT; ++i) {
		float cx = m_slotStartX + i * SLOT_STRIDE;
		float cy = m_slotStartY;
		Gdiplus::RectF slotRect(cx - SLOT_WIDTH * 0.5f, cy - SLOT_HEIGHT * 0.5f, SLOT_WIDTH, SLOT_HEIGHT);
		
		if (slotRect.Contains(mouseScreenX, mouseScreenY)) {
			HandleSlotClick(i, player);
			return true;
		}
	}

	if (ContainsScreenPoint(mouseScreenX, mouseScreenY)) return true;
	return false;
}
