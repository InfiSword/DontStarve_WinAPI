#include "../pch.h"
#include "EditorMap.h"
#include "../Resource.h"
#include "../00_MainEditor/DontStarve_EditorMain.h"
#include "../01_EditorView/EditorView.h"
#include "../02_EditorResourceManager/EditorResourceManager.h"
#include "../09_EditorLayerComposer/EditorLayerComposer.h"
#include <memory>
#include <commdlg.h>

bool EditorMap::SaveMap(DontStarve_EditorMain* pMain, const WCHAR* filename) {
	std::wofstream outFile(filename);
	if (!outFile.is_open()) {
		OutputDebugStringW(L"맵 파일 열기 실패: ");
		OutputDebugStringW(filename);
		OutputDebugStringW(L"\n");
		return false;
	}
	outFile << L"# MAP_METADATA\n";
	outFile << L"MAP_WIDTH=" << pMain->GetMapWidth() << L"\n";
	outFile << L"MAP_HEIGHT=" << pMain->GetMapHeight() << L"\n";
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
	int saveW = pMain->GetMapWidth(), saveH = pMain->GetMapHeight();
	for (int y = 0; y < saveH; ++y) {
		for (int x = 0; x < saveW; ++x) {
			ResourcePathUtils::TileResourceDef tile = pMain->m_tileMap[y][x];
			outFile << EnumTables::GetEnumName(tile.type) << L"," << EnumTables::GetEnumName(tile.id);
			if (x < saveW - 1) outFile << L",";
		}
		outFile << L"\n";
	}
	outFile << L"\n# OBJECTS\n";
	for (const auto& obj : pMain->m_gameObjects) {
		outFile << EnumTables::GetEnumName(obj.type) << L"," << EnumTables::GetEnumName(obj.id) << L","
			<< obj.x << L"," << obj.y << L"," << obj.baseDir << L","
			<< obj.pivotX << L"," << obj.pivotY << L"\n";
		outFile << L"Collider," << obj.hasCollider << L"," << (int)obj.colliderType << L","
			<< obj.colliderOffsetX << L"," << obj.colliderOffsetY << L","
			<< obj.colliderWidth << L"," << obj.colliderHeight << L","
			<< obj.colliderCenterX << L"," << obj.colliderCenterY << L"," << obj.colliderRadius << L"\n";
	}
	outFile << L"\n# WALKABLE_AREAS\n";
	for (int y = 0; y < saveH; ++y) {
		for (int x = 0; x < saveW; ++x) {
			outFile << (pMain->m_walkableAreaMap[y][x] ? L"1" : L"0");
			if (x < saveW - 1) outFile << L",";
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

void EditorMap::LoadColliderTemplates(DontStarve_EditorMain* pMain) {
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

void EditorMap::SaveColliderTemplates(DontStarve_EditorMain* pMain) {
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

bool EditorMap::ShowSaveFileDialog(DontStarve_EditorMain* pMain, WCHAR* fileName, DWORD fileNameSize) {
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

bool EditorMap::ShowOpenFileDialog(DontStarve_EditorMain* pMain, WCHAR* fileName, DWORD fileNameSize) {
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

bool EditorMap::LoadMap(DontStarve_EditorMain* pMain, const WCHAR* filename) {
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

	// Function.h의 공통 파싱 함수 사용 (type+id로 리소스 조회 → baseDir/imageName 복원, 아틀라스 제거 후에도 정상 로드)
	MapData mapData;
	auto getObjectResourceInfo = [pMain](GameObjectType type, GameObjectID id) -> const ResourcePathUtils::ObjectResourceDef* {
		const ResourcePathUtils::ObjectResourceDef* result = pMain->m_pResources->GetObjectVariant(type, id);
		if (!result) {
			std::wstringstream debugSS;
			debugSS << L"[맵 로드] 경고: 오브젝트 리소스를 찾을 수 없음 (Type: " << (int)type << L", ID: " << (int)id << L")\n";
			OutputDebugStringW(debugSS.str().c_str());
		}
		return result;
	};

	if (!ResourcePathUtils::ParseMapFileInto(filename, mapData, getObjectResourceInfo)) {
		OutputDebugStringW(L"맵 파일 파싱 실패: ");
		OutputDebugStringW(filename);
		OutputDebugStringW(L"\n");
		return false;
	}

	// 맵 크기 설정 (파일에서 읽은 크기, 1~MAP_WIDTH/HEIGHT로 클램프)
	pMain->m_mapWidth = max(1, min(MAP_WIDTH, mapData.mapWidth));
	pMain->m_mapHeight = max(1, min(MAP_HEIGHT, mapData.mapHeight));
	
	std::wstringstream mapSizeDebugSS;
	mapSizeDebugSS << L"[맵 로드] 맵 크기 설정: " << pMain->m_mapWidth << L" x " << pMain->m_mapHeight 
	               << L" (파일: " << mapData.mapWidth << L" x " << mapData.mapHeight << L")\n";
	OutputDebugStringW(mapSizeDebugSS.str().c_str());

	// MapData를 DontStarve_EditorMain에 복사
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			pMain->m_tileMap[y][x] = ResourcePathUtils::TileResourceDef();
			pMain->m_walkableAreaMap[y][x] = true;
		}
	}

	// 타일 데이터 복사 (절대경로로 직접 복사)
	// MapData.tiles[x][y] → m_tileMap[y][x] 형식 변환 (맵 파일은 열 우선, 에디터는 행 우선)
	int loadedTileCount = 0;
	for (int y = 0; y < mapData.mapHeight && y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < mapData.mapWidth && x < MAP_WIDTH; ++x) {
			const ResourcePathUtils::TileResourceDef& tile = mapData.tiles[x][y];
			if (tile.type != TILE_NONE && tile.id != TILEID_NONE) {
				pMain->m_tileMap[y][x] = tile;
				loadedTileCount++;
				
				// 처음 몇 개 타일 상세 정보 출력
				if (loadedTileCount <= 3) {
					std::wstringstream detailSS;
					detailSS << L"  타일[" << y << L"][" << x << L"]: type=" << (int)tile.type 
					         << L", id=" << (int)tile.id << L", imageName=" << tile.imageName << L"\n";
					OutputDebugStringW(detailSS.str().c_str());
				}
			}
		}
	}
	
	std::wstringstream debugSS;
	debugSS << L"타일 로드 완료: " << loadedTileCount << L"개 타일이 복사됨\n";
	OutputDebugStringW(debugSS.str().c_str());

	// 오브젝트 데이터 복사
	pMain->m_gameObjects = mapData.gameObjects;
	debugSS.str(L"");
	debugSS << L"오브젝트 로드 완료: " << pMain->m_gameObjects.size() << L"개 오브젝트가 복사됨\n";
	OutputDebugStringW(debugSS.str().c_str());

	// Walkable 영역 복사 (mapData와 에디터 모두 [x][y] 형식 사용)
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

	// 오브젝트 콜라이더 기본값 설정 (이미지 파일에서 크기 로드)
	for (ResourcePathUtils::ObjectResourceDef& obj : pMain->m_gameObjects) {
		if (!obj.hasCollider) continue;
		if (obj.colliderType == COLLIDER_BOX && obj.colliderWidth > 0 && obj.colliderHeight > 0) continue;
		if (obj.colliderType == COLLIDER_CIRCLE && obj.colliderRadius > 0.0f) continue;
		
		const ResourcePathUtils::ObjectResourceDef* ov = pMain->m_pResources->GetObjectVariant(obj.type, obj.id);
		if (!ov) {
			std::wstringstream debugSS;
			debugSS << L"[맵 로드] 경고: 오브젝트 리소스 정보 없음 (Type: " << (int)obj.type << L", ID: " << (int)obj.id << L")\n";
			OutputDebugStringW(debugSS.str().c_str());
			continue;
		}
		
		if (ov->imageName.empty()) {
			OutputDebugStringW(L"[맵 로드] 경고: 오브젝트 이미지 경로가 비어있음\n");
			continue;
		}
		
		std::wstring fullPath = ov->baseDir;
		if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') {
			fullPath += L"\\";
		}
		fullPath += ov->imageName;
		std::unique_ptr<Gdiplus::Bitmap> pBitmap(Gdiplus::Bitmap::FromFile(fullPath.c_str()));
		int iw = (pBitmap && pBitmap->GetLastStatus() == Gdiplus::Ok) ? (int)pBitmap->GetWidth() : 32;
		int ih = (pBitmap && pBitmap->GetLastStatus() == Gdiplus::Ok) ? (int)pBitmap->GetHeight() : 32;
		obj.colliderType = COLLIDER_BOX;
		obj.colliderOffsetX = -(int)(ov->pivotX * iw);
		obj.colliderOffsetY = -(int)(ov->pivotY * ih);
		obj.colliderWidth = iw;
		obj.colliderHeight = ih;
		obj.colliderCenterX = iw * (0.5f - ov->pivotX);
		obj.colliderCenterY = ih * (0.5f - ov->pivotY);
		obj.colliderRadius = (iw < ih ? (float)iw : (float)ih) * 0.5f;
	}

	if (!pMain->m_hasPlayerSpawn) {
		float cx = (pMain->GetMapWidth() / 2.0f) * TILE_SIZE;
		float cy = (pMain->GetMapHeight() / 2.0f) * TILE_SIZE;
		pMain->m_playerSpawnPoint = Gdiplus::PointF(cx, cy);
		pMain->m_hasPlayerSpawn = true;
	}

	// 레이어 갱신 플래그 설정 및 강제 렌더링
	pMain->m_pLayerComposer->SetGridLayerDirty(true);
	pMain->m_pLayerComposer->SetTileLayerDirty(true);
	pMain->m_pLayerComposer->SetObjectLayerDirty(true);
	pMain->m_objectsDirty = true;
	
	// 카메라를 맵 중심으로 이동 (로드된 맵이 보이도록)
	HWND hWnd = g_hWnd;
	RECT clientRect;
	if (GetClientRect(hWnd, &clientRect)) {
		int clientWidth = clientRect.right - clientRect.left;
		int clientHeight = clientRect.bottom - clientRect.top;
		
		// 맵 중심 좌표 (월드 좌표)
		float mapCenterX = (pMain->m_mapWidth / 2.0f) * TILE_SIZE;
		float mapCenterY = (pMain->m_mapHeight / 2.0f) * TILE_SIZE;
		
		// 카메라 오프셋 계산 (맵 중심이 화면 중앙에 오도록)
		float zoomFactor = pMain->m_pView->GetZoomFactor();
		int offsetX = (int)(mapCenterX * zoomFactor - clientWidth / 2.0f);
		int offsetY = (int)(mapCenterY * zoomFactor - clientHeight / 2.0f);
		
		pMain->m_pView->SetMapOffsetClamped(offsetX, offsetY, clientWidth, clientHeight, 
		                                     pMain->m_mapWidth, pMain->m_mapHeight);
		
		std::wstringstream debugSS;
		debugSS << L"[맵 로드] 카메라를 맵 중심으로 이동: (" << offsetX << L", " << offsetY << L")\n";
		OutputDebugStringW(debugSS.str().c_str());
	}
	
	// 즉시 화면 갱신 강제
	if (hWnd) {
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
	}
	
	OutputDebugStringW(L"===== 맵 불러오기 완료 =====\n");
	OutputDebugStringW(L"파일: ");
	OutputDebugStringW(filename);
	OutputDebugStringW(L"\n");
	std::wstringstream summaryDebug;
	summaryDebug << L"맵 크기: " << pMain->m_mapWidth << L"x" << pMain->m_mapHeight << L"\n";
	summaryDebug << L"타일: " << loadedTileCount << L"개, 오브젝트: " << pMain->m_gameObjects.size() << L"개\n";
	summaryDebug << L"===========================\n";
	OutputDebugStringW(summaryDebug.str().c_str());
	return true;
}

// ----- 맵 크기 다이얼로그 -----
struct MapSizeDlgParam {
	int curW, curH;
	int outW, outH;
	bool ok;
};

static LRESULT CALLBACK MapSizeDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	MapSizeDlgParam* p = (MapSizeDlgParam*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
	switch (msg) {
	case WM_CREATE:
	{
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		p = (MapSizeDlgParam*)cs->lpCreateParams;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)p);
		HINSTANCE hInst = (HINSTANCE)GetWindowLongPtrW(hWnd, GWLP_HINSTANCE);
		CreateWindowW(L"Static", L"Width (1-50):", WS_CHILD | WS_VISIBLE, 10, 12, 72, 18, hWnd, nullptr, hInst, nullptr);
		CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", nullptr, WS_CHILD | WS_VISIBLE | ES_NUMBER, 88, 10, 50, 18, hWnd, (HMENU)(UINT_PTR)IDC_MAP_WIDTH, hInst, nullptr);
		CreateWindowW(L"Static", L"Height (1-50):", WS_CHILD | WS_VISIBLE, 10, 38, 72, 18, hWnd, nullptr, hInst, nullptr);
		CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", nullptr, WS_CHILD | WS_VISIBLE | ES_NUMBER, 88, 36, 50, 18, hWnd, (HMENU)(UINT_PTR)IDC_MAP_HEIGHT, hInst, nullptr);
		CreateWindowW(L"Button", L"OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 45, 62, 50, 22, hWnd, (HMENU)IDOK, hInst, nullptr);
		CreateWindowW(L"Button", L"Cancel", WS_CHILD | WS_VISIBLE, 105, 62, 50, 22, hWnd, (HMENU)IDCANCEL, hInst, nullptr);
		WCHAR buf[16];
		swprintf_s(buf, L"%d", p->curW);
		SetDlgItemTextW(hWnd, IDC_MAP_WIDTH, buf);
		swprintf_s(buf, L"%d", p->curH);
		SetDlgItemTextW(hWnd, IDC_MAP_HEIGHT, buf);
		return 0;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			p = (MapSizeDlgParam*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
			if (p) {
				int w = GetDlgItemInt(hWnd, IDC_MAP_WIDTH, nullptr, TRUE);
				int h = GetDlgItemInt(hWnd, IDC_MAP_HEIGHT, nullptr, TRUE);
				if (w < 1) w = 1; if (w > MAP_WIDTH) w = MAP_WIDTH;
				if (h < 1) h = 1; if (h > MAP_HEIGHT) h = MAP_HEIGHT;
				p->outW = w;
				p->outH = h;
				p->ok = true;
			}
			DestroyWindow(hWnd);
			return 0;
		}
		if (LOWORD(wParam) == IDCANCEL) {
			DestroyWindow(hWnd);
			return 0;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	}
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

void EditorMap::ShowMapSizeDialog(DontStarve_EditorMain* pMain, HWND parent) {
	MapSizeDlgParam param = { pMain->GetMapWidth(), pMain->GetMapHeight(), 0, 0, false };
	HINSTANCE hInst = (HINSTANCE)GetWindowLongPtrW(parent, GWLP_HINSTANCE);
	WNDCLASSEXW wc = {};
	wc.cbSize = sizeof(wc);
	if (!GetClassInfoExW(hInst, L"MapSizeDlgClass", &wc)) {
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = MapSizeDlgProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInst;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszClassName = L"MapSizeDlgClass";
		RegisterClassExW(&wc);
	}
	HWND hDlg = CreateWindowExW(WS_EX_DLGMODALFRAME | WS_EX_TOPMOST, L"MapSizeDlgClass", L"맵 크기",
		WS_POPUP | WS_CAPTION | WS_SYSMENU, 0, 0, 220, 120, parent, nullptr, hInst, &param);
	if (!hDlg) return;
	SetWindowLongPtrW(hDlg, GWLP_USERDATA, (LONG_PTR)&param);
	RECT rc, rcMain;
	GetWindowRect(hDlg, &rc);
	GetWindowRect(parent, &rcMain);
	SetWindowPos(hDlg, nullptr,
		rcMain.left + (rcMain.right - rcMain.left - (rc.right - rc.left)) / 2,
		rcMain.top + (rcMain.bottom - rcMain.top - (rc.bottom - rc.top)) / 2,
		0, 0, SWP_NOSIZE | SWP_NOZORDER);
	ShowWindow(hDlg, SW_SHOW);
	EnableWindow(parent, FALSE);
	MSG dlgMsg;
	while (IsWindow(hDlg) && GetMessage(&dlgMsg, nullptr, 0, 0)) {
		if (!IsDialogMessage(hDlg, &dlgMsg)) {
			TranslateMessage(&dlgMsg);
			DispatchMessage(&dlgMsg);
		}
	}
	EnableWindow(parent, TRUE);
	SetForegroundWindow(parent);
	if (param.ok) {
		pMain->SetMapSize(param.outW, param.outH);
		MessageBoxW(parent, L"맵 크기가 적용되었습니다.", L"맵 크기", MB_OK | MB_ICONINFORMATION);
	}
}
