#include "PicrossEditor.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

IMPLEMENT_GAME_MODULE(FPicrossEditorModule, PicrossEditor);

DEFINE_LOG_CATEGORY(PicrossEditor);
#define LOCTEXT_NAMESPACE "PicrossEditor"

void FPicrossEditorModule::StartupModule()
{
    UE_LOG(PicrossEditor, Warning, TEXT("PicrossEditor: Log Started"));
}

void FPicrossEditorModule::ShutdownModule()
{
    UE_LOG(PicrossEditor, Warning, TEXT("PicrossEditor: Log Ended"));
}

#undef LOCTEXT_NAMESPACE