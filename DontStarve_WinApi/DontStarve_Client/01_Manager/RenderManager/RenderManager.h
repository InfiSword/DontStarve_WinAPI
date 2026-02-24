#pragma once

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
	void Release();

	// 기본 렌더 명령 등록
	void AddDrawCommand(Gdiplus::Bitmap* pBitmap, const Gdiplus::RectF& destRect, const Gdiplus::RectF& sourceRect, Gdiplus::Unit srcUnit, const Gdiplus::PointF& objectScreenPos, RenderLayer layer, float sortKey, Direction direction, const Gdiplus::Color& tintColor = Gdiplus::Color(255, 255, 255, 255), bool hasTint = false, bool preFlipped = false);
	void AddTextCommand(const std::wstring* text, Gdiplus::Font* pFont, Gdiplus::Brush* pBrush, Gdiplus::StringFormat* pStringFormat, const Gdiplus::RectF& destRect, RenderLayer layer, float sortKey);
	void AddDrawCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float sortKey);
	void AddFillRectangleCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, RenderLayer layer, float sortKey);  // 채워진 사각형 명령
	void AddDrawEllipseCommand(const Gdiplus::RectF& rect, const Gdiplus::Color& color, float thickness, RenderLayer layer, float sortKey);  // 원/타원 외곽선

	// 렌더링 헬퍼 함수
	void RenderUIImageWithPivot(Gdiplus::Bitmap* bitmap, float x, float y, float width, float height,
		float pivotX, float pivotY,
		RenderLayer layer = LAYER_UI_BACKGROUND, float sortKey = 0.0f, const Gdiplus::Color& tintColor = Gdiplus::Color(255, 255, 255, 255), bool hasTint = false);
	void RenderGameObject(GameObject* pObject);
	void RenderTile(Gdiplus::Bitmap* pTileBitmap, float worldX, float worldY, float width, float height);

	void Clear();

	void Flush(Gdiplus::Graphics* pGraphics);

private:
	// 방향에 따른 스프라이트 반전 적용 (월드 오브젝트만)
	void ApplyDirectionFlip(Gdiplus::Graphics* pGraphics, const DrawCommand& command, float scaledWidth, float scaledHeight);

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
