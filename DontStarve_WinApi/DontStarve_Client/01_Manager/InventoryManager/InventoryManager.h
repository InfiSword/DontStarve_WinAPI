#pragma once

class Player;
class GameObject;
class Item;
class Inventory;

// InventoryManager는 플레이어 인벤토리 시스템을 관리하는 싱글톤 매니저 클래스입니다.
class InventoryManager : public CSingleTon<InventoryManager>
{
	friend class CSingleTon<InventoryManager>;
public:
	InventoryManager();
	~InventoryManager();

	void Init();
	void LateInit();
	void Update(float deltaTime);
	void LateUpdate();
	void Render();
	void Release();

	// 플레이어 인벤토리 관련 인터페이스
	
	// 월드 오브젝트로부터 아이템 획득
	bool TryGainItemFromWorldObject(Player* player, GameObject* worldObject);
	
	// 아이템 사용 처리 (도구 사용, 음식 아이템 사용 등)
	bool TryUseItem(Player* player, int slotIndex);
	
	// 아이템 버리기 (월드에 드롭)
	bool TryDropItem(Player* player, int slotIndex, UINT count = 1);
	
	// 아이템 제작 시스템
	bool TryCraftItem(Player* player, GameObjectID targetItemID);
	
	// 아이템 교환 시스템 (거래, NPC 등)
	bool TryTradeItem(Player* player, const std::map<UINT, UINT>& giveItems, const std::map<UINT, UINT>& receiveItems);
	
	// 인벤토리 전체 저장 기능
	void SaveInventoryToFile(Player* player, const std::wstring& filePath);
	void LoadInventoryFromFile(Player* player, const std::wstring& filePath);

	// 아이템 제작 레시피 조회 (CraftingUI에서 사용)
	bool HasCraftingRecipe(GameObjectID itemID) const;
	const std::map<UINT, UINT>* GetCraftingRecipe(GameObjectID itemID) const;

private:
	// 아이템 제작 레시피 관리
	void LoadCraftingRecipes();
	
	// 월드 오브젝트로부터 드롭 가능한 아이템 목록
	std::vector<std::pair<GameObjectID, UINT>> CalculateDropsFromObject(GameObject* worldObject);

private:
	// 아이템 제작 레시피 데이터 (아이템ID -> 필요 재료)
	std::map<GameObjectID, std::map<UINT, UINT>> m_craftingRecipes;
}; 
