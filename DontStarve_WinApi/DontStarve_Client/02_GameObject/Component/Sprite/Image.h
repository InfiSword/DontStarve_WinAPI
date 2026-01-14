#pragma once

#include "../Component.h"
#include "Sprite.h"

class GameObject;

namespace ComponentElement 
{
	struct ImageStyle {
		RenderLayer layer;
		float sortKey;
	};

	class Image : public Component
	{
	protected:
		std::shared_ptr<Sprite> m_sprite;
		RenderLayer m_layer;
		float m_sortKey;

	public:
		Image(GameObject* owner, RenderLayer layer = LAYER_UI_BACKGROUND, float sortKey = 0.0f);
		virtual ~Image();

		virtual void Init() override;
		virtual void Release() override;

		Gdiplus::Bitmap* GetSprite() const { return m_sprite ? m_sprite->bitmap.get() : nullptr; }
		std::shared_ptr<Sprite> GetSpriteHandle() const { return m_sprite; }
		void SetSprite(const std::shared_ptr<Sprite>& sprite) { m_sprite = sprite; }

		RenderLayer GetLayer() const { return m_layer; }
		void SetLayer(RenderLayer layer) { m_layer = layer; }

		float GetSortKey() const { return m_sortKey; }
		void SetSortKey(float sortKey) { m_sortKey = sortKey; }

		void ApplyStyle(const ImageStyle& style) { m_layer = style.layer; m_sortKey = style.sortKey; }

		void LoadSprite(const std::wstring& fullPath);
	};
}
