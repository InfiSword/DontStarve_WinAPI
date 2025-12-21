#pragma once
#include <vector>
#include <string>
#include <map>
#include "../../Header/Enum.h"

// 애니메이션 정의 구조체 (GameObject가 Animator에 제공하는 애니메이션 정보)
struct AnimationDefinition {
    int state;
    Direction direction;
    std::wstring imagePath;
    UINT frameWidth;
    UINT frameHeight;
    UINT framesPerRow;
    UINT totalFrames;
    float frameDuration;
    float pivotX;
    float pivotY;
    bool isLoop;
    std::map<int, std::wstring> events;
    
    AnimationDefinition()
        : state(0), direction(DIR_DOWN), frameWidth(0), frameHeight(0),
          framesPerRow(0), totalFrames(0), frameDuration(0.1f),
          pivotX(0.5f), pivotY(1.0f), isLoop(true) {}
};

