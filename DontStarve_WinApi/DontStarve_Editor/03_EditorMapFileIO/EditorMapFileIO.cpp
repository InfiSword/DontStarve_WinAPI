#include "../pch.h"
#include "EditorMapFileIO.h"
#include "../00_MainEditor/MapEditor.h"
#include "../01_EditorView/EditorView.h"
#include "../02_EditorResourceManager/EditorResourceManager.h"
#include "../09_EditorLayerComposer/EditorLayerComposer.h"

struct MapSizeDlgParam {
	int curW, curH;
	int outW, outH;
	int idWidth, idHeight;
	bool ok;
};

bool EditorMapFileIO::SaveMap(MapEditor* pMain, const WCHAR* filename) {
	std::wofstream outFile(filename);
	if (!outFile.is_open()) {
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
		// 저장 시에는 GOBJ_NONE 또는 상징적인 값을 넣거나 ID만 사용 (Function.h 파서 호환)
		outFile << L"GOBJ_UNUSED," << EnumTables::GetEnumName(obj.id) << L","
			<< obj.x << L"," << obj.y << L"\n";
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
	return true;
}

static bool GetProjectRoot(WCHAR* outPath, DWORD pathSize) {
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
	wcscpy_s(outPath, pathSize, projectRoot);
	return true;
}

static bool GetGameDataDialogPath(WCHAR* outPath, DWORD pathSize) {
	WCHAR projectRoot[MAX_PATH];
	GetProjectRoot(projectRoot, MAX_PATH);
	swprintf_s(outPath, pathSize, L"%s\\GameData", projectRoot);
	DWORD attrs = GetFileAttributesW(outPath);
	if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
		wcscpy_s(outPath, pathSize, projectRoot);
	return true;
}

bool EditorMapFileIO::ShowSaveFileDialog(MapEditor* pMain, WCHAR* fileName, DWORD fileNameSize) {
	WCHAR gameDataPath[MAX_PATH];
	GetGameDataDialogPath(gameDataPath, MAX_PATH);
	fileName[0] = L'\0';
	OPENFILENAME ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_hWnd;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = fileNameSize;
	ofn.lpstrFilter = L"Map Files (*.dsm)\0*.dsm\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = gameDataPath;
	ofn.lpstrTitle = L"Save Map";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = L"dsm";
	return GetSaveFileName(&ofn) != 0;
}

bool EditorMapFileIO::ShowOpenFileDialog(MapEditor* pMain, WCHAR* fileName, DWORD fileNameSize) {
	WCHAR gameDataPath[MAX_PATH];
	GetGameDataDialogPath(gameDataPath, MAX_PATH);
	fileName[0] = L'\0';
	OPENFILENAME ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_hWnd;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = fileNameSize;
	ofn.lpstrFilter = L"Map Files (*.dsm)\0*.dsm\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = gameDataPath;
	ofn.lpstrTitle = L"Open Map";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	return GetOpenFileName(&ofn) != 0;
}

bool EditorMapFileIO::LoadMap(MapEditor* pMain, const WCHAR* filename) {
	if (GetFileAttributesW(filename) == INVALID_FILE_ATTRIBUTES) {
		return false;
	}
	std::wstring pathStr(filename);
	size_t pathSep = pathStr.find_last_of(L"\\/");
	pMain->m_lastMapDirectory = (pathSep != std::wstring::npos) ? pathStr.substr(0, pathSep + 1) : L"";

	MapData mapData;
	auto getObjectResourceInfo = [pMain](int /*type_unused*/, GameObjectID id) -> const ResourcePathUtils::ObjectResourceDef* {
		return pMain->m_pResources->GetObjectVariant(id);
	};
	if (!ResourcePathUtils::ParseMapFileInto(filename, mapData, getObjectResourceInfo)) {
		return false;
	}

	pMain->m_mapWidth = max(1, min(MAP_WIDTH, mapData.mapWidth));
	pMain->m_mapHeight = max(1, min(MAP_HEIGHT, mapData.mapHeight));
	
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			pMain->m_tileMap[y][x] = ResourcePathUtils::TileResourceDef();
			pMain->m_walkableAreaMap[y][x] = true;
		}
	}

	for (int y = 0; y < mapData.mapHeight && y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < mapData.mapWidth && x < MAP_WIDTH; ++x) {
			const ResourcePathUtils::TileResourceDef& tile = mapData.tiles[x][y];
			if (tile.type != TILE_NONE && tile.id != TILEID_NONE) {
				pMain->m_tileMap[y][x] = tile;
			}
		}
	}
	
	pMain->m_gameObjects = mapData.gameObjects;
	for (ResourcePathUtils::ObjectResourceDef& obj : pMain->m_gameObjects) {
		const ResourcePathUtils::ObjectResourceDef* ov = pMain->m_pResources->GetObjectVariant(obj.id);
		if (!ov) continue;
		
		obj.baseDir = ov->baseDir;
		obj.imageName = ov->imageName;
		obj.pivotX = ov->pivotX;
		obj.pivotY = ov->pivotY;
		obj.hasCollider = ov->hasCollider;
		obj.colliderType = ov->colliderType;
		obj.colliderOffsetX = ov->colliderOffsetX;
		obj.colliderOffsetY = ov->colliderOffsetY;
		obj.colliderWidth = ov->colliderWidth;
		obj.colliderHeight = ov->colliderHeight;
		obj.colliderCenterX = ov->colliderCenterX;
		obj.colliderCenterY = ov->colliderCenterY;
		obj.colliderRadius = ov->colliderRadius;
	}

	for (int y = 0; y < mapData.mapHeight && y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < mapData.mapWidth && x < MAP_WIDTH; ++x) {
			pMain->m_walkableAreaMap[y][x] = mapData.walkableAreas[x][y];
		}
	}

	if (mapData.playerSpawn.x >= 0 && mapData.playerSpawn.y >= 0) {
		pMain->m_playerSpawnPoint = Gdiplus::PointF(mapData.playerSpawn.x, mapData.playerSpawn.y);
		pMain->m_hasPlayerSpawn = true;
	}
	else {
		pMain->m_hasPlayerSpawn = false;
		pMain->m_playerSpawnPoint = Gdiplus::PointF(0.0f, 0.0f);
	}

	if (!pMain->m_hasPlayerSpawn) {
		float cx = (pMain->GetMapWidth() / 2.0f) * TILE_SIZE;
		float cy = (pMain->GetMapHeight() / 2.0f) * TILE_SIZE;
		pMain->m_playerSpawnPoint = Gdiplus::PointF(cx, cy);
		pMain->m_hasPlayerSpawn = true;
	}

	pMain->m_pLayerComposer->SetGridLayerDirty(true);
	pMain->m_pLayerComposer->SetTileLayerDirty(true);
	pMain->m_pLayerComposer->SetObjectLayerDirty(true);
	pMain->m_objectsDirty = true;
	
	HWND hWnd = g_hWnd;
	RECT clientRect;
	if (GetClientRect(hWnd, &clientRect)) {
		int clientWidth = clientRect.right - clientRect.left;
		int clientHeight = clientRect.bottom - clientRect.top;
		
		float mapCenterX = (pMain->m_mapWidth / 2.0f) * TILE_SIZE;
		float mapCenterY = (pMain->m_mapHeight / 2.0f) * TILE_SIZE;
		
		float zoomFactor = pMain->m_pView->GetZoomFactor();
		int offsetX = (int)(mapCenterX * zoomFactor - clientWidth / 2.0f);
		int offsetY = (int)(mapCenterY * zoomFactor - clientHeight / 2.0f);
		
		pMain->m_pView->SetMapOffsetClamped(offsetX, offsetY, clientWidth, clientHeight, 
		                                     pMain->m_mapWidth, pMain->m_mapHeight);
	}
	
	if (hWnd) {
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
	}
	
	return true;
}

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
		CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", nullptr, WS_CHILD | WS_VISIBLE | ES_NUMBER, 88, 10, 50, 18, hWnd, (HMENU)(UINT_PTR)p->idWidth, hInst, nullptr);
		CreateWindowW(L"Static", L"Height (1-50):", WS_CHILD | WS_VISIBLE, 10, 38, 72, 18, hWnd, nullptr, hInst, nullptr);
		CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", nullptr, WS_CHILD | WS_VISIBLE | ES_NUMBER, 88, 36, 50, 18, hWnd, (HMENU)(UINT_PTR)p->idHeight, hInst, nullptr);
		CreateWindowW(L"Button", L"OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 45, 62, 50, 22, hWnd, (HMENU)IDOK, hInst, nullptr);
		CreateWindowW(L"Button", L"Cancel", WS_CHILD | WS_VISIBLE, 105, 62, 50, 22, hWnd, (HMENU)IDCANCEL, hInst, nullptr);
		WCHAR buf[16];
		swprintf_s(buf, L"%d", p->curW);
		SetDlgItemTextW(hWnd, p->idWidth, buf);
		swprintf_s(buf, L"%d", p->curH);
		SetDlgItemTextW(hWnd, p->idHeight, buf);
		return 0;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			p = (MapSizeDlgParam*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
			if (p) {
				int w = GetDlgItemInt(hWnd, p->idWidth, nullptr, TRUE);
				int h = GetDlgItemInt(hWnd, p->idHeight, nullptr, TRUE);
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

void EditorMapFileIO::ShowMapSizeDialog(MapEditor* pMain, HWND parent) {
	MapSizeDlgParam param = { pMain->GetMapWidth(), pMain->GetMapHeight(), 0, 0, pMain->GetMapSizeDlgControlIdWidth(), pMain->GetMapSizeDlgControlIdHeight(), false };
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
