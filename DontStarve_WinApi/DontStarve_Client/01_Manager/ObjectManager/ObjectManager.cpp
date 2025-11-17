#include "../../99_Default/pch.h"
#include "ObjectManager.h"
#include "../../02_GameObject/GameObject/GameObject.h"
#include "../../02_GameObject/Player/Player.h"
#include "../../02_GameObject/GameObject/Tree.h"
#include "../../02_GameObject/GameObject/Rock.h"
#include "../../02_GameObject/GameObject/Grass.h"
#include "../../02_GameObject/GameObject/BerryBush.h"
#include "../../02_GameObject/GameObject/Sapling.h"
#include "../../02_GameObject/GameObject/Monster.h"
#include "../../02_GameObject/GameObject/Pig.h"
#include "../../02_GameObject/GameObject/Spider.h"
#include "../../02_GameObject/GameObject/Boss_SpiderQueen.h"
#include "../../02_GameObject/GameObject/Hound.h"
#include "../../02_GameObject/GameObject/Boss_Hound.h"
#include "../../02_GameObject/GameObject/Item.h"
#include "../../02_GameObject/GameObject/Building.h"
#include "../../02_GameObject/GameObject/PigHouse.h"
#include "../../02_GameObject/GameObject/SpiderEgg.h"
#include "../../02_GameObject/GameObject/Ingredient.h"
#include "../../02_GameObject/Tool/Axe/Axe.h"
#include "../ResourceManager/ResourceManager.h"
#include "../RenderManager/RenderManager.h"
#include "../CameraManager/CameraManager.h"

ObjectManager::ObjectManager()
{
	m_cachedPlayer = nullptr;
	m_showBounds = true; // 테두리 표시 기본값은 false
}

ObjectManager::~ObjectManager()
{
	ClearAllObjects();
}

void ObjectManager::Init()
{
}

void ObjectManager::LateInit()
{
	// 모든 게임오브젝트의 LateInit 호출
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
	// 모든 게임오브젝트의 LateUpdate 호출
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
	// 모든 렌더링 가능한 게임오브젝트 렌더링 (플레이어 우선)
	RenderManager::GetInstance()->RenderVisibleGameObjects();
	
	// 테두리 표시가 활성화된 경우 모든 오브젝트의 테두리를 그리기
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
			OutputDebugStringW((L"ObjectManager: 플레이어 캐시 완료 - ID: " + std::to_wstring(player->GetID()) + L"\n").c_str());
		}

		OutputDebugStringW((L"ObjectManager: 새로운 게임오브젝트 추가 완료 - ID: " + std::to_wstring(pObj->GetID()) + L", 총 게임오브젝트 수: " + std::to_wstring(m_gameObjects.size()) + L"\n").c_str());
	}
}

void ObjectManager::RemoveGameObject(GameObject* pObj)
{
	if (!pObj) return;

	auto it = std::find(m_gameObjects.begin(), m_gameObjects.end(), pObj);
	if (it != m_gameObjects.end())
	{
		// 플레이어 캐시 정리
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

// 플레이어 캐시를 통한 빠른 접근
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
			// 오브젝트의 이미지 테두리 크기 가져오기
			float objWidth = obj->GetWidth();
			float objHeight = obj->GetHeight();
			float objX = obj->GetX();
			float objY = obj->GetY();
			
			// 오브젝트의 피벗 정보 가져오기
			float pivotX = obj->GetPivotX();
			float pivotY = obj->GetPivotY();
			
			// 피벗 기준으로 테두리 영역 계산
			float left = objX - (objWidth * pivotX);
			float right = objX + (objWidth * (1.0f - pivotX));
			float top = objY - (objHeight * pivotY);
			float bottom = objY + (objHeight * (1.0f - pivotY));
			
			// 클릭한 위치가 오브젝트의 이미지 테두리 안에 있는지 확인
			if (x >= left && x <= right && y >= top && y <= bottom)
			{
				return obj;
			}
		}
	}
	return nullptr;
}

// 팩토리 패턴 함수
GameObject* ObjectManager::CreateGameObject(GameObjectID id, float x, float y, const GameObjectData* resourceData)
{
	// ResourceManager에서 리소스 정보 가져오기
	const GameObjectData* data = resourceData;
	if (!data) {
		data = ResourceManager::GetInstance()->GetObjectResourceInfo(id);
	}

	GameObject* newObj = nullptr;

	switch (id)
	{
		// 플레이어 타입들 - Player 클래스로 생성
	case GOID_PLAYER_WILSON:
	case GOID_PLAYER_WILLOW:
	case GOID_PLAYER_WOLFGANG:
		newObj = new Player(x, y, id,
			data ? data->objectAssetBaseDirectory : L"",
			data ? data->assetImageName : L"");
		break;

		// 나무 타입들 - Tree 클래스로 생성
	case GOID_NORMAL_TREE_SHORT:
	case GOID_NORMAL_TREE_NORMAL:
	case GOID_NORMAL_TREE_TALL:
		newObj = new Tree(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
		break;

		// 바위 타입들 - Rock 클래스로 생성
	case GOID_NORMAL_ROCK:
	case GOID_GOLD_ROCK:
		newObj = new Rock(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
		break;

		// 잔디 타입들 - 각각의 클래스로 생성
	case GOID_NORMAL_GRASS:
		newObj = new Grass(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
		break;
	case GOID_NORMAL_SAPLING:
		newObj = new Sapling(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
		break;
	case GOID_BERRY_TREE:
		newObj = new BerryBush(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
		break;

		// 몬스터 타입들 - 각각의 클래스로 생성
	case GOID_MONSTER_PIG:
		newObj = new Pig(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
		break;
	case GOID_MONSTER_SPIDER:
	case GOID_MONSTER_WARRIOR_SPIDER:
		newObj = new Spider(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
		break;
	case GOID_MONSTER_QUEEN_SPIDER:
		newObj = new Boss_SpiderQueen(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
		break;
	case GOID_MONSTER_HOUNDDOG:
		newObj = new Hound(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
		break;
	case GOID_MONSTER_REDHOUNDDOG:
	case GOID_MONSTER_ICEHOUNDDOG:
		newObj = new Boss_Hound(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
		break;

		// 건물 타입들 - 각각의 클래스로 생성
	case GOID_BUILDING_PIGHOUSE:
		newObj = new PigHouse(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			DIR_DOWN, data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
		break;
	case GOID_BUILDING_SPIDER_SMALLEGG:
	case GOID_BUILDING_SPIDER_NORMALEGG:
	case GOID_BUILDING_SPIDER_TALLEGG:
		newObj = new SpiderEgg(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			DIR_DOWN, data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
		break;

		// 아이템 타입들 - Ingredient 클래스로 생성
	case GOID_ITEM_NORMAL_ROCK:
	case GOID_ITEM_CUT_NORMAL_GRASS:
	case GOID_ITEM_NORMAL_TWIGS:
	case GOID_ITEM_GOLD_ROCK:
	case GOID_ITEM_NORMAL_TREE_LOG:
	case GOID_ITEM_ROPE:
	case GOID_ITEM_CUT_NORMAL_STONE:
	case GOID_ITEM_MEAT:
	case GOID_ITEM_BERRY:
	{
		newObj = new Ingredient(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data->objectAssetBaseDirectory, data->assetImageName);
	}
	break;
	}

	// 생성된 게임오브젝트를 매니저에 추가 (GameObject 계층구조로 관리)
	if (newObj) {
		AddGameObject(newObj);
		OutputDebugStringW((L"ObjectManager: 새로운 게임오브젝트 생성 완료 - ID: " + std::to_wstring(id) + L" at (" + std::to_wstring(x) + L", " + std::to_wstring(y) + L")\n").c_str());
	}
	else {
		OutputDebugStringW((L"ObjectManager: 새로운 게임오브젝트 생성 실패 - ID: " + std::to_wstring(id) + L"\n").c_str());
	}

	return newObj;
}

// === 아이템 생성 함수 ===
std::shared_ptr<Item> ObjectManager::CreateItem(GameObjectID itemID)
{
	const GameObjectData* resourceData = ResourceManager::GetInstance()->GetObjectResourceInfo(itemID);
	if (!resourceData) {
		OutputDebugStringW(L"ObjectManager: 아이템 리소스 정보를 찾을 수 없습니다.\n");
		return nullptr;
	}

	// 리소스 정보에서 경로 구성
	const std::wstring& resourcePath = resourceData->objectAssetBaseDirectory;
	const std::wstring& imagePath = resourceData->assetImageName;

	switch (itemID)
	{
	case GOID_ITEM_NORMAL_TREE_LOG:
		return std::make_shared<Item>(GOBJ_ITEM, itemID, L"LOG", L"A Log.", resourcePath, imagePath);

	case GOID_ITEM_NORMAL_TWIGS:
		return std::make_shared<Item>(GOBJ_ITEM, itemID, L"Twigs", L"A common twig.", resourcePath, imagePath);

	case GOID_ITEM_NORMAL_ROCK:
		return std::make_shared<Item>(GOBJ_ITEM, itemID, L"Rock Shard", L"A small piece of rock.", resourcePath, imagePath);

	case GOID_ITEM_CUT_NORMAL_GRASS:
		return std::make_shared<Item>(GOBJ_ITEM, itemID, L"Cut Grass", L"Bundled grass, good for crafting.", resourcePath, imagePath);

	case GOID_ITEM_GOLD_ROCK:
		return std::make_shared<Item>(GOBJ_ITEM, itemID, L"Gold", L"Shiny and valuable.", resourcePath, imagePath);

	case GOID_ITEM_ROPE:
		return std::make_shared<Item>(GOBJ_ITEM, itemID, L"Rope", L"Useful for crafting.", resourcePath, imagePath);

	case GOID_ITEM_MEAT:
		return std::make_shared<Item>(GOBJ_ITEM, itemID, L"Meat", L"Fresh meat.", resourcePath, imagePath);

	case GOID_ITEM_BERRY:
		return std::make_shared<Item>(GOBJ_ITEM, itemID, L"Berry", L"Sweet and nutritious.", resourcePath, imagePath);

	case GOID_ITEM_AXE:
		return std::make_shared<Axe>(itemID, L"Axe", L"Cuts down trees.", resourcePath + L"/" + imagePath, 100.0f, 1.0f);

	default:
		return std::make_shared<Item>(GOBJ_ITEM, itemID, L"Unknown Item", L"Unknown item.", resourcePath, imagePath);
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
			// 오브젝트의 이미지 테두리 크기 가져오기
			float objWidth = obj->GetWidth();
			float objHeight = obj->GetHeight();
			float objX = obj->GetX();
			float objY = obj->GetY();
			
			// 오브젝트의 피벗 정보 가져오기
			float pivotX = obj->GetPivotX();
			float pivotY = obj->GetPivotY();
			
			// 피벗 기준으로 테두리 영역 계산
			float left = objX - (objWidth * pivotX);
			float right = objX + (objWidth * (1.0f - pivotX));
			float top = objY - (objHeight * pivotY);
			float bottom = objY + (objHeight * (1.0f - pivotY));
			
			// 월드 좌표를 스크린 좌표로 변환
			Gdiplus::PointF screenLeft = cameraManager->WorldToScreen(left, top);
			Gdiplus::PointF screenRight = cameraManager->WorldToScreen(right, bottom);
			
			// 테두리 사각형 생성
			Gdiplus::RectF boundsRect(
				screenLeft.X,
				screenLeft.Y,
				screenRight.X - screenLeft.X,
				screenRight.Y - screenLeft.Y
			);
			
			// 오브젝트 타입에 따라 다른 색상 사용 (더 선명한 색상)
			Gdiplus::Color boundsColor;
			switch (obj->GetType()) {
				case GOBJ_ITEM:
					boundsColor = Gdiplus::Color(255, 0, 255, 0); // 밝은 녹색 - 아이템
					break;
				case GOBJ_NATURAL_ENVIR:
					boundsColor = Gdiplus::Color(255, 0, 150, 255); // 밝은 파란색 - 자연환경
					break;
				case GOBJ_MONSTER:
					boundsColor = Gdiplus::Color(255, 255, 0, 0); // 빨간색 - 몬스터
					break;
				default:
					boundsColor = Gdiplus::Color(255, 255, 255, 255); // 흰색 - 기타
					break;
			}
			
			// 테두리 그리기 (더 두껍게, UI 레이어에 그리기)
			renderManager->AddDrawCommand(boundsRect, boundsColor, 3.0f, LAYER_DEBUG_OVERLAY, 9999.0f);
			
			// 반투명 배경도 추가하여 테두리를 더 잘 보이게 함
			Gdiplus::Color bgColor = Gdiplus::Color(50, boundsColor.GetR(), boundsColor.GetG(), boundsColor.GetB());
			renderManager->AddFillRectangleCommand(boundsRect, bgColor, LAYER_DEBUG_OVERLAY, 9998.0f);
		}
	}
}
