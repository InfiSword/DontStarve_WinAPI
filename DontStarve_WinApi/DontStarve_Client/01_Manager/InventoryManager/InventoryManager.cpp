#include "99_Default/pch.h"
#include "InventoryManager.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../02_GameObject/GameObject.h"
#include "../../02_GameObject/Item/Item.h"
#include "../../02_GameObject/Item/Tool/Tool.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../../02_GameObject/UI/Inventory.h"
#include "../../02_GameObject/UI/CraftingRecipe.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../01_Manager/GameProgressManager/GameProgressManager.h"


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

void InventoryManager::Render() {}

void InventoryManager::Release() {
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
	auto* objMgr = ObjectManager::GetInstance();

	for (const auto& drop : drops) {
		// ID 기반으로 임시 아이템 객체 생성 (AddItem 내부에서 데이터만 추출 후 파괴됨)
		Item* tempItem = objMgr->CreateItem(drop.first, 0.0f, 0.0f);
		if (tempItem) {
			if (inventory->AddItem(tempItem, drop.second)) {
				anyItemAdded = true;
				OutputDebugStringW((L"InventoryManager: 아이템 획득 - ID: " + std::to_wstring(drop.first) + L", 개수: " + std::to_wstring(drop.second) + L"\n").c_str());
			}
			else {
				// 인벤토리에 못 들어갔으면 임시 객체 수동 삭제
				objMgr->RemoveGameObject(tempItem);
				OutputDebugStringW((L"InventoryManager: 아이템 획득 실패 (인벤토리 가득 참) - ID: " + std::to_wstring(drop.first) + L"\n").c_str());
			}
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
	
	GameObjectID itemID = slot.id;
	
	// 도구 여부 확인 (DataTable 사용)
	if (DataTable::GetToolInfo(itemID) != nullptr) {
		player->ToggleEquipItem(slotIndex);
		return true;
	}

	// 음식 여부 확인 및 처리
	int healAmount = GetFoodHealAmount(itemID);
	if (healAmount > 0) {
		if (inventory->RemoveItem(slotIndex, 1)) {
			player->Heal(healAmount);
			return true;
		}
		return false;
	}
	
	// TODO: 그 외 아이템 타입별 사용 로직
	OutputDebugStringW((L"InventoryManager: 아이템 사용 - 슬롯 " + std::to_wstring(slotIndex) + L", ID: " + std::to_wstring(itemID) + L"\n").c_str());
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

	GameObjectID itemID = slot.id;
	
	// 인벤토리에서 아이템 제거
	if (inventory->RemoveItem(slotIndex, count)) {
		// 월드에 실제 아이템 게임오브젝트 생성
		Transform* playerTrans = player->GetComponent<Transform>();
		if (playerTrans) {
			// 버리는 위치: 플레이어 좌표에서 y좌표만 0.1f 앞에
			ObjectManager::GetInstance()->CreateItem(itemID, playerTrans->GetX(), playerTrans->GetY() - 0.1f);
		}
		
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
		auto* objMgr = ObjectManager::GetInstance();
		Item* item = objMgr->CreateItem(targetItemID, 0.0f, 0.0f);
		if (item) {
			if (inventory->AddItem(item, 1)) {
				OutputDebugStringW((L"InventoryManager: 아이템 제작 완료 - ID: " + std::to_wstring(targetItemID) + L"\n").c_str());
				return true;
			}
			objMgr->RemoveGameObject(item);
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
	
	// 아이템 교환 처리
	if (inventory->ConsumeItems(giveItems)) {
		for (const auto& receive : receiveItems) {
			inventory->AddItemByID((GameObjectID)receive.first, receive.second);
		}
		OutputDebugStringW(L"InventoryManager: 아이템 교환 완료\n");
		return true;
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

void InventoryManager::ResetPlayerInventory(Player* player)
{
	if (!player) return;

	Inventory* inventory = player->GetInventory();
	if (!inventory) return;

	// 인벤토리의 모든 아이템 제거
	inventory->ClearAllItems();

	OutputDebugStringW(L"InventoryManager: 플레이어 인벤토리 초기화 완료\n");
}
