#pragma once

class Player;
class GameObject;
class Item;
class Inventory;

// InventoryManager는 외부 환경과 인벤토리 시스템을 연결하는 인터페이스 역할을 담당합니다.
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

	// 외부 환경과 인벤토리 상호작용 인터페이스
	
	// 월드 오브젝트로부터 아이템 획득
	bool TryGainItemFromWorldObject(Player* player, GameObject* worldObject);
	
	// 아이템 사용 처리 (도구 착용, 소비 아이템 사용 등)
	bool TryUseItem(Player* player, int slotIndex);
	
	// 아이템 버리기 (월드에 드롭)
	bool TryDropItem(Player* player, int slotIndex, UINT count = 1);
	
	// 아이템 조합 시스템
	bool TryCraftItem(Player* player, GameObjectID targetItemID);
	
	// 아이템 교환 시스템 (상점, NPC 등)
	bool TryTradeItem(Player* player, const std::map<UINT, UINT>& giveItems, const std::map<UINT, UINT>& receiveItems);
	
	// 인벤토리 전체 저장 기능
	void SaveInventoryToFile(Player* player, const std::wstring& filePath);
	void LoadInventoryFromFile(Player* player, const std::wstring& filePath);

private:
	// 이미지 경로를 받아서 Gdiplus::Bitmap*을 반환하는 함수 (캐시 기능)
	Gdiplus::Bitmap* GetBitmapForPath(const std::wstring& imagePath);
	
	// 아이템 조합 레시피 관리
	void LoadCraftingRecipes();
	bool HasCraftingRecipe(GameObjectID itemID) const;
	std::map<UINT, UINT> GetCraftingRecipe(GameObjectID itemID) const;
	
	// 월드 오브젝트에서 얻을 수 있는 아이템 계산
	std::vector<std::pair<GameObjectID, UINT>> CalculateDropsFromObject(GameObject* worldObject);

private:
	std::map<std::wstring, Gdiplus::Bitmap*> m_bitmapCache;
	
	// 아이템 조합 레시피 데이터 (아이템ID -> 필요 재료)
	std::map<GameObjectID, std::map<UINT, UINT>> m_craftingRecipes;
}; 