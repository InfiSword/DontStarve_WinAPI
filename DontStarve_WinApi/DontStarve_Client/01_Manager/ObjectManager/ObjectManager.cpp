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
#include "../../02_GameObject/Item/Tool/Tool.h"
#include "../../02_GameObject/Item/Tool/Weapon/Weapon.h"
#include "../../02_GameObject/Item/Tool/Pickaxe/Pickaxe.h"
#include "../../02_GameObject/Item/Tool/Torch/Torch.h"

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
	m_gameObjects.shrink_to_fit(); // 벡터 capacity도 해제
	m_pendingDeletions.clear();
	m_pendingDeletions.shrink_to_fit();
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
		return new Pig(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName);
	};
	registerIds({ GOID_MONSTER_SPIDER, GOID_MONSTER_WARRIOR_SPIDER }, [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new Spider(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName);
	});
	m_gameObjectFactories[GOID_MONSTER_QUEEN_SPIDER] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new Boss_SpiderQueen(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName);
	};
	m_gameObjectFactories[GOID_MONSTER_HOUNDDOG] = [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new Hound(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName);
	};
	registerIds({ GOID_MONSTER_REDHOUNDDOG, GOID_MONSTER_ICEHOUNDDOG }, [](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		return new Boss_Hound(id, x, y, data->pivotX, data->pivotY, data->baseDir, data->imageName);
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
	
	// 도구 테이블 - 모든 도구 ID로 name/desc만 조회 (내구도·효율는 Tool/Axe 생성자 기본값 또는 Axe는 GetAxeStats 사용)
	struct ToolDef { std::wstring name; std::wstring desc; };
	auto GetToolDef = [](GameObjectID id) -> ToolDef {
		switch (id) {
			case GOID_TOOL_GOLDEN_SCYTHE: return { L"Golden Scythe", L"A golden scythe for harvesting." };
			case GOID_TOOL_HAM_BAT:       return { L"Ham Bat", L"A weapon made from ham." };
			case GOID_TOOL_PICKAXE:       return { L"Pickaxe", L"Mines rocks and ores." };
			case GOID_TOOL_SPEAR:         return { L"Spear", L"A simple spear for combat." };
			case GOID_TOOL_SWAP_SPEAR:    return { L"Swap Spear", L"A lightning-infused spear." };
			case GOID_TOOL_TORCH:         return { L"Torch", L"Provides light in darkness." };
			case GOID_TOOL_RED_AXE:       return { L"Red Axe", L"Cuts down trees." };
			case GOID_TOOL_SWAP_AXE:      return { L"Swap Axe", L"An axe with special properties." };
			default:                      return { L"Tool", L"" };
		}
	};

	// Tool 팩토리: GetToolDef로 name/desc 조회 후, ID에 따라 Weapon / Pickaxe / Torch / Axe / Tool 생성
	auto toolFactory = [GetToolDef](GameObjectID id, float x, float y, const ResourcePathUtils::ObjectResourceDef* data) -> GameObject* {
		ToolDef def = GetToolDef(id);
		if (id == GOID_TOOL_RED_AXE || id == GOID_TOOL_SWAP_AXE)
			return new Axe(id, def.name, def.desc, data->baseDir, data->imageName);
		if (id == GOID_TOOL_GOLDEN_SCYTHE || id == GOID_TOOL_HAM_BAT || id == GOID_TOOL_SPEAR || id == GOID_TOOL_SWAP_SPEAR)
			return new Weapon(id, def.name, def.desc, data->baseDir, data->imageName);
		if (id == GOID_TOOL_PICKAXE)
			return new Pickaxe(id, def.name, def.desc, data->baseDir, data->imageName);
		if (id == GOID_TOOL_TORCH)
			return new Torch(id, def.name, def.desc, data->baseDir, data->imageName);
		return new Tool(id, def.name, def.desc, data->baseDir, data->imageName, 0.0f);
	};

	// 다른 도구들 설정 (Tool 팩토리 등록): GOID_TOOL_GOLDEN_SCYTHE, GOID_TOOL_HAM_BAT, GOID_TOOL_PICKAXE, GOID_TOOL_SPEAR, GOID_TOOL_SWAP_SPEAR, GOID_TOOL_TORCH, GOID_TOOL_RED_AXE, GOID_TOOL_SWAP_AXE
	m_gameObjectFactories[GOID_TOOL_GOLDEN_SCYTHE] = toolFactory;
	m_gameObjectFactories[GOID_TOOL_HAM_BAT] = toolFactory;
	m_gameObjectFactories[GOID_TOOL_PICKAXE] = toolFactory;
	m_gameObjectFactories[GOID_TOOL_SPEAR] = toolFactory;
	m_gameObjectFactories[GOID_TOOL_SWAP_SPEAR] = toolFactory;
	m_gameObjectFactories[GOID_TOOL_TORCH] = toolFactory;
	m_gameObjectFactories[GOID_TOOL_RED_AXE] = toolFactory;
	m_gameObjectFactories[GOID_TOOL_SWAP_AXE] = toolFactory;
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
		
		// 생성된 게임오브젝트를 오브ject매니저에 추가 (GameObject 생명주기 관리)
		if (newObj) {
			if (addToManager) {
				AddGameObject(newObj);
				newObj->Init();

				// 맵 데이터의 콜라이더 정보를 컴포넌트로 첨부 (월드 배치 오브젝트만)
				if (data->hasCollider) {
					if (data->colliderType == COLLIDER_BOX) {
						BoxCollider* col = newObj->AddComponent<BoxCollider>();
						col->SetBoundingBox(
							data->colliderOffsetX,
							data->colliderOffsetY,
							data->colliderWidth,
							data->colliderHeight
						);
					}
					else if (data->colliderType == COLLIDER_CIRCLE) {
						newObj->AddComponent<CircleCollider>(
							data->colliderCenterX,
							data->colliderCenterY,
							data->colliderRadius
						);
					}
				}
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
