#include "../pch.h"
#include "EditorResourceManager.h"
#include "../../Header/Function.h"
#include <sstream>

void EditorResourceManager::LoadResources()
{
	ReleaseResources();


	OutputDebugStringW(L"=== Loading resources from code ===\n");

	// 정적 테이블에서 직접 로드
	for (size_t i = 0; i < ResourcePathUtils::TileResourceCount; ++i) {
		const auto& def = ResourcePathUtils::TileResourceTable[i];
		// 절대경로로 직접 저장 (아틀라스 사용 안 함)
		m_tileVariants[def.type][def.id] = ResourcePathUtils::TileResourceDef(def.type, def.id, def.baseDir, def.imageName);
	}

	// 정적 테이블에서 직접 로드
	for (size_t i = 0; i < ResourcePathUtils::ObjectResourceCount; ++i) {
		const auto& def = ResourcePathUtils::ObjectResourceTable[i];
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
	ClearBitmapCache();
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

std::shared_ptr<Gdiplus::Bitmap> EditorResourceManager::GetCachedBitmap(const std::wstring& fullPath)
{
	// 캐시 조회
	auto it = m_bitmapCache.find(fullPath);
	if (it != m_bitmapCache.end()) {
		return it->second;  // 캐시 히트
	}

	// 캐시 미스 → 로드 후 캐싱
	Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromFile(fullPath.c_str());
	if (pBitmap && pBitmap->GetLastStatus() == Gdiplus::Ok) {
		auto sharedBitmap = std::shared_ptr<Gdiplus::Bitmap>(pBitmap);
		m_bitmapCache[fullPath] = sharedBitmap;
		return sharedBitmap;
	}
	return nullptr;
}

void EditorResourceManager::ClearBitmapCache()
{
	m_bitmapCache.clear();
}
