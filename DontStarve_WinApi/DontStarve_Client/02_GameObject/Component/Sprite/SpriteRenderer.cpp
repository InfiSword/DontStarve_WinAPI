#include "99_Default/pch.h"
#include "SpriteRenderer.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"
#include "../../../02_GameObject/GameObject.h"
#include "../../../02_GameObject/Component/Transform/Transform.h"

SpriteRenderer::SpriteRenderer(GameObject* owner, RenderLayer layer)
	: Component(owner), m_sprite(nullptr), m_layer(layer), m_sortKey(0.0f), m_pCachedTransform(nullptr)
{
}

SpriteRenderer::~SpriteRenderer()
{
	Release();
}

void SpriteRenderer::Init()
{
	m_pCachedTransform = m_owner->GetComponent<Transform>();
}

void SpriteRenderer::Release()
{
	m_sprite.reset();
}
