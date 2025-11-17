#pragma once

class GameObject;
class Player;
class Tree;
class Rock;
class Grass;
class Monster;
class Item;
class Building;
class Ingredient;

class ObjectManager : public CSingleTon<ObjectManager>
{
	friend class CSingleTon<ObjectManager>;
public:
	ObjectManager();
	~ObjectManager();

	void Init();
	void LateInit();
	void Update(float deltaTime);
	void LateUpdate();
	void Render();
	void Release();

	// 게임오브젝트 관리
	void AddGameObject(GameObject* pObj);
	void RemoveGameObject(GameObject* pObj);
	void ClearAllObjects();

	// 게임오브젝트 초기화
	void InitializeObjects();

	// 플레이어와 상호작용 가능한 오브젝트를 찾아서 처리
	Player* GetPlayer() const;

	// 게임오브젝트 반환
	const std::vector<GameObject*>& GetGameObjects() const { return m_gameObjects; }
	std::vector<GameObject*>& GetGameObjects() { return m_gameObjects; }
	
	// 이미지 테두리 기반으로 정확한 충돌 검사
	GameObject* FindObjectAtPositionWithBounds(float x, float y);

	// === 팩토리 패턴 함수들 ===
	// 게임오브젝트 생성 (ResourceManager 연동)
	GameObject* CreateGameObject(GameObjectID id, float x, float y, const GameObjectData* resourceData = nullptr);
	
	// 아이템 생성 (ResourceManager 연동)
	std::shared_ptr<Item> CreateItem(GameObjectID itemID);

	// 테두리 표시 기능
	void ToggleBoundsDisplay() { m_showBounds = !m_showBounds; }
	bool IsBoundsDisplayEnabled() const { return m_showBounds; }
	void RenderBounds();
private:
	std::vector<GameObject*> m_gameObjects;
	Player* m_cachedPlayer; // 플레이어 캐시
	bool m_showBounds; // 테두리 표시 여부
	
	// 테두리 렌더링 함수
};
