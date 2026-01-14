#pragma once

#include "../Component.h"

// 월드 오브젝트의 이미지와 레이어를 관리하는 컴포넌트
class SpriteRenderer : public Component
{
protected:
	Gdiplus::Bitmap* m_sprite;		    // 렌더링할 스프라이트 (외부에서 lifetime 관리)
	RenderLayer m_layer;				// 렌더 레이어
	float m_sortKey;					// 정렬 키 (기본값은 Transform에서 계산)
	Gdiplus::RectF m_sourceRect;		// 현재 사용할 소스 영역 (애니메이션 프레임 or 전체 비트맵)
	float m_pivotX;						// 스프라이트 기준 피벗 X (0.0 ~ 1.0)
	float m_pivotY;						// 스프라이트 기준 피벗 Y (0.0 ~ 1.0)

public:
	SpriteRenderer(GameObject* owner, RenderLayer layer = LAYER_WORLD_OBJECT);
	virtual ~SpriteRenderer();

	virtual void Init() override;
	virtual void Release() override;

	Gdiplus::Bitmap* GetSprite() const { return m_sprite; }
	void SetSprite(Gdiplus::Bitmap* sprite, const Gdiplus::RectF& sourceRect, float pivotX, float pivotY);

	// 레이어 Getter/Setter
	RenderLayer GetLayer() const { return m_layer; }
	void SetLayer(RenderLayer layer) { m_layer = layer; }

	// 정렬 키 Getter/Setter
	float GetSortKey() const { return m_sortKey; }
	void SetSortKey(float sortKey) { m_sortKey = sortKey; }

	// 현재 사용 중인 소스 영역 / 피벗 조회
	const Gdiplus::RectF& GetSourceRect() const { return m_sourceRect; }
	float GetPivotX() const { return m_pivotX; }
	float GetPivotY() const { return m_pivotY; }
};

