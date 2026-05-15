#pragma once
#ifndef PERF_OPTIMIZATION_DEFAULT_ON
#define PERF_OPTIMIZATION_DEFAULT_ON 1
#endif

#ifdef _DEBUG
// 최적화 모드 활성화 (ON: 최적화된 코드 / OFF: 비최적화 코드)
extern bool g_bEnableOptimizationMode;

// 각 최적화별 세부 플래그
extern bool g_bEnableBufferReuse;              // 버퍼 재사용
extern bool g_bEnableTileCaching;              // 타일 캐싱
extern bool g_bEnableSpatialPartitioning;      // 공간 분할

// 현재 모드 상태 문자열
extern const char* g_CurrentOptimizationMode;

// 토글 함수
void ToggleOptimizationMode();
#endif
