#include "99_Default/pch.h"
#include "InventoryManager.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../02_GameObject/GameObject.h"
#include "../../02_GameObject/Item/Item.h"
#include "../../02_GameObject/Item/Tool/Tool.h"
#include "../../02_GameObject/UI/Inventory.h"
#include "../../02_GameObject/UI/CraftingRecipe.h"
#include "../../01_Manager/SceneManager/SceneManager.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../01_Manager/GameProgressManager/GameProgressManager.h"
#include "../../01_Manager/InputManager/InputManager.h"

InventoryManager::InventoryManager() {}
InventoryManager::~InventoryManager() {
	Release(); 
}

void InventoryManager::Init() {
	LoadCraftingRecipes();
}

void InventoryManager::LateInit() {}
void InventoryManager::Update(float deltaTime) 
{	
}
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
	
	// 현재 장착된 아이템의 슬롯 인덱스 가져오기
	int equippedSlotIndex = player->GetEquippedSlotIndex();
	
	// RenderManager를 통해 인벤토리 렌더링
	inventory->Render(equippedSlotIndex);
}

void InventoryManager::Release() {
	// 레시피 맵은 자동으로 정리됨
	m_craftingRecipes.clear();
}

// 월드 오브젝트로부터 아이템 획득
bool InventoryManager::TryGainItemFromWorldObject(Player* player, GameObject* worldObject) {
	if (!player || !worldObject) return false;
	
	Inventory* inventory = player->GetInventory();
	if (!inventory) return false;
	
	// 월드 오브젝트로부터 획득 가능한 아이템 드롭 목록 계산
	std::vector<std::pair<GameObjectID, UINT>> drops = CalculateDropsFromObject(worldObject);
	
	bool anyItemAdded = false;
	for (const auto& drop : drops) {
		// TODO: ObjectManager에 GetItemDefinition 함수 추가 필요
		// 임시로 최소 처리
		/*
		std::shared_ptr<Item> itemDef = ObjectManager::GetInstance()->GetItemDefinition(drop.first);
		if (itemDef) {
			if (inventory->AddItem(itemDef, drop.second)) {
				anyItemAdded = true;
				OutputDebugStringW((L"InventoryManager: 아이템 획득 - ID: " + std::to_wstring(drop.first) + L", 개수: " + std::to_wstring(drop.second) + L"\n").c_str());
			}
		}
		*/
		OutputDebugStringW((L"InventoryManager: 아이템 획득 실패 - ID: " + std::to_wstring(drop.first) + L", 개수: " + std::to_wstring(drop.second) + L"\n").c_str());
	}
	
	// 아이템 획득 이벤트 발생 (실제 획득 여부와 관계없이 드롭된 아이템은 진행도에 반영)
	if (!drops.empty()) {
		SceneType currentScene = SceneManager::GetInstance()->GetCurrentSceneType();
		for (const auto& drop : drops) {
			GameProgressManager::GetInstance()->OnItemCollected(drop.first, drop.second, currentScene);
		}
	}
	
	return anyItemAdded;
}

// 아이템 사용 처리 (도구 장착 토글, 음식 섭취 등)
bool InventoryManager::TryUseItem(Player* player, int slotIndex) {
	if (!player) return false;
	
	Inventory* inventory = player->GetInventory();
	if (!inventory) return false;
	
	const ItemSlot& slot = inventory->GetSlot(slotIndex);
	if (slot.IsEmpty()) return false;
	
	Item* item = slot.item;
	
	// 도구: 장착/해제 토글
	if (dynamic_cast<Tool*>(item)) {
		player->ToggleEquipItem(slotIndex);
		return true;
	}

	// 음식
	GameObjectID itemID = item->GetID();
	int healAmount = GetFoodHealAmount(itemID);
	if (healAmount > 0) {
		if (inventory->RemoveItem(slotIndex, 1)) {
			player->Heal(healAmount);
			return true;
		}
		return false;
	}
	
	// TODO: 그 외 아이템 타입별 사용 로직
	OutputDebugStringW((L"InventoryManager: 아이템 사용 - 슬롯 " + std::to_wstring(slotIndex) + L"\n").c_str());
	return true;
}

int InventoryManager::GetFoodHealAmount(GameObjectID itemID) const {
	switch (itemID) {
	case GOID_ITEM_BERRY:                return 8;
	case GOID_ITEM_MEAT:                 return 12;
	case GOID_ITEM_SMALL_MEAT:          return 8;
	case GOID_ITEM_MONSTER_MEAT:         return 10;
	case GOID_ITEM_COOKED_MONSTER_MEAT: return 20;
	case GOID_ITEM_COOKED_SMALL_MEAT:   return 12;
	case GOID_ITEM_COOKED_MEAT:         return 25;
	default:                             return 0;
	}
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
		// TODO: 월드에 아이템 게임오브젝트 생성
		// 플레이어 위치 근처에 아이템 드롭
		
		OutputDebugStringW((L"InventoryManager: 아이템 버림 - 슬롯 " + std::to_wstring(slotIndex) + L", 개수: " + std::to_wstring(count) + L"\n").c_str());
		return true;
	}
	
	return false;
}

// 아이템 제작 시스템
bool InventoryManager::TryCraftItem(Player* player, GameObjectID targetItemID) {
	if (!player) return false;
	
	Inventory* inventory = player->GetInventory();
	if (!inventory) return false;
	
	// 제작 레시피 확인
	const std::map<UINT, UINT>* recipe = GetCraftingRecipe(targetItemID);
	if (!recipe) {
		OutputDebugStringW(L"InventoryManager: 제작 레시피가 존재하지 않습니다.\n");
		return false;
	}
	
	// 필요한 재료가 있는지 확인
	if (!inventory->CheckHasEnoughItems(*recipe)) {
		OutputDebugStringW(L"InventoryManager: 인벤토리에 필요한 재료가 부족합니다.\n");
		return false;
	}
	
	// 재료 소모 후 제작 아이템 생성 및 인벤토리 추가
	if (inventory->ConsumeItems(*recipe)) {
		GameObject* itemObj = ObjectManager::GetInstance()->CreateGameObject(targetItemID, 0.0f, 0.0f, nullptr, false);
		Item* item = dynamic_cast<Item*>(itemObj);
		if (item) {
			if (inventory->AddItem(item, 1)) {
				OutputDebugStringW((L"InventoryManager: 아이템 제작 완료 - ID: " + std::to_wstring(targetItemID) + L"\n").c_str());
				return true;
			}
			delete item;
		}
		OutputDebugStringW((L"InventoryManager: 아이템 제작 완료 실패 (인벤토리 추가 실패) - ID: " + std::to_wstring(targetItemID) + L"\n").c_str());
		return false;
	}
	return false;
}

// 아이템 교환 시스템
bool InventoryManager::TryTradeItem(Player* player, const std::map<UINT, UINT>& giveItems, const std::map<UINT, UINT>& receiveItems) {
	if (!player) return false;
	
	Inventory* inventory = player->GetInventory();
	if (!inventory) return false;
	
	// 보유한 아이템이 충분한지 확인
	if (!inventory->CheckHasEnoughItems(giveItems)) {
		OutputDebugStringW(L"InventoryManager: 교환할 아이템이 부족합니다.\n");
		return false;
	}
	
	// 받을 아이템의 인벤토리 공간 확인
	// TODO: 인벤토리 공간 확인 로직 필요
	
	// 아이템 교환 처리
	if (inventory->ConsumeItems(giveItems)) {
		bool allReceived = true;
		for (const auto& receive : receiveItems) {
			// TODO: ObjectManager에 GetItemDefinition 함수 추가 필요
			// 임시로 최소 처리
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
			// TODO: 롤백 처리 필요
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

// 아이템 제작 레시피 로드
void InventoryManager::LoadCraftingRecipes() {
	LoadCraftingRecipesFromTable(m_craftingRecipes);
	OutputDebugStringW(L"InventoryManager: 제작 레시피 로드 완료\n");
}

bool InventoryManager::HasCraftingRecipe(GameObjectID itemID) const {
	return m_craftingRecipes.find(itemID) != m_craftingRecipes.end();
}

const std::map<UINT, UINT>* InventoryManager::GetCraftingRecipe(GameObjectID itemID) const {
	auto it = m_craftingRecipes.find(itemID);
	if (it != m_craftingRecipes.end()) {
		return &(it->second);
	}
	return nullptr;
}

// 월드 오브젝트로부터 드롭 가능한 아이템 목록
std::vector<std::pair<GameObjectID, UINT>> InventoryManager::CalculateDropsFromObject(GameObject* worldObject) {
	std::vector<std::pair<GameObjectID, UINT>> drops;
	
	if (!worldObject) return drops;
	
	// TODO: 오브젝트 타입에 따른 드롭 아이템 목록 설정
	// 예시: 나무 -> 통나무 1개, 나뭇가지 1개
	//       돌 -> 돌 2-3개
	//       금 -> 금 1개
	
	// 임시 하드 코드
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
