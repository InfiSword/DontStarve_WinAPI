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
	// �ʱ� ���� ����
	m_transitionState = TransitionState::NONE;
	m_fadeAlpha = 0.0f;

	// ù ��° �� (Ÿ��Ʋ ��) �ε�
	LoadTitleScene();
}

void SceneManager::Update(float deltaTime)
{
	// ���̵� ȿ�� ������Ʈ
	UpdateFadeEffect(deltaTime);

	// ���� �� ������Ʈ (������ �Ŵ����� ������Ʈ)
	if (m_currentScene) {
		m_currentScene->Update(deltaTime);
	}
}

void SceneManager::LateUpdate()
{
	// ���� �� LateUpdate (������ �Ŵ����� LateUpdate)
	if (m_currentScene) {
		m_currentScene->LateUpdate();
	}
}

void SceneManager::Render()
{
	// �� ��ȯ �߿��� ���������� ���� (�ʱ�ȭ�� �Ϸ���� ���� ����)
	if (m_transitionState == TransitionState::SCENE_SWITCH) {
		// ���̵� ȿ���� ������
		RenderFadeEffect();
		return;
	}

	// ���� �� ������ (������ �Ŵ����� ������)
	if (m_currentScene) {
		m_currentScene->Render();
	}

	// ���̵� ȿ�� ������
	RenderFadeEffect();
}

void SceneManager::Release()
{
	// ���� �� ����
	if (m_currentScene) {
		m_currentScene->Release();
		delete m_currentScene;
		m_currentScene = nullptr;
	}

	// ���� �� ����
	if (m_nextScene) {
		m_nextScene->Release();
		delete m_nextScene;
		m_nextScene = nullptr;
	}
}

void SceneManager::LoadTitleScene()
{
	// Ÿ��Ʋ �� ���� �� �ε�
	TitleScene* titleScene = new TitleScene();
	titleScene->Init();

	// ���� ���� �ִٸ� ����
	if (m_currentScene) {
		m_currentScene->Release();
		delete m_currentScene;
	}

	m_currentScene = titleScene;

	// �ʱ� �ε尡 �ƴ� ��쿡�� ���̵� �� ����
	if (m_transitionState != TransitionState::NONE) {
		StartFadeIn();
	}

	OutputDebugStringW(L"SceneManager: Ÿ��Ʋ �� �ε� �Ϸ�\n");
}

void SceneManager::LoadCharacterSelectScene()
{
	// ĳ���� ���� �� ���� (�ʱ�ȭ�� ���̵� �ƿ� �Ϸ� �Ŀ�)
	CharacterSelectScene* characterSelectScene = new CharacterSelectScene();
	m_nextScene = characterSelectScene;

	StartFadeOut();

	OutputDebugStringW(L"SceneManager: ĳ���� ���� �� �ε� ���� - ���̵� �ƿ� ����\n");
}

void SceneManager::LoadGameScene(const std::wstring& mapFileName, GameObjectID selectedCharacterID)
{
	OutputDebugStringW(L"SceneManager: ���� �� �ε� ����\n");

	// �ӽ� ������ ����
	m_tempMapFileName = mapFileName;
	m_tempSelectedCharacterID = selectedCharacterID;

	// ���� �� ���� (�ʱ�ȭ�� ���̵� �ƿ� �Ϸ� �Ŀ�)
	GameScene* gameScene = new GameScene();
	gameScene->SetSelectedCharacterID(selectedCharacterID);

	m_nextScene = gameScene;
	StartFadeOut();

	OutputDebugStringW((L"SceneManager: ���� �� �ε� ���� - ��: " + mapFileName +
		L", ĳ���� ID: " + std::to_wstring(selectedCharacterID) + L", ���̵� �ƿ� ����\n").c_str());
}

void SceneManager::ReturnToTitle()
{
	// Ÿ��Ʋ ������ ���ư���
	OutputDebugStringW(L"SceneManager: Ÿ��Ʋ ������ ���ư��� ����\n");

	// Ÿ��Ʋ �� ���� (�ʱ�ȭ�� ���̵� �ƿ� �Ϸ� �Ŀ�)
	TitleScene* titleScene = new TitleScene();

	m_nextScene = titleScene;
	StartFadeOut();
}

void SceneManager::ParseMapFileInto(const std::wstring& mapFileName, MapData& mapData)
{
	// MapData �ʱ�ȭ
	mapData.mapFilePath = mapFileName;

	// ���ϸ���� �� �̸� ���� (Ȯ���� ����)
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
		// �ּ� ���� ó��
		if (line.empty() || line[0] == L'#') {
			// ���� ���� üũ
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

		// ��Ÿ������ �Ľ�
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

			// ��� ��ū�� ���� ����
			while (std::getline(ss, token, L',')) {
				token.erase(0, token.find_first_not_of(L" \t"));
				token.erase(token.find_last_not_of(L" \t") + 1);
				tokens.push_back(token);
			}

			// Ÿ�� ������ �Ľ� - ��� Ÿ���� ��ġ (TILE_NONE�� ����)
			for (int i = 0; i < tokens.size(); i += 2)
			{
				int tileX = i / 2;
				if (tileX < mapData.mapWidth && currentTileRow < mapData.mapHeight) {
					TileType tileType = EnumUtils::GetEnumValue<TileType>(tokens[i].c_str(), TILE_NONE);
					TileID tileID = EnumUtils::GetEnumValue<TileID>(tokens[i + 1].c_str(), TILEID_NONE);

					// ResourceManager���� Ÿ�� ���� ��������
					const TileData* resourceData = resourceManager->GetTileResourceInfo(tileID);

					// Ÿ�� ������ ����
					mapData.tiles[tileX][currentTileRow].type = tileType;
					mapData.tiles[tileX][currentTileRow].id = tileID;

					// ResourceManager���� ������ ������ ����
					if (resourceData) {
						mapData.tiles[tileX][currentTileRow].tileAssetBaseDirectory = resourceData->tileAssetBaseDirectory;
						mapData.tiles[tileX][currentTileRow].tileImageName = resourceData->tileImageName;
						// pAtlasBitmap�� sourceRect�� ���߿� �ε� ������ ����
						mapData.tiles[tileX][currentTileRow].pAtlasBitmap = nullptr;
						mapData.tiles[tileX][currentTileRow].sourceRect = Gdiplus::RectF(0.0f, 0.0f, 0.0f, 0.0f);
					}
					else {
						// ���ҽ� ������ ���� ��� �⺻�� ����
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

			// 0,0,0,0,0 ������ ���� �ǳʶٱ�
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

				// ���ڿ� ����
				type.erase(0, type.find_first_not_of(L" \t"));
				type.erase(type.find_last_not_of(L" \t") + 1);
				id.erase(0, id.find_first_not_of(L" \t"));
				id.erase(id.find_last_not_of(L" \t") + 1);
				resource.erase(0, resource.find_first_not_of(L" \t"));
				resource.erase(resource.find_last_not_of(L" \t") + 1);

				// �� �����Ϳ� ���ӿ�����Ʈ ������ ���� (���� ������ GameScene����)
				GameObjectID objID = EnumUtils::GetEnumValue<GameObjectID>(id.c_str(), GOID_NONE);
				GameObjectType objType = EnumUtils::GetEnumValue<GameObjectType>(type.c_str(), GOBJ_NONE);
				float objX = std::stof(x);
				float objY = std::stof(y);
				float objPivotX = std::stof(pivotX);
				float objPivotY = std::stof(pivotY);

				// ResourceManager���� ������Ʈ ���� ��������
				const GameObjectData* resourceData = resourceManager->GetObjectResourceInfo(objID);

				// ��ȿ�� ������Ʈ ID�� ��쿡�� �߰�
				if (objID != GOID_NONE) {
					// �� �����Ϳ� �߰�
					GameObjectData objData;
					objData.type = objType;
					objData.id = objID;
					objData.x = objX;
					objData.y = objY;
					objData.pivotX = objPivotX;
					objData.pivotY = objPivotY;

					// ResourceManager���� ������ ������ ����
					if (resourceData) {
						objData.objectAssetBaseDirectory = resourceData->objectAssetBaseDirectory;
						objData.assetImageName = resourceData->assetImageName;
						// �ݶ��̴� ������ ����
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
			// walkableAreas �Ľ� (1,1,1,0,1,...)
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

// �� Ÿ�� ��, ����, ��ȯ ��� ���� SceneType�� ����ϵ��� ����
SceneType SceneManager::GetCurrentSceneType() const
{
	if (!m_currentScene) {
		return SCENE_NONE; // �⺻��
	}
	return m_currentScene->GetSceneType();
}

// ���̵� ȿ�� ���� �޼���� ����
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
	// TimeManager���� �̹� ���ѵ� deltaTime�� �����Ƿ� �߰� ���� ���ʿ�
	switch (m_transitionState) {
	case TransitionState::FADE_OUT:
		m_fadeAlpha += deltaTime / m_fadeDuration;
		if (m_fadeAlpha >= 1.0f) {
			m_fadeAlpha = 1.0f;

			// ���̵� �ƿ� �Ϸ� �� �� ��ȯ
			if (m_nextScene) {
				// ���� ���� �ִٸ� ����
				if (m_currentScene) {
					m_currentScene->Release();
					delete m_currentScene;
				}

				// ���� ���� ���� ������ ����
				m_currentScene = m_nextScene;
				m_nextScene = nullptr;

				// �� ��ȯ ���·� ����
				m_transitionState = TransitionState::SCENE_SWITCH;
			}
			else {
				// ���� ���� ������ ���̵� ���� ����
				m_transitionState = TransitionState::NONE;
			}
		}
		break;

	case TransitionState::SCENE_SWITCH:
	{
		// �� ��ȯ �� - �� �� �ʱ�ȭ
		if (m_currentScene) {
			// GameScene�� ��� �� �����ͷ� �ʱ�ȭ
			if (m_currentScene->GetSceneType() == SCENE_GAME_FARMING_AREA ||
				m_currentScene->GetSceneType() == SCENE_GAME_HOUND_FOREST ||
				m_currentScene->GetSceneType() == SCENE_GAME_SPIDER_QUEEN_HOUSE)
			{
				GameScene* gameScene = dynamic_cast<GameScene*>(m_currentScene);
				if (gameScene) {
					// �� ������ �Ľ� �� �ʱ�ȭ (���� �Ҵ��Ͽ� ���� �����÷ο� ����)
					std::unique_ptr<MapData> mapData = std::make_unique<MapData>();
					ParseMapFileInto(m_tempMapFileName, *mapData);
					gameScene->Init(*mapData);
					// unique_ptr�� �ڵ����� ������

					OutputDebugStringW(L"SceneManager: ���� �� �ʱ�ȭ �Ϸ�\n");
				}
			}
			else {
				// �ٸ� ������ �Ϲ� �ʱ�ȭ
				m_currentScene->Init();
				OutputDebugStringW(L"SceneManager: �� �� �ʱ�ȭ �Ϸ�\n");
			}
		}

		// �ʱ�ȭ �Ϸ� �� ��� ���̵� �� ����
		StartFadeIn();
	}
	break;

	case TransitionState::FADE_IN:
		m_fadeAlpha -= deltaTime / m_fadeDuration;
		if (m_fadeAlpha <= 0.0f) {
			m_fadeAlpha = 0.0f;
			m_transitionState = TransitionState::NONE;
			OutputDebugStringW(L"SceneManager: ���̵� �� �Ϸ�\n");
		}
		break;

	case TransitionState::NONE:
	default:
		break;
	}

	// ����� ��� �߰� (�ʹ� ���� ������� �ʵ��� ����)
	static float debugTimer = 0.0f;
	debugTimer += deltaTime;
	if (m_transitionState != TransitionState::NONE && debugTimer >= 0.1f) {
		OutputDebugStringW((L"SceneManager: ���̵� ȿ�� ������Ʈ - ����: " + std::to_wstring(m_fadeAlpha) + L", ����: " + std::to_wstring(static_cast<int>(m_transitionState)) + L"\n").c_str());
		debugTimer = 0.0f;
	}
}

void SceneManager::RenderFadeEffect()
{
	if (m_transitionState != TransitionState::NONE) {
		// ���̵� ȿ�� ������
		RenderManager* renderManager = RenderManager::GetInstance();
		if (renderManager) {
			// SCENE_SWITCH ���¿����� ������ ���� ȭ�� ����
			BYTE alpha;
			if (m_transitionState == TransitionState::SCENE_SWITCH) {
				alpha = 255; // ������ ���� ȭ��
			}
			else {
				// ���İ��� 0-255 ������ ��ȯ�ϰ�, ���������� ���̵�
				alpha = static_cast<BYTE>(m_fadeAlpha * 255);
			}

			Gdiplus::Color fadeColor(alpha, 0, 0, 0);

			// ����� ��� ���� (�ʹ� ���� ������� �ʵ���)
			static float renderDebugTimer = 0.0f;
			renderDebugTimer += 0.016f; // �뷫���� ������ �ð�
			if (renderDebugTimer >= 0.5f) {
				OutputDebugStringW((L"SceneManager: ���̵� ȿ�� ������ - ����: " + std::to_wstring(alpha) +
					L", ����: " + std::to_wstring(static_cast<int>(m_transitionState)) + L"\n").c_str());
				renderDebugTimer = 0.0f;
			}

			renderManager->AddFillRectangleCommand(
				Gdiplus::RectF(0, 0, WINCX, WINCY),
				fadeColor,
				LAYER_DEBUG_OVERLAY,  // ���� ���� ������
				999.0f  // ���� ���� ������
			);
		}
		else {
			OutputDebugStringW(L"SceneManager: RenderManager�� null�Դϴ�. ���̵� ȿ�� ������ ����.\n");
		}
	}
}
