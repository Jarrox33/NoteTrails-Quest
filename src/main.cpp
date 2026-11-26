#include "scotland2/shared/modloader.h"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "GlobalNamespace/NoteMovement.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/ColorType.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"
#include "System/Type.hpp"

#define MOD_ID "notetrails"
#define VERSION "0.4.0"

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};

MAKE_HOOK_MATCH(NoteMovement_ManualUpdate, &GlobalNamespace::NoteMovement::ManualUpdate, void, GlobalNamespace::NoteMovement* self) {
    NoteMovement_ManualUpdate(self);
    if (!self) return;
    auto go = self->get_gameObject();
    auto trType = System::Type::GetType("UnityEngine.TrailRenderer, UnityEngine.CoreModule");
    if (!trType) return;
    auto tr = go->GetComponent(trType);
    if (!tr) {
        auto noteCtrl = go->GetComponent<GlobalNamespace::NoteController*>();
        if (!noteCtrl || !noteCtrl->get_noteData()) return;
        tr = go->AddComponent(trType);
        if (!tr) return;
        if (auto sh = UnityEngine::Shader::Find("Sprites/Default")) {
            il2cpp_utils::RunMethod(tr, "set_material", UnityEngine::Material::New_ctor(sh));
        }
        il2cpp_utils::RunMethod(tr, "set_time", 0.25f);
        il2cpp_utils::RunMethod(tr, "set_startWidth", 0.45f);
        il2cpp_utils::RunMethod(tr, "set_endWidth", 0.0f);
        bool left = (noteCtrl->get_noteData()->get_colorType() == GlobalNamespace::ColorType::ColorA);
        UnityEngine::Color c = left ? UnityEngine::Color(1.0f, 0.12f, 0.18f, 0.85f) : UnityEngine::Color(0.12f, 0.55f, 1.0f, 0.85f);
        il2cpp_utils::RunMethod(tr, "set_startColor", c);
        c.a = 0.0f;
        il2cpp_utils::RunMethod(tr, "set_endColor", c);
    }
    il2cpp_utils::RunMethod(tr, "set_enabled", true);
}

extern "C" void setup(CModInfo* info) {
    *info = modInfo.to_c();
    Paper::Logger::RegisterFileContextId(MOD_ID);
}

extern "C" void late_load() {
    il2cpp_functions::Init();
    INSTALL_HOOK(Paper::Logger::WithContext<MOD_ID>(), NoteMovement_ManualUpdate);
}
