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
#include "../../02_GameObject/Entity/Monster/Boss_Hound.h"
#include "../../02_GameObject/Building/PigHouse.h"
#include "../../02_GameObject/Building/SpiderEgg.h"
#include "../../02_GameObject/Item/Ingredient.h"
#include "../../02_GameObject/Item/Tool/Axe/Axe.h"

#include "../../02_GameObject/Component/Transform/Transform.h"

ObjectManager::ObjectManager()
{
	m_cachedPlayer = nullptr;
	m_showBounds = false; // 바운드 표시 기본값은 false (성능 최적화)
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
	ForEachEnabledObject([deltaTime](GameObject* obj) { obj->Update(deltaTime); });
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
	
	// 바운드 표시가 활성화되어 있으면 모든 게임오브젝트의 바운드를 그림
	if (m_showBounds) {
		RenderBounds();
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
	
	CameraManager* cam = CameraManager::GetInstance();
	if (cam) cam->RemoveFromVisibleObjects(pObj);
	m_pendingDeletions.push_back(pObj);
}

void ObjectManager::ProcessPendingDeletions()
{
	if (m_pendingDeletions.empty()) {
		return;
	}

	// 삭제 대기 중인 모든 객체 처리
	for (GameObject* obj : m_pendingDeletions)
	{
		if (!obj) continue;

		// m_gameObjects에서 찾아서 제거
		auto it = std::find(m_gameObjects.begin(), m_gameObjects.end(), obj);
		if (it != m_gameObjects.end())
		{
			// 플레이어 캐시 해제
			if (obj == m_cachedPlayer) {
				m_cachedPlayer = nullptr;
			}

			(*it)->Release();
			Utils::SafeDelete(*it);
			m_gameObjects.erase(it);
		}
	}

	// 삭제 지연 큐 비우기
	m_pendingDeletions.clear();
}

void ObjectManager::ClearAllObjects()
{
	ProcessPendingDeletions();
	m_cachedPlayer = nullptr;
	ForEachObject([](GameObject* obj) { obj->Release(); Utils::SafeDelete(obj); });
	m_gameObjects.clear();
	m_pendingDeletions.clear();
}

void ObjectManager::InitializeObjects()
{
	ForEachObject([](GameObject* obj) { obj->Init(); });
}

// 플레이어 캐시된 포인터 반환 함수
Player* ObjectManager::GetPlayer() const
{
	return m_cachedPlayer;
}


GameObject* ObjectManager::FindObjectAtPositionWithBounds(float x, float y)
{
	CameraManager* cameraManager = CameraManager::GetInstance();
	if (!cameraManager) return nullptr;

	GameObject* found = nullptr;
	ForEachEnabledObject([&](GameObject* obj) {
		if (found) return;
		Gdiplus::RectF objBounds = cameraManager->GetSpriteBoundingBox(obj);
		if (x >= objBounds.X && x <= objBounds.X + objBounds.Width &&
			y >= objBounds.Y && y <= objBounds.Y + objBounds.Height)
			found = obj;
	});
	return found;
}

void ObjectManager::InitializeFactories()
{
	// 동일 팩토리 함수를 여러 ID에 등록하는 헬퍼
	auto registerIds = [this](const std::vector<GameObjectID>& ids, GameObjectFactoryFunc fn) {
		for (GameObjectID id : ids) m_gameObjectFactories[id] = fn;
	};

	// 플레이어 (동일 람다)
	registerIds({ GOID_PLAYER_WILSON, GOID_PLAYER_WILLOW, GOID_PLAYER_WOLFGANG }, [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new Player(x, y, id, data->baseDir, data->imageName);
	});

	// 나무 (동일 람다) - 맵 파일의 피벗값 사용
	registerIds({ GOID_NORMAL_TREE_SHORT, GOID_NORMAL_TREE_NORMAL, GOID_NORMAL_TREE_TALL }, [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new Tree(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName);
	});

	// 돌 (동일 람다) - 맵 파일의 피벗값 사용
	registerIds({ GOID_NORMAL_ROCK, GOID_GOLD_ROCK }, [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new Rock(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName);
	});

	// 환경 오브젝트 (클래스별 1개씩) - 맵 파일의 피벗값 사용
	m_gameObjectFactories[GOID_NORMAL_GRASS] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new Grass(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName);
	};
	m_gameObjectFactories[GOID_NORMAL_SAPLING] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new Sapling(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName);
	};
	m_gameObjectFactories[GOID_BERRY_TREE] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new BerryBush(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName);
	};

	// 몬스터 - 맵 파일의 피벗값 사용
	m_gameObjectFactories[GOID_MONSTER_PIG] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new Pig(id, x, y, data->pivotX, data->pivotY, data->imageName);
	};
	registerIds({ GOID_MONSTER_SPIDER, GOID_MONSTER_WARRIOR_SPIDER }, [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new Spider(id, x, y, data->pivotX, data->pivotY, data->imageName);
	});
	m_gameObjectFactories[GOID_MONSTER_QUEEN_SPIDER] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new Boss_SpiderQueen(id, x, y, data->pivotX, data->pivotY, data->imageName);
	};
	m_gameObjectFactories[GOID_MONSTER_HOUNDDOG] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new Hound(id, x, y, data->pivotX, data->pivotY, data->imageName);
	};
	registerIds({ GOID_MONSTER_REDHOUNDDOG, GOID_MONSTER_ICEHOUNDDOG }, [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new Boss_Hound(id, x, y, data->pivotX, data->pivotY, data->imageName);
	});

	// 건물 - 맵 파일의 피벗값 사용
	m_gameObjectFactories[GOID_BUILDING_PIGHOUSE] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new PigHouse(id, x, y, data->pivotX, data->pivotY, DIR_DOWN, data->baseDir, data->imageName);
	};
	registerIds({ GOID_BUILDING_SPIDER_SMALLEGG, GOID_BUILDING_SPIDER_NORMALEGG, GOID_BUILDING_SPIDER_TALLEGG }, [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new SpiderEgg(id, x, y, data->pivotX, data->pivotY, DIR_DOWN, data->baseDir, data->imageName);
	});

	// 아이템 (이름/설명만 다른 Item 생성 패턴) - 맵 파일의 피벗값 사용
	auto itemFactory = [](const wchar_t* name, const wchar_t* desc) {
		return [name, desc](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
			return new Item(GOBJ_ITEM, id, name, desc, data->baseDir, data->imageName, x, y, data->pivotX, data->pivotY);
		};
	};
	m_gameObjectFactories[GOID_ITEM_NORMAL_TREE_LOG] = itemFactory(L"LOG", L"A Log.");
	m_gameObjectFactories[GOID_ITEM_NORMAL_TWIGS] = itemFactory(L"Twigs", L"A common twig.");
	m_gameObjectFactories[GOID_ITEM_NORMAL_ROCK] = itemFactory(L"Rock Shard", L"A small piece of rock.");
	m_gameObjectFactories[GOID_ITEM_CUT_NORMAL_GRASS] = itemFactory(L"Cut Grass", L"Bundled grass, good for crafting.");
	m_gameObjectFactories[GOID_ITEM_GOLD_ROCK] = itemFactory(L"Gold", L"Shiny and valuable.");
	m_gameObjectFactories[GOID_ITEM_ROPE] = itemFactory(L"Rope", L"Useful for crafting.");
	m_gameObjectFactories[GOID_ITEM_MEAT] = itemFactory(L"Meat", L"Fresh meat.");
	m_gameObjectFactories[GOID_ITEM_BERRY] = itemFactory(L"Berry", L"Sweet and nutritious.");
	m_gameObjectFactories[GOID_ITEM_AXE] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		std::wstring path = ResourcePathUtils::BuildResourcePath(data->baseDir, data->imageName);
		return new Axe(id, L"Axe", L"Cuts down trees.", path, 100.0f, 1.0f);
	};
}

// ========================================
// 팩토리 패턴: 게임오브젝트 생성 및 관리 (모든 GameObject와 Item 통합)
// ========================================
GameObject* ObjectManager::CreateGameObject(GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* resourceData, bool addToManager)
{
	const ResourcePathUtils::ObjectResourceDef* data = resourceData ? resourceData : ResourceManager::GetInstance()->GetObjectResourceInfo(id);
	if (!data) {
		OutputDebugStringW((L"ObjectManager: 리소스 정보 없음 - ID: " + std::to_wstring(id) + L"\n").c_str());
		return nullptr;
	}

	auto it = m_gameObjectFactories.find(id);
	if (it != m_gameObjectFactories.end()) {
		GameObject* newObj = it->second(id, x, y, data);
		
		// 생성된 게임오브젝트를 오브젝트매니저에 추가 (GameObject 생명주기 관리)
		if (newObj) {
			
			if (addToManager) {
				AddGameObject(newObj);
				OutputDebugStringW((L"ObjectManager: 새로운 게임오브젝트 생성 완료 - ID: " + std::to_wstring(id) + L" at (" + std::to_wstring(x) + L", " + std::to_wstring(y) + L")\n").c_str());
			}
			else {
				OutputDebugStringW((L"ObjectManager: 새로운 게임오브젝트 생성 완료 (인벤토리용) - ID: " + std::to_wstring(id) + L"\n").c_str());
			}
		}
		else {
			OutputDebugStringW((L"ObjectManager: 새로운 게임오브젝트 생성 실패 - ID: " + std::to_wstring(id) + L"\n").c_str());
		}
		
		return newObj;
	}
	else {
		OutputDebugStringW((L"ObjectManager: 알 수 없는 GameObjectID - ID: " + std::to_wstring(id) + L"\n").c_str());
		return nullptr;
	}
}


void ObjectManager::RenderBounds()
{
	RenderManager* renderManager = RenderManager::GetInstance();
	CameraManager* cameraManager = CameraManager::GetInstance();
	if (!renderManager || !cameraManager) return;

	ForEachEnabledObject([renderManager, cameraManager](GameObject* obj) {
		Gdiplus::RectF objBounds = cameraManager->GetSpriteBoundingBox(obj);
		Gdiplus::PointF screenLeft = cameraManager->WorldToScreen(objBounds.X, objBounds.Y);
		Gdiplus::PointF screenRight = cameraManager->WorldToScreen(
			objBounds.X + objBounds.Width, objBounds.Y + objBounds.Height);
		Gdiplus::RectF boundsRect(screenLeft.X, screenLeft.Y,
			screenRight.X - screenLeft.X, screenRight.Y - screenLeft.Y);

		Gdiplus::Color boundsColor;
		switch (obj->GetType()) {
			case GOBJ_ITEM:         boundsColor = Gdiplus::Color(255, 0, 255, 0); break;
			case GOBJ_NATURAL_ENVIR: boundsColor = Gdiplus::Color(255, 0, 150, 255); break;
			case GOBJ_MONSTER:      boundsColor = Gdiplus::Color(255, 255, 0, 0); break;
			default:                boundsColor = Gdiplus::Color(255, 255, 255, 255); break;
		}
		renderManager->AddDrawCommand(boundsRect, boundsColor, 3.0f, LAYER_DEBUG_OVERLAY, 9999.0f);
		Gdiplus::Color bgColor = Gdiplus::Color(50, boundsColor.GetR(), boundsColor.GetG(), boundsColor.GetB());
		renderManager->AddFillRectangleCommand(boundsRect, bgColor, LAYER_DEBUG_OVERLAY, 9998.0f);
	});
}
