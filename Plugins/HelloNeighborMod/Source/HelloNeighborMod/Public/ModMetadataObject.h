// (C) Xrisofor

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "IDetailCustomization.h"
#include "ModMetadataObject.generated.h"

class IDetailLayoutBuilder;
struct FPluginDescriptor;

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
	bool bIsBetaVersion;

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