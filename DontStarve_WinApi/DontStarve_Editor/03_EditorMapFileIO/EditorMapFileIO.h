#pragma once

class MapEditor;

class EditorMapFileIO
{
public:
	static bool SaveMap(MapEditor* pMain, const WCHAR* filename);
	static bool LoadMap(MapEditor* pMain, const WCHAR* filename);
	static bool ShowSaveFileDialog(MapEditor* pMain, WCHAR* fileName, DWORD fileNameSize);
	static bool ShowOpenFileDialog(MapEditor* pMain, WCHAR* fileName, DWORD fileNameSize);
	static void ShowMapSizeDialog(MapEditor* pMain, HWND parent);
};
