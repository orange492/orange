#include "stdafx.h"
#include "MapTool.h"
#include "monsterManager.h"
#include "star.h"

//지우개, 전체지우개, 그리개 - 나무 막아둠
//바위갯수 넘었을 때, 나무걸렸을때 빨강
//있던 벽에 연결하면  자연스럽게 하기
//그냥타일 까는 것
//가로세로이미지만들기
//가로세로길 추가
//던전입구
//맵 저장슬롯
//벽생성 막기, 삭제
//몬스터도같이세이브
//애니메이션
//체력바
//공격타일설정
//죽으면파이트해제,방향
//플레이어,속성
//공속,공격방향

HRESULT MapTool::init()
{
	_map = new image;
	_map = IMAGEMANAGER->addImage("field", TILESIZEX, TILESIZEY);
	_bullet = new Bullet;
	_bullet->init(50);
	setup();
	_select = ROOM;
	// 버튼 렉트
	for (int i = 0; i < 5; i++)
	{
		_rc[i] = RectMakeCenter((WINSIZEX / 2 + 485) + i * 105, 130, 90, 60);
	}
	for (int i = 5; i < 10; i++)
	{
		_rc[i] = RectMakeCenter((WINSIZEX / 2 + 485) + (i-5) * 105, 1020, 90, 60);
	}

	_drag = 0;
	_eraser = true;
	_side = true;
	_currentXY = { 20,20 };
	_stone[0] =_stone[1] = 150;
	_food[0] =_food[1] = 20;
	_money = 100;
	_currentMon = -1;
	_stage = 0;
	_day = 1;
	_monCount = 0;
	_interval = 250;
	_bulletCount = 0;
	_ballCount = 0;

	_mapX = 250, _mapY = 250;
	for (int i = 0; i < 3; i++)
	{
		_minimap[i] = new image;
		_minimap[i]->init(_mapX, _mapY);
		PatBlt(_minimap[i]->getMemDC(), 0, 0, _mapX, _mapY, WHITENESS);
		_save[i].current = {0,0};
		_save[i].stone = 0;
	}
	_tempImg = new image;
	_tempImg->init((_currentXY.x + 4)*TILESIZE, (_currentXY.y + 4) * TILESIZE);

	setTree();
	setMinimap2();
	drawMap2(true);

	return S_OK;
}

void MapTool::release(){}

void MapTool::update()
{
	_bullet->update();
	if (_player != 0&&_vRoad.size()!=0&& _stage != 2)
		_stage = 1;
	else if(_stage!=2)
		_stage = 0;

	if (_stage == 2)
	{
		_monCount++;
		if (_monCount% _interval == 100)
		{
			int rand = 0;
			int dir = 0;
			int player = _player;
			if (_mM->getDex(_tiles[_player].mon).size.x > 2)
				player += 1;
			if (_mM->getDex(_tiles[_player].mon).size.y > 2)
				player += 100;
			rand = RND->getInt(_vDoor.size());
			while (_star->findRoad(_vDoor[rand], player, _player - 101, _mM->getDex(_tiles[_player].mon).size.x, _mM->getDex(_tiles[_player].mon).size.y).size() == 0)
				rand = RND->getInt(_vDoor.size());
			if (_tiles[_vDoor[rand] - TILEX].terrain == TR_NULL)
				dir = 0;
			if (_tiles[_vDoor[rand] + TILEX].terrain == TR_NULL)
				dir = 1;
			if (_tiles[_vDoor[rand] + 1].terrain == TR_NULL)
				dir = 2;
			if (_tiles[_vDoor[rand] - 1].terrain == TR_NULL)
				dir = 3;
			
			int rand2 = 0;

			if(_interval<50)
				rand2 = RND->getInt(15);
			else if(_interval<100)
				rand2 = RND->getInt(12);
			else if (_interval<150)
				rand2 = RND->getInt(9);
			else if (_interval<200)
				rand2 = RND->getInt(6);
			else
				rand2 = RND->getInt(3);
			if (rand2 == 1)
				rand2 = 3;
			else if (rand2 == 2)
				rand2 = 6;
			else if (rand2 == 3)
				rand2 = 1;
			else if (rand2 == 5)
				rand2 = 7;
			else if (rand2 == 6)
				rand2 = 2;
			else if (rand2 == 7)
				rand2 = 5;
			else if (rand2 == 10)
				rand2 = 13;
			else if (rand2 == 13)
				rand2 = 10;

			_mM->addEmon(100, rand2, dir, _star->findRoad(_vDoor[rand], player, _player - 101, _mM->getDex(_tiles[_player].mon).size.x, _mM->getDex(_tiles[_player].mon).size.y));
	
			if(_interval>30)
				_interval -= 5;
		}

		if (_tiles[_player].mon == -1)
		{
			_side = true;
			_monCount = 0;
			_interval = 250;
			_mM->eraseEmon();
			_stage = 0;
			_ballCount = 0;
			_day++;
			save();
			_player = 0;
			for (_viRoad = _vRoad.begin(); _viRoad != _vRoad.end();)
			{
				_viRoad = _vRoad.erase(_viRoad);
			}
			_bullet->resetBullet();
			drawMap();
		}
	}

	_mouseTile[0].x = getMemDCPoint().x / 32;
	_mouseTile[0].y = getMemDCPoint().y / 32;
	monFrame();
	monUpdate();
	collision();
	char str[128];
	if (_stage == 2)
	{
		if (_bulletCount < 15)
			_bulletCount++;
		if (_ballCount < 1000)
			_ballCount++;
	}
	if (KEYMANAGER->isOnceKeyDown(VK_LBUTTON))
	{
		if (_stage != 2&&_ptMouse.y>80)
		{
			if (_side == true)
				setmap();
			if ((_ptMouse.x < 1400 && _side == true) || _side == false)
			{
				_mouseTile[1] = _mouseTile[0];
				_drag = 1;
			}

			if (_select == MONSTER)
			{
				for (int i = 0; i < 14; i++)
				{
					if (PtInRect(&RectMake(1414 + (i % 2 * 250), 170 + (i / 2 * 110), 220, 100), _ptMouse) && _mM->getVDmon().size() != i + CAMERAMANAGER->getScroll() * 2)
					{
						if (_mM->getVDmon().size() < i) break;
						if (_currentMon == i + CAMERAMANAGER->getScroll() * 2 && _mM->getDex(_mM->getVDmon()[_currentMon].num).evolution == true && _mM->getVDmon()[_currentMon].have > 1)
						{
							_mM->setHave(_currentMon, _mM->getVDmon()[_currentMon].have - 2);
							_mM->addDmon(_mM->getVDmon()[_currentMon].num + 1);
							if (_mM->getVDmon()[_currentMon].have == 0)
								_mM->eraseDmon(_currentMon);

							_currentMon = -1;
							drawList(_currentMon);
							break;
						}
						else if (_currentMon == i + CAMERAMANAGER->getScroll() * 2)
						{
							_currentMon = -1;
							drawList(_currentMon);
							break;
						}
						_currentMon = i + CAMERAMANAGER->getScroll() * 2;
						drawList(_currentMon);
						break;
					}
				}
			}

			if (_tiles[_mouseTile[0].x + (_mouseTile[0].y) * 100].object == OBJ_MON && _tiles[_mouseTile[0].x + (_mouseTile[0].y) * 100].monPos != MPOS_0)
			{
				tagCurrentTile mouseTile = _mouseTile[0];
				mouseTile = findMon(mouseTile);
				if(_player==0)
					_player = mouseTile.x + (mouseTile.y) * 100;
				else
				{
					_player = 0;
					for (_viRoad = _vRoad.begin(); _viRoad != _vRoad.end();)
					{
						_viRoad = _vRoad.erase(_viRoad);
					}
				}
				findRoad();
			}
			else if (_stage != 2)
			{
				_player = 0;
				for (_viRoad = _vRoad.begin(); _viRoad != _vRoad.end();)
				{
					_viRoad = _vRoad.erase(_viRoad);
				}
			}
		}
		else if(_stage==2&&_bulletCount>=15)
		{
			int bulletloca= _player;
			if (_tiles[_player].mon == 1)
				bulletloca = _player + 100;
			else if (_tiles[_player].mon == 2)
				bulletloca = _player + 101;
			else if (_tiles[_player].mon == 5)
				bulletloca = _player + 100;
			else if (_tiles[_player].mon == 8)
				bulletloca = _player + 101;
			else if (_tiles[_player].mon == 10)
				bulletloca = _player + 101;
			else if (_tiles[_player].mon == 12)
				bulletloca = _player + 101;
			else if (_tiles[_player].mon == 13)
				bulletloca = _player + 100;
			else if (_tiles[_player].mon == 14)
				bulletloca = _player + 101;
			sprintf_s(str, "b%d", (int)_mM->getDex(_tiles[_player].mon).type);
			_bullet->bulletFire(str, _tiles[bulletloca].rc.left + 16, _tiles[bulletloca].rc.top + 16,
				GetAngle(_tiles[bulletloca].rc.left + 16, _tiles[bulletloca].rc.top + 16, _ptMouse.x+CAMERAMANAGER->getCameraCenter().x-WINSIZEX/2, _ptMouse.y + CAMERAMANAGER->getCameraCenter().y - WINSIZEY / 2),
				(int)_mM->getDex(_tiles[_player].mon).power, (int)_mM->getDex(_tiles[_player].mon).spd, (int)_mM->getDex(_tiles[_player].mon).type);
			_bulletCount = 0;
		}
	}
	else if (KEYMANAGER->isOnceKeyUp(VK_LBUTTON)&&(_drag!=0||_select>4))
	{
		if (_stage == 2) return;
		{
			if (_select == ROOM)
				_drag = 2;
			else if (_select == HROAD)
				_drag = 3;
			else if (_select == VROAD)
				_drag = 4;
			else if (_select == TERRAINDRAW)
				_drag = 5;
			else if (_select == OBJDRAW)
				_drag = 6;
			else if (_select == ITEMDRAW)
				_drag = 9;
			else if (_select == MONSTER)
				_drag = 10;
		}
		if (_select == SAVE)
			_drag = 11;
		else if (_select == LOAD)
			_drag = 12;
	}
	else if (KEYMANAGER->isOnceKeyDown(VK_RBUTTON))
	{
		if (_stage != 2)
		{
			if ((_ptMouse.x < 1400 && _side == true) || _side == false)
			{
				if (_tiles[_mouseTile[0].x + (_mouseTile[0].y) * 100].object == OBJ_MON && _tiles[_mouseTile[0].x + (_mouseTile[0].y) * 100].monPos != MPOS_0)
				{
					cancelMon(_mouseTile[0], false, false);
				}
				else
				{
					_mouseTile[1] = _mouseTile[0];
					_drag = -1;
				}
			}
		}
		else if (_stage == 2 && _ballCount >= 1000)
		{
			int bulletloca = _player;
			if (_tiles[_player].mon == 1)
				bulletloca = _player + 100;
			else if (_tiles[_player].mon == 2)
				bulletloca = _player + 101;
			else if (_tiles[_player].mon == 5)
				bulletloca = _player + 100;
			else if (_tiles[_player].mon == 8)
				bulletloca = _player + 101;
			else if (_tiles[_player].mon == 10)
				bulletloca = _player + 101;
			else if (_tiles[_player].mon == 12)
				bulletloca = _player + 101;
			else if (_tiles[_player].mon == 13)
				bulletloca = _player + 100;
			else if (_tiles[_player].mon == 14)
				bulletloca = _player + 101;
			sprintf_s(str, "ball", (int)_mM->getDex(_tiles[_player].mon).type);
			_bullet->ballFire(_tiles[bulletloca].rc.left + 16, _tiles[bulletloca].rc.top + 16,
				GetAngle(_tiles[bulletloca].rc.left + 16, _tiles[bulletloca].rc.top + 16, _ptMouse.x + CAMERAMANAGER->getCameraCenter().x - WINSIZEX / 2, _ptMouse.y + CAMERAMANAGER->getCameraCenter().y - WINSIZEY / 2));
			_ballCount = 0;
		}
	}
	else if (KEYMANAGER->isOnceKeyUp(VK_RBUTTON) && _drag != 0)
	{
		if (_stage == 2) return;
		_eraser==false ?	_drag = 7 : _drag=8;
	}
}

void MapTool::render()
{
	IMAGEMANAGER->render("field", DC, CAMERAMANAGER->getCameraCenter().x - WINSIZEX / 2, CAMERAMANAGER->getCameraCenter().y - WINSIZEY / 2, CAMERAMANAGER->getCameraCenter().x - WINSIZEX / 2, CAMERAMANAGER->getCameraCenter().y - WINSIZEY / 2, WINSIZEX, WINSIZEY);
	if (_side == true)
	{
		IMAGEMANAGER->render("side", UIDC, 0, 0);

		if (_select != SAVE && _select != LOAD&& _select != MONSTER&& _select != ITEMDRAW)
		{
			IMAGEMANAGER->render("sample", UIDC, WINSIZEX - IMAGEMANAGER->findImage("sample")->getWidth() - SAMPLEX, SAMPLEY);
			if (_select == ROOM)
			{
				fdraw("map", UIDC, 1650 - 32, 300 - 32, 1 + 7 * (_wall / 16) - 1, 4 + 3 * (_wall % 16) - 1);
				fdraw("map", UIDC, 1650, 300 - 32, 1 + 7 * (_wall / 16), 4 + 3 * (_wall % 16) - 1);
				fdraw("map", UIDC, 1650 - 32, 300, 1 + 7 * (_wall / 16) - 1, 4 + 3 * (_wall % 16));
				fdraw("map", UIDC, 1650 + 32, 300, 1 + 7 * (_wall / 16) - 1, 4 + 3 * (_wall % 16));
				fdraw("map", UIDC, 1650 + 32, 300 - 32, 3 + 7 * (_wall / 16) - 1, 3 + 3 * (_wall % 16));
				fdraw("map", UIDC, 1650 + 32, 300 + 32, 3 + 7 * (_wall / 16) - 1, 5 + 3 * (_wall % 16));
				fdraw("map", UIDC, 1650, 300 + 32, 2 + 7 * (_wall / 16) - 1, 3 + 3 * (_wall % 16));
				fdraw("map", UIDC, 1650 - 32, 300 + 32, 1 + 7 * (_wall / 16) - 1, 5 + 3 * (_wall % 16));
				fdraw("map", UIDC, 1650, 300, 25 + 7 * (_floor / 16), 3 + 3 * (_floor % 16));
			}
			if (_select == HROAD)
			{
				fdraw("map", UIDC, 1650 - 32, 300 - 32, 1 + 7 * (_wall / 16), 4 + 3 * (_wall % 16) - 1);
				fdraw("map", UIDC, 1650, 300 - 32, 1 + 7 * (_wall / 16), 4 + 3 * (_wall % 16) - 1);
				fdraw("map", UIDC, 1650 + 32, 300 - 32, 1 + 7 * (_wall / 16), 4 + 3 * (_wall % 16) - 1);
				fdraw("map", UIDC, 1650 - 32, 300 + 32, 1 + 7 * (_wall / 16), 4 + 3 * (_wall % 16) - 1);
				fdraw("map", UIDC, 1650, 300 + 32, 1 + 7 * (_wall / 16), 4 + 3 * (_wall % 16) - 1);
				fdraw("map", UIDC, 1650 + 32, 300 + 32, 1 + 7 * (_wall / 16), 4 + 3 * (_wall % 16) - 1);
				fdraw("map", UIDC, 1650 - 32, 300, 24 + 7 * (_floor / 16), 4 + 3 * (_floor % 16));
				fdraw("map", UIDC, 1650, 300, 25 + 7 * (_floor / 16), 4 + 3 * (_floor % 16));
				fdraw("map", UIDC, 1650 + 32, 300, 26 + 7 * (_floor / 16), 4 + 3 * (_floor % 16));
			}
			if (_select == VROAD)
			{
				fdraw("map", UIDC, 1650 - 32, 300 - 32, 0 + 7 * (_wall / 16), 5 + 3 * (_wall % 16) - 1);
				fdraw("map", UIDC, 1650 - 32, 300, 0 + 7 * (_wall / 16), 5 + 3 * (_wall % 16) - 1);
				fdraw("map", UIDC, 1650 - 32, 300 + 32, +7 * (_wall / 16), 5 + 3 * (_wall % 16) - 1);
				fdraw("map", UIDC, 1650 + 32, 300 - 32, 0 + 7 * (_wall / 16), 5 + 3 * (_wall % 16) - 1);
				fdraw("map", UIDC, 1650 + 32, 300, 0 + 7 * (_wall / 16), 5 + 3 * (_wall % 16) - 1);
				fdraw("map", UIDC, 1650 + 32, 300 + 32, +7 * (_wall / 16), 5 + 3 * (_wall % 16) - 1);

				fdraw("map", UIDC, 1650, 300 - 32, 23 + 7 * (_floor / 16), 3 + 3 * (_floor % 16));
				fdraw("map", UIDC, 1650, 300, 23 + 7 * (_floor / 16), 4 + 3 * (_floor % 16));
				fdraw("map", UIDC, 1650, 300 + 32, 23 + 7 * (_floor / 16), 5 + 3 * (_floor % 16));
			}
			if (_select == OBJDRAW)
				fdraw("map", UIDC, 1650, 300, 1 + 7 * (_wall / 16), 4 + 3 * (_wall % 16));
			if (_select == TERRAINDRAW)
				fdraw("map", UIDC, 1650, 300, 25 + 7 * (_floor / 16), 3 + 3 * (_floor % 16));
		}

		for (int i = 0; i < 10; i++)
		{
			fdraw("button", UIDC, _rc[i].left, _rc[i].top, 0, 0);
			if (_select == ROOM)
			{
				fdraw("button", UIDC, _rc[0].left, _rc[0].top, 0, 1);
			}
			else if (_select == HROAD)
			{
				fdraw("button", UIDC, _rc[1].left, _rc[1].top, 0, 1);
			}
			else if (_select == VROAD)
			{
				fdraw("button", UIDC, _rc[2].left, _rc[2].top, 0, 1);
			}
			else if (_select == TERRAINDRAW || _select == OBJDRAW)
			{
				fdraw("button", UIDC, _rc[3].left, _rc[3].top, 0, 1);
			}
			else if (_select == ITEMDRAW)
			{
				fdraw("button", UIDC, _rc[4].left, _rc[4].top, 0, 1);
			}
			else if (_select == MONSTER)
			{
				fdraw("button", UIDC, _rc[5].left, _rc[5].top, 0, 1);
			}
			else if (_select == SAVE)
			{
				fdraw("button", UIDC, _rc[8].left, _rc[8].top, 0, 1);
			}
			else if (_select == LOAD)
			{
				fdraw("button", UIDC, _rc[9].left, _rc[9].top, 0, 1);
			}
			if (_eraser == false)
			{
				fdraw("button", UIDC, _rc[6].left, _rc[6].top, 0, 1);
			}
			else if (_eraser == true)
			{
				fdraw("button", UIDC, _rc[7].left, _rc[7].top, 0, 1);
			}
		}

		// 폰트
		SetTextColor(UIDC, RGB(10, 10, 10));
		SetBkMode(UIDC, TRANSPARENT);
		HFONT font, oldFont;
		font = CreateFont(30, 0, 0, 0, 100, 0, 0, 0, DEFAULT_CHARSET,
			OUT_STRING_PRECIS, CLIP_CHARACTER_PRECIS, PROOF_QUALITY,
			DEFAULT_PITCH | FF_SWISS, TEXT("HY얕은샘물M"));
		oldFont = (HFONT)SelectObject(UIDC, font);
		DrawText(UIDC, TEXT("방"), strlen("방"), &_rc[0], DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		DrawText(UIDC, TEXT("가로길"), strlen("가로길"), &_rc[1], DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		DrawText(UIDC, TEXT("세로길"), strlen("세로길"), &_rc[2], DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		DrawText(UIDC, TEXT("타일"), strlen("타일"), &_rc[3], DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		DrawText(UIDC, TEXT("상점"), strlen("상점"), &_rc[4], DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		DrawText(UIDC, TEXT("몬스터"), strlen("몬스터"), &_rc[5], DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		DrawText(UIDC, TEXT("벽삭제"), strlen("아몬터"), &_rc[6], DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		DrawText(UIDC, TEXT("다삭제"), strlen("아몬터"), &_rc[7], DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		DrawText(UIDC, TEXT("세이브"), strlen("세이브"), &_rc[8], DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		DrawText(UIDC, TEXT("로드"), strlen("몬터"), &_rc[9], DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		char str[128];
		char str2[128];

		if (_select == SAVE || _select == LOAD)
		{
			for (int i = 0; i < 3; i++)
			{
				_minimap[i]->render(UIDC, WINSIZEX - _mapX - 200, 190 + (i * 260));
				sprintf_s(str, "%d X %d", _save[i].current.x, _save[i].current.y);
				sprintf_s(str2, "%d", _save[i].stone);
				DrawText(UIDC, TEXT(str), strlen(str), &RectMake(WINSIZEX - 180, 270 + (i * 260), 200, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
				DrawText(UIDC, TEXT(str2), strlen(str2), &RectMake(WINSIZEX - 180 + 35, 320 + (i * 260), 200, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
				IMAGEMANAGER->render("stone2", UIDC, WINSIZEX - 185, 320 + (i * 260));
			}
			
		}

		if (_select == MONSTER)
		{
			int count=0;
			CAMERAMANAGER->cameraRender(UIDC);
			sprintf_s(str, "배치 %d",_monNum);
			DrawText(UIDC, TEXT(str), strlen(str), &RectMake(WINSIZEX - 366, 939, 200, 50), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			if (_currentMon >= 0)
			{
				for (int i = -1; i < _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y+1; i++)
				{
					for (int j = -1; j < _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x+1; j++)
					{
						if (i == -1 || i == _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y || j == _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x || j == -1)
						{
							if (_mouseTile[0].x < 1 || _mouseTile[0].y < 1) continue;
							if (_tiles[_mouseTile[0].x + j + (_mouseTile[0].y + i) * 100].object == OBJ_MON)
								count++;
						}
						else if (_tiles[_mouseTile[0].x+j + (_mouseTile[0].y+i) * 100].object == OBJ_MON || _tiles[_mouseTile[0].x+j + (_mouseTile[0].y+i) * 100].object != OBJ_NULL|| _tiles[_mouseTile[0].x + j + (_mouseTile[0].y + i) * 100].terrain != TR_FLOOR)
							count++;
					}
				}
							
				for (int i = -1; i < _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y+1; i++)
				{
					for (int j = -1; j < _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x+1; j++)
					{
						if (_mouseTile[0].x < 1 || _mouseTile[0].y < 1) continue;
						if(count>0)
							IMAGEMANAGER->findImage("tile2")->render(DC, _tiles[_mouseTile[0].x + j + (_mouseTile[0].y + i) * 100].rc.left, _tiles[_mouseTile[0].x + j + (_mouseTile[0].y + i) * 100].rc.top);
						else
						{
							IMAGEMANAGER->findImage("tile")->render(DC, _tiles[_mouseTile[0].x + j + (_mouseTile[0].y + i) * 100].rc.left, _tiles[_mouseTile[0].x + j + (_mouseTile[0].y + i) * 100].rc.top);	
							if (i == _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y)
							{
								for (int i = -1; i < _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y + 1; i++)
								{
									for (int j = -1; j < _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x + 1; j++)
									{
										if (!(i == -1 || i == _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y || j == _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x || j == -1))
											IMAGEMANAGER->findImage("tile3")->render(DC, _tiles[_mouseTile[0].x + j + (_mouseTile[0].y + i) * 100].rc.left, _tiles[_mouseTile[0].x + j + (_mouseTile[0].y + i) * 100].rc.top);
									}
								}
							}
						}
					}
				}
			}

			else
			{
				IMAGEMANAGER->findImage("tile")->render(DC, _tiles[_mouseTile[0].x + (_mouseTile[0].y) * 100].rc.left, _tiles[_mouseTile[0].x + (_mouseTile[0].y) * 100].rc.top);
			}
		}

		if (_select == ITEMDRAW)
		{
			int stonePrice=100;
			int foodPrice=100;
			int treePrice=500;
			char str[128];
			font = CreateFont(30, 0, 0, 0, 100, 0, 0, 0, DEFAULT_CHARSET,
				OUT_STRING_PRECIS, CLIP_CHARACTER_PRECIS, PROOF_QUALITY,
				DEFAULT_PITCH | FF_SWISS, TEXT("HY얕은샘물M"));
			oldFont = (HFONT)SelectObject(UIDC, font);
			draw("stone", UIDC, WINSIZEX - 470, 270);
			draw("food", UIDC, WINSIZEX - 470, 500);
			draw("tree2", UIDC, WINSIZEX - 470, 730);
			draw("coin", UIDC, WINSIZEX - 300, 320);
			draw("coin", UIDC, WINSIZEX - 300, 550);
			draw("coin", UIDC, WINSIZEX - 290, 780);
			sprintf_s(str, "%d", stonePrice);
			DrawText(UIDC, TEXT("석재(10)"), strlen("석재(10)"), &RectMake(WINSIZEX - 360, 260, 300, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			DrawText(UIDC, TEXT(str), strlen(str), &RectMake(WINSIZEX - 360, 330, 300, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			DrawText(UIDC, TEXT("식량(5)"), strlen("식량(5)"), &RectMake(WINSIZEX - 360, 490, 300, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			sprintf_s(str, "%d", foodPrice);
			DrawText(UIDC, TEXT(str), strlen(str), &RectMake(WINSIZEX - 360, 560, 300, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			DrawText(UIDC, TEXT("확장(1x1)"), strlen("확장(1x1)"), &RectMake(WINSIZEX - 360, 720, 300, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			sprintf_s(str, "%d", treePrice);
			DrawText(UIDC, TEXT(str), strlen(str), &RectMake(WINSIZEX - 360, 790, 300, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			DrawText(UIDC, TEXT("구입"), strlen("구입"), &RectMake(WINSIZEX - 160, 290, 300, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			DrawText(UIDC, TEXT("구입"), strlen("구입"), &RectMake(WINSIZEX - 160, 520, 300, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			DrawText(UIDC, TEXT("구입"), strlen("구입"), &RectMake(WINSIZEX - 160, 750, 300, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);

			SetTextColor(UIDC, RGB(230, 25, 40));
			if (PtInRect(&RectMake(WINSIZEX - 160, 290, 300, 50), _ptMouse))
			{
				DrawText(UIDC, TEXT("구입"), strlen("구입"), &RectMake(WINSIZEX - 160, 290, 300, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
				if (KEYMANAGER->isOnceKeyDown(VK_LBUTTON) && _money >= stonePrice)
				{
					_money -= stonePrice;
					_stone[0] += 10;
					_stone[1] += 10;
				}

			}
			else if (PtInRect(&RectMake(WINSIZEX - 160, 520, 300, 50), _ptMouse))
			{
				DrawText(UIDC, TEXT("구입"), strlen("구입"), &RectMake(WINSIZEX - 160, 520, 300, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
				if (KEYMANAGER->isOnceKeyDown(VK_LBUTTON) && _money >= foodPrice)
				{
					_money -= foodPrice;
					_food[0] += 5;
					_food[1] += 5;
				}
			}
			else if (PtInRect(&RectMake(WINSIZEX - 160, 750, 300, 50), _ptMouse))
			{
				DrawText(UIDC, TEXT("구입"), strlen("구입"), &RectMake(WINSIZEX - 160, 750, 300, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
				if (KEYMANAGER->isOnceKeyDown(VK_LBUTTON) && _money >= treePrice)
				{
					_money -= treePrice;
					_currentXY.x++;
					_currentXY.y++;
					setTree();
					drawMap2(false);
				}
			}
		}

		SelectObject(UIDC, oldFont);
		DeleteObject(font);
		/*
		희진아 화이팅~!~!!!!!
		*/
	
	}

	dragMake();

	char str[128];
	if (_vMon.size() != 0)
	{
		for (int i = 0; i < TILEX * TILEY; i++)
		{
			if (_tiles[i].mon != -1)
			{
				sprintf_s(str, "a%d", _tiles[i].mon);
				if (_tiles[i].mon == 13)
					fdraw(str, DC, _tiles[i].rc.left + 10, _tiles[i].rc.top + 10, _vMon[findMonVec(i)].currentX, _vMon[findMonVec(i)].currentY);
				else
					fdraw(str, DC, _tiles[i].rc.left, _tiles[i].rc.top, _vMon[findMonVec(i)].currentX, _vMon[findMonVec(i)].currentY);
			}
		}
	}

	for (_viMon = _vMon.begin(); _viMon != _vMon.end(); _viMon++)
		(*_viMon).hpbar->render();

	_bullet->render();

	if (_side == true)
		IMAGEMANAGER->render("side", DC, CAMERAMANAGER->getCameraCenter().x - WINSIZEX / 2, CAMERAMANAGER->getCameraCenter().y - WINSIZEY / 2);
}

void MapTool::save()
{
	tagTile	_tiles2[TILEX * TILEY];

	for (int j = 0; j < TILEX * TILEY; j++)
	{
		if (_tiles[j].object == OBJ_TREE)
			_tiles[j].object == OBJ_NULL;
		_tiles2[j] = _tiles[j];
	}

	HANDLE	file;
	DWORD	save;
	file = CreateFile("map/myDungen.map", GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	WriteFile(file, _tiles2, sizeof(tagTile) * TILEX * TILEY, &save, NULL);
	_savem.current = _currentXY;
	_savem.food[0] = _food[0];
	_savem.food[1] = _food[1];
	_savem.stone[0] = _stone[0];
	_savem.stone[1] = _stone[1];
	_savem.money = _money;
	_savem.day = _day;

	WriteFile(file, &_savem, sizeof(SAVEM), &save, NULL);

	int size = _mM->getVDmon().size();
	WriteFile(file, &size, sizeof(int), &save, NULL);
	for(int i=0;i<size;i++)
		WriteFile(file, &_mM->getVDmon()[i], sizeof(tagMon2), &save, NULL);

	CloseHandle(file);
}

void MapTool::save(int i)
{
	tagTile	_tiles2[TILEX * TILEY];

	for (int j = 0; j < TILEX * TILEY; j++)
	{
		if (_tiles[j].object == OBJ_TREE)
			_tiles[j].object == OBJ_NULL;
		_tiles2[j] = _tiles[j];
	}
		
	HANDLE	file;
	DWORD	save;
	char str[128];
	sprintf_s(str, "map/slot%d.map", i);
	file = CreateFile(str, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	WriteFile(file, _tiles2, sizeof(tagTile) * TILEX * TILEY, &save, NULL);
	_savef = _save[i];
	WriteFile(file, &_savef, sizeof(SAVEF), &save, NULL);

	CloseHandle(file);
}

void MapTool::load()
{
	for (int i = 0; i < TILEX * TILEY; ++i)
	{
		if (_tiles[i].mon != -1)
		{
			for (int j = 0; j < _mM->getVDmon().size(); j++)
			{
				if (_mM->getVDmon()[j].num == _tiles[i].mon)
				{
					_mM->addDmon(_tiles[i].mon);
					break;
				}
			}
		}
	}

	HANDLE	file;
	DWORD	load;
	int size;
	vector<tagMon2>				vmon;
	vector<tagMon2>::iterator	vimon;

	ZeroMemory(&_tiles, sizeof(tagTile) * TILEX * TILEY);

	file = CreateFile("map/myDungen.map", GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	ReadFile(file, _tiles, sizeof(tagTile) * TILEX * TILEY, &load, NULL);
	ReadFile(file, &_savem, sizeof(SAVEM), &load, NULL);
	ReadFile(file, &size, sizeof(int), &load, NULL);
	for (int i = 0; i < size; i++)
	{
		tagMon2	mon;
		ZeroMemory(&mon, sizeof(tagMon2));
		ReadFile(file, &mon, sizeof(tagMon2), &load, NULL);
		vmon.push_back(mon);
	}
	_mM->setVDmon(vmon);

	_currentXY = _savem.current;
	_food[0] = _savem.food[0];
	_food[1]= _savem.food[1];
	_stone[0]= _savem.stone[0];
	_stone[1]=_savem.stone[1];
	_money= _savem.money;
	_day = _savem.day;

	for (_viMon = _vMon.begin(); _viMon != _vMon.end();)
	{
		_viMon = _vMon.erase(_viMon);
	}
	for (int i = 0; i < TILEX * TILEY; i++)
	{
		if (_tiles[i].mon != -1)
		{
			setMon(i);
			_monNum++;
		}
	}

	CloseHandle(file);
}

void MapTool::load(int i)
{
	for (int i = 0; i < TILEX * TILEY; ++i)
	{
		if (_tiles[i].mon != -1)
		{
			for (int j = 0; j < _mM->getVDmon().size(); j++)
			{
				if (_mM->getVDmon()[j].num == _tiles[i].mon)
				{
					_mM->addDmon(_tiles[i].mon);
					break;
				}
			}
		}
	}

	HANDLE	file;
	DWORD	load;
	char str[128];
	sprintf_s(str, "map/slot%d.map", i);

	ZeroMemory(&_tiles, sizeof(tagTile) * TILEX * TILEY);
	ZeroMemory(&_savef, sizeof(SAVEF));

	file = CreateFile(str, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	ReadFile(file, _tiles, sizeof(tagTile) * TILEX * TILEY, &load, NULL);
	ReadFile(file, &_savef, sizeof(SAVEF), &load, NULL);
	_save[i] = _savef;
	
	for (int i = 0; i < TILEX * TILEY; ++i)
	{
		if (_tiles[i].mon != -1)
		{
			for (int j = 0; j < _mM->getVDmon().size(); j++)
			{
				if (_mM->getVDmon()[j].num == _tiles[i].mon)
				{
					if (_mM->getVDmon()[j].have > 0)
						_mM->setHave(j, _mM->getVDmon()[j].have - 1);
					else
						_tiles[i].mon = -1;
					break;
				}
			}
		}
	}

	for (_viMon = _vMon.begin(); _viMon != _vMon.end();)
	{
		_viMon = _vMon.erase(_viMon);
	}
	for (int i = 0; i < TILEX * TILEY; i++)
	{
		if (_tiles[i].mon != -1)
		{
			setMon(i);
			_monNum++;
		}
	}

	CloseHandle(file);
}

void MapTool::setup()
{
	
	for (int i = 0; i < SAMPLETILEY; ++i)
	{
		for (int j = 0; j < SAMPLETILEX; ++j)
		{
			_sampleTile[i * SAMPLETILEX + j].terrainFrameX = j;
			_sampleTile[i * SAMPLETILEX + j].terrainFrameY = i;

			SetRect(&_sampleTile[i * SAMPLETILEX + j].rctile,
				(WINSIZEX - IMAGEMANAGER->findImage("sample")->getWidth()) + j * TILESIZE - SAMPLEX,
				i * TILESIZE+ SAMPLEY,
				(WINSIZEX - IMAGEMANAGER->findImage("sample")->getWidth()) + j * TILESIZE + TILESIZE- SAMPLEX,
				i * TILESIZE + TILESIZE+ SAMPLEY);
		}
	}

	for (int i = 0; i < TILEY; i++)
	{
		for (int j = 0; j < TILEX; j++)
		{
			SetRect(&_tiles[i * TILEX + j].rc, j * TILESIZE, i * TILESIZE, j* TILESIZE + TILESIZE, i * TILESIZE + TILESIZE);
		}
	}

	for (int i = 0; i < TILEX * TILEY; ++i)
	{
		_tiles[i].terrainFrameX = 0;
		_tiles[i].terrainFrameY = 0;
		_tiles[i].objFrameX = 0;
		_tiles[i].objFrameY = 0;
		setTFrame(i, 9, 0);
		_tiles[i].terrain = TR_NULL;
		_tiles[i].object = OBJ_NULL;
		
		_tiles[i].mon = -1;
		_tiles[i].mon2 = -1;
	}
}

void MapTool::setmap()
{
	POINT mouse = getMemDCPoint();
	for (int i = 0; i < 10; i++)
	{
		if (PtInRect(&_rc[i], _ptMouse))
		{
			_currentMon = -1;
			if (i == 0) _select = ROOM;
			else if (i == 1) _select = HROAD;
			else if (i == 2) _select = VROAD;
			else if (i == 3) _select = OBJDRAW;
			else if (i == 4) _select = ITEMDRAW;
			else if (i == 5)
			{
				_select = MONSTER;
				drawList(_currentMon);
			}
			else if (i == 6) _eraser = false;
			else if (i == 7) _eraser = true;
			else if (i == 8) _select = SAVE;
			else if (i == 9) _select = LOAD;
			break;
		}
	}

	if (_select != SAVE && _select != LOAD && _select != MONSTER && _select != ITEMDRAW)
	{
		for (int i = 0; i < SAMPLETILEX * SAMPLETILEY; i++)
		{
			if (PtInRect(&_sampleTile[i].rctile, _ptMouse))
			{
				if (objSelect(_sampleTile[i].terrainFrameX, _sampleTile[i].terrainFrameY) == OBJ_NULL)
				{
					set_floor(i);
					_currentTileT.x = _sampleTile[i].terrainFrameX;
					_currentTileT.y = _sampleTile[i].terrainFrameY;
					if (_select == TERRAINDRAW || _select == OBJDRAW)
						_select = TERRAINDRAW;
				}
				else
				{
					set_wall(i);
					if (_select == TERRAINDRAW || _select == OBJDRAW)
						_select = OBJDRAW;
				}
				break;
			}
		}
	}
}

int MapTool::findMon(int i)
{
	for (int j = 0; j < _vMon.size(); j++)
	{
		if(i== _vMon[j].tile)
			return j;
	}
}

TERRAIN MapTool::terrainSelect(int FrameX, int FrameY)
{
	return TR_FLOOR;
}

OBJECT MapTool::objSelect(int FrameX, int FrameY)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 12; j++)
		{
			if (FrameX == i && FrameY == j) return OBJ_WALL;
		}
	}

	return OBJ_NULL;
}

void MapTool::setTree()
{
	for (int i = 0; i < TILEX * TILEY; i++)
	{
		if (_tiles[i].object == OBJ_TREE)
		{
			_tiles[i].object = OBJ_NULL;
		}
	}

	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j <  1; j++)
		{
			setPosition(i * 100 + j, TR_NULL, OBJ_TREE, POS_5,_tree);
		}
	}
	for (int i = 0; i < 100; i++)	//오른쪽이빈나무
	{
		for (int j = 1; j < 2; j++)
		{
			setPosition(i * 100 + j, TR_NULL, OBJ_TREE, POS_6, _tree);
		}
	}
	for (int i = 0; i < 100; i++)	//왼쪽이빈나무
	{
		for (int j = _currentXY.x+2; j < _currentXY.x+3; j++)
		{
			setPosition(i * 100 + j, TR_NULL, OBJ_TREE, POS_4, _tree);
		}
	}
	for (int i = 0; i <100; i++)
	{
		for (int j = _currentXY.x+3; j < 100; j++)
		{
			setPosition(i * 100 + j, TR_NULL, OBJ_TREE, POS_5, _tree);
		}
	}


	for (int i = 0; i <4; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			setPosition(i * 100 + j, TR_NULL, OBJ_TREE, POS_5, _tree);
		}
	}
	for (int i = 4; i <5; i++)		//아래쪽이빈나무
	{
		for (int j = 1; j < 100; j++)
		{
			if(j<_currentXY.x+2)
				setPosition(i * 100 + j, TR_NULL, OBJ_TREE, POS_8, _tree);
			else
				setPosition(i * 100 + j, TR_NULL, OBJ_TREE, POS_5, _tree);
		}
	}
	for (int i = _currentXY.y+5; i <_currentXY.y+6; i++)		//윗쪽이빈나무
	{
		for (int j = 1; j < 100; j++)
		{
			_tiles[i * 100 + j].object = OBJ_TREE;
			if (j<_currentXY.x + 2)
				setPosition(i * 100 + j, TR_NULL, OBJ_TREE, POS_2, _tree);
			else
				setPosition(i * 100 + j, TR_NULL, OBJ_TREE, POS_5, _tree);
		}
	}
	for (int i = _currentXY.y+6; i <100; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			setPosition(i * 100 + j, TR_NULL, OBJ_TREE, POS_5, _tree);
		}
	}

	for (int i = 4; i <100; i++)
	{
		for (int j = 1; j < 100; j++)
		{
			if (i == 4 && j == 1)		//오른쪽아래가빈나무
			{
				setPosition(i * 100 + j, TR_NULL, OBJ_TREE, POS_11, _tree);
			}
			if (i == 4 && j == _currentXY.x+2)		//왼쪽아래가빈나무
			{
				setPosition(i * 100 + j, TR_NULL, OBJ_TREE, POS_12, _tree);
			}
			if (i == _currentXY.y+5 && j ==1)		//오른쪽위가빈나무
			{
				setPosition(i * 100 + j, TR_NULL, OBJ_TREE, POS_14, _tree);
			}
			if (i == _currentXY.y + 5 && j == _currentXY.x + 2)		//왼쪽위가빈나무
			{
				setPosition(i * 100 + j, TR_NULL, OBJ_TREE, POS_15, _tree);
			}
		}
	}
	for (int i = 0; i < TILEX * TILEY; i++)
	{
		if (_tiles[i].object == OBJ_TREE)
		{
			switch (_tiles[i].position)
			{
			case POS_2:
				setOFrame(i, 1 + 7 * (_tiles[i].type / 11), 3 + 3 * (_tiles[i].type % 11));
				break;
			case POS_4:
				setOFrame(i, 7 * (_tiles[i].type / 11), 4 + 3 * (_tiles[i].type % 11));
				break;
			case POS_5:
				setOFrame(i, 1 + 7 * (_tiles[i].type / 11), 4 + 3 * (_tiles[i].type % 11));
				break;
			case POS_6:
				setOFrame(i, 2 + 7 * (_tiles[i].type / 11), 4 + 3 * (_tiles[i].type % 11));
				break;
			case POS_8:
				setOFrame(i, 1 + 7 * (_tiles[i].type / 11), 5 + 3 * (_tiles[i].type % 11));
				break;
			case POS_11:
				setOFrame(i, 4 + 7 * (_tiles[i].type / 11), 3 + 3 * (_tiles[i].type % 11));
				break;
			case POS_12:
				setOFrame(i, 5 + 7 * (_tiles[i].type / 11), 3 + 3 * (_tiles[i].type % 11));

				break;
			case POS_14:
				setOFrame(i, 4 + 7 * (_tiles[i].type / 11), 4 + 3 * (_tiles[i].type % 11));
				break;
			case POS_15:
				setOFrame(i, 5 + 7 * (_tiles[i].type / 11), 4 + 3 * (_tiles[i].type % 11));
				break;
			default:
				break;
			}
		}
	}
}

void MapTool::setWall(int i)
{
	if (_tiles[i].object == OBJ_WALL)
	{
		switch (_tiles[i].position)
		{
		case POS_1:
			setOFrame(i, 0 + 7 * (_tiles[i].type / 16), 3 + 3 * (_tiles[i].type % 16));
			break;
		case POS_2:
			setOFrame(i, 1 + 7 * (_tiles[i].type / 16), 3 + 3 * (_tiles[i].type % 16));
			break;
		case POS_3:
			setOFrame(i, 2 + 7 * (_tiles[i].type / 16), 3 + 3 * (_tiles[i].type % 16));
			break;
		case POS_4:
			setOFrame(i, 0 + 7 * (_tiles[i].type / 16), 4 + 3 * (_tiles[i].type % 16));
			break;
		case POS_5:
			setOFrame(i, 1 + 7 * (_tiles[i].type / 16), 4 + 3 * (_tiles[i].type % 16));
			break;
		case POS_7:
			setOFrame(i, 0 + 7 * (_tiles[i].type / 16), 5 + 3 * (_tiles[i].type % 16));
			break;
		case POS_9:
			setOFrame(i, 2 + 7 * (_tiles[i].type / 16), 5 + 3 * (_tiles[i].type % 16));
			break;
		case POS_10:
			setOFrame(i, 3 + 7 * (_tiles[i].type / 16), 3 + 3 * (_tiles[i].type % 16));
			break;
		case POS_11:
			setOFrame(i, 4 + 7 * (_tiles[i].type / 16), 3 + 3 * (_tiles[i].type % 16));
			break;
		case POS_12:
			setOFrame(i, 5 + 7 * (_tiles[i].type / 16), 3 + 3 * (_tiles[i].type % 16));
			break;
		case POS_13:
			setOFrame(i, 3 + 7 * (_tiles[i].type / 16), 4 + 3 * (_tiles[i].type % 16));
			break;
		case POS_14:
			setOFrame(i, 4 + 7 * (_tiles[i].type / 16), 4 + 3 * (_tiles[i].type % 16));
			break;
		case POS_15:
			setOFrame(i, 5 + 7 * (_tiles[i].type / 16), 4 + 3 * (_tiles[i].type % 16));
			break;
		case POS_16:
			setOFrame(i, 3 + 7 * (_tiles[i].type / 16), 5 + 3 * (_tiles[i].type % 16));
			break;
		case POS_17:
			setOFrame(i, 4 + 7 * (_tiles[i].type / 16), 5 + 3 * (_tiles[i].type % 16));
			break;
		case POS_18:
			setOFrame(i, 5 + 7 * (_tiles[i].type / 16), 5 + 3 * (_tiles[i].type % 16));
			break;
		default:
			break;
		}
	}
}

void MapTool::setFloor(int i)
{
	if (_tiles[i].terrain == TR_FLOOR)
	{
		switch (_tiles[i].position)
		{
		case POS_1:
			setTFrame(i, 20 + 7 * (_tiles[i].type /16), 3 + 3 * (_tiles[i].type %16));
			break;
		case POS_2:
			setTFrame(i, 21 + 7 * (_tiles[i].type / 16), 3 + 3 * (_tiles[i].type % 16));
			break;
		case POS_3:
			setTFrame(i, 22 + 7 * (_tiles[i].type / 16), 3 + 3 * (_tiles[i].type % 16));
			break;
		case POS_4:
			setTFrame(i, 20 + 7 * (_tiles[i].type / 16), 4 + 3 * (_tiles[i].type % 16));
			break;
		case POS_5:
			setTFrame(i, 21 + 7 * (_tiles[i].type / 16), 4 + 3 * (_tiles[i].type % 16));
			break;
		case POS_6:
			setTFrame(i, 22 + 7 * (_tiles[i].type / 16), 4 + 3 * (_tiles[i].type % 16));
			break;
		case POS_7:
			setTFrame(i, 20 + 7 * (_tiles[i].type / 16), 5 + 3 * (_tiles[i].type % 16));
			break;
		case POS_8:
			setTFrame(i, 21 + 7 * (_tiles[i].type / 16), 5 + 3 * (_tiles[i].type % 16));
			break;
		case POS_9:
			setTFrame(i, 22 + 7 * (_tiles[i].type / 16), 5 + 3 * (_tiles[i].type % 16));
			break;
		case POS_10:
			setTFrame(i, 23 + 7 * (_tiles[i].type / 16), 3 + 3 * (_tiles[i].type % 16));
			break;
		case POS_12:
			setTFrame(i, 25 + 7 * (_tiles[i].type / 16), 3 + 3 * (_tiles[i].type % 16));
			break;
		case POS_14:
			setTFrame(i, 23 + 7 * (_tiles[i].type / 16), 4 + 3 * (_tiles[i].type % 16));
			break;
		case POS_15:
			setTFrame(i, 24 + 7 * (_tiles[i].type / 16), 4 + 3 * (_tiles[i].type % 16));
			break;
		case POS_16:
			setTFrame(i, 25 + 7 * (_tiles[i].type / 16), 4 + 3 * (_tiles[i].type % 16));
			break;
		case POS_17:
			setTFrame(i, 26 + 7 * (_tiles[i].type / 16), 4 + 3 * (_tiles[i].type % 16));
			break;
		case POS_18:
			setTFrame(i, 23 + 7 * (_tiles[i].type / 16), 5 + 3 * (_tiles[i].type % 16));
			break;
		default:
			break;
		}
	}
}

void MapTool::set_wall(int i)
{
int num = 0;

	for (int l = 0; l < 3; l++)
	{
		for (int k = 0; k < 12; k += 3, num += 4)
		{
			for (int j = 0; j < 4; j++)
			{
				if (_sampleTile[i].terrainFrameX == j && _sampleTile[i].terrainFrameY == k + l)
					_wall = j + num;
			}
		}
	}
}

void MapTool::set_floor(int i)
{
	int num = 0;

	for (int k = 0; k < 7; k++)
	{
		for (int j = 5; j < 9; j++)
		{
			if (_sampleTile[i].terrainFrameX == j && _sampleTile[i].terrainFrameY == k)
				_floor = j-5 + num;
		}
		num += 4;
	}
}

void MapTool::dragMake()
{
	int	temp;
	int count=0;
	int count2=0;
	int count3=-1;

	tagCurrentTile LT = _mouseTile[1];
	tagCurrentTile RB = _mouseTile[0];
	tagCurrentTile mouseTile;
	
	if (RB.y < LT.y)
	{
		temp = RB.y;
		RB.y = LT.y;
		LT.y = temp;
	}
	if (RB.x < LT.x)
	{
		temp = RB.x;
		RB.x = LT.x;
		LT.x = temp;
	}

	tagCurrentTile LT2 = LT;
	tagCurrentTile RB2 = RB;

	int x = RB.x - LT.x;
	int y = RB.y - LT.y;
	
	if (_select != MONSTER)
	{
		if (_tiles[_mouseTile[0].x + (_mouseTile[0].y) * 100].object == OBJ_TREE || _tiles[_mouseTile[0].x + (_mouseTile[0].y) * 100].object == OBJ_MON || _drag == -1 || _stone[1] - count < 0)
			IMAGEMANAGER->findImage("tile2")->render(DC, _tiles[_mouseTile[0].x + (_mouseTile[0].y) * 100].rc.left, _tiles[_mouseTile[0].x + (_mouseTile[0].y) * 100].rc.top);
		else
			IMAGEMANAGER->findImage("tile")->render(DC, _tiles[_mouseTile[0].x + (_mouseTile[0].y) * 100].rc.left, _tiles[_mouseTile[0].x + (_mouseTile[0].y) * 100].rc.top);
	}

	if (_drag == 11)
	{
		RECT rc[3];
		for (int i = 0; i < 3; i++)
		{
			rc[i] = RectMake(WINSIZEX - _mapX - 200, 190 + (i * 260), _mapX, _mapY);
		}
		for (int i = 0; i < 3; i++)
		{
			if (PtInRect(&rc[i], _ptMouse))
			{
				setMinimap(i);
				_save[i].current = _currentXY;
				_save[i].stone = _stone[0] - _stone[1];
				save(i);
			}
		}
	}
	else if (_drag == 12)
	{
		RECT rc[3];
		for (int i = 0; i < 3; i++)
		{
			rc[i] = RectMake(WINSIZEX - _mapX - 200, 190 + (i * 260), _mapX, _mapY);
		}
		for (int i = 0; i < 3; i++)
		{
			if (PtInRect(&rc[i], _ptMouse) && _save[i].stone<=_stone[0]&&_save[i].stone!=0)
			{
				load(i);
				setTree();
			}
		}
	}
	else if (_drag == 10&&_currentMon!=-1&&_mM->getDex(_mM->getVDmon()[_currentMon].num).food<=_food[1])
	{
		for (int i = -1; i < _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y+1; i++)
		{
			if (_mouseTile[0].x < 1 || _mouseTile[0].y < 1) continue;
			for (int j = -1; j < _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x+1; j++)
			{
				if (i == -1 || i == _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y || j == _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x || j == -1)
				{
					if (_tiles[_mouseTile[0].x + j + (_mouseTile[0].y + i) * 100].object == OBJ_MON)
					{
						_drag = 0;
						drawMap();
						return;
					}
					if (i == _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y && j == _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x)
					{
						_tiles[_mouseTile[0].x + (_mouseTile[0].y) * 100].mon = _mM->getVDmon()[_currentMon].num;
						for (int k = -1; k < _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y + 1; k++)
							for (int l = -1; l < _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x + 1; l++)
							{
								if (_tiles[_mouseTile[0].x + l + (_mouseTile[0].y + k) * 100].object == OBJ_NULL)
									_tiles[_mouseTile[0].x + l + (_mouseTile[0].y + k) * 100].object = OBJ_MON;
								if (k == 0 && l == 0)
									_tiles[_mouseTile[0].x + l + (_mouseTile[0].y + k) * 100].monPos = MPOS_1;
								else if (k == 0 && l == 1 && _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x>1)
									_tiles[_mouseTile[0].x + l + (_mouseTile[0].y + k) * 100].monPos = MPOS_2;
								else if (k == 0 && l == 2 && _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x>2)
									_tiles[_mouseTile[0].x + l + (_mouseTile[0].y + k) * 100].monPos = MPOS_3;
								else if (k == 1 && l == 0 && _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y>1)
									_tiles[_mouseTile[0].x + l + (_mouseTile[0].y + k) * 100].monPos = MPOS_4;
								else if (k == 1 && l == 1 && _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x>1 && _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y>1)
									_tiles[_mouseTile[0].x + l + (_mouseTile[0].y + k) * 100].monPos = MPOS_5;
								else if (k == 1 && l == 2 && _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x>2 && _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y>1)
									_tiles[_mouseTile[0].x + l + (_mouseTile[0].y + k) * 100].monPos = MPOS_6;
								else if (k == 2 && l == 0 && _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y>2)
									_tiles[_mouseTile[0].x + l + (_mouseTile[0].y + k) * 100].monPos = MPOS_7;
								else if (k == 2 && l == 1 && _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x>1 && _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y>2)
									_tiles[_mouseTile[0].x + l + (_mouseTile[0].y + k) * 100].monPos = MPOS_8;
								else if (k == 2 && l == 2 && _mM->getDex(_mM->getVDmon()[_currentMon].num).size.x>2 && _mM->getDex(_mM->getVDmon()[_currentMon].num).size.y>2)\
									_tiles[_mouseTile[0].x + l + (_mouseTile[0].y + k) * 100].monPos = MPOS_9;
								else
									_tiles[_mouseTile[0].x + l + (_mouseTile[0].y + k) * 100].monPos = MPOS_0;
							}
						_food[1] -= _mM->getDex(_mM->getVDmon()[_currentMon].num).food;
						setMon(_mouseTile[0].x + (_mouseTile[0].y) * 100);

						_mM->setHave(_currentMon, _mM->getVDmon()[_currentMon].have - 1);
						if (_mM->getVDmon()[_currentMon].have == 0)
						{
							_mM->eraseDmon(_currentMon);
							_currentMon = -1;
						}
						drawList(_currentMon);
						_drag = 0;
						_monNum++;
						drawMap();
						return;
					}
				}
				else if (_tiles[_mouseTile[0].x + j + (_mouseTile[0].y + i) * 100].terrain == TR_FLOOR && _tiles[_mouseTile[0].x + j + (_mouseTile[0].y + i) * 100].object != OBJ_MON)
				{
					
				}
				else
				{
					_drag = 0;
					drawMap();
					return;
				}
			}
		}
	}
	else if (_drag != 0 && _select != SAVE && _select != LOAD && _select != ITEMDRAW )
	{
		for (int i = 0; i <= y; i++)
		{
			for (int j = 0; j <= x; j++)
			{
				temp = LT.x + j + (LT.y + i) * 100;
				if (_select==OBJDRAW)
				{
					if (_tiles[temp].terrain == TR_FLOOR|| _tiles[temp].object==OBJ_WALL) continue;
					if (_tiles[temp - 1].terrain == TR_FLOOR|| _tiles[temp + 1].terrain == TR_FLOOR || _tiles[temp - 100].terrain == TR_FLOOR || _tiles[temp + 100].terrain == TR_FLOOR
						|| _tiles[temp - 101].terrain == TR_FLOOR || _tiles[temp - 99].terrain == TR_FLOOR || _tiles[temp +101].terrain == TR_FLOOR || _tiles[temp +99].terrain == TR_FLOOR) count++;
				}
				else
				{
					if (checkstoneTile(temp) == 0)
						count++;
				}
				if (_select != MONSTER)
					IMAGEMANAGER->findImage("tile")->render(DC, _tiles[temp].rc.left, _tiles[temp].rc.top);
				if (_tiles[temp].object == OBJ_TREE || _tiles[temp].object == OBJ_MON|| _drag == -1 || _drag == 7 || _drag == 8)
				{
					IMAGEMANAGER->findImage("tile2")->render(DC, _tiles[temp].rc.left, _tiles[temp].rc.top);
					if (_tiles[temp].object == OBJ_TREE)
					{
						if (_tiles[temp].position == POS_6)
							LT2.x = 2;
						else if (_tiles[temp].position == POS_8)
							LT2.y = 5;
						else if (_tiles[temp].position == POS_2)
							RB2.y = _currentXY.y + 4;
						else if (_tiles[temp].position == POS_4)
							RB2.x = _currentXY.x + 1;
					}
					if (_tiles[temp].object == OBJ_MON)
					{
						count += 10000;
					}
				}
			}
		}
		
		x = RB2.x - LT2.x;
		y = RB2.y - LT2.y;

		if (_select == ROOM && (x < 2 || y < 2))
		{
			for (int i = 0; i <= y; i++)
			{
				for (int j = 0; j <= x; j++)
				{
					temp = LT2.x + j + (LT2.y + i) * 100;
					if (count3 == -1)	count3 = 0;
					if ((i == 0 || i == y) && _tiles[temp - 1].terrain == TR_FLOOR || _tiles[temp + 1].terrain == TR_FLOOR || _tiles[temp - 100].terrain == TR_FLOOR || _tiles[temp + 100].terrain == TR_FLOOR
						|| _tiles[temp - 101].terrain == TR_FLOOR || _tiles[temp - 99].terrain == TR_FLOOR || _tiles[temp + 101].terrain == TR_FLOOR || _tiles[temp + 99].terrain == TR_FLOOR) count3++;
					if ((j == 0 || j == x) && _tiles[temp - 1].terrain == TR_FLOOR || _tiles[temp + 1].terrain == TR_FLOOR || _tiles[temp - 100].terrain == TR_FLOOR || _tiles[temp + 100].terrain == TR_FLOOR
						|| _tiles[temp - 101].terrain == TR_FLOOR || _tiles[temp - 99].terrain == TR_FLOOR || _tiles[temp + 101].terrain == TR_FLOOR || _tiles[temp + 99].terrain == TR_FLOOR) count3++;
				}
			}
		}

		for (int i = 0; i <= y; i++)
		{
			for (int j = 0; j <= x; j++)
			{
				temp = LT2.x + j + (LT2.y + i) * 100;
				if (_stone[1] - count < 0||(_select==OBJDRAW&&count==0)|| (_select == ROOM && count3 == 0))
				{ 
					IMAGEMANAGER->findImage("tile2")->render(DC, _tiles[temp].rc.left, _tiles[temp].rc.top);
					if (_drag == 6&& _tiles[temp].object==OBJ_WALL)
					{
						_tiles[temp].type=_wall;
						setWall(temp);
					}
				}
				else
				{
					if (_drag == 2 && _tiles[temp].object != OBJ_TREE)
					{
						setPosition(temp, TR_FLOOR, OBJ_NULL, POS_5, _floor);
						if (j == 0 || j == x)
						{
							setPosition(temp, TR_NULL, OBJ_WALL, POS_4, _wall);
						}
						else if (i == 0 || i == y)
						{
							setPosition(temp, TR_NULL, OBJ_WALL, POS_2, _wall);
						}
						else if (i == 1)
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_2, _floor);
						}
						else if (i == y - 1)
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_8, _floor);
						}
						else if (j == 1)
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_4, _floor);
						}
						else if (j == x - 1)
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_6, _floor);
						}

						if (j == 0 && i == 0)
							setPosition(temp, TR_NULL, OBJ_WALL, POS_1, _wall);
						else if (j == 0 && i == y)
							setPosition(temp, TR_NULL, OBJ_WALL, POS_7, _wall);
						else if (j == x && i == 0)
							setPosition(temp, TR_NULL, OBJ_WALL, POS_3, _wall);
						else if (j == x && i == y)
							setPosition(temp, TR_NULL, OBJ_WALL, POS_9, _wall);
						if (x != 1 && y != 1)
						{
							if (j == 1 && i == 1)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_1, _floor);
							else if (j == 1 && i == y - 1)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_7, _floor);
							else if (j == x - 1 && i == 1)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_3, _floor);
							else if (j == x - 1 && i == y - 1)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_9, _floor);
						}
						if (x == 2 && j == 1 && i != 0 && i != y)
						{
							if (i == 1)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_10, _floor);
							else if (i == y - 1)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_18, _floor);
							else
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_14, _floor);
						}
						if (y == 2 && i == 1 && j != 0 && j != x)
						{
							if (j == 1)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_15, _floor);
							else if (j == x - 1)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_17, _floor);
							else
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_16, _floor);
						}
						if (x==y&&x==0)
							setPosition(temp, TR_NULL, OBJ_WALL, POS_5, _wall);

						setWall(temp);
						setFloor(temp);
					}
					if (_drag == 3 && _tiles[temp].object != OBJ_TREE)
					{
						setPosition(temp, TR_FLOOR, OBJ_NULL, POS_5, _floor);

						if (i == 1)
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_2, _floor);
						}
						else if (i == y - 1)
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_8, _floor);
						}
						else if (j == 0)
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_4, _floor);
						}
						else if (j == x )
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_6, _floor);
						}

						if (x != 1 && y != 1)
						{
							if (j == 0 && i == 1)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_1, _floor);
							else if (j == 0 && i == y - 1)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_7, _floor);
							else if (j == x && i == 1)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_3, _floor);
							else if (j == x && i == y - 1)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_9, _floor);
						}
						if (y == 2 && i == 1)
						{
							if (j == 0)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_15, _floor);
							else if (j == x)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_17, _floor);
							else
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_16, _floor);
						}
						if (i == 0 || i == y)
						{
							setPosition(temp, TR_NULL, OBJ_WALL, POS_2, _wall);
						}
						if (_mouseTile[0].y == _mouseTile[1].y&&_mouseTile[0].x == _mouseTile[1].x)
							setPosition(temp, TR_NULL, OBJ_WALL, POS_5, _wall);

						setWall(temp);
						setFloor(temp);
					}
					if (_drag == 4 && _tiles[temp].object != OBJ_TREE)
					{
						setPosition(temp, TR_FLOOR, OBJ_NULL, POS_5, _floor);
						if (i == 0)
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_2, _floor);
						}
						else if (i == y)
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_8, _floor);
						}
						else if (j == 1)
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_4, _floor);
						}
						else if (j == x - 1)
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_6, _floor);
						}

						if (x != 1 && y != 1)
						{
							if (j == 1 && i ==0)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_1, _floor);
							else if (j == 1 && i == y )
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_7, _floor);
							else if (j == x - 1 && i == 0)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_3, _floor);
							else if (j == x - 1 && i == y)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_9, _floor);
						}
						if (x == 2 && j == 1)
						{
							if (i == 0)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_10, _floor);
							else if (i == y )
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_18, _floor);
							else
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_14, _floor);
						}
						if (j == 0 || j == x)
						{
							setPosition(temp, TR_NULL, OBJ_WALL, POS_4, _wall);
						}
						if (_mouseTile[0].y == _mouseTile[1].y&&_mouseTile[0].x == _mouseTile[1].x)
							setPosition(temp, TR_NULL, OBJ_WALL, POS_5, _wall);

						setWall(temp);
						setFloor(temp);
					}
					if (_drag == 5 && _tiles[temp].object != OBJ_TREE && _stone[1] >= 0)
					{
						setPosition(temp, TR_FLOOR, OBJ_NULL, POS_5, _floor);
						if (i == 0)
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_2, _floor);
						}
						else if (i == y )
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_8, _floor);
						}
						else if (j == 0)
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_4, _floor);
						}
						else if (j == x)
						{
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_6, _floor);
						}
						if (x != 0 && y != 0)
						{
							if (j == 0 && i == 0)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_1, _floor);
							else if (j == 0 && i == y )
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_7, _floor);
							else if (j == x  && i == 0)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_3, _floor);
							else if (j == x  && i == y )
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_9, _floor);
						}
						if (x == 0 && j == 0)
						{
							if (i == 0)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_10, _floor);
							else if (i == y)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_18, _floor);
							else
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_14, _floor);
						}
						if (y == 0 && i == 0)
						{
							if (j == 0)
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_15, _floor);
							else if (j == x )
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_17, _floor);
							else
								setPosition(temp, TR_FLOOR, OBJ_NULL, POS_16, _floor);
						}
						if (_mouseTile[0].y == _mouseTile[1].y&&_mouseTile[0].x == _mouseTile[1].x)
							setPosition(temp, TR_FLOOR, OBJ_NULL, POS_12, _floor);
						setFloor(temp);
					}
					if (_drag == 6 && _tiles[temp].object != OBJ_TREE)
					{
						count2 = 0;
						if (_tiles[temp].terrain == TR_FLOOR) continue;
						if (_tiles[temp - 1].terrain == TR_FLOOR) count2 += 10;
						if (_tiles[temp + 1].terrain == TR_FLOOR) count2 += 12;
						if (_tiles[temp - 100].terrain == TR_FLOOR) count2 += 15;
						if (_tiles[temp + 100].terrain == TR_FLOOR) count2 += 19;
						if (count2 == 10)
						{
							if(_tiles[temp - 99].terrain == TR_FLOOR|| _tiles[temp + 101].terrain == TR_FLOOR)
								setPosition(temp, TR_NULL, OBJ_WALL, POS_13, _wall);
							else
								setPosition(temp, TR_NULL, OBJ_WALL, POS_4, _wall);
						}
						else if(count2 == 12)
						{
							if (_tiles[temp - 101].terrain == TR_FLOOR || _tiles[temp + 99].terrain == TR_FLOOR)
								setPosition(temp, TR_NULL, OBJ_WALL, POS_15, _wall);
							else
							setPosition(temp, TR_NULL, OBJ_WALL, POS_4, _wall);
						}
						else if (count2 == 15)
						{
							if (_tiles[temp + 101].terrain == TR_FLOOR || _tiles[temp + 99].terrain == TR_FLOOR)
								setPosition(temp, TR_NULL, OBJ_WALL, POS_11, _wall);
							else
							setPosition(temp, TR_NULL, OBJ_WALL, POS_2, _wall);
						}
						else if(count2 == 19)
						{
							if (_tiles[temp - 99].terrain == TR_FLOOR || _tiles[temp - 101].terrain == TR_FLOOR)
								setPosition(temp, TR_NULL, OBJ_WALL, POS_17, _wall);
							else
							setPosition(temp, TR_NULL, OBJ_WALL, POS_2, _wall);
						}
						else if(count2==22)	setPosition(temp, TR_NULL, OBJ_WALL, POS_4,_wall);
						else if(count2==34)	setPosition(temp, TR_NULL, OBJ_WALL, POS_2,_wall);
						else if(count2==27)	setPosition(temp, TR_NULL, OBJ_WALL, POS_3,_wall);
						else if(count2==25)	setPosition(temp, TR_NULL, OBJ_WALL, POS_1,_wall);
						else if(count2==31)	setPosition(temp, TR_NULL, OBJ_WALL, POS_9,_wall);
						else if(count2==29)	setPosition(temp, TR_NULL, OBJ_WALL, POS_7,_wall);
						else if(count2==37)	setPosition(temp, TR_NULL, OBJ_WALL, POS_12, _wall);
						else if(count2==46)	setPosition(temp, TR_NULL, OBJ_WALL, POS_18, _wall);
						else if(count2==44)	setPosition(temp, TR_NULL, OBJ_WALL, POS_16, _wall);
						else if(count2==41|| count2 == 56)	setPosition(temp, TR_NULL, OBJ_WALL, POS_5, _wall);
						else if (count2 == 0)
						{
							if (_tiles[temp - 101].terrain == TR_FLOOR) count2 += 10;
							if (_tiles[temp + 101].terrain == TR_FLOOR) count2 += 12;
							if (_tiles[temp - 99].terrain == TR_FLOOR) count2 += 15;
							if (_tiles[temp + 99].terrain == TR_FLOOR) count2 += 19;
							if (count2 == 22 || count2 == 34 || count2 == 37 || count2 == 46 || count2 == 41 || count2 == 44 || count2 == 56)	setPosition(temp, TR_NULL, OBJ_WALL, POS_14, _wall);
							else if (count2 == 10)	setPosition(temp, TR_NULL, OBJ_WALL, POS_9,_wall);
							else if (count2 == 12)	setPosition(temp, TR_NULL, OBJ_WALL, POS_1,_wall);
							else if (count2 == 15)	setPosition(temp, TR_NULL, OBJ_WALL, POS_7,_wall);
							else if (count2 == 19)	setPosition(temp, TR_NULL, OBJ_WALL, POS_3,_wall);
							else if (count2 == 27)	setPosition(temp, TR_NULL, OBJ_WALL, POS_13,_wall);
							else if (count2 == 25)	setPosition(temp, TR_NULL, OBJ_WALL, POS_17,_wall);
							else if (count2 == 31)	setPosition(temp, TR_NULL, OBJ_WALL, POS_11,_wall);
							else if (count2 == 29)	setPosition(temp, TR_NULL, OBJ_WALL, POS_15,_wall);
						}
						setWall(temp);
					}
				}
				if (_drag == 7 && _tiles[temp].object != OBJ_TREE)
				{
					if (_tiles[temp].object == OBJ_MON && _tiles[temp].monPos != MPOS_0 /*&&(i==0||j==0)*/)
					{
						mouseTile = { LT2.x + j , LT2.y + i };
						cancelMon(mouseTile,true,false);
					}

					_tiles[temp].terrainFrameX = 9;
					_tiles[temp].terrainFrameY = 0;
					_tiles[temp].objFrameX = NULL;
					_tiles[temp].objFrameY = NULL;

					_tiles[temp].object = OBJ_NULL;

					InvalidateRect(_hWnd, NULL, false);

					if (j == x && i == y)
						drawList(_currentMon);
				}
				else if (_drag == 8&& _tiles[temp].object!=OBJ_TREE)
				{
					if (_tiles[temp].object == OBJ_MON && _tiles[temp].monPos!=MPOS_0 /*&& (i == 0 || j == 0)*/)
					{
						mouseTile = { LT2.x + j , LT2.y + i };
						cancelMon(mouseTile, true,false);
					}

					_tiles[temp].terrainFrameX = 9;
					_tiles[temp].terrainFrameY = 0;
					_tiles[temp].objFrameX = NULL;
					_tiles[temp].objFrameY = NULL;

					_tiles[temp].terrain =TR_NULL;
					_tiles[temp].object = OBJ_NULL;

					InvalidateRect(_hWnd, NULL, false);

					if (j == x && i == y)
						drawList(_currentMon);
				}
			}
		}
	}
	if (_drag != 0&&_drag != 1 && _drag != -1)
	{
		_drag = 0;
		floorDir();
		wallDir();
		checkstone();
		drawMap();
	}

}

void MapTool::wallDir()
{
	int count = 0;
	for (int i = 0; i < TILEX * (_currentXY.y + 6); i++)
	{
		count = 0;
		if (_tiles[i].object != OBJ_WALL) continue;
		if (_tiles[i - 1].terrain == TR_FLOOR) count += 10;
		if (_tiles[i + 1].terrain == TR_FLOOR) count += 12;
		if (_tiles[i - 100].terrain == TR_FLOOR) count += 15;
		if (_tiles[i + 100].terrain == TR_FLOOR) count += 19;
		if (count == 10)
		{
			if (_tiles[i - 99].terrain == TR_FLOOR || _tiles[i + 101].terrain == TR_FLOOR)
				setPosition(i, TR_NULL, OBJ_WALL, POS_13, _tiles[i].type);
			else
				setPosition(i, TR_NULL, OBJ_WALL, POS_4, _tiles[i].type);
		}
		else if (count == 12)
		{
			if (_tiles[i - 101].terrain == TR_FLOOR || _tiles[i + 99].terrain == TR_FLOOR)
				setPosition(i, TR_NULL, OBJ_WALL, POS_15, _tiles[i].type);
			else
				setPosition(i, TR_NULL, OBJ_WALL, POS_4, _tiles[i].type);
		}
		else if (count == 15)
		{
			if (_tiles[i + 101].terrain == TR_FLOOR || _tiles[i + 99].terrain == TR_FLOOR)
				setPosition(i, TR_NULL, OBJ_WALL, POS_11, _tiles[i].type);
			else
				setPosition(i, TR_NULL, OBJ_WALL, POS_2, _tiles[i].type);
		}
		else if (count == 19)
		{
			if (_tiles[i - 99].terrain == TR_FLOOR || _tiles[i - 101].terrain == TR_FLOOR)
				setPosition(i, TR_NULL, OBJ_WALL, POS_17, _tiles[i].type);
			else
				setPosition(i, TR_NULL, OBJ_WALL, POS_2, _tiles[i].type);
		}
		else if (count == 22)	setPosition(i, TR_NULL, OBJ_WALL, POS_4,_tiles[i].type);
		else if (count == 34)	setPosition(i, TR_NULL, OBJ_WALL, POS_2,_tiles[i].type);
		else if (count == 27)	setPosition(i, TR_NULL, OBJ_WALL, POS_3,_tiles[i].type);
		else if (count == 25)	setPosition(i, TR_NULL, OBJ_WALL, POS_1,_tiles[i].type);
		else if (count == 31)	setPosition(i, TR_NULL, OBJ_WALL, POS_9,_tiles[i].type);
		else if (count == 29)	setPosition(i, TR_NULL, OBJ_WALL, POS_7,_tiles[i].type);
		else if (count == 37)	setPosition(i, TR_NULL, OBJ_WALL, POS_12, _tiles[i].type);
		else if (count == 46)	setPosition(i, TR_NULL, OBJ_WALL, POS_18, _tiles[i].type);
		else if (count == 44)	setPosition(i, TR_NULL, OBJ_WALL, POS_16, _tiles[i].type);
		else if (count == 41 || count == 56)	setPosition(i, TR_NULL, OBJ_WALL, POS_5, _tiles[i].type);
		else if (count == 0)
		{
			if (_tiles[i - 101].terrain == TR_FLOOR) count += 10;
			if (_tiles[i + 101].terrain == TR_FLOOR) count += 12;
			if (_tiles[i - 99].terrain == TR_FLOOR) count += 15;
			if (_tiles[i + 99].terrain == TR_FLOOR) count += 19;
			if (count == 22 || count == 34 || count == 37 || count == 46 || count == 41 || count == 44 || count == 56)	setPosition(i, TR_NULL, OBJ_WALL, POS_14, _tiles[i].type);
			else if (count == 10)	setPosition(i, TR_NULL, OBJ_WALL, POS_9, _tiles[i].type);
			else if (count == 12)	setPosition(i, TR_NULL, OBJ_WALL, POS_1, _tiles[i].type);
			else if (count == 15)	setPosition(i, TR_NULL, OBJ_WALL, POS_7, _tiles[i].type);
			else if (count == 19)	setPosition(i, TR_NULL, OBJ_WALL, POS_3, _tiles[i].type);
			else if (count == 27)	setPosition(i, TR_NULL, OBJ_WALL, POS_13, _tiles[i].type);
			else if (count == 25)	setPosition(i, TR_NULL, OBJ_WALL, POS_17, _tiles[i].type);
			else if (count == 31)	setPosition(i, TR_NULL, OBJ_WALL, POS_11, _tiles[i].type);
			else if (count == 29)	setPosition(i, TR_NULL, OBJ_WALL, POS_15, _tiles[i].type);
			else if (count == 0)
			{
				setPosition(i, TR_NULL, OBJ_NULL, POS_15, _tiles[i].type);
				_tiles[i].terrainFrameX = 9;
				_tiles[i].terrainFrameY = 0;
			}
		}
		setWall(i);
	}
}

void MapTool::floorDir()
{
	int count = 0;
	for (int i = 0; i < TILEX * (_currentXY.y + 6); i++)
	{
		count = 0;
		if (_tiles[i].terrain != TR_FLOOR) continue;
		if (_tiles[i - 1].terrain == TR_FLOOR&& _tiles[i - 1].type== _tiles[i].type) count += 10;
		if (_tiles[i + 1].terrain == TR_FLOOR && _tiles[i + 1].type == _tiles[i].type) count += 12;
		if (_tiles[i - 100].terrain == TR_FLOOR && _tiles[i - 100].type == _tiles[i].type) count += 15;
		if (_tiles[i + 100].terrain == TR_FLOOR && _tiles[i + 100].type == _tiles[i].type) count += 19;
		if (count == 10)	setPosition(i, TR_FLOOR, _tiles[i].object, POS_17, _tiles[i].type);
		else if (count == 12)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_15, _tiles[i].type);
		else if (count == 15)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_18, _tiles[i].type);
		else if (count == 19)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_10, _tiles[i].type);
		else if (count == 22)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_16, _tiles[i].type);
		else if (count == 27)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_7, _tiles[i].type);
		else if (count == 34)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_14, _tiles[i].type);
		else if (count == 25)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_9, _tiles[i].type);
		else if (count == 31)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_1, _tiles[i].type);
		else if (count == 29)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_3, _tiles[i].type);
		else if (count == 37)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_8, _tiles[i].type);
		else if (count == 46)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_4, _tiles[i].type);
		else if (count == 41)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_2, _tiles[i].type);
		else if (count == 44)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_6, _tiles[i].type);
		else if (count == 0)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_12, _tiles[i].type);
		else if (count == 56)	setPosition(i, TR_FLOOR,_tiles[i].object, POS_5, _tiles[i].type);
		setFloor(i);
	}
}

void MapTool::setPosition(int i, TERRAIN ter, OBJECT obj, POS pos, int type)
{
	_tiles[i].terrain = ter;
	_tiles[i].object = obj;
	_tiles[i].position = pos;
	_tiles[i].type = type;
}

void MapTool::setOFrame(int i, int x, int y)
{
	_tiles[i].objFrameX = x;
	_tiles[i].objFrameY = y;
}

void MapTool::setTFrame(int i, int x, int y)
{
	_tiles[i].terrainFrameX = x;
	_tiles[i].terrainFrameY = y;
}

void MapTool::checkstone()
{
	int count=0;
	for (int i = 0; i < TILEX * (_currentXY.y + 5); i++)
	{
		if ((_tiles[i].object != OBJ_NULL || _tiles[i].terrain != TR_NULL)&& _tiles[i].object != OBJ_TREE)	count++;
	}
	_stone[1] = _stone[0] - count;
}

int MapTool::checkstoneTile(int i)
{
	if ((_tiles[i].object != OBJ_NULL || _tiles[i].terrain != TR_NULL) && _tiles[i].object != OBJ_TREE)
		return 1;
	else 
		return 0;
}

void MapTool::setMinimap(int i)
{
	PatBlt(_tempImg->getMemDC(), 0, 0, (_currentXY.x + 4) * TILESIZE, (_currentXY.y + 7) * TILESIZE, WHITENESS);

	for (int i = 5; i < _currentXY.y + 5; i++)
	{
		for (int j = 2; j < _currentXY.x + 2; j++)
		{
			if (_tiles[i * TILEX + j].object == OBJ_NULL && _tiles[i * TILEX + j].terrain == TR_NULL) continue;
			if(_tiles[i * TILEX + j].terrain==TR_FLOOR)
				IMAGEMANAGER->frameRender("map", _tempImg->getMemDC(), _tiles[(i - 5) * TILEX + j-2].rc.left, _tiles[(i - 5) * TILEX + j - 2].rc.top, 
					_tiles[i * TILEX + j].terrainFrameX, _tiles[i * TILEX + j].terrainFrameY);
			else if (_tiles[i * TILEX + j].object==OBJ_WALL)
				IMAGEMANAGER->frameRender("map", _tempImg->getMemDC(), _tiles[(i - 5) * TILEX + j-2].rc.left, _tiles[(i - 5) * TILEX + j - 2].rc.top, 
					_tiles[i * TILEX + j].objFrameX, _tiles[i * TILEX + j].objFrameY);
			}
	}
	PatBlt(_minimap[i]->getMemDC(), 0, 0, _mapX, _mapY, WHITENESS);
	StretchBlt(_minimap[i]->getMemDC(), 0, 0, _mapX, _mapY, _tempImg->getMemDC(), 0, 0, _currentXY.x * TILESIZE, _currentXY.y * TILESIZE, SRCCOPY);
}
 
void MapTool::setMinimap2()
{
	tagTile	_tiles2[TILEX * TILEY];

	for (int i = 0; i < 3; i++)
	{
		HANDLE	file;
		DWORD	load;
		char str[128];
		sprintf_s(str, "map/slot%d.map", i);

		ZeroMemory(&_tiles2, sizeof(tagTile) * TILEX * TILEY);
		ZeroMemory(&_savef, sizeof(SAVEF));

		file = CreateFile(str, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		ReadFile(file, _tiles2, sizeof(tagTile) * TILEX * TILEY, &load, NULL);
		ReadFile(file, &_savef, sizeof(SAVEF), &load, NULL);
		_save[i] = _savef;

		CloseHandle(file);

		PatBlt(_tempImg->getMemDC(), 0, 0, (_currentXY.x + 4) * TILESIZE, (_currentXY.y + 7) * TILESIZE, WHITENESS);

		for (int i = 3; i < _currentXY.y + 5; i++)
		{
			for (int j = 0; j < _currentXY.x + 2; j++)
			{
				if (_tiles2[i * TILEX + j].object == OBJ_NULL && _tiles2[i * TILEX + j].terrain == TR_NULL) continue;
				if (_tiles2[i * TILEX + j].terrain == TR_FLOOR)
					IMAGEMANAGER->frameRender("map", _tempImg->getMemDC(), _tiles2[(i - 5) * TILEX + j - 2].rc.left, _tiles2[(i - 5) * TILEX + j - 2].rc.top, 
						_tiles2[i * TILEX + j].terrainFrameX, _tiles2[i * TILEX + j].terrainFrameY);
				else if (_tiles2[i * TILEX + j].object == OBJ_WALL)
					IMAGEMANAGER->frameRender("map", _tempImg->getMemDC(), _tiles2[(i - 5) * TILEX + j - 2].rc.left, _tiles2[(i - 5) * TILEX + j - 2].rc.top, 
						_tiles2[i * TILEX + j].objFrameX, _tiles2[i * TILEX + j].objFrameY);
				}
		}
		PatBlt(_minimap[i]->getMemDC(), 0, 0, _mapX, _mapY, WHITENESS);
		StretchBlt(_minimap[i]->getMemDC(), 0, 0, _mapX, _mapY, _tempImg->getMemDC(), 0, 0, (_currentXY.x) * TILESIZE, (_currentXY.y) * TILESIZE, SRCCOPY);
	}
}

void MapTool::drawMap()
{
	char str[128];
	// 지형
	for (int i = 0; i < TILEX * TILEY; i++)
	{
		if (_tiles[i].rc.left < WINSIZEX + CAMERAMANAGER->getCameraCenter().x - WINSIZEX / 2 && _tiles[i].rc.top < WINSIZEY + CAMERAMANAGER->getCameraCenter().y - WINSIZEY / 2
			&& _tiles[i].rc.right > CAMERAMANAGER->getCameraCenter().x - WINSIZEX / 2 && _tiles[i].rc.bottom > CAMERAMANAGER->getCameraCenter().y - WINSIZEY / 2)
			fdraw("map", MAP, _tiles[i].rc.left, _tiles[i].rc.top, _tiles[i].terrainFrameX, _tiles[i].terrainFrameY);
	}

	// 오브젝트
	for (int i = 0; i < TILEX * TILEY; i++)
	{
		char str[128];
		// 오브젝트 속성이 아니면 그리지마
		if (_tiles[i].object == OBJ_NULL) continue;
		if (_tiles[i].rc.left < WINSIZEX + CAMERAMANAGER->getCameraCenter().x - WINSIZEX / 2 && _tiles[i].rc.top < WINSIZEY + CAMERAMANAGER->getCameraCenter().y - WINSIZEY / 2
			&& _tiles[i].rc.right > CAMERAMANAGER->getCameraCenter().x - WINSIZEX / 2 && _tiles[i].rc.bottom > CAMERAMANAGER->getCameraCenter().y - WINSIZEY / 2)
		{

			if (_tiles[i].object == OBJ_WALL)
				fdraw("map", MAP, _tiles[i].rc.left, _tiles[i].rc.top, _tiles[i].objFrameX, _tiles[i].objFrameY);
			else if (_tiles[i].object == OBJ_TREE)
				IMAGEMANAGER->frameRender("tree", MAP, _tiles[i].rc.left, _tiles[i].rc.top, _tiles[i].objFrameX, _tiles[i].objFrameY);
			else
				IMAGEMANAGER->frameRender("samplemap", MAP, _tiles[i].rc.left, _tiles[i].rc.top, _tiles[i].objFrameX, _tiles[i].objFrameY);

			if (_tiles[i].object == OBJ_MON && _tiles[i].monPos == MPOS_0)
				draw("tile", MAP, _tiles[i].rc.left, _tiles[i].rc.top);
		}
	}
	if (_vRoad.size() > 0)
	{
		for (_viRoad = _vRoad.begin(); _viRoad != _vRoad.end(); _viRoad++)
		{
			IMAGEMANAGER->render("road", MAP, _tiles[*_viRoad].rc.left, _tiles[*_viRoad].rc.top);
		}
	}
}

void MapTool::drawMap2(bool first)
{
	char str[128];
	// 지형
	for (int i = 0; i < TILEX * TILEY; i++)
	{
		fdraw("map", MAP, _tiles[i].rc.left, _tiles[i].rc.top, _tiles[i].terrainFrameX, _tiles[i].terrainFrameY);
	}

	// 오브젝트
	for (int i = 0; i < TILEX * TILEY; i++)
	{
		if (_tiles[i].object == OBJ_NULL) continue;
		{
			if (_tiles[i].object == OBJ_WALL)
				fdraw("map", MAP, _tiles[i].rc.left, _tiles[i].rc.top, _tiles[i].objFrameX, _tiles[i].objFrameY);
			else if (_tiles[i].object == OBJ_TREE)
				IMAGEMANAGER->frameRender("tree", MAP, _tiles[i].rc.left, _tiles[i].rc.top, _tiles[i].objFrameX, _tiles[i].objFrameY);
			else
				IMAGEMANAGER->frameRender("samplemap", MAP, _tiles[i].rc.left, _tiles[i].rc.top, _tiles[i].objFrameX, _tiles[i].objFrameY);
			if (_tiles[i].object == OBJ_MON && _tiles[i].monPos == MPOS_0)
				draw("tile", MAP, _tiles[i].rc.left, _tiles[i].rc.top);
		}
		if (first == true)
		{
			if (_tiles[i].mon != -1)
			{
				sprintf_s(str, "a%d", _tiles[i].mon);
				fdraw(str, MAP, _tiles[i].rc.left, _tiles[i].rc.top);
			}
		}
	}
	if (_vRoad.size() > 0)
	{
		for (_viRoad = _vRoad.begin(); _viRoad != _vRoad.end(); _viRoad++)
		{
			IMAGEMANAGER->render("road", MAP, _tiles[*_viRoad].rc.left, _tiles[*_viRoad].rc.top);
		}
	}
}

void MapTool::drawList(int num)
{
	PatBlt(UIDC2, 0, 0, 500, 5000, BLACKNESS);
	char str[128];
	SetTextColor(UIDC2, RGB(10, 10, 10));
	SetBkMode(UIDC2, TRANSPARENT);
	HFONT font, oldFont;
	font = CreateFont(25, 0, 0, 0, 100, 0, 0, 0, DEFAULT_CHARSET,
		OUT_STRING_PRECIS, CLIP_CHARACTER_PRECIS, PROOF_QUALITY,
		DEFAULT_PITCH | FF_SWISS, TEXT("HY얕은샘물M"));
	oldFont = (HFONT)SelectObject(UIDC2, font);

	for (int i = 0; i < _mM->getVDmon().size(); i++)
	{
		IMAGEMANAGER->render("slot", UIDC2, 0 + (i % 2 * 250), 52 + (i / 2 * 110));
		IMAGEMANAGER->frameRender("Fimg", UIDC2, 8 + (i % 2 * 250), 60 + (i / 2 * 110), (_mM->getVDmon()[i].num)%6, (_mM->getVDmon()[i].num) / 6);
		IMAGEMANAGER->frameRender("type", UIDC2, 173 + (i % 2 * 250), 70 + (i / 2 * 110), _mM->getDex(_mM->getVDmon()[i].num).type % 4, _mM->getDex(_mM->getVDmon()[i].num).type / 4);
		if (_mM->getVDmon()[i].have > 0)
			sprintf_s(str, "%s x %d", _mM->getDex(_mM->getVDmon()[i].num).name.c_str(), _mM->getVDmon()[i].have);
		else
			sprintf_s(str, "%d %s", _mM->getDex(_mM->getVDmon()[i].num).food,_mM->getDex(_mM->getVDmon()[i].num).name.c_str());
		DrawText(UIDC2, TEXT(str), strlen(str), &RectMake(56 + (i % 2 * 250), 57 + (i / 2 * 110), 150, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		sprintf_s(str, "%d HP:%d", _mM->getDex(_mM->getVDmon()[i].num).food,(int)_mM->getDex(_mM->getVDmon()[i].num).hp);
		DrawText(UIDC2, TEXT(str), strlen(str), &RectMake(10 + (i % 2 * 250), 105 + (i / 2 * 110), 400, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		sprintf_s(str, "ATT:%d", _mM->getDex(_mM->getVDmon()[i].num).power);
		DrawText(UIDC2, TEXT(str), strlen(str), &RectMake(96 + (i % 2 * 250), 105 + (i / 2 * 110), 400, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		sprintf_s(str, "SPD:%.1f", (float)_mM->getDex(_mM->getVDmon()[i].num).spd);
		DrawText(UIDC2, TEXT(str), strlen(str), &RectMake(148 + (i % 2 * 250), 105 + (i / 2 * 110), 400, 50), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	}

	if(num>=0)
		IMAGEMANAGER->render("slot2", UIDC2, 0 + (num % 2 * 250), 52 + (num / 2 * 110));
	SelectObject(UIDC2, oldFont);
	DeleteObject(font);
}

void MapTool::cancelMon(tagCurrentTile i, bool drag, bool die)
{
	tagCurrentTile mouseTile=i;

	mouseTile=findMon(mouseTile);

	if (_player == mouseTile.x + (mouseTile.y) * 100)
		_player = 0;

	for (int i = -1; i < _mM->getDex(_tiles[mouseTile.x + (mouseTile.y) * 100].mon).size.y+1; i++)
	{
		for (int j = -1; j < _mM->getDex(_tiles[mouseTile.x + (mouseTile.y) * 100].mon).size.x+1; j++)
		{
			if (_tiles[mouseTile.x + j + (mouseTile.y + i) * 100].object == OBJ_MON)
				_tiles[mouseTile.x + j + (mouseTile.y + i) * 100].object = OBJ_NULL;
		}
	}
	if(die==false)
		_mM->addDmon(_tiles[mouseTile.x + (mouseTile.y) * 100].mon);
	_food[1] += _mM->getDex(_tiles[mouseTile.x + (mouseTile.y) * 100].mon).food;
	_tiles[mouseTile.x + (mouseTile.y) * 100].mon = -1;
	
	for (_viMon = _vMon.begin(); _viMon != _vMon.end(); _viMon++)
	{
		if (mouseTile.x + (mouseTile.y) * 100 == (*_viMon).tile)
		{
			for (_viMonNum = _vMonNum.begin(); _viMonNum != _vMonNum.end(); _viMonNum++)
			{
				if ((*_viMon).id == (*_viMonNum))
				{
					_viMonNum = _vMonNum.erase(_viMonNum);
					break;
				}
			}
			for (int i = 0; i < (*_viMon).vSlot.size(); i++)
			{
				_tiles[(*_viMon).vSlot[i]].fight = false;
				_tiles[(*_viMon).vSlot[i]].mon2 = -1;
			}
			_viMon = _vMon.erase(_viMon);
			break;
		}
	}
	
	if (drag == false)
	{
		drawList(-1);
		drawMap();
	}
	_monNum--;
}

tagCurrentTile MapTool::findMon(tagCurrentTile i)
{
	if (_tiles[i.x + (i.y) * 100].monPos == MPOS_2)
		i.x -= 1;
	else if (_tiles[i.x + (i.y) * 100].monPos == MPOS_3)
		i.x -= 2;
	else if (_tiles[i.x + (i.y) * 100].monPos == MPOS_4)
		i.y -= 1;
	else if (_tiles[i.x + (i.y) * 100].monPos == MPOS_5)
	{
		i.x -= 1;
		i.y -= 1;
	}
	else if (_tiles[i.x + (i.y) * 100].monPos == MPOS_6)
	{
		i.x -= 2;
		i.y -= 1;
	}
	else if (_tiles[i.x + (i.y) * 100].monPos == MPOS_7)
		i.y -= 2;
	else if (_tiles[i.x + (i.y) * 100].monPos == MPOS_8)
	{
		i.x -= 1;
		i.y -= 2;
	}
	else if (_tiles[i.x + (i.y) * 100].monPos == MPOS_9)
	{
		i.x -= 2;
		i.y -= 2;

	}
	return i;
}

void MapTool::findRoad()
{
	vector<int>				_vTemp;
	vector<int>::iterator	_viTemp;
	int player = _player;
	if (_mM->getDex(_tiles[_player].mon).size.x > 2)
		player += 1;
	if (_mM->getDex(_tiles[_player].mon).size.y > 2)
		player += 100;

	if (_player == 0) return;
	if (_vRoad.size() > 0)
	{
		for (_viRoad = _vRoad.begin(); _viRoad != _vRoad.end();)
		{
			_viRoad = _vRoad.erase(_viRoad);
		}
	}
	findDoor();
	if (_vDoor.size() < 1) return;
	for (_viDoor = _vDoor.begin(); _viDoor != _vDoor.end(); _viDoor++)
	{
		_vTemp = _star->findRoad(*_viDoor, player,_player-101,_mM->getDex(_tiles[_player].mon).size.x, _mM->getDex(_tiles[_player].mon).size.y);
		for (_viTemp = _vTemp.begin(); _viTemp != _vTemp.end(); _viTemp++)
		{
			_vRoad.push_back(*_viTemp);
		}
		int a = 0;
		a++;
	}
}

void MapTool::findDoor()
{
	if (_vDoor.size() > 0)
	{
		for (_viDoor = _vDoor.begin(); _viDoor != _vDoor.end();)
		{
			_viDoor = _vDoor.erase(_viDoor);
		}
	}
	for (int i = 0; i < TILEX * (_currentXY.y + 6); i++)
	{
		if (_tiles[i].terrain != TR_FLOOR) continue;
		if ((_tiles[i - 1].terrain != TR_FLOOR && _tiles[i - 1].object != OBJ_WALL) || (_tiles[i + 1].terrain != TR_FLOOR && _tiles[i + 1].object != OBJ_WALL)
			|| (_tiles[i - 100].terrain != TR_FLOOR && _tiles[i - 100].object != OBJ_WALL) || (_tiles[i + 100].terrain != TR_FLOOR && _tiles[i + 100].object != OBJ_WALL))
			_vDoor.push_back(i);
	}
}

void MapTool::setMon(int i)
{
	tagMon mon;
	mon.rc = RectMake(_tiles[i].rc.left - 64, _tiles[i].rc.top - 64, (_mM->getDex(_tiles[i].mon).size.x + 4) * 32, (_mM->getDex(_tiles[i].mon).size.y + 4) * 32);
	mon.tile = i;
	mon.hp = _mM->getDex(_tiles[i].mon).hp;
	mon.atCount = 0;
	mon.currentX = 0;
	mon.currentY = 0;
	mon.frameNum = 0;
	mon.frameCount = 0;
	mon.id = 0;
	for (int k = -1; k < _mM->getDex(_tiles[i].mon).size.y + 1; k++)
	{
		for (int j = -1; j < _mM->getDex(_tiles[i].mon).size.x + 1; j++)
		{
			if (_tiles[i + j + k * 100].object == OBJ_MON && _tiles[i + j + k * 100].monPos == MPOS_0)
			{
				mon.vSlot.push_back(i + j + k * 100);
				_tiles[i + j + k * 100].mon2 = i;
			}
		}
	}
	mon.hpbar = new progressBar;
	if (_vMonNum.size() == 0)
		mon.id = 0;
	else
	{
		for (int l = 0; l < _vMonNum.size(); l++)
		{
			if (_vMonNum[l] == mon.id)
				mon.id++;
			else
				break;
		}
	}
	int loca = 0;
	if (_mM->getDex(_tiles[i].mon).size.x == 2)
		loca = 16;
	else if(_mM->getDex(_tiles[i].mon).size.x == 3)
		loca = 32;
	_vMonNum.insert(_vMonNum.begin()+ mon.id, mon.id);
	mon.hpbar->init(_tiles[i].rc.left+ loca, _tiles[i].rc.top-2,32,5, mon.id);
	_vMon.push_back(mon);
}

int MapTool::findMonVec(int i)
{
	for (int j=0; j<_vMon.size(); j++)
	{
		if (i == _vMon[j].tile)
			return j;
	}
}

void MapTool::monFrame()
{
	for (_viMon = _vMon.begin(); _viMon != _vMon.end(); _viMon++)
	{
		(*_viMon).frameCount++;
		if ((*_viMon).frameCount % 10 == 0)
		{
			if ((*_viMon).frameNum < _mM->getDex(_tiles[(*_viMon).tile].mon).frameNum-1)
			{
				if((*_viMon).currentX<_mM->getDex(_tiles[(*_viMon).tile].mon).frameX-1)
					(*_viMon).currentX++;
				else
				{
					(*_viMon).currentX = 0;
					(*_viMon).currentY++;
				}
				(*_viMon).frameNum++;
			}
			else
			{
				(*_viMon).frameNum = 0;
				(*_viMon).currentX = 0;
				(*_viMon).currentY=0;
			}
			(*_viMon).frameCount = 0;
		}
	}
}

void MapTool::monUpdate()
{
	RECT temp;
	int distance=-1;
	int num = 0;
	for (_viMon = _vMon.begin(); _viMon != _vMon.end(); _viMon++)
	{
		(*_viMon).hpbar->setGauge((*_viMon).hp, _mM->getDex(_tiles[(*_viMon).tile].mon).hp);
		(*_viMon).hpbar->update();
		for (int i = 0; i < _mM->getEmon().size(); i++)
		{
			distance = -1;
			num = 0;
			if (IntersectRect(&temp, &(*_viMon).rc, &_mM->getEmon()[i]->getRc())&& _mM->getEmon()[i]->getAct()==false)
			{
				for ((*_viMon).viSlot = (*_viMon).vSlot.begin(); (*_viMon).viSlot != (*_viMon).vSlot.end(); (*_viMon).viSlot++)
				{
					if (_tiles[(*(*_viMon).viSlot)].fight == 1) continue;
					if (distance == -1)
					{
						distance = getDistance((_tiles[(*(*_viMon).viSlot)].rc.right + _tiles[(*(*_viMon).viSlot)].rc.left) / 2, (_tiles[(*(*_viMon).viSlot)].rc.bottom + _tiles[(*(*_viMon).viSlot)].rc.top) / 2,
							(_mM->getEmon()[i]->getRc().right + _mM->getEmon()[i]->getRc().left) / 2, (_mM->getEmon()[i]->getRc().bottom + _mM->getEmon()[i]->getRc().top) / 2);
						num = (*(*_viMon).viSlot);
					}
					else
					{
						if (getDistance((_tiles[(*(*_viMon).viSlot)].rc.right + _tiles[(*(*_viMon).viSlot)].rc.left) / 2, (_tiles[(*(*_viMon).viSlot)].rc.bottom + _tiles[(*(*_viMon).viSlot)].rc.top) / 2,
							(_mM->getEmon()[i]->getRc().right + _mM->getEmon()[i]->getRc().left) / 2, (_mM->getEmon()[i]->getRc().bottom + _mM->getEmon()[i]->getRc().top) / 2) < distance)
						{
							distance = getDistance((_tiles[(*(*_viMon).viSlot)].rc.right + _tiles[(*(*_viMon).viSlot)].rc.left) / 2, (_tiles[(*(*_viMon).viSlot)].rc.bottom + _tiles[(*(*_viMon).viSlot)].rc.top) / 2,
								(_mM->getEmon()[i]->getRc().right + _mM->getEmon()[i]->getRc().left) / 2, (_mM->getEmon()[i]->getRc().bottom + _mM->getEmon()[i]->getRc().top) / 2);
							num = (*(*_viMon).viSlot);
						}
					}
				}

				if (num != 0)
				{
					_tiles[num].fight = 1;
					vector<int> temp;
					_mM->getEmon()[i]->eraseRoad();
					_mM->getEmon()[i]->setRoad(_star->findRoad(_mM->getEmon()[i]->getLoca(),num));
					_mM->getEmon()[i]->setAct(true);
				}
			}
		}
	}
	for (int i=0;i<_vMon.size();i++)
	{
		if (_vMon[i].hp <= 0)
		{
			tagCurrentTile Tile = { _vMon[i].tile%TILEX ,_vMon[i].tile/TILEX };
			if (_vMon[i].tile == _player)
				_mM->addDmon(_tiles[_player].mon);
			cancelMon(Tile, false,true);
			i--;
		}
	}
}

void MapTool::collision()
{
	
	for (int i=0;i<_bullet->getVBullet().size();i++)
	{
		if (_bullet->getVBullet()[i].x < 32*2|| _bullet->getVBullet()[i].y <32*5|| _bullet->getVBullet()[i].x > (_currentXY.x+2)*32|| _bullet->getVBullet()[i].y >(_currentXY.y + 5) * 32)
			_bullet->removeBullet(i);
	}
	if (_bullet->getBall().x < 32 * 2 || _bullet->getBall().y <32 * 5 || _bullet->getBall().x >(_currentXY.x + 2) * 32 || _bullet->getBall().y >(_currentXY.y + 5) * 32)
		_bullet->setBallFire(false);
	for (int j = 0; j < _mM->getEmon().size(); j++)
	{
		RECT temp;
		if (IntersectRect(&temp, &_bullet->getBallRc(), &_mM->getEmon()[j]->getRc())&&_bullet->getBallFire()==true)
		{
			int rand = RND->getInt(2);
			_bullet->setBallFire(false);
			if (rand == 1)
			{
				_mM->addDmon(_mM->getEmon()[j]->getDex().num);
				_tiles[_mM->getEmon()[j]->getRoad()[0]].fight=false;
				_mM->deleteEmon(j);
				return;
			}
		} // 몬스터볼 포획
		for (int i = 0; i < _bullet->getVBullet().size(); i++)
		{
			if (IntersectRect(&temp, &_bullet->getVBullet()[i].rc, &_mM->getEmon()[j]->getRc()))
			{
				if(_bullet->getVBullet()[i].type- _mM->getEmon()[j]->getDex().type==1|| _bullet->getVBullet()[i].type - _mM->getEmon()[j]->getDex().type == -2)
					_mM->getEmon()[j]->setHp(_mM->getEmon()[j]->getHp() - (_bullet->getVBullet()[i].power * 2)*1.5);
				else
					_mM->getEmon()[j]->setHp(_mM->getEmon()[j]->getHp() - _bullet->getVBullet()[i].power * 2);
				_bullet->removeBullet(i);
				return;
			}
		}
	}
}

MapTool::MapTool()
	: _currentTileO({ 0,0 }),_currentTileT({ 5,0 }), _player(0)
{}

MapTool::~MapTool(){}
