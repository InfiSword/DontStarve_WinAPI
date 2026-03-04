#include "99_Default/pch.h"
#include "Monster.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../02_GameObject/Component/Transform/Transform.h"
#include "../../../02_GameObject/Entity/Player/Player.h"

Monster::Monster(GameObjectID id, float x, float y, float pivotX, float pivotY, 
                 const std::wstring& baseDir, const std::wstring& imageName)
    : Entity(id, x, y, pivotX, pivotY, DIR_DOWN, baseDir, imageName, true, true),
      m_aggroTarget(nullptr), m_attackCooldownTimer(0.0f), m_walkSpeed(0.0f), m_runSpeed(0.0f),
      m_targetX(x), m_targetY(y), m_distToPlayerSq(1e10f), m_dirToPlayer({0, 0}),
      m_aiTickTimer(0.0f)
{
    // AI 연산 부하 분산을 위해 개체별로 0.1s ~ 0.2s 사이의 고유 틱 간격 부여
    m_aiTickInterval = 0.1f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.1f));
}

Monster::~Monster()
{
}

void Monster::Init()
{
    Entity::Init();
}

void Monster::Update(float deltaTime)
{
    if (m_isDead) return;

    // --- 1. [매 프레임] 플레이어와의 정보 계산 (제곱 거리 사용) ---
    Player* player = ObjectManager::GetInstance()->GetPlayer();
    if (player)
    {
        Transform* playerTr = player->GetComponent<Transform>();
        if (playerTr && transform)
        {
            float dx = playerTr->GetX() - transform->GetX();
            float dy = playerTr->GetY() - transform->GetY();
            m_distToPlayerSq = dx * dx + dy * dy; // sqrtf 제거 (최적화)

            // 방향 벡터는 이동에 필요하므로 매 프레임 정규화 (최소한의 sqrt 허용)
            if (m_distToPlayerSq > 0.0001f)
            {
                float invDist = 1.0f / sqrtf(m_distToPlayerSq);
                m_dirToPlayer.X = dx * invDist;
                m_dirToPlayer.Y = dy * invDist;
            }
        }
    }

    // --- 2. [AI Tick] 일정 간격으로만 AI 상태 결정 수행 ---
    m_aiTickTimer += deltaTime;
    if (m_aiTickTimer >= m_aiTickInterval)
    {
        UpdateAI(m_aiTickTimer); // 누적된 시간 전달
        m_aiTickTimer = 0.0f;
    }

    // --- 3. [매 프레임] 이동 및 쿨타임, 애니메이션 처리 ---
    if (m_attackCooldownTimer > 0.0f)
        m_attackCooldownTimer -= deltaTime;

    UpdateMovement(deltaTime); // 부드러운 이동
    Entity::Update(deltaTime); // 애니메이션 업데이트
}
