#pragma once
/*
I am not a complete author of this code. I just rewrote it and added Lua Api
Code autor: ARMOR https://www.blast.hk/members/287503/
Original code: https://gist.github.com/imring/a6f5f3ec629bf23aba3dbfeb9f2dd84c
*/

#include "main.h"
#include "plugin.h"
#include "CVisibilityPlugins.h"
#include "CScene.h"
#include "extensions/ScriptCommands.h"
#include "CModelInfo.h"
#include "CRenderer.h"
#include <map>

using namespace plugin;

class CEntityRender
{
	struct tDraw 
	{
		RwCamera* pCamera;
		RwFrame* pFrame;
		RpLight* pLight;
		RwTexture* pTexture;
		RwRaster* pBuffer;
		RwRaster* pZBuffer;
		RwV3d vPos;
		RwV3d vRotate;
		RwV2d vTextureSize;
		RwRGBA nColor;
		std::uint32_t nModel;
		std::uint8_t nPrimaryColor;
		std::uint8_t nSecondaryColor;
		bool bNeedRecreate;
		bool bIsInit;
	};

	std::map<std::uint32_t, tDraw> m_DrawMap;
	std::uint32_t m_nTexturesCount;

	CdeclEvent<AddressList<0x53EA12, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void()> BeforeMainRender;

	static CEntityRender* StaticInstance;

	CEntityRender() = default;
	CEntityRender(CEntityRender&) = delete;

	void CreateLight(RpLight**);
	void CreateTexture(RwRaster** ppBuffer, RwTexture** ppTexture, const RwV2d& vTextureSize);
	void CreateFrame(RwFrame** ppFrame);
	void AddCameraToScene(RwCamera* pCamera);
	void AddLightToScene(RpLight* pLight);
	void RemoveLightFromScene(RpLight* pLight);
	void BeginCamera(tDraw& draw, RwRaster** pRaster);
	void EndCamera(const tDraw& draw);

	void RotateEntity(CEntity* pEntity, const RwV3d& vRotate);
	void UpdateMatrix(CEntity* pEntity, RwMatrix* pMatrix);

	void RenderEntity(tDraw& draw, CEntity* pEntity);
	void RenderPed(tDraw& draw);
	void RenderVehicle(tDraw& draw);
	void RenderObject(tDraw& draw);

	void Render();
public:
	~CEntityRender();
	static CEntityRender* GetInstance();

	void Init();
	void* GetD3D9Texture(std::uint32_t nId);
	void RecreateTexture(std::uint32_t nId);

	bool SetTexturePos(std::uint32_t nId, const RwV3d& vPos);
	bool SetTextureRotate(std::uint32_t nId, const RwV3d& vRotate);
	bool SetTextureModel(std::uint32_t nId, std::uint32_t nModel);
	bool SetTextureBackground(std::uint32_t nId, const RwRGBA& nColor);
	bool SetTextureVehicleColor(std::uint32_t nId, std::uint8_t nPrimaryColor, std::uint8_t nSecondColor);
	bool DeleteTexture(std::uint32_t nId);
	void DeleteAllTextures();
	bool IsTextureExist(std::uint32_t nId);
	std::uint32_t AddEntity(std::uint32_t nModel = 0, const RwV3d& vPos = { 0.0f, 0.0f, 0.0f }, const RwV3d& vRotate = { 0.0f, 0.0f, 0.0f }, const RwRGBA& nBackgroundColor = { 0, 0, 0, 0 }, const RwV2d& vTextureSize = {256.f, 256.f}, std::uint8_t nPrimaryCarColor = { 0 }, std::uint8_t nSecondaryCarColor = { 0 });
};

