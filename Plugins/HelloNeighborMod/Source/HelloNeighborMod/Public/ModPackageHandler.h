// (C) Xrisofor

#pragma once

#include "CoreMinimal.h"
#include "Misc/Paths.h"

class FModPackageHandler
{
public:
	DECLARE_DELEGATE_ThreeParams(FOnPackageComplete, bool, FString, FString);

	static void PackageMod(FString ModName, FString TargetPlatform, FString CookFlavor, FOnPackageComplete OnComplete = FOnPackageComplete());
	static FString GetStageFolderName(FString Platform, FString Flavor);
	
	static FString GetPackagePath(FString ModName) { return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ModPackage"), ModName); };
	static bool IsModPackaged(FString ModName, FString Platform);
	
private:
	static void MovePackagedFiles(FString ModName, FString ProjectName, FString StageFolder, FOnPackageComplete OnComplete);
};
