#include "main.h"
/*
I am not a complete author of this code. I just rewrote it and added Lua Api
Code autor: ARMOR https://www.blast.hk/members/287503/
Original code: https://gist.github.com/imring/a6f5f3ec629bf23aba3dbfeb9f2dd84c
*/

sol::table open(sol::this_state ts) {
    sol::state_view lua{ ts };
    sol::table myModule = lua.create_table();

    myModule["VERSION"] = 1.7;

    myModule.set_function("init", []() {
        CEntityRender::GetInstance()->Init();
    });

    myModule.set_function("getD3D9Texture", [](std::uint32_t nId) {
        return CEntityRender::GetInstance()->GetD3D9Texture(nId);
    });

    myModule.set_function("recreateTexture", [](std::uint32_t nId) {
        CEntityRender::GetInstance()->RecreateTexture(nId);
    });

    myModule.set_function("setTexturePos", [](std::uint32_t nId, float fX, float fY, float fZ) {
        return CEntityRender::GetInstance()->SetTexturePos(nId, { fX, fY, fZ });
    });

    myModule.set_function("setTextureRotate", [](std::uint32_t nId, float fX, float fY, float fZ) {
        return CEntityRender::GetInstance()->SetTextureRotate(nId, { fX, fY, fZ });
    });

    myModule.set_function("setTextureModel", [](std::uint32_t nId, std::uint32_t nModel) {
        return CEntityRender::GetInstance()->SetTextureModel(nId, nModel);
    });

    myModule.set_function("setTextureBackground", [](std::uint32_t nId, float nR, float nG, float nB, float nA) {
        return CEntityRender::GetInstance()->SetTextureBackground(
            nId, 
            { 
            static_cast<std::uint8_t>(nR),
            static_cast<std::uint8_t>(nG), 
            static_cast<std::uint8_t>(nB), 
            static_cast<std::uint8_t>(nA) 
            }
        );
    });

    myModule.set_function("setTextureVehicleColor", [](std::uint32_t nId, std::uint8_t nPrimaryColor, std::uint8_t nSecondaryColor) {
        return CEntityRender::GetInstance()->SetTextureVehicleColor(nId, nPrimaryColor, nSecondaryColor);
    });

    myModule.set_function("deleteTexture", [](std::uint32_t nId) {
        return CEntityRender::GetInstance()->DeleteTexture(nId);
    });

    myModule.set_function("deleteAllTextures", [] {
        CEntityRender::GetInstance()->DeleteAllTextures();
    });

    myModule.set_function("isTextureExist", [] (std::uint32_t nId){
        return CEntityRender::GetInstance()->IsTextureExist(nId);
    });

    myModule.set_function("addEntity", [](
        std::uint32_t nModel = 0,
        float textureSizeX = 256.f,
        float textureSizeY = 256.f
        ) 
    {
        return CEntityRender::GetInstance()->AddEntity(nModel, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0, 0, 0, 255 }, { textureSizeX, textureSizeY });
    });

    return myModule;
}

SOL_MODULE_ENTRYPOINT(open);