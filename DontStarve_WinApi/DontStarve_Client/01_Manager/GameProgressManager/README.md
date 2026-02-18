# GameProgressManager

게임 진행도를 중앙에서 관리하는 싱글톤 매니저 클래스입니다.

## 주요 기능

### 1. 씬 클리어 관리
- 씬별 클리어 조건 정의
- 클리어 여부 확인
- 클리어 날짜 기록

### 2. 캐릭터 해금 시스템
- 씬 클리어에 따른 자동 캐릭터 해금
- 캐릭터 해금 상태 확인

### 3. 이벤트 기반 진행도 추적
- 몬스터 처치 카운트
- 아이템 수집 카운트
- 실시간 클리어 조건 체크

### 4. 진행도 저장/로드
- 텍스트 파일 형식 (.txt)
- 디버깅 편의를 위한 가독성 있는 포맷
- 저장 위치: `../GameData/game_progress.txt`

## 사용 방법

### 초기화
```cpp
// GameScene::Init()에서 호출
GameProgressManager::GetInstance()->Init();
```

### 씬 클리어 확인
```cpp
bool isCleared = GameProgressManager::GetInstance()->IsSceneCleared(SCENE_GAME_HOUND_FOREST);
```

### 캐릭터 해금 확인
```cpp
bool isUnlocked = GameProgressManager::GetInstance()->IsCharacterUnlocked(GOID_PLAYER_WILLOW);
```

### 몬스터 처치 알림
```cpp
// Monster::Damaged()에서 HP가 0 이하가 되면 호출
SceneType currentScene = SceneManager::GetInstance()->GetCurrentSceneType();
GameProgressManager::GetInstance()->OnMonsterKilled(GetID(), currentScene);
```

### 아이템 획득 알림
```cpp
// InventoryManager::TryGainItemFromWorldObject()에서 호출
SceneType currentScene = SceneManager::GetInstance()->GetCurrentSceneType();
GameProgressManager::GetInstance()->OnItemCollected(itemID, count, currentScene);
```

## 저장 파일 형식

```text
[GAME_PROGRESS_V1]
# Scene Clear Info
SCENE:3,CLEARED:1,DATE:2026-02-18
SCENE:4,CLEARED:0,DATE:
SCENE:5,CLEARED:0,DATE:

# Character Unlock Info
CHARACTER:164,UNLOCKED:1
CHARACTER:165,UNLOCKED:0
CHARACTER:166,UNLOCKED:0

# Kill Count (현재 씬 진행도 추적용)
KILL_COUNT:146:3
KILL_COUNT:147:1

# Item Collection (현재 씬 진행도 추적용)
ITEM_COUNT:160:2
ITEM_COUNT:161:3
```

## 클리어 조건

### SCENE_GAME_FARMING_AREA
- 기본으로 해금된 씬

### SCENE_GAME_HOUND_FOREST
- 필수 처치: GOID_MONSTER_HOUNDDOG, GOID_MONSTER_REDHOUNDDOG, GOID_MONSTER_ICEHOUNDDOG
- 해금 캐릭터: Willow

### SCENE_GAME_SPIDER_QUEEN_HOUSE
- 필수 처치: GOID_MONSTER_QUEEN_SPIDER
- 필수 아이템: GOID_ITEM_MEAT, GOID_ITEM_BERRY (총 5개)
- 해금 캐릭터: Wolfgang

## 주의사항

1. **씬 전환 시 진행도 초기화**: 새로운 씬으로 진입할 때 `ResetCurrentSceneProgress()`를 호출하여 이전 씬의 진행도를 초기화해야 합니다.
2. **Release 시 자동 저장**: 게임 종료 시 `Release()` 메서드에서 자동으로 진행도를 저장합니다.
3. **스레드 안전성**: 현재 싱글 스레드 환경을 가정하고 설계되었습니다.

## 구조체

### SceneClearInfo
씬 클리어 정보를 담는 구조체

### SceneClearCondition
씬별 클리어 조건을 정의하는 구조체

### CharacterUnlockInfo
캐릭터 해금 정보를 담는 구조체

### GameProgress
전체 게임 진행도를 관리하는 구조체
