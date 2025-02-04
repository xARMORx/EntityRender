#include "CEntityRender.h"
/*
I am not a complete author of this code. I just rewrote it and added Lua Api
Code autor: ARMOR https://www.blast.hk/members/287503/
Original code: https://gist.github.com/imring/a6f5f3ec629bf23aba3dbfeb9f2dd84c
*/

CEntityRender* CEntityRender::StaticInstance = nullptr;

void CEntityRender::CreateLight(RpLight** ppLight)
{
	RpLight* pLight = RpLightCreate(2);
	if (pLight) {
		printf("Set light color %p\n", pLight);
		RwRGBAReal color{ 1.0f, 1.0f, 1.0f, 1.0f };
		RpLightSetColor(pLight, &color);
		*ppLight = pLight;
	}
}

void CEntityRender::CreateTexture(RwRaster** ppBuffer, RwTexture** ppTexture, const RwV2d& vTextureSize)
{
	*ppBuffer = RwRasterCreate(static_cast<RwInt32>(vTextureSize.x), static_cast<RwInt32>(vTextureSize.y), 0, rwRASTERTYPECAMERATEXTURE);
	if (!*ppBuffer) return;
	*ppTexture = RwTextureCreate(*ppBuffer);
}

void CEntityRender::CreateFrame(RwFrame** ppFrame)
{
	*ppFrame = RwFrameCreate();
	if (*ppFrame) {
		RwV3d vPos = { 0.0f, 0.0f, 50.0f };
		RwFrameTranslate(*ppFrame, &vPos, rwCOMBINEPRECONCAT);
		RwV3d vXaxis = { 1.0f, 0.0f, 0.0f };
		RwFrameRotate(*ppFrame, &vXaxis, 90.0f, rwCOMBINEPRECONCAT);
	}
}

void CEntityRender::AddCameraToScene(RwCamera* pCamera)
{
	if (Scene.m_pWorld) {
		RpWorldAddCamera(Scene.m_pWorld, pCamera);
	}
}

void CEntityRender::AddLightToScene(RpLight* pLight)
{
	if (Scene.m_pWorld) {
		RpWorldAddLight(Scene.m_pWorld, pLight);
	}
}

void CEntityRender::RemoveLightFromScene(RpLight* pLight)
{
	if (Scene.m_pWorld) {
		RpWorldRemoveLight(Scene.m_pWorld, pLight);
	}
}

void CEntityRender::BeginCamera(tDraw& draw, RwRaster** pRaster)
{
	draw.pCamera->frameBuffer = *pRaster;
	draw.pCamera->zBuffer = draw.pZBuffer;

	CVisibilityPlugins::SetRenderWareCamera(draw.pCamera);
	RwCameraClear(draw.pCamera, &draw.nColor, rwCAMERACLEARIMAGE | rwCAMERACLEARZ);
	RwCameraBeginUpdate(draw.pCamera);
	this->AddLightToScene(draw.pLight);

	RwRenderStateSet(rwRENDERSTATECULLMODE, RWRSTATE(rwCULLMODECULLBACK));
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, RWRSTATE(TRUE));
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, RWRSTATE(TRUE));
	RwRenderStateSet(rwRENDERSTATESHADEMODE, RWRSTATE(rwSHADEMODEGOURAUD));
	RwRenderStateSet(rwRENDERSTATEFOGENABLE, RWRSTATE(FALSE));
}

void CEntityRender::EndCamera(const tDraw& draw)
{
	RwCameraEndUpdate(draw.pCamera);
	this->RemoveLightFromScene(draw.pLight);
}

void CEntityRender::RotateEntity(CEntity* pEntity, const RwV3d& vRotate)
{
	RwMatrix mMatrix{};

	pEntity->GetMatrix()->UpdateRW(&mMatrix);
	RwV3d vXaxis = { 1.0f, 0.0f, 0.0f };
	RwV3d vYaxis = { 0.0f, 1.0f, 0.0f };
	RwV3d vZaxis = { 0.0f, 0.0f, 1.0f };
	if (vRotate.x != 0.0f) {
		RwMatrixRotate(&mMatrix, &vXaxis, vRotate.x, rwCOMBINEPRECONCAT);
	}
	if (vRotate.y != 0.0f) {
		RwMatrixRotate(&mMatrix, &vYaxis, vRotate.y, rwCOMBINEPRECONCAT);
	}
	if (vRotate.z != 0.0f) {
		RwMatrixRotate(&mMatrix, &vZaxis, vRotate.z, rwCOMBINEPRECONCAT);
	}

	this->UpdateMatrix(pEntity, &mMatrix);
}

void CEntityRender::UpdateMatrix(CEntity* pEntity, RwMatrix* pRwMatrix)
{
	if (!pEntity || !pRwMatrix) return;

	RwFrame* pFrame = static_cast<RwFrame*>(pEntity->m_pRwObject->parent);
	pEntity->Remove();
	memcpy(pEntity->m_matrix, pRwMatrix, sizeof(RwMatrix));
	pEntity->GetMatrix()->UpdateRW(&pFrame->modelling);
	pEntity->UpdateRwFrame();
	pEntity->Add();
}

void CEntityRender::RenderEntity(tDraw& draw, CEntity* pEntity)
{
	if (!pEntity) return;

	RwObject* pRwObject = pEntity->m_pRwObject;
	if (!pRwObject) return;

	RwRaster* pBuffer{};
	RwTexture* pTexture{};

	this->CreateTexture(&pBuffer, &pTexture, draw.vTextureSize);
	if (!pBuffer || !pTexture) return;


	this->BeginCamera(draw, &pBuffer);
	pEntity->Add();
	pEntity->PreRender();
	CVisibilityPlugins::RenderEntity(pEntity, false, 999.f);
	pEntity->Remove();
	this->EndCamera(draw);

	draw.pTexture = pTexture;
}

void CEntityRender::RenderPed(tDraw& draw)
{
	Command<Commands::REQUEST_MODEL>(draw.nModel);
	Command<Commands::LOAD_ALL_MODELS_NOW>();
	if (!Command<Commands::HAS_MODEL_LOADED>(draw.nModel)) return;

	std::uint32_t nHandle{};
	Command<Commands::CREATE_CHAR>(0, draw.nModel, 0.0f, 0.0f, 50.0f, &nHandle);
	CPed* pPed = CPools::GetPed(nHandle);
	if (!pPed) return;

	pPed->SetIdle();
	pPed->Teleport(draw.vPos, false);
	pPed->m_bCollisionProcessed = false;
	pPed->m_bUsesCollision = false;

	this->RotateEntity(pPed, draw.vRotate);
	RpAnimBlendClumpUpdateAnimations(pPed->m_pRwClump, 100.0f, true);
	this->RenderEntity(draw, pPed);
	Command<Commands::DELETE_CHAR>(nHandle);
}

void CEntityRender::RenderVehicle(tDraw& draw)
{
	if (draw.nModel == 570)
		draw.nModel = 538;
	else if (draw.nModel == 569)
		draw.nModel = 537;

	Command<Commands::REQUEST_MODEL>(draw.nModel);
	Command<Commands::LOAD_ALL_MODELS_NOW>();
	if (!Command<Commands::HAS_MODEL_LOADED>(draw.nModel)) return;

	std::uint32_t nHandle{};
	Command<Commands::CREATE_CAR>(draw.nModel, 0.0f, 0.0f, 50.f, &nHandle);
	CVehicle* pVehicle = CPools::GetVehicle(nHandle);
	if (!pVehicle) return;

	pVehicle->Teleport(draw.vPos, false);
	pVehicle->m_bCollisionProcessed = false;
	pVehicle->m_bUsesCollision = false;

	this->RotateEntity(pVehicle, draw.vRotate);

	if (draw.nPrimaryColor != -1 && draw.nSecondaryColor != -1) {
		pVehicle->m_nPrimaryColor = draw.nPrimaryColor;
		pVehicle->m_nSecondaryColor = draw.nSecondaryColor;
	}

	this->RenderEntity(draw, pVehicle);
	Command<Commands::DELETE_CAR>(nHandle);
}

void CEntityRender::RenderObject(tDraw& draw)
{
	Command<Commands::REQUEST_MODEL>(draw.nModel);
	Command<Commands::LOAD_ALL_MODELS_NOW>();
	if (!Command<Commands::HAS_MODEL_LOADED>(draw.nModel)) return;

	std::uint32_t nHandle{};
	Command<Commands::CREATE_OBJECT>(draw.nModel, 0.0f, 0.0f, 50.0f, &nHandle);
	CObject* pObject = CPools::GetObject(nHandle);
	if (!pObject) return;

	pObject->CreateRwObject();
	pObject->Teleport(draw.vPos, false);

	this->RotateEntity(pObject, draw.vRotate);
	this->RenderEntity(draw, pObject);
	Command<Commands::DELETE_OBJECT>(nHandle);
}

void CEntityRender::Render()
{
	for (auto& it : this->m_DrawMap) {
		if (it.second.bNeedRecreate) {
			RwTextureDestroy(it.second.pTexture);
			it.second.pTexture = nullptr;
			it.second.bNeedRecreate = false;
		}

		if (!it.second.pTexture) {
			CBaseModelInfo* pModelInfo = CModelInfo::GetModelInfo(it.second.nModel);

			if (!pModelInfo)
				continue;

			switch (pModelInfo->GetModelType()) {
			case MODEL_INFO_VEHICLE:
				this->RenderVehicle(it.second);
				break;
			case MODEL_INFO_PED:
				this->RenderPed(it.second);
				break;
			case MODEL_INFO_WEAPON:
			case MODEL_INFO_ATOMIC:
			case MODEL_INFO_CLUMP:
				this->RenderObject(it.second);
				break;
			default:
				break;
			}
		}
	}
}

CEntityRender::~CEntityRender()
{
	this->StaticInstance = nullptr;
	delete this->StaticInstance;
}

CEntityRender* CEntityRender::GetInstance()
{
	if (!CEntityRender::StaticInstance) {
		CEntityRender::StaticInstance = new CEntityRender();
		CEntityRender::StaticInstance->m_nTexturesCount = 0;
		CEntityRender::StaticInstance->m_DrawMap = {};
	}

	return CEntityRender::StaticInstance;
}

void CEntityRender::Init()
{
	printf("CEntity Render init!\n");
	this->GetInstance()->BeforeMainRender += [this] { this->Render(); };
}

void* CEntityRender::GetD3D9Texture(std::uint32_t nId)
{
	auto it = this->m_DrawMap.find(nId);
	if (it == this->m_DrawMap.end()) {
		printf("Error id %d is not exist\n", nId);
		return nullptr;
	}

	// Get IDirect3DTexture9* from raster texture type;
	return *(void**)(it->second.pTexture->raster + 1);
}

void CEntityRender::RecreateTexture(std::uint32_t nId)
{
	auto it = this->m_DrawMap.find(nId);
	if (it == this->m_DrawMap.end())
		return;

	it->second.bNeedRecreate = true;
}

bool CEntityRender::SetTexturePos(std::uint32_t nId, const RwV3d& vPos)
{
	auto it = this->m_DrawMap.find(nId);
	if (it == this->m_DrawMap.end())
		return false;

	it->second.vPos = vPos;
	return true;
}

bool CEntityRender::SetTextureRotate(std::uint32_t nId, const RwV3d& vRotate)
{
	auto it = this->m_DrawMap.find(nId);
	if (it == this->m_DrawMap.end())
		return false;

	it->second.vRotate = vRotate;
	return true;
}

bool CEntityRender::SetTextureModel(std::uint32_t nId, std::uint32_t nModel)
{
	auto it = this->m_DrawMap.find(nId);
	if (it == this->m_DrawMap.end())
		return false;

	it->second.nModel = nModel;
	return true;
}

bool CEntityRender::SetTextureBackground(std::uint32_t nId, const RwRGBA& nColor)
{
	auto it = this->m_DrawMap.find(nId);
	if (it == this->m_DrawMap.end())
		return false;

	it->second.nColor = nColor;
	return true;
}

bool CEntityRender::SetTextureVehicleColor(std::uint32_t nId, std::uint8_t nPrimaryColor, std::uint8_t nSecondColor)
{
	auto it = this->m_DrawMap.find(nId);
	if (it == this->m_DrawMap.end())
		return false;

	it->second.nPrimaryColor = nPrimaryColor;
	it->second.nSecondaryColor = nSecondColor;
	return true;
}

bool CEntityRender::DeleteTexture(std::uint32_t nId)
{
	auto it = this->m_DrawMap.find(nId);
	if (it == this->m_DrawMap.end())
		return false;

	this->m_DrawMap.erase(nId);
	return true;
}

void CEntityRender::DeleteAllTextures()
{
	this->m_DrawMap.clear();
}

bool CEntityRender::IsTextureExist(std::uint32_t nId)
{
	auto it = this->m_DrawMap.find(nId);
	if (it == this->m_DrawMap.end())
		return false;

	return true;
}

std::uint32_t CEntityRender::AddEntity(std::uint32_t nModel, const RwV3d& vPos, const RwV3d& vRotate, const RwRGBA& nBackgroundColor, const RwV2d& vTextureSize, std::uint8_t nPrimaryCarColor, std::uint8_t nSecondaryCarColor)
{
    tDraw newDraw{};

    this->CreateLight(&newDraw.pLight);
	printf("pLight: %p!\n", newDraw.pLight);
    if (!newDraw.pLight) return -1;
	newDraw.vTextureSize = vTextureSize;
    newDraw.pZBuffer = RwRasterCreate(static_cast<RwInt32>(vTextureSize.x), static_cast<RwInt32>(vTextureSize.y), 0, rwRASTERTYPEZBUFFER);
    this->CreateTexture(&newDraw.pBuffer, &newDraw.pTexture, vTextureSize);
	printf("pZBuffer: %p | pTexture: %p | pBuffer: %p\n", newDraw.pZBuffer, newDraw.pTexture, newDraw.pBuffer);
	if (!newDraw.pZBuffer || !newDraw.pTexture || !newDraw.pBuffer) return -1;

    newDraw.pCamera = RwCameraCreate();
    this->CreateFrame(&newDraw.pFrame);
	printf("pCamera: %p | pFrame: %p\n", newDraw.pCamera, newDraw.pFrame);
    if (!newDraw.pCamera || !newDraw.pFrame) return -1;

    newDraw.pCamera->frameBuffer = newDraw.pBuffer;
    newDraw.pCamera->zBuffer = newDraw.pZBuffer;
    rwObjectHasFrameSetFrame(newDraw.pCamera, newDraw.pFrame);
    RwCameraSetFarClipPlane(newDraw.pCamera, 300.f);
    RwCameraSetNearClipPlane(newDraw.pCamera, 0.01f);

    RwV2d vViewMin = { 0.5f, 0.5f };
    RwCameraSetViewWindow(newDraw.pCamera, &vViewMin);
    RwCameraSetProjection(newDraw.pCamera, rwPERSPECTIVE);
    this->AddCameraToScene(newDraw.pCamera);

    newDraw.bIsInit = true;

    auto result = this->m_DrawMap.emplace(++this->m_nTexturesCount, newDraw);

	printf("%d\n", this->m_nTexturesCount);

    if (!result.second) return -1;

    return this->m_nTexturesCount;
}