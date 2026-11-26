#include "scotland2/shared/modloader.h"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "GlobalNamespace/NoteMovement.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/ColorType.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Component.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
#include "System/Type.hpp"

#define MOD_ID "notetrails"
#define VERSION "0.2.0"

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
static bool trailsEnabled = true;
static float trailTime = 0.25f;
static float trailWidth = 0.45f;
MAKE_HOOK_MATCH(NoteMovement_ManualUpdate, &GlobalNamespace::NoteMovement::ManualUpdate, void, GlobalNamespace::NoteMovement* self) {
    NoteMovement_ManualUpdate(self);
    if (!self) return;
    auto* go = self->get_gameObject();
    auto* noteCtrl = go->GetComponent<GlobalNamespace::NoteController*>();
    if (!noteCtrl || !noteCtrl->get_noteData()) return;
    auto* trType = System::Type::GetType("UnityEngine.TrailRenderer, UnityEngine.CoreModule");
    if (!trType) return;
    auto* tr = go->GetComponent(trType);
    if (!trailsEnabled) {
        if (tr) il2cpp_utils::RunMethod(tr, "set_enabled", false);
        return;
    }
    if (!tr) {
        tr = go->AddComponent(trType);
        if (auto* shader = UnityEngine::Shader::Find("Sprites/Default")) {
            il2cpp_utils::RunMethod(tr, "set_material", UnityEngine::Material::New_ctor(shader));
        }
        if (tr) {
            il2cpp_utils::RunMethod(tr, "Clear");
            il2cpp_utils::RunMethod(tr, "set_time", trailTime);
            il2cpp_utils::RunMethod(tr, "set_startWidth", trailWidth);
            il2cpp_utils::RunMethod(tr, "set_endWidth", 0.0f);
            bool isLeft = (noteCtrl->get_noteData()->get_colorType() == GlobalNamespace::ColorType::ColorA);
            UnityEngine::Color col = isLeft ? UnityEngine::Color(1.0f, 0.12f, 0.18f, 0.85f) : UnityEngine::Color(0.12f, 0.55f, 1.0f, 0.85f);
            il2cpp_utils::RunMethod(tr, "set_startColor", col);
            col.a = 0.0f;
            il2cpp_utils::RunMethod(tr, "set_endColor", col);
        }
    }
    if (tr) il2cpp_utils::RunMethod(tr, "set_enabled", true);
}

static void didActivate(HMUI::ViewController* self, bool firstActivation, bool, bool) {
    if (!firstActivation) return;
    auto* parent = BSML::Lite::CreateScrollableSettingsContainer(self->get_transform());
    if (auto* vlg = parent->GetComponent<UnityEngine::UI::VerticalLayoutGroup*>()) { vlg->set_spacing(3.0f); vlg->set_childForceExpandHeight(false); }
    BSML::Lite::CreateText(parent, "=== NoteTrails v0.2.0 (Quest 1.40.8) ===");
    BSML::Lite::CreateToggle(parent, "Enable Note Trails", trailsEnabled, [](bool v) { trailsEnabled = v; });
    BSML::Lite::CreateIncrementSetting(parent, "Trail Duration (sec)", 2, 0.05f, trailTime, 0.05f, 1.0f, [](float v) { trailTime = v; });
    BSML::Lite::CreateIncrementSetting(parent, "Trail Width", 2, 0.05f, trailWidth, 0.10f, 1.0f, [](float v) { trailWidth = v; });
}

MOD_EXPORT void setup(CModInfo* info) {
    info->id = MOD_ID; info->version = VERSION; info->version_long = 0;
    modInfo.assign(*info); Paper::Logger::RegisterFileContextId(MOD_ID);
}

MOD_EXPORT void late_load() {
    il2cpp_functions::Init(); BSML::Init();
    INSTALL_HOOK(Paper::Logger::WithContext<MOD_ID>(), NoteMovement_ManualUpdate);
    BSML::Register::RegisterMainMenu("NoteTrails", "NoteTrails", "Color-matched trails on incoming notes", didActivate);
}
