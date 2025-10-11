// (C) Xrisofor

#pragma once

#include "IPluginWizardDefinition.h"
#include "PluginDescriptor.h"
#include "Widgets/SNullWidget.h"

class FMyPluginWizardDefinition : public IPluginWizardDefinition
{
public:
	FMyPluginWizardDefinition();
	
	virtual const TArray<TSharedRef<FPluginTemplateDescription>>& GetTemplatesSource() const override;
	virtual void OnTemplateSelectionChanged(TArray<TSharedRef<FPluginTemplateDescription>> InSelectedItems, ESelectInfo::Type SelectInfo) override;
	virtual TArray<TSharedPtr<FPluginTemplateDescription>> GetSelectedTemplates() const override;
	virtual void ClearTemplateSelection() override;
	virtual bool HasValidTemplateSelection() const override;

	virtual ESelectionMode::Type GetSelectionMode() const override { return ESelectionMode::Type::Multi; }
	virtual bool AllowsEnginePlugins() const override { return false; };
	virtual bool CanShowOnStartup() const override { return true; }
	virtual bool CanContainContent() const override { return true; };
	virtual bool HasModules() const override { return false; };
	virtual bool IsMod() const override { return true; };
	virtual void OnShowOnStartupCheckboxChanged(ECheckBoxState CheckBoxState) override { ShowOnStartupState = CheckBoxState; }
	virtual ECheckBoxState GetShowOnStartupCheckBoxState() const override { return ShowOnStartupState; }
	virtual TSharedPtr<class SWidget> GetCustomHeaderWidget() override;
	virtual FText GetInstructions() const override;

	virtual bool GetPluginIconPath(FString& OutIconPath) const override;
	virtual EHostType::Type GetPluginModuleDescriptor() const override;
	virtual ELoadingPhase::Type GetPluginLoadingPhase() const override;
	virtual bool GetTemplateIconPath(TSharedRef<FPluginTemplateDescription> InTemplate, FString& OutIconPath) const override;
	virtual FString GetPluginFolderPath() const override;
	virtual TArray<FString> GetFoldersForSelection() const override;
	virtual void PluginCreated(const FString& PluginName, bool bWasSuccessful) const override {};
	
	virtual void OnTemplateDoubleClick(TSharedPtr<FPluginTemplateDescription> InTemplate) {}
	virtual FText GetTemplateCategory() const { return FText::FromString(TEXT("Mods")); }
	virtual bool CanShowOnStarterContentOnly() const { return false; }
	virtual bool HasAdditionalSettings() const { return false; }
	virtual TSharedRef<SWidget> GetAdditionalSettingsWidget() { return SNullWidget::NullWidget; }
	virtual bool IsPluginValidForEnabledModules() const { return true; }

private:
	void PopulateTemplatesSource();
	FString GetFolderForTemplate(TSharedRef<FPluginTemplateDescription> InTemplate) const;
	
	FString PluginBaseDir;
	
	TArray<TSharedRef<FPluginTemplateDescription>> TemplateDefinitions;
	TSharedPtr<FPluginTemplateDescription> CurrentTemplateDefinition;

	ECheckBoxState ShowOnStartupState = ECheckBoxState::Unchecked;
};
