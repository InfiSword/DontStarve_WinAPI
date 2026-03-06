#include "99_Default/pch.h"
#include "RenderManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../../02_GameObject/Component/Transform/RectTransform.h"
#include "../../02_GameObject/Component/Sprite/SpriteRenderer.h"
#include "../../02_GameObject/Component/Sprite/Image.h"
#include "../../03_Animation/Animator.h"
#include "../../03_Animation/SpriteSheet.h"
#include "../../02_GameObject/GameObject.h"

RenderManager::RenderManager()
{
}

RenderManager::~RenderManager()
{
	Release();
}

void RenderManager::Init()
{
	// 각 레이어별로 충분한 메모리를 미리 할당하여 런타임 재할당 오버헤드 방지
	// 총합 약 2000개 이상의 커맨드를 수용할 수 있도록 레이어당 넉넉히 예약 (프레임간 메모리 유지)
	for (int i = 0; i < LAYER_COUNT; ++i) {
		m_layerCommands[i].reserve(512);
	}

	// GDI+ 객체 캐싱 초기화
	m_pCachedPen = new Gdiplus::Pen(Gdiplus::Color(0, 0, 0, 0));
	m_pCachedBrush = new Gdiplus::SolidBrush(Gdiplus::Color(0, 0, 0, 0));
	m_pCachedAttr = new Gdiplus::ImageAttributes();
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

void RenderManager::Release()
{
	Clear();

	// 캐싱된 GDI+ 객체 해제
	if (m_pCachedPen) { delete m_pCachedPen; m_pCachedPen = nullptr; }
	if (m_pCachedBrush) { delete m_pCachedBrush; m_pCachedBrush = nullptr; }
	if (m_pCachedAttr) { delete m_pCachedAttr; m_pCachedAttr = nullptr; }
}

void RenderManager::AddDrawCommand(Gdiplus::Bitmap* pBitmap, const Gdiplus::RectF& destRect, const Gdiplus::RectF& sourceRect, Gdiplus::Unit srcUnit, const Gdiplus::PointF& objectScreenPos, RenderLayer layer, float sortKey, Direction direction, const Gdiplus::Color& tintColor, bool hasTint, bool preFlipped)
{
	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_IMAGE;
	cmd.pBitmap = pBitmap;
	cmd.destRect = destRect;
	cmd.sourceRect = sourceRect;
	cmd.srcUnit = srcUnit;
	cmd.objectScreenPos = objectScreenPos;
	cmd.layer = layer;
	cmd.sortKey = sortKey;
	cmd.direction = direction;
	cmd.tintColor = tintColor;
	cmd.hasTint = hasTint;
	cmd.preFlipped = preFlipped;

	m_layerCommands[layer].push_back(cmd);
}

void RenderManager::AddTextCommand(const std::wstring* text, Gdiplus::Font* pFont, Gdiplus::Brush* pBrush, Gdiplus::StringFormat* pStringFormat, const Gdiplus::RectF& destRect, RenderLayer layer, float sortKey)
{
	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_TEXT;
	cmd.textPtr = text;
	cmd.pFont = pFont;
	cmd.pBrush = pBrush;
	cmd.pStringFormat = pStringFormat;
	cmd.destRect = destRect;
	cmd.layer = layer;
	cmd.sortKey = sortKey;

	m_layerCommands[layer].push_back(cmd);
}

void RenderManager::AddDrawRectCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float sortKey)
{
	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_RECTANGLE;
	cmd.destRect = rect;
	cmd.color = color;
	cmd.thickness = thickness;
	cmd.layer = layer;
	cmd.sortKey = sortKey;

	m_layerCommands[layer].push_back(cmd);
}

void RenderManager::AddFillRectangleCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, RenderLayer layer, float sortKey)
{
	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_FILL_RECTANGLE;
	cmd.destRect = rect;
	cmd.color = color;
	cmd.layer = layer;
	cmd.sortKey = sortKey;

	m_layerCommands[layer].push_back(cmd);
}

void RenderManager::AddDrawEllipseCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float sortKey)
{
	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_ELLIPSE;
	cmd.destRect = rect;
	cmd.color = color;
	cmd.thickness = thickness;
	cmd.layer = layer;
	cmd.sortKey = sortKey;

	m_layerCommands[layer].push_back(cmd);
}

void RenderManager::AddUIImageCommand(Gdiplus::Bitmap* bitmap, float x, float y, 
	float width, float height, float pivotX, float pivotY, RenderLayer layer, 
	float sortKey, const Gdiplus::Color& tintColor, bool hasTint)
{
	if (!bitmap) return;

	float renderX = x - (pivotX * width);
	float renderY = y - (pivotY * height);

	Gdiplus::RectF destRect(renderX, renderY, width, height);
	Gdiplus::RectF sourceRect(0, 0, static_cast<float>(bitmap->GetWidth()), static_cast<float>(bitmap->GetHeight()));

	AddDrawCommand(
		bitmap,
		destRect,
		sourceRect,
		Gdiplus::UnitPixel,
		Gdiplus::PointF(x, y),
		layer,
		sortKey,
		DIR_DOWN,
		tintColor,
		hasTint
	);
}

void RenderManager::RenderGameObject(GameObject* pObject)
{
	if (!pObject || !pObject->IsEnabled()) return;

	// [최적화] GetComponent 대신 미리 캐싱된 컴포넌트들을 확인합니다.
	// SpriteRenderer(월드)와 Image(UI) 중 어떤 것이 있는지 확인
	SpriteRenderer* spriteRenderer = pObject->GetComponent<SpriteRenderer>();
	ComponentElement::Image* image = pObject->GetComponent<ComponentElement::Image>();
	Animator* anim = pObject->GetComponent<Animator>();

	if (!spriteRenderer && !image && !anim) return;

	// --- 1. 월드 오브젝트 처리 (Transform 기반) ---
	if (spriteRenderer || anim)
	{
		// [캐싱 활용] SpriteRenderer 내부에 미리 저장된 Transform을 가져옵니다.
		Transform* transform =  pObject->GetComponent<Transform>();
		if (!transform) return;

		CameraManager* pCam = CameraManager::GetInstance();
		Gdiplus::RectF viewport = pCam->GetViewportWorldRect();

		float worldX = transform->GetX();
		float worldY = transform->GetY();

		Gdiplus::PointF screenPos = pCam->WorldToScreen(worldX, worldY);
		RenderLayer layer = (spriteRenderer) ? spriteRenderer->GetLayer() : LAYER_WORLD_OBJECT;
		float sortKey = transform->GetSortKey(layer);
		Direction dir = transform->GetDirection();

		if (anim) {
			anim->Draw(nullptr, screenPos, 1.0f, dir, layer, sortKey);
		}
		else if (spriteRenderer) {
			auto spriteHandle = spriteRenderer->GetSpriteHandle();
			if (!spriteHandle || !spriteHandle->bitmap) return;

			float width = spriteHandle->sourceRect.Width;
			float height = spriteHandle->sourceRect.Height;
			float x = screenPos.X - width * transform->GetPivotX();
			float y = screenPos.Y - height * transform->GetPivotY();

			AddDrawCommand(spriteHandle->bitmap.get(), Gdiplus::RectF(x, y, width, height),
				spriteHandle->sourceRect, Gdiplus::UnitPixel, screenPos,
				layer, sortKey, dir, spriteHandle->tintColor,
				(spriteHandle->tintColor.GetA() < 255));
		}
	}
	// --- 2. UI 오브젝트 처리 (RectTransform 기반) ---
	else if (image)
	{
		// [캐싱 활용] Image 컴포넌트 내부에 캐싱된 RectTransform 사용
		RectTransform* rectTransform = pObject->GetComponent<RectTransform>();
		if (!rectTransform) return;

		auto spriteHandle = image->GetSpriteHandle();
		if (!spriteHandle || !spriteHandle->bitmap) return;

		float width = spriteHandle->sourceRect.Width * rectTransform->GetScaleX();
		float height = spriteHandle->sourceRect.Height * rectTransform->GetScaleY();
		float x = rectTransform->GetX();
		float y = rectTransform->GetY();

		// UI는 카메라 변환 없이 RectTransform의 좌표를 직접 사용합니다.
		float renderX = x - (rectTransform->GetPivotX() * width);
		float renderY = y - (rectTransform->GetPivotY() * height);

		AddDrawCommand(spriteHandle->bitmap.get(), Gdiplus::RectF(renderX, renderY, width, height),
			spriteHandle->sourceRect, Gdiplus::UnitPixel, Gdiplus::PointF(x, y),
			image->GetLayer(), image->GetSortKey(), DIR_DOWN,
			spriteHandle->tintColor, (spriteHandle->tintColor.GetA() < 255));
	}
}

void RenderManager::RenderTile(Gdiplus::Bitmap* pTileBitmap, float worldX, float worldY, float width, float height)
{
	if (!pTileBitmap) return;

	Gdiplus::PointF screenPos = CameraManager::GetInstance()->WorldToScreen(worldX, worldY);
	float renderX = screenPos.X - width * 0.5f;
	float renderY = screenPos.Y - height * 0.5f;

	AddDrawCommand(pTileBitmap, Gdiplus::RectF(renderX, renderY, width, height), Gdiplus::RectF(0, 0, (float)pTileBitmap->GetWidth(), (float)pTileBitmap->GetHeight()), Gdiplus::UnitPixel, screenPos, LAYER_TILE_BACKGROUND, worldY, DIR_DOWN);
}

void RenderManager::Clear()
{
	for (int i = 0; i < LAYER_COUNT; ++i) {
		m_layerCommands[i].clear();
	}
}

void RenderManager::Flush(Gdiplus::Graphics* pGraphics)
{
	if (!pGraphics) return;

	// 레이어 순서대로 순회하며 각 레이어 내의 오브젝트들만 정렬하여 출력
	for (int i = 0; i < LAYER_COUNT; ++i) {
		if (m_layerCommands[i].empty()) continue;
		
		// 해당 레이어 내에서만 sortKey 기준으로 정렬 (전체 정렬보다 훨씬 빠름)
		std::sort(m_layerCommands[i].begin(), m_layerCommands[i].end(), CompareDrawCommands);
		
		for (const auto& cmd : m_layerCommands[i]) {
			switch (cmd.type) {
			case DRAW_COMMAND_IMAGE:
				if (cmd.pBitmap) {
					if (cmd.hasTint) {
						float r = cmd.tintColor.GetR() / 255.0f;
						float g = cmd.tintColor.GetG() / 255.0f;
						float b = cmd.tintColor.GetB() / 255.0f;
						float a = cmd.tintColor.GetA() / 255.0f;
						Gdiplus::ColorMatrix matrix = 
						{
						  r,0,0,0,0,
						  0,g,0,0,0,
						  0,0,b,0,0,
						  0,0,0,a,0,
					      0,0,0,0,1 
						};
						m_pCachedAttr->SetColorMatrix(&matrix);
						pGraphics->DrawImage(cmd.pBitmap,
							cmd.destRect, cmd.sourceRect.X, cmd.sourceRect.Y,
							cmd.sourceRect.Width, cmd.sourceRect.Height, cmd.srcUnit, m_pCachedAttr);
					}
					else {
						pGraphics->DrawImage(cmd.pBitmap, 
							cmd.destRect, cmd.sourceRect.X, cmd.sourceRect.Y, 
							cmd.sourceRect.Width, cmd.sourceRect.Height, cmd.srcUnit);
					}
				}
				break;
			case DRAW_COMMAND_TEXT:
				if (cmd.textPtr) {
					pGraphics->DrawString(cmd.textPtr->c_str(), -1, cmd.pFont, cmd.destRect, cmd.pStringFormat, cmd.pBrush);
				}
				break;
			case DRAW_COMMAND_RECTANGLE:
			{
				m_pCachedPen->SetColor(cmd.color);
				m_pCachedPen->SetWidth(cmd.thickness);
				pGraphics->DrawRectangle(m_pCachedPen, cmd.destRect);
			}
			break;
			case DRAW_COMMAND_FILL_RECTANGLE:
			{
				m_pCachedBrush->SetColor(cmd.color);
				pGraphics->FillRectangle(m_pCachedBrush, cmd.destRect);
			}
			break;
			case DRAW_COMMAND_ELLIPSE:
			{
				m_pCachedPen->SetColor(cmd.color);
				m_pCachedPen->SetWidth(cmd.thickness);
				pGraphics->DrawEllipse(m_pCachedPen, cmd.destRect);
			}
			break;
			}
		}
		m_layerCommands[i].clear();
	}
}