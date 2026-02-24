#pragma once

#include "EditorView.h"

/// ObjectEditor 전용 뷰: 이동 제한을 윈도우 크기(client) 기준으로 적용.
class ObjectView : public EditorView
{
public:
	ObjectView() = default;
	~ObjectView() = default;

	/// 맵 월드 크기를 clientWidth x clientHeight 로 간주하여 오프셋 클램프 (mapWidthTiles, mapHeightTiles 무시)
	void SetMapOffsetClamped(int x, int y, int clientWidth, int clientHeight, int mapWidthTiles, int mapHeightTiles) override;
};
