#pragma once

#include "Struct.h"
#include <vector>
#include <map>

class EditorResourceManager;

class EditorPalette
{
public:
	enum class SubPaletteClickResult {
		NotHandled,       // sub-palette was not open
		ClosedWithSelection, // user selected an item; sub-palette closed
		ClosedOutside     // user clicked outside; sub-palette closed
	};

	EditorPalette() = default;
	~EditorPalette() = default;

	void InitPalette(int clientWidth, int clientHeight, EditorResourceManager* pResources, bool showTiles = true);

	void DrawPalette(Gdiplus::Graphics* pGraphics) const;
	void DrawSubPalette(Gdiplus::Graphics* pGraphics) const;
	void ComposePaletteLayer(Gdiplus::Bitmap* pLayerBitmap, bool* pDirty);

	SubPaletteClickResult HandleSubPaletteClick(POINT clickPoint);
	bool HandleMainPaletteClick(POINT clickPoint, int clientHeight);

	void CloseSubPalette();
	void ResetSelection();

	int GetSelectedPaletteIndex() const { return m_selectedPaletteIndex; }
	const RECT& GetPaletteRect() const { return m_paletteRect; }
	size_t GetPaletteItemCount() const { return m_paletteItems.size(); }
	const PaletteItem* GetPaletteItem(size_t index) const;
	bool IsSubPaletteOpen() const { return m_subPalette.isOpen; }

	const ResourcePathUtils::TileResourceDef* GetSelectedTileVariant() const;
	const ResourcePathUtils::ObjectResourceDef* GetSelectedObjectVariant() const;
	TileID GetSelectedTileID() const;
	GameObjectID GetSelectedGameObjectID() const;

private:
	struct SubPaletteData {
		bool isOpen = false;
		ItemCategory category = ItemCategory::CATEGORY_NONE;
		int targetCategoryId = -1; // TileType(int) 또는 ObjectCategory(int)
		RECT rect = { 0 };
		std::vector<RECT> itemRects;
		std::vector<std::pair<TileID, const ResourcePathUtils::TileResourceDef*>> currentTileVariantDefs;
		std::vector<std::pair<GameObjectID, const ResourcePathUtils::ObjectResourceDef*>> currentObjectVariantDefs;
		int selectedTileVariantIndex = -1;
		int selectedObjectVariantIndex = -1;
	};

	std::vector<PaletteItem> m_paletteItems;
	RECT m_paletteRect = { 0 };
	int m_selectedPaletteIndex = -1;
	SubPaletteData m_subPalette;

	EditorResourceManager* m_pResources = nullptr;
};
