#include "../99_Default/pch.h"
#include "../01_Manager/RenderManager/RenderManager.h"
#include "Animator.h"
#include "AnimationClip.h"
#include "SpriteSheet.h"  

Animator::Animator(GameObject* owner)
    : Component(owner), m_currentClip(nullptr), m_currentState(-1), m_currentDirection(-1), 
      m_elapsed(0.0f), m_isPlaying(false), m_lastTriggeredFrame(-1) {}

Animator::~Animator() { 
    m_animations.clear(); // unique_ptr이므로 소멸 시 자동으로 정리됨
    m_currentClip = nullptr; 
}

void Animator::Init() {
    Component::Init();
    
    // 애니메이션 등록은 Entity::Init()에서 직접 처리하도록 변경
    // GetComponent를 통한 로직을 건너뛰기 위해 여기서는 기본 초기화만 수행
}

// 애니메이션 등록 구현
void Animator::RegisterAnimation(int state, Direction dir, 
                                const std::wstring& imagePath,
                                UINT frameWidth, UINT frameHeight,
                                UINT framesPerRow, UINT totalFrames,
                                float frameDuration,
                                float pivotX, float pivotY,
                                bool loop,
                                const std::map<int, std::wstring>& events) 
{
    OutputDebugStringW((L"Animator: RegisterAnimation 시작 - State: " + std::to_wstring(state) + 
                       L", Direction: " + std::to_wstring(static_cast<int>(dir)) + L"\n").c_str());
    
    int key = GetAnimationKey(state, static_cast<int>(dir));
    
    OutputDebugStringW((L"Animator: 애니메이션 키 생성 - Key: " + std::to_wstring(key) + L"\n").c_str());
    
    // SpriteSheet를 먼저 생성하고, 생성자로 Clip을 초기화
    auto spriteSheet = SpriteSheet::CreateFromFile(
        imagePath,
        frameWidth,
        frameHeight,
        framesPerRow,
        totalFrames
    );

    if (!spriteSheet) {
        OutputDebugStringW((L"Animator: SpriteSheet 생성 실패 - " + imagePath + L"\n").c_str());
        return;
    }

    auto clip = std::make_unique<AnimationClip>(
        L"", // 이름은 현재 사용하지 않으므로 빈 문자열 전달
        std::move(spriteSheet),
        frameDuration,
        pivotX,
        pivotY,
        loop
    );
    
    if (clip) {
        // 이벤트 프레임 등록
        for (const auto& eventPair : events) {
            clip->AddEventFrame(eventPair.first, eventPair.second);
        }
        
        // 전역 이벤트 콜백 설정
        if (m_globalEventCallback) {
            clip->SetEventCallback(m_globalEventCallback);
        }
        
        m_animations[key] = std::move(clip);
        OutputDebugStringW((L"Animator: 애니메이션 등록 완료 - Key: " + std::to_wstring(key) + L"\n").c_str());
    } else {
        OutputDebugStringW(L"Animator: AnimationClip 생성 실패\n");
    }
}

// SetState 구현
void Animator::SetState(int state, Direction direction) {
    int newDirection = static_cast<int>(direction);
    
    if (m_currentState != state || m_currentDirection != newDirection) {
        m_currentState = state;
        m_currentDirection = newDirection;
        SelectAndPlayAnimation();
    }
}

void Animator::SelectAndPlayAnimation() {
    int key = GetAnimationKey(m_currentState, m_currentDirection);
    OutputDebugStringW((L"Animator: SelectAndPlayAnimation - State: " + std::to_wstring(m_currentState) + 
                       L", Direction: " + std::to_wstring(m_currentDirection) + L", Key: " + std::to_wstring(key) + L"\n").c_str());
    
    auto it = m_animations.find(key);
    
    if (it != m_animations.end()) {
        AnimationClip* newClip = it->second.get();
        if (m_currentClip != newClip) {
            m_currentClip = newClip;
            m_elapsed = 0.0f;
            m_isPlaying = true;
            m_lastTriggeredFrame = -1;
            
            OutputDebugStringW((L"Animator: 애니메이션 재생 - State: " + 
                               std::to_wstring(m_currentState) + L", Direction: " + 
                               std::to_wstring(m_currentDirection) + L"\n").c_str());
        }
    } else {
        OutputDebugStringW((L"Animator: 애니메이션을 찾을 수 없음 - State: " + 
                           std::to_wstring(m_currentState) + L", Direction: " + 
                           std::to_wstring(m_currentDirection) + L", Key: " + std::to_wstring(key) + L"\n").c_str());
        
        // 등록된 애니메이션 목록 출력
        OutputDebugStringW(L"Animator: 등록된 애니메이션 목록:\n");
        for (const auto& pair : m_animations) {
            OutputDebugStringW((L"  - Key: " + std::to_wstring(pair.first) + L"\n").c_str());
        }
    }
}

void Animator::Update(float deltaTime)
{
    if (m_isPlaying && m_currentClip)
	{
        // 이전 프레임 인덱스 저장 (이벤트 트리거용)
        int prevFrameIndex = GetCurrentFrameIndex();
        
        // 경과 시간 누적
        m_elapsed += deltaTime;
        
        // 루프가 아닌 애니메이션의 종료 체크
        if (!m_currentClip->IsLooping() && m_elapsed >= m_currentClip->GetTotalDuration()) {
            m_elapsed = m_currentClip->GetTotalDuration();
            m_isPlaying = false;
        }

        // 현재 프레임 인덱스 계산
        int currentFrameIndex = GetCurrentFrameIndex();
        
        // 프레임이 변경되었고 이벤트가 있는 경우 트리거
        if (currentFrameIndex != -1 && currentFrameIndex != m_lastTriggeredFrame)
		{
            auto it = m_currentClip->GetEventFrames().find(currentFrameIndex);
            if (it != m_currentClip->GetEventFrames().end()) {
                if (m_currentClip->GetEventCallback()) {
                    m_currentClip->GetEventCallback()(currentFrameIndex, it->second);
                    m_lastTriggeredFrame = currentFrameIndex;
                }
            }
        }       
    }
}

void Animator::Draw(Gdiplus::Graphics* pGraphics, const Gdiplus::PointF& characterFootCenterScreenPos, 
                   float zoomFactor, Direction currentDir, RenderLayer layer, float sortKey)
{
    if (!m_currentClip) {
        return;
    }

    const AnimationFrame& currentFrame = GetCurrentFrame();
    const SpriteSheet* currentSheet = m_currentClip->GetSpriteSheet();

    if (!currentSheet || !currentSheet->GetBitmap()) {
        return;
    }

    float scaledWidth = currentFrame.width * zoomFactor; 
    float scaledHeight = currentFrame.height * zoomFactor;

    float finalRenderTopLeftScreenX = characterFootCenterScreenPos.X - (currentFrame.pivotX * scaledWidth);
    float finalRenderTopLeftScreenY = characterFootCenterScreenPos.Y - (currentFrame.pivotY * scaledHeight);

    Gdiplus::RectF destRect(finalRenderTopLeftScreenX, finalRenderTopLeftScreenY, scaledWidth, scaledHeight);
    Gdiplus::RectF sourceRect(currentFrame.sourceRect.X, currentFrame.sourceRect.Y, 
                             currentFrame.sourceRect.Width, currentFrame.sourceRect.Height); 

    RenderManager::GetInstance()->AddDrawCommand(
        currentSheet->GetBitmap(),
        destRect,
        sourceRect,
        Gdiplus::UnitPixel,
        characterFootCenterScreenPos,
        layer,
        sortKey,
        currentDir
    );
}

const AnimationFrame& Animator::GetCurrentFrame() const {
    if (!m_currentClip || m_currentClip->GetFrames().empty()) {
        static AnimationFrame dummyFrame;
        return dummyFrame;
    }
    return m_currentClip->GetCurrentFrame(m_elapsed);
}

const SpriteSheet* Animator::GetSpriteSheet() const {
    if (!m_currentClip) {
		OutputDebugStringW((L"Animator: GetSpriteSheet - m_currentClip이 null입니다. State: " + 
						   std::to_wstring(m_currentState) + L", Direction: " + 
						   std::to_wstring(m_currentDirection) + L"\n").c_str());
		return nullptr;
	}
    return m_currentClip->GetSpriteSheet();
}

bool Animator::IsAnimationDone() const {
    if (!m_currentClip) return true;
    if (m_currentClip->IsLooping()) return false;
    return m_elapsed >= m_currentClip->GetTotalDuration();
}

float Animator::GetCurrentClipTotalDuration() const {
    if (m_currentClip) {
        return m_currentClip->GetTotalDuration();
    }
    return 0.0f;
}

int Animator::GetCurrentFrameIndex() const {
    if (!m_currentClip || m_currentClip->GetFrames().empty()) {
        return -1;
    }
    
    float t = m_currentClip->IsLooping() ? fmod(m_elapsed, m_currentClip->GetTotalDuration()) : min(m_elapsed, m_currentClip->GetTotalDuration());
    float acc = 0.0f;
    
    for (size_t i = 0; i < m_currentClip->GetFrames().size(); ++i) {
        acc += m_currentClip->GetFrames()[i].duration;
        if (t < acc) {
            return static_cast<int>(i);
        }
    }
    
    return static_cast<int>(m_currentClip->GetFrames().size() - 1);
}

void Animator::Play() { if (m_currentClip) { m_isPlaying = true; } }
void Animator::Pause() { m_isPlaying = false; }
void Animator::Stop() { m_isPlaying = false; m_elapsed = 0.0f; }
