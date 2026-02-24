#include "99_Default/pch.h"
#include "ResourceManager.h"
#include <fstream>

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
		RegisterObjectResource(entry.id, def);
	}

	// GameData/object_resource_overrides.txt 로드 후 id별 pivot/콜라이더 덮어쓰기 (Function.h 공통 파서 사용)
	ResourcePathUtils::ParseObjectResourceOverridesFile(L"GameData/object_resource_overrides.txt",
		[this](GameObjectType, GameObjectID id, const ResourcePathUtils::ObjectResourceDef& overrideDef) {
			auto it = m_objectResources.find(id);
			if (it != m_objectResources.end()) {
				it->second.pivotX = overrideDef.pivotX;
				it->second.pivotY = overrideDef.pivotY;
				it->second.hasCollider = overrideDef.hasCollider;
				it->second.colliderType = overrideDef.colliderType;
				it->second.colliderOffsetX = overrideDef.colliderOffsetX;
				it->second.colliderOffsetY = overrideDef.colliderOffsetY;
				it->second.colliderWidth = overrideDef.colliderWidth;
				it->second.colliderHeight = overrideDef.colliderHeight;
				it->second.colliderCenterX = overrideDef.colliderCenterX;
				it->second.colliderCenterY = overrideDef.colliderCenterY;
				it->second.colliderRadius = overrideDef.colliderRadius;
			}
		});
}

void ResourceManager::Release()
{
	m_objectResources.clear();
	m_spriteCache.clear();
	m_spriteSheetCache.clear();
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
		// 만료된 weak_ptr 엔트리 제거
		m_spriteCache.erase(found);
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

std::shared_ptr<SpriteSheet> ResourceManager::LoadSpriteSheet(
    const std::wstring& imagePath,
    UINT frameWidth, UINT frameHeight,
    UINT framesPerRow, UINT totalFrames,
    bool flipHorizontal)
{
    std::wstring key = imagePath
        + L"_" + std::to_wstring(frameWidth)
        + L"x" + std::to_wstring(frameHeight)
        + L"_" + std::to_wstring(framesPerRow)
        + L"x" + std::to_wstring(totalFrames)
        + (flipHorizontal ? L"_flip" : L"");

    auto it = m_spriteSheetCache.find(key);
    if (it != m_spriteSheetCache.end()) {
        if (auto cached = it->second.lock()) {
            return cached;
        }
        // 만료된 weak_ptr 엔트리 제거
        m_spriteSheetCache.erase(it);
    }

    auto sheet = SpriteSheet::CreateFromFile(imagePath, frameWidth, frameHeight, framesPerRow, totalFrames, flipHorizontal);
    if (!sheet) return nullptr;

    std::shared_ptr<SpriteSheet> shared = std::move(sheet);
    m_spriteSheetCache[key] = shared;
    return shared;
}
