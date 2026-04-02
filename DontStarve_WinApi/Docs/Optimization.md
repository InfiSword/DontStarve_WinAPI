레이어별 렌더 큐 분할 (Layered Render Queue): 모든 드로우 커맨드를 하나의 배열에 넣지 않고 m_layerCommands[LAYER_COUNT]로 분할했습니다.

조건부 상태 변경 (Conditional State Saving): GDI+에서 가장 무거운 작업 이 바로 변환 행렬 스택 저장/복구임 

뷰포트 컬링 (Viewport Culling): CameraManager::UpdateVisibleObjects()를 통해 현재 카메라 화면(Viewport)의 일정 여유 공간(Margin) 안에 들어오는 객체들만 m_visibleObjects에 담아내어 렌더링
타일도 비슷하게 맵 전체의 타일 이미지를 한 번에 다 메모리에 올리지 않고, RenderVisibleTiles에서 현재 화면에 보이는 타일 ID만 m_tileCache에 로드해서 최적화함

굶지마(Don't Starve) 류의 게임처럼 월드에 나무, 돌, 풀이 수천 개씩 깔려있다면 이 부분에서 CPU가 한계에 부딪침 QuadTree(쿼드트리) 나 Grid 기반의 2D 공간 해시(Spatial Hash) 맵을 통해 최적화 함

ColorMatrix를 이용한 틴트 효과(RenderManager::RenderSprite)나, 반투명 스프라이트가 여러 겹 겹치는 오버드로우(Overdraw)가 발생하면 코드를 아무리 최적화해도 프레임이 떨어집니다.