//미리 컴파일된 헤더파일
#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             
//#define WIN32_WINNT 0x0400             
// 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <iostream>
#include "utils.h"
//#include "winuser.h"

using namespace std;
using namespace CLASS_TEN;


#include "randomFunction.h"
#include "keyManager.h"
#include "imageManager.h"
#include "commonMacroFunction.h"
#include "timeManager.h"
#include "soundManager.h"
#include "effectManager.h"
#include "sceneManager.h"
#include "cameraManager.h" // 카메라 매니저 
#include "txtData.h"
#include "iniDataManager.h"
#include "keyAniManager.h"
#include "tileNode.h"


//==========================================
// ## 18.05.01 ## - 디파인문 - 
//==========================================

#define WINNAME (LPCTSTR)(TEXT("Alpha"))
#define WINSTARTX 0	//윈도우 시작좌표 X
#define WINSTARTY 0	//윈도우 시작좌표 Y
#define WINSIZEX GetSystemMetrics(SM_CXSCREEN)	//윈도우 가로크기 : 1920
#define WINSIZEY GetSystemMetrics(SM_CYSCREEN)	//윈도우 세로크기 : 1080
#define TOTALSIZEX 7680
#define TOTALSIZEY 7680
#define WINSTYLE WS_CAPTION | WS_SYSMENU

#define RND				randomFunction::getSingleton()
#define KEYMANAGER		keyManager::getSingleton()
#define IMAGEMANAGER	imageManager::getSingleton()
#define TIMEMANAGER		timeManager::getSingleton()									
#define SOUNDMANAGER	soundManager::getSingleton()
#define EFFECTMANAGER	effectManager::getSingleton()
#define SCENEMANAGER	sceneManager::getSingleton()
#define KEYANIMANAGER	keyAniManager::getSingleton()
#define CAMERAMANAGER	cameraManager::getSingleton()
#define TXTDATA			txtData::getSingleton()
#define INIDATA			iniDataManager::getSingleton()

//#define DC				CAMERAMANAGER->getCameraDC()->getMemDC()
//#define DC2				CAMERAMANAGER->getCameraDC2()->getMemDC()

#define DC		getMemDC()
#define UIDC	CAMERAMANAGER->getCameraDC()
#define UIDC2	CAMERAMANAGER->getCameraDC2()
#define MAP		IMAGEMANAGER->findImage("field")->getMemDC()

//#define PTMOUSE_X       CAMERAMANAGER->getCameraX() + _ptMouse.x
//#define PTMOUSE_Y       CAMERAMANAGER->getCameraY() + _ptMouse.y

#define PTMOUSE_X getMemDCPoint().x
#define PTMOUSE_Y getMemDCPoint().y

//============================
// ## 18.05.09 ## Macro ##
//============================

#define SAFE_DELETE(p) { if (p)	{ delete(p);(p) = NULL;	}}
#define SAFE_RELEASE(p) { if (p)	{ (p)->release();(p) = NULL;}}
#define SAFE_DELETE_ARRAY(p) {if(p) { delete[](p); (p) = NULL;}}

extern HINSTANCE _hInstance;
extern HWND _hWnd;
extern POINT _ptMouse;
extern BOOL _leftButtonDown;
