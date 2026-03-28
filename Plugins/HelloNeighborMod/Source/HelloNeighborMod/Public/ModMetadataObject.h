// (C) Xrisofor

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "IDetailCustomization.h"
#include "ModMetadataObject.generated.h"

class IDetailLayoutBuilder;
struct FPluginDescriptor;
struct FPluginReferenceDescriptor;

USTRUCT()
struct FModPluginReferenceMetadata
{
public:
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Plugin Reference")
	FString Name;
	
	UPROPERTY(EditAnywhere, Category = "Plugin Reference")
	bool bOptional = false;
	
	UPROPERTY(EditAnywhere, Category = "Plugin Reference")
	bool bEnabled = true;
	
	void PopulateFromDescriptor(const FPluginReferenceDescriptor& InDescriptor);
	void CopyIntoDescriptor(FPluginReferenceDescriptor& OutDescriptor) const;
};

UCLASS()
class UModMetadataObject : public UObject
{
	GENERATED_BODY()

public:
	FString TargetIconPath;
	
	UPROPERTY(VisibleAnywhere, Category = Details)
	int32 Version;
	
	UPROPERTY(EditAnywhere, Category = Details)
	FString VersionName;
	
	UPROPERTY(EditAnywhere, Category = Details)
	FString FriendlyName;
	
	UPROPERTY(EditAnywhere, Category = Details)
	FString Description;
	
	UPROPERTY(EditAnywhere, Category = Details)
	FString Category;
	
	UPROPERTY(EditAnywhere, Category = Details)
	FString CreatedBy;
	
	UPROPERTY(EditAnywhere, Category = Details)
	FString CreatedByURL;
	
	UPROPERTY(EditAnywhere, Category = Details)
	FString DocsURL;
	
	UPROPERTY(EditAnywhere, Category = Details)
	FString MarketplaceURL;
	
	UPROPERTY(EditAnywhere, Category = Details)
	FString SupportURL;
	
	UPROPERTY(EditAnywhere, Category = Details)
	bool bCanContainContent;
	
	UPROPERTY(EditAnywhere, Category = Details)
	bool bIsBetaVersion;
	
	UPROPERTY(EditAnywhere, Category = Dependencies)
	TArray<FModPluginReferenceMetadata> Plugins;

	void PopulateFromDescriptor(const FPluginDescriptor& InDescriptor);
	void CopyIntoDescriptor(FPluginDescriptor& OutDescriptor) const;
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};

class FModPluginMetadataCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShareable(new FModPluginMetadataCustomization()); };
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};