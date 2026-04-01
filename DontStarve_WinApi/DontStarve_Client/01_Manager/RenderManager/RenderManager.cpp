#include "99_Default/pch.h"
#include "RenderManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../../02_GameObject/Component/Transform/RectTransform.h"
#include "../../02_GameObject/Component/Sprite/SpriteRenderer.h"
#include "../../02_GameObject/Component/Sprite/Image.h"
#include "../../02_GameObject/Component/Text/Text.h"
#include "../../03_Animation/Animator.h"
#include "../../02_GameObject/Component/Sprite/SpriteSheet.h"
#include "../../02_GameObject/GameObject.h"

RenderManager::RenderManager()
{
	// 생성자에서 레이어별 커맨드 벡터의 초기 공간을 미리 확보 (싱글톤 생성 시 1회 수행)
	for (int i = 0; i < LAYER_COUNT; ++i) {
		m_layerCommands[i].reserve(512);
	}
}

RenderManager::~RenderManager()
{
	Release();
}

void RenderManager::Init()
{
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

	// 캐싱된 GDI+ 객체 해제 및 nullptr 설정
	if (m_pCachedPen) { delete m_pCachedPen; m_pCachedPen = nullptr; }
	if (m_pCachedBrush) { delete m_pCachedBrush; m_pCachedBrush = nullptr; }
	if (m_pCachedAttr) { delete m_pCachedAttr; m_pCachedAttr = nullptr; }
}

void RenderManager::AddWorldEntityCommand(Gdiplus::Bitmap* pBitmap, const Gdiplus::RectF& sourceRect, float worldX, float worldY, float scaleX, float scaleY, float pivotX, float pivotY, RenderLayer layer, float zOrder, Direction direction, const Gdiplus::Color& tintColor, bool hasTint, bool preFlipped, float rotation)
{
	// 월드 좌표를 화면 좌표로 변환 (RenderManager가 전담)
	Gdiplus::PointF screenPos = CameraManager::GetInstance()->WorldToScreen(worldX, worldY);

	float width = sourceRect.Width * scaleX;
	float height = sourceRect.Height * scaleY;
	float renderX = screenPos.X - width * pivotX;
	float renderY = screenPos.Y - height * pivotY;

	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_ENTITY;
	cmd.destRect = Gdiplus::RectF(renderX, renderY, width, height);
	cmd.rotationPivot = screenPos; // 피벗 위치가 회전 중심
	cmd.layer = layer;
	cmd.zOrder = zOrder;
	cmd.rotation = rotation;

	cmd.sprite.pBitmap = pBitmap;
	cmd.sprite.sourceRect = sourceRect;
	cmd.sprite.srcUnit = Gdiplus::UnitPixel;
	cmd.sprite.direction = direction;
	cmd.sprite.tintColor = tintColor;
	cmd.sprite.hasTint = hasTint;
	cmd.sprite.preFlipped = preFlipped;

	m_layerCommands[layer].push_back(cmd);
}

void RenderManager::AddUICommand(Gdiplus::Bitmap* pBitmap, const Gdiplus::RectF& sourceRect, float screenX, float screenY, float scaleX, float scaleY, float pivotX, float pivotY, RenderLayer layer, float zOrder, const Gdiplus::Color& tintColor, bool hasTint, float rotation)
{
	float width = sourceRect.Width * scaleX;
	float height = sourceRect.Height * scaleY;
	float renderX = screenX - width * pivotX;
	float renderY = screenY - height * pivotY;

	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_UI_IMAGE;
	cmd.destRect = Gdiplus::RectF(renderX, renderY, width, height);
	cmd.rotationPivot = Gdiplus::PointF(screenX, screenY);
	cmd.layer = layer;
	cmd.zOrder = zOrder;
	cmd.rotation = rotation;

	cmd.sprite.pBitmap = pBitmap;
	cmd.sprite.sourceRect = sourceRect;
	cmd.sprite.srcUnit = Gdiplus::UnitPixel;
	cmd.sprite.tintColor = tintColor;
	cmd.sprite.hasTint = hasTint;

	m_layerCommands[layer].push_back(cmd);
}

void RenderManager::AddTextCommand(const std::wstring* text, Gdiplus::Font* pFont, Gdiplus::Brush* pBrush, Gdiplus::StringFormat* pStringFormat, const Gdiplus::RectF& destRect, RenderLayer layer, float zOrder, float rotation, const Gdiplus::PointF& rotationPivot)
{
	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_TEXT;
	cmd.destRect = destRect;
	cmd.layer = layer;
	cmd.zOrder = zOrder;
	cmd.rotation = rotation;
	cmd.rotationPivot = rotationPivot;

	cmd.text.textPtr = text;
	cmd.text.pFont = pFont;
	cmd.text.pBrush = pBrush;
	cmd.text.pStringFormat = pStringFormat;

	m_layerCommands[layer].push_back(cmd);
}

void RenderManager::AddDrawRectCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float zOrder)
{
	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_RECTANGLE;
	cmd.destRect = rect;
	cmd.layer = layer;
	cmd.zOrder = zOrder;

	cmd.primitive.color = color;
	cmd.primitive.thickness = thickness;
	cmd.primitive.isFilled = false;

	m_layerCommands[layer].push_back(cmd);
}

void RenderManager::AddFillRectangleCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, RenderLayer layer, float zOrder)
{
	DrawCommand cmd;
	cmd.type = DRAW_COMMAND_FILL_RECTANGLE;
	cmd.destRect = rect;
	cmd.layer = layer;
	cmd.zOrder = zOrder;

	cmd.primitive.color = color;
	cmd.primitive.thickness = 0.0f;
	cmd.primitive.isFilled = true;

	m_layerCommands[layer].push_back(cmd);
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

	for (int i = LAYER_TILE_BACKGROUND; i < LAYER_COUNT; ++i) {
		if (m_layerCommands[i].empty()) continue;
		
		std::sort(m_layerCommands[i].begin(), m_layerCommands[i].end(), CompareDrawCommands);
		
		for (const auto& cmd : m_layerCommands[i]) {
			Gdiplus::GraphicsState state;
			bool rotated = (cmd.rotation != 0.0f);
			
			if (rotated) {
				state = pGraphics->Save();
				pGraphics->TranslateTransform(cmd.rotationPivot.X, cmd.rotationPivot.Y);
				pGraphics->RotateTransform(cmd.rotation);
				pGraphics->TranslateTransform(-cmd.rotationPivot.X, -cmd.rotationPivot.Y);
			}

			switch (cmd.type) {
			case DRAW_COMMAND_ENTITY:
			case DRAW_COMMAND_UI_IMAGE:
				RenderSprite(pGraphics, cmd.sprite, cmd.destRect);
				break;
			case DRAW_COMMAND_TEXT:
				if (cmd.text.textPtr && cmd.text.pBrush) {
					pGraphics->DrawString(cmd.text.textPtr->c_str(), -1, cmd.text.pFont, cmd.destRect, cmd.text.pStringFormat, cmd.text.pBrush);
				}
				break;
			case DRAW_COMMAND_RECTANGLE:
				if (m_pCachedPen) {
					m_pCachedPen->SetColor(cmd.primitive.color);
					m_pCachedPen->SetWidth(cmd.primitive.thickness);
					pGraphics->DrawRectangle(m_pCachedPen, cmd.destRect);
				}
				break;
			case DRAW_COMMAND_FILL_RECTANGLE:
				if (m_pCachedBrush) {
					m_pCachedBrush->SetColor(cmd.primitive.color);
					pGraphics->FillRectangle(m_pCachedBrush, cmd.destRect);
				}
				break;			
			}

			if (rotated) {
				pGraphics->Restore(state);
			}
		}
		m_layerCommands[i].clear();
	}
}

void RenderManager::RenderSprite(Gdiplus::Graphics* pGraphics, const DrawCommand::SpriteData& data, const Gdiplus::RectF& destRect)
{
	if (!data.pBitmap) return;

	if (data.hasTint && m_pCachedAttr) {
		float r = data.tintColor.GetR() / 255.0f;
		float g = data.tintColor.GetG() / 255.0f;
		float b = data.tintColor.GetB() / 255.0f;
		float a = data.tintColor.GetA() / 255.0f;
		Gdiplus::ColorMatrix matrix = { r,0,0,0,0, 0,g,0,0,0, 0,0,b,0,0, 0,0,0,a,0, 0,0,0,0,1 };
		m_pCachedAttr->SetColorMatrix(&matrix);
		pGraphics->DrawImage(data.pBitmap, destRect, data.sourceRect.X, data.sourceRect.Y, data.sourceRect.Width, data.sourceRect.Height, data.srcUnit, m_pCachedAttr);
	}
	else {
		pGraphics->DrawImage(data.pBitmap, destRect, data.sourceRect.X, data.sourceRect.Y, data.sourceRect.Width, data.sourceRect.Height, data.srcUnit);
	}
}
