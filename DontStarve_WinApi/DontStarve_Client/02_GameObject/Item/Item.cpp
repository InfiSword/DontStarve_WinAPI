#include "../../99_Default/pch.h"
#include "Item.h"
#include "../../01_Manager/InventoryManager/InventoryManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"

Item::Item(GameObjectType type, GameObjectID id, const std::wstring& name, const std::wstring& desc,
	const std::wstring resourcePath, const std::wstring& imagePath,
	float x, float y, float pivotX, float pivotY, Direction dir, bool isActive, bool isInteractive)
    : GameObject(type, id, x, y, pivotX, pivotY, dir, resourcePath, imagePath, isActive, isInteractive)
{
    m_name = name;
    m_description = desc;
    m_orignalBitmap=nullptr;

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

    if (m_resourcePath.empty() || m_imageName.empty()) {
        OutputDebugStringW((L"Item: LoadBitmap ���� - ��γ� �̹������� ������� (ID: " + std::to_wstring(m_id) + L")\n").c_str());
        m_orignalBitmap = nullptr;
        return;
    }
    
    // ResourceManager�� ����Ͽ� ��� ����
    auto* pRM = ResourceManager::GetInstance();
    std::wstring fullPath = pRM->BuildObjectResourcePath(m_id, L"", m_imageName);
    
    OutputDebugStringW((L"Item: LoadBitmap - ��ü ���: " + fullPath + L"\n").c_str());
    
    // ��Ʈ�� �ε�
    m_orignalBitmap = new Gdiplus::Bitmap(fullPath.c_str());
    if (m_orignalBitmap && m_orignalBitmap->GetLastStatus() != Gdiplus::Ok) {
        OutputDebugStringW((L"Item: LoadBitmap ���� - ��Ʈ�� ���� ���� (ID: " + std::to_wstring(m_id) + L")\n").c_str());
        delete m_orignalBitmap;
        m_orignalBitmap = nullptr;
    } else if (m_orignalBitmap)
    {
        OutputDebugStringW((L"Item: LoadBitmap ���� - ID: " + std::to_wstring(m_id) + L"\n").c_str());
    } else {
        OutputDebugStringW((L"Item: LoadBitmap ���� - ��Ʈ�� ���� ���� (ID: " + std::to_wstring(m_id) + L")\n").c_str());
    }
}


