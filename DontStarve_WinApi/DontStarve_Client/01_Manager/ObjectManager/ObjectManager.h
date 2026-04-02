#pragma once
#include "../../../Header/SingleTon.h"

class GameObject;
class Player;
class Entity;
class Building;
class Item;
class UIImage;
class UIButton;
class UIText;
class MenuUI;
class HPUI;
class GameOverUI;
class GameClearUI;
class IntroNoticeUI;

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

	// 공간 분할 (Spatial Partitioning) 관련
	void UpdateObjectGridCell(GameObject* pObj);
	void GetObjectsInRect(const Gdiplus::RectF& rect, std::vector<GameObject*>& outObjects);

	// ID로 오브젝트 찾기 
	GameObject* FindGameObject(GameObjectID id);
	template <typename T>
	T* FindGameObject(GameObjectID id) { 
		GameObject* obj = FindGameObject(id);
		if (!obj) return nullptr;
		// dynamic_cast 대신 static_cast를 쓰기 위해 호출 측에서 타입을 확신할 때 사용하거나, 
		// 안전을 위해 여기서 간단한 타입 체크를 수행할 수 있습니다.
		return static_cast<T*>(obj); 
	}

	Player* GetPlayer() const;
	const std::vector<GameObject*>& GetWorldObjects() const { return m_worldObjects; }
	const std::vector<GameObject*>& GetUIObjects() const { return m_uiObjects; }

	// 게임오브젝트 생성 헬퍼
	Entity*   CreateEntity(GameObjectID id, float x, float y);
	Item*     CreateItem(GameObjectID id, float x, float y);
	Building* CreateBuilding(GameObjectID id, float x, float y);

	// UI 생성 헬퍼
	UIButton* CreateButton(GameObjectID id, float width, float height, const std::wstring& normalPath, const std::wstring& hoverPath, float anchorMinX, float anchorMinY, float anchorMaxX, float anchorMaxY, float x, float y, std::function<void()> onClick);
	UIImage*  CreateImage(GameObjectID id, float width, float height, RenderLayer layer, const std::wstring& path, float depth, float anchorMinX, float anchorMinY, float anchorMaxX, float anchorMaxY, float x, float y);
	UIText*   CreateText(GameObjectID id, float width, float height, const std::wstring& text, Gdiplus::Color color, float fontSize, Gdiplus::FontStyle fontStyle, float anchorMinX, float anchorMinY, float anchorMaxX, float anchorMaxY, float x, float y, float sortKey = 0, Gdiplus::StringAlignment hAlign = Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignment vAlign = Gdiplus::StringAlignmentCenter);

	// 특수 UI 생성 헬퍼
	MenuUI* CreateMenuUI();
	HPUI*   CreateHPUI(Entity* pTarget, const std::wstring& name, float width, float height, Gdiplus::Color bgColor, Gdiplus::Color barColor, Gdiplus::Color nameColor, float anchorX, float anchorY, float pivotX, float pivotY, float x, float y, float bgSortKey, float barSortKey, bool usePortrait, bool useName);
	GameOverUI* CreateGameOverUI();
	GameClearUI* CreateGameClearUI();
	IntroNoticeUI* CreateIntroNoticeUI();

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

	std::vector<GameObject*> m_worldObjects;
	std::vector<GameObject*> m_uiObjects;
	std::vector<GameObject*> m_pendingDeletions; // 삭제 지연 큐

	// 공간 분할용 그리드
	static constexpr int GRID_CELL_SIZE = 256;
	static constexpr int GRID_WIDTH = (MAP_WIDTH * TILE_SIZE / GRID_CELL_SIZE) + 1;
	static constexpr int GRID_HEIGHT = (MAP_HEIGHT * TILE_SIZE / GRID_CELL_SIZE) + 1;
	std::vector<GameObject*> m_spatialGrid[GRID_WIDTH][GRID_HEIGHT];

	void AddToGrid(GameObject* pObj);
	void RemoveFromGrid(GameObject* pObj);

	Player* m_cachedPlayer; // 플레이어 캐시

	// 삭제 지연 처리
	void ProcessPendingDeletions();

};
