#include "../../99_Default/pch.h"
#include "Inventory.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../02_GameObject/Item/Item.h"

Inventory::Inventory() : m_slots(INVENTORY_SLOT_COUNT) {
	m_inventoryBgBitmap = nullptr;
	m_slotBgBitmap = nullptr;
	m_font = nullptr;
	m_solidBrush = nullptr;
	m_shadowBrush = nullptr;
	m_stringFormat = nullptr;
}

Inventory::~Inventory() {
	delete m_inventoryBgBitmap;
	m_inventoryBgBitmap = nullptr;

	delete m_slotBgBitmap;
	m_slotBgBitmap = nullptr;

	if (m_font) { delete m_font; m_font = nullptr; }
	if (m_solidBrush) { delete m_solidBrush; m_solidBrush = nullptr; }
	if (m_shadowBrush) { delete m_shadowBrush; m_shadowBrush = nullptr; }
	if (m_stringFormat) { delete m_stringFormat; m_stringFormat = nullptr; }
}

// 인벤토리 초기화: 슬롯 UI의 화면 위치를 설정합니다.
void Inventory::Init(std::vector<Gdiplus::RectF>& slotRects)
{	
	float slotWidth = 64.0f;
	float slotHeight = 64.0f;
	float slotPadding = 10.0f;

	float totalSlotsWidth = (slotWidth + slotPadding) * INVENTORY_SLOT_COUNT - slotPadding;

	float startX = (WINCX / 2.0f) - (totalSlotsWidth / 2.0f);
	float startY = WINCY - slotHeight - 30.0f;

	for (int i = 0; i < INVENTORY_SLOT_COUNT; ++i) {
		slotRects[i] = Gdiplus::RectF(
			startX + i * (slotWidth + slotPadding), 
			startY,                                
			slotWidth,                             
			slotHeight                              
		);
	}
	
	// 안전하게 m_slotRects 할당
	m_slotRects.clear();
	m_slotRects = slotRects;
	
	LoadUIBitmaps();
}

// 아이템 추가 함수
bool Inventory::AddItem(std::shared_ptr<Item> itemDef, UINT count) {
	if (!itemDef) return false; // 유효하지 않은 아이템 정의

	//  이미 인벤토리에 같은 아이템이 저장되어 있고, 스택이 가능하다면 그 슬롯에 추가합니다.
	int existingSlotIndex = FindExistingStack(itemDef->GetID());
	if (existingSlotIndex != -1 && m_slots[existingSlotIndex].count < ITEM_STACK_MAX) {
		// 추가할 수 있는 최대 개수를 계산하여 추가하고, 남은 개수를 업데이트합니다.
		UINT canAdd = ITEM_STACK_MAX - m_slots[existingSlotIndex].count;
		UINT actualAdd = min(count, canAdd); // min() 함수는 <algorithm>에 있음
		m_slots[existingSlotIndex].count += actualAdd;
		count -= actualAdd;
		if (count == 0)
			return true; // 모든 아이템이 추가 완료
	}

	// 남은 아이템을 새 슬롯에 계속 추가합니다.
	while (count > 0) {
		int emptySlotIndex = FindFirstEmptySlot();
		if (emptySlotIndex == -1) {
			return false; // 인벤토리 공간 부족
		}
		UINT actualAdd = min(count, ITEM_STACK_MAX); // 한 슬롯에 넣을 수 있는 최대 개수
		m_slots[emptySlotIndex].item = itemDef;     // 아이템 정의 설정
		m_slots[emptySlotIndex].count = actualAdd;   // 아이템 개수 설정
		count -= actualAdd;                         // 남은 추가 대상 개수업데이트
	}

	return true; // 모든 아이템이 성공적으로 추가
}

// 아이템 제거: 특정 슬롯에서 특정 개수만큼 제거
bool Inventory::RemoveItem(UINT slotIndex, UINT count) {
	// 슬롯 인덱스 유효성 및 아이템 존재여부, 개수가 충분한지 검사
	if (slotIndex >= INVENTORY_SLOT_COUNT || m_slots[slotIndex].IsEmpty() || m_slots[slotIndex].count < count) {
		return false;
	}

	m_slots[slotIndex].count -= count; // 아이템 개수 감소
	if (m_slots[slotIndex].count == 0) {
		m_slots[slotIndex].Clear(); // 개수가 0이 되면 슬롯 정리
	}
	return true;
}

// 아이템 소모 --> 제작 시스템에서 여러 아이템 소모
bool Inventory::ConsumeItems(const std::map<UINT, UINT>& requiredItems) {
	// 소모 필요한 모든 아이템이 인벤토리에 충분히 있는지 먼저 확인
	if (!CheckHasEnoughItems(requiredItems)) {
		return false;
	}

	// 충분하다면 아이템 소모 
	std::map<UINT, UINT> tempRequired = requiredItems;

	for (int i = INVENTORY_SLOT_COUNT - 1; i >= 0; --i) {
		ItemSlot& slot = m_slots[i];
		if (slot.IsEmpty() || tempRequired.count(slot.item->GetID()) == 0) continue; // 아이템 없거나 소모 대상 아이템이 아니면 넘기기

		UINT itemIdToConsume = slot.item->GetID();
		UINT& neededCount = tempRequired.at(itemIdToConsume); // 참조를 받아서 바로 필요 개수를 업데이트

		if (neededCount > 0) { // 아직 이 아이템이 필요하다면
			UINT consumeAmount = min(slot.count, neededCount); // 현재 슬롯에서 소모할 수 있는 최대 개수
			slot.count -= consumeAmount;
			neededCount -= consumeAmount; // 필요한 아이템이 감소

			if (slot.count == 0) { // 아이템 소진되면 클리어
				slot.Clear();
			}
		}
	}
	return true;
}

// 특정 Item ID의 전체 아이템의 총량 수 반환
UINT Inventory::GetItemCount(UINT itemId) const {
	UINT totalCount = 0;
	for (const auto& slot : m_slots) {
		if (!slot.IsEmpty() && slot.item->GetID() == itemId) {
			totalCount += slot.count;
		}
	}
	return totalCount;
}

// 특정 아이템 슬롯의 const 참조를 반환
const ItemSlot& Inventory::GetSlot(int index) const {
	static ItemSlot emptySlot;
	if (index < 0 || index >= INVENTORY_SLOT_COUNT) {
		return emptySlot;
	}
	return m_slots[index];
}

// 인벤토리 UI와 각 슬롯에 있는 아이템들을 그리기 위한 함수 - RenderManager를 통해 렌더링 명령 추가
void Inventory::Render(int equippedSlotIndex) {
	RenderManager* pRM = RenderManager::GetInstance();

	if (!pRM) {
		OutputDebugStringW(L"Inventory: RenderManager가 없습니다.\n");
		return;
	}

	if (m_slotRects.empty()) {
		OutputDebugStringW(L"Inventory: 슬롯 위치가 초기화되지 않았습니다.\n");
		return;
	}

	if (m_inventoryBgBitmap) {
		Gdiplus::Bitmap* inventoryBgBitmap = m_inventoryBgBitmap;
		float bgWidth = (float)inventoryBgBitmap->GetWidth();
		float bgHeight = (float)inventoryBgBitmap->GetHeight();

		float bgX = WINCX / 2.0f - bgWidth / 2.0f;
		float bgY = WINCY - bgHeight - 5.f; // 화면 하단에서 10픽셀 위로

		Gdiplus::RectF inventoryBgDestRect(bgX, bgY, bgWidth, bgHeight);

		pRM->AddDrawCommand(
			inventoryBgBitmap,
			inventoryBgDestRect,
			Gdiplus::RectF(0, 0, (float)inventoryBgBitmap->GetWidth(), (float)inventoryBgBitmap->GetHeight()),
			Gdiplus::UnitPixel,
			Gdiplus::PointF(inventoryBgDestRect.X, inventoryBgDestRect.Y),
			LAYER_UI_BACKGROUND,
			0.f,
			DIR_DOWN
		);
	} else {
		OutputDebugStringW(L"Inventory: 인벤토리 배경 비트맵이 없습니다.\n");
	}

	for (int i = 0; i < INVENTORY_SLOT_COUNT; ++i) {
		const ItemSlot& slot = m_slots[i];

		//  슬롯 배경 UI 그리기 -
		//  Inventory::s_slotBgBitmap 사용
		if (m_slotBgBitmap) {
			Gdiplus::Bitmap* slotBgBitmap = m_slotBgBitmap;
			pRM->AddDrawCommand(
				slotBgBitmap,
				m_slotRects[i],
				Gdiplus::RectF(0, 0, (float)slotBgBitmap->GetWidth(), (float)slotBgBitmap->GetHeight()),
				Gdiplus::UnitPixel,
				Gdiplus::PointF(m_slotRects[i].X, m_slotRects[i].Y),
				LAYER_UI_FOREGROUND,
				0.f + 1.0f + (float)i * 0.001f,
				DIR_DOWN
			);
		}

		// 슬롯에 아이템이 있다면 아이템 이미지 그리기
		if (!slot.IsEmpty()) {
			Gdiplus::Bitmap* itemBitmap = slot.item->GetBitmap();
			if (itemBitmap) {
				Gdiplus::RectF destRect = m_slotRects[i];

				float itemImageWidth = (float)itemBitmap->GetWidth();
				float itemImageHeight = (float)itemBitmap->GetHeight();

				float scaleFactor = min(destRect.Width / itemImageWidth, destRect.Height / itemImageHeight);
				float renderWidth = itemImageWidth * scaleFactor;
				float renderHeight = itemImageHeight * scaleFactor;

				Gdiplus::RectF itemRenderRect(
					destRect.X + (destRect.Width - renderWidth) / 2.0f,
					destRect.Y + (destRect.Height - renderHeight) / 2.0f,
					renderWidth,
					renderHeight
				);

				pRM->AddDrawCommand(
					itemBitmap,
					itemRenderRect,
					Gdiplus::RectF(0, 0, itemImageWidth, itemImageHeight),
					Gdiplus::UnitPixel,
					Gdiplus::PointF(itemRenderRect.X, itemRenderRect.Y),
					LAYER_UI_FOREGROUND,
					0.f + 2.0f + (float)i * 0.001f,
					DIR_DOWN
				);

				// 아이템 개수 텍스트 렌더링
				if (slot.count >= 1) {

					wchar_t countStr[16];
					swprintf_s(countStr, 16, L"%u", slot.count);

					Gdiplus::RectF textRect(
						itemRenderRect.X,
						itemRenderRect.Y,
						itemRenderRect.Width,
						itemRenderRect.Height
					);

					pRM->AddTextCommand(countStr, m_font, m_shadowBrush, m_stringFormat,
						Gdiplus::RectF(textRect.X + 1, textRect.Y + 1, textRect.Width, textRect.Height),
						LAYER_UI_FOREGROUND, 0.f + 3.0f + (float)i * 0.001f);

					pRM->AddTextCommand(countStr, m_font, m_solidBrush, m_stringFormat,
						textRect,
						LAYER_UI_FOREGROUND, 0.f + 3.1f + (float)i * 0.001f);
				}
			}
		}
	}
	if (equippedSlotIndex != -1 &&
		equippedSlotIndex >= 0 && equippedSlotIndex < INVENTORY_SLOT_COUNT) {

		// 하이라이트를 AddDrawCommand로 직접 그리기
		Gdiplus::RectF highlightRect = m_slotRects[equippedSlotIndex];
		
		// 하이라이트 사각형 그리기 (노란색 테두리)
		pRM->AddDrawCommand(
			highlightRect,
			Gdiplus::Color(255, 255, 255, 0), // 노란색
			3.0f, // 테두리 두께
			LAYER_UI_FOREGROUND,
			0.f + 3.2f  // UI 텍스트보다 살짝 뒤에
		);
	}
}

int Inventory::FindFirstEmptySlot() const {
	for (int i = 0; i < INVENTORY_SLOT_COUNT; ++i) {
		if (m_slots[i].IsEmpty()) {
			return i;
		}
	}
	return -1; // 빈 슬롯 없음
}

int Inventory::FindExistingStack(UINT itemId) const {
	for (int i = 0; i < INVENTORY_SLOT_COUNT; ++i) {
		// 아이템 있으면서 같고, 같은 아이템이면, 스택이 최대가 아니면 반환
		if (!m_slots[i].IsEmpty() && m_slots[i].item->GetID() == itemId && m_slots[i].count < ITEM_STACK_MAX) {
			return i;
		}
	}
	return -1; // 기존 스택을 찾을 수 없음
}

bool Inventory::CheckHasEnoughItems(const std::map<UINT, UINT>& requiredItems) const {
	for (const auto& req : requiredItems) {
		if (GetItemCount(req.first) < req.second) {
			return false; // 필요한 아이템 개수 부족
		}
	}
	return true; // 모든 필요한 아이템 개수 충분
}

void Inventory::LoadUIBitmaps() 
{
	// ResourceManager를 사용하여 경로 구성
	auto* pRM = ResourceManager::GetInstance();
	
	if (!m_inventoryBgBitmap) {
		std::wstring invenPath = pRM->BuildResourcePath(L"Resource\\UI", L"", L"Inven.png");
		m_inventoryBgBitmap = BitmapUtils::LoadBitmapFromFile(invenPath.c_str());
	}
	if (!m_slotBgBitmap) {
		std::wstring slotPath = pRM->BuildResourcePath(L"Resource\\UI", L"", L"slot.png");
		m_slotBgBitmap = BitmapUtils::LoadBitmapFromFile(slotPath.c_str());
	}

	if (!m_font) {
		Gdiplus::FontFamily fontFamily(L"Arial"); // 폰트 패밀리
		m_font = new Gdiplus::Font(&fontFamily, 18, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
		if (m_font->GetLastStatus() != Gdiplus::Ok) {
			delete m_font;
			m_font = nullptr;
		}
	}
	if (!m_solidBrush) m_solidBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 255, 255, 255)); // 흰색 브러시
	if (!m_shadowBrush) m_shadowBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 0, 0, 0));       // 그림자 브러시 (그림자용)
	if (!m_stringFormat) {
		m_stringFormat = new Gdiplus::StringFormat();
		m_stringFormat->SetAlignment(Gdiplus::StringAlignmentFar);
		m_stringFormat->SetLineAlignment(Gdiplus::StringAlignmentFar);
	}
}
void Inventory::ReleaseUIBitmaps() {
	if (m_inventoryBgBitmap) { delete m_inventoryBgBitmap; m_inventoryBgBitmap = nullptr; }
	if (m_slotBgBitmap) { delete m_slotBgBitmap; m_slotBgBitmap = nullptr; }
}

bool Inventory::HandleMouseClick(float mouseScreenX, float mouseScreenY, Player* player) {
	if (!player) return false;
	
	for (int i = 0; i < INVENTORY_SLOT_COUNT; ++i) {
		if (m_slotRects[i].Contains(mouseScreenX, mouseScreenY)) {
			const ItemSlot& slot = GetSlot(i);
			
			if (slot.IsEmpty()) {
				return true; // 빈 슬롯 클릭도 처리됨으로 간주
			}
			
			// 아이템이 있는 슬롯 클릭 시 장착 시도
			player->ToggleEquipItem(i);
			
			return true; 
		}
	}
	return false; 
}
