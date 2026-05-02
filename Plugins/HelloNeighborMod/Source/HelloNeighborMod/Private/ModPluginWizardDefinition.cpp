// (C) Xrisofor

#include "ModPluginWizardDefinition.h"
#include "HelloNeighborModStyle.h"
#include "HelloNeighborModSettings.h"

#include "Interfaces/IPluginManager.h"
#include "PluginDescriptor.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Features/IPluginsEditorFeature.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SBoxPanel.h"

FModPluginWizardDefinition::FModPluginWizardDefinition()
{
	PluginBaseDir = IPluginManager::Get().FindPlugin(TEXT("HelloNeighborMod"))->GetBaseDir();
	PopulateTemplatesSource();
}

FText FModPluginWizardDefinition::GetInstructions() const
{
	return FText::FromString(TEXT("Select the content you want to first create in your mod, then give your mod a name to create it."));
}

bool FModPluginWizardDefinition::GetPluginIconPath(FString& OutIconPath) const
{
	OutIconPath = PluginBaseDir / TEXT("Resources/Icon128.png");
	return true;
}

#if ENGINE_MAJOR_VERSION >= 5
void FModPluginWizardDefinition::OnTemplateSelectionChanged(TSharedPtr<FPluginTemplateDescription> InSelectedItem, ESelectInfo::Type SelectInfo)
{
	CurrentTemplateDefinition = InSelectedItem;
}

TSharedPtr<FPluginTemplateDescription> FModPluginWizardDefinition::GetSelectedTemplate() const
{
	return CurrentTemplateDefinition;
}
#else
void FModPluginWizardDefinition::OnTemplateSelectionChanged(TArray<TSharedRef<FPluginTemplateDescription>> InSelectedItems, ESelectInfo::Type SelectInfo)
{
	CurrentTemplateDefinition.Reset();
	if (InSelectedItems.Num() > 0)
	{
		CurrentTemplateDefinition = InSelectedItems[0];
	}
}

TArray<TSharedPtr<FPluginTemplateDescription>> FModPluginWizardDefinition::GetSelectedTemplates() const
{
	TArray<TSharedPtr<FPluginTemplateDescription>> Selection;
	if (CurrentTemplateDefinition.IsValid())
	{
		Selection.Add(CurrentTemplateDefinition);
	}
	return Selection;
}
#endif

TSharedPtr<class SWidget> FModPluginWizardDefinition::GetCustomHeaderWidget()
{
	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.AutoWidth()
	[
		SNew(SImage)
		.Image(FHelloNeighborModStyle::Get().GetBrush("HelloNeighborMod.ModIcon"))
	];
}

EHostType::Type FModPluginWizardDefinition::GetPluginModuleDescriptor() const
{
	return CurrentTemplateDefinition.IsValid() ? CurrentTemplateDefinition->ModuleDescriptorType : EHostType::Runtime;
}

ELoadingPhase::Type FModPluginWizardDefinition::GetPluginLoadingPhase() const
{
	return CurrentTemplateDefinition.IsValid() ? CurrentTemplateDefinition->LoadingPhase : ELoadingPhase::Default;
}

bool FModPluginWizardDefinition::GetTemplateIconPath(TSharedRef<FPluginTemplateDescription> InTemplate, FString& OutIconPath) const
{
	const UHelloNeighborModSettings* Settings = UHelloNeighborModSettings::Get();
	const FString DefaultIcon = FPaths::Combine(PluginBaseDir, TEXT("Resources/Icon128.png"));

	if (const FPluginTemplate* Found = Settings->FindTemplateByName(InTemplate->Name))
	{
		OutIconPath = FPaths::Combine(PluginBaseDir, TEXT("Resources"), Found->IconFileName);
		if (FPaths::FileExists(OutIconPath)) return true;
	}
    
	OutIconPath = DefaultIcon;
	return true;
}

TArray<FString> FModPluginWizardDefinition::GetFoldersForSelection() const
{
	TArray<FString> SelectedFolders;
	if (CurrentTemplateDefinition.IsValid())
	{
		SelectedFolders.Add(GetFolderForTemplate(CurrentTemplateDefinition.ToSharedRef()));
	}
	return SelectedFolders;
}

void FModPluginWizardDefinition::PopulateTemplatesSource()
{
	for (const FPluginTemplate& T : UHelloNeighborModSettings::Get()->Templates)
	{
		TemplateDefinitions.Add(MakeShared<FPluginTemplateDescription>(
			T.TemplateName, T.TemplateDescription,
			FPaths::Combine(PluginBaseDir, TEXT("Templates"), T.TemplatePath),
			true, EHostType::Runtime
		));
	}
}

void FModPluginWizardDefinition::PluginCreated(const FString& PluginName, bool bWasSuccessful) const
{
    if (!bWasSuccessful) return;
    
    FString ProjectFilePath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
    FString ProjectJsonStr;
    if (!FFileHelper::LoadFileToString(ProjectJsonStr, *ProjectFilePath)) return;
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ProjectJsonStr);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid()) return;
    
    if (JsonObject->HasField("AdditionalPluginDirectories"))
    {
        JsonObject->RemoveField("AdditionalPluginDirectories");
        FString OutputString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
        if (FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
        {
            FFileHelper::SaveStringToFile(OutputString, *ProjectFilePath);
        }
    }
}