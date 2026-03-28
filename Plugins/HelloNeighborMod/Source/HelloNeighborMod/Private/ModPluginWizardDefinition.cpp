// (C) Xrisofor

#include "ModPluginWizardDefinition.h"
#include "HelloNeighborModStyle.h"
#include "HelloNeighborModSettings.h"

#include "Interfaces/IPluginManager.h"
#include "PluginDescriptor.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"

FModPluginWizardDefinition::FModPluginWizardDefinition()
{
	PluginBaseDir = IPluginManager::Get().FindPlugin(TEXT("HelloNeighborMod"))->GetBaseDir();
	PopulateTemplatesSource();
}

FText FModPluginWizardDefinition::GetInstructions() const
{
	return FText::FromString(TEXT("Select the content you want to first create in your mod, then give your mod a name to create it.\n\nUse Ctrl+Click to select multiple options, or Shift+Click to select a range of options."));
}

bool FModPluginWizardDefinition::GetPluginIconPath(FString& OutIconPath) const
{
	OutIconPath = PluginBaseDir / TEXT("Resources/Icon128.png");
	return true;
}

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
	EHostType::Type ModuleDescriptorType = EHostType::Runtime;

	if (CurrentTemplateDefinition.IsValid())
	{
		ModuleDescriptorType = CurrentTemplateDefinition->ModuleDescriptorType;
	}

	return ModuleDescriptorType;
}

ELoadingPhase::Type FModPluginWizardDefinition::GetPluginLoadingPhase() const
{
	ELoadingPhase::Type Phase = ELoadingPhase::Default;

	if (CurrentTemplateDefinition.IsValid())
	{
		Phase = CurrentTemplateDefinition->LoadingPhase;
	}

	return Phase;
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
	if (!bWasSuccessful)
		return;
	
	FString ProjectFilePath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
	
	FString ProjectJsonStr;
	if (!FFileHelper::LoadFileToString(ProjectJsonStr, *ProjectFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load project file: %s"), *ProjectFilePath);
		return;
	}
	
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ProjectJsonStr);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to parse project JSON: %s"), *ProjectFilePath);
		return;
	}
	
	if (JsonObject->HasField("AdditionalPluginDirectories"))
	{
		JsonObject->RemoveField("AdditionalPluginDirectories");
		UE_LOG(LogTemp, Display, TEXT("Removed AdditionalPluginDirectories from project file"));
	}
	
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to serialize project JSON: %s"), *ProjectFilePath);
		return;
	}

	if (!FFileHelper::SaveStringToFile(OutputString, *ProjectFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to save project file: %s"), *ProjectFilePath);
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("PluginCreated cleanup completed for %s"), *PluginName);
}
