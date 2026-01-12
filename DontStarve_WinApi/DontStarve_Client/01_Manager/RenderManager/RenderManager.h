#pragma once

#include <Struct.h>

// Forward declarations
class GameObject;

class RenderManager : public CSingleTon<RenderManager>
{
	friend class CSingleTon<RenderManager>;
public:
	RenderManager();
	~RenderManager();

	void Init();
	void LateInit();
	void Update(float deltaTime);
	void LateUpdate();
	void Render();
	void Release();

	// 기본 렌더 명령 등록
	void AddDrawCommand(Gdiplus::Bitmap* pBitmap, const Gdiplus::RectF& destRect, const Gdiplus::RectF& sourceRect, Gdiplus::Unit srcUnit, const Gdiplus::PointF& objectScreenPos, RenderLayer layer, float sortKey, Direction direction);
	void AddTextCommand(const std::wstring& text, Gdiplus::Font* pFont, Gdiplus::Brush* pBrush, Gdiplus::StringFormat* pStringFormat, const Gdiplus::RectF& destRect, RenderLayer layer, float sortKey);
	void AddDrawCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float sortKey);
	void AddFillRectangleCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, RenderLayer layer, float sortKey);  // 채워진 사각형 명령

	// UI 전용 렌더링 헬퍼
	// Pivot 포함 렌더링 헬퍼
	void RenderUIImageWithPivot(Gdiplus::Bitmap* bitmap, float x, float y, float width, float height,
		float pivotX, float pivotY,
		RenderLayer layer = LAYER_UI_BACKGROUND, float sortKey = 0.0f);

	// UI 텍스트 렌더링
	void RenderUIText(const std::wstring& text, Gdiplus::Font* font, Gdiplus::Brush* brush,
		float x, float y, float width, float height,
		RenderLayer layer = LAYER_UI_FOREGROUND, float sortKey = 0.0f);

	// 일반 GameObject 렌더링
	// GameObject 정보를 바탕으로 렌더 명령 큐에 추가
	void RenderGameObject(GameObject* pObject);

	// 타일 렌더링
	void RenderTile(Gdiplus::Bitmap* pTileBitmap, float worldX, float worldY, float width, float height);

	// 카메라에 보이는 객체만 렌더
	void RenderVisibleGameObjects();

	void Clear();

	void ApplyGdiTransform(Gdiplus::Graphics* pGraphics, const DrawCommand& command, float scaledWidth, float scaledHeight);

	void Flush(Gdiplus::Graphics* pGraphics);

private:
	std::vector<DrawCommand> m_drawCommands;

	// 정렬에서 레이어를 우선 비교하고 그 다음 sortKey 비교
	static bool CompareDrawCommands(const DrawCommand& a, const DrawCommand& b)
	{
		if (a.layer != b.layer) {
			return a.layer < b.layer;
		}
		return a.sortKey < b.sortKey;
	}
};
