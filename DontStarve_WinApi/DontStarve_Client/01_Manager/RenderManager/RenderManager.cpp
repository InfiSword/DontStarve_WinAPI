#include "../../99_Default/pch.h"
#include "RenderManager.h"
#include "../../02_GameObject/GameObject.h"
#include "../../03_Animation/Animator.h"
#include "../CameraManager/CameraManager.h"
#include "../ObjectManager/ObjectManager.h"

RenderManager::RenderManager() {}
RenderManager::~RenderManager() {}

void RenderManager::Init()
{
	m_drawCommands.reserve(1000); // 대규모 렌더링을 대비한 초기 메모리 확보
}

void RenderManager::LateInit()
{
}

void RenderManager::Update(float deltaTime)
{
}

void RenderManager::LateUpdate()
{
}

void RenderManager::Render()
{
	// 카메라에 보이는 객체만 렌더
	RenderVisibleGameObjects();
	
	// ObjectManager가 요청한 디버그 바운딩 박스를 함께 그림
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager && objectManager->IsBoundsDisplayEnabled()) {
		objectManager->RenderBounds();
	}
}

void RenderManager::Release()
{
	Clear();
}

void RenderManager::AddDrawCommand(Gdiplus::Bitmap* pBitmap, const Gdiplus::RectF& destRect, const Gdiplus::RectF& sourceRect, Gdiplus::Unit srcUnit, const Gdiplus::PointF& objectScreenPos, RenderLayer layer, float sortKey, Direction direction)
{
	if (!pBitmap) {
		return;
	}

	if (pBitmap->GetLastStatus() != Gdiplus::Ok) {
		return;
	}

	m_drawCommands.emplace_back(DrawCommand(pBitmap, destRect, sourceRect, srcUnit, objectScreenPos, layer, sortKey, direction));
}

void RenderManager::AddTextCommand(const std::wstring& text, Gdiplus::Font* pFont, Gdiplus::Brush* pBrush, Gdiplus::StringFormat* pStringFormat, const Gdiplus::RectF& destRect, RenderLayer layer, float sortKey)
{
	m_drawCommands.emplace_back(DrawCommand(text, pFont, pBrush, pStringFormat, destRect, layer, sortKey));
}

// 사각형 외곽선 렌더링
void RenderManager::AddDrawCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float sortKey) {
	m_drawCommands.emplace_back(DrawCommand(rect, color, thickness, layer, sortKey));
}

void RenderManager::AddFillRectangleCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, RenderLayer layer, float sortKey) {
	m_drawCommands.emplace_back(DrawCommand(rect, color, layer, sortKey, true));
}

// === UI 전용 렌더링 헬퍼 ===

void RenderManager::RenderUIImage(Gdiplus::Bitmap* bitmap, float x, float y, float width, float height,
	RenderLayer layer, float sortKey)
{
	if (!bitmap) return;

	Gdiplus::RectF destRect(x, y, width, height);
	Gdiplus::RectF sourceRect(0, 0, static_cast<float>(bitmap->GetWidth()), static_cast<float>(bitmap->GetHeight()));

	AddDrawCommand(bitmap, destRect, sourceRect, Gdiplus::UnitPixel,
		Gdiplus::PointF(x + width / 2, y + height / 2), layer, sortKey, DIR_DOWN);
}

void RenderManager::RenderUIText(const std::wstring& text, Gdiplus::Font* font, Gdiplus::Brush* brush,
	float x, float y, float width, float height,
	RenderLayer layer, float sortKey)
{
	// 전달 인자의 유효성을 확인
	if (!font || !brush || text.empty()) return;

	// 폰트 패밀리를 얻을 수 있는지 확인
	Gdiplus::FontFamily fontFamily;
	if (font->GetFamily(&fontFamily) != Gdiplus::Ok) return;

	Gdiplus::StringFormat* stringFormat = new Gdiplus::StringFormat();
	if (!stringFormat) return;

	stringFormat->SetAlignment(Gdiplus::StringAlignmentCenter);
	stringFormat->SetLineAlignment(Gdiplus::StringAlignmentCenter);

	Gdiplus::RectF textRect(x, y, width, height);
	AddTextCommand(text, font, brush, stringFormat, textRect, layer, sortKey);

	// StringFormat은 DrawCommand 내부에서 관리되므로 해제하지 않는다
}

// === GameObject 렌더링 ===

void RenderManager::RenderGameObject(GameObject* pObject)
{
	if (!pObject || !pObject->GetActive()) {
		return;
	}

	Animator* anim = pObject->GetComponent<Animator>();

	if (anim != nullptr) {
		// 월드 좌표를 스크린 좌표로 변환
		Gdiplus::PointF screenPos = CameraManager::GetInstance()->WorldToScreen(pObject->GetX(), pObject->GetY());

		// 렌더 레이어 및 정렬 키 계산
		RenderLayer layer = pObject->GetRenderLayer();
		float sortKey = pObject->GetSortKey(layer);

		// Animator가 렌더 명령을 생성하도록 위임
		anim->Draw(nullptr, screenPos, 1.0f, pObject->GetDir(), layer, sortKey);
	}
	else 
	{
		// Animator가 없으면 비트맵을 직접 렌더
		Gdiplus::Bitmap* pBitmap = pObject->GetBitmap();
		if (!pBitmap) {
			return;
		}
		
		// 월드 좌표 -> 스크린 좌표
		Gdiplus::PointF screenPos = CameraManager::GetInstance()->WorldToScreen(pObject->GetX(), pObject->GetY());

		// 비트맵 크기
		float width = static_cast<float>(pBitmap->GetWidth());
		float height = static_cast<float>(pBitmap->GetHeight());

		// 피벗을 고려한 렌더 위치
		float renderX = screenPos.X - width * pObject->GetPivotX();
		float renderY = screenPos.Y - height * pObject->GetPivotY();

		// 레이어 및 정렬 키 계산
		RenderLayer layer = pObject->GetRenderLayer();
		float sortKey = pObject->GetSortKey(layer);

		// RenderManager 큐에 직접 명령 추가
		AddDrawCommand(
			pBitmap,
			Gdiplus::RectF(renderX, renderY, width, height),
			Gdiplus::RectF(0, 0, width, height),
			Gdiplus::UnitPixel,
			screenPos,
			layer,
			sortKey,
			pObject->GetDir()
		);
	}
}

void RenderManager::RenderTile(Gdiplus::Bitmap* pTileBitmap, float worldX, float worldY, float width, float height)
{
	if (!pTileBitmap) return;

	// 월드 좌표(타일 중심)를 스크린 좌표로 변환
	Gdiplus::PointF screenPos = CameraManager::GetInstance()->WorldToScreen(worldX, worldY);

	// 타일 중심 기준 화면 위치 계산
	float renderX = screenPos.X - width * 0.5f; // 중심 X - 절반 너비
	float renderY = screenPos.Y - height * 0.5f; // 중심 Y - 절반 높이

	// 타일은 항상 바닥 레이어에 위치
	float sortKey = LAYER_WORLD_TILE;

	// 렌더 명령 추가
	Gdiplus::RectF destRect(renderX, renderY, width, height); 
	Gdiplus::RectF sourceRect(0, 0, static_cast<float>(pTileBitmap->GetWidth()), static_cast<float>(pTileBitmap->GetHeight()));

	AddDrawCommand(pTileBitmap, destRect, sourceRect, Gdiplus::UnitPixel, screenPos, LAYER_WORLD_TILE, sortKey, DIR_DOWN);
}

void RenderManager::RenderVisibleGameObjects()
{
	// CameraManager에서 가시 객체 목록을 가져온다
	CameraManager* cameraManager = CameraManager::GetInstance();
	if (!cameraManager) {
		return;
	}

	const std::vector<GameObject*>& visibleObjects = cameraManager->GetVisibleObjects();
	
	// 이미 클리핑된 객체만 순회하면서 렌더
	for (GameObject* obj : visibleObjects) {
		if (obj && obj->GetActive()) {
			RenderGameObject(obj);
		}
	}
}

void RenderManager::Clear() {
	m_drawCommands.clear();
}

// Animator::Draw에서 사용하는 변환 로직과 동일하게 유지한다.
void RenderManager::ApplyGdiTransform(Gdiplus::Graphics* pGraphics, const DrawCommand& command, float scaledWidth, float scaledHeight)
{
	if (command.layer >= LAYER_UI_BACKGROUND || command.type == DRAW_COMMAND_TEXT)
	{
		return;
	}

	float transformCenterX = command.destRect.X + scaledWidth / 2.0f;
	float transformCenterY = command.destRect.Y + scaledHeight / 2.0f;
	switch (command.direction) {
	case DIR_DOWN:
		break;
	case DIR_UP:
		break;
	case DIR_LEFT:
		pGraphics->TranslateTransform(transformCenterX, transformCenterY);
		pGraphics->ScaleTransform(-1.0f, 1.0f); // X축 반전
		pGraphics->TranslateTransform(-transformCenterX, -transformCenterY);
		break;
	case DIR_RIGHT:
		break;
	}
}

void RenderManager::Flush(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics || m_drawCommands.empty()) return;

	// 1. 레이어 → sortKey 순으로 정렬해 Z-order를 보장
	std::sort(m_drawCommands.begin(), m_drawCommands.end(), CompareDrawCommands);

	Gdiplus::Bitmap* currentBitmap = nullptr;
	Gdiplus::GraphicsState originalGraphicsState; // Flush 시작 시점의 그래픽 상태

	int commandIndex = 0;
	for (const auto& cmd : m_drawCommands) {
		Gdiplus::GraphicsState commandSpecificState = pGraphics->Save(); // 명령 단위로 상태 저장

		float currentScaledWidth = cmd.destRect.Width;
		float currentScaledHeight = cmd.destRect.Height;

		if (cmd.type == DRAW_COMMAND_IMAGE) {
			// 비트맵 유효성 재검사
			if (!cmd.pBitmap) {
				pGraphics->Restore(commandSpecificState);
				commandIndex++;
				continue;
			}

			// 비트맵 상태 확인
			if (cmd.pBitmap->GetLastStatus() != Gdiplus::Ok) {
				pGraphics->Restore(commandSpecificState);
				commandIndex++;
				continue;
			}

			// 소스/목적 사각형 검증
			if (cmd.sourceRect.Width <= 0 || cmd.sourceRect.Height <= 0) {
				pGraphics->Restore(commandSpecificState);
				commandIndex++;
				continue;
			}

			if (cmd.destRect.Width <= 0 || cmd.destRect.Height <= 0) {
				pGraphics->Restore(commandSpecificState);
				commandIndex++;
				continue;
			}

			ApplyGdiTransform(pGraphics, cmd, cmd.destRect.Width, cmd.destRect.Height);
			pGraphics->DrawImage(cmd.pBitmap, cmd.destRect, cmd.sourceRect.X, cmd.sourceRect.Y,
				cmd.sourceRect.Width, cmd.sourceRect.Height, cmd.srcUnit);

		}
		else if (cmd.type == DRAW_COMMAND_TEXT) {
			if (cmd.pFont && cmd.pBrush && cmd.pStringFormat) {

				pGraphics->DrawString(cmd.text.c_str(), -1, cmd.pFont, cmd.destRect, cmd.pStringFormat, cmd.pBrush);
			}
		}
		else if (cmd.type == DRAW_COMMAND_RECTANGLE) {

			Gdiplus::Pen pen(cmd.color, cmd.thickness);
			pGraphics->DrawRectangle(&pen, cmd.destRect);

		}
		else if (cmd.type == DRAW_COMMAND_FILL_RECTANGLE) {
			Gdiplus::SolidBrush brush(cmd.color);
			pGraphics->FillRectangle(&brush, cmd.destRect);

		}
		pGraphics->Restore(commandSpecificState);
		commandIndex++;
	}
	Clear(); // 모든 명령을 처리한 뒤 큐를 초기화
}