#include "99_Default/pch.h"
#include "RenderManager.h"
#include "../../02_GameObject/GameObject.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../../02_GameObject/Component/Transform/RectTransform.h"
#include "../../02_GameObject/Component/Sprite/SpriteRenderer.h"
#include "../../02_GameObject/Component/Sprite/Image.h"
#include "../../03_Animation/Animator.h"
#include "../CameraManager/CameraManager.h"
#include "../ObjectManager/ObjectManager.h"
#include "../ColliderManager/ColliderManager.h"

RenderManager::RenderManager() {}
RenderManager::~RenderManager() {}

void RenderManager::Init()
{
	m_drawCommands.reserve(1000); // 그리기 명령 큐를 위한 초기 메모리 할당
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
	
	// ColliderManager의 Gizmo 렌더링
	ColliderManager* colliderManager = ColliderManager::GetInstance();
	if (colliderManager) {
		colliderManager->RenderGizmos();
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

	DrawCommand command{};
	command.type = DRAW_COMMAND_IMAGE;
	command.pBitmap = pBitmap;
	command.destRect = destRect;
	command.sourceRect = sourceRect;
	command.srcUnit = srcUnit;
	command.objectScreenPos = objectScreenPos;
	command.layer = layer;
	command.sortKey = sortKey;
	command.direction = direction;

	m_drawCommands.emplace_back(std::move(command));
}

void RenderManager::AddTextCommand(const std::wstring& text, Gdiplus::Font* pFont, Gdiplus::Brush* pBrush, Gdiplus::StringFormat* pStringFormat, const Gdiplus::RectF& destRect, RenderLayer layer, float sortKey)
{
	DrawCommand command{};
	command.type = DRAW_COMMAND_TEXT;
	command.text = text;
	command.pFont = pFont;
	command.pBrush = pBrush;
	command.pStringFormat = pStringFormat;
	command.destRect = destRect;
	command.objectScreenPos = Gdiplus::PointF(destRect.X, destRect.Y);
	command.layer = layer;
	command.sortKey = sortKey;

	m_drawCommands.emplace_back(std::move(command));
}

// 사각형 외곽선 명령 추가
void RenderManager::AddDrawCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float sortKey) {
	DrawCommand command{};
	command.type = DRAW_COMMAND_RECTANGLE;
	command.destRect = rect;
	command.color = color;
	command.thickness = thickness;
	command.layer = layer;
	command.sortKey = sortKey;
	command.objectScreenPos = Gdiplus::PointF(rect.X, rect.Y);

	m_drawCommands.emplace_back(std::move(command));
}

void RenderManager::AddFillRectangleCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, RenderLayer layer, float sortKey) {
	DrawCommand command{};
	command.type = DRAW_COMMAND_FILL_RECTANGLE;
	command.destRect = rect;
	command.color = color;
	command.layer = layer;
	command.sortKey = sortKey;
	command.objectScreenPos = Gdiplus::PointF(rect.X, rect.Y);

	m_drawCommands.emplace_back(std::move(command));
}

// === UI 전용 명령 큐 함수 ===

void RenderManager::RenderUIImageWithPivot(Gdiplus::Bitmap* bitmap, float x, float y, float width, float height,
	float pivotX, float pivotY,
	RenderLayer layer, float sortKey)
{
	if (!bitmap) return;

	Gdiplus::RectF destRect(x - (pivotX * width), y - (pivotY * height), width, height);
	Gdiplus::RectF sourceRect(0, 0, static_cast<float>(bitmap->GetWidth()), static_cast<float>(bitmap->GetHeight()));

	AddDrawCommand(bitmap, destRect, sourceRect, Gdiplus::UnitPixel,
		Gdiplus::PointF(x, y), layer, sortKey, DIR_DOWN);
}

void RenderManager::RenderUIText(const std::wstring& text, Gdiplus::Font* font, Gdiplus::Brush* brush,
	float x, float y, float width, float height,
	RenderLayer layer, float sortKey)
{
	// 폰트 인자의 유효성을 확인
	if (!font || !brush || text.empty()) return;

	// 폰트 패밀리를 가져올 수 있는지 확인
	Gdiplus::FontFamily fontFamily;
	if (font->GetFamily(&fontFamily) != Gdiplus::Ok) return;

	Gdiplus::StringFormat* stringFormat = new Gdiplus::StringFormat();
	if (!stringFormat) return;

	stringFormat->SetAlignment(Gdiplus::StringAlignmentCenter);
	stringFormat->SetLineAlignment(Gdiplus::StringAlignmentCenter);

	Gdiplus::RectF textRect(x, y, width, height);
	AddTextCommand(text, font, brush, stringFormat, textRect, layer, sortKey);

	// TODO: StringFormat 수명 관리가 필요하면 별도 해제 로직 추가
}

// === GameObject 렌더링 ===

void RenderManager::RenderGameObject(GameObject* pObject)
{
	if (!pObject || !pObject->IsEnabled()) {
		return;
	}

	// Transform 또는 RectTransform 컴포넌트 가져오기
	Transform* transform = pObject->GetComponent<Transform>();
	RectTransform* rectTransform = pObject->GetComponent<RectTransform>();
	
	// Transform이 없으면 렌더링 불가 (UI는 자체 Render() 사용)
	if (!transform && !rectTransform) {
		return;
	}

	// SpriteRenderer 또는 Image 컴포넌트 가져오기
	SpriteRenderer* spriteRenderer = pObject->GetComponent<SpriteRenderer>();
	ComponentElement::Image* image = pObject->GetComponent<ComponentElement::Image>();

	Animator* anim = pObject->GetComponent<Animator>();

	if (anim != nullptr) {
		// Animator는 Transform만 지원 (월드 오브젝트)
		if (!transform) {
			return;
		}

		// 월드 좌표를 화면 좌표로 변환
		Gdiplus::PointF screenPos = CameraManager::GetInstance()->WorldToScreen(transform->GetX(), transform->GetY());

		// 렌더 레이어 및 정렬 키 계산
		RenderLayer layer = spriteRenderer ? spriteRenderer->GetLayer() : LAYER_WORLD_OBJECT;
		float sortKey = transform->GetSortKey(layer);

		// Animator가 렌더 명령을 생성하도록 위치
		anim->Draw(nullptr, screenPos, 1.0f, transform->GetDirection(), layer, sortKey);
	}
	else 
	{
		// SpriteRenderer 또는 Image 중 하나라도 있어야 렌더링 가능
		Gdiplus::Bitmap* pBitmap = nullptr;
		std::shared_ptr<Sprite> spriteHandle = nullptr;
		Gdiplus::RectF srcRect(0, 0, 0, 0);
		RenderLayer layer = LAYER_WORLD_OBJECT;
		float sortKey = 0.0f;
		float x = 0.0f, y = 0.0f;
		float width = 0.0f, height = 0.0f;
		float pivotX = 0.5f, pivotY = 0.5f;
		Direction direction = DIR_DOWN;
		Gdiplus::PointF screenPos;

		// SpriteRenderer 우선 확인 (월드 오브젝트)
		if (spriteRenderer && transform) {
			spriteHandle = spriteRenderer->GetSpriteHandle();
			if (!spriteHandle || !spriteHandle->bitmap) {
				return;
			}
			pBitmap = spriteHandle->bitmap.get();
			srcRect = spriteHandle->sourceRect;

			// 월드 좌표 -> 화면 좌표
			screenPos = CameraManager::GetInstance()->WorldToScreen(transform->GetX(), transform->GetY());

			// Sprite의 실제 크기 사용 (비트맵 크기)
			width = srcRect.Width;
			height = srcRect.Height;

			// 피벗을 고려한 렌더 위치
			x = screenPos.X - width * transform->GetPivotX();
			y = screenPos.Y - height * transform->GetPivotY();
			pivotX = transform->GetPivotX();
			pivotY = transform->GetPivotY();
			direction = transform->GetDirection();

			// 레이어 및 정렬 키 계산
			layer = spriteRenderer->GetLayer();
			sortKey = transform->GetSortKey(layer);
		}
		// Image 컴포넌트 확인 (UI 오브젝트)
		else if (image && rectTransform) {
			spriteHandle = image->GetSpriteHandle();
			if (!spriteHandle || !spriteHandle->bitmap) {
				return;
			}
			pBitmap = spriteHandle->bitmap.get();
			srcRect = spriteHandle->sourceRect;

			// UI는 화면 좌표를 직접 사용
			x = rectTransform->GetX();
			y = rectTransform->GetY();
			pivotX = rectTransform->GetPivotX();
			pivotY = rectTransform->GetPivotY();

			// Sprite 크기 * scale 계산
			float bitmapWidth = srcRect.Width;
			float bitmapHeight = srcRect.Height;
			width = bitmapWidth * rectTransform->GetScaleX();
			height = bitmapHeight * rectTransform->GetScaleY();

			// 피벗을 고려한 렌더 위치
			x = x - (pivotX * width);
			y = y - (pivotY * height);

			// 레이어 및 정렬 키는 Image 컴포넌트에서 가져오기
			layer = image->GetLayer();
			sortKey = image->GetSortKey();

			// UI는 화면 좌표 중심점 계산
			screenPos = Gdiplus::PointF(x + width / 2, y + height / 2);
		}
		else {
			// 렌더링할 수 없음
			return;
		}

		// RenderManager 큐에 직접 명령 추가
		AddDrawCommand(
			pBitmap,
			Gdiplus::RectF(x, y, width, height),
			srcRect,
			Gdiplus::UnitPixel,
			screenPos,
			layer,
			sortKey,
			direction
		);
	}
}

void RenderManager::RenderTile(Gdiplus::Bitmap* pTileBitmap, float worldX, float worldY, float width, float height)
{
	if (!pTileBitmap) return;

	// 월드 좌표(타일 중심)를 화면 좌표로 변환
	Gdiplus::PointF screenPos = CameraManager::GetInstance()->WorldToScreen(worldX, worldY);

	// 타일 중심 기준으로 화면 위치 계산
	float renderX = screenPos.X - width * 0.5f; // 중심 X - 절반 너비
	float renderY = screenPos.Y - height * 0.5f; // 중심 Y - 절반 높이

	// 타일은 항상 같은 레이어에 위치
	float sortKey = LAYER_WORLD_TILE;

	// 렌더 명령 추가
	Gdiplus::RectF destRect(renderX, renderY, width, height); 
	Gdiplus::RectF sourceRect(0, 0, static_cast<float>(pTileBitmap->GetWidth()), static_cast<float>(pTileBitmap->GetHeight()));

	AddDrawCommand(pTileBitmap, destRect, sourceRect, Gdiplus::UnitPixel, screenPos, LAYER_WORLD_TILE, sortKey, DIR_DOWN);
}

void RenderManager::RenderVisibleGameObjects()
{
	// CameraManager에서 보이는 객체 리스트를 가져온다
	CameraManager* cameraManager = CameraManager::GetInstance();
	if (!cameraManager) {
		return;
	}

	const std::vector<GameObject*>& visibleObjects = cameraManager->GetVisibleObjects();
	
	// 이렇게 필터링된 객체만 순회하면서 렌더
	for (GameObject* obj : visibleObjects) {
		if (obj && obj->IsEnabled()) {
			RenderGameObject(obj);
		}
	}
}

void RenderManager::Clear() {
	m_drawCommands.clear();
}

// Animator::Draw에서 사용하는 변환 방식과 동일하게 적용한다.
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

	// 1. 레이어 높이 sortKey 순서로 정렬하여 Z-order를 보장
	std::sort(m_drawCommands.begin(), m_drawCommands.end(), RenderManager::CompareDrawCommands);

	for (const auto& cmd : m_drawCommands) {
		Gdiplus::GraphicsState commandSpecificState = pGraphics->Save(); // 명령 별도로 상태 저장

		if (cmd.type == DRAW_COMMAND_IMAGE) {
			// 비트맵 유효성 확인
			if (!cmd.pBitmap) {
				pGraphics->Restore(commandSpecificState);
				continue;
			}

			// 비트맵 상태 확인
			if (cmd.pBitmap->GetLastStatus() != Gdiplus::Ok) {
				pGraphics->Restore(commandSpecificState);
				continue;
			}

			// 소스/대상 사각형 유효성
			if (cmd.sourceRect.Width <= 0 || cmd.sourceRect.Height <= 0) {
				pGraphics->Restore(commandSpecificState);
				continue;
			}

			if (cmd.destRect.Width <= 0 || cmd.destRect.Height <= 0) {
				pGraphics->Restore(commandSpecificState);
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
	}
	Clear(); // 모든 명령을 처리한 뒤 큐를 초기화
}
