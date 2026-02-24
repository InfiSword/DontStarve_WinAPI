#include "../pch.h"
#include "ObjectView.h"

void ObjectView::SetMapOffsetClamped(int x, int y, int clientWidth, int clientHeight, int mapWidthTiles, int mapHeightTiles)
{
	(void)mapWidthTiles;
	(void)mapHeightTiles;
	// 이동 제한: 최대 드레그 영역 = 25% 줌 아웃 시 한 화면 크기 (client/0.25 = 4*client)
	const float zoom25ScreenW = (float)clientWidth / 0.25f;
	const float zoom25ScreenH = (float)clientHeight / 0.25f;
	const float mapWorldW = zoom25ScreenW;
	const float mapWorldH = zoom25ScreenH;
	float zoom = GetZoomFactor();

	int minOffsetX = (int)(clientWidth - mapWorldW * zoom);
	int maxOffsetX = 0;
	int minOffsetY = (int)(clientHeight - mapWorldH * zoom);
	int maxOffsetY = 0;

	int clampedX = (minOffsetX > maxOffsetX) ? x : min(maxOffsetX, max(minOffsetX, x));
	int clampedY = (minOffsetY > maxOffsetY) ? y : min(maxOffsetY, max(minOffsetY, y));
	SetMapOffset(clampedX, clampedY);
}
