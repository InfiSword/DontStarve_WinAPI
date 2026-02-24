#include "../pch.h"
#include "ObjectEditor.h"
#include "../01_EditorView/ObjectView.h"
#include "../02_EditorResourceManager/EditorResourceManager.h"
#include "../04_EditorPalette/EditorPalette.h"
#include "../05_EditorPivotEditor/EditorPivotEditor.h"
#include "../06_EditorColliderEditor/EditorColliderEditor.h"
#include "../08_EditorDebugPanel/EditorDebugPanel.h"


ObjectEditor::ObjectEditor()
	: m_pGraphics(nullptr), m_pDoubleBufferBitmap(nullptr),
	m_pView(std::make_unique<ObjectView>()),
	m_pResources(std::make_unique<EditorResourceManager>()),
	m_pPalette(std::make_unique<EditorPalette>()),
	m_pPivotEditor(std::make_unique<EditorPivotEditor>()),
	m_pColliderEditor(std::make_unique<EditorColliderEditor>()),
	m_pDebugPanel(std::make_unique<EditorDebugPanel>())
{
}

ObjectEditor::~ObjectEditor()
{
	Release();
}

void ObjectEditor::InitPalette()
{
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	// 오브젝트 에디터에서는 타일을 팔레트에 표시하지 않음 (오브젝트만 표시)
	m_pPalette->InitPalette(clientRect.right, clientRect.bottom, m_pResources.get(), false);
}

void ObjectEditor::Initialize()
{
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);

	m_pDoubleBufferBitmap = new Gdiplus::Bitmap(clientRect.right, clientRect.bottom, PixelFormat32bppARGB);
	m_pGraphics = Gdiplus::Graphics::FromImage(m_pDoubleBufferBitmap);
	m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
	m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
	m_pGraphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);

	m_pResources->LoadResources();
	m_pPivotEditor->SetDependencies(m_pView.get(), m_pResources.get());
	m_pColliderEditor->SetDependencies(m_pView.get(), m_pResources.get(), this);
	m_pDebugPanel->SetDependencies(m_pView.get(), m_pPalette.get(), m_pPivotEditor.get(), m_pColliderEditor.get(), this);
	InitPalette();

	const RECT& pr = m_pPalette->GetPaletteRect();
	int paletteW = pr.right - pr.left;
	int paletteH = pr.bottom - pr.top;
	if (paletteW > 0 && paletteH > 0)
		m_paletteLayerBitmap = new Gdiplus::Bitmap(paletteW, paletteH, PixelFormat32bppARGB);

	m_pView->SetZoomFactor(1.0f);
	// 이동 제한 = 25% 줌 시 한 화면 크기. 초기 뷰 = 가상 맵 중앙이 윈도우 중앙에 오도록
	CenterViewOnMap(g_hWnd);
	m_paletteLayerDirty = true;
	UpdateLauncherButtonRect(clientRect.right, clientRect.bottom);
	UpdateCenterButtonRect(clientRect.right, clientRect.bottom);
	UpdateSaveButtonRect(clientRect.right, clientRect.bottom);
}

void ObjectEditor::UpdateLauncherButtonRect(int clientW, int clientH)
{
	const int btnW = 120;
	const int btnH = 32;
	const int margin = 12;
	m_rectLauncherButton = Gdiplus::RectF((Gdiplus::REAL)margin, (Gdiplus::REAL)(clientH - margin - btnH), (Gdiplus::REAL)btnW, (Gdiplus::REAL)btnH);
}

void ObjectEditor::UpdateCenterButtonRect(int clientW, int clientH)
{
	const int saveBtnW = 100;
	const int centerBtnW = 140;
	const int gap = 8;
	const int totalW = saveBtnW + gap + centerBtnW;
	const int btnH = 28;
	const int margin = 12;
	float startX = (float)(clientW - totalW) / 2.0f;
	m_rectSaveButton = Gdiplus::RectF(startX, (Gdiplus::REAL)margin, (Gdiplus::REAL)saveBtnW, (Gdiplus::REAL)btnH);
	m_rectCenterButton = Gdiplus::RectF(startX + (Gdiplus::REAL)(saveBtnW + gap), (Gdiplus::REAL)margin, (Gdiplus::REAL)centerBtnW, (Gdiplus::REAL)btnH);
}

void ObjectEditor::UpdateSaveButtonRect(int clientW, int clientH)
{
	// 저장/맵중앙 버튼 위치는 UpdateCenterButtonRect에서 함께 설정
	(void)clientW;
	(void)clientH;
}

bool ObjectEditor::IsPointInLauncherButton(POINT pt) const
{
	return m_rectLauncherButton.Contains((Gdiplus::REAL)pt.x, (Gdiplus::REAL)pt.y) != FALSE;
}

bool ObjectEditor::IsPointInCenterButton(POINT pt) const
{
	return m_rectCenterButton.Contains((Gdiplus::REAL)pt.x, (Gdiplus::REAL)pt.y) != FALSE;
}

bool ObjectEditor::IsPointInSaveButton(POINT pt) const
{
	return m_rectSaveButton.Contains((Gdiplus::REAL)pt.x, (Gdiplus::REAL)pt.y) != FALSE;
}

void ObjectEditor::CenterViewOnMap(HWND hWnd)
{
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	// 윈도우 중앙 (1000x800 → 500, 400)에 가상 맵 중앙이 오도록. 가상 맵 크기 = 25% 줌 시 한 화면(4*client) → 중앙 = 2*client
	float screenCenterX = (float)(clientRect.right) * 0.5f;
	float screenCenterY = (float)(clientRect.bottom) * 0.5f;
	float centerWorldX = (float)(clientRect.right) * 2.0f;  // 4*clientW / 2
	float centerWorldY = (float)(clientRect.bottom) * 2.0f;
	float zoom = m_pView->GetZoomFactor();
	int offsetX = (int)(screenCenterX - centerWorldX * zoom);
	int offsetY = (int)(screenCenterY - centerWorldY * zoom);
	m_pView->SetMapOffsetClamped(offsetX, offsetY, clientRect.right, clientRect.bottom, 0, 0);
	InvalidateRect(hWnd, NULL, FALSE);
}

void ObjectEditor::Update()
{
	// 저장 메시지 표시 중에는 계속 그리기 요청하여 2.5초 후 자연스럽게 사라지게 함
	if (m_savedMessageShowUntil != 0 && GetTickCount64() < m_savedMessageShowUntil)
		InvalidateRect(g_hWnd, NULL, FALSE);
}

void ObjectEditor::DrawObjects(Gdiplus::Graphics* pGraphics)
{
	if (!pGraphics || !m_pView || !m_pResources) return;
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);

	for (const auto& obj : m_gameObjects) {
		const ResourcePathUtils::ObjectResourceDef* ov = m_pResources->GetObjectVariant(obj.type, obj.id);
		if (!ov || ov->imageName.empty()) continue;
		std::wstring fullPath = ov->baseDir;
		if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') fullPath += L"\\";
		fullPath += ov->imageName;
		std::shared_ptr<Gdiplus::Bitmap> pBitmap = m_pResources->GetCachedBitmap(fullPath);
		if (!pBitmap) continue;
		float w = (float)pBitmap->GetWidth();
		float h = (float)pBitmap->GetHeight();
		float left = (float)obj.x - (ov->pivotX * w);
		float top = (float)obj.y - (ov->pivotY * h);
		Gdiplus::PointF screenTopLeft = WorldToScreen(Gdiplus::PointF(left, top));
		float screenW = w * m_pView->GetZoomFactor();
		float screenH = h * m_pView->GetZoomFactor();
		Gdiplus::RectF destRect(screenTopLeft.X, screenTopLeft.Y, screenW, screenH);
		pGraphics->DrawImage(pBitmap.get(), destRect, 0, 0, w, h, Gdiplus::UnitPixel);
		if (m_selectedObjectPtr == &obj) {
			Gdiplus::Pen pen(Gdiplus::Color(255, 255, 0, 0), 3.0f);
			pGraphics->DrawRectangle(&pen, destRect);
		}
	}
}

void ObjectEditor::DrawPreview(Gdiplus::Graphics* pGraphics)
{
	if (!pGraphics || !m_isPlacingMode) return;
	int selIdx = m_pPalette->GetSelectedPaletteIndex();
	const PaletteItem* pItem = (selIdx >= 0) ? m_pPalette->GetPaletteItem((size_t)selIdx) : nullptr;
	if (!pItem || pItem->category != CATEGORY_OBJECT) return;
	const ResourcePathUtils::ObjectResourceDef* ov = m_pPalette->GetSelectedObjectVariant();
	if (!ov || ov->imageName.empty()) return;
	std::wstring fullPath = ov->baseDir;
	if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') fullPath += L"\\";
	fullPath += ov->imageName;
	std::shared_ptr<Gdiplus::Bitmap> pBitmap = m_pResources->GetCachedBitmap(fullPath);
	if (!pBitmap) return;
	Gdiplus::PointF screenPos = WorldToScreen(m_snappedPreviewPos);
	float screenW = (float)pBitmap->GetWidth() * m_pView->GetZoomFactor();
	float screenH = (float)pBitmap->GetHeight() * m_pView->GetZoomFactor();
	float x = screenPos.X - (ov->pivotX * screenW);
	float y = screenPos.Y - (ov->pivotY * screenH);
	Gdiplus::ColorMatrix cm = { 1,0,0,0,0, 0,1,0,0,0, 0,0,1,0,0, 0,0,0,0.6f,0, 0,0,0,0,1 };
	Gdiplus::ImageAttributes ia;
	ia.SetColorMatrix(&cm);
	pGraphics->DrawImage(pBitmap.get(), Gdiplus::RectF(x, y, screenW, screenH), 0, 0, (float)pBitmap->GetWidth(), (float)pBitmap->GetHeight(), Gdiplus::UnitPixel, &ia);
}

Gdiplus::PointF ObjectEditor::ScreenToWorld(Gdiplus::PointF screenPos) const
{
	return m_pView->ScreenToWorld(screenPos);
}

Gdiplus::PointF ObjectEditor::WorldToScreen(Gdiplus::PointF worldPos) const
{
	return m_pView->WorldToScreen(worldPos);
}

void ObjectEditor::AddObject(const ResourcePathUtils::ObjectResourceDef& obj)
{
	m_gameObjects.push_back(obj);
	m_objectsDirty = true;
}

void ObjectEditor::RemoveObject(ResourcePathUtils::ObjectResourceDef* objToRemove)
{
	auto it = std::remove_if(m_gameObjects.begin(), m_gameObjects.end(),
		[objToRemove](const ResourcePathUtils::ObjectResourceDef& o) { return &o == objToRemove; });
	if (it != m_gameObjects.end()) {
		m_gameObjects.erase(it, m_gameObjects.end());
		m_objectsDirty = true;
	}
}

void ObjectEditor::DeselectObject(HWND hWnd)
{
	if (m_selectedObjectPtr) {
		m_pColliderEditor->EndColliderEdit();
		m_selectedObjectPtr = nullptr;
		m_objectsDirty = true;
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

void ObjectEditor::ExitAllEditModes()
{
	m_pPivotEditor->EndPivotEdit();
	m_pColliderEditor->EndColliderEdit();
	m_isPlacingMode = false;
	if (m_pPalette->IsSubPaletteOpen()) {
		m_pPalette->CloseSubPalette();
		m_paletteLayerDirty = true;
	}
}

void ObjectEditor::SpawnSelectedPaletteObjectAtViewCenter(HWND hWnd)
{
	const PaletteItem* pItem = (m_pPalette->GetSelectedPaletteIndex() >= 0) ? m_pPalette->GetPaletteItem((size_t)m_pPalette->GetSelectedPaletteIndex()) : nullptr;
	if (!pItem || pItem->category != CATEGORY_OBJECT) return;
	const ResourcePathUtils::ObjectResourceDef* ov = m_pPalette->GetSelectedObjectVariant();
	if (!ov) return;

	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	int centerX = (clientRect.left + clientRect.right) / 2;
	int centerY = (clientRect.top + clientRect.bottom) / 2;
	Gdiplus::PointF worldCenter = ScreenToWorld(Gdiplus::PointF((float)centerX, (float)centerY));

	GameObjectID selectedObjectID = m_pPalette->GetSelectedGameObjectID();
	float pivotX = ov->pivotX, pivotY = ov->pivotY;
	bool hasCollider = ov->hasCollider;
	ColliderType colliderType = ov->colliderType;
	int cox = ov->colliderOffsetX, coy = ov->colliderOffsetY, cw = ov->colliderWidth, ch = ov->colliderHeight;
	float ccx = ov->colliderCenterX, ccy = ov->colliderCenterY, cr = ov->colliderRadius;
	bool needFallback = !hasCollider || (colliderType == COLLIDER_BOX && cw <= 0 && ch <= 0) || (colliderType == COLLIDER_CIRCLE && cr <= 0.0f);
	if (needFallback) {
		const ResourcePathUtils::ObjectResourceDef* same = nullptr;
		for (const auto& o : m_gameObjects) {
			if (o.type == (GameObjectType)pItem->typeId && o.id == selectedObjectID) { same = &o; break; }
		}
		if (same) {
			pivotX = same->pivotX; pivotY = same->pivotY; hasCollider = same->hasCollider; colliderType = same->colliderType;
			cox = same->colliderOffsetX; coy = same->colliderOffsetY; cw = same->colliderWidth; ch = same->colliderHeight;
			ccx = same->colliderCenterX; ccy = same->colliderCenterY; cr = same->colliderRadius;
		} else {
			int iw = 32, ih = 32;
			if (!ov->imageName.empty()) {
				std::wstring fp = ov->baseDir;
				if (!fp.empty() && fp.back() != L'\\' && fp.back() != L'/') fp += L"\\";
				fp += ov->imageName;
				auto b = m_pResources->GetCachedBitmap(fp);
				if (b) { iw = b->GetWidth(); ih = b->GetHeight(); }
			}
			cox = -(int)(ov->pivotX * iw); coy = -(int)(ov->pivotY * ih); cw = iw; ch = ih;
			ccx = iw * (0.5f - ov->pivotX); ccy = ih * (0.5f - ov->pivotY);
			cr = (float)((iw < ih) ? iw : ih) * 0.5f;
		}
	}
	ResourcePathUtils::ObjectResourceDef newObj((GameObjectType)pItem->typeId, selectedObjectID,
		worldCenter.X, worldCenter.Y, ov->baseDir, ov->imageName, pivotX, pivotY,
		hasCollider, colliderType, cox, coy, cw, ch, ccx, ccy, cr);
	AddObject(newObj);
	m_selectedObjectPtr = &m_gameObjects.back();
	m_objectsDirty = true;
	InvalidateRect(hWnd, NULL, FALSE);
}

void ObjectEditor::ReplaceSelectedObjectWithPaletteSelection(HWND hWnd)
{
	if (!m_selectedObjectPtr) return;
	const PaletteItem* pItem = (m_pPalette->GetSelectedPaletteIndex() >= 0) ? m_pPalette->GetPaletteItem((size_t)m_pPalette->GetSelectedPaletteIndex()) : nullptr;
	if (!pItem || pItem->category != CATEGORY_OBJECT) return;
	const ResourcePathUtils::ObjectResourceDef* ov = m_pPalette->GetSelectedObjectVariant();
	if (!ov) return;

	float oldX = m_selectedObjectPtr->x;
	float oldY = m_selectedObjectPtr->y;

	if (m_pPivotEditor->GetEditingObject() == m_selectedObjectPtr) m_pPivotEditor->EndPivotEdit();
	if (m_pColliderEditor->GetEditingColliderObject() == m_selectedObjectPtr) m_pColliderEditor->EndColliderEdit();

	GameObjectID selectedObjectID = m_pPalette->GetSelectedGameObjectID();
	float pivotX = ov->pivotX, pivotY = ov->pivotY;
	bool hasCollider = ov->hasCollider;
	ColliderType colliderType = ov->colliderType;
	int cox = ov->colliderOffsetX, coy = ov->colliderOffsetY, cw = ov->colliderWidth, ch = ov->colliderHeight;
	float ccx = ov->colliderCenterX, ccy = ov->colliderCenterY, cr = ov->colliderRadius;
	bool needFallback = !hasCollider || (colliderType == COLLIDER_BOX && cw <= 0 && ch <= 0) || (colliderType == COLLIDER_CIRCLE && cr <= 0.0f);
	if (needFallback) {
		const ResourcePathUtils::ObjectResourceDef* same = nullptr;
		for (const auto& o : m_gameObjects) {
			if (o.type == (GameObjectType)pItem->typeId && o.id == selectedObjectID) { same = &o; break; }
		}
		if (same) {
			pivotX = same->pivotX; pivotY = same->pivotY; hasCollider = same->hasCollider; colliderType = same->colliderType;
			cox = same->colliderOffsetX; coy = same->colliderOffsetY; cw = same->colliderWidth; ch = same->colliderHeight;
			ccx = same->colliderCenterX; ccy = same->colliderCenterY; cr = same->colliderRadius;
		} else {
			int iw = 32, ih = 32;
			if (!ov->imageName.empty()) {
				std::wstring fp = ov->baseDir;
				if (!fp.empty() && fp.back() != L'\\' && fp.back() != L'/') fp += L"\\";
				fp += ov->imageName;
				auto b = m_pResources->GetCachedBitmap(fp);
				if (b) { iw = b->GetWidth(); ih = b->GetHeight(); }
			}
			cox = -(int)(ov->pivotX * iw); coy = -(int)(ov->pivotY * ih); cw = iw; ch = ih;
			ccx = iw * (0.5f - ov->pivotX); ccy = ih * (0.5f - ov->pivotY);
			cr = (float)((iw < ih) ? iw : ih) * 0.5f;
		}
	}
	ResourcePathUtils::ObjectResourceDef newObj((GameObjectType)pItem->typeId, selectedObjectID,
		oldX, oldY, ov->baseDir, ov->imageName, pivotX, pivotY,
		hasCollider, colliderType, cox, coy, cw, ch, ccx, ccy, cr);

	size_t idx = 0;
	for (; idx < m_gameObjects.size(); ++idx) {
		if (&m_gameObjects[idx] == m_selectedObjectPtr) break;
	}
	if (idx < m_gameObjects.size()) {
		m_gameObjects[idx] = std::move(newObj);
		m_selectedObjectPtr = &m_gameObjects[idx];
		m_objectsDirty = true;
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

void ObjectEditor::HandlePlacingModeClick(POINT clickPoint, HWND hWnd)
{
	const PaletteItem* pItem = (m_pPalette->GetSelectedPaletteIndex() >= 0) ? m_pPalette->GetPaletteItem((size_t)m_pPalette->GetSelectedPaletteIndex()) : nullptr;
	if (!pItem || pItem->category != CATEGORY_OBJECT) return;
	const ResourcePathUtils::ObjectResourceDef* ov = m_pPalette->GetSelectedObjectVariant();
	if (!ov) return;
	Gdiplus::PointF mouseWorld = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));
	GameObjectID selectedObjectID = m_pPalette->GetSelectedGameObjectID();
	float pivotX = ov->pivotX, pivotY = ov->pivotY;
	bool hasCollider = ov->hasCollider;
	ColliderType colliderType = ov->colliderType;
	int cox = ov->colliderOffsetX, coy = ov->colliderOffsetY, cw = ov->colliderWidth, ch = ov->colliderHeight;
	float ccx = ov->colliderCenterX, ccy = ov->colliderCenterY, cr = ov->colliderRadius;
	bool needFallback = !hasCollider || (colliderType == COLLIDER_BOX && cw <= 0 && ch <= 0) || (colliderType == COLLIDER_CIRCLE && cr <= 0.0f);
	if (needFallback) {
		const ResourcePathUtils::ObjectResourceDef* same = nullptr;
		for (const auto& o : m_gameObjects) {
			if (o.type == (GameObjectType)pItem->typeId && o.id == selectedObjectID) { same = &o; break; }
		}
		if (same) {
			pivotX = same->pivotX; pivotY = same->pivotY; hasCollider = same->hasCollider; colliderType = same->colliderType;
			cox = same->colliderOffsetX; coy = same->colliderOffsetY; cw = same->colliderWidth; ch = same->colliderHeight;
			ccx = same->colliderCenterX; ccy = same->colliderCenterY; cr = same->colliderRadius;
		} else {
			int iw = 32, ih = 32;
			if (!ov->imageName.empty()) {
				std::wstring fp = ov->baseDir;
				if (!fp.empty() && fp.back() != L'\\' && fp.back() != L'/') fp += L"\\";
				fp += ov->imageName;
				auto b = m_pResources->GetCachedBitmap(fp);
				if (b) { iw = b->GetWidth(); ih = b->GetHeight(); }
			}
			cox = -(int)(ov->pivotX * iw); coy = -(int)(ov->pivotY * ih); cw = iw; ch = ih;
			ccx = iw * (0.5f - ov->pivotX); ccy = ih * (0.5f - ov->pivotY);
			cr = (float)((iw < ih) ? iw : ih) * 0.5f;
		}
	}
	ResourcePathUtils::ObjectResourceDef newObj((GameObjectType)pItem->typeId, selectedObjectID,
		mouseWorld.X, mouseWorld.Y, ov->baseDir, ov->imageName, pivotX, pivotY,
		hasCollider, colliderType, cox, coy, cw, ch, ccx, ccy, cr);
	AddObject(newObj);
	InvalidateRect(hWnd, NULL, FALSE);
}

void ObjectEditor::HandleObjectSelectionClick(POINT clickPoint, HWND hWnd)
{
	Gdiplus::PointF worldClick = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));
	for (int i = (int)m_gameObjects.size() - 1; i >= 0; --i) {
		ResourcePathUtils::ObjectResourceDef& obj = m_gameObjects[i];
		const ResourcePathUtils::ObjectResourceDef* ov = m_pResources->GetObjectVariant(obj.type, obj.id);
		if (!ov || ov->imageName.empty()) continue;
		std::wstring fullPath = ov->baseDir;
		if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') fullPath += L"\\";
		fullPath += ov->imageName;
		std::shared_ptr<Gdiplus::Bitmap> pBitmap = m_pResources->GetCachedBitmap(fullPath);
		if (!pBitmap) continue;
		float w = (float)pBitmap->GetWidth();
		float h = (float)pBitmap->GetHeight();
		float left = (float)obj.x - (ov->pivotX * w);
		float top = (float)obj.y - (ov->pivotY * h);
		Gdiplus::RectF objWorldRect(left, top, w, h);
		// 이미지 크기 바운딩 박스로 클릭 시 선택 (클릭 시 빨간 테두리 = 이미지 테두리)
		if (!objWorldRect.Contains(worldClick.X, worldClick.Y)) continue;
		m_selectedObjectPtr = &obj;
		m_objectsDirty = true;
		InvalidateRect(hWnd, NULL, FALSE);
		return;
	}
	// 빈 영역 클릭 시 선택 해제하지 않음 (팔레트에서 다른 오브젝트 선택 시 교체하려면 선택 유지)
}

float ObjectEditor::GetLayerMemoryUsageMB() const
{
	return -1.0f;
}

bool ObjectEditor::SaveObjects()
{
	if (!m_pResources) return false;
	if (!m_selectedObjectPtr) {
		MessageBox(g_hWnd, L"선택된 오브젝트가 없습니다.", L"저장", MB_OK | MB_ICONINFORMATION);
		return false;
	}
	// 피벗/콜라이더 편집 내용은 m_selectedObjectPtr(인스턴스)에 있음. 이 값을 variant에 반영하고 파일로 저장
	bool ok = m_pResources->SaveObjectResourceOverride(m_selectedObjectPtr->type, m_selectedObjectPtr->id, *m_selectedObjectPtr);
	if (ok)
		m_savedMessageShowUntil = GetTickCount64() + 2500;  // 2.5초 동안 "저장했습니다" 표시
	return ok;
}

void ObjectEditor::Render()
{
	if (!m_pGraphics) return;
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	m_pGraphics->Clear(Gdiplus::Color(255, 255, 255, 255));
	m_pPalette->ComposePaletteLayer(m_paletteLayerBitmap, &m_paletteLayerDirty);
	DrawObjects(m_pGraphics);
	if (m_paletteLayerBitmap) {
		const RECT& pr = m_pPalette->GetPaletteRect();
		m_pGraphics->DrawImage(m_paletteLayerBitmap,
			(Gdiplus::REAL)pr.left, (Gdiplus::REAL)pr.top,
			(Gdiplus::REAL)(pr.right - pr.left), (Gdiplus::REAL)(pr.bottom - pr.top));
	}
	DrawPreview(m_pGraphics);
	m_pPalette->DrawSubPalette(m_pGraphics);
	m_pPivotEditor->DrawPivotEditor(m_pGraphics);
	m_pColliderEditor->DrawColliders(m_pGraphics);
	if (m_pDebugPanel->IsVisible()) {
		m_pDebugPanel->DrawDebugInfo(m_pGraphics);
	}
	// 상단 저장하기 / 맵 중앙 버튼
	{
		Gdiplus::SolidBrush btnBrush(Gdiplus::Color(255, 70, 130, 180));
		Gdiplus::Pen btnPen(Gdiplus::Color(255, 50, 80, 120), 2.0f);
		Gdiplus::Font font(L"Malgun Gothic", 11, Gdiplus::FontStyleBold);
		Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 255));
		Gdiplus::StringFormat sf;
		sf.SetAlignment(Gdiplus::StringAlignmentCenter);
		sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		m_pGraphics->FillRectangle(&btnBrush, m_rectSaveButton);
		m_pGraphics->DrawRectangle(&btnPen, m_rectSaveButton);
		Gdiplus::RectF textRectSave(m_rectSaveButton.X, m_rectSaveButton.Y, m_rectSaveButton.Width, m_rectSaveButton.Height);
		m_pGraphics->DrawString(L"저장하기", -1, &font, textRectSave, &sf, &textBrush);
		m_pGraphics->FillRectangle(&btnBrush, m_rectCenterButton);
		m_pGraphics->DrawRectangle(&btnPen, m_rectCenterButton);
		Gdiplus::RectF textRectCenter(m_rectCenterButton.X, m_rectCenterButton.Y, m_rectCenterButton.Width, m_rectCenterButton.Height);
		m_pGraphics->DrawString(L"맵 중앙", -1, &font, textRectCenter, &sf, &textBrush);
	}
	// 좌측 하단 Launcher 버튼
	{
		Gdiplus::SolidBrush btnBrush(Gdiplus::Color(255, 70, 130, 180));
		Gdiplus::Pen btnPen(Gdiplus::Color(255, 50, 80, 120), 2.0f);
		m_pGraphics->FillRectangle(&btnBrush, m_rectLauncherButton);
		m_pGraphics->DrawRectangle(&btnPen, m_rectLauncherButton);
		Gdiplus::Font font(L"Malgun Gothic", 12, Gdiplus::FontStyleBold);
		Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 255));
		Gdiplus::StringFormat sf;
		sf.SetAlignment(Gdiplus::StringAlignmentCenter);
		sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		Gdiplus::RectF textRect(m_rectLauncherButton.X, m_rectLauncherButton.Y, m_rectLauncherButton.Width, m_rectLauncherButton.Height);
		m_pGraphics->DrawString(L"Launcher", -1, &font, textRect, &sf, &textBrush);
	}
	// 저장 후 일정 시간 동안 "저장했습니다" 문구 표시
	if (GetTickCount64() < m_savedMessageShowUntil) {
		const WCHAR* msg = L"저장했습니다";
		Gdiplus::Font msgFont(L"Malgun Gothic", 18, Gdiplus::FontStyleBold);
		Gdiplus::RectF layoutRect(0, 0, (float)clientRect.right, (float)clientRect.bottom);
		Gdiplus::RectF bound;
		m_pGraphics->MeasureString(msg, -1, &msgFont, layoutRect, nullptr, &bound, nullptr, nullptr);
		float pad = 24.0f;
		Gdiplus::RectF backRect(
			((float)clientRect.right - bound.Width) / 2.0f - pad,
			((float)clientRect.bottom - bound.Height) / 2.0f - pad,
			bound.Width + pad * 2.0f,
			bound.Height + pad * 2.0f);
		Gdiplus::SolidBrush backBrush(Gdiplus::Color(220, 50, 50, 50));
		Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 255));
		Gdiplus::StringFormat sf;
		sf.SetAlignment(Gdiplus::StringAlignmentCenter);
		sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		m_pGraphics->FillRectangle(&backBrush, backRect);
		m_pGraphics->DrawString(msg, -1, &msgFont, backRect, &sf, &textBrush);
	}
	HDC hdcScreen = GetDC(g_hWnd);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hBitmap = NULL;
	Gdiplus::Color color(0, 0, 0, 0);
	if (m_pDoubleBufferBitmap->GetHBITMAP(color, &hBitmap) == Gdiplus::Ok && hBitmap) {
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
		BitBlt(hdcScreen, 0, 0, clientRect.right, clientRect.bottom, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, hOldBitmap);
		DeleteObject(hBitmap);
	}
	DeleteDC(hdcMem);
	ReleaseDC(g_hWnd, hdcScreen);
}

void ObjectEditor::Release()
{
	m_pResources->ReleaseResources();
	Utils::SafeDelete(m_pGraphics);
	Utils::SafeDelete(m_pDoubleBufferBitmap);
	Utils::SafeDelete(m_paletteLayerBitmap);
}

EditorScreenSwitch ObjectEditor::GetRequestedSwitch()
{
	EditorScreenSwitch s = m_requestedSwitch;
	m_requestedSwitch = EditorScreenSwitch::None;
	return s;
}

LRESULT ObjectEditor::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_SIZE: {
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		Utils::SafeDelete(m_pDoubleBufferBitmap);
		Utils::SafeDelete(m_pGraphics);
		m_pDoubleBufferBitmap = new Gdiplus::Bitmap(clientRect.right, clientRect.bottom, PixelFormat32bppARGB);
		m_pGraphics = Gdiplus::Graphics::FromImage(m_pDoubleBufferBitmap);
		m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
		m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
		m_pPalette->InitPalette(clientRect.right, clientRect.bottom, m_pResources.get(), false);
		Utils::SafeDelete(m_paletteLayerBitmap);
		const RECT& pr = m_pPalette->GetPaletteRect();
		m_paletteLayerBitmap = new Gdiplus::Bitmap(pr.right - pr.left, pr.bottom - pr.top, PixelFormat32bppARGB);
		m_paletteLayerDirty = true;
		UpdateLauncherButtonRect(clientRect.right, clientRect.bottom);
		UpdateCenterButtonRect(clientRect.right, clientRect.bottom);
		UpdateSaveButtonRect(clientRect.right, clientRect.bottom);
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	}
	case WM_MOUSEMOVE: {
		POINT pt = { LOWORD(lParam), HIWORD(lParam) };
		m_rawMousePos = pt;
		if (m_isDraggingCamera) {
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			int dx = m_rawMousePos.x - m_cameraDragStart.x;
			int dy = m_rawMousePos.y - m_cameraDragStart.y;
			m_pView->SetMapOffsetClamped(m_initialMapOffset.x + dx, m_initialMapOffset.y + dy,
				clientRect.right, clientRect.bottom, OBJECT_EDITOR_MAP_W, OBJECT_EDITOR_MAP_H);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		if (m_pColliderEditor->IsDraggingCollider()) {
			m_pColliderEditor->OnMouseMove(m_rawMousePos, hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		if (m_pPivotEditor->IsPivotEditMode() && (GetKeyState(VK_LBUTTON) & 0x8000)) {
			m_pPivotEditor->UpdatePivotEdit(m_rawMousePos);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		int selIdx = m_pPalette->GetSelectedPaletteIndex();
		if (selIdx != -1 && m_isPlacingMode) {
			const PaletteItem* pItem = m_pPalette->GetPaletteItem((size_t)selIdx);
			if (pItem && pItem->category == CATEGORY_OBJECT) {
				Gdiplus::PointF world = ScreenToWorld(Gdiplus::PointF((float)pt.x, (float)pt.y));
				m_snappedPreviewPos = world;
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
	}
	break;
	case WM_LBUTTONDOWN: {
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		POINT clickPoint = { LOWORD(lParam), HIWORD(lParam) };
		// 상단 저장하기 버튼 클릭 시 선택 오브젝트 수정사항 저장
		if (IsPointInSaveButton(clickPoint)) {
			if (SaveObjects()) InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		// 가운데 상단 맵 중앙 버튼 클릭 시 뷰를 맵 중앙으로
		if (IsPointInCenterButton(clickPoint)) {
			CenterViewOnMap(hWnd);
			return 0;
		}
		// 좌측 하단 Launcher 버튼 클릭 시 런처로 복귀
		if (IsPointInLauncherButton(clickPoint)) {
			m_requestedSwitch = EditorScreenSwitch::BackToLauncher;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		const RECT& paletteRect = m_pPalette->GetPaletteRect();
		bool onPalette = PtInRect(&paletteRect, clickPoint) != FALSE;
		if (m_isPlacingMode && !onPalette) {
			HandlePlacingModeClick(clickPoint, hWnd);
			return 0;
		}
		if (m_pColliderEditor->IsColliderEditMode()) {
			m_pColliderEditor->OnLeftButtonDown(clickPoint, hWnd);
			if (m_pColliderEditor->IsDraggingCollider()) { InvalidateRect(hWnd, NULL, FALSE); return 0; }
		}
		if (m_pPivotEditor->IsPivotEditMode()) {
			m_pPivotEditor->UpdatePivotEdit(clickPoint);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		auto subResult = m_pPalette->HandleSubPaletteClick(clickPoint);
		if (subResult != EditorPalette::SubPaletteClickResult::NotHandled) {
			if (subResult == EditorPalette::SubPaletteClickResult::ClosedWithSelection) {
				// 선택된 오브젝트가 있으면 해당 오브젝트를 팔레트 선택으로 교체, 없으면 뷰 중앙에 스폰
				if (m_selectedObjectPtr)
					ReplaceSelectedObjectWithPaletteSelection(hWnd);
				else
					SpawnSelectedPaletteObjectAtViewCenter(hWnd);
				m_isPlacingMode = false;
			} else {
				bool hasSel = (m_pPalette->GetSelectedPaletteIndex() >= 0 && m_pPalette->GetSelectedObjectVariant() != nullptr);
				m_isPlacingMode = false;
				if (!hasSel) m_pColliderEditor->EndColliderEdit();
				// 선택 해제하지 않음 (m_selectedObjectPtr 유지)
			}
			m_paletteLayerDirty = true;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		if (onPalette) {
			if (m_pPalette->HandleMainPaletteClick(clickPoint, clientRect.bottom)) {
				m_isPlacingMode = false;
				m_paletteLayerDirty = true;
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
		}
		// 맵 영역 클릭: 오브젝트 선택만 (배치 모드 없음, 스폰은 팔레트 선택 시 뷰 중앙에만 수행)
		HandleObjectSelectionClick(clickPoint, hWnd);
	}
	break;
	case WM_RBUTTONDOWN: {
		POINT clickPoint = { LOWORD(lParam), HIWORD(lParam) };
		if (m_pPalette->IsSubPaletteOpen()) {
			m_pPalette->CloseSubPalette();
			m_isPlacingMode = false;
			m_paletteLayerDirty = true;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		if (m_isPlacingMode) {
			m_isPlacingMode = false;
			m_pPalette->ResetSelection();
			m_paletteLayerDirty = true;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		// 디버그 패널 위에서는 카메라 드래그 시작 안 함
		if (m_pDebugPanel->IsVisible()) {
			Gdiplus::RectF vr = m_pDebugPanel->GetViewportRect();
			if (clickPoint.x >= (LONG)vr.X && clickPoint.x < (LONG)(vr.X + vr.Width) &&
				clickPoint.y >= (LONG)vr.Y && clickPoint.y < (LONG)(vr.Y + vr.Height)) {
				return 0;
			}
		}
		const RECT& pr = m_pPalette->GetPaletteRect();
		if (!PtInRect(&pr, clickPoint)) {
			m_isDraggingCamera = true;
			m_cameraDragStart = clickPoint;
			m_initialMapOffset = m_pView->GetMapOffset();
			SetCapture(hWnd);
			return 0;
		}
	}
	break;
	case WM_CAPTURECHANGED:
		if (m_isDraggingCamera) m_isDraggingCamera = false;
		if (m_pColliderEditor->IsDraggingCollider()) m_pColliderEditor->OnLeftButtonUp();
		break;
	case WM_RBUTTONUP:
		if (m_isDraggingCamera) {
			m_isDraggingCamera = false;
			ReleaseCapture();
			int dx = m_rawMousePos.x - m_cameraDragStart.x;
			int dy = m_rawMousePos.y - m_cameraDragStart.y;
			if (dx * dx + dy * dy <= 25) {
				POINT cp = { m_rawMousePos.x, m_rawMousePos.y };
				HandleObjectSelectionClick(cp, hWnd);
			}
			return 0;
		}
		break;
	case WM_MOUSEWHEEL: {
		short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		// WM_MOUSEWHEEL의 lParam은 화면 좌표이므로 클라이언트 좌표로 변환 (MapEditor와 동일)
		POINT ptScreen = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		POINT ptClient = ptScreen;
		ScreenToClient(hWnd, &ptClient);

		if (m_pDebugPanel->IsVisible()) {
			Gdiplus::RectF vr = m_pDebugPanel->GetViewportRect();
			if ((float)ptClient.x >= vr.X && (float)ptClient.x < vr.X + vr.Width &&
				(float)ptClient.y >= vr.Y && (float)ptClient.y < vr.Y + vr.Height) {
				m_pDebugPanel->HandleMouseWheel(zDelta, ptClient.x, ptClient.y);
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
		}

		// 윈도우 중앙을 기준으로 줌: 줌 후에도 윈도우 중앙에 있던 월드 점이 그대로 윈도우 중앙에 고정되도록 오프셋 보정
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		float centerX = (float)(clientRect.right) * 0.5f;
		float centerY = (float)(clientRect.bottom) * 0.5f;
		Gdiplus::PointF centerWorldBefore = ScreenToWorld(Gdiplus::PointF(centerX, centerY));

		float oldZoom = m_pView->GetZoomFactor();
		if (zDelta > 0) m_pView->ZoomIn();
		else m_pView->ZoomOut();
		float newZoom = m_pView->GetZoomFactor();

		if (newZoom != oldZoom) {
			int newOffsetX = (int)(centerX - centerWorldBefore.X * newZoom);
			int newOffsetY = (int)(centerY - centerWorldBefore.Y * newZoom);
			m_pView->SetMapOffsetClamped(newOffsetX, newOffsetY, clientRect.right, clientRect.bottom, OBJECT_EDITOR_MAP_W, OBJECT_EDITOR_MAP_H);
		}
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	}
	case WM_KEYDOWN: {
		// Ctrl+S: 선택 오브젝트 수정사항 저장
		if ((GetKeyState(VK_CONTROL) & 0x8000) && (wParam == 'S' || wParam == 's')) {
			if (SaveObjects()) InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		if (wParam == VK_F1) {
			m_pDebugPanel->ToggleVisibility();
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		if (wParam == VK_ESCAPE) {
			bool redraw = false;
			if (m_pPalette->IsSubPaletteOpen()) {
				m_pPalette->CloseSubPalette();
				m_isPlacingMode = false;
				m_paletteLayerDirty = true;
				redraw = true;
			} else if (m_pColliderEditor->IsColliderEditMode()) {
				m_pColliderEditor->EndColliderEdit();
				redraw = true;
			} else if (m_pPivotEditor->IsPivotEditMode()) {
				m_pPivotEditor->EndPivotEdit();
				redraw = true;
			} else if (m_selectedObjectPtr) {
				// 선택 해제 없음 (팔레트 교체를 위해 유지)
				return 0;
			} else {
				m_requestedSwitch = EditorScreenSwitch::BackToLauncher;
			}
			if (redraw) InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		if (wParam == 'V') {
			if (m_pPivotEditor->IsPivotEditMode()) {
				m_pPivotEditor->EndPivotEdit();
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
			if (!m_selectedObjectPtr) {
				MessageBoxW(hWnd, L"오브젝트를 먼저 선택해주세요.", L"피벗 편집", MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			ExitAllEditModes();
			m_pPivotEditor->StartPivotEdit(m_selectedObjectPtr);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		if (wParam == 'P' && m_pPivotEditor->IsPivotEditMode()) {
			m_pPivotEditor->ShowPivotDialog(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		if (wParam == 'C') {
			if (!m_selectedObjectPtr) {
				MessageBoxW(hWnd, L"오브젝트를 먼저 선택해주세요.", L"콜라이더 편집", MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			if (!m_pColliderEditor->IsColliderEditMode()) {
				ExitAllEditModes();
				m_pColliderEditor->StartColliderEdit(m_selectedObjectPtr);
			} else m_pColliderEditor->EndColliderEdit();
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		if ((wParam == 'I' || wParam == 'i') && m_pColliderEditor->IsColliderEditMode()) {
			m_pColliderEditor->ShowColliderDialog(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		if (wParam == 'B' && m_pColliderEditor->IsColliderEditMode()) {
			m_pColliderEditor->ToggleColliderType();
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
	}
	break;
	case WM_LBUTTONUP:
		if (m_pColliderEditor->IsDraggingCollider()) {
			m_pColliderEditor->OnLeftButtonUp();
			ReleaseCapture();
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
