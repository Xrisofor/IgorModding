// (C) Xrisofor

#include "ModMetadataObject.h"
#include "Misc/Paths.h"
#include "Layout/Margin.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Interfaces/IPluginManager.h"
#include "PluginReferenceDescriptor.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "SExternalImageReference.h"

#pragma region Mod Plugin Reference Metadata
void FModPluginReferenceMetadata::PopulateFromDescriptor(const FPluginReferenceDescriptor& InDescriptor)
{
	Name = InDescriptor.Name;
	bEnabled = InDescriptor.bEnabled;
	bOptional = InDescriptor.bOptional;
}

void FModPluginReferenceMetadata::CopyIntoDescriptor(FPluginReferenceDescriptor& OutDescriptor) const
{
	OutDescriptor.Name = Name;
	OutDescriptor.bEnabled = bEnabled;
	OutDescriptor.bOptional = bOptional;
}
#pragma endregion

#pragma region Mod Metadata Object
void UModMetadataObject::PopulateFromDescriptor(const FPluginDescriptor& InDescriptor)
{
	Version = InDescriptor.Version;
	VersionName = InDescriptor.VersionName;
	FriendlyName = InDescriptor.FriendlyName;
	Description = InDescriptor.Description;
	Category = InDescriptor.Category;
	CreatedBy = InDescriptor.CreatedBy;
	CreatedByURL = InDescriptor.CreatedByURL;
	DocsURL = InDescriptor.DocsURL;
	MarketplaceURL = InDescriptor.MarketplaceURL;
	SupportURL = InDescriptor.SupportURL;
	bCanContainContent = InDescriptor.bCanContainContent;
	bIsBetaVersion = InDescriptor.bIsBetaVersion;

	Plugins.Reset(InDescriptor.Plugins.Num());
	for (const FPluginReferenceDescriptor& PluginRefDesc : InDescriptor.Plugins)
	{
		FModPluginReferenceMetadata& PluginRef = Plugins.AddDefaulted_GetRef();
		PluginRef.PopulateFromDescriptor(PluginRefDesc);
	}
}

void UModMetadataObject::CopyIntoDescriptor(FPluginDescriptor& OutDescriptor) const
{
	OutDescriptor.Version = Version;
	OutDescriptor.VersionName = VersionName;
	OutDescriptor.FriendlyName = FriendlyName;
	OutDescriptor.Description = Description;
	OutDescriptor.Category = Category;
	OutDescriptor.CreatedBy = CreatedBy;
	OutDescriptor.CreatedByURL = CreatedByURL;
	OutDescriptor.DocsURL = DocsURL;
	OutDescriptor.MarketplaceURL = MarketplaceURL;
	OutDescriptor.SupportURL = SupportURL;
	OutDescriptor.bCanContainContent = bCanContainContent;
	OutDescriptor.bIsBetaVersion = bIsBetaVersion;

	TArray<FPluginReferenceDescriptor> NewPlugins;
	NewPlugins.Reserve(Plugins.Num());

	for (const FModPluginReferenceMetadata& PluginRefMetadata : Plugins)
	{
		if (PluginRefMetadata.Name.IsEmpty())
		{
			continue;
		}

		FPluginReferenceDescriptor& NewPluginRefDesc = NewPlugins.AddDefaulted_GetRef();

		if (FPluginReferenceDescriptor* OldPluginRefDesc = OutDescriptor.Plugins.FindByPredicate([&PluginRefMetadata](const FPluginReferenceDescriptor& Item) { return Item.Name == PluginRefMetadata.Name; }))
		{
			NewPluginRefDesc = *OldPluginRefDesc;
			OldPluginRefDesc->Name.Empty();
		}

		PluginRefMetadata.CopyIntoDescriptor(NewPluginRefDesc);
	}

	OutDescriptor.Plugins = MoveTemp(NewPlugins);
}

void UModMetadataObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetName() == TEXT("TargetIconPath"))
		FEditorDelegates::RefreshAllBrowsers.Broadcast();
}
#pragma endregion

#pragma region Mod Plugin Metadata Customization
void FModPluginMetadataCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	if(Objects.Num() == 1 && Objects[0].IsValid())
	{
		UModMetadataObject* PluginMetadata = Cast<UModMetadataObject>(Objects[0].Get());
		if(PluginMetadata != nullptr && PluginMetadata->TargetIconPath.Len() > 0)
		{
			FString CurrentIconPath = PluginMetadata->TargetIconPath;
			if(!FPaths::FileExists(CurrentIconPath))
			{
				CurrentIconPath = IPluginManager::Get().FindPlugin(TEXT("PluginBrowser"))->GetBaseDir() / TEXT("Resources") / TEXT("DefaultIcon128.png");
			}
			
			IDetailCategoryBuilder& ImageCategory = DetailBuilder.EditCategory(TEXT("Icon"));
			const FText IconDesc(NSLOCTEXT("PluginBrowser", "PluginIcon", "Icon"));
			ImageCategory.AddCustomRow(IconDesc)
			.NameContent()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.Padding( FMargin( 0, 1, 0, 1 ) )
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(IconDesc)
					.Font(DetailBuilder.GetDetailFont())
				]
			]
			.ValueContent()
			.MaxDesiredWidth(500.0f)
			.MinDesiredWidth(100.0f)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(SExternalImageReference, CurrentIconPath, PluginMetadata->TargetIconPath)
					.FileDescription(IconDesc)
					.RequiredSize(FIntPoint(128, 128))
				]
			];
		}
	}
}
#pragma endregion