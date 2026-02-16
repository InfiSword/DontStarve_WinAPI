#include "../pch.h"
#include "EditorPalette.h"
#include "../02_EditorResourceManager/EditorResourceManager.h"

void EditorPalette::InitPalette(int clientWidth, int clientHeight, const EditorResourceManager* pResources) {
	m_pResources = pResources;
	if (!m_pResources) return;

	int paletteWidth = 140;
	int paletteHeight = clientHeight;

	m_paletteRect = {
		clientWidth - paletteWidth,
		0,
		clientWidth,
		paletteHeight
	};

	m_paletteItems.clear();

	const int itemSize = 64;
	const int padding = 5;
	int currentY = m_paletteRect.top + padding;

	for (int i = 0; i < TILE_COUNT; ++i) {
		TileType type = (TileType)i;
		if (type == TILE_NONE || type == TILE_COUNT) continue;

		auto type_map_it = m_pResources->GetTileVariants().find(type);
		if (type_map_it != m_pResources->GetTileVariants().end() && !type_map_it->second.empty()) {
			const ResourcePathUtils::TileResourceDef& sampleVariant = type_map_it->second.begin()->second;
			Gdiplus::Bitmap* iconBitmap = nullptr;
			Gdiplus::RectF iconSrcRect(0, 0, 0, 0);
			if (!sampleVariant.imageName.empty()) {
				std::wstring fullPath = ResourcePathUtils::BuildResourcePath(sampleVariant.baseDir, sampleVariant.imageName);
				iconBitmap = BitmapUtils::LoadBitmapFromFile(fullPath.c_str());
				if (iconBitmap && iconBitmap->GetLastStatus() == Gdiplus::Ok) {
					iconSrcRect = Gdiplus::RectF(0, 0, (float)iconBitmap->GetWidth(), (float)iconBitmap->GetHeight());
				}
			}
			RECT itemRect = { m_paletteRect.left + padding, currentY, m_paletteRect.left + padding + itemSize, currentY + itemSize };
			m_paletteItems.push_back({ CATEGORY_TILE, (int)type, (UINT)type, itemRect, iconBitmap, iconSrcRect });
			currentY += itemSize + padding;
		}
	}

	for (int i = 0; i < GOBJ_COUNT; ++i) {
		GameObjectType type = (GameObjectType)i;
		if (type == GOBJ_NONE || type == GOBJ_COUNT) continue;

		auto type_map_it = m_pResources->GetObjectVariants().find(type);
		if (type_map_it != m_pResources->GetObjectVariants().end() && !type_map_it->second.empty()) {
			const ResourcePathUtils::ObjectResourceDef& sampleVariant = type_map_it->second.begin()->second;
			Gdiplus::Bitmap* iconBitmap = nullptr;
			Gdiplus::RectF iconSrcRect(0, 0, 0, 0);
			if (!sampleVariant.imageName.empty()) {
				std::wstring fullPath = ResourcePathUtils::BuildResourcePath(sampleVariant.baseDir, sampleVariant.imageName);
				iconBitmap = BitmapUtils::LoadBitmapFromFile(fullPath.c_str());
				if (iconBitmap && iconBitmap->GetLastStatus() == Gdiplus::Ok) {
					iconSrcRect = Gdiplus::RectF(0, 0, (float)iconBitmap->GetWidth(), (float)iconBitmap->GetHeight());
				}
			}
			RECT itemRect = { m_paletteRect.left + padding, currentY, m_paletteRect.left + padding + itemSize, currentY + itemSize };
			m_paletteItems.push_back({ CATEGORY_OBJECT, (int)type, (UINT)type, itemRect, iconBitmap, iconSrcRect });
			currentY += itemSize + padding;
		}
	}

	if (!m_paletteItems.empty()) {
		m_selectedPaletteIndex = 0;
	}
	m_subPalette.isOpen = false;
	m_subPalette.selectedTileVariantIndex = -1;
	m_subPalette.selectedObjectVariantIndex = -1;
}

void EditorPalette::DrawPalette(Gdiplus::Graphics* pGraphics) const {
	if (!pGraphics) return;

	Gdiplus::SolidBrush paletteBackgroundBrush(Gdiplus::Color(100, 50, 50, 50));
	pGraphics->FillRectangle(&paletteBackgroundBrush,
		(Gdiplus::REAL)0, (Gdiplus::REAL)0,
		(Gdiplus::REAL)(m_paletteRect.right - m_paletteRect.left),
		(Gdiplus::REAL)(m_paletteRect.bottom - m_paletteRect.top));

	for (size_t i = 0; i < m_paletteItems.size(); ++i) {
		const PaletteItem& item = m_paletteItems[i];
		Gdiplus::SolidBrush itemBackgroundBrush(Gdiplus::Color(100, 70, 70, 70));
		pGraphics->FillRectangle(&itemBackgroundBrush,
			(Gdiplus::REAL)(item.displayRect.left - m_paletteRect.left),
			(Gdiplus::REAL)(item.displayRect.top - m_paletteRect.top),
			(Gdiplus::REAL)(item.displayRect.right - item.displayRect.left),
			(Gdiplus::REAL)(item.displayRect.bottom - item.displayRect.top));

		if (item.hBitmap && item.hBitmap->GetLastStatus() == Gdiplus::Ok) {
			pGraphics->DrawImage(item.hBitmap,
				Gdiplus::RectF((Gdiplus::REAL)(item.displayRect.left - m_paletteRect.left),
					(Gdiplus::REAL)(item.displayRect.top - m_paletteRect.top),
					(Gdiplus::REAL)(item.displayRect.right - item.displayRect.left),
					(Gdiplus::REAL)(item.displayRect.bottom - item.displayRect.top)),
				item.iconSourceRect.X, item.iconSourceRect.Y, item.iconSourceRect.Width, item.iconSourceRect.Height,
				Gdiplus::UnitPixel);
		}

		if ((int)i == m_selectedPaletteIndex) {
			Gdiplus::Pen highlightPen(Gdiplus::Color(255, 255, 255, 0), 3.0f);
			pGraphics->DrawRectangle(&highlightPen,
				(Gdiplus::REAL)(item.displayRect.left - m_paletteRect.left),
				(Gdiplus::REAL)(item.displayRect.top - m_paletteRect.top),
				(Gdiplus::REAL)(item.displayRect.right - item.displayRect.left),
				(Gdiplus::REAL)(item.displayRect.bottom - item.displayRect.top));
		}
	}
}

void EditorPalette::DrawSubPalette(Gdiplus::Graphics* pGraphics) const {
	if (!pGraphics || !m_subPalette.isOpen) return;

	Gdiplus::SolidBrush subPaletteBackgroundBrush(Gdiplus::Color(100, 50, 50, 50));
	pGraphics->FillRectangle(&subPaletteBackgroundBrush,
		(Gdiplus::REAL)m_subPalette.rect.left, (Gdiplus::REAL)m_subPalette.rect.top,
		(Gdiplus::REAL)(m_subPalette.rect.right - m_subPalette.rect.left),
		(Gdiplus::REAL)(m_subPalette.rect.bottom - m_subPalette.rect.top));

	const int subItemSize = 48;
	for (size_t i = 0; i < m_subPalette.itemRects.size(); ++i) {
		const RECT& itemRect = m_subPalette.itemRects[i];
		Gdiplus::Bitmap* itemBitmap = nullptr;
		Gdiplus::RectF itemSourceRect;
		std::wstring itemName = L"";

		if (m_subPalette.category == CATEGORY_TILE) {
			const ResourcePathUtils::TileResourceDef* tv = m_subPalette.currentTileVariantDefs[i].second;
			if (tv && !tv->imageName.empty()) {
				std::wstring fullPath = ResourcePathUtils::BuildResourcePath(tv->baseDir, tv->imageName);
				itemBitmap = BitmapUtils::LoadBitmapFromFile(fullPath.c_str());
				if (itemBitmap && itemBitmap->GetLastStatus() == Gdiplus::Ok) {
					itemSourceRect = Gdiplus::RectF(0, 0, (float)itemBitmap->GetWidth(), (float)itemBitmap->GetHeight());
					itemName = EnumUtils::GetEnumName(tv->id);
				}
			}
		}
		else if (m_subPalette.category == CATEGORY_OBJECT) {
			const ResourcePathUtils::ObjectResourceDef* ov = m_subPalette.currentObjectVariantDefs[i].second;
			if (ov && !ov->imageName.empty()) {
				std::wstring fullPath = ResourcePathUtils::BuildResourcePath(ov->baseDir, ov->imageName);
				itemBitmap = BitmapUtils::LoadBitmapFromFile(fullPath.c_str());
				if (itemBitmap && itemBitmap->GetLastStatus() == Gdiplus::Ok) {
					itemSourceRect = Gdiplus::RectF(0, 0, (float)itemBitmap->GetWidth(), (float)itemBitmap->GetHeight());
					itemName = EnumUtils::GetEnumName(ov->id);
				}
			}
		}

		if (itemBitmap && itemBitmap->GetLastStatus() == Gdiplus::Ok) {
			Gdiplus::SolidBrush itemBackgroundBrush(Gdiplus::Color(100, 70, 70, 70));
			pGraphics->FillRectangle(&itemBackgroundBrush,
				(Gdiplus::REAL)itemRect.left, (Gdiplus::REAL)itemRect.top,
				(Gdiplus::REAL)(itemRect.right - itemRect.left),
				(Gdiplus::REAL)(itemRect.bottom - itemRect.top));
			pGraphics->DrawImage(itemBitmap,
				Gdiplus::RectF((float)itemRect.left, (float)itemRect.top,
					(float)(itemRect.right - itemRect.left), (float)(itemRect.bottom - itemRect.top)),
				itemSourceRect.X, itemSourceRect.Y, itemSourceRect.Width, itemSourceRect.Height,
				Gdiplus::UnitPixel);
			Gdiplus::Font font(L"Arial", 7);
			Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 255));
			pGraphics->DrawString(itemName.c_str(), -1, &font,
				Gdiplus::PointF((float)itemRect.left, (float)itemRect.bottom - 12), &textBrush);
		}

		if ((m_subPalette.category == CATEGORY_TILE && (int)i == m_subPalette.selectedTileVariantIndex) ||
			(m_subPalette.category == CATEGORY_OBJECT && (int)i == m_subPalette.selectedObjectVariantIndex)) {
			Gdiplus::Pen highlightPen(Gdiplus::Color(255, 255, 255, 0), 3.0f);
			pGraphics->DrawRectangle(&highlightPen,
				(Gdiplus::REAL)itemRect.left, (Gdiplus::REAL)itemRect.top,
				(Gdiplus::REAL)(itemRect.right - itemRect.left),
				(Gdiplus::REAL)(itemRect.bottom - itemRect.top));
		}
	}
}

void EditorPalette::ComposePaletteLayer(Gdiplus::Bitmap* pLayerBitmap, bool* pDirty) {
	if (!pDirty || !*pDirty || !pLayerBitmap) return;
	Gdiplus::Graphics paletteLayerGraphics(pLayerBitmap);
	paletteLayerGraphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
	paletteLayerGraphics.SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
	paletteLayerGraphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
	paletteLayerGraphics.Clear(Gdiplus::Color(0, 0, 0, 0));
	DrawPalette(&paletteLayerGraphics);
	*pDirty = false;
}

EditorPalette::SubPaletteClickResult EditorPalette::HandleSubPaletteClick(POINT clickPoint) {
	if (!m_subPalette.isOpen) return SubPaletteClickResult::NotHandled;

	if (PtInRect(&m_subPalette.rect, clickPoint)) {
		for (size_t i = 0; i < m_subPalette.itemRects.size(); ++i) {
			if (PtInRect(&m_subPalette.itemRects[i], clickPoint)) {
				if (m_subPalette.category == CATEGORY_TILE) {
					m_subPalette.selectedTileVariantIndex = (int)i;
				}
				else if (m_subPalette.category == CATEGORY_OBJECT) {
					m_subPalette.selectedObjectVariantIndex = (int)i;
				}
				m_subPalette.isOpen = false;
				return SubPaletteClickResult::ClosedWithSelection;
			}
		}
		return SubPaletteClickResult::NotHandled; // in rect but not on item
	}

	m_subPalette.isOpen = false;
	m_subPalette.selectedTileVariantIndex = -1;
	m_subPalette.selectedObjectVariantIndex = -1;
	return SubPaletteClickResult::ClosedOutside;
}

bool EditorPalette::HandleMainPaletteClick(POINT clickPoint, int clientHeight) {
	if (!PtInRect(&m_paletteRect, clickPoint)) return false;

	for (size_t i = 0; i < m_paletteItems.size(); ++i) {
		if (PtInRect(&m_paletteItems[i].displayRect, clickPoint)) {
			m_selectedPaletteIndex = (int)i;

			if (m_paletteItems[i].category == CATEGORY_TILE) {
				m_subPalette.category = CATEGORY_TILE;
				m_subPalette.targetCategoryId = m_paletteItems[i].typeId;
				m_subPalette.currentTileVariantDefs.clear();
				auto type_map_it = m_pResources->GetTileVariants().find((TileType)m_paletteItems[i].typeId);
				if (type_map_it != m_pResources->GetTileVariants().end()) {
					for (auto const& pair : type_map_it->second) {
						m_subPalette.currentTileVariantDefs.push_back({ pair.first, &(pair.second) });
					}
				}
				m_subPalette.selectedTileVariantIndex = m_subPalette.currentTileVariantDefs.empty() ? -1 : 0;
			}
			else if (m_paletteItems[i].category == CATEGORY_OBJECT) {
				m_subPalette.category = CATEGORY_OBJECT;
				m_subPalette.targetCategoryId = m_paletteItems[i].typeId;
				m_subPalette.currentObjectVariantDefs.clear();
				auto type_map_it = m_pResources->GetObjectVariants().find((GameObjectType)m_paletteItems[i].typeId);
				if (type_map_it != m_pResources->GetObjectVariants().end()) {
					for (auto const& pair : type_map_it->second) {
						m_subPalette.currentObjectVariantDefs.push_back({ pair.first, &(pair.second) });
					}
				}
				m_subPalette.selectedObjectVariantIndex = m_subPalette.currentObjectVariantDefs.empty() ? -1 : 0;
			}

			const int subPaletteWidth = 170;
			const int subItemSize = 48;
			const int subPadding = 5;
			int subItemsPerRow = subPaletteWidth / (subItemSize + subPadding);
			if (subItemsPerRow == 0) subItemsPerRow = 1;

			size_t numItemsInSubPalette = (m_subPalette.category == CATEGORY_TILE)
				? m_subPalette.currentTileVariantDefs.size()
				: m_subPalette.currentObjectVariantDefs.size();
			int subPaletteHeight = (int)ceil((float)numItemsInSubPalette / subItemsPerRow) * (subItemSize + subPadding) + subPadding;
			if (subPaletteHeight > clientHeight) subPaletteHeight = clientHeight;

			m_subPalette.rect = { m_paletteRect.left - subPaletteWidth, 0, m_paletteRect.left, subPaletteHeight };
			m_subPalette.isOpen = true;
			m_subPalette.itemRects.clear();

			int currentSubX = m_subPalette.rect.left + subPadding;
			int currentSubY = m_subPalette.rect.top + subPadding;
			for (size_t j = 0; j < numItemsInSubPalette; ++j) {
				m_subPalette.itemRects.push_back({ currentSubX, currentSubY, currentSubX + subItemSize, currentSubY + subItemSize });
				currentSubX += subItemSize + subPadding;
				if ((j + 1) % subItemsPerRow == 0) {
					currentSubX = m_subPalette.rect.left + subPadding;
					currentSubY += subItemSize + subPadding;
				}
			}
			return true;
		}
	}
	return false;
}

void EditorPalette::CloseSubPalette() {
	m_subPalette.isOpen = false;
	m_subPalette.selectedTileVariantIndex = -1;
	m_subPalette.selectedObjectVariantIndex = -1;
}

void EditorPalette::ResetSelection() {
	m_selectedPaletteIndex = -1;
	CloseSubPalette();
}

const PaletteItem* EditorPalette::GetPaletteItem(size_t index) const {
	if (index >= m_paletteItems.size()) return nullptr;
	return &m_paletteItems[index];
}

const ResourcePathUtils::TileResourceDef* EditorPalette::GetSelectedTileVariant() const {
	if (m_subPalette.selectedTileVariantIndex < 0 || m_subPalette.category != CATEGORY_TILE ||
		(size_t)m_subPalette.selectedTileVariantIndex >= m_subPalette.currentTileVariantDefs.size())
		return nullptr;
	return m_subPalette.currentTileVariantDefs[m_subPalette.selectedTileVariantIndex].second;
}

const ResourcePathUtils::ObjectResourceDef* EditorPalette::GetSelectedObjectVariant() const {
	if (m_subPalette.selectedObjectVariantIndex < 0 || m_subPalette.category != CATEGORY_OBJECT ||
		(size_t)m_subPalette.selectedObjectVariantIndex >= m_subPalette.currentObjectVariantDefs.size())
		return nullptr;
	return m_subPalette.currentObjectVariantDefs[m_subPalette.selectedObjectVariantIndex].second;
}

TileID EditorPalette::GetSelectedTileID() const {
	if (m_subPalette.selectedTileVariantIndex >= 0 &&
		(size_t)m_subPalette.selectedTileVariantIndex < m_subPalette.currentTileVariantDefs.size())
		return m_subPalette.currentTileVariantDefs[m_subPalette.selectedTileVariantIndex].first;
	return TILEID_NONE;
}

GameObjectID EditorPalette::GetSelectedGameObjectID() const {
	if (m_subPalette.selectedObjectVariantIndex >= 0 &&
		(size_t)m_subPalette.selectedObjectVariantIndex < m_subPalette.currentObjectVariantDefs.size())
		return m_subPalette.currentObjectVariantDefs[m_subPalette.selectedObjectVariantIndex].first;
	return GOID_NONE;
}
