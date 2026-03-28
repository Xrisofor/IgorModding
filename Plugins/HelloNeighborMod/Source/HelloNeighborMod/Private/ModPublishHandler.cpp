// (C) Xrisofor

#include "ModPublishHandler.h"
#include "ModPackageHandler.h"
#include "DesktopPlatformModule.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

#define LOCTEXT_NAMESPACE "FHelloNeighborMod"

void FModPublishHandler::PublishMod(FString ModName, FString Platform)
{
	if (!SteamAPI_Init()) return;
	CurrentPlatform = Platform;
	
	if (FModPackageHandler::IsModPackaged(ModName, Platform))
	{
		InternalStartUpload(ModName);
	}
	else
	{
		FModPackageHandler::PackageMod(ModName, Platform, TEXT(""), 
			FModPackageHandler::FOnPackageComplete::CreateRaw(this, &FModPublishHandler::OnAutoPackageComplete));
	}
}

void FModPublishHandler::InternalStartUpload(FString ModName)
{
	check(IsInGameThread());
	
	CurrentModName = ModName;
	CurrentPackagePath = FModPackageHandler::GetPackagePath(ModName);
	
	PromptUserForPreview();

	bIsActive = true;
	FNotificationInfo Info(FText::Format(LOCTEXT("PublishingStart", "Initializing Workshop upload for {0}..."), FText::FromString(ModName)));
	Info.bFireAndForget = false;
	Info.bUseThrobber = true;
	PublicationNotification = FSlateNotificationManager::Get().AddNotification(Info);

	CreateOrUpdateItem();
}

void FModPublishHandler::CreateOrUpdateItem()
{
	FString SavedIDStr;
	FString IDPath = GetModPersistentIDPath(CurrentModName);
	
	if (!IDPath.IsEmpty() && FFileHelper::LoadFileToString(SavedIDStr, *IDPath))
	{
		SubmitModUpdate((PublishedFileId_t)FCString::Atod(*SavedIDStr));
		return;
	}
	
	SteamAPICall_t hSteamAPICall = SteamUGC()->CreateItem(SteamUtils()->GetAppID(), k_EWorkshopFileTypeCommunity);
	CallResultItemCreated.Set(hSteamAPICall, this, &FModPublishHandler::OnItemCreated);
}

void FModPublishHandler::SubmitModUpdate(PublishedFileId_t PublishedFileId)
{
	CurrentUpdateHandle = SteamUGC()->StartItemUpdate(SteamUtils()->GetAppID(), PublishedFileId);

	TSharedPtr<IPlugin> ModPlugin = IPluginManager::Get().FindPlugin(CurrentModName);
	if (ModPlugin.IsValid())
	{
		const FPluginDescriptor& Descriptor = ModPlugin->GetDescriptor();
		
		SteamUGC()->SetItemTitle(CurrentUpdateHandle, TCHAR_TO_UTF8(*Descriptor.FriendlyName));
		SteamUGC()->SetItemDescription(CurrentUpdateHandle, TCHAR_TO_UTF8(*Descriptor.Description));
		
		FString IconToUse = SelectedPreviewPath;
		
		FString Dummy;
		bool bIsNewMod = !FFileHelper::LoadFileToString(Dummy, *GetModPersistentIDPath(CurrentModName));

		if (IconToUse.IsEmpty() && bIsNewMod)
		{
			FString DefaultIcon = FPaths::Combine(ModPlugin->GetBaseDir(), TEXT("Resources"), TEXT("Icon128.png"));
			if (FPaths::FileExists(DefaultIcon)) IconToUse = DefaultIcon;
		}
		
		if (!IconToUse.IsEmpty())
		{
			FString AbsPreviewPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*IconToUse);
			SteamUGC()->SetItemPreview(CurrentUpdateHandle, TCHAR_TO_UTF8(*AbsPreviewPath));
		}
	}

	FString AbsContentPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*CurrentPackagePath);
	SteamUGC()->SetItemContent(CurrentUpdateHandle, TCHAR_TO_UTF8(*AbsContentPath));
	SteamUGC()->SetItemVisibility(CurrentUpdateHandle, k_ERemoteStoragePublishedFileVisibilityPublic);

	SteamAPICall_t hSteamAPICall = SteamUGC()->SubmitItemUpdate(CurrentUpdateHandle, "Version update");
	CallResultItemUpdated.Set(hSteamAPICall, this, &FModPublishHandler::OnItemUpdated);

	bIsUploading = true;
}

void FModPublishHandler::Tick(float DeltaTime)
{
	SteamAPI_RunCallbacks();

	if (!bIsUploading || !PublicationNotification.IsValid()) return;

	uint64 BytesProcessed = 0, BytesTotal = 0;
	EItemUpdateStatus Status = SteamUGC()->GetItemUpdateProgress(CurrentUpdateHandle, &BytesProcessed, &BytesTotal);

	FText StatusText = LOCTEXT("Status_Working", "Working...");
	switch (Status)
	{
		case k_EItemUpdateStatusPreparingConfig: StatusText = LOCTEXT("Status_Config", "Preparing config..."); break;
		case k_EItemUpdateStatusUploadingContent: 
		{
			int32 P = (BytesTotal > 0) ? FMath::FloorToInt((float)BytesProcessed / (float)BytesTotal * 100.0f) : 0;
			StatusText = FText::Format(LOCTEXT("Status_Content", "Uploading content: {0}%"), FText::AsNumber(P));
			break;
		}
		case k_EItemUpdateStatusUploadingPreviewFile: StatusText = LOCTEXT("Status_Preview", "Uploading preview image..."); break;
		case k_EItemUpdateStatusCommittingChanges: StatusText = LOCTEXT("Status_Final", "Committing changes..."); break;
	}
	PublicationNotification->SetText(StatusText);
}

void FModPublishHandler::OnItemCreated(CreateItemResult_t* pResult, bool bIOFailure)
{
	if (bIOFailure || !pResult || pResult->m_eResult != k_EResultOK)
	{
		bIsActive = false;
		if (PublicationNotification.IsValid()) PublicationNotification->SetCompletionState(SNotificationItem::CS_Fail);
		return;
	}

	// Сохраняем ID в корень мода
	FString IDPath = GetModPersistentIDPath(CurrentModName);
	if (!IDPath.IsEmpty()) FFileHelper::SaveStringToFile(FString::Printf(TEXT("%llu"), pResult->m_nPublishedFileId), *IDPath);

	SubmitModUpdate(pResult->m_nPublishedFileId);
}

void FModPublishHandler::OnItemUpdated(SubmitItemUpdateResult_t* pResult, bool bIOFailure)
{
	bIsActive = bIsUploading = false;
	if (PublicationNotification.IsValid())
	{
		bool bSuccess = !bIOFailure && pResult->m_eResult == k_EResultOK;
		PublicationNotification->SetText(bSuccess ? LOCTEXT("PubSuccess", "Mod published!") : LOCTEXT("PubFail", "Upload failed!"));
		PublicationNotification->SetCompletionState(bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
		PublicationNotification->ExpireAndFadeout();
	}
}

bool FModPublishHandler::PromptUserForPreview()
{
	IDesktopPlatform* DP = FDesktopPlatformModule::Get();
	TArray<FString> OutFiles;
	if (DP && DP->OpenFileDialog(nullptr, TEXT("Select Preview (Optional)"), TEXT(""), TEXT(""), TEXT("Images|*.jpg;*.png;*.jpeg"), EFileDialogFlags::None, OutFiles))
	{
		SelectedPreviewPath = OutFiles[0];
		return true;
	}
	SelectedPreviewPath = "";
	return false;
}

FString FModPublishHandler::GetModPersistentIDPath(FString ModName)
{
	TSharedPtr<IPlugin> P = IPluginManager::Get().FindPlugin(ModName);
	return P.IsValid() ? FPaths::Combine(P->GetBaseDir(), TEXT("steam_id.txt")) : FString();
}

#undef LOCTEXT_NAMESPACE