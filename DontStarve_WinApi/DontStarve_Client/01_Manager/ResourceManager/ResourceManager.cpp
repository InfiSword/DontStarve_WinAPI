#include "99_Default/pch.h"
#include "ResourceManager.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../02_GameObject/Item/Item.h"
#include "../../02_GameObject/Entity/Monster/Spider.h"
#include "../../02_GameObject/Entity/Monster/Pig.h"
#include "../../02_GameObject/Entity/Monster/Hound.h"
#include "../../02_GameObject/Entity/Monster/Boss_SpiderQueen.h"
#include "../../02_GameObject/Entity/Monster/Boss_Hound.h"
#include "../../02_GameObject/Entity/Enviorment/Tree.h"
#include "../../02_GameObject/Entity/Enviorment/Grass.h"
#include "../../02_GameObject/Entity/Enviorment/Rock.h"
#include "../../02_GameObject/Entity/Enviorment/Sapling.h"
#include "../../02_GameObject/Entity/Enviorment/BerryBush.h"
#include "../../02_GameObject/Building/SpiderEgg.h"
#include "../../02_GameObject/Building/PigHouse.h"

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	Release();
}

void ResourceManager::Init()
{
	// 오브젝트 리소스 등록 (Player, Item, 몬스터, 건물 등)
	Player::RegisterResources(this);
	Item::RegisterResources(this);
	Spider::RegisterResources(this);
	Pig::RegisterResources(this);
	Hound::RegisterResources(this);
	Boss_SpiderQueen::RegisterResources(this);
	Boss_Hound::RegisterResources(this);
	Tree::RegisterResources(this);
	Grass::RegisterResources(this);
	Rock::RegisterResources(this);
	Sapling::RegisterResources(this);
	BerryBush::RegisterResources(this);
	SpiderEgg::RegisterResources(this);
	PigHouse::RegisterResources(this);
}

void ResourceManager::Release()
{
	m_objectResources.clear();
	m_mapDataCache.clear();
	m_spriteCache.clear();
}

void ResourceManager::RegisterObjectResource(GameObjectID id, const GameObjectData& data)
{
	m_objectResources[id] = data;
}

const GameObjectData* ResourceManager::GetObjectResourceInfo(GameObjectID id) const
{
	auto it = m_objectResources.find(id);
	if (it != m_objectResources.end()) {
		return &(it->second);
	}
	return nullptr;
}

static void GetTilePathForParse(TileID id, std::wstring& outBaseDir, std::wstring& outImageName)
{
	outBaseDir.clear();
	outImageName.clear();
	switch (id) {
	case TILEID_DIRT_00: outBaseDir = L"Resource/Tiles/Dirt"; outImageName = L"dirt_01.png"; break;
	case TILEID_DIRT_01: outBaseDir = L"Resource/Tiles/Dirt"; outImageName = L"dirt_02.png"; break;
	case TILEID_DIRT_02: outBaseDir = L"Resource/Tiles/Dirt"; outImageName = L"dirt_03.png"; break;
	case TILEID_DIRT_03: outBaseDir = L"Resource/Tiles/Dirt"; outImageName = L"dirt_04.png"; break;
	case TILEID_GRASS_00: outBaseDir = L"Resource/Tiles/Grass"; outImageName = L"grass_01.png"; break;
	case TILEID_GRASS_01: outBaseDir = L"Resource/Tiles/Grass"; outImageName = L"grass_02.png"; break;
	case TILEID_GRASS_02: outBaseDir = L"Resource/Tiles/Grass"; outImageName = L"grass_03.png"; break;
	case TILEID_GRASS_03: outBaseDir = L"Resource/Tiles/Grass"; outImageName = L"grass_04.png"; break;
	case TILEID_FOREST_00: outBaseDir = L"Resource/Tiles/Forest"; outImageName = L"forest_01.png"; break;
	case TILEID_FOREST_01: outBaseDir = L"Resource/Tiles/Forest"; outImageName = L"forest_02.png"; break;
	case TILEID_FOREST_02: outBaseDir = L"Resource/Tiles/Forest"; outImageName = L"forest_03.png"; break;
	case TILEID_FOREST_03: outBaseDir = L"Resource/Tiles/Forest"; outImageName = L"forest_04.png"; break;
	default: break;
	}
}

const MapData* ResourceManager::LoadMapData(const std::wstring& mapFileName)
{
	auto it = m_mapDataCache.find(mapFileName);
	if (it != m_mapDataCache.end()) {
		return &it->second;
	}
	MapData mapData;
	ParseMapFileInto(mapFileName, mapData);
	m_mapDataCache[mapFileName] = std::move(mapData);
	return &m_mapDataCache[mapFileName];
}

void ResourceManager::ParseMapFileInto(const std::wstring& mapFileName, MapData& outMapData)
{
	outMapData.mapFilePath = mapFileName;

	size_t lastSlash = mapFileName.find_last_of(L"\\/");
	size_t lastDot = mapFileName.find_last_of(L".");
	if (lastSlash != std::wstring::npos) {
		outMapData.mapName = mapFileName.substr(lastSlash + 1, lastDot - lastSlash - 1);
	} else {
		outMapData.mapName = mapFileName.substr(0, lastDot);
	}

	std::wifstream file(mapFileName);
	file.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));

	if (!file.is_open()) {
		return;
	}

	std::wstring line;
	enum Section { NONE, METADATA, PLAYER, TILES, OBJECTS, WALKABLE } section = NONE;
	int currentTileRow = 0;

	while (std::getline(file, line)) {
		if (line.empty() || line[0] == L'#') {
			if (line.find(L"# TILES") != std::wstring::npos) {
				section = TILES;
				currentTileRow = 0;
			} else if (line.find(L"# OBJECTS") != std::wstring::npos) {
				section = OBJECTS;
			} else if (line.find(L"# WALKABLE_AREAS") != std::wstring::npos) {
				section = WALKABLE;
			}
			continue;
		}

		if (line.find(L"MAP_WIDTH=") != std::wstring::npos) {
			outMapData.mapWidth = std::stoi(line.substr(line.find(L"=") + 1));
		} else if (line.find(L"MAP_HEIGHT=") != std::wstring::npos) {
			outMapData.mapHeight = std::stoi(line.substr(line.find(L"=") + 1));
		} else if (line.find(L"PLAYER_SPAWN_X=") != std::wstring::npos) {
			outMapData.playerSpawn.x = std::stof(line.substr(line.find(L"=") + 1));
		} else if (line.find(L"PLAYER_SPAWN_Y=") != std::wstring::npos) {
			outMapData.playerSpawn.y = std::stof(line.substr(line.find(L"=") + 1));
		} else if (section == TILES) {
			std::wstringstream ss(line);
			std::wstring token;
			std::vector<std::wstring> tokens;
			while (std::getline(ss, token, L',')) {
				token.erase(0, token.find_first_not_of(L" \t"));
				token.erase(token.find_last_not_of(L" \t") + 1);
				tokens.push_back(token);
			}
			for (int i = 0; i < (int)tokens.size(); i += 2) {
				int tileX = i / 2;
				if (tileX < outMapData.mapWidth && currentTileRow < outMapData.mapHeight) {
					TileType tileType = EnumUtils::GetEnumValue<TileType>(tokens[i].c_str(), TILE_NONE);
					TileID tileID = EnumUtils::GetEnumValue<TileID>(tokens[i + 1].c_str(), TILEID_NONE);
					std::wstring baseDir, imageName;
					GetTilePathForParse(tileID, baseDir, imageName);
					outMapData.tiles[tileX][currentTileRow].type = tileType;
					outMapData.tiles[tileX][currentTileRow].id = tileID;
					outMapData.tiles[tileX][currentTileRow].tileAssetBaseDirectory = baseDir;
					outMapData.tiles[tileX][currentTileRow].tileImageName = imageName;
					outMapData.tiles[tileX][currentTileRow].pAtlasBitmap = nullptr;
					outMapData.tiles[tileX][currentTileRow].sourceRect = Gdiplus::RectF(0.0f, 0.0f, 0.0f, 0.0f);
				}
			}
			currentTileRow++;
		} else if (section == OBJECTS) {
			if (line.find(L"0,0,0,0,0") != std::wstring::npos) continue;
			std::wstringstream ss(line);
			std::wstring type, id, x, y, resource, pivotX, pivotY;
			if (std::getline(ss, type, L',') &&
			    std::getline(ss, id, L',') &&
			    std::getline(ss, x, L',') &&
			    std::getline(ss, y, L',') &&
			    std::getline(ss, resource, L',') &&
			    std::getline(ss, pivotX, L',') &&
			    std::getline(ss, pivotY, L',')) {
				type.erase(0, type.find_first_not_of(L" \t"));
				type.erase(type.find_last_not_of(L" \t") + 1);
				id.erase(0, id.find_first_not_of(L" \t"));
				id.erase(id.find_last_not_of(L" \t") + 1);
				resource.erase(0, resource.find_first_not_of(L" \t"));
				resource.erase(resource.find_last_not_of(L" \t") + 1);
				GameObjectID objID = EnumUtils::GetEnumValue<GameObjectID>(id.c_str(), GOID_NONE);
				GameObjectType objType = EnumUtils::GetEnumValue<GameObjectType>(type.c_str(), GOBJ_NONE);
				float objX = std::stof(x);
				float objY = std::stof(y);
				float objPivotX = std::stof(pivotX);
				float objPivotY = std::stof(pivotY);
				const GameObjectData* resourceData = GetObjectResourceInfo(objID);
				if (objID != GOID_NONE) {
					GameObjectData objData;
					objData.type = objType;
					objData.id = objID;
					objData.x = objX;
					objData.y = objY;
					objData.pivotX = objPivotX;
					objData.pivotY = objPivotY;
					if (resourceData) {
						objData.objectAssetBaseDirectory = resourceData->objectAssetBaseDirectory;
						objData.assetImageName = resourceData->assetImageName;
						objData.hasCollider = resourceData->hasCollider;
						objData.colliderOffsetX = resourceData->colliderOffsetX;
						objData.colliderOffsetY = resourceData->colliderOffsetY;
						objData.colliderWidth = resourceData->colliderWidth;
						objData.colliderHeight = resourceData->colliderHeight;
					}
					outMapData.gameObjects.push_back(objData);
				}
			}
		} else if (section == WALKABLE) {
			std::wstringstream ss(line);
			std::wstring token;
			int currentCol = 0;
			while (std::getline(ss, token, L',') && currentCol < outMapData.mapWidth) {
				token.erase(0, token.find_first_not_of(L" \t"));
				token.erase(token.find_last_not_of(L" \t") + 1);
				if (currentTileRow - outMapData.mapHeight >= 0 && currentTileRow - outMapData.mapHeight < outMapData.mapHeight) {
					outMapData.walkableAreas[currentCol][currentTileRow - outMapData.mapHeight] = (std::stoi(token) == 1);
				}
				currentCol++;
			}
		}
	}
	file.close();
}

std::wstring ResourceManager::BuildResourcePath(const std::wstring& basePath, const std::wstring& subFolder, const std::wstring& filename) const
{
	std::wstring path = basePath;
	if (!subFolder.empty()) {
		path += L"/" + subFolder;
	}
	if (!filename.empty()) {
		path += L"/" + filename;
	}

	std::wstring relativePath = L"../" + path;

	// 경로 구분자를 백슬래시로 변환 (Windows 표준, 파일 시스템 호환)
	for (size_t i = 0; i < relativePath.length(); ++i) {
		if (relativePath[i] == L'/') {
			relativePath[i] = L'\\';
		}
	}

	// 상대 경로 파일 존재 확인
	DWORD fileAttributes = GetFileAttributesW(relativePath.c_str());
	if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
		return relativePath;
	}

	// 상대경로가 실패하면 절대 경로 변환 시도
	wchar_t fullPath[MAX_PATH];
	if (GetFullPathNameW(relativePath.c_str(), MAX_PATH, fullPath, nullptr) > 0) {
		std::wstring absolutePath = std::wstring(fullPath);

		// 절대 경로도 파일 존재 여부 확인
		fileAttributes = GetFileAttributesW(absolutePath.c_str());
		if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
			return absolutePath;
		}
	}

	return relativePath;
}

std::shared_ptr<Sprite> ResourceManager::LoadSprite(const std::wstring& fullPath)
{
	if (fullPath.empty()) return nullptr;

	auto found = m_spriteCache.find(fullPath);
	if (found != m_spriteCache.end()) {
		if (std::shared_ptr<Sprite> cached = found->second.lock()) {
			return cached;
		}
	}

	std::shared_ptr<Gdiplus::Bitmap> bmp = std::make_shared<Gdiplus::Bitmap>(fullPath.c_str());
	if (!bmp || bmp->GetLastStatus() != Gdiplus::Ok) {
		return nullptr;
	}
	Gdiplus::RectF src(0, 0, static_cast<float>(bmp->GetWidth()), static_cast<float>(bmp->GetHeight()));
	std::shared_ptr<Sprite> sprite = std::make_shared<Sprite>(bmp, src, 0.5f, 0.5f, fullPath, false);
	m_spriteCache[fullPath] = sprite;
	return sprite;
}

std::shared_ptr<Sprite> ResourceManager::LoadSpriteFromAtlas(const std::wstring& atlasPath, const Gdiplus::RectF& srcRect, float pivotX, float pivotY)
{
	std::wstring key = atlasPath + L"|" +
		std::to_wstring(srcRect.X) + L"," + std::to_wstring(srcRect.Y) + L"," +
		std::to_wstring(srcRect.Width) + L"," + std::to_wstring(srcRect.Height);

	auto found = m_spriteCache.find(key);
	if (found != m_spriteCache.end()) {
		if (auto cached = found->second.lock()) {
			return cached;
		}
	}

	auto bmp = std::make_shared<Gdiplus::Bitmap>(atlasPath.c_str());
	if (!bmp || bmp->GetLastStatus() != Gdiplus::Ok) {
		return nullptr;
	}
	auto sprite = std::make_shared<Sprite>(bmp, srcRect, pivotX, pivotY, key, true);
	m_spriteCache[key] = sprite;
	return sprite;
}
