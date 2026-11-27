extern "C" void late_load() { il2cpp_functions::Init(); INSTALL_HOOK(Paper::Logger::WithContext<MOD_ID>(), NoteMovement_ManualUpdate); }
