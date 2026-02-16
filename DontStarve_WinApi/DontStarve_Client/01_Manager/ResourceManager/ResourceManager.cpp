#include "99_Default/pch.h"
#include "ResourceManager.h"


ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
	Release();
}

void ResourceManager::Init()
{
	// 오브젝트 리소스 등록 (Function.h의 ResourcePathUtils에서 직접 가져와서 캐싱)
	size_t defCount;
	const ResourcePathUtils::ObjectResourceDef* defs = ResourcePathUtils::GetObjectResourceDefs(defCount);
	
	for (size_t i = 0; i < defCount; ++i) {
		const auto& def = defs[i];
		RegisterObjectResource(def.id, def);
	}
}

void ResourceManager::Release()
{
	m_objectResources.clear();
	m_mapDataCache.clear();
	m_spriteCache.clear();
}

void ResourceManager::RegisterObjectResource(GameObjectID id, const ResourcePathUtils::ObjectResourceDef& data)
{
	m_objectResources[id] = data;
}

const ResourcePathUtils::ObjectResourceDef* ResourceManager::GetObjectResourceInfo(GameObjectID id) const
{
	auto it = m_objectResources.find(id);
	if (it != m_objectResources.end()) {
		return &(it->second);
	}
	return nullptr;
}

const MapData* ResourceManager::LoadMapData(const std::wstring& mapFileName)
{
	auto it = m_mapDataCache.find(mapFileName);
	if (it != m_mapDataCache.end()) {
		return &it->second;
	}

	MapData mapData;
	ParseMapFileInto(mapFileName, mapData);
	m_mapDataCache[mapFileName] = std::move(mapData);
	return &m_mapDataCache[mapFileName];
}

void ResourceManager::ParseMapFileInto(const std::wstring& mapFileName, MapData& outMapData)
{
	// Function.h의 공통 파싱 함수 사용
	auto getObjectResourceInfo = [this](GameObjectID id) -> const ResourcePathUtils::ObjectResourceDef* {
		return this->GetObjectResourceInfo(id);
	};
	
	ResourcePathUtils::ParseMapFileInto(mapFileName, outMapData, getObjectResourceInfo);
}

std::wstring ResourceManager::BuildResourcePath(const std::wstring& basePath, const std::wstring& subFolder, const std::wstring& filename) const
{
	return ResourcePathUtils::BuildResourcePath(basePath, subFolder, filename);
}

std::shared_ptr<Sprite> ResourceManager::LoadSprite(const std::wstring& fullPath)
{
	if (fullPath.empty()) return nullptr;

	auto found = m_spriteCache.find(fullPath);
	if (found != m_spriteCache.end()) {
		if (std::shared_ptr<Sprite> cached = found->second.lock()) {
			return cached;
		}
	}

	std::shared_ptr<Gdiplus::Bitmap> bmp = std::make_shared<Gdiplus::Bitmap>(fullPath.c_str());
	if (!bmp || bmp->GetLastStatus() != Gdiplus::Ok) {
		return nullptr;
	}
	Gdiplus::RectF src(0, 0, static_cast<float>(bmp->GetWidth()), static_cast<float>(bmp->GetHeight()));
	std::shared_ptr<Sprite> sprite = std::make_shared<Sprite>(bmp, src, 0.5f, 0.5f, fullPath);
	m_spriteCache[fullPath] = sprite;
	return sprite;
}
