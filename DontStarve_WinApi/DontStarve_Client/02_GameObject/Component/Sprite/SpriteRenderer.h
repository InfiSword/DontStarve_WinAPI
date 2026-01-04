#pragma once

#include "../Component.h"

// 월드 오브젝트의 이미지와 레이어를 관리하는 컴포넌트
class SpriteRenderer : public Component
{
protected:
	Gdiplus::Bitmap* m_sprite;		// 렌더링할 스프라이트
	RenderLayer m_layer;				// 렌더 레이어
	float m_sortKey;					// 정렬 키 (기본값은 Transform에서 계산)

public:
	SpriteRenderer(GameObject* owner, RenderLayer layer = LAYER_WORLD_OBJECT);
	virtual ~SpriteRenderer();

	virtual void Init() override;
	virtual void Release() override;

	// 스프라이트 Getter/Setter
	Gdiplus::Bitmap* GetSprite() const { return m_sprite; }
	void SetSprite(Gdiplus::Bitmap* sprite) { m_sprite = sprite; }

	// 레이어 Getter/Setter
	RenderLayer GetLayer() const { return m_layer; }
	void SetLayer(RenderLayer layer) { m_layer = layer; }

	// 정렬 키 Getter/Setter
	float GetSortKey() const { return m_sortKey; }
	void SetSortKey(float sortKey) { m_sortKey = sortKey; }

	// 스프라이트 로드 (GameObjectID와 이미지 이름 사용)
	void LoadSprite(GameObjectID id, const std::wstring& imageName);

	// 스프라이트 로드 (전체 경로 사용)
	void LoadSprite(const std::wstring& fullPath);
};

