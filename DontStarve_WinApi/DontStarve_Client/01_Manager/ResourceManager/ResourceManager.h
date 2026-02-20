#pragma once

#include "../../02_GameObject/Component/Sprite/Sprite.h"
#include "../../03_Animation/SpriteSheet.h"

class ResourceManager : public CSingleTon<ResourceManager>
{
    friend class CSingleTon<ResourceManager>;
private:
    ResourceManager();
    ~ResourceManager();

public:
    void Init();
    void Release();

    const ResourcePathUtils::ObjectResourceDef* GetObjectResourceInfo(GameObjectID id) const;
    const std::map<GameObjectID, ResourcePathUtils::ObjectResourceDef>& GetAllObjectResources() const { return m_objectResources; }

    std::shared_ptr<Sprite> LoadSprite(const std::wstring& fullPath);

    // SpriteSheet 캐시 로드 - 동일 경로+반전 조합은 공유 포인터 반환
    std::shared_ptr<SpriteSheet> LoadSpriteSheet(
        const std::wstring& imagePath,
        UINT frameWidth, UINT frameHeight,
        UINT framesPerRow, UINT totalFrames,
        bool flipHorizontal = false);

private:
    void RegisterObjectResource(GameObjectID id, const ResourcePathUtils::ObjectResourceDef& data);

    std::map<GameObjectID, ResourcePathUtils::ObjectResourceDef> m_objectResources;
    std::unordered_map<std::wstring, std::weak_ptr<Sprite>> m_spriteCache;
    std::unordered_map<std::wstring, std::weak_ptr<SpriteSheet>> m_spriteSheetCache;
};
