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
	
	// 게임오브젝트 생성 (모든 GameObject와 Item 통합 관리)
	// addToManager: true면 ObjectManager에 추가, false면 생성만 하고 추가하지 않음 (인벤토리 아이템 등)
	GameObject* CreateGameObject(GameObjectID id, float x, float y, const GameObjectData* resourceData = nullptr, bool addToManager = true);

	// 바운드 표시 토글
	void ToggleBoundsDisplay() { m_showBounds = !m_showBounds; }
	bool IsBoundsDisplayEnabled() const { return m_showBounds; }
	void RenderBounds();

private:
	// ========================================
	// 팩토리 맵 패턴: GameObjectID -> 생성 함수 (모든 GameObject와 Item 통합)
	// ========================================
	using GameObjectFactoryFunc = std::function<GameObject*(GameObjectID id, float x, float y, const GameObjectData* data)>;
	
	std::map<GameObjectID, GameObjectFactoryFunc> m_gameObjectFactories;
	
	// 팩토리 맵 초기화
	void InitializeFactories();
	
	std::vector<GameObject*> m_gameObjects;
	Player* m_cachedPlayer; // 플레이어 캐시
	bool m_showBounds; // 바운드 표시 여부
};
