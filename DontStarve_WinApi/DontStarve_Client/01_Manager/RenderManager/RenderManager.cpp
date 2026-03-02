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
	m_drawCommands.clear();
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

	m_drawCommands.push_back(cmd);
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

	m_drawCommands.push_back(cmd);
}

void RenderManager::AddDrawCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float sortKey)
{
	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_RECTANGLE;
	cmd.destRect = rect;
	cmd.color = color;
	cmd.thickness = thickness;
	cmd.layer = layer;
	cmd.sortKey = sortKey;

	m_drawCommands.push_back(cmd);
}

void RenderManager::AddFillRectangleCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, RenderLayer layer, float sortKey)
{
	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_FILL_RECTANGLE;
	cmd.destRect = rect;
	cmd.color = color;
	cmd.layer = layer;
	cmd.sortKey = sortKey;

	m_drawCommands.push_back(cmd);
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

	m_drawCommands.push_back(cmd);
}

void RenderManager::RenderUIImageWithPivot(Gdiplus::Bitmap* bitmap, float x, float y, float width, float height, float pivotX, float pivotY, RenderLayer layer, float sortKey, const Gdiplus::Color& tintColor, bool hasTint)
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

	Transform* transform = pObject->GetComponent<Transform>();
	RectTransform* rectTransform = pObject->GetComponent<RectTransform>();
	
	if (!transform && !rectTransform) return;

	SpriteRenderer* spriteRenderer = pObject->GetComponent<SpriteRenderer>();
	ComponentElement::Image* image = pObject->GetComponent<ComponentElement::Image>();
	Animator* anim = pObject->GetComponent<Animator>();

	if (anim != nullptr) {
		if (!transform) return;
		if (spriteRenderer && !spriteRenderer->IsEnabled()) return;

		Gdiplus::PointF screenPos = CameraManager::GetInstance()->WorldToScreen(transform->GetX(), transform->GetY());
		RenderLayer layer = spriteRenderer ? spriteRenderer->GetLayer() : LAYER_WORLD_OBJECT;
		float sortKey = transform->GetSortKey(layer);

		anim->Draw(nullptr, screenPos, 1.0f, transform->GetDirection(), layer, sortKey);
	}
	else 
	{
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

		if (spriteRenderer && transform) {
			spriteHandle = spriteRenderer->GetSpriteHandle();
			if (!spriteHandle || !spriteHandle->bitmap) return;
			
			pBitmap = spriteHandle->bitmap.get();
			srcRect = spriteHandle->sourceRect;
			screenPos = CameraManager::GetInstance()->WorldToScreen(transform->GetX(), transform->GetY());
			width = srcRect.Width;
			height = srcRect.Height;
			pivotX = transform->GetPivotX();
			pivotY = transform->GetPivotY();
			x = screenPos.X - width * pivotX;
			y = screenPos.Y - height * pivotY;
			direction = transform->GetDirection();
			layer = spriteRenderer->GetLayer();
			sortKey = transform->GetSortKey(layer);
		}
		else if (image && rectTransform) {
			spriteHandle = image->GetSpriteHandle();
			if (!spriteHandle || !spriteHandle->bitmap) return;

			pBitmap = spriteHandle->bitmap.get();
			srcRect = spriteHandle->sourceRect;
			x = rectTransform->GetX();
			y = rectTransform->GetY();
			pivotX = rectTransform->GetPivotX();
			pivotY = rectTransform->GetPivotY();
			width = srcRect.Width * rectTransform->GetScaleX();
			height = srcRect.Height * rectTransform->GetScaleY();
			x = x - (pivotX * width);
			y = y - (pivotY * height);
			layer = image->GetLayer();
			sortKey = image->GetSortKey();
			screenPos = Gdiplus::PointF(rectTransform->GetX(), rectTransform->GetY());
		}
		else return;

		Gdiplus::Color tintColor = spriteHandle ? spriteHandle->tintColor : Gdiplus::Color(255, 255, 255, 255);
		bool hasTint = (tintColor.GetR() != 255 || tintColor.GetG() != 255 || tintColor.GetB() != 255 || tintColor.GetA() != 255);

		AddDrawCommand(pBitmap, Gdiplus::RectF(x, y, width, height), srcRect, Gdiplus::UnitPixel, screenPos, layer, sortKey, direction, tintColor, hasTint);
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
	m_drawCommands.clear();
}

void RenderManager::Flush(Gdiplus::Graphics* pGraphics)
{
	if (!pGraphics || m_drawCommands.empty()) return;

	std::sort(m_drawCommands.begin(), m_drawCommands.end(), CompareDrawCommands);

	for (const auto& cmd : m_drawCommands) {
		switch (cmd.type) {
		case DRAW_COMMAND_IMAGE:
			if (cmd.pBitmap) {
				if (cmd.hasTint) {
					Gdiplus::ImageAttributes attr;
					float r = cmd.tintColor.GetR() / 255.0f;
					float g = cmd.tintColor.GetG() / 255.0f;
					float b = cmd.tintColor.GetB() / 255.0f;
					float a = cmd.tintColor.GetA() / 255.0f;
					Gdiplus::ColorMatrix matrix = { r,0,0,0,0, 0,g,0,0,0, 0,0,b,0,0, 0,0,0,a,0, 0,0,0,0,1 };
					attr.SetColorMatrix(&matrix);
					pGraphics->DrawImage(cmd.pBitmap, cmd.destRect, cmd.sourceRect.X, cmd.sourceRect.Y, cmd.sourceRect.Width, cmd.sourceRect.Height, cmd.srcUnit, &attr);
				}
				else {
					pGraphics->DrawImage(cmd.pBitmap, cmd.destRect, cmd.sourceRect.X, cmd.sourceRect.Y, cmd.sourceRect.Width, cmd.sourceRect.Height, cmd.srcUnit);
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
				Gdiplus::Pen pen(cmd.color, cmd.thickness);
				pGraphics->DrawRectangle(&pen, cmd.destRect);
			}
			break;
		case DRAW_COMMAND_FILL_RECTANGLE:
			{
				Gdiplus::SolidBrush brush(cmd.color);
				pGraphics->FillRectangle(&brush, cmd.destRect);
			}
			break;
		case DRAW_COMMAND_ELLIPSE:
			{
				Gdiplus::Pen pen(cmd.color, cmd.thickness);
				pGraphics->DrawEllipse(&pen, cmd.destRect);
			}
			break;
		}
	}
	m_drawCommands.clear();
}

void RenderManager::ApplyDirectionFlip(Gdiplus::Graphics* pGraphics, const DrawCommand& command, float scaledWidth, float scaledHeight)
{
	// 현재는 Draw() 단에서 처리하거나 Animator에서 처리함. 필요 시 구현.
}
