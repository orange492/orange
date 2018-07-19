#pragma once
#include "tileNode.h"
#include "gameNode.h"

enum SELECT
{
	ROOM,
	HROAD,
	VROAD,
	TERRAINDRAW,
	OBJDRAW,
	ITEMDRAW,
	MONSTER
};


class MapTool :	public gameNode
{
private:
	tagTile		_tiles[TILEX * TILEY];						//맵타일
	tagCurrentTile	_mouseTile[2];							//드래그할때 마우스좌표
	tagCurrentTile	_currentTileT;							//현재 지형타일
	tagCurrentTile	_currentTileO;							//현재 오브젝트타일
	tagCurrentTile	_currentXY;								//현재 사용가능한 맵크기
	tagSampleTile	_sampleTile[SAMPLETILEX * SAMPLETILEY];	//샘플타일
	//DWORD		_attribute[TILEX * TILEY];					// 타일 속성	

	RECT		_rc[10];									// 버튼 렉트
	SELECT		_select;									//어떤 버튼 선택중인지

	//int			_pos[2];
	int			_rock[2];									//최대,현재 보유중인 벽돌 수
	int			_food[2];									//최대,현재 보유중인 식량 수
	int			_drag;										//드래그 후 _select에따라 다른 기능 실행
	int			_wall;										//몇번째 벽 선택중인지
	int			_floor;										//몇번째 바닥 선택중인지
	int			_tree;										//무슨 나무 선택중인지

	bool		_canMove;									//탭으로 타일 확인할때 못움직이게
	bool		_eraser;									//무슨 지우개 선택중인지

public:

	HRESULT init();
	void release();
	void update();
	void render();
		
	void setTree();
	void setWall(int i);
	void setFloor(int i);
	void set_wall(int i);
	void set_floor(int i);
	void dragMake();
	void setPosition(int i, TERRAIN ter, OBJECT obj, POS pos);
	void setOFrame(int i, int x, int y);
	void setTFrame(int i, int x, int y);
	void checkRock();
	int checkRockTile(int i);
	void setup();
	virtual void save();											// 세이브
	virtual void load();											// 로드
	virtual void setmap();

	TERRAIN		terrainSelect(int FrameX, int FrameY);		// 어떤 지형을 선택했는지
	OBJECT		objSelect(int FrameX, int FrameY);			// 어던 오브젝트를 선택했는지
	tagTile*	getTiles()		{ return _tiles; }			// 타일 접근자
	//DWORD*		getAttribute()	{ return _attribute; }		// 타일 속성 접근자
	tagCurrentTile	getCurrentXY() { return _currentXY; }
	int getRock(int i) { return _rock[i]; }
	int getFood(int i) { return _food[i]; }
	bool	getCanMove() { return _canMove; }

	MapTool();
	~MapTool();
};

