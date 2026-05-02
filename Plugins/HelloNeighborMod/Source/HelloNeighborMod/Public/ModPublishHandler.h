// (C) Xrisofor

#pragma once

#pragma once

#include "CoreMinimal.h"
#include "TickableEditorObject.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Interfaces/IPluginManager.h"

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	#include "Runtime/Launch/Resources/Version.h"
	#if ENGINE_MAJOR_VERSION >= 5
		#include "steam/steam_api.h"
		#include "steam/isteamugc.h"
	#else
		#include "ThirdParty/Steamworks/Steamv151/sdk/public/steam/steam_api.h"
		#include "ThirdParty/Steamworks/Steamv151/sdk/public/steam/isteamugc.h"
	#endif
#endif

class FModPublishHandler : public FTickableEditorObject
{
public:
	static FModPublishHandler& Get() { static FModPublishHandler Instance; return Instance; };
	
	void PublishMod(FString ModName, FString Platform);
	
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return bIsActive; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(FModPublishHandler, STATGROUP_Tickables); }

private:
	FModPublishHandler() : bIsActive(false), bIsUploading(false), CurrentUpdateHandle(0) { };
	
	void InternalStartUpload(FString ModName);
	void OnAutoPackageComplete(bool bSuccess, FString ModName, FString Platform) { if (bSuccess) InternalStartUpload(ModName); };
	
	void CreateOrUpdateItem();
	void SubmitModUpdate(PublishedFileId_t PublishedFileId);
	void OnItemCreated(CreateItemResult_t* pResult, bool bIOFailure);
	void OnItemUpdated(SubmitItemUpdateResult_t* pResult, bool bIOFailure);

	bool PromptUserForPreview();
	FString GetModPersistentIDPath(FString ModName);

	CCallResult<FModPublishHandler, CreateItemResult_t> CallResultItemCreated;
	CCallResult<FModPublishHandler, SubmitItemUpdateResult_t> CallResultItemUpdated;

	FString CurrentModName;
	FString CurrentPlatform;
	FString CurrentPackagePath;
	FString SelectedPreviewPath;
	
	bool bIsActive;
	bool bIsUploading;
	UGCUpdateHandle_t CurrentUpdateHandle;
	TSharedPtr<SNotificationItem> PublicationNotification;
};