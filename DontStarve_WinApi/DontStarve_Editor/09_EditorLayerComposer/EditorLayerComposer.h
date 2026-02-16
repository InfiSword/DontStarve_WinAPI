#pragma once

class EditorView;
class EditorResourceManager;
class DontStarve_EditorMain;

class EditorLayerComposer
{
	friend class DontStarve_EditorMain;

public:
	EditorLayerComposer();
	~EditorLayerComposer();

	void SetDependencies(EditorView* pView, EditorResourceManager* pResources, DontStarve_EditorMain* pMain);

	// 레이어 합성 함수들
	void ComposeGridLayer();
	void ComposeTileLayer();
	void ComposeObjectLayer();

	// 레이어 그리기 함수들
	void DrawLayers(Gdiplus::Graphics* pGraphics);

	// 레이어 비트맵 관리
	void ResizeLayerBitmaps(UINT width, UINT height);
	void DeleteLayerBitmaps();
	Gdiplus::Bitmap* GetGridLayerBitmap() const { return m_gridLayerBitmap; }
	Gdiplus::Bitmap* GetTileLayerBitmap() const { return m_tileLayerBitmap; }
	Gdiplus::Bitmap* GetObjectLayerBitmap() const { return m_objectLayerBitmap; }

	// Dirty 플래그 관리
	bool IsGridLayerDirty() const { return m_gridLayerDirty; }
	bool IsTileLayerDirty() const { return m_tileLayerDirty; }
	bool IsObjectLayerDirty() const { return m_objectLayerDirty; }
	void SetGridLayerDirty(bool dirty) { m_gridLayerDirty = dirty; }
	void SetTileLayerDirty(bool dirty) { m_tileLayerDirty = dirty; }
	void SetObjectLayerDirty(bool dirty) { m_objectLayerDirty = dirty; }

private:
	EditorView* m_pView = nullptr;
	EditorResourceManager* m_pResources = nullptr;
	DontStarve_EditorMain* m_pMain = nullptr;

	// 레이어 비트맵들
	Gdiplus::Bitmap* m_gridLayerBitmap = nullptr;
	Gdiplus::Bitmap* m_tileLayerBitmap = nullptr;
	Gdiplus::Bitmap* m_objectLayerBitmap = nullptr;

	// Dirty 플래그들
	bool m_gridLayerDirty = true;
	bool m_tileLayerDirty = true;
	bool m_objectLayerDirty = true;

	// 내부 그리기 함수들
	void DrawGrid(Gdiplus::Graphics* pGraphics);
	void DrawTileMap(Gdiplus::Graphics* pGraphics);
	void DrawObjects(Gdiplus::Graphics* pGraphics);
};
