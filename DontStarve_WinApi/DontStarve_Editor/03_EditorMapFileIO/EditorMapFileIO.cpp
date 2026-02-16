#include "../pch.h"
#include "EditorMapFileIO.h"
#include "../00_MainEditor/DontStarve_EditorMain.h"
#include "../01_EditorView/EditorView.h"
#include <commdlg.h>

bool EditorMapFileIO::SaveMap(DontStarve_EditorMain* pMain, const WCHAR* filename) {
	std::wofstream outFile(filename);
	if (!outFile.is_open()) {
		OutputDebugStringW(L"맵 파일 열기 실패: ");
		OutputDebugStringW(filename);
		OutputDebugStringW(L"\n");
		return false;
	}
	outFile << L"# MAP_METADATA\n";
	outFile << L"MAP_WIDTH=" << MAP_WIDTH << L"\n";
	outFile << L"MAP_HEIGHT=" << MAP_HEIGHT << L"\n";
	outFile << L"# Editor Info: FPS=" << (int)pMain->GetCurrentFPS()
		<< L", Memory=" << (int)pMain->GetLayerMemoryUsageMB() << L"MB"
		<< L", Zoom=" << (int)(pMain->m_pView->GetZoomFactor() * 100) << L"%\n\n";
	outFile << L"# PLAYER_SPAWN\n";
	if (pMain->m_hasPlayerSpawn) {
		outFile << L"PLAYER_SPAWN_X=" << pMain->m_playerSpawnPoint.X << L"\n";
		outFile << L"PLAYER_SPAWN_Y=" << pMain->m_playerSpawnPoint.Y << L"\n";
	} else {
		outFile << L"PLAYER_SPAWN_X=-1\nPLAYER_SPAWN_Y=-1\n";
	}
	outFile << L"\n# TILES\n";
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			ResourcePathUtils::TileResourceDef tile = pMain->m_tileMap[y][x];
			outFile << EnumUtils::GetEnumName(tile.type) << L"," << EnumUtils::GetEnumName(tile.id);
			if (x < MAP_WIDTH - 1) outFile << L",";
		}
		outFile << L"\n";
	}
	outFile << L"\n# OBJECTS\n";
	for (const auto& obj : pMain->m_gameObjects) {
		outFile << EnumUtils::GetEnumName(obj.type) << L"," << EnumUtils::GetEnumName(obj.id) << L","
			<< obj.x << L"," << obj.y << L"," << obj.baseDir << L","
			<< obj.pivotX << L"," << obj.pivotY << L"\n";
		outFile << L"Collider," << obj.hasCollider << L"," << (int)obj.colliderType << L","
			<< obj.colliderOffsetX << L"," << obj.colliderOffsetY << L","
			<< obj.colliderWidth << L"," << obj.colliderHeight << L","
			<< obj.colliderCenterX << L"," << obj.colliderCenterY << L"," << obj.colliderRadius << L"\n";
	}
	outFile << L"\n# WALKABLE_AREAS\n";
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			outFile << (pMain->m_walkableAreaMap[y][x] ? L"1" : L"0");
			if (x < MAP_WIDTH - 1) outFile << L",";
		}
		outFile << L"\n";
	}
	outFile.close();
	std::wstring pathStr(filename);
	size_t pathSep = pathStr.find_last_of(L"\\/");
	pMain->m_lastMapDirectory = (pathSep != std::wstring::npos) ? pathStr.substr(0, pathSep + 1) : L"";
	SaveColliderTemplates(pMain);
	OutputDebugStringW(L"맵 저장 완료: ");
	OutputDebugStringW(filename);
	OutputDebugStringW(L"\n");
	return true;
}

void EditorMapFileIO::LoadColliderTemplates(DontStarve_EditorMain* pMain) {
	pMain->m_colliderTemplates.clear();
	std::wstring path = pMain->m_lastMapDirectory + L"collider_templates.txt";
	std::wifstream in(path);
	if (!in.is_open()) return;
	int type = 0, id = 0, hasC = 0, cType = 0;
	int ox = 0, oy = 0, w = 0, h = 0;
	float cx = 0.0f, cy = 0.0f, r = 0.0f;
	while (in >> type >> id >> hasC >> cType >> ox >> oy >> w >> h >> cx >> cy >> r) {
		DontStarve_EditorMain::ColliderTemplate t;
		t.hasCollider = (hasC != 0);
		t.colliderType = (ColliderType)cType;
		t.colliderOffsetX = ox; t.colliderOffsetY = oy;
		t.colliderWidth = w; t.colliderHeight = h;
		t.colliderCenterX = cx; t.colliderCenterY = cy; t.colliderRadius = r;
		pMain->m_colliderTemplates[std::make_pair(type, id)] = t;
	}
}

void EditorMapFileIO::SaveColliderTemplates(DontStarve_EditorMain* pMain) {
	if (pMain->m_lastMapDirectory.empty()) return;
	std::wstring path = pMain->m_lastMapDirectory + L"collider_templates.txt";
	std::wofstream out(path);
	if (!out) return;
	for (const auto& kv : pMain->m_colliderTemplates) {
		const DontStarve_EditorMain::ColliderTemplate& t = kv.second;
		out << kv.first.first << L" " << kv.first.second << L" "
			<< (t.hasCollider ? 1 : 0) << L" " << (int)t.colliderType << L" "
			<< t.colliderOffsetX << L" " << t.colliderOffsetY << L" "
			<< t.colliderWidth << L" " << t.colliderHeight << L" "
			<< t.colliderCenterX << L" " << t.colliderCenterY << L" " << t.colliderRadius << L"\n";
	}
}

static bool GetMapDataDialogPath(WCHAR* outPath, DWORD pathSize) {
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
		} else wcscpy_s(projectRoot, MAX_PATH, modulePath);
	} else {
		wcscpy_s(projectRoot, MAX_PATH, modulePath);
		WCHAR* lastSlash = wcsrchr(projectRoot, L'\\');
		if (lastSlash) { *lastSlash = L'\0'; lastSlash = wcsrchr(projectRoot, L'\\'); if (lastSlash) *lastSlash = L'\0'; }
	}
	swprintf_s(outPath, pathSize, L"%s\\MapData", projectRoot);
	DWORD attrs = GetFileAttributesW(outPath);
	if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
		wcscpy_s(outPath, pathSize, projectRoot);
	return true;
}

bool EditorMapFileIO::ShowSaveFileDialog(DontStarve_EditorMain* pMain, WCHAR* fileName, DWORD fileNameSize) {
	(void)pMain;
	WCHAR mapDataPath[MAX_PATH];
	GetMapDataDialogPath(mapDataPath, MAX_PATH);
	fileName[0] = L'\0';
	OPENFILENAME ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_hWnd;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = fileNameSize;
	ofn.lpstrFilter = L"Map Files (*.dsm)\0*.dsm\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = mapDataPath;
	ofn.lpstrTitle = L"Save Map";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = L"dsm";
	return GetSaveFileName(&ofn) != 0;
}

bool EditorMapFileIO::ShowOpenFileDialog(DontStarve_EditorMain* pMain, WCHAR* fileName, DWORD fileNameSize) {
	(void)pMain;
	WCHAR mapDataPath[MAX_PATH];
	GetMapDataDialogPath(mapDataPath, MAX_PATH);
	fileName[0] = L'\0';
	OPENFILENAME ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_hWnd;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = fileNameSize;
	ofn.lpstrFilter = L"Map Files (*.dsm)\0*.dsm\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = mapDataPath;
	ofn.lpstrTitle = L"Open Map";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	return GetOpenFileName(&ofn) != 0;
}

bool EditorMapFileIO::LoadMap(DontStarve_EditorMain* pMain, const WCHAR* filename) {
	if (GetFileAttributesW(filename) == INVALID_FILE_ATTRIBUTES) {
		OutputDebugStringW(L"맵 파일을 찾을 수 없습니다: ");
		OutputDebugStringW(filename);
		OutputDebugStringW(L"\n");
		return false;
	}
	std::wstring pathStr(filename);
	size_t pathSep = pathStr.find_last_of(L"\\/");
	pMain->m_lastMapDirectory = (pathSep != std::wstring::npos) ? pathStr.substr(0, pathSep + 1) : L"";
	LoadColliderTemplates(pMain);
	
	// Function.h의 공통 파싱 함수 사용
	MapData mapData;
	auto getObjectResourceInfo = [pMain](GameObjectID id) -> const ResourcePathUtils::ObjectResourceDef* {
		// Editor는 절대경로만 사용하므로, 기본 정보만 반환
		return nullptr;
	};
	
	if (!ResourcePathUtils::ParseMapFileInto(filename, mapData, getObjectResourceInfo)) {
		OutputDebugStringW(L"맵 파일 파싱 실패: ");
		OutputDebugStringW(filename);
		OutputDebugStringW(L"\n");
		return false;
	}
	
	// MapData를 DontStarve_EditorMain에 복사
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			pMain->m_tileMap[y][x] = ResourcePathUtils::TileResourceDef();
			pMain->m_walkableAreaMap[y][x] = true;
		}
	}
	
	// 타일 데이터 복사 (절대경로로 직접 복사)
	for (int y = 0; y < mapData.mapHeight && y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < mapData.mapWidth && x < MAP_WIDTH; ++x) {
			const ResourcePathUtils::TileResourceDef& tile = mapData.tiles[x][y];
			if (tile.type != TILE_NONE && tile.id != TILEID_NONE) {
				pMain->m_tileMap[y][x] = tile;
			}
		}
	}
	
	// 오브젝트 데이터 복사
	pMain->m_gameObjects = mapData.gameObjects;
	
	// Walkable 영역 복사
	for (int y = 0; y < mapData.mapHeight && y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < mapData.mapWidth && x < MAP_WIDTH; ++x) {
			pMain->m_walkableAreaMap[y][x] = mapData.walkableAreas[x][y];
		}
	}
	
	// 플레이어 스폰 설정
	if (mapData.playerSpawn.x >= 0 && mapData.playerSpawn.y >= 0) {
		pMain->m_playerSpawnPoint = Gdiplus::PointF(mapData.playerSpawn.x, mapData.playerSpawn.y);
		pMain->m_hasPlayerSpawn = true;
	}
	else {
		pMain->m_hasPlayerSpawn = false;
		pMain->m_playerSpawnPoint = Gdiplus::PointF(0.0f, 0.0f);
	}
	
	// 오브젝트 콜라이더 기본값 설정 (Editor 전용 로직)
	for (ResourcePathUtils::ObjectResourceDef& obj : pMain->m_gameObjects) {
		if (!obj.hasCollider) continue;
		if (obj.colliderType == COLLIDER_BOX && obj.colliderWidth > 0 && obj.colliderHeight > 0) continue;
		if (obj.colliderType == COLLIDER_CIRCLE && obj.colliderRadius > 0.0f) continue;
		const ResourcePathUtils::ObjectResourceDef* ov = pMain->m_pResources->GetObjectVariant(obj.type, obj.id);
		if (!ov) continue;
		// 이미지 크기는 절대경로에서 로드해야 하지만, 여기서는 기본값만 설정
		obj.colliderType = COLLIDER_BOX;
		obj.colliderOffsetX = -(int)(ov->pivotX * 32); // 기본값
		obj.colliderOffsetY = -(int)(ov->pivotY * 32); // 기본값
		obj.colliderWidth = iw;
		obj.colliderHeight = ih;
		obj.colliderCenterX = iw * (0.5f - ov->pivotX);
		obj.colliderCenterY = ih * (0.5f - ov->pivotY);
		obj.colliderRadius = (iw < ih ? (float)iw : (float)ih) * 0.5f;
	}
	
	if (!pMain->m_hasPlayerSpawn) {
		float cx = (MAP_WIDTH / 2.0f) * TILE_SIZE;
		float cy = (MAP_HEIGHT / 2.0f) * TILE_SIZE;
		pMain->m_playerSpawnPoint = Gdiplus::PointF(cx, cy);
		pMain->m_hasPlayerSpawn = true;
	}
	
	pMain->m_pLayerComposer->SetTileLayerDirty(true);
	pMain->m_pLayerComposer->SetObjectLayerDirty(true);
	pMain->m_objectsDirty = true;
	OutputDebugStringW(L"맵 불러오기 완료: ");
	OutputDebugStringW(filename);
	OutputDebugStringW(L"\n");
	return true;
}
