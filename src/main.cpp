#include "scotland2/shared/modloader.h"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/ColorType.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/TrailRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/UI/VerticalLayoutGroup.hpp"

#define MOD_ID "notetrails"
#define VERSION "0.1.0"

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
static bool trailsEnabled = true;
static float trailTime = 0.25f;
static float trailWidth = 0.45f;
MAKE_HOOK_MATCH(GameNoteController_Init, &GlobalNamespace::GameNoteController::Init, void,
    GlobalNamespace::GameNoteController* self, GlobalNamespace::NoteData* noteData,
    ByRef<GlobalNamespace::NoteSpawnData> noteSpawnData, float endRotation, float uniformScale, bool rotateTowardsPlayer, bool useRandomRotation) {
    GameNoteController_Init(self, noteData, noteSpawnData, endRotation, uniformScale, rotateTowardsPlayer, useRandomRotation);
    if (!self || !noteData) return;
    auto* go = self->get_gameObject();
    auto* tr = go->GetComponent<UnityEngine::TrailRenderer*>();
    if (!trailsEnabled) { if (tr) tr->set_enabled(false); return; }
    if (!tr) {
        tr = go->AddComponent<UnityEngine::TrailRenderer*>();
        if (auto* shader = UnityEngine::Shader::Find("Sprites/Default")) tr->set_material(UnityEngine::Material::New_ctor(shader));
    }
    tr->Clear(); tr->set_enabled(true); tr->set_time(trailTime); tr->set_startWidth(trailWidth); tr->set_endWidth(0.0f);
    bool isLeft = (noteData->colorType == GlobalNamespace::ColorType::ColorA);
    UnityEngine::Color col = isLeft ? UnityEngine::Color(1.0f, 0.12f, 0.18f, 0.85f) : UnityEngine::Color(0.12f, 0.55f, 1.0f, 0.85f);
    tr->set_startColor(col); col.a = 0.0f; tr->set_endColor(col);
}

static void didActivate(HMUI::ViewController* self, bool firstActivation, bool, bool) {
    if (!firstActivation) return;
    auto* parent = BSML::Lite::CreateScrollableSettingsContainer(self->get_transform());
    if (auto* vlg = parent->GetComponent<UnityEngine::UI::VerticalLayoutGroup*>()) { vlg->set_spacing(3.0f); vlg->set_childForceExpandHeight(false); }
    BSML::Lite::CreateText(parent, "=== NoteTrails (Quest 1.40.8 Port) ===");
    BSML::Lite::CreateToggle(parent, "Enable Note Trails", trailsEnabled, [](bool v) { trailsEnabled = v; });
    BSML::Lite::CreateIncrementSetting(parent, "Trail Duration (sec)", 2, 0.05f, trailTime, 0.05f, 1.0f, [](float v) { trailTime = v; });
    BSML::Lite::CreateIncrementSetting(parent, "Trail Width", 2, 0.05f, trailWidth, 0.10f, 1.0f, [](float v) { trailWidth = v; });
}

extern "C" void setup(CModInfo* info) { *info = modInfo.to_c(); Paper::Logger::RegisterFileContextId(MOD_ID); }
extern "C" void late_load() { il2cpp_functions::Init(); BSML::Init(); INSTALL_HOOK(Paper::Logger::WithContext(MOD_ID), GameNoteController_Init); BSML::Register::RegisterMainMenu("NoteTrails", "NoteTrails", "Color-matched trails on incoming notes", didActivate); }
