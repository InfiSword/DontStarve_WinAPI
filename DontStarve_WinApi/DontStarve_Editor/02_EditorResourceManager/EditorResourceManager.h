#pragma once

#include <map>
#include <memory>
#include "Struct.h"

class EditorResourceManager
{
public:
	EditorResourceManager() = default;
	~EditorResourceManager() = default;

	void LoadResources();
	void ReleaseResources();

	const ResourcePathUtils::TileResourceDef* GetTileVariant(TileType type, TileID id) const;
	const ResourcePathUtils::ObjectResourceDef* GetObjectVariant(GameObjectType type, GameObjectID id) const;

	const std::map<TileType, std::map<TileID, ResourcePathUtils::TileResourceDef>>& GetTileVariants() const { return m_tileVariants; }
	const std::map<GameObjectType, std::map<GameObjectID, ResourcePathUtils::ObjectResourceDef>>& GetObjectVariants() const { return m_objectVariants; }

private:
	std::map<TileType, std::map<TileID, ResourcePathUtils::TileResourceDef>> m_tileVariants;
	std::map<GameObjectType, std::map<GameObjectID, ResourcePathUtils::ObjectResourceDef>> m_objectVariants;
};
