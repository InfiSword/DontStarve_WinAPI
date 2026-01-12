#include "99_Default/pch.h"
#include "DontStarve_MainGame.h"

// Manager includes
#include "../01_Manager/TimeManager/TimeManager.h"
#include "../01_Manager/CameraManager/CameraManager.h"
#include "../01_Manager/InputManager/InputManager.h"
#include "../01_Manager/GraphicsManager/GraphicsManager.h"
#include "../01_Manager/RenderManager/RenderManager.h"
#include "../01_Manager/ObjectManager/ObjectManager.h"
#include "../01_Manager/ColliderManager/ColliderManager.h"
#include "../01_Manager/InventoryManager/InventoryManager.h"
#include "../01_Manager/UIManager/UIManager.h"
#include "../01_Manager/SceneManager/SceneManager.h"
#include "../01_Manager/ResourceManager/ResourceManager.h"

DontStarve_MainGame::DontStarve_MainGame()
    : m_bIsInitialized(false)
{
}

DontStarve_MainGame::~DontStarve_MainGame()
{
    Release();
}

void DontStarve_MainGame::Init()
{
    if (m_bIsInitialized)
        return;

    // �⺻ �ý��� �Ŵ����鸸 �ʱ�ȭ (���� ������ �ý���)
    TimeManager::GetInstance()->Init();
    TimeManager::GetInstance()->SetFPS(20);
    
    GraphicsManager::GetInstance()->Init();
    RenderManager::GetInstance()->Init();
    ResourceManager::GetInstance()->Init(); // ���ҽ��� ���� �����ϰ� ����
    
    // SceneManager �ʱ�ȭ (ù ��° �� �ε�)
    SceneManager::GetInstance()->Init();
     
    m_bIsInitialized = true;
}

void DontStarve_MainGame::LateInit()
{
    if (!m_bIsInitialized)
        return;
        
}

void DontStarve_MainGame::Update()
{
    if (!m_bIsInitialized)
        return;

    // TimeManager ������Ʈ (�⺻ �ð� ����)
    TimeManager::GetInstance()->Update();
    float deltaTime = TimeManager::GetInstance()->GetDeltaTime(); // �̹� ���ѵ� deltaTime
    
    // SceneManager�� ���� �� ������Ʈ
    SceneManager::GetInstance()->Update(deltaTime);
    
    // ������ ���� ���� (FPS ��� ����)
    TimeManager::GetInstance()->UpdateFrameLimit();
}

void DontStarve_MainGame::LateUpdate()
{
    if (!m_bIsInitialized)
        return;

    // SceneManager�� ���� �� LateUpdate
    SceneManager::GetInstance()->LateUpdate();
}

void DontStarve_MainGame::Render()
{
    if (!m_bIsInitialized)
        return;

    // Graphics ��ü ���
    Gdiplus::Graphics* pGraphics = GraphicsManager::GetInstance()->GetGraphics();
    if (!pGraphics) return;

    // SceneManager�� ���� �� ������
    SceneManager::GetInstance()->Render();

    // RenderManager���� ���� ������ ����
    RenderManager::GetInstance()->Flush(pGraphics);

    // ����ۿ� �׷��� ������ ȭ�鿡 ǥ��
    GraphicsManager::GetInstance()->Render();
}

void DontStarve_MainGame::Release()
{
    if (!m_bIsInitialized)
        return;

    // SceneManager ���� (���� �Ŵ������� SceneManager���� ������)
    SceneManager::GetInstance()->Release();
    
    // �⺻ �ý��� �Ŵ����� ����
    ResourceManager::DestroyInstance();
    RenderManager::DestroyInstance();
    GraphicsManager::DestroyInstance();
    TimeManager::DestroyInstance();
    
    m_bIsInitialized = false;
}

void DontStarve_MainGame::InitializeManagers()
{
    // �� �Լ��� �� �̻� ������� ����
    // �Ŵ��� �ʱ�ȭ�� �� ������ ���
}

