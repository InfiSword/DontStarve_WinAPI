#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include "Struct.h"
#include "../DontStarve_Client/03_Animation/AnimationClip.h"
#include "../DontStarve_Client/03_Animation/SpriteSheet.h"

using namespace Gdiplus;

struct AnimationFrame;

// 아래 전역 유틸 함수들은 에디터와 클라이언트(게임) 모두에서 필요한 인라인 함수
// 만약 에디터 전용/클라이언트 전용 함수가 추가되었다면, 별도 파일로 분리하거나 주석으로 명확히 표시해주세요.

template<typename T>
inline void SafeDelete(T& obj)
{
	if (obj)
	{
		delete obj;
		obj = nullptr;
	}
}

// 비트맵 관련 전역 유틸 함수들
namespace BitmapUtils
{
	inline int GetBitmapWidth(Bitmap* pBitmap) {
		return pBitmap ? pBitmap->GetWidth() : 0;
	}
	inline int GetBitmapHeight(Bitmap* pBitmap) {
		return pBitmap ? pBitmap->GetHeight() : 0;
	}
	inline Bitmap* LoadBitmapFromFile(const WCHAR* filename) {
		if (!filename) return nullptr;
		Bitmap* pBitmap = Bitmap::FromFile(filename);
		if (!pBitmap || pBitmap->GetLastStatus() != Ok) {
			SafeDelete(pBitmap);
			return nullptr;
		}
		return pBitmap;
	}
}

// 리소스 관련 전역 유틸 함수들
namespace ResourceUtils 
{
	template<typename VariantType>
	inline void SafeDeleteVariant(VariantType& variant) {
		if (variant.pAtlasBitmap) {
			delete variant.pAtlasBitmap;
			variant.pAtlasBitmap = nullptr;
		}
	}
	template<typename VariantMap>
	inline void ClearVariantMap(VariantMap& map) {
		for (auto& pair : map) {
			for (auto& innerPair : pair.second) {
				SafeDeleteVariant(innerPair.second);
			}
		}
		map.clear();
	}
}

// 거리 계산 전역 유틸 함수들
inline float CalculateDistance(float x1, float y1, float x2, float y2)
{
	float dx = x2 - x1;
	float dy = y2 - y1;
	return sqrtf(dx * dx + dy * dy);
}

inline Direction GetDirectionToTarget(float fromX, float fromY, float toX, float toY)
{
	float dx = toX - fromX;
	float dy = toY - fromY;

	// 절댓값이 더 큰 방향을 우선적으로 선택
	if (abs(dx) > abs(dy))
	{
		return (dx > 0) ? DIR_RIGHT : DIR_LEFT;
	}
	else
	{
		return (dy > 0) ? DIR_DOWN : DIR_UP;
	}
}
