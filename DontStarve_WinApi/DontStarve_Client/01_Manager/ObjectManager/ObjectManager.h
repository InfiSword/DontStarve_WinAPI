#pragma once



class GameObject;
class Player;
class Item;

namespace ResourcePathUtils { struct ObjectResourceDef; }  // 전방 선언

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



	// 게임오브젝트 및 UI(UIElement) 통합 관리

	void AddGameObject(GameObject* pObj);

	void RemoveGameObject(GameObject* pObj);

	void ClearAllObjects();



	bool IsScreenPointBlockedByUI(float screenX, float screenY) const;



	// ID로 오브젝트 찾기 (월드 오브젝트·UI 공통)

	GameObject* FindGameObject(GameObjectID id);

	template <typename T>
	T* FindGameObject(GameObjectID id) { return dynamic_cast<T*>(FindGameObject(id)); }


	Player* GetPlayer() const;



	const std::vector<GameObject*>& GetGameObjects() const { return m_gameObjects; }

	

	// 게임오브젝트 생성 (모든 GameObject와 Item 통합 관리)

	// addToManager: true면 ObjectManager에 추가, false면 생성만 하고 추가하지 않음 (인벤토리 아이템 등)

	GameObject* CreateGameObject(GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* resourceData = nullptr, bool addToManager = true);



private:

	// 팩토리 맵 패턴: GameObjectID -> 생성 함수 (모든 GameObject와 Item 통합)

	using GameObjectFactoryFunc = std::function<GameObject*(GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data)>;

	

	std::map<GameObjectID, GameObjectFactoryFunc> m_gameObjectFactories;

	

	// 팩토리 맵 초기화

	void InitializeFactories();

	

	// 게임오브젝트 순회 공통화 (중복 제거)

	void ForEachObject(std::function<void(GameObject*)> fn);

	void ForEachEnabledObject(std::function<void(GameObject*)> fn);

	

	std::vector<GameObject*> m_gameObjects;

	std::vector<GameObject*> m_pendingDeletions; // 삭제 지연 큐



	Player* m_cachedPlayer; // 플레이어 캐시



	// 삭제 지연 처리

	void ProcessPendingDeletions();

};



