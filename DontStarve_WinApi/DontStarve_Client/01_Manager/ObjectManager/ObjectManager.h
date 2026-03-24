#pragma once

class GameObject;
class Player;
class Entity;
class Building;
class Item;
class UIImage;
class UIButton;
class UIText;
class MenuUI;

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

	void AddGameObject(GameObject* pObj);
	void RemoveGameObject(GameObject* pObj);
	void ClearAllObjects();
	bool IsScreenPointBlockedByUI(float screenX, float screenY) const;

	// ID로 오브젝트 찾기 
	GameObject* FindGameObject(GameObjectID id);
	template <typename T>
	T* FindGameObject(GameObjectID id) { return dynamic_cast<T*>(FindGameObject(id)); }

	Player* GetPlayer() const;
	const std::vector<GameObject*>& GetGameObjects() const { return m_gameObjects; }

	// 게임오브젝트 생성 헬퍼
	Entity*   CreateEntity(GameObjectID id, float x, float y);
	Item*     CreateItem(GameObjectID id, float x, float y);
	Building* CreateBuilding(GameObjectID id, float x, float y);

	// UI 생성 헬퍼
	UIButton* CreateButton(GameObjectID id, float width, float height, const std::wstring& normalPath, const std::wstring& hoverPath, float anchorX, float anchorY, float x, float y, std::function<void()> onClick);
	UIImage*  CreateImage(GameObjectID id, float width, float height, RenderLayer layer, const std::wstring& path, float depth, float anchorX, float anchorY, float x, float y);
	UIText*   CreateText(GameObjectID id, float width, float height, const std::wstring& text, Gdiplus::Color color, float fontSize, float anchorX, float anchorY, float x, float y, float sortKey = 0 );

private:

	// 팩토리 맵 패턴
	using EntityFactoryFunc   = std::function<Entity*(GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data)>;
	using ItemFactoryFunc     = std::function<Item*(GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data)>;
	using BuildingFactoryFunc = std::function<Building*(GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data)>;

	std::map<GameObjectID, EntityFactoryFunc>   m_entityFactories;
	std::map<GameObjectID, ItemFactoryFunc>     m_itemFactories;
	std::map<GameObjectID, BuildingFactoryFunc> m_buildingFactories;

	// 팩토리 맵 초기화
	void InitializeFactories();

	template<typename T>
	T* PostCreate(T* pObj, const ResourcePathUtils::ObjectResourceDef* data);

	void ForEachObject(std::function<void(GameObject*)> fn);
	void ForEachEnabledObject(std::function<void(GameObject*)> fn);

	std::vector<GameObject*> m_gameObjects;
	std::vector<GameObject*> m_pendingDeletions; // 삭제 지연 큐

	Player* m_cachedPlayer; // 플레이어 캐시

	// 삭제 지연 처리
	void ProcessPendingDeletions();

};



