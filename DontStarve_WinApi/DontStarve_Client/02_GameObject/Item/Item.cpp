#include "../../99_Default/pch.h"
#include "Item.h"
#include "../../01_Manager/InventoryManager/InventoryManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"

Item::Item(GameObjectType type, GameObjectID id, const std::wstring& name, const std::wstring& desc, const std::wstring resourcePath, const std::wstring& imagePath)
    : GameObject(type, id, 0, 0, 0.5f, 0.5f, DIR_DOWN, resourcePath, imagePath)
{
    m_name = name;
    m_description = desc;
    m_bitmap=nullptr;

    LoadBitmap();

    Gdiplus::Bitmap* itemBmp = GetBitmap();
    if (itemBmp) {
        this->m_width = (float)itemBmp->GetWidth();
        this->m_height = (float)itemBmp->GetHeight();
    }
}

Item::~Item() 
{
  
}

void Item::LoadBitmap()
{

    if (resourcePath.empty() || imageName.empty()) {
        OutputDebugStringW((L"Item: LoadBitmap 실패 - 경로나 이미지명이 비어있음 (ID: " + std::to_wstring(m_id) + L")\n").c_str());
        m_bitmap = nullptr;
        return;
    }
    
    // ResourceManager를 사용하여 경로 구성
    auto* pRM = ResourceManager::GetInstance();
    std::wstring fullPath = pRM->BuildObjectResourcePath(m_id, L"", imageName);
    
    OutputDebugStringW((L"Item: LoadBitmap - 전체 경로: " + fullPath + L"\n").c_str());
    
    // 비트맵 로드
    m_bitmap = new Gdiplus::Bitmap(fullPath.c_str());
    if (m_bitmap && m_bitmap->GetLastStatus() != Gdiplus::Ok) {
        OutputDebugStringW((L"Item: LoadBitmap 실패 - 비트맵 상태 오류 (ID: " + std::to_wstring(m_id) + L")\n").c_str());
        delete m_bitmap;
        m_bitmap = nullptr;
    } else if (m_bitmap)
    {
        OutputDebugStringW((L"Item: LoadBitmap 성공 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
    } else {
        OutputDebugStringW((L"Item: LoadBitmap 실패 - 비트맵 생성 실패 (ID: " + std::to_wstring(m_id) + L")\n").c_str());
    }
}


