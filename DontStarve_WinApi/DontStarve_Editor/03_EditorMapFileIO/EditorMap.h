#pragma once

class DontStarve_EditorMain;

class EditorMap
{
public:
	static bool SaveMap(DontStarve_EditorMain* pMain, const WCHAR* filename);
	static bool LoadMap(DontStarve_EditorMain* pMain, const WCHAR* filename);
	static bool ShowSaveFileDialog(DontStarve_EditorMain* pMain, WCHAR* fileName, DWORD fileNameSize);
	static bool ShowOpenFileDialog(DontStarve_EditorMain* pMain, WCHAR* fileName, DWORD fileNameSize);
	static void LoadColliderTemplates(DontStarve_EditorMain* pMain);
	static void SaveColliderTemplates(DontStarve_EditorMain* pMain);
	static void ShowMapSizeDialog(DontStarve_EditorMain* pMain, HWND parent);
};
