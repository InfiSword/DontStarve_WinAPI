#include "../../99_Default/pch.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "CharacterSelectScene.h"
#include "GameScene.h"
#include "../UIManager/UIManager.h"
#include "../InputManager/InputManager.h"
#include "../CameraManager/CameraManager.h"
#include "../ObjectManager/ObjectManager.h"
#include "../RenderManager/RenderManager.h"
#include "../InventoryManager/InventoryManager.h"
#include "../ColliderManager/ColliderManager.h"
#include "../ResourceManager/ResourceManager.h"
#include <codecvt>
#include <locale>
#include <memory>

SceneManager::SceneManager()
	: m_currentScene(nullptr), m_nextScene(nullptr),
	m_transitionState(TransitionState::NONE), m_fadeAlpha(0.0f), m_fadeDuration(0.8f)
{
}

SceneManager::~SceneManager()
{
	Release();
}

void SceneManager::Init()
{
	// 초기 상태 설정
	m_transitionState = TransitionState::NONE;
	m_fadeAlpha = 0.0f;

	// 첫 번째 씬 (타이틀 씬) 로드
	LoadTitleScene();
}

void SceneManager::Update(float deltaTime)
{
	// 전환 효과 업데이트
	UpdateFadeEffect(deltaTime);

	// 현재 씬 업데이트 (해당 씬의 매니저들 업데이트)
	if (m_currentScene) {
		m_currentScene->Update(deltaTime);
	}
}

void SceneManager::LateUpdate()
{
	// 현재 씬 LateUpdate (해당 씬의 매니저들 LateUpdate)
	if (m_currentScene) {
		m_currentScene->LateUpdate();
	}
}

void SceneManager::Render()
{
	// 씬 전환 중에는 검은 화면만 표시 (초기화가 완료될 때까지 대기)
	if (m_transitionState == TransitionState::SCENE_SWITCH) {
		// 전환 효과만 렌더링
		RenderFadeEffect();
		return;
	}

	// 현재 씬 렌더링 (해당 씬의 매니저들 렌더링)
	if (m_currentScene) {
		m_currentScene->Render();
	}

	// 전환 효과 렌더링
	RenderFadeEffect();
}

void SceneManager::Release()
{
	// 현재 씬 해제
	if (m_currentScene) {
		m_currentScene->Release();
		delete m_currentScene;
		m_currentScene = nullptr;
	}

	// 다음 씬 해제
	if (m_nextScene) {
		m_nextScene->Release();
		delete m_nextScene;
		m_nextScene = nullptr;
	}
}

void SceneManager::LoadTitleScene()
{
	// 타이틀 씬 생성 후 즉시 로드
	TitleScene* titleScene = new TitleScene();
	titleScene->Init();

	// 현재 씬이 있으면 해제
	if (m_currentScene) {
		m_currentScene->Release();
		delete m_currentScene;
	}

	m_currentScene = titleScene;

	// 초기 로드가 아닌 경우 전환 효과 시작
	if (m_transitionState != TransitionState::NONE) {
		StartFadeIn();
	}

	OutputDebugStringW(L"SceneManager: 타이틀 씬 로드 완료\n");
}

void SceneManager::LoadCharacterSelectScene()
{
	// 캐릭터 선택 씬 생성 (초기화는 전환 효과 완료 후)
	CharacterSelectScene* characterSelectScene = new CharacterSelectScene();
	m_nextScene = characterSelectScene;

	StartFadeOut();

	OutputDebugStringW(L"SceneManager: 캐릭터 선택 씬 로드 시작 - 전환 효과 시작\n");
}

void SceneManager::LoadGameScene(const std::wstring& mapFileName, GameObjectID selectedCharacterID)
{
	OutputDebugStringW(L"SceneManager: 게임 씬 로드 시작\n");

	// 임시 변수에 저장
	m_tempMapFileName = mapFileName;
	m_tempSelectedCharacterID = selectedCharacterID;

	// 게임 씬 생성 (초기화는 전환 효과 완료 후)
	GameScene* gameScene = new GameScene();
	gameScene->SetSelectedCharacterID(selectedCharacterID);

	m_nextScene = gameScene;
	StartFadeOut();

	OutputDebugStringW((L"SceneManager: 게임 씬 로드 시작 - 맵: " + mapFileName +
		L", 캐릭터 ID: " + std::to_wstring(selectedCharacterID) + L", 전환 효과 시작\n").c_str());
}

void SceneManager::ReturnToTitle()
{
	// 타이틀 씬으로 되돌리기
	OutputDebugStringW(L"SceneManager: 타이틀 씬으로 되돌리기 시작\n");

	// 타이틀 씬 생성 (초기화는 전환 효과 완료 후)
	TitleScene* titleScene = new TitleScene();

	m_nextScene = titleScene;
	StartFadeOut();
}

void SceneManager::ParseMapFileInto(const std::wstring& mapFileName, MapData& mapData)
{
	// MapData 초기화
	mapData.mapFilePath = mapFileName;

	// 파일명에서 맵 이름 추출 (확장자 제거)
	size_t lastSlash = mapFileName.find_last_of(L"\\/");
	size_t lastDot = mapFileName.find_last_of(L".");
	if (lastSlash != std::wstring::npos) {
		mapData.mapName = mapFileName.substr(lastSlash + 1, lastDot - lastSlash - 1);
	}
	else {
		mapData.mapName = mapFileName.substr(0, lastDot);
	}

	std::wifstream file(mapFileName);
	file.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));

	if (!file.is_open()) {
		return;
	}

	std::wstring line;
	enum Section { NONE, METADATA, PLAYER, TILES, OBJECTS, WALKABLE } section = NONE;
	int currentTileRow = 0;

	ResourceManager* resourceManager = ResourceManager::GetInstance();
	if (!resourceManager) {
		return;
	}

	while (std::getline(file, line)) {
		// 주석 처리
		if (line.empty() || line[0] == L'#') {
			// 섹션 시작 체크
			if (line.find(L"# TILES") != std::wstring::npos) {
				section = TILES;
				currentTileRow = 0;
			}
			else if (line.find(L"# OBJECTS") != std::wstring::npos) {
				section = OBJECTS;
			}
			else if (line.find(L"# WALKABLE_AREAS") != std::wstring::npos) {
				section = WALKABLE;
			}
			continue;
		}

		// 메타데이터 파싱
		if (line.find(L"MAP_WIDTH=") != std::wstring::npos) {
			mapData.mapWidth = std::stoi(line.substr(line.find(L"=") + 1));
		}
		else if (line.find(L"MAP_HEIGHT=") != std::wstring::npos) {
			mapData.mapHeight = std::stoi(line.substr(line.find(L"=") + 1));
		}
		else if (line.find(L"PLAYER_SPAWN_X=") != std::wstring::npos) {
			mapData.playerSpawn.x = std::stof(line.substr(line.find(L"=") + 1));
		}
		else if (line.find(L"PLAYER_SPAWN_Y=") != std::wstring::npos) {
			mapData.playerSpawn.y = std::stof(line.substr(line.find(L"=") + 1));
		}
		else if (section == TILES)
		{
			std::wstringstream ss(line);
			std::wstring token;
			std::vector<std::wstring> tokens;

			// 타일 토큰들을 분리 저장
			while (std::getline(ss, token, L',')) {
				token.erase(0, token.find_first_not_of(L" \t"));
				token.erase(token.find_last_not_of(L" \t") + 1);
				tokens.push_back(token);
			}

			// 타일 데이터 파싱 - 각 타일의 위치 (TILE_NONE은 무시)
			for (int i = 0; i < tokens.size(); i += 2)
			{
				int tileX = i / 2;
				if (tileX < mapData.mapWidth && currentTileRow < mapData.mapHeight) {
					TileType tileType = EnumUtils::GetEnumValue<TileType>(tokens[i].c_str(), TILE_NONE);
					TileID tileID = EnumUtils::GetEnumValue<TileID>(tokens[i + 1].c_str(), TILEID_NONE);

					// ResourceManager에서 타일 리소스 정보 가져오기
					const TileData* resourceData = resourceManager->GetTileResourceInfo(tileID);

					// 타일 데이터 저장
					mapData.tiles[tileX][currentTileRow].type = tileType;
					mapData.tiles[tileX][currentTileRow].id = tileID;

					// ResourceManager에서 가져온 리소스 정보 저장
					if (resourceData) {
						mapData.tiles[tileX][currentTileRow].tileAssetBaseDirectory = resourceData->tileAssetBaseDirectory;
						mapData.tiles[tileX][currentTileRow].tileImageName = resourceData->tileImageName;
						// pAtlasBitmap과 sourceRect는 나중에 로드 시 설정
						mapData.tiles[tileX][currentTileRow].pAtlasBitmap = nullptr;
						mapData.tiles[tileX][currentTileRow].sourceRect = Gdiplus::RectF(0.0f, 0.0f, 0.0f, 0.0f);
					}
					else {
						// 리소스가 없는 경우 기본값으로 설정
						mapData.tiles[tileX][currentTileRow].tileAssetBaseDirectory = L"";
						mapData.tiles[tileX][currentTileRow].tileImageName = L"";
						mapData.tiles[tileX][currentTileRow].pAtlasBitmap = nullptr;
						mapData.tiles[tileX][currentTileRow].sourceRect = Gdiplus::RectF(0.0f, 0.0f, 0.0f, 0.0f);
					}
				}
			}

			currentTileRow++;

		}
		else if (section == OBJECTS) {

			// 0,0,0,0,0 형식의 빈 줄 무시하기
			if (line.find(L"0,0,0,0,0") != std::wstring::npos) {
				continue;
			}

			std::wstringstream ss(line);
			std::wstring type, id, x, y, resource, pivotX, pivotY;

			if (std::getline(ss, type, L',') &&
				std::getline(ss, id, L',') &&
				std::getline(ss, x, L',') &&
				std::getline(ss, y, L',') &&
				std::getline(ss, resource, L',') &&
				std::getline(ss, pivotX, L',') &&
				std::getline(ss, pivotY, L',')) {

				// 문자열 정리
				type.erase(0, type.find_first_not_of(L" \t"));
				type.erase(type.find_last_not_of(L" \t") + 1);
				id.erase(0, id.find_first_not_of(L" \t"));
				id.erase(id.find_last_not_of(L" \t") + 1);
				resource.erase(0, resource.find_first_not_of(L" \t"));
				resource.erase(resource.find_last_not_of(L" \t") + 1);

				// 맵 데이터에 게임오브젝트 데이터 추가 (실제 생성은 GameScene에서)
				GameObjectID objID = EnumUtils::GetEnumValue<GameObjectID>(id.c_str(), GOID_NONE);
				GameObjectType objType = EnumUtils::GetEnumValue<GameObjectType>(type.c_str(), GOBJ_NONE);
				float objX = std::stof(x);
				float objY = std::stof(y);
				float objPivotX = std::stof(pivotX);
				float objPivotY = std::stof(pivotY);

				// ResourceManager에서 게임오브젝트 리소스 정보 가져오기
				const GameObjectData* resourceData = resourceManager->GetObjectResourceInfo(objID);

				// 유효한 게임오브젝트 ID일 경우 추가
				if (objID != GOID_NONE) {
					// 맵 데이터에 추가
					GameObjectData objData;
					objData.type = objType;
					objData.id = objID;
					objData.x = objX;
					objData.y = objY;
					objData.pivotX = objPivotX;
					objData.pivotY = objPivotY;

					// ResourceManager에서 가져온 리소스 정보 저장
					if (resourceData) {
						objData.objectAssetBaseDirectory = resourceData->objectAssetBaseDirectory;
						objData.assetImageName = resourceData->assetImageName;
						// 콜라이더 정보 저장
						objData.hasCollider = resourceData->hasCollider;
						objData.colliderOffsetX = resourceData->colliderOffsetX;
						objData.colliderOffsetY = resourceData->colliderOffsetY;
						objData.colliderWidth = resourceData->colliderWidth;
						objData.colliderHeight = resourceData->colliderHeight;
					}
					mapData.gameObjects.push_back(objData);
				}
			}
		}
		else if (section == WALKABLE) {
			// walkableAreas 파싱 (1,1,1,0,1,...)
			std::wstringstream ss(line);
			std::wstring token;
			int currentCol = 0;

			while (std::getline(ss, token, L',') && currentCol < mapData.mapWidth) {
				token.erase(0, token.find_first_not_of(L" \t"));
				token.erase(token.find_last_not_of(L" \t") + 1);

				if (currentTileRow - mapData.mapHeight >= 0 && currentTileRow - mapData.mapHeight < mapData.mapHeight) {
					mapData.walkableAreas[currentCol][currentTileRow - mapData.mapHeight] = (std::stoi(token) == 1);
				}
				currentCol++;
			}
		}
	}

	file.close();
}

// 현재 타입 씬, 로드, 전환 시에 사용할 SceneType을 반환하기 위한 함수
SceneType SceneManager::GetCurrentSceneType() const
{
	if (!m_currentScene) {
		return SCENE_NONE; // 기본값
	}
	return m_currentScene->GetSceneType();
}

// 전환 효과 관련 함수들 구현
void SceneManager::StartFadeOut()
{
	m_transitionState = TransitionState::FADE_OUT;
	m_fadeAlpha = 0.0f;
}

void SceneManager::StartFadeIn()
{
	m_transitionState = TransitionState::FADE_IN;
	m_fadeAlpha = 1.0f;
}

void SceneManager::UpdateFadeEffect(float deltaTime)
{
	// TimeManager에서 이미 계산된 deltaTime을 사용하므로 추가 계산 불필요
	switch (m_transitionState) {
	case TransitionState::FADE_OUT:
		m_fadeAlpha += deltaTime / m_fadeDuration;
		if (m_fadeAlpha >= 1.0f) {
			m_fadeAlpha = 1.0f;

			// 전환 효과 완료 후 씬 전환
			if (m_nextScene) {
				// 현재 씬이 있으면 해제
				if (m_currentScene) {
					m_currentScene->Release();
					delete m_currentScene;
				}

				// 다음 씬을 현재 씬으로 설정
				m_currentScene = m_nextScene;
				m_nextScene = nullptr;

				// 씬 전환 상태로 변경
				m_transitionState = TransitionState::SCENE_SWITCH;
			}
			else {
				// 다음 씬이 없으면 전환 효과 종료
				m_transitionState = TransitionState::NONE;
			}
		}
		break;

	case TransitionState::SCENE_SWITCH:
	{
		// 씬 전환 중 - 새 씬 초기화
		if (m_currentScene) {
			// GameScene인 경우 맵 데이터로 초기화
			if (m_currentScene->GetSceneType() == SCENE_GAME_FARMING_AREA ||
				m_currentScene->GetSceneType() == SCENE_GAME_HOUND_FOREST ||
				m_currentScene->GetSceneType() == SCENE_GAME_SPIDER_QUEEN_HOUSE)
			{
				GameScene* gameScene = dynamic_cast<GameScene*>(m_currentScene);
				if (gameScene) {
					// 맵 데이터를 파싱 후 초기화 (메모리 할당하여 전달)
					std::unique_ptr<MapData> mapData = std::make_unique<MapData>();
					ParseMapFileInto(m_tempMapFileName, *mapData);
					gameScene->Init(*mapData);
					// unique_ptr은 자동으로 해제됨

					OutputDebugStringW(L"SceneManager: 게임 씬 초기화 완료\n");
				}
			}
			else {
				// 다른 씬은 일반 초기화
				m_currentScene->Init();
				OutputDebugStringW(L"SceneManager: 씬 초기화 완료\n");
			}
		}

		// 초기화 완료 후 페이드 인 시작
		StartFadeIn();
	}
	break;

	case TransitionState::FADE_IN:
		m_fadeAlpha -= deltaTime / m_fadeDuration;
		if (m_fadeAlpha <= 0.0f) {
			m_fadeAlpha = 0.0f;
			m_transitionState = TransitionState::NONE;
			OutputDebugStringW(L"SceneManager: 전환 효과 완료\n");
		}
		break;

	case TransitionState::NONE:
	default:
		break;
	}

	// 디버그 로그 추가 (너무 많이 출력되지 않도록 제한)
	static float debugTimer = 0.0f;
	debugTimer += deltaTime;
	if (m_transitionState != TransitionState::NONE && debugTimer >= 0.1f) {
		OutputDebugStringW((L"SceneManager: 전환 효과 업데이트 - 알파: " + std::to_wstring(m_fadeAlpha) + L", 상태: " + std::to_wstring(static_cast<int>(m_transitionState)) + L"\n").c_str());
		debugTimer = 0.0f;
	}
}

void SceneManager::RenderFadeEffect()
{
	if (m_transitionState != TransitionState::NONE) {
		// 전환 효과 렌더링
		RenderManager* renderManager = RenderManager::GetInstance();
		if (renderManager) {
			// SCENE_SWITCH 상태에서는 완전히 검은 화면 표시
			BYTE alpha;
			if (m_transitionState == TransitionState::SCENE_SWITCH) {
				alpha = 255; // 완전히 검은 화면
			}
			else {
				// 알파값을 0-255 범위로 변환하여, 점진적으로 전환
				alpha = static_cast<BYTE>(m_fadeAlpha * 255);
			}

			Gdiplus::Color fadeColor(alpha, 0, 0, 0);

			// 디버그 로그 출력 (너무 많이 출력되지 않도록 제한)
			static float renderDebugTimer = 0.0f;
			renderDebugTimer += 0.016f; // 프레임당 대략적인 시간
			if (renderDebugTimer >= 0.5f) {
				OutputDebugStringW((L"SceneManager: 전환 효과 렌더링 - 알파: " + std::to_wstring(alpha) +
					L", 상태: " + std::to_wstring(static_cast<int>(m_transitionState)) + L"\n").c_str());
				renderDebugTimer = 0.0f;
			}

			renderManager->AddFillRectangleCommand(
				Gdiplus::RectF(0, 0, WINCX, WINCY),
				fadeColor,
				LAYER_DEBUG_OVERLAY,  // 최상위 레이어에 표시
				999.0f  // 최상위 레이어에 표시
			);
		}
		else {
			OutputDebugStringW(L"SceneManager: RenderManager가 null입니다. 전환 효과 렌더링 실패.\n");
		}
	}
}
