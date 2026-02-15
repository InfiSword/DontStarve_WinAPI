#include "99_Default/pch.h"
#include "Animator.h"
#include "AnimationClip.h"
#include "SpriteSheet.h"
#include "../01_Manager/RenderManager/RenderManager.h"

Animator::Animator(GameObject* owner)
    : Component(owner), m_currentClip(nullptr), m_currentState(-1), m_currentDirection(-1), 
      m_elapsed(0.0f), m_isPlaying(false), m_lastTriggeredFrame(-1) {}

Animator::~Animator() { 
    m_animations.clear(); 
    m_currentClip = nullptr; 
}

void Animator::Init() {
    Component::Init();
}

// 애니메이션 등록 (frameDuration: 모든 프레임에 적용되는 지속 시간, 기본 0.03초)
void Animator::RegisterAnimation(int state, Direction dir, 
                                const std::wstring& imagePath,
                                UINT frameWidth, UINT frameHeight,
                                UINT framesPerRow, UINT totalFrames,
                                float pivotX, float pivotY,
                                bool loop,
                                const std::map<int, std::wstring>& events,
                                bool flipHorizontal,
                                float frameDuration) 
{
    int key = GetAnimationKey(state, static_cast<int>(dir));
    
    auto sheet = SpriteSheet::CreateFromFile(imagePath, frameWidth, frameHeight, framesPerRow, totalFrames, flipHorizontal);
    if (!sheet) return;

    auto clip = std::make_unique<AnimationClip>(L"", std::move(sheet), pivotX, pivotY, loop, flipHorizontal, frameDuration);
    
    if (!clip) return;
    
    // 이벤트 프레임 등록
    for (const auto& eventPair : events) {
        clip->AddEventFrame(eventPair.first, eventPair.second);
    }
    
    // 전역 이벤트 콜백 설정
    if (m_globalEventCallback) {
        clip->SetEventCallback(m_globalEventCallback);
    }
    
    m_animations[key] = std::move(clip);
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
    auto it = m_animations.find(key);
    // 같은 state에서 방향만 DIR_DOWN으로 폴백 시도 (클립 누락 시 PICKUP 등이 재생되도록)
    if (it == m_animations.end() && m_currentDirection != static_cast<int>(DIR_DOWN)) {
        key = GetAnimationKey(m_currentState, static_cast<int>(DIR_DOWN));
        it = m_animations.find(key);
    }
    if (it != m_animations.end()) {
        AnimationClip* newClip = it->second.get();
        if (m_currentClip != newClip) {
            m_currentClip = newClip;
            m_elapsed = 0.0f;
            m_isPlaying = true;
            m_lastTriggeredFrame = -1;
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

    // preFlipped 클립은 이미 비트맵이 반전되어 있으므로 Transform 불필요 (DIR_DOWN으로 그리기)
    Direction drawDir = (m_currentClip->IsPreFlipped() ? DIR_DOWN : currentDir);
    RenderManager::GetInstance()->AddDrawCommand(
        currentSheet->GetBitmap(),
        destRect,
        sourceRect,
        Gdiplus::UnitPixel,
        characterFootCenterScreenPos,
        layer,
        sortKey,
        drawDir
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
