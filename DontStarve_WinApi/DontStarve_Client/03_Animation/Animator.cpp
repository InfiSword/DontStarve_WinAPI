#include "99_Default/pch.h"
#include "Animator.h"
#include "AnimationClip.h"
#include "SpriteSheet.h"
#include "../01_Manager/RenderManager/RenderManager.h"
#include "../01_Manager/ResourceManager/ResourceManager.h"

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

void Animator::RegisterAnimation(int state, Direction dir, 
                                const std::wstring& imagePath,
                                UINT frameWidth, UINT frameHeight,
                                UINT framesPerRow, UINT totalFrames,
                                float pivotX, float pivotY,
                                bool loop,
                                float frameDuration,
                                bool flipHorizontal) 
{
    int key = GetAnimationKey(state, static_cast<int>(dir));

    bool shouldFlip = flipHorizontal ? true : (dir == DIR_LEFT);

    // ResourceManager 캐시를 통해 SpriteSheet 공유 - 동일 이미지/설정은 비트맵 재사용
    std::shared_ptr<SpriteSheet> sheet = ResourceManager::GetInstance()->LoadSpriteSheet(
        imagePath, frameWidth, frameHeight, framesPerRow, totalFrames, shouldFlip);
    if (!sheet) return;

    // 이미 해당 key로 등록된 클립이 있으면 교체하지 않음 (중복 등록 방지 → 댕글링 원인 제거)
    if (m_animations.find(key) != m_animations.end()) {
        return;
    }

    auto clip = std::make_unique<AnimationClip>(L"", sheet, pivotX, pivotY, loop, shouldFlip, frameDuration);
    if (!clip) return;

    m_animations[key] = std::move(clip);
}

AnimationClip* Animator::GetAnimationClip(int state, Direction dir)
{
    int key = GetAnimationKey(state, static_cast<int>(dir));
    auto it = m_animations.find(key);
    if (it != m_animations.end()) {
        return it->second.get();
    }
    return nullptr;
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
        // 경과 시간 누적
        m_elapsed += deltaTime;
        
        // 루프가 아닌 애니메이션의 종료 체크
        if (!m_currentClip->IsLooping() && m_elapsed >= m_currentClip->GetTotalDuration()) {
            m_elapsed = m_currentClip->GetTotalDuration();
            m_isPlaying = false;
        }

        // 현재 프레임 인덱스 계산
        int currentFrameIndex = GetCurrentFrameIndex();
        
        // 프레임 변경 시, 건너뛴 프레임 포함해 지나친 모든 프레임에 대해 이벤트 발생 (deltaTime이 커서 frame 4 등을 건너뛰는 경우 대비)
        if (currentFrameIndex != -1 && currentFrameIndex != m_lastTriggeredFrame)
		{
            const std::map<int, std::wstring>& eventFrames = m_currentClip->GetEventFrames();
            const AnimationEventCallback& callback = m_currentClip->GetEventCallback();
            const int startIdx = m_lastTriggeredFrame + 1;
            const int endIdx = currentFrameIndex;
            for (int fi = startIdx; fi <= endIdx && callback; ++fi) {
                auto eventIt = eventFrames.find(fi);
                if (eventIt != eventFrames.end())
                    callback(fi, eventIt->second);
            }
            m_lastTriggeredFrame = currentFrameIndex;
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
    Gdiplus::Bitmap* pBitmap = currentSheet ? currentSheet->GetBitmap() : nullptr;
    if (!pBitmap) {
        return;
    }

    float scaledWidth = currentFrame.width * zoomFactor; 
    float scaledHeight = currentFrame.height * zoomFactor;

    float finalRenderTopLeftScreenX = characterFootCenterScreenPos.X - (currentFrame.pivotX * scaledWidth);
    float finalRenderTopLeftScreenY = characterFootCenterScreenPos.Y - (currentFrame.pivotY * scaledHeight);

    Gdiplus::RectF destRect(finalRenderTopLeftScreenX, finalRenderTopLeftScreenY, scaledWidth, scaledHeight);
    Gdiplus::RectF sourceRect(currentFrame.sourceRect.X, currentFrame.sourceRect.Y, 
                             currentFrame.sourceRect.Width, currentFrame.sourceRect.Height); 

    // LEFT 방향은 미리 반전된 비트맵을 사용하므로 RenderManager에서 추가 Transform 불필요
    // preFlipped 플래그를 전달하여 이중 반전 방지
    RenderManager::GetInstance()->AddDrawCommand(
        pBitmap,
        destRect,
        sourceRect,
        Gdiplus::UnitPixel,
        characterFootCenterScreenPos,
        layer,
        sortKey,
        currentDir,
        Gdiplus::Color(255, 255, 255, 255),
        false,
        m_currentClip->IsPreFlipped()  // LEFT 방향일 때 true (미리 반전된 비트맵)
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
