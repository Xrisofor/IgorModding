// (C) Xrisofor

#include "MyPluginWizardDefinition.h"

#include "HelloNeighborModStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Features/IPluginsEditorFeature.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "PluginDescriptor.h"
#include "Brushes/SlateImageBrush.h"

FMyPluginWizardDefinition::FMyPluginWizardDefinition()
{
	PluginBaseDir = IPluginManager::Get().FindPlugin(TEXT("HelloNeighborMod"))->GetBaseDir();
	PopulateTemplatesSource();
}

const TArray<TSharedRef<FPluginTemplateDescription>>& FMyPluginWizardDefinition::GetTemplatesSource() const
{
	return TemplateDefinitions;
}

void FMyPluginWizardDefinition::OnTemplateSelectionChanged(TArray<TSharedRef<FPluginTemplateDescription>> InSelectedItems, ESelectInfo::Type SelectInfo)
{
	CurrentTemplateDefinition.Reset();
	
	if (InSelectedItems.Num() > 0)
	{
		CurrentTemplateDefinition = InSelectedItems[0];
	}
}

TArray<TSharedPtr<FPluginTemplateDescription>> FMyPluginWizardDefinition::GetSelectedTemplates() const
{
	TArray<TSharedPtr<FPluginTemplateDescription>> Selection;
	if (CurrentTemplateDefinition.IsValid())
	{
		Selection.Add(CurrentTemplateDefinition);
	}

	return Selection;
}

bool FMyPluginWizardDefinition::HasValidTemplateSelection() const
{
	return CurrentTemplateDefinition.IsValid();
}

void FMyPluginWizardDefinition::ClearTemplateSelection()
{
	CurrentTemplateDefinition.Reset();
}

TSharedPtr<class SWidget> FMyPluginWizardDefinition::GetCustomHeaderWidget()
{
	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.AutoWidth()
	[
		SNew(SImage)
		.Image(FHelloNeighborModStyle::Get().GetBrush("HelloNeighborMod.ModIcon"))
	];
}

FText FMyPluginWizardDefinition::GetInstructions() const
{
	return FText::FromString(TEXT("Select the content you want to first create in your mod, then give your mod a name to create it.\n\nUse Ctrl+Click to select multiple options, or Shift+Click to select a range of options."));
}

bool FMyPluginWizardDefinition::GetPluginIconPath(FString& OutIconPath) const
{
	OutIconPath = PluginBaseDir / TEXT("Resources/Icon128.png");
	return true;
}

EHostType::Type FMyPluginWizardDefinition::GetPluginModuleDescriptor() const
{
	EHostType::Type ModuleDescriptorType = EHostType::Runtime;

	if (CurrentTemplateDefinition.IsValid())
	{
		ModuleDescriptorType = CurrentTemplateDefinition->ModuleDescriptorType;
	}

	return ModuleDescriptorType;
}

ELoadingPhase::Type FMyPluginWizardDefinition::GetPluginLoadingPhase() const
{
	ELoadingPhase::Type Phase = ELoadingPhase::Default;

	if (CurrentTemplateDefinition.IsValid())
	{
		Phase = CurrentTemplateDefinition->LoadingPhase;
	}

	return Phase;
}

bool FMyPluginWizardDefinition::GetTemplateIconPath(TSharedRef<FPluginTemplateDescription> InTemplate, FString& OutIconPath) const
{
	FString TemplateFolderName = GetFolderForTemplate(InTemplate);
	FString IconFileName;
	
	if (InTemplate->Name.ToString() == TEXT("Empty Map"))
	{
		IconFileName = TEXT("EmptyMap128.png");
	}
	else if (InTemplate->Name.ToString() == TEXT("Test Field Map"))
	{
		IconFileName = TEXT("TestFieldMap128.png");
	}
	else if (InTemplate->Name.ToString() == TEXT("AI Setup Map"))
	{
		IconFileName = TEXT("AiSetupMap128.png");
	}
	else
	{
		IconFileName = TEXT("Icon128.png");
	}

	OutIconPath = PluginBaseDir / TEXT("Resources") / IconFileName;
	
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*OutIconPath))
	{
		OutIconPath = PluginBaseDir / TEXT("Resources/Icon128.png");
		return true;
	}

	return false;
}

FString FMyPluginWizardDefinition::GetPluginFolderPath() const
{
	return GetFolderForTemplate(CurrentTemplateDefinition.ToSharedRef());
}

TArray<FString> FMyPluginWizardDefinition::GetFoldersForSelection() const
{
	TArray<FString> SelectedFolders;

	if (CurrentTemplateDefinition.IsValid())
	{
		SelectedFolders.Add(GetFolderForTemplate(CurrentTemplateDefinition.ToSharedRef()));
	}

	return SelectedFolders;
}

void FMyPluginWizardDefinition::PopulateTemplatesSource()
{
	TSharedRef<FPluginTemplateDescription> EmptyMapTemplate = MakeShareable(new FPluginTemplateDescription(
		FText::FromString("Empty Map"),
		FText::FromString("Creates a new empty map"),
		PluginBaseDir / TEXT("Templates") / TEXT("EmptyMap"),
		true,
		EHostType::Runtime)
	);
	TemplateDefinitions.Add(EmptyMapTemplate);

	TSharedRef<FPluginTemplateDescription> TestFieldMapTemplate = MakeShareable(new FPluginTemplateDescription(
		FText::FromString("Test Field Map"),
		FText::FromString("Simple map to try new things out"),
		PluginBaseDir / TEXT("Templates") / TEXT("TestFieldMap"),
		true,
		EHostType::Runtime)
	);
	TemplateDefinitions.Add(TestFieldMapTemplate);

	TSharedRef<FPluginTemplateDescription> AISetupMapTemplate = MakeShareable(new FPluginTemplateDescription(
		FText::FromString("AI Setup Map"),
		FText::FromString("Basic map to learn AI setup"),
		PluginBaseDir / TEXT("Templates") / TEXT("AISetupMap"),
		true,
		EHostType::Runtime)
	);
	TemplateDefinitions.Add(AISetupMapTemplate);
}

FString FMyPluginWizardDefinition::GetFolderForTemplate(TSharedRef<FPluginTemplateDescription> InTemplate) const
{
	return InTemplate->OnDiskPath;
}
