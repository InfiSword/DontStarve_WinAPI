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
	m_cameraPos = CameraManager::GetInstance()->GetCameraPos();
}

void RenderManager::LateUpdate()
{
}

void RenderManager::Release()
{
	Clear();

	Utils::SafeDelete(m_pCachedPen);
	Utils::SafeDelete(m_pCachedBrush);
	Utils::SafeDelete(m_pCachedAttr);
}

void RenderManager::AddWorldEntityCommand(Gdiplus::Bitmap* pBitmap, const Gdiplus::RectF& sourceRect, float worldX, float worldY, float scaleX, float scaleY, float pivotX, float pivotY, RenderLayer layer, float zOrder, Direction direction, const Gdiplus::Color& tintColor, bool hasTint, bool preFlipped, float rotation)
{
	// 월드 좌표를 화면 좌표로 변환 (RenderManager가 캐싱된 카메라 좌표를 사용하여 직접 계산)
	float screenX = worldX - m_cameraPos.X + (float)WINCX * 0.5f;
	float screenY = worldY - m_cameraPos.Y + (float)WINCY * 0.5f;
	Gdiplus::PointF screenPos(screenX, screenY);

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

	if (!m_bUseRenderQueue && m_pDirectGraphics) {
		ExecuteCommand(m_pDirectGraphics, cmd);
		return;
	}
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

	if (!m_bUseRenderQueue && m_pDirectGraphics) {
		ExecuteCommand(m_pDirectGraphics, cmd);
		return;
	}
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

	if (!m_bUseRenderQueue && m_pDirectGraphics) {
		ExecuteCommand(m_pDirectGraphics, cmd);
		return;
	}
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

	if (!m_bUseRenderQueue && m_pDirectGraphics) {
		ExecuteCommand(m_pDirectGraphics, cmd);
		return;
	}
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

	if (!m_bUseRenderQueue && m_pDirectGraphics) {
		ExecuteCommand(m_pDirectGraphics, cmd);
		return;
	}
	m_layerCommands[layer].push_back(cmd);
}

void RenderManager::Clear()
{
	for (int i = 0; i < LAYER_COUNT; ++i) {
		m_layerCommands[i].clear();
	}
}

void RenderManager::BeginFrame(Gdiplus::Graphics* pGraphics)
{
	m_pDirectGraphics = pGraphics;
}

void RenderManager::ExecuteCommand(Gdiplus::Graphics* pGraphics, const DrawCommand& cmd)
{
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

void RenderManager::Flush(Gdiplus::Graphics* pGraphics)
{
	if (!pGraphics) return;

	if (!m_bUseRenderQueue) {
		// 비최적화(일반 렌더) 모드: AddXXXCommand에서 이미 즉시 그렸으므로 큐만 초기화
		Clear();
		m_pDirectGraphics = nullptr;
		return;
	}

	for (int i = LAYER_TILE_BACKGROUND; i < LAYER_COUNT; ++i) {
		if (m_layerCommands[i].empty()) continue;
		
		std::stable_sort(m_layerCommands[i].begin(), m_layerCommands[i].end(), CompareDrawCommands);
		
		for (const auto& cmd : m_layerCommands[i]) {
			ExecuteCommand(pGraphics, cmd);
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
