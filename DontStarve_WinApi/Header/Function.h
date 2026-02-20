#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <cstring>
#include "Enum.h"

using namespace Gdiplus;

// ====================== Enum 문자열 변환 유틸리티 함수 =======================

namespace EnumTables {
	// TileType -> 문자열
	inline const WCHAR* GetEnumName(TileType value) {
		for (const auto& entry : TileTypeTable) {
			if (entry.value == value) return entry.name;
		}
		return L"UNKNOWN";
	}

	// 문자열 -> TileType
	inline TileType GetTileType(const WCHAR* name) {
		for (const auto& entry : TileTypeTable) {
			if (std::wcscmp(entry.name, name) == 0) return entry.value;
		}
		return TILE_NONE;
	}

	// TileID -> 문자열
	inline const WCHAR* GetEnumName(TileID value) {
		for (const auto& entry : TileIDTable) {
			if (entry.value == value) return entry.name;
		}
		return L"UNKNOWN";
	}

	// 문자열 -> TileID
	inline TileID GetTileID(const WCHAR* name) {
		for (const auto& entry : TileIDTable) {
			if (std::wcscmp(entry.name, name) == 0) return entry.value;
		}
		return TILEID_NONE;
	}

	// GameObjectType -> 문자열
	inline const WCHAR* GetEnumName(GameObjectType value) {
		for (const auto& entry : GameObjectTypeTable) {
			if (entry.value == value) return entry.name;
		}
		return L"UNKNOWN";
	}

	// 문자열 -> GameObjectType
	inline GameObjectType GetGameObjectType(const WCHAR* name) {
		for (const auto& entry : GameObjectTypeTable) {
			if (std::wcscmp(entry.name, name) == 0) return entry.value;
		}
		return GOBJ_NONE;
	}

	// GameObjectID -> 문자열
	inline const WCHAR* GetEnumName(GameObjectID value) {
		for (const auto& entry : GameObjectIDTable) {
			if (entry.value == value) return entry.name;
		}
		return L"UNKNOWN";
	}

	// 문자열 -> GameObjectID
	inline GameObjectID GetGameObjectID(const WCHAR* name) {
		for (const auto& entry : GameObjectIDTable) {
			if (std::wcscmp(entry.name, name) == 0) return entry.value;
		}
		return GOID_NONE;
	}
}

// 유틸리티 함수들
namespace Utils
{
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

// 리소스 경로 관련 유틸리티 함수들
namespace ResourcePathUtils
{
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
						TileType tileType = EnumTables::GetTileType(tokens[i].c_str());
						TileID tileID = EnumTables::GetTileID(tokens[i + 1].c_str());
						
						// TileID로 경로 찾기
						std::wstring baseDir, imageName;
						for (size_t j = 0; j < TileResourceCount; ++j) {
							if (TileResourceTable[j].id == tileID) {
								baseDir = TileResourceTable[j].baseDir;
								imageName = TileResourceTable[j].imageName;
								break;
							}
						}
						
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
					
					GameObjectID objID = EnumTables::GetGameObjectID(id.c_str());
					GameObjectType objType = EnumTables::GetGameObjectType(type.c_str());
					
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
					
					// 리소스 정보 가져오기 (콜백: type+id로 오브젝트 리소스 조회, Editor는 baseDir/imageName 복원용)
					const ResourcePathUtils::ObjectResourceDef* resourceData = getObjectResourceInfo(objType, objID);
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
						// 콜백이 nullptr을 반환하면 파일에서 파싱한 resource 필드 사용
						objData.baseDir = resource;
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
										// Circle 콜라이더 정보 (Box도 이 값들을 가질 수 있으므로 타입 체크 필요)
										if (!cX.empty() && !cY.empty() && !rad.empty()) {
											objData.colliderCenterX = std::stof(cX);
											objData.colliderCenterY = std::stof(cY);
											objData.colliderRadius = std::stof(rad);
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

	// baseDir와 imageName을 결합하여 전체 경로 생성 (경로 결합 로직 중복 제거)
	inline std::wstring BuildResourcePath(const std::wstring& baseDir, const std::wstring& imageName) {
		if (baseDir.empty()) {
			OutputDebugStringW(L"BuildResourcePath: baseDir가 비어있습니다. 경로를 생성할 수 없습니다.\n");
			return L"";  // 오류: baseDir가 비어있으면 빈 문자열 반환
		}
		if (imageName.empty()) {
			OutputDebugStringW(L"BuildResourcePath: imageName이 비어있습니다. 경로를 생성할 수 없습니다.\n");
			return L"";  // 오류: imageName이 비어있으면 빈 문자열 반환
		}
		std::wstring fullPath = baseDir;
		if (fullPath.back() != L'\\' && fullPath.back() != L'/') {
			fullPath += L'\\';  // Windows 경로 구분자는 하나면 충분
		}
		fullPath += imageName;
		return fullPath;
	}
}
