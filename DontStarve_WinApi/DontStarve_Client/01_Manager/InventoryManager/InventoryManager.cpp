#include "../../99_Default/pch.h"
#include "InventoryManager.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../02_GameObject/GameObject.h"
#include "../../02_GameObject/Item/Item.h"
#include "../../02_GameObject/UI/Inventory.h"
#include "../../01_Manager/SceneManager/SceneManager.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"

InventoryManager::InventoryManager() {}
InventoryManager::~InventoryManager() {
	Release(); 
}

void InventoryManager::Init() {
	LoadCraftingRecipes();
}

void InventoryManager::LateInit() {}
void InventoryManager::Update(float deltaTime) {}
void InventoryManager::LateUpdate() {}
void InventoryManager::Render() {
	// Player의 인벤토리 렌더링
	Player* player = ObjectManager::GetInstance()->GetPlayer();
	if (!player) {
		return;
	}
	
	Inventory* inventory = player->GetInventory();
	if (!inventory) {
		return;
	}
	
	// 현재 장착된 아이템 슬롯 인덱스 가져오기
	int equippedSlotIndex = player->GetEquippedSlotIndex();
	
	// RenderManager를 통해 인벤토리 렌더링
	inventory->Render(equippedSlotIndex);
}

void InventoryManager::Release() {
	// 비트맵 캐시 정리
	for (auto& pair : m_bitmapCache) {
		if (pair.second) {
			delete pair.second;
		}
	}
	m_bitmapCache.clear();
	m_craftingRecipes.clear();
}

// 월드 오브젝트로부터 아이템 획득
bool InventoryManager::TryGainItemFromWorldObject(Player* player, GameObject* worldObject) {
	if (!player || !worldObject) return false;
	
	Inventory* inventory = player->GetInventory();
	if (!inventory) return false;
	
	// 월드 오브젝트로부터 획득 가능한 아이템들 계산
	std::vector<std::pair<GameObjectID, UINT>> drops = CalculateDropsFromObject(worldObject);
	
	bool anyItemAdded = false;
	for (const auto& drop : drops) {
		// TODO: ObjectManager에 GetItemDefinition 메서드 추가 필요
		// 임시로 주석 처리
		/*
		std::shared_ptr<Item> itemDef = ObjectManager::GetInstance()->GetItemDefinition(drop.first);
		if (itemDef) {
			if (inventory->AddItem(itemDef, drop.second)) {
				anyItemAdded = true;
				OutputDebugStringW((L"InventoryManager: 아이템 획득 - ID: " + std::to_wstring(drop.first) + L", 개수: " + std::to_wstring(drop.second) + L"\n").c_str());
			}
		}
		*/
		OutputDebugStringW((L"InventoryManager: 아이템 획득 예정 - ID: " + std::to_wstring(drop.first) + L", 개수: " + std::to_wstring(drop.second) + L"\n").c_str());
	}
	
	return anyItemAdded;
}

// 아이템 사용 처리
bool InventoryManager::TryUseItem(Player* player, int slotIndex) {
	if (!player) return false;
	
	Inventory* inventory = player->GetInventory();
	if (!inventory) return false;
	
	const ItemSlot& slot = inventory->GetSlot(slotIndex);
	if (slot.IsEmpty()) return false;
	
	// 아이템 타입에 따른 사용 처리
	std::shared_ptr<Item> item = slot.item;
	
	// TODO: 아이템 타입별 사용 로직 구현
	// 예: 도구 착용, 소비 아이템 사용, 건설 아이템 사용 등
	
	OutputDebugStringW((L"InventoryManager: 아이템 사용 - 슬롯 " + std::to_wstring(slotIndex) + L"\n").c_str());
	return true;
}

// 아이템 버리기 (월드에 드롭)
bool InventoryManager::TryDropItem(Player* player, int slotIndex, UINT count) {
	if (!player) return false;
	
	Inventory* inventory = player->GetInventory();
	if (!inventory) return false;
	
	const ItemSlot& slot = inventory->GetSlot(slotIndex);
	if (slot.IsEmpty() || slot.count < count) return false;
	
	// 인벤토리에서 아이템 제거
	if (inventory->RemoveItem(slotIndex, count)) {
		// TODO: 월드에 아이템 오브젝트 생성
		// 플레이어 위치 근처에 아이템 드롭
		
		OutputDebugStringW((L"InventoryManager: 아이템 드롭 - 슬롯 " + std::to_wstring(slotIndex) + L", 개수: " + std::to_wstring(count) + L"\n").c_str());
		return true;
	}
	
	return false;
}

// 아이템 조합 시스템
bool InventoryManager::TryCraftItem(Player* player, GameObjectID targetItemID) {
	if (!player) return false;
	
	Inventory* inventory = player->GetInventory();
	if (!inventory) return false;
	
	// 조합 레시피 확인
	if (!HasCraftingRecipe(targetItemID)) {
		OutputDebugStringW(L"InventoryManager: 조합 레시피가 없습니다.\n");
		return false;
	}
	
	std::map<UINT, UINT> recipe = GetCraftingRecipe(targetItemID);
	
	// 필요한 재료가 있는지 확인
	if (!inventory->CheckHasEnoughItems(recipe)) {
		OutputDebugStringW(L"InventoryManager: 조합에 필요한 재료가 부족합니다.\n");
		return false;
	}
	
	// 재료 소모
	if (inventory->ConsumeItems(recipe)) {
		// TODO: ObjectManager에 GetItemDefinition 메서드 추가 필요
		// 임시로 주석 처리
		/*
		std::shared_ptr<Item> craftedItem = ObjectManager::GetInstance()->GetItemDefinition(targetItemID);
		if (craftedItem && inventory->AddItem(craftedItem, 1)) {
			OutputDebugStringW((L"InventoryManager: 아이템 조합 완료 - ID: " + std::to_wstring(targetItemID) + L"\n").c_str());
			return true;
		}
		*/
		OutputDebugStringW((L"InventoryManager: 아이템 조합 완료 예정 - ID: " + std::to_wstring(targetItemID) + L"\n").c_str());
		return true;
	}
	
	return false;
}

// 아이템 교환 시스템
bool InventoryManager::TryTradeItem(Player* player, const std::map<UINT, UINT>& giveItems, const std::map<UINT, UINT>& receiveItems) {
	if (!player) return false;
	
	Inventory* inventory = player->GetInventory();
	if (!inventory) return false;
	
	// 주는 아이템이 충분한지 확인
	if (!inventory->CheckHasEnoughItems(giveItems)) {
		OutputDebugStringW(L"InventoryManager: 교환할 아이템이 부족합니다.\n");
		return false;
	}
	
	// 받을 아이템을 넣을 공간이 있는지 확인
	// TODO: 인벤토리 공간 확인 로직 필요
	
	// 아이템 교환 실행
	if (inventory->ConsumeItems(giveItems)) {
		bool allReceived = true;
		for (const auto& receive : receiveItems) {
			// TODO: ObjectManager에 GetItemDefinition 메서드 추가 필요
			// 임시로 주석 처리
			/*
			std::shared_ptr<Item> item = ObjectManager::GetInstance()->GetItemDefinition((GameObjectID)receive.first);
			if (!item || !inventory->AddItem(item, receive.second)) {
				allReceived = false;
				break;
			}
			*/
		}
		
		if (allReceived) {
			OutputDebugStringW(L"InventoryManager: 아이템 교환 완료\n");
			return true;
		}
		else {
			OutputDebugStringW(L"InventoryManager: 아이템 교환 중 오류 발생\n");
			// TODO: 롤백 로직 필요
		}
	}
	
	return false;
}

// 인벤토리 저장/로드 기능
void InventoryManager::SaveInventoryToFile(Player* player, const std::wstring& filePath) {
	// TODO: 파일 저장 기능 구현
	OutputDebugStringW((L"InventoryManager: 인벤토리 저장 - " + filePath + L"\n").c_str());
}

void InventoryManager::LoadInventoryFromFile(Player* player, const std::wstring& filePath) {
	// TODO: 파일 로드 기능 구현
	OutputDebugStringW((L"InventoryManager: 인벤토리 로드 - " + filePath + L"\n").c_str());
}

// 이미지 경로를 받아서 Gdiplus::Bitmap*을 반환하는 함수
Gdiplus::Bitmap* InventoryManager::GetBitmapForPath(const std::wstring& imagePath) {
	auto it = m_bitmapCache.find(imagePath);
	if (it != m_bitmapCache.end()) {
		return it->second; 
	}

	Gdiplus::Bitmap* newBitmap = BitmapUtils::LoadBitmapFromFile(imagePath.c_str());
	if (newBitmap && newBitmap->GetLastStatus() == Gdiplus::Ok) {
		m_bitmapCache[imagePath] = newBitmap;
		return newBitmap;
	}
	else {
		if (newBitmap) delete newBitmap; 
		m_bitmapCache[imagePath] = nullptr; 
		return nullptr;
	}
}

// 아이템 조합 레시피 로드
void InventoryManager::LoadCraftingRecipes() {
	m_craftingRecipes.clear();
	
	// 도끼 조합 레시피: 나무 1개 + 나뭇가지 1개
	m_craftingRecipes[GOID_ITEM_AXE] = {
		{GOID_ITEM_NORMAL_TREE_LOG, 1},
		{GOID_ITEM_NORMAL_TWIGS, 1}
	};
	
	// 곡괭이 조합 레시피: 나뭇가지 2개 + 돌 2개
	m_craftingRecipes[GOID_ITEM_PICKAXE] = {
		{GOID_ITEM_NORMAL_TWIGS, 2},
		{GOID_ITEM_NORMAL_ROCK, 2}
	};
	
	OutputDebugStringW(L"InventoryManager: 조합 레시피 로드 완료\n");
}

bool InventoryManager::HasCraftingRecipe(GameObjectID itemID) const {
	return m_craftingRecipes.find(itemID) != m_craftingRecipes.end();
}

std::map<UINT, UINT> InventoryManager::GetCraftingRecipe(GameObjectID itemID) const {
	auto it = m_craftingRecipes.find(itemID);
	if (it != m_craftingRecipes.end()) {
		return it->second;
	}
	return {};
}

// 월드 오브젝트에서 얻을 수 있는 아이템 계산
std::vector<std::pair<GameObjectID, UINT>> InventoryManager::CalculateDropsFromObject(GameObject* worldObject) {
	std::vector<std::pair<GameObjectID, UINT>> drops;
	
	if (!worldObject) return drops;
	
	// TODO: 오브젝트 타입에 따른 드롭 아이템 계산
	// 예시: 나무 -> 나무 1개, 나뭇가지 1개
	//       바위 -> 돌 2-3개
	//       풀 -> 풀 1개
	
	// 임시 예시 코드
	GameObjectID objID = worldObject->GetID();
	
	switch (objID) {
		case GOID_NORMAL_TREE_SHORT:
		case GOID_NORMAL_TREE_NORMAL:
		case GOID_NORMAL_TREE_TALL:
			drops.push_back({GOID_ITEM_NORMAL_TREE_LOG, 1});
			drops.push_back({GOID_ITEM_NORMAL_TWIGS, 1});
			break;
			
		case GOID_NORMAL_ROCK:
			drops.push_back({GOID_ITEM_NORMAL_ROCK, 2});
			break;
			
		case GOID_GOLD_ROCK:
			drops.push_back({GOID_ITEM_GOLD_ROCK, 1});
			drops.push_back({GOID_ITEM_NORMAL_ROCK, 1});
			break;
			
		case GOID_NORMAL_GRASS:
			drops.push_back({GOID_ITEM_CUT_NORMAL_GRASS, 1});
			break;
			
		case GOID_NORMAL_SAPLING:
			drops.push_back({GOID_ITEM_NORMAL_TWIGS, 1});
			break;
			
		case GOID_BERRY_TREE:
			drops.push_back({GOID_ITEM_BERRY, 1});
			break;
	}
	
	return drops;
} 