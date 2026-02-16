#include "../pch.h"
#include "EditorResourceManager.h"
#include "../../Header/Function.h"
#include <sstream>

void EditorResourceManager::LoadResources()
{
	ReleaseResources();


	OutputDebugStringW(L"=== Loading resources from code ===\n");

	size_t tileDefCount;
	const ResourcePathUtils::TileResourceDef* tileDefs = ResourcePathUtils::GetTileResourceDefs(tileDefCount);

	for (size_t i = 0; i < tileDefCount; ++i) {
		const auto& def = tileDefs[i];
		// 절대경로로 직접 저장 (아틀라스 사용 안 함)
		m_tileVariants[def.type][def.id] = ResourcePathUtils::TileResourceDef(def.type, def.id, def.baseDir, def.imageName);
	}

	size_t objectDefCount;
	const ResourcePathUtils::ObjectResourceDef* objectDefs = ResourcePathUtils::GetObjectResourceDefs(objectDefCount);

	for (size_t i = 0; i < objectDefCount; ++i) {
		const auto& def = objectDefs[i];
		// 절대경로로 직접 저장 (아틀라스 사용 안 함)
		m_objectVariants[def.type][def.id] = ResourcePathUtils::ObjectResourceDef(def.type, def.id, 0, 0, def.baseDir, def.imageName, def.pivotX, def.pivotY);
	}

	OutputDebugStringW((L"Final Tile Variants Map Size: " + std::to_wstring(m_tileVariants.size()) + L"\n").c_str());
	OutputDebugStringW((L"Final Object Variants Map Size: " + std::to_wstring(m_objectVariants.size()) + L"\n").c_str());
}

void EditorResourceManager::ReleaseResources()
{
	m_tileVariants.clear();
	m_objectVariants.clear();
}

const ResourcePathUtils::TileResourceDef* EditorResourceManager::GetTileVariant(TileType type, TileID id) const
{
	auto type_it = m_tileVariants.find(type);
	if (type_it != m_tileVariants.end()) {
		auto id_it = type_it->second.find(id);
		if (id_it != type_it->second.end()) {
			return &(id_it->second);
		}
	}
	return nullptr;
}

const ResourcePathUtils::ObjectResourceDef* EditorResourceManager::GetObjectVariant(GameObjectType type, GameObjectID id) const
{
	auto type_it = m_objectVariants.find(type);
	if (type_it != m_objectVariants.end()) {
		auto id_it = type_it->second.find(id);
		if (id_it != type_it->second.end()) {
			return &(id_it->second);
		}
	}
	return nullptr;
}
