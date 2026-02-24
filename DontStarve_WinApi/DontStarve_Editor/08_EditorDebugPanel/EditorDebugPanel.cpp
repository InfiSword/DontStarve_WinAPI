#include "../pch.h"
#include "EditorDebugPanel.h"
#include "../01_EditorView/EditorView.h"
#include "../02_EditorResourceManager/EditorResourceManager.h"
#include "../04_EditorPalette/EditorPalette.h"
#include "../05_EditorPivotEditor/EditorPivotEditor.h"
#include "../06_EditorColliderEditor/EditorColliderEditor.h"
#include "../07_EditorWalkableEditor/EditorWalkableEditor.h"
#include "../09_EditorLayerComposer/EditorLayerComposer.h"
#include "../00_MainEditor/MapEditor.h"
#include "../00_MainEditor/ObjectEditor.h"

void EditorDebugPanel::SetDependencies(EditorView* pView, EditorPalette* pPalette, EditorPivotEditor* pPivotEditor,
	EditorColliderEditor* pColliderEditor, EditorWalkableEditor* pWalkableEditor, MapEditor* pMain) {
	m_pView = pView;
	m_pPalette = pPalette;
	m_pPivotEditor = pPivotEditor;
	m_pColliderEditor = pColliderEditor;
	m_pWalkableEditor = pWalkableEditor;
	m_pMain = pMain;
}

void EditorDebugPanel::SetDependencies(EditorView* pView, EditorPalette* pPalette, EditorPivotEditor* pPivotEditor,
	EditorColliderEditor* pColliderEditor, ObjectEditor* pMain) {
	m_pView = pView;
	m_pPalette = pPalette;
	m_pPivotEditor = pPivotEditor;
	m_pColliderEditor = pColliderEditor;
	m_pWalkableEditor = nullptr;
	m_pMain = nullptr;
	m_pObjectMain = pMain;
}

void EditorDebugPanel::DrawDebugInfo(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics || !m_pView) return;

	if (m_pObjectMain) {
		// ---- ObjectEditor 전용 디버그 정보 ----
		const POINT& rawMousePos = m_pObjectMain->GetRawMousePos();
		bool isPlacingMode = m_pObjectMain->IsPlacingMode();
		const ResourcePathUtils::ObjectResourceDef* selectedObjectPtr = m_pObjectMain->GetSelectedObjectPtr();
		float currentFPS = m_pObjectMain->GetCurrentFPS();
		const std::vector<ResourcePathUtils::ObjectResourceDef>& gameObjects = m_pObjectMain->GetGameObjects();

		static ULONGLONG lastUpdateTick = 0;
		static std::wstring debugInfoString;
		ULONGLONG currentTick = GetTickCount64();

		if (currentTick - lastUpdateTick > 250) {
			lastUpdateTick = currentTick;

			std::wstringstream ss;
			ss << L"FPS: " << (int)currentFPS << L"\n";
			ss << L"Zoom: " << (int)(m_pView->GetZoomFactor() * 100) << L"%\n";

			Gdiplus::PointF mouseWorldPos = m_pView->ScreenToWorld(Gdiplus::PointF((float)rawMousePos.x, (float)rawMousePos.y));
			ss << L"Mouse World: (" << (int)mouseWorldPos.X << L", " << (int)mouseWorldPos.Y << L")\n";
			ss << L"Map Offset: " << m_pView->GetMapOffset().x << L", " << m_pView->GetMapOffset().y << L"\n";
			ss << L"Objects: " << gameObjects.size() << L"\n";

			ss << L"\nMode: ";
			if (m_pColliderEditor && m_pColliderEditor->IsColliderEditMode()) ss << L"Collider\n";
			else if (m_pPivotEditor && m_pPivotEditor->IsPivotEditMode())    ss << L"Pivot\n";
			else if (isPlacingMode)                                           ss << L"Place\n";
			else                                                              ss << L"Select\n";

			if (isPlacingMode && m_pPalette) {
				int spIdx = m_pPalette->GetSelectedPaletteIndex();
				const ResourcePathUtils::ObjectResourceDef* ov = m_pPalette->GetSelectedObjectVariant();
				if (ov) {
					ss << L"Placing: " << EnumTables::GetEnumName(ov->type)
						<< L" / " << EnumTables::GetEnumName(ov->id) << L"\n";
				}
			}

			if (selectedObjectPtr) {
				ss << L"\n--- Selected Object ---\n";
				ss << L"Type: " << EnumTables::GetEnumName(selectedObjectPtr->type)
					<< L"  ID: " << EnumTables::GetEnumName(selectedObjectPtr->id) << L"\n";
				ss << L"Pos: (" << (int)selectedObjectPtr->x << L", " << (int)selectedObjectPtr->y << L")\n";
				ss << L"Pivot: (" << selectedObjectPtr->pivotX << L", " << selectedObjectPtr->pivotY << L")\n";
				if (selectedObjectPtr->hasCollider) {
					if (selectedObjectPtr->colliderType == COLLIDER_BOX) {
						ss << L"Collider: Box  off(" << selectedObjectPtr->colliderOffsetX
							<< L"," << selectedObjectPtr->colliderOffsetY << L")  "
							<< selectedObjectPtr->colliderWidth << L"x" << selectedObjectPtr->colliderHeight << L"\n";
					} else {
						ss << L"Collider: Circle  c(" << (int)selectedObjectPtr->colliderCenterX
							<< L"," << (int)selectedObjectPtr->colliderCenterY
							<< L")  r=" << (int)selectedObjectPtr->colliderRadius << L"\n";
					}
				} else {
					ss << L"Collider: None\n";
				}
			}

			ss << L"\n--- Hotkeys ---\n";
			ss << L"ESC: Cancel/Exit\n";
			ss << L"F1: Toggle Debug Panel\n";
			ss << L"V: Pivot Edit On / Off\n";
			ss << L"P: Pivot Input Dialog (0.0~1.0, in Pivot mode only)\n";
			ss << L"C: Collider Edit\n";
			ss << L"B: Box <-> Circle, I: Collider Input Dialog\n";
			ss << L"Ctrl+S: Save selected object\n";
			ss << L"Top [Save]: Save selected object\n";
			ss << L"Palette: Select other object -> Replace selected\n";
			ss << L"Right Drag: Camera, Wheel: Zoom\n";

			debugInfoString = ss.str();
		}

		// 공통 렌더링 블록
		Gdiplus::Font font(L"Malgun Gothic", 14, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush brush(Gdiplus::Color(255, 0, 0, 0));
		pGraphics->SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

		const float INFO_MAX_WIDTH = 480.0f;
		const float INFO_PADDING = 8.0f;
		RECT clientRect;
		GetClientRect(g_hWnd, &clientRect);
		float viewW = INFO_MAX_WIDTH + INFO_PADDING * 2.0f;
		float viewH = (float)(clientRect.bottom - 20);
		if (viewH > 420.0f) viewH = 420.0f;
		if (viewW > (float)(clientRect.right - 20)) viewW = (float)(clientRect.right - 20);
		Gdiplus::RectF viewportRect(10.0f, 10.0f, viewW, viewH);
		m_debugInfoViewportRect = viewportRect;

		Gdiplus::RectF layoutRect(0, 0, INFO_MAX_WIDTH, 10000.0f);
		Gdiplus::RectF boundingBox;
		pGraphics->MeasureString(debugInfoString.c_str(), -1, &font, layoutRect, nullptr, &boundingBox, nullptr, nullptr);
		float contentH = boundingBox.Height + INFO_PADDING * 2.0f;
		float contentW = boundingBox.Width + INFO_PADDING * 2.0f;
		m_debugInfoContentHeight = contentH;
		float maxScroll = (contentH - viewH) > 0.0f ? (contentH - viewH) : 0.0f;
		if (m_debugInfoScrollY > maxScroll) m_debugInfoScrollY = maxScroll;
		if (m_debugInfoScrollY < 0.0f) m_debugInfoScrollY = 0.0f;

		Gdiplus::GraphicsState state = pGraphics->Save();
		pGraphics->SetClip(Gdiplus::RectF(viewportRect.X, viewportRect.Y, viewportRect.Width, viewportRect.Height));
		pGraphics->TranslateTransform(0.0f, -m_debugInfoScrollY);
		Gdiplus::SolidBrush backBrush(Gdiplus::Color(230, 255, 255, 255));
		pGraphics->FillRectangle(&backBrush, Gdiplus::RectF(10.0f, 10.0f, (contentW > viewW) ? contentW : viewW, contentH));
		Gdiplus::RectF textRect(10.0f + INFO_PADDING, 10.0f + INFO_PADDING, INFO_MAX_WIDTH, contentH - INFO_PADDING * 2.0f);
		pGraphics->DrawString(debugInfoString.c_str(), -1, &font, textRect, nullptr, &brush);
		pGraphics->Restore(state);

		if (maxScroll > 0.0f) {
			float barW = 12.0f;
			float barX = viewportRect.X + viewportRect.Width - barW - 2.0f;
			Gdiplus::SolidBrush trackBrush(Gdiplus::Color(180, 200, 200, 200));
			pGraphics->FillRectangle(&trackBrush, Gdiplus::RectF(barX, viewportRect.Y, barW, viewportRect.Height));
			float thumbH = viewportRect.Height * (viewportRect.Height / contentH);
			if (thumbH < 24.0f) thumbH = 24.0f;
			float thumbY = viewportRect.Y + (m_debugInfoScrollY / maxScroll) * (viewportRect.Height - thumbH);
			Gdiplus::SolidBrush thumbBrush(Gdiplus::Color(220, 100, 100, 100));
			pGraphics->FillRectangle(&thumbBrush, Gdiplus::RectF(barX + 2.0f, thumbY, barW - 4.0f, thumbH));
		}
		return;
	}

	// ---- 기존 MapEditor 전용 디버그 정보 ----
	if (!m_pPalette || !m_pWalkableEditor || !m_pMain) return;

	const POINT& rawMousePos = m_pMain->GetRawMousePos();
	bool isPlacingMode = m_pMain->IsPlacingMode();
	bool isDraggingCamera = m_pMain->IsDraggingCamera();
	const ResourcePathUtils::ObjectResourceDef* selectedObjectPtr = m_pMain->GetSelectedObjectPtr();
	float currentFPS = m_pMain->GetCurrentFPS();
	Gdiplus::Bitmap* tileLayerBitmap = m_pMain->GetTileLayerBitmap();
	bool isPlayerSpawnMode = m_pMain->IsPlayerSpawnMode();
	bool hasPlayerSpawn = m_pMain->HasPlayerSpawn();
	Gdiplus::PointF playerSpawnPoint = m_pMain->GetPlayerSpawnPoint();
	const std::vector<ResourcePathUtils::ObjectResourceDef>& gameObjects = m_pMain->GetGameObjects();

	// 렉을 줄이기 위해 디버그 정보 업데이트 빈도 조절 (초당 4회)
	static ULONGLONG lastUpdateTick = 0;
	static std::wstring debugInfoString;
	ULONGLONG currentTick = GetTickCount64();

	if (currentTick - lastUpdateTick > 250) {
		lastUpdateTick = currentTick;

		std::wstringstream ss;
		ss << L"Mouse: " << rawMousePos.x << L", " << rawMousePos.y << L"\n";
		ss << L"Map Size (Limit): " << MAP_WIDTH << L"x" << MAP_HEIGHT << L" tiles (Tile: " << TILE_SIZE << L"px)\n";
		ss << L"Map Size (Loaded): " << m_pMain->GetMapWidth() << L"x" << m_pMain->GetMapHeight() << L" tiles\n";
		ss << L"World Size: " << (m_pMain->GetMapWidth() * TILE_SIZE) << L"x" << (m_pMain->GetMapHeight() * TILE_SIZE) << L"px\n";
		ss << L"Map Offset: " << m_pView->GetMapOffset().x << L", " << m_pView->GetMapOffset().y << L"\n";

		Gdiplus::PointF mouseWorldPos = m_pView->ScreenToWorld(Gdiplus::PointF((float)rawMousePos.x, (float)rawMousePos.y));
		ss << L"World Pos: (" << (int)mouseWorldPos.X << L", " << (int)mouseWorldPos.Y << L")\n";

		ss << L"\nPlacing Mode: " << (isPlacingMode ? L"ON" : L"OFF");
		int spIdx = m_pPalette->GetSelectedPaletteIndex();
		if (isPlacingMode && spIdx != -1) {
			const PaletteItem* pItem = m_pPalette->GetPaletteItem((size_t)spIdx);
			if (pItem) {
				ss << L"  (Edit Mode: " << (pItem->category == CATEGORY_TILE ? L"Tile" : L"Object") << L")";
			}
		}
		ss << L"\n";

		ss << L"Pivot Edit Mode: " << (m_pPivotEditor && m_pPivotEditor->IsPivotEditMode() ? L"ON" : L"OFF") << L"\n";
		ss << L"Collider Edit Mode: " << (m_pColliderEditor && m_pColliderEditor->IsColliderEditMode() ? L"ON" : L"OFF") << L"\n";
		ss << L"Walkable Area Edit Mode: " << (m_pWalkableEditor->IsWalkableEditMode() ? L"ON" : L"OFF") << L"\n";
		ss << L"Camera Dragging: " << (isDraggingCamera ? L"ON" : L"OFF") << L"\n";

		if (isPlacingMode) {
			ss << L"\n 선택된 타일 정보\n";
			const ResourcePathUtils::TileResourceDef* tv = m_pPalette->GetSelectedTileVariant();
			if (tv) {
				ss << L"선택된 타일 - Type: " << EnumTables::GetEnumName(tv->type)
					<< L", ID: " << EnumTables::GetEnumName(tv->id) << L"\n";
			}
			const ResourcePathUtils::ObjectResourceDef* ov = m_pPalette->GetSelectedObjectVariant();
			if (ov) {
				ss << L"선택된 오브젝트 - Type: " << EnumTables::GetEnumName(ov->type)
					<< L", ID: " << EnumTables::GetEnumName(ov->id) << L"\n";
			}
		}

		if (selectedObjectPtr) {
			ss << L"\n선택된 맵 오브젝트:" << L"\n";
			ss << L"Type: " << EnumTables::GetEnumName(selectedObjectPtr->type)
				<< L", ID: " << EnumTables::GetEnumName(selectedObjectPtr->id) << L"\n";
			ss << L"위치 - X: " << selectedObjectPtr->x << L", Y: " << selectedObjectPtr->y << L"\n";
			ss << L"Pivot: " << selectedObjectPtr->pivotX << L", " << selectedObjectPtr->pivotY << L"\n";
		}

		ss << L"\n성능 정보 확인" << L"\n";
		ss << L"FPS: " << (int)currentFPS << L"\n";
		ss << L"Layer Memory: " << (int)m_pMain->GetLayerMemoryUsageMB() << L"MB\n";

		float screenTileSize = (float)TILE_SIZE * m_pView->GetZoomFactor();
		ss << L"\nScreen Tile Size: " << (int)screenTileSize << L"px (World: " << TILE_SIZE << L"px)\n";
		ss << L"Zoom Factor: " << (int)(m_pView->GetZoomFactor() * 100) << L"%\n";
		if (tileLayerBitmap) {
			ss << L"Layer Size: " << tileLayerBitmap->GetWidth() << L"x" << tileLayerBitmap->GetHeight() << L"px\n";
		}

		ss << L"\nPlayer Spawn Mode: " << (isPlayerSpawnMode ? L"ON" : L"OFF") << L"\n";
		if (hasPlayerSpawn) {
			ss << L"Player Spawn: (" << (int)playerSpawnPoint.X << L", " << (int)playerSpawnPoint.Y << L")\n";
		}
		else {
			ss << L"Player Spawn: Not Set\n";
		}

		ss << L"Objects in Map: " << gameObjects.size() << L"\n";
		
		// 비트맵 캐시 사용량
		size_t cacheSize = m_pMain->GetBitmapCacheSize();
		ss << L"Bitmap Cache: " << cacheSize << L" images\n";

		if (m_pWalkableEditor->IsWalkableEditMode()) {
			int walkableCount = 0;
			int blockedCount = 0;
			int mapW = m_pMain->GetMapWidth(), mapH = m_pMain->GetMapHeight();
			for (int y = 0; y < mapH; ++y) {
				for (int x = 0; x < mapW; ++x) {
					if (m_pMain->GetWalkableAt(x, y)) walkableCount++;
					else blockedCount++;
				}
			}
			ss << L"Walkable Areas - Walkable: " << walkableCount
				<< L", Blocked: " << blockedCount << L" tiles\n";
		}

		ss << L"\n--- Hotkeys ---\n";
		ss << L"ESC: Cancel/Exit (sub-palette, walkable edit, player spawn mode, deselect)\n";
		ss << L"F1: Toggle Debug Info (wheel on panel to scroll)\n";
		ss << L"Shift: Toggle 3x3 tile placement (tile mode only)\n";
		ss << L"P: Player Spawn Mode (click to set spawn)\n";
		ss << L"R: Delete selected object (select object first)\n";
		ss << L"G: Walkable Area Edit (toggle, drag to paint)\n";
		ss << L"Right Click + Drag: Camera movement\n";
		ss << L"Right Click: Select object\n";
		ss << L"Ctrl+N: New Map\n";
		ss << L"Ctrl+S: Save Map\n";
		ss << L"Ctrl+O: Load Map\n";
		ss << L"Ctrl+M: Map size dialog\n";
		if (m_pPivotEditor || m_pColliderEditor) {
			ss << L"\n[Object Editor]\n";
			ss << L"Ctrl+S: Save objects (pivot/collider → GameData)\n";
			ss << L"V: Pivot Edit  C: Collider Edit  I: Collider dialog  B: Box<->Circle  A: Apply to same type\n";
		}

		debugInfoString = ss.str();
	}

	Gdiplus::Font font(L"Malgun Gothic", 14, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	Gdiplus::SolidBrush brush(Gdiplus::Color(255, 0, 0, 0));
	pGraphics->SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

	const float INFO_MAX_WIDTH = 480.0f;
	const float INFO_PADDING = 8.0f;
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	float viewW = INFO_MAX_WIDTH + INFO_PADDING * 2.0f;
	float viewH = (float)(clientRect.bottom - 20);
	if (viewH > 420.0f) viewH = 420.0f;
	if (viewW > (float)(clientRect.right - 20)) viewW = (float)(clientRect.right - 20);
	Gdiplus::RectF viewportRect(10.0f, 10.0f, viewW, viewH);
	m_debugInfoViewportRect = viewportRect;

	Gdiplus::RectF layoutRect(0, 0, INFO_MAX_WIDTH, 10000.0f);
	Gdiplus::RectF boundingBox;
	pGraphics->MeasureString(debugInfoString.c_str(), -1, &font, layoutRect, nullptr, &boundingBox, nullptr, nullptr);
	float contentW = boundingBox.Width + INFO_PADDING * 2.0f;
	float contentH = boundingBox.Height + INFO_PADDING * 2.0f;
	m_debugInfoContentHeight = contentH;
	float maxScroll = (contentH - viewH) > 0.0f ? (contentH - viewH) : 0.0f;
	if (m_debugInfoScrollY > maxScroll) m_debugInfoScrollY = maxScroll;
	if (m_debugInfoScrollY < 0.0f) m_debugInfoScrollY = 0.0f;

	Gdiplus::GraphicsState state = pGraphics->Save();
	Gdiplus::RectF clipRect(viewportRect.X, viewportRect.Y, viewportRect.Width, viewportRect.Height);
	pGraphics->SetClip(clipRect);
	pGraphics->TranslateTransform(0.0f, -m_debugInfoScrollY);
	Gdiplus::SolidBrush backBrush(Gdiplus::Color(230, 255, 255, 255));
	Gdiplus::RectF contentBackRect(10.0f, 10.0f, (contentW > viewW) ? contentW : viewW, contentH);
	pGraphics->FillRectangle(&backBrush, contentBackRect);
	Gdiplus::RectF textRect(10.0f + INFO_PADDING, 10.0f + INFO_PADDING, INFO_MAX_WIDTH, contentH - INFO_PADDING * 2.0f);
	pGraphics->DrawString(debugInfoString.c_str(), -1, &font, textRect, nullptr, &brush);
	pGraphics->Restore(state);

	if (maxScroll > 0.0f) {
		float barW = 12.0f;
		float barX = viewportRect.X + viewportRect.Width - barW - 2.0f;
		Gdiplus::RectF trackRect(barX, viewportRect.Y, barW, viewportRect.Height);
		Gdiplus::SolidBrush trackBrush(Gdiplus::Color(180, 200, 200, 200));
		pGraphics->FillRectangle(&trackBrush, trackRect);
		float thumbH = viewportRect.Height * (viewportRect.Height / contentH);
		if (thumbH < 24.0f) thumbH = 24.0f;
		float thumbY = viewportRect.Y + (m_debugInfoScrollY / maxScroll) * (viewportRect.Height - thumbH);
		Gdiplus::RectF thumbRect(barX + 2.0f, thumbY, barW - 4.0f, thumbH);
		Gdiplus::SolidBrush thumbBrush(Gdiplus::Color(220, 100, 100, 100));
		pGraphics->FillRectangle(&thumbBrush, thumbRect);
	}
}

void EditorDebugPanel::HandleMouseWheel(int zDelta, int mx, int my) {
	if (!m_showDebugInfo || m_debugInfoViewportRect.Width <= 0 || m_debugInfoViewportRect.Height <= 0) return;

	if (mx >= m_debugInfoViewportRect.X && mx < m_debugInfoViewportRect.X + m_debugInfoViewportRect.Width &&
		my >= m_debugInfoViewportRect.Y && my < m_debugInfoViewportRect.Y + m_debugInfoViewportRect.Height) {
		float maxScroll = (m_debugInfoContentHeight - m_debugInfoViewportRect.Height) > 0.0f
			? (m_debugInfoContentHeight - m_debugInfoViewportRect.Height) : 0.0f;
		m_debugInfoScrollY -= (float)zDelta * 0.4f;
		if (m_debugInfoScrollY < 0.0f) m_debugInfoScrollY = 0.0f;
		if (m_debugInfoScrollY > maxScroll) m_debugInfoScrollY = maxScroll;
	}
}

void EditorDebugPanel::ToggleVisibility() {
	m_showDebugInfo = !m_showDebugInfo;
}
