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
	// 오브젝트 리소스 등록 (Function.h의 정적 테이블에서 직접 가져와서 캐싱)
	for (size_t i = 0; i < ResourcePathUtils::ObjectResourceCount; ++i) {
		const auto& entry = ResourcePathUtils::ObjectResourceTable[i];
		ResourcePathUtils::ObjectResourceDef def;
		def.type = entry.type;
		def.id = entry.id;
		def.baseDir = entry.baseDir;
		def.imageName = entry.imageName;
		def.pivotX = entry.pivotX;
		def.pivotY = entry.pivotY;
		RegisterObjectResource(entry.id, def);
	}
}

void ResourceManager::Release()
{
	// ObjectResourceDef 내부의 std::wstring 멤버들 명시적 정리
	for (auto& pair : m_objectResources) {
		ResourcePathUtils::ObjectResourceDef& def = pair.second;
		def.baseDir.clear();
		def.baseDir.shrink_to_fit();
		def.imageName.clear();
		def.imageName.shrink_to_fit();
	}
	m_objectResources.clear();
	
	// weak_ptr은 참조 카운트를 증가시키지 않으므로 Sprite는 자동 해제됨
	// 맵 자체의 메모리 해제를 위해 빈 맵으로 교체
	m_spriteCache = std::unordered_map<std::wstring, std::weak_ptr<Sprite>>();
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
