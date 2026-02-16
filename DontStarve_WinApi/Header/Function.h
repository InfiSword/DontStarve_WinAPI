#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>
#include "Struct.h"

using namespace Gdiplus;

// 유틸리티 함수들
namespace Utils
{
	// 안전한 포인터 삭제
	template<typename T>
	inline void SafeDelete(T& obj)
	{
		if (obj)
		{
			delete obj;
			obj = nullptr;
		}
	}

	// 거리 계산
	inline float CalculateDistance(float x1, float y1, float x2, float y2)
	{
		float dx = x2 - x1;
		float dy = y2 - y1;
		return sqrtf(dx * dx + dy * dy);
	}

	// 목표까지의 방향 계산
	inline Direction GetDirectionToTarget(float fromX, float fromY, float toX, float toY)
	{
		float dx = toX - fromX;
		float dy = toY - fromY;

		// 절댓값이 더 큰 방향을 우선적으로 선택
		if (abs(dx) > abs(dy))
		{
			return (dx > 0) ? DIR_RIGHT : DIR_LEFT;
		}
		else
		{
			return (dy > 0) ? DIR_DOWN : DIR_UP;
		}
	}
}

// 비트맵 관련 전역 유틸 함수들
namespace BitmapUtils
{
	inline Bitmap* LoadBitmapFromFile(const WCHAR* filename) {
		if (!filename) return nullptr;
		Bitmap* pBitmap = Bitmap::FromFile(filename);
		if (!pBitmap || pBitmap->GetLastStatus() != Ok) {
			Utils::SafeDelete(pBitmap);
			return nullptr;
		}
		return pBitmap;
	}
}

// 리소스 경로 관련 구조체 및 함수들
namespace ResourcePathUtils
{
	// 리소스 경로 빌드 헬퍼 함수 (Editor와 Client 공통 사용)
	inline std::wstring BuildResourcePath(const std::wstring& basePath, const std::wstring& filename)
	{
		WCHAR modulePath[MAX_PATH];
		GetModuleFileNameW(NULL, modulePath, MAX_PATH);

		WCHAR projectRoot[MAX_PATH] = { 0 };
		WCHAR* winApiPos = wcsstr(modulePath, L"DontStarve_WinApi");
		if (winApiPos) {
			size_t len = wcslen(L"DontStarve_WinApi");
			size_t copyLen = winApiPos - modulePath + len;
			if (copyLen < MAX_PATH) {
				wcsncpy_s(projectRoot, MAX_PATH, modulePath, copyLen);
				projectRoot[copyLen] = L'\0';
			}
			else {
				wcscpy_s(projectRoot, MAX_PATH, modulePath);
			}
		}
		else {
			wcscpy_s(projectRoot, MAX_PATH, modulePath);
			WCHAR* lastSlash = wcsrchr(projectRoot, L'\\');
			if (lastSlash) {
				*lastSlash = L'\0';
				lastSlash = wcsrchr(projectRoot, L'\\');
				if (lastSlash) *lastSlash = L'\0';
				lastSlash = wcsrchr(projectRoot, L'\\');
				if (lastSlash) *lastSlash = L'\0';
				lastSlash = wcsrchr(projectRoot, L'\\');
				if (lastSlash) *lastSlash = L'\0';
			}
		}

		std::wstring relativePath = L"../" + basePath;
		if (!filename.empty()) {
			relativePath += L"/" + filename;
		}
		for (size_t i = 0; i < relativePath.length(); ++i) {
			if (relativePath[i] == L'/') relativePath[i] = L'\\';
		}
		DWORD fileAttributes = GetFileAttributesW(relativePath.c_str());
		if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
			return relativePath;
		}
		wchar_t fullPath[MAX_PATH];
		if (GetFullPathNameW(relativePath.c_str(), MAX_PATH, fullPath, nullptr) > 0) {
			std::wstring absolutePath = std::wstring(fullPath);
			fileAttributes = GetFileAttributesW(absolutePath.c_str());
			if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
				return absolutePath;
			}
		}
		return relativePath;
	}

	// Client용 BuildResourcePath (subFolder 포함)
	inline std::wstring BuildResourcePath(const std::wstring& basePath, const std::wstring& subFolder, const std::wstring& filename)
	{
		std::wstring path = basePath;
		if (!subFolder.empty()) {
			path += L"/" + subFolder;
		}
		if (!filename.empty()) {
			path += L"/" + filename;
		}
		return BuildResourcePath(path, L"");
	}

	// 타일 리소스 정의 배열 (Editor와 Client 공통 사용, 절대경로로 확정)
	inline const TileResourceDef* GetTileResourceDefs(size_t& outCount)
	{
		static std::vector<TileResourceDef> tileDefs;
		static bool initialized = false;
		
		if (!initialized) {
			// 상대 경로 정의
			struct TileDefRaw {
				TileType type;
				TileID id;
				const wchar_t* baseDir;
				const wchar_t* imageName;
			};
			static const TileDefRaw rawDefs[] = {
				{ TILE_DIRT, TILEID_DIRT_00, L"Resource/Tiles/Dirt", L"dirt_01.png" },
				{ TILE_DIRT, TILEID_DIRT_01, L"Resource/Tiles/Dirt", L"dirt_02.png" },
				{ TILE_DIRT, TILEID_DIRT_02, L"Resource/Tiles/Dirt", L"dirt_03.png" },
				{ TILE_DIRT, TILEID_DIRT_03, L"Resource/Tiles/Dirt", L"dirt_04.png" },
				{ TILE_GRASS, TILEID_GRASS_00, L"Resource/Tiles/Grass", L"grass_01.png" },
				{ TILE_GRASS, TILEID_GRASS_01, L"Resource/Tiles/Grass", L"grass_02.png" },
				{ TILE_GRASS, TILEID_GRASS_02, L"Resource/Tiles/Grass", L"grass_03.png" },
				{ TILE_GRASS, TILEID_GRASS_03, L"Resource/Tiles/Grass", L"grass_04.png" },
				{ TILE_FOREST, TILEID_FOREST_00, L"Resource/Tiles/Forest", L"forest_01.png" },
				{ TILE_FOREST, TILEID_FOREST_01, L"Resource/Tiles/Forest", L"forest_02.png" },
				{ TILE_FOREST, TILEID_FOREST_02, L"Resource/Tiles/Forest", L"forest_03.png" },
				{ TILE_FOREST, TILEID_FOREST_03, L"Resource/Tiles/Forest", L"forest_04.png" },
			};
			
			for (const auto& raw : rawDefs) {
				std::wstring absBaseDir = BuildResourcePath(raw.baseDir, L"");
				std::wstring absImageName = BuildResourcePath(raw.baseDir, raw.imageName);
				tileDefs.emplace_back(raw.type, raw.id, absBaseDir, absImageName);
			}
			initialized = true;
		}
		
		outCount = tileDefs.size();
		return tileDefs.data();
	}

	// 오브젝트 리소스 정의 배열 (Editor와 Client 공통 사용, 절대경로로 확정)
	inline const ObjectResourceDef* GetObjectResourceDefs(size_t& outCount)
	{
		static std::vector<ObjectResourceDef> objectDefs;
		static bool initialized = false;
		
		if (!initialized) {
			// 상대 경로 정의
			struct ObjectDefRaw {
				GameObjectType type;
				GameObjectID id;
				const wchar_t* baseDir;
				const wchar_t* imageName;
				float pivotX;
				float pivotY;
			};
			static const ObjectDefRaw rawDefs[] = {
				{ GOBJ_PLAYER, GOID_PLAYER_WILSON, L"Resource/Objects/Player/Wilson", L"", 0.5f, 1.0f },
				{ GOBJ_PLAYER, GOID_PLAYER_WILLOW, L"Resource/Objects/Player/Willow", L"", 0.5f, 1.0f },
				{ GOBJ_PLAYER, GOID_PLAYER_WOLFGANG, L"Resource/Objects/Player/Wolfgang", L"", 0.5f, 1.0f },
				{ GOBJ_NATURAL_ENVIR, GOID_NORMAL_TREE_SHORT, L"Resource/Objects/Tree1/Short", L"evergreen_evergreen_short_idle_short_01.png", 0.5f, 1.0f },
				{ GOBJ_NATURAL_ENVIR, GOID_NORMAL_TREE_NORMAL, L"Resource/Objects/Tree1/Normal", L"evergreen_evergreen_short_idle_normal_01.png", 0.5f, 1.0f },
				{ GOBJ_NATURAL_ENVIR, GOID_NORMAL_TREE_TALL, L"Resource/Objects/Tree1/Tall", L"evergreen_evergreen_short_idle_tall_01.png", 0.5f, 1.0f },
				{ GOBJ_NATURAL_ENVIR, GOID_NORMAL_ROCK, L"Resource/Objects/Rock/Rock_Normal", L"rock01-0.png", 0.5f, 1.0f },
				{ GOBJ_NATURAL_ENVIR, GOID_GOLD_ROCK, L"Resource/Objects/Rock/Rock_Gold", L"rock02-0.png", 0.5f, 1.0f },
				{ GOBJ_NATURAL_ENVIR, GOID_NORMAL_GRASS, L"Resource/Objects/Grass", L"grass.png", 0.5f, 1.0f },
				{ GOBJ_NATURAL_ENVIR, GOID_NORMAL_SAPLING, L"Resource/Objects/Twign", L"sapling.png", 0.5f, 1.0f },
				{ GOBJ_NATURAL_ENVIR, GOID_BERRY_TREE, L"Resource/Objects/Bush", L"BerryBush.png", 0.5f, 1.0f },
				{ GOBJ_ITEM, GOID_ITEM_CUT_NORMAL_GRASS, L"Resource/Objects/ingredient", L"cutgrass01-0.png", 0.5f, 1.0f },
				{ GOBJ_ITEM, GOID_ITEM_NORMAL_ROCK, L"Resource/Objects/ingredient", L"rocks01-0.png", 0.5f, 1.0f },
				{ GOBJ_ITEM, GOID_ITEM_NORMAL_TWIGS, L"Resource/Objects/ingredient", L"twigs01-0.png", 0.5f, 1.0f },
				{ GOBJ_ITEM, GOID_ITEM_NORMAL_TREE_LOG, L"Resource/Objects/ingredient", L"Tree1_log.png", 0.5f, 1.0f },
				{ GOBJ_ITEM, GOID_ITEM_GOLD_ROCK, L"Resource/Objects/ingredient", L"Gold_Item.png", 0.5f, 1.0f },
				{ GOBJ_ITEM, GOID_ITEM_ROPE, L"Resource/Objects/ingredient", L"rope01-0.png", 0.5f, 1.0f },
				{ GOBJ_ITEM, GOID_ITEM_CUT_NORMAL_STONE, L"Resource/Objects/ingredient", L"cutstone01-0.png", 0.5f, 1.0f },
				{ GOBJ_ITEM, GOID_ITEM_MEAT, L"Resource/Objects/ingredient", L"meat-0.png", 0.5f, 1.0f },
				{ GOBJ_ITEM, GOID_ITEM_BERRY, L"Resource/Objects/ingredient", L"Berry.png", 0.5f, 1.0f },
				{ GOBJ_MONSTER, GOID_MONSTER_SPIDER, L"Resource/Objects/Monster/Spider/Normal_Spider", L"Spider_spider_idle_01.png", 0.5f, 1.0f },
				{ GOBJ_MONSTER, GOID_MONSTER_WARRIOR_SPIDER, L"Resource/Objects/Monster/Spider/Warrior_Spider", L"Warrior_spider_idle_01.png", 0.5f, 1.0f },
				{ GOBJ_MONSTER, GOID_MONSTER_PIG, L"Resource/Objects/Monster/Pig", L"pig_Image.png", 0.5f, 1.0f },
				{ GOBJ_MONSTER, GOID_MONSTER_HOUNDDOG, L"Resource/Objects/Monster/Hound/Normal_Hound", L"Hound_hound_Image.png", 0.5f, 1.0f },
				{ GOBJ_MONSTER, GOID_MONSTER_QUEEN_SPIDER, L"Resource/Objects/Monster/Spider/Queen", L"Queen_spider_queen_Image.png", 0.5f, 1.0f },
				{ GOBJ_MONSTER, GOID_MONSTER_REDHOUNDDOG, L"Resource/Objects/Monster/Hound/Red_Hound", L"RedHound_hound_Image.png", 0.5f, 1.0f },
				{ GOBJ_MONSTER, GOID_MONSTER_ICEHOUNDDOG, L"Resource/Objects/Monster/Hound/Ice_Hound", L"IceHound_hound_Image.png", 0.5f, 1.0f },
				{ GOBJ_BUILDING, GOID_BUILDING_SPIDER_SMALLEGG, L"Resource/Objects/Building/Egg", L"Egg_spider_cocoon_small_Image.png", 0.5f, 1.0f },
				{ GOBJ_BUILDING, GOID_BUILDING_SPIDER_NORMALEGG, L"Resource/Objects/Building/Egg", L"Egg_spider_cocoon_medium_Image.png", 0.5f, 1.0f },
				{ GOBJ_BUILDING, GOID_BUILDING_SPIDER_TALLEGG, L"Resource/Objects/Building/Egg", L"Egg_spider_cocoon_large_Image.png", 0.5f, 1.0f },
				{ GOBJ_BUILDING, GOID_BUILDING_PIGHOUSE, L"Resource/Objects/Building/House", L"pig_house.png", 0.5f, 1.0f },
			};
			
			for (const auto& raw : rawDefs) {
				std::wstring absBaseDir = BuildResourcePath(raw.baseDir, L"");
				std::wstring absImageName = raw.imageName[0] ? BuildResourcePath(raw.baseDir, raw.imageName) : L"";
				objectDefs.emplace_back(raw.type, raw.id, 0, 0, absBaseDir, absImageName, raw.pivotX, raw.pivotY);
			}
			initialized = true;
		}
		
		outCount = objectDefs.size();
		return objectDefs.data();
	}

	// TileID로부터 타일 경로 정보 가져오기 (Client의 GetTilePathForParse 대체)
	inline void GetTilePathForParse(TileID id, std::wstring& outBaseDir, std::wstring& outImageName)
	{
		outBaseDir.clear();
		outImageName.clear();
		size_t count;
		const TileResourceDef* defs = GetTileResourceDefs(count);
		for (size_t i = 0; i < count; ++i) {
			if (defs[i].id == id) {
				outBaseDir = defs[i].baseDir;
				outImageName = defs[i].imageName;
				return;
			}
		}
	}

	// 맵 파일 파싱 함수 (Client와 Editor 공통 사용)
	// ResourceManager의 GetObjectResourceInfo 콜백을 통해 오브젝트 리소스 정보를 가져옴
	template<typename GetObjectResourceInfoFunc>
	inline bool ParseMapFileInto(const std::wstring& mapFileName, MapData& outMapData, GetObjectResourceInfoFunc getObjectResourceInfo)
	{
		outMapData.mapFilePath = mapFileName;

		// 맵 이름 추출
		size_t lastSlash = mapFileName.find_last_of(L"\\/");
		size_t lastDot = mapFileName.find_last_of(L".");
		if (lastSlash != std::wstring::npos) {
			outMapData.mapName = mapFileName.substr(lastSlash + 1, lastDot - lastSlash - 1);
		}
		else {
			outMapData.mapName = mapFileName.substr(0, lastDot);
		}

		// 파일 열기
		std::wifstream file(mapFileName);
		file.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));

		if (!file.is_open()) {
			return false;
		}

		// BOM 처리
		wchar_t bom[3] = { 0 };
		file.read(bom, 3);
		if (bom[0] != 0xFEFF && !(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
			file.seekg(0, std::ios::beg);
		}
		else if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
			file.seekg(3, std::ios::beg);
		}

		std::wstring line;
		enum Section { NONE, METADATA, PLAYER, TILES, OBJECTS, WALKABLE } section = NONE;
		int currentTileRow = 0;
		int currentWalkRow = 0;

		while (std::getline(file, line)) {
			if (line.empty() || line[0] == L'#') {
				if (line.find(L"# TILES") != std::wstring::npos || line == L"# TILES") {
					section = TILES;
					currentTileRow = 0;
				}
				else if (line.find(L"# OBJECTS") != std::wstring::npos || line == L"# OBJECTS") {
					section = OBJECTS;
				}
				else 				if (line.find(L"# WALKABLE_AREAS") != std::wstring::npos || line == L"# WALKABLE_AREAS") {
					section = WALKABLE;
					currentWalkRow = 0;
				}
				else if (line.find(L"# PLAYER_SPAWN") != std::wstring::npos || line == L"# PLAYER_SPAWN") {
					section = PLAYER;
				}
				continue;
			}

			// 메타데이터 파싱
			if (line.find(L"MAP_WIDTH=") != std::wstring::npos) {
				outMapData.mapWidth = std::stoi(line.substr(line.find(L"=") + 1));
			}
			else if (line.find(L"MAP_HEIGHT=") != std::wstring::npos) {
				outMapData.mapHeight = std::stoi(line.substr(line.find(L"=") + 1));
			}
			// 플레이어 스폰 파싱
			else if (section == PLAYER) {
				if (line.find(L"PLAYER_SPAWN_X=") != std::wstring::npos) {
					float x = std::stof(line.substr(line.find(L"=") + 1));
					if (x >= 0) {
						outMapData.playerSpawn.x = x;
					}
				}
				else if (line.find(L"PLAYER_SPAWN_Y=") != std::wstring::npos) {
					float y = std::stof(line.substr(line.find(L"=") + 1));
					if (y >= 0) {
						outMapData.playerSpawn.y = y;
					}
				}
			}
			// 타일 파싱
			else if (section == TILES && currentTileRow < outMapData.mapHeight) {
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
						outMapData.tiles[tileX][currentTileRow].baseDir = baseDir;
						outMapData.tiles[tileX][currentTileRow].imageName = imageName;
					}
				}
				currentTileRow++;
			}
			// 오브젝트 파싱
			else if (section == OBJECTS) {
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
					
					// 공백 제거
					type.erase(0, type.find_first_not_of(L" \t"));
					type.erase(type.find_last_not_of(L" \t") + 1);
					id.erase(0, id.find_first_not_of(L" \t"));
					id.erase(id.find_last_not_of(L" \t") + 1);
					resource.erase(0, resource.find_first_not_of(L" \t"));
					resource.erase(resource.find_last_not_of(L" \t") + 1);
					
					GameObjectID objID = EnumUtils::GetEnumValue<GameObjectID>(id.c_str(), GOID_NONE);
					GameObjectType objType = EnumUtils::GetEnumValue<GameObjectType>(type.c_str(), GOBJ_NONE);
					
					if (objID != GOID_NONE && objType != GOBJ_NONE) {
						float objX = std::stof(x);
						float objY = std::stof(y);
						float objPivotX = std::stof(pivotX);
						float objPivotY = std::stof(pivotY);
						
					ResourcePathUtils::ObjectResourceDef objData;
					objData.type = objType;
					objData.id = objID;
					objData.x = objX;
					objData.y = objY;
					objData.pivotX = objPivotX;
					objData.pivotY = objPivotY;
					
					// 리소스 정보 가져오기 (콜백 함수 사용)
					const ResourcePathUtils::ObjectResourceDef* resourceData = getObjectResourceInfo(objID);
					if (resourceData) {
						objData.baseDir = resourceData->baseDir;
						objData.imageName = resourceData->imageName;
						objData.hasCollider = resourceData->hasCollider;
						objData.colliderType = resourceData->colliderType;
						objData.colliderOffsetX = resourceData->colliderOffsetX;
						objData.colliderOffsetY = resourceData->colliderOffsetY;
						objData.colliderWidth = resourceData->colliderWidth;
						objData.colliderHeight = resourceData->colliderHeight;
						objData.colliderCenterX = resourceData->colliderCenterX;
						objData.colliderCenterY = resourceData->colliderCenterY;
						objData.colliderRadius = resourceData->colliderRadius;
					}
					else {
						// 콜백이 nullptr을 반환하면 파일에서 파싱한 resource 필드를 절대경로로 변환 (Editor의 경우)
						objData.baseDir = BuildResourcePath(resource, L"");
						objData.imageName = L"";
					}
						
						// 콜라이더 정보 파싱 (다음 줄 확인)
						std::streampos currentPos = file.tellg();
						if (std::getline(file, line)) {
							std::wstringstream cs(line);
							std::wstring lbl, hasC, cTyp, oX, oY, wd, ht, cX, cY, rad;
							std::getline(cs, lbl, L',');
							lbl.erase(0, lbl.find_first_not_of(L" \t"));
							lbl.erase(lbl.find_last_not_of(L" \t") + 1);
							
							if (lbl == L"Collider") {
								std::getline(cs, hasC, L',');
								std::getline(cs, cTyp, L',');
								std::getline(cs, oX, L',');
								std::getline(cs, oY, L',');
								std::getline(cs, wd, L',');
								std::getline(cs, ht, L',');
								std::getline(cs, cX, L',');
								std::getline(cs, cY, L',');
								std::getline(cs, rad);
								
								if (!hasC.empty()) {
									objData.hasCollider = (hasC == L"1" || hasC == L"true");
									if (objData.hasCollider) {
										int cVal = cTyp.empty() ? 0 : std::stoi(cTyp);
										objData.colliderType = (cVal >= 0 && cVal <= COLLIDER_COUNT) ? (ColliderType)cVal : COLLIDER_BOX;
										if (!oX.empty()) objData.colliderOffsetX = std::stoi(oX);
										if (!oY.empty()) objData.colliderOffsetY = std::stoi(oY);
										if (!wd.empty()) objData.colliderWidth = std::stoi(wd);
										if (!ht.empty()) objData.colliderHeight = std::stoi(ht);
										if (!cX.empty() && !cY.empty() && !rad.empty()) {
											float centerX = std::stof(cX);
											float centerY = std::stof(cY);
											float radius = std::stof(rad);
											if (radius > 0.0f) {
												objData.colliderType = COLLIDER_CIRCLE;
												objData.colliderCenterX = centerX;
												objData.colliderCenterY = centerY;
												objData.colliderRadius = radius;
											}
										}
									}
								}
							}
							else {
								// Collider 라벨이 아니면 파일 포인터 되돌리기
								file.seekg(currentPos);
							}
						}
						
						outMapData.gameObjects.push_back(objData);
					}
				}
			}
			// Walkable 영역 파싱
			else if (section == WALKABLE && currentWalkRow < outMapData.mapHeight) {
				std::wstringstream ss(line);
				std::wstring token;
				int currentCol = 0;
				while (std::getline(ss, token, L',') && currentCol < outMapData.mapWidth) {
					token.erase(0, token.find_first_not_of(L" \t"));
					token.erase(token.find_last_not_of(L" \t") + 1);
					outMapData.walkableAreas[currentCol][currentWalkRow] = (std::stoi(token) == 1);
					currentCol++;
				}
				currentWalkRow++;
			}
		}
		
		file.close();
		return true;
	}
}
