#include "stdafx.h"
#include "imageManager.h"


imageManager::imageManager()
{
}


imageManager::~imageManager()
{

}

HRESULT imageManager::init(void)
{

	return S_OK;
}

void imageManager::release(void)
{
	deleteAll();
}


image* imageManager::addImage(string strKey, int width, int height, BOOL blend)																			
{
	//�ش� Ű ���� ã�ƺ���
	image* img = findImage(strKey);

	//�� Ű���� ���� �̹����� ������ �� �̹����� ����
	if (img) return img;

	//������� �Դٸ� �� Ű ���� ���� ��!

	img = new image;

	//�̹����� �ʱ�ȭ �Ѵ�, (���� ���н� �� �� ��ȯ)
	if (FAILED(img->init(width, height, blend)))
	{
		SAFE_DELETE(img);

		return NULL;
	}

	//�ʱ�ȭ �� �̹����� Ű �� �ٿ��� �������̳ʿ� �߰��Ѵ�
	_mImageList.insert(make_pair(strKey, img));

	return img;
}

image* imageManager::addImage(string strKey, int width, int height, bool trans, COLORREF transColor, BOOL blend)
{
	//�ش� Ű ���� ã�ƺ���
	image* img = findImage(strKey);

	//�� Ű���� ���� �̹����� ������ �� �̹����� ����
	if (img) return img;

	//������� �Դٸ� �� Ű ���� ���� ��!

	img = new image;

	//�̹����� �ʱ�ȭ �Ѵ�, (���� ���н� �� �� ��ȯ)
	if (FAILED(img->init(width, height, trans, transColor, blend)))
	{
		SAFE_DELETE(img);

		return NULL;
	}

	//�ʱ�ȭ �� �̹����� Ű �� �ٿ��� �������̳ʿ� �߰��Ѵ�
	_mImageList.insert(make_pair(strKey, img));

	return img;
}

image* imageManager::addImage(string strKey, const char* fileName, int width, int height, bool trans, COLORREF transColor, BOOL blend)					
{
	//�ش� Ű ���� ã�ƺ���
	image* img = findImage(strKey);

	//�� Ű���� ���� �̹����� ������ �� �̹����� ����
	if (img) return img;

	//������� �Դٸ� �� Ű ���� ���� ��!

	img = new image;

	//�̹����� �ʱ�ȭ �Ѵ�, (���� ���н� �� �� ��ȯ)
	if (FAILED(img->init(fileName, width, height, trans, transColor, blend)))
	{
		SAFE_DELETE(img);

		return NULL;
	}

	//�ʱ�ȭ �� �̹����� Ű �� �ٿ��� �������̳ʿ� �߰��Ѵ�
	_mImageList.insert(make_pair(strKey, img));

	return img;
}

image* imageManager::addImage(string strKey, const char* fileName, float x, float y, int width, int height, bool trans, COLORREF transColor, BOOL blend)
{
	//�ش� Ű ���� ã�ƺ���
	image* img = findImage(strKey);

	//�� Ű���� ���� �̹����� ������ �� �̹����� ����
	if (img) return img;

	//������� �Դٸ� �� Ű ���� ���� ��!

	img = new image;

	//�̹����� �ʱ�ȭ �Ѵ�, (���� ���н� �� �� ��ȯ)
	if (FAILED(img->init(fileName, x, y, width, height, trans, transColor, blend)))
	{
		SAFE_DELETE(img);

		return NULL;
	}

	//�ʱ�ȭ �� �̹����� Ű �� �ٿ��� �������̳ʿ� �߰��Ѵ�
	_mImageList.insert(make_pair(strKey, img));

	return img;
}


image* imageManager::addFrameImage(string strKey, const char* fileName, float x, float y, int width, int height, int frameX, int frameY, bool trans, COLORREF transColor, BOOL blend)
{
	//�ش� Ű ���� ã�ƺ���
	image* img = findImage(strKey);

	//�� Ű���� ���� �̹����� ������ �� �̹����� ����
	if (img) return img;

	//������� �Դٸ� �� Ű ���� ���� ��!

	img = new image;

	//�̹����� �ʱ�ȭ �Ѵ�, (���� ���н� �� �� ��ȯ)
	if (FAILED(img->init(fileName, x, y, width, height, frameX, frameY, trans, transColor, blend)))
	{
		SAFE_DELETE(img);

		return NULL;
	}

	//�ʱ�ȭ �� �̹����� Ű �� �ٿ��� �������̳ʿ� �߰��Ѵ�
	_mImageList.insert(make_pair(strKey, img));

	return img;
}

image* imageManager::addFrameImage(string strKey, const char* fileName, int width, int height, int frameX, int frameY, bool trans, COLORREF transColor, BOOL blend)					 
{
	//�ش� Ű ���� ã�ƺ���
	image* img = findImage(strKey);

	//�� Ű���� ���� �̹����� ������ �� �̹����� ����
	if (img) return img;

	//������� �Դٸ� �� Ű ���� ���� ��!

	img = new image;
	
	//�̹����� �ʱ�ȭ �Ѵ�, (���� ���н� �� �� ��ȯ)
	if (FAILED(img->init(fileName, width, height, frameX, frameY, trans, transColor, blend)))
	{
		SAFE_DELETE(img);

		return NULL;
	}

	//�ʱ�ȭ �� �̹����� Ű �� �ٿ��� �������̳ʿ� �߰��Ѵ�
	_mImageList.insert(make_pair(strKey, img));

	return img;
}

image* imageManager::addRotateImage(string strKey, const char * fileName,  int width, int height, bool trans, COLORREF transColor, BOOL blend)
{
	image* img = findImage(strKey);

	if (img) return img;

	img = new image;

	if (FAILED(img->rotateInit(fileName, width, height, trans, transColor, blend)))
	{
		SAFE_DELETE(img);

		return NULL;
	}
	// �� �����̳ʿ� �߰� 
	_mImageList.insert(make_pair(strKey, img));

	return img;
}

image* imageManager::addRotateImage(string strKey, int width, int height, bool trans, COLORREF transColor, BOOL blend)
{
	//�ش� Ű ���� ã�ƺ���
	image* img = findImage(strKey);

	//�� Ű���� ���� �̹����� ������ �� �̹����� ����
	if (img) return img;

	//������� �Դٸ� �� Ű ���� ���� ��!

	img = new image;

	if (FAILED(img->rotateInit(width, height, trans, transColor, blend)))
	{
		SAFE_DELETE(img);

		return NULL;
	}

	//�ʱ�ȭ �� �̹����� Ű �� �ٿ��� �������̳ʿ� �߰��Ѵ�
	_mImageList.insert(make_pair(strKey, img));

	return img;
}

image * imageManager::addRotateFrameImage(string strKey, const char * fileName, int width, int height, int frameX, int frameY, bool trans, COLORREF transColor, BOOL blend)
{
	image * img = findImage(strKey);

	if (img) return img;

	img = new image;

	if (FAILED(img->rotateInit(fileName, width, height, frameX, frameY, trans, transColor, blend)))
	{
		SAFE_DELETE(img);

		return NULL;
	}

	_mImageList.insert(make_pair(strKey, img));
	return img;
}


//Ű �� ã�� �Լ�
image* imageManager::findImage(string strKey)
{
	mapImageIter key = _mImageList.find(strKey);

	//ã�Ҵ�
	if (key != _mImageList.end())
	{
		return key->second;
	}

	return NULL;
}


//�̱���ȭ �Ǿ��ִ� �̹��� �����ִ� �Լ�
BOOL imageManager::deleteImage(string strKey)
{
	//������� �̹����� Ű ������ �˻�
	mapImageIter key = _mImageList.find(strKey);

	//������� �̹����� ã������
	if (key != _mImageList.end())
	{
		//�̹��� Ŭ������ ������� �ش� �̹��� DC�� �̹������� ������
		key->second->release();
		SAFE_DELETE(key->second);	//�����Ҵ� ����
		_mImageList.erase(key);		//�� �����̳� �ȿ����� ����

		return true;
	}

	return false;
}


//��� �̹��� ���� �Լ�
BOOL imageManager::deleteAll(void)
{
	mapImageIter iter = _mImageList.begin();

	for (; iter != _mImageList.end();)
	{
		//�̹��� Ŭ������ �ִ�
		if (iter->second != NULL)
		{
			iter->second->release();
			SAFE_DELETE(iter->second);
			iter = _mImageList.erase(iter);
		}
		else ++iter;
	}

	_mImageList.clear();

	return TRUE;
}


void imageManager::render(string strKey, HDC hdc)																			
{
	image* img = findImage(strKey);

	if (img) img->render(hdc);
}

void imageManager::render(string strKey, HDC hdc, int destX, int destY)														
{
	image* img = findImage(strKey);

	if (img) img->render(hdc, destX, destY);
}

void imageManager::render(string strKey, HDC hdc, int destX, int destY, int sourX, int sourY, int sourWidth, int sourHeight)
{
	image* img = findImage(strKey);

	if (img) img->render(hdc, destX, destY, sourX, sourY, sourWidth, sourHeight);
}

void imageManager::frameRender(string strKey, HDC hdc, int destX, int destY)
{
	image* img = findImage(strKey);

	if (img) img->frameRender(hdc, destX, destY);
}

void imageManager::frameRender(string strKey, HDC hdc, int destX, int destY, int currentFrameX, int currentFrameY)
{
	image* img = findImage(strKey);

	if (img) img->frameRender(hdc, destX, destY, currentFrameX, currentFrameY);
}

void imageManager::rotateRender(string strKey, HDC hdc, float x, float y, float angle)
{
	image *img = findImage(strKey);

	img->rotateRender(hdc, x, y, angle);
}

void imageManager::rotateFrameRender(string strKey, HDC hdc, float x, float y, float angle)
{
	image *img = findImage(strKey);

	img->rotateFrameRender(hdc, x, y, angle);
}

void imageManager::loopRender(string strKey, HDC hdc, const LPRECT drawArea, int offSetX, int offSetY)
{
	image* img = findImage(strKey);

	if (img) img->loopRender(hdc, drawArea, offSetX, offSetY);
}

void imageManager::alphaRender(string strKey, HDC hdc, BYTE alpha)
{
	image* img = findImage(strKey);

	if (img) img->alphaRender(hdc, alpha);
}

void imageManager::alphaRender(string strKey, HDC hdc, int destX, int destY, BYTE alpha)
{
	image* img = findImage(strKey);

	if (img) img->alphaRender(hdc, destX, destY, alpha);
}

void imageManager::alphaRender(string strKey, HDC hdc, int destX, int destY, int sourX, int sourY, int sourWidth, int sourHeight, BYTE alpha)
{
	image* img = findImage(strKey);

	if (img) img->alphaRender(hdc, destX, destY, sourX, sourY, sourWidth, sourHeight, alpha);
}

void imageManager::alphaFrameRender(string strKey, HDC hdc, int destX, int destY, BYTE alpha)
{
	image* img = findImage(strKey);

	if (img) img->alphaFrameRender(hdc, destX, destY, alpha);
}

void imageManager::alphaFrameRender(string strKey, HDC hdc, int destX, int destY, int currentFrameX, int currentFrameY, BYTE alpha)
{
	image* img = findImage(strKey);

	if (img) img->alphaFrameRender(hdc, destX, destY, currentFrameX, currentFrameY, alpha);
}

void imageManager::alphaLoopRender(string strKey, HDC hdc, const LPRECT drawArea, int offSetX, int offSetY, BYTE alpha)
{
	image* img = findImage(strKey);

	if (img) img->alphaLoopRender(hdc, drawArea, offSetX, offSetY, alpha);
}
