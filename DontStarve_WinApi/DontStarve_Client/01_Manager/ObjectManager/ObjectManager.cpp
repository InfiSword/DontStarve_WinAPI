#include "99_Default/pch.h"
#include "ObjectManager.h"
#include <fstream>
#include <chrono>
#include <windows.h>
#include "../ResourceManager/ResourceManager.h"
#include "../RenderManager/RenderManager.h"
#include "../CameraManager/CameraManager.h"

// #region agent log helper
static std::wstring GetLogPath() {
	wchar_t exePath[MAX_PATH];
	GetModuleFileNameW(NULL, exePath, MAX_PATH);
	std::wstring path(exePath);
	size_t pos = path.find_last_of(L"\\");
	if (pos != std::wstring::npos) {
		path = path.substr(0, pos);
	}
	return path + L"\\debug.log";
}
// #endregion

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
	// #region agent log
	auto startTime = std::chrono::high_resolution_clock::now();
	// #endregion
	
	// 모든 게임오브젝트 업데이트
	int updatedCount = 0;
	for (GameObject* obj : m_gameObjects)
	{
		if (obj && obj->IsEnabled())
		{
			obj->Update(deltaTime);
			updatedCount++;
		}
	}
	
	// #region agent log
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
	// 로그 파일 I/O는 성능 테스트를 위해 임시로 비활성화
	// std::ofstream logFile(GetLogPath(), std::ios::app);
	// if (logFile.is_open()) {
	// 	logFile << "{\"runId\":\"perf1\",\"hypothesisId\":\"D\",\"location\":\"ObjectManager.cpp:55\",\"message\":\"ObjectManager::Update\",\"data\":{\"duration_us\":" << duration << ",\"totalObjects\":" << m_gameObjects.size() << ",\"updatedCount\":" << updatedCount << "},\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << "}\n";
	// 	logFile.close();
	// }
	// #endregion
}

void ObjectManager::LateUpdate()
{
	// 모든 게임오브젝트에 LateUpdate 호출
	for (GameObject* obj : m_gameObjects)
	{
		if (obj && obj->IsEnabled())
		{
			obj->LateUpdate();
		}
	}
	
	// 루프 종료 후 삭제 지연 처리
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
			SafeDelete(*it);
			m_gameObjects.erase(it);
		}
	}

	// 삭제 지연 큐 비우기
	m_pendingDeletions.clear();
}

void ObjectManager::ClearAllObjects()
{
	// 삭제 지연 큐도 함께 처리
	ProcessPendingDeletions();

	m_cachedPlayer = nullptr; // 씬 전환 시 이전 플레이어 포인터 무효화

	for (GameObject* obj : m_gameObjects)
	{
		if (obj)
		{
			obj->Release();
			SafeDelete(obj);
		}
	}
	m_gameObjects.clear();
	m_pendingDeletions.clear();
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
	// m_cachedPlayer는 이미 m_gameObjects에 포함되어 있으므로 중복 초기화 불필요
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
		if (obj && obj->IsEnabled())
		{
			// Sprite 크기 기반 바운딩 박스 사용
			CameraManager* cameraManager = CameraManager::GetInstance();
			if (!cameraManager) continue;
			
			Gdiplus::RectF objBounds = cameraManager->GetSpriteBoundingBox(obj);
			
			// 클릭한 위치가 오브젝트의 실제 바운드 박스 안에 있는지 확인
			if (x >= objBounds.X && x <= objBounds.X + objBounds.Width && 
				y >= objBounds.Y && y <= objBounds.Y + objBounds.Height)
			{
				return obj;
			}
		}
	}
	return nullptr;
}

void ObjectManager::InitializeFactories()
{
	// 플레이어 타입
	m_gameObjectFactories[GOID_PLAYER_WILSON] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Player(x, y, id, data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_PLAYER_WILLOW] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Player(x, y, id, data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_PLAYER_WOLFGANG] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Player(x, y, id, data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};

	// 나무 타입
	m_gameObjectFactories[GOID_NORMAL_TREE_SHORT] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Tree(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_NORMAL_TREE_NORMAL] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Tree(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_NORMAL_TREE_TALL] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Tree(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};

	// 돌 타입
	m_gameObjectFactories[GOID_NORMAL_ROCK] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Rock(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_GOLD_ROCK] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Rock(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};

	// 풀 타입 - 환경 오브젝트
	m_gameObjectFactories[GOID_NORMAL_GRASS] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Grass(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_NORMAL_SAPLING] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Sapling(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_BERRY_TREE] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new BerryBush(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f,
			data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};

	// 몬스터 타입
	m_gameObjectFactories[GOID_MONSTER_PIG] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Pig(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f, data ? data->assetImageName : L"");
	};
	
	m_gameObjectFactories[GOID_MONSTER_SPIDER] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Spider(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f, data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_MONSTER_WARRIOR_SPIDER] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Spider(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f, data ? data->assetImageName : L"");
	};
	
	m_gameObjectFactories[GOID_MONSTER_QUEEN_SPIDER] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Boss_SpiderQueen(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f, data ? data->assetImageName : L"");
	};
	
	m_gameObjectFactories[GOID_MONSTER_HOUNDDOG] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Hound(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f, data ? data->assetImageName : L"");
	};
	
	m_gameObjectFactories[GOID_MONSTER_REDHOUNDDOG] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Boss_Hound(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f, data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_MONSTER_ICEHOUNDDOG] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new Boss_Hound(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f, data ? data->assetImageName : L"");
	};

	// 건물 타입
	m_gameObjectFactories[GOID_BUILDING_PIGHOUSE] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new PigHouse(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f, DIR_DOWN, data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	
	m_gameObjectFactories[GOID_BUILDING_SPIDER_SMALLEGG] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new SpiderEgg(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f, DIR_DOWN, data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_BUILDING_SPIDER_NORMALEGG] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new SpiderEgg(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f, DIR_DOWN, data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};
	m_gameObjectFactories[GOID_BUILDING_SPIDER_TALLEGG] = [](GameObjectID id, float x, float y, const GameObjectData* data) -> GameObject* {
		return new SpiderEgg(id, x, y, data ? data->pivotX : 0.5f, data ? data->pivotY : 0.5f, DIR_DOWN, data ? data->objectAssetBaseDirectory : L"", data ? data->assetImageName : L"");
	};

	// 아이템 팩토리 등록
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
		if (obj && obj->IsEnabled())
		{
			// Sprite 크기 기반 바운딩 박스 사용
			Gdiplus::RectF objBounds = cameraManager->GetSpriteBoundingBox(obj);
			
			// 월드 좌표를 스크린 좌표로 변환
			Gdiplus::PointF screenLeft = cameraManager->WorldToScreen(objBounds.X, objBounds.Y);
			Gdiplus::PointF screenRight = cameraManager->WorldToScreen(
				objBounds.X + objBounds.Width, 
				objBounds.Y + objBounds.Height);
			
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
