#include "../../99_Default/pch.h"
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

ObjectManager::ObjectManager()
{
	m_cachedPlayer = nullptr;
	m_showBounds = true; // 바운드 표시 기본값은 false
}

ObjectManager::~ObjectManager()
{
	ClearAllObjects();
}

void ObjectManager::Init()
{
	InitializeFactories();
}

void ObjectManager::LateInit()
{
	// 모든 게임오브젝트에 LateInit 호출
	for (GameObject* obj : m_gameObjects)
	{
		if (obj)
		{
			obj->LateInit();
		}
	}
}

void ObjectManager::Update(float deltaTime)
{
	// 모든 게임오브젝트 업데이트
	for (GameObject* obj : m_gameObjects)
	{
		if (obj && obj->GetActive())
		{
			obj->Update(deltaTime);
		}
	}
}

void ObjectManager::LateUpdate()
{
	// 모든 게임오브젝트에 LateUpdate 호출
	for (GameObject* obj : m_gameObjects)
	{
		if (obj && obj->GetActive())
		{
			obj->LateUpdate();
		}
	}
}

void ObjectManager::Render()
{
	// 카메라에 보이는 게임오브젝트 렌더링 (플레이어 우선)
	RenderManager::GetInstance()->RenderVisibleGameObjects();
	
	// 바운드 표시가 활성화되면 모든 게임오브젝트의 바운드를 그림
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
	if (pObj)
	{
		m_gameObjects.push_back(pObj);

		Player* player = dynamic_cast<Player*>(pObj);
		if (player) {
			m_cachedPlayer = player;
			OutputDebugStringW((L"ObjectManager: 플레이어 캐싱 완료 - ID: " + std::to_wstring(player->GetID()) + L"\n").c_str());
		}

		OutputDebugStringW((L"ObjectManager: 새로운 게임오브젝트 추가 완료 - ID: " + std::to_wstring(pObj->GetID()) + L", 전체 게임오브젝트 수: " + std::to_wstring(m_gameObjects.size()) + L"\n").c_str());
	}
}

void ObjectManager::RemoveGameObject(GameObject* pObj)
{
	if (!pObj) return;

	auto it = std::find(m_gameObjects.begin(), m_gameObjects.end(), pObj);
	if (it != m_gameObjects.end())
	{
		// 플레이어 캐시 해제
		if (pObj == m_cachedPlayer) {
			m_cachedPlayer = nullptr;
		}

		(*it)->Release();
		SafeDelete(*it);
		m_gameObjects.erase(it);
	}
}

void ObjectManager::ClearAllObjects()
{
	for (GameObject* obj : m_gameObjects)
	{
		if (obj)
		{
			obj->Release();
			SafeDelete(obj);
		}
	}
	m_gameObjects.clear();
}

void ObjectManager::InitializeObjects()
{
	// 모든 게임오브젝트 초기화
	for (GameObject* obj : m_gameObjects)
	{
		if (obj)
		{
			obj->Init();
		}
	}
	if (m_cachedPlayer)
		m_cachedPlayer->Init();
}

// 플레이어 캐시된 포인터 반환 함수
Player* ObjectManager::GetPlayer() const
{
	return m_cachedPlayer;
}


GameObject* ObjectManager::FindObjectAtPositionWithBounds(float x, float y)
{
	for (GameObject* obj : m_gameObjects)
	{
		if (obj && obj->GetActive() && obj->CanInteract())
		{
			// 오브젝트의 실제 바운드 박스 크기 계산
			float objWidth = obj->GetWidth();
			float objHeight = obj->GetHeight();
			float objX = obj->GetX();
			float objY = obj->GetY();
			
			// 오브젝트의 피벗 값 가져오기
			float pivotX = obj->GetPivotX();
			float pivotY = obj->GetPivotY();
			
			// 피벗을 고려한 바운드 박스 경계 계산
			float left = objX - (objWidth * pivotX);                                                 
			float right = objX + (objWidth * (1.0f - pivotX));
			float top = objY - (objHeight * pivotY);
			float bottom = objY + (objHeight * (1.0f - pivotY));
			
			// 클릭한 위치가 오브젝트의 실제 바운드 박스 안에 있는지 확인
			if (x >= left && x <= right && y >= top && y <= bottom)
			{
				return obj;
			}
		}
	}
	return nullptr;
}

// 팩토리 맵 초기화: GameObjectID -> 생성 함수 등록
void ObjectManager::InitializeFactories()
{
	// 플레이어 타입
	m_gameObjectFactories[GOID_PLAYER_WILSON] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Player(x, y, id, data->objectAssetBaseDirectory, data->assetImageName);
	};
	m_gameObjectFactories[GOID_PLAYER_WILLOW] = m_gameObjectFactories[GOID_PLAYER_WILSON];
	m_gameObjectFactories[GOID_PLAYER_WOLFGANG] = m_gameObjectFactories[GOID_PLAYER_WILSON];

	// 나무 타입
	m_gameObjectFactories[GOID_NORMAL_TREE_SHORT] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Tree(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_NORMAL_TREE_NORMAL] = m_gameObjectFactories[GOID_NORMAL_TREE_SHORT];
	m_gameObjectFactories[GOID_NORMAL_TREE_TALL] = m_gameObjectFactories[GOID_NORMAL_TREE_SHORT];

	// 돌 타입
	m_gameObjectFactories[GOID_NORMAL_ROCK] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Rock(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_GOLD_ROCK] = m_gameObjectFactories[GOID_NORMAL_ROCK];

	// 풀 타입 - 환경 오브젝트
	m_gameObjectFactories[GOID_NORMAL_GRASS] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Grass(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data->objectAssetBaseDirectory, data->assetImageName);
	};
	m_gameObjectFactories[GOID_NORMAL_SAPLING] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Sapling(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data->objectAssetBaseDirectory, data->assetImageName);
	};
	m_gameObjectFactories[GOID_BERRY_TREE] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new BerryBush(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data->objectAssetBaseDirectory, data->assetImageName);
	};

	// 몬스터 타입
	m_gameObjectFactories[GOID_MONSTER_PIG] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Pig(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_MONSTER_SPIDER] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Spider(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_MONSTER_WARRIOR_SPIDER] = m_gameObjectFactories[GOID_MONSTER_SPIDER];
	m_gameObjectFactories[GOID_MONSTER_QUEEN_SPIDER] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Boss_SpiderQueen(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_MONSTER_HOUNDDOG] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Hound(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_MONSTER_REDHOUNDDOG] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Boss_Hound(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_MONSTER_ICEHOUNDDOG] = m_gameObjectFactories[GOID_MONSTER_REDHOUNDDOG];

	// 건물 타입
	m_gameObjectFactories[GOID_BUILDING_PIGHOUSE] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new PigHouse(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			DIR_DOWN, data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_BUILDING_SPIDER_SMALLEGG] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new SpiderEgg(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			DIR_DOWN, data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_BUILDING_SPIDER_NORMALEGG] = m_gameObjectFactories[GOID_BUILDING_SPIDER_SMALLEGG];
	m_gameObjectFactories[GOID_BUILDING_SPIDER_TALLEGG] = m_gameObjectFactories[GOID_BUILDING_SPIDER_SMALLEGG];

	// 아이템 팩토리 등록 (GameObjectFactory에 통합)
	// 인벤토리 아이템은 x, y 좌표를 사용하지 않지만 통합을 위해 파라미터로 받음
	m_gameObjectFactories[GOID_ITEM_NORMAL_TREE_LOG] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Item(GOBJ_ITEM, id, L"LOG", L"A Log.", data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"", x, y);
	};
	m_gameObjectFactories[GOID_ITEM_NORMAL_TWIGS] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Item(GOBJ_ITEM, id, L"Twigs", L"A common twig.", data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"", x, y);
	};
	m_gameObjectFactories[GOID_ITEM_NORMAL_ROCK] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Item(GOBJ_ITEM, id, L"Rock Shard", L"A small piece of rock.", data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"", x, y);
	};
	m_gameObjectFactories[GOID_ITEM_CUT_NORMAL_GRASS] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Item(GOBJ_ITEM, id, L"Cut Grass", L"Bundled grass, good for crafting.", data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"", x, y);
	};
	m_gameObjectFactories[GOID_ITEM_GOLD_ROCK] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Item(GOBJ_ITEM, id, L"Gold", L"Shiny and valuable.", data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"", x, y);
	};
	m_gameObjectFactories[GOID_ITEM_ROPE] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Item(GOBJ_ITEM, id, L"Rope", L"Useful for crafting.", data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"", x, y);
	};
	m_gameObjectFactories[GOID_ITEM_MEAT] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Item(GOBJ_ITEM, id, L"Meat", L"Fresh meat.", data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"", x, y);
	};
	m_gameObjectFactories[GOID_ITEM_BERRY] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Item(GOBJ_ITEM, id, L"Berry", L"Sweet and nutritious.", data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"", x, y);
	};
	m_gameObjectFactories[GOID_ITEM_AXE] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		// Axe는 Tool을 상속받아 x, y를 받지 않으므로 무시하고 생성
		return new Axe(id, L"Axe", L"Cuts down trees.", data ? (data->objectAssetBaseDirectory + L"/" + data->assetImageName) : L"", 100.0f, 1.0f);
	};
}

// ========================================
// 팩토리 패턴: 게임오브젝트 생성 및 관리 (모든 GameObject와 Item 통합)
// ========================================
GameObject* ObjectManager::CreateGameObject(GameObjectID id, float x, float y, const GameObjectData* resourceData, bool addToManager)
{
	// ResourceManager에서 리소스 정보 가져오기
	const GameObjectData* data = resourceData;
	if (!data) {
		data = ResourceManager::GetInstance()->GetObjectResourceInfo(id);
	}

	// 팩토리 맵에서 생성 함수 찾기
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
	if (!renderManager) return;
	
	CameraManager* cameraManager = CameraManager::GetInstance();
	if (!cameraManager) return;
	
	for (GameObject* obj : m_gameObjects)
	{
		if (obj && obj->GetActive() && obj->CanInteract())
		{
			// 오브젝트의 실제 바운드 박스 크기 계산
			float objWidth = obj->GetWidth();
			float objHeight = obj->GetHeight();
			float objX = obj->GetX();
			float objY = obj->GetY();
			
			// 오브젝트의 피벗 값 가져오기
			float pivotX = obj->GetPivotX();
			float pivotY = obj->GetPivotY();
			
			// 피벗을 고려한 바운드 박스 경계 계산
			float left = objX - (objWidth * pivotX);
			float right = objX + (objWidth * (1.0f - pivotX));
			float top = objY - (objHeight * pivotY);
			float bottom = objY + (objHeight * (1.0f - pivotY));
			
			// 월드 좌표를 스크린 좌표로 변환
			Gdiplus::PointF screenLeft = cameraManager->WorldToScreen(left, top);
			Gdiplus::PointF screenRight = cameraManager->WorldToScreen(right, bottom);
			
			// 바운드 사각형 생성
			Gdiplus::RectF boundsRect(
				screenLeft.X,
				screenLeft.Y,
				screenRight.X - screenLeft.X,
				screenRight.Y - screenLeft.Y
			);
			
			// 오브젝트 타입에 따라 다른 색상 사용 (디버그 구분용)
			Gdiplus::Color boundsColor;
			switch (obj->GetType()) {
				case GOBJ_ITEM:
					boundsColor = Gdiplus::Color(255, 0, 255, 0); // 아이템 - 초록색
					break;
				case GOBJ_NATURAL_ENVIR:
					boundsColor = Gdiplus::Color(255, 0, 150, 255); // 자연 환경 - 파란색
					break;
				case GOBJ_MONSTER:
					boundsColor = Gdiplus::Color(255, 255, 0, 0); // 몬스터 - 빨간색
					break;
				default:
					boundsColor = Gdiplus::Color(255, 255, 255, 255); // 기타 - 흰색
					break;
			}
			
			// 바운드 그리기 (선 두께, UI 레이어에 그림)
			renderManager->AddDrawCommand(boundsRect, boundsColor, 3.0f, LAYER_DEBUG_OVERLAY, 9999.0f);
			
			// 반투명 배경 추가하여 바운드가 더 잘 보이도록 함
			Gdiplus::Color bgColor = Gdiplus::Color(50, boundsColor.GetR(), boundsColor.GetG(), boundsColor.GetB());
			renderManager->AddFillRectangleCommand(boundsRect, bgColor, LAYER_DEBUG_OVERLAY, 9998.0f);
		}
	}
}
