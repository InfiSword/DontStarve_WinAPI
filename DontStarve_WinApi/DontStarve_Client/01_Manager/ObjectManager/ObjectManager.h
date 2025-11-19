#pragma once

// 필수 전방 선언만 유지 (실제로 헤더에서 사용하는 타입만)
class GameObject;
class Player;
class Item;

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

	// 플레이어 캐시된 포인터 반환 함수
	Player* GetPlayer() const;

	// 게임오브젝트 반환
	const std::vector<GameObject*>& GetGameObjects() const { return m_gameObjects; }
	std::vector<GameObject*>& GetGameObjects() { return m_gameObjects; }
	
	// 실제 바운드 박스를 이용한 정확한 충돌 검사
	GameObject* FindObjectAtPositionWithBounds(float x, float y);

	// ========================================
	// 팩토리 패턴: 객체 생성
	// ========================================
	// 헤더에서는 구체적인 클래스(Tree, Rock 등)를 몰라도 됨
	// 모든 생성 로직은 cpp 파일에 캡슐화됨
	// ========================================
	
	// 게임오브젝트 생성 (ResourceManager 활용)
	GameObject* CreateGameObject(GameObjectID id, float x, float y, const GameObjectData* resourceData = nullptr);
	
	// 아이템 생성 (ResourceManager 활용, shared_ptr로 자동 메모리 관리)
	std::shared_ptr<Item> CreateItem(GameObjectID itemID);

	// 바운드 표시 토글
	void ToggleBoundsDisplay() { m_showBounds = !m_showBounds; }
	bool IsBoundsDisplayEnabled() const { return m_showBounds; }
	void RenderBounds();

private:
	// ========================================
	// 팩토리 맵 패턴: GameObjectID -> 생성 함수
	// ========================================
	// switch 문 대신 맵을 사용하여 확장성 향상
	// 새 오브젝트 추가 시 맵에 등록만 하면 됨
	// ========================================
	using GameObjectFactoryFunc = std::function<GameObject*(GameObjectID id, float x, float y, const GameObjectData* data)>;
	using ItemFactoryFunc = std::function<std::shared_ptr<Item>(GameObjectID id, const GameObjectData* data)>;
	
	std::map<GameObjectID, GameObjectFactoryFunc> m_gameObjectFactories;
	std::map<GameObjectID, ItemFactoryFunc> m_itemFactories;
	
	// 팩토리 맵 초기화 (Init()에서 호출)
	void InitializeFactories();
	
	std::vector<GameObject*> m_gameObjects;
	Player* m_cachedPlayer; // 플레이어 캐시
	bool m_showBounds; // 바운드 표시 여부
};
