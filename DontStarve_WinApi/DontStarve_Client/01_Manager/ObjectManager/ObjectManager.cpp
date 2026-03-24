#include "99_Default/pch.h"
#include "ObjectManager.h"
#include "../ResourceManager/ResourceManager.h"
#include "../RenderManager/RenderManager.h"
#include "../CameraManager/CameraManager.h"
#include "../../02_GameObject/GameObject.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../02_GameObject/Item/Item.h"
#include "../../02_GameObject/Entity/Enviorment/Tree.h"
#include "../../02_GameObject/Entity/Enviorment/Rock.h"
#include "../../02_GameObject/Entity/Enviorment/Grass.h"
#include "../../02_GameObject/Entity/Enviorment/BerryBush.h"
#include "../../02_GameObject/Entity/Enviorment/Sapling.h"
#include "../../02_GameObject/Entity/Monster/Pig.h"
#include "../../02_GameObject/Entity/Monster/Spider.h"
#include "../../02_GameObject/Entity/Monster/Boss_SpiderQueen.h"
#include "../../02_GameObject/Entity/Monster/Hound.h"
#include "../../02_GameObject/Entity/Monster/Boss_RedHound.h"
#include "../../02_GameObject/Entity/Monster/Boss_IceHound.h"
#include "../../02_GameObject/Building/PigHouse.h"
#include "../../02_GameObject/Building/SpiderEgg.h"
#include "../../02_GameObject/Item/Ingredient.h"
#include "../../02_GameObject/Item/Tool/Tool.h"
#include "../../02_GameObject/UI/MenuUI.h"
#include "../../02_GameObject/UI/UIImage.h"
#include "../../02_GameObject/UI/UIButton.h"
#include "../../02_GameObject/UI/UIText.h"
#include "../../02_GameObject/UI/UIElement.h"
#include "../../02_GameObject/Component/Transform/RectTransform.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../../02_GameObject/Component/Collider/BoxCollider.h"
#include "../../02_GameObject/Component/Collider/CircleCollider.h"

ObjectManager::ObjectManager()
{
	m_cachedPlayer = nullptr;
}

ObjectManager::~ObjectManager()
{
	ClearAllObjects();
}

void ObjectManager::Init()
{
	InitializeFactories();
}

void ObjectManager::ForEachObject(std::function<void(GameObject*)> fn)
{
	for (GameObject* obj : m_gameObjects)
		if (obj) fn(obj);
}

void ObjectManager::ForEachEnabledObject(std::function<void(GameObject*)> fn)
{
	for (GameObject* obj : m_gameObjects)
		if (obj && obj->IsEnabled()) fn(obj);
}

void ObjectManager::LateInit()
{
	ForEachObject([](GameObject* obj) { obj->LateInit(); });
}

void ObjectManager::Update(float deltaTime)
{
	for (GameObject* obj : m_gameObjects) {
		if (obj && obj->IsEnabled())
		{
			obj->Update(deltaTime);
		}
	}
}

void ObjectManager::LateUpdate()
{
	ForEachEnabledObject([](GameObject* obj) { obj->LateUpdate(); });

	ProcessPendingDeletions();
}

void ObjectManager::Render()
{
	// 카메라에 보이는 게임오브젝트 렌더링
	CameraManager* cameraManager = CameraManager::GetInstance();
	if (cameraManager) {
		cameraManager->RenderVisibleGameObjects();
	}

	// UI 렌더링 (월드 오브젝트는 카메라가 처리)
	for (GameObject* obj : m_gameObjects) {
		UIElement* ui = dynamic_cast<UIElement*>(obj);
		if (ui && ui->IsEnabled()) {
			ui->Render();
		}
	}

}

void ObjectManager::Release()
{
	ClearAllObjects();
}

void ObjectManager::AddGameObject(GameObject* pObj)
{
	if (!pObj) return;

	// 중복 추가 체크
	auto it = std::find(m_gameObjects.begin(), m_gameObjects.end(), pObj);
	if (it != m_gameObjects.end()) return;

	// 삭제 대기 중인지 확인
	auto pendingIt = std::find(m_pendingDeletions.begin(), m_pendingDeletions.end(), pObj);
	if (pendingIt != m_pendingDeletions.end()) return;

	m_gameObjects.push_back(pObj);

	// UI는 스크린 공간이므로 카메라 가시 목록에 넣지 않음
	if (!dynamic_cast<UIElement*>(pObj)) {
		CameraManager* cam = CameraManager::GetInstance();
		if (cam) cam->TryAddToVisibleIfInViewport(pObj);
	}

	Player* player = dynamic_cast<Player*>(pObj);
	if (player) {
		m_cachedPlayer = player;
	}
}

void ObjectManager::RemoveGameObject(GameObject* pObj)
{
	if (!pObj) return;

	auto pendingIt = std::find(m_pendingDeletions.begin(), m_pendingDeletions.end(), pObj);
	if (pendingIt != m_pendingDeletions.end()) return;

	pObj->SetActive(false);

	CameraManager* cam = CameraManager::GetInstance();
	if (cam) cam->RemoveFromVisibleObjects(pObj);
	m_pendingDeletions.push_back(pObj);
}

bool ObjectManager::IsScreenPointBlockedByUI(float screenX, float screenY) const
{

	// 활성화된 UIElement의 RectTransform 바운딩 박스 검사
	for (const GameObject* obj : m_gameObjects) {
		const UIElement* element = dynamic_cast<const UIElement*>(obj);
		if (!element || !element->IsEnabled()) continue;
		RectTransform* rt = element->GetRectTransform();
		if (!rt) continue;
		Gdiplus::RectF bounds = rt->GetScreenBoundingBox();
		if (bounds.Width > 0.0f && bounds.Height > 0.0f && bounds.Contains(screenX, screenY))
			return true;
	}
	return false;
}

GameObject* ObjectManager::FindGameObject(GameObjectID id)
{
	for (GameObject* obj : m_gameObjects) {
		if (obj && obj->GetID() == id) return obj;
	}
	return nullptr;
}

void ObjectManager::ProcessPendingDeletions()
{
	// 게임 오브젝트 삭제 처리
	if (!m_pendingDeletions.empty()) {
		for (GameObject* obj : m_pendingDeletions) {
			auto it = std::find(m_gameObjects.begin(), m_gameObjects.end(), obj);
			if (it != m_gameObjects.end()) {
				if (obj == m_cachedPlayer) m_cachedPlayer = nullptr;
				(*it)->Release();
				Utils::SafeDelete(*it);
				m_gameObjects.erase(it);
			}
		}
		m_pendingDeletions.clear();
	}
}

void ObjectManager::ClearAllObjects()
{
	ProcessPendingDeletions();
	m_cachedPlayer = nullptr;
	ForEachObject([](GameObject* obj) { obj->Release(); Utils::SafeDelete(obj); });
	m_gameObjects.clear();
	m_gameObjects.shrink_to_fit();
	m_pendingDeletions.clear();
	m_pendingDeletions.shrink_to_fit();
}

// 플레이어 캐시된 포인터 반환 함수
Player* ObjectManager::GetPlayer() const
{
	return m_cachedPlayer;
}

void ObjectManager::InitializeFactories()
{
	// 동일 팩토리 함수를 여러 ID에 등록하는 헬퍼
	auto registerEntityIds = [this](const std::vector<GameObjectID>& ids, EntityFactoryFunc fn) {
		for (GameObjectID id : ids) m_entityFactories[id] = fn;
		};

	// 플레이어 (동일 람다)
	registerEntityIds({ GOID_PLAYER_WILSON, GOID_PLAYER_WILLOW, GOID_PLAYER_WOLFGANG }, [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Entity* {
		return new Player(x, y, id, data->baseDir, data->imageName);
		});

	// 나무 
	registerEntityIds({ GOID_NORMAL_TREE_SHORT, GOID_NORMAL_TREE_NORMAL, GOID_NORMAL_TREE_TALL }, [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Entity* {
		ColliderType colliderType = (data && data->hasCollider) ? data->colliderType : COLLIDER_BOX;
		return new Tree(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName, colliderType);
		});

	// 돌 
	registerEntityIds({ GOID_NORMAL_ROCK, GOID_GOLD_ROCK }, [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Entity* {
		ColliderType colliderType = (data && data->hasCollider) ? data->colliderType : COLLIDER_BOX;
		return new Rock(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName, colliderType);
		});

	// 환경 오브젝트 
	m_entityFactories[GOID_NORMAL_GRASS] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Entity* {
		ColliderType colliderType = (data && data->hasCollider) ? data->colliderType : COLLIDER_BOX;
		return new Grass(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName, colliderType);
		};
	m_entityFactories[GOID_NORMAL_SAPLING] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Entity* {
		ColliderType colliderType = (data && data->hasCollider) ? data->colliderType : COLLIDER_BOX;
		return new Sapling(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName, colliderType);
		};
	m_entityFactories[GOID_BERRY_TREE] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Entity* {
		ColliderType colliderType = (data && data->hasCollider) ? data->colliderType : COLLIDER_BOX;
		return new BerryBush(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName, colliderType);
		};

	// 몬스터 
	m_entityFactories[GOID_MONSTER_PIG] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Entity* {
		ColliderType colliderType = (data && data->hasCollider) ? data->colliderType : COLLIDER_BOX;
		return new Pig(id, x, y, data->pivotX, data->pivotY, DIR_DOWN, data->baseDir, data->imageName, colliderType);
		};
	registerEntityIds({ GOID_MONSTER_SPIDER, GOID_MONSTER_WARRIOR_SPIDER }, [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Entity* {
		ColliderType colliderType = (data && data->hasCollider) ? data->colliderType : COLLIDER_BOX;
		return new Spider(id, x, y, data->pivotX, data->pivotY, DIR_DOWN, data->baseDir, data->imageName, colliderType);
		});
	m_entityFactories[GOID_MONSTER_QUEEN_SPIDER] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Entity* {
		ColliderType colliderType = (data && data->hasCollider) ? data->colliderType : COLLIDER_BOX;
		return new Boss_SpiderQueen(id, x, y, data->pivotX, data->pivotY, DIR_DOWN, data->baseDir, data->imageName, colliderType);
		};
	m_entityFactories[GOID_MONSTER_HOUNDDOG] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Entity* {
		ColliderType colliderType = (data && data->hasCollider) ? data->colliderType : COLLIDER_BOX;
		return new Hound(id, x, y, data->pivotX, data->pivotY, DIR_DOWN, data->baseDir, data->imageName, colliderType);
		};
	m_entityFactories[GOID_MONSTER_REDHOUNDDOG] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Entity* {
		ColliderType colliderType = (data && data->hasCollider) ? data->colliderType : COLLIDER_BOX;
		return new Boss_RedHound(id, x, y, data->pivotX, data->pivotY, DIR_DOWN, data->baseDir, data->imageName, colliderType);
		};
	m_entityFactories[GOID_MONSTER_ICEHOUNDDOG] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Entity* {
		ColliderType colliderType = (data && data->hasCollider) ? data->colliderType : COLLIDER_BOX;
		return new Boss_IceHound(id, x, y, data->pivotX, data->pivotY, DIR_DOWN, data->baseDir, data->imageName, colliderType);
		};

	// 건물 
	m_buildingFactories[GOID_BUILDING_PIGHOUSE] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Building* {
		return new PigHouse(id, x, y, data->pivotX, data->pivotY, DIR_DOWN, data->baseDir, data->imageName);
		};
	auto registerBuildingIds = [this](const std::vector<GameObjectID>& ids, BuildingFactoryFunc fn) {
		for (GameObjectID id : ids) m_buildingFactories[id] = fn;
		};
	registerBuildingIds({ GOID_BUILDING_SPIDER_SMALLEGG, GOID_BUILDING_SPIDER_NORMALEGG, GOID_BUILDING_SPIDER_TALLEGG, GOID_BUILDING_SPIDER_SACEGG }, [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Building* {
		return new SpiderEgg(id, x, y, data->pivotX, data->pivotY, DIR_DOWN, data->baseDir, data->imageName);
		});

	// 아이템
	auto itemFactory = [](const wchar_t* name, const wchar_t* desc) -> ItemFactoryFunc {
		return [name, desc](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Item* {
			return new Ingredient(id, name, desc, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName);
			};
		};

	m_itemFactories[GOID_ITEM_NORMAL_TREE_LOG] = itemFactory(L"LOG", L"A Log.");
	m_itemFactories[GOID_ITEM_NORMAL_TWIGS] = itemFactory(L"Twigs", L"A common twig.");
	m_itemFactories[GOID_ITEM_NORMAL_ROCK] = itemFactory(L"Rock Shard", L"A small piece of rock.");
	m_itemFactories[GOID_ITEM_CUT_NORMAL_GRASS] = itemFactory(L"Cut Grass", L"Bundled grass, good for crafting.");
	m_itemFactories[GOID_ITEM_GOLD_ROCK] = itemFactory(L"Gold", L"Shiny and valuable.");
	m_itemFactories[GOID_ITEM_ROPE] = itemFactory(L"Rope", L"Useful for crafting.");
	m_itemFactories[GOID_ITEM_CUT_NORMAL_STONE] = itemFactory(L"Cut Stone", L"Stone blocks for building.");
	m_itemFactories[GOID_ITEM_MEAT] = itemFactory(L"Meat", L"Fresh meat.");
	m_itemFactories[GOID_ITEM_BERRY] = itemFactory(L"Berry", L"Sweet and nutritious.");
	m_itemFactories[GOID_ITEM_WOOD_2] = itemFactory(L"Wooden Plank", L"Planks for crafting.");
	m_itemFactories[GOID_ITEM_SMALL_MEAT] = itemFactory(L"Small Meat", L"A small piece of meat.");
	m_itemFactories[GOID_ITEM_MONSTER_MEAT] = itemFactory(L"Monster Meat", L"Strange meat from a monster.");
	m_itemFactories[GOID_ITEM_COOKED_MONSTER_MEAT] = itemFactory(L"Cooked Monster Meat", L"Cooked monster meat.");
	m_itemFactories[GOID_ITEM_COOKED_SMALL_MEAT] = itemFactory(L"Cooked Small Meat", L"Cooked small meat.");
	m_itemFactories[GOID_ITEM_COOKED_MEAT] = itemFactory(L"Cooked Meat", L"A nicely cooked piece of meat.");

	// 도구
	struct ToolDef { std::wstring name; std::wstring desc; };
	auto GetToolDef = [](GameObjectID id) -> ToolDef {
		switch (id) {
		case GOID_TOOL_GOLDEN_PICKAXE: return { L"Golden Scythe", L"A golden scythe for harvesting." };
		case GOID_TOOL_HAM_BAT:       return { L"Ham Bat",       L"A weapon made from ham." };
		case GOID_TOOL_PICKAXE:       return { L"Pickaxe",       L"Mines rocks and ores." };
		case GOID_TOOL_SPEAR:         return { L"Spear",         L"A simple spear for combat." };
		case GOID_TOOL_SWAP_SPEAR:    return { L"Swap Spear",    L"A lightning-infused spear." };
		case GOID_TOOL_TORCH:         return { L"Torch",         L"Provides light in darkness." };
		case GOID_TOOL_RED_AXE:       return { L"Red Axe",       L"Cuts down trees." };
		case GOID_TOOL_SWAP_AXE:      return { L"Swap Axe",      L"An axe with special properties." };
		case GOID_TOOL_HALBERD:       return { L"Halberd",       L"A heavy polearm for long reach." };
		case GOID_TOOL_HAMMER:        return { L"Hammer",        L"Used for deconstructing structures." };
		default:                      return { L"Tool",          L"" };
		}
		};

	auto toolFactory = [GetToolDef](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> Item* {
		ToolDef def = GetToolDef(id);
		const ToolDataUtils::ToolStatsEntry* stats = ToolDataUtils::GetToolStats(id);
		int damage = stats->damage;
		float attackRange = stats->attackRange;
		return new Tool(id, def.name, def.desc, data->baseDir, data->imageName, damage, attackRange);
		};

	auto registerItemIds = [this](const std::vector<GameObjectID>& ids, ItemFactoryFunc fn) {
		for (GameObjectID id : ids) m_itemFactories[id] = fn;
		};
	registerItemIds({
		GOID_TOOL_GOLDEN_PICKAXE, GOID_TOOL_HAM_BAT, GOID_TOOL_PICKAXE,
		GOID_TOOL_SPEAR, GOID_TOOL_SWAP_SPEAR, GOID_TOOL_TORCH,
		GOID_TOOL_RED_AXE, GOID_TOOL_SWAP_AXE, GOID_TOOL_HALBERD,
		GOID_TOOL_HAMMER
		}, toolFactory);
}

// ========================================
// 팩토리 패턴: 게임오브젝트 생성 및 관리 (모든 GameObject와 Item 통합)
// ========================================
template<typename T>
T* ObjectManager::PostCreate(T* pObj, const ResourcePathUtils::ObjectResourceDef* data)
{
	if (!pObj) return nullptr;

	// 오브젝트 데이터의 콜라이더 정보를 컴포넌트로 첨부 (월드 배치 오브젝트만)
	if (data && data->hasCollider) {
		if (data->colliderType == COLLIDER_BOX)
		{
			BoxCollider* col = pObj->template AddComponent<BoxCollider>();
			col->SetObjectCollider(
				data->colliderOffsetX,
				data->colliderOffsetY,
				data->colliderWidth,
				data->colliderHeight
			);
		}
		else if (data->colliderType == COLLIDER_CIRCLE)
		{
			CircleCollider* col = pObj->template AddComponent<CircleCollider>();
			col->SetObjectCollider(
				data->colliderCenterX,
				data->colliderCenterY,
				data->colliderRadius
			);
		}
	}
	AddGameObject(pObj);
	pObj->Init();

	return pObj;
}

Entity* ObjectManager::CreateEntity(GameObjectID id, float x, float y)
{
	const ResourcePathUtils::ObjectResourceDef* data = ResourceManager::GetInstance()->GetObjectResourceInfo(id);
	auto it = m_entityFactories.find(id);
	if (it != m_entityFactories.end()) {
		return PostCreate(it->second(id, x, y, data), data);
	}
	OutputDebugStringW((L"ObjectManager: 알 수 없는 Entity ID - ID: " + std::to_wstring(id) + L"\n").c_str());
	return nullptr;
}

Item* ObjectManager::CreateItem(GameObjectID id, float x, float y)
{
	const ResourcePathUtils::ObjectResourceDef* data = ResourceManager::GetInstance()->GetObjectResourceInfo(id);
	auto it = m_itemFactories.find(id);
	if (it != m_itemFactories.end()) {
		return PostCreate(it->second(id, x, y, data), data);
	}
	OutputDebugStringW((L"ObjectManager: 알 수 없는 Item ID - ID: " + std::to_wstring(id) + L"\n").c_str());
	return nullptr;
}

Building* ObjectManager::CreateBuilding(GameObjectID id, float x, float y)
{
	const ResourcePathUtils::ObjectResourceDef* data = ResourceManager::GetInstance()->GetObjectResourceInfo(id);
	auto it = m_buildingFactories.find(id);
	if (it != m_buildingFactories.end()) {
		return PostCreate(it->second(id, x, y, data), data);
	}
	OutputDebugStringW((L"ObjectManager: 알 수 없는 Building ID - ID: " + std::to_wstring(id) + L"\n").c_str());
	return nullptr;
}

UIButton* ObjectManager::CreateButton(GameObjectID id, float width, float height, const std::wstring& normalPath, const std::wstring& hoverPath, float anchorX, float anchorY, float x, float y, std::function<void()> onClick)
{
	auto* resMgr = ResourceManager::GetInstance();
	auto normalSprite = resMgr->LoadSprite(normalPath);
	auto hoverSprite = resMgr->LoadSprite(hoverPath);

	UIButton* button = new UIButton(id, width, height, normalSprite, hoverSprite, anchorX, anchorY, anchorX, anchorY, x, y);
	if (button) {
		button->SetOnClickCallback(onClick);
		AddGameObject(button);
	}
	return button;
}

UIImage* ObjectManager::CreateImage(GameObjectID id, float width, float height, RenderLayer layer, const std::wstring& path, float depth, float anchorX, float anchorY, float x, float y)
{
	UIImage* image = new UIImage(id, width, height, layer, path, depth, anchorX, anchorY, anchorX, anchorY, x, y);
	if (image) {
		AddGameObject(image);
	}
	return image;
}

UIText* ObjectManager::CreateText(GameObjectID id, float width, float height, const std::wstring& text, Gdiplus::Color color, float fontSize, float anchorX, float anchorY, float x, float y, float sortKey)
{
	RenderLayer layer = LAYER_UI_FOREGROUND;
	std::wstring fontName = L"Arial";

	UIText* uiText = new UIText(id, width, height, text, color, layer, sortKey, fontName, fontSize, Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter, anchorX, anchorY, anchorX, anchorY, x, y);
	if (uiText) {
		AddGameObject(uiText);
	}
	return uiText;
}


