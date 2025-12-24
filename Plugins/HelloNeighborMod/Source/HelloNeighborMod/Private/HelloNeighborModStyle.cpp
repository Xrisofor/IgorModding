// (C) Xrisofor

#include "HelloNeighborModStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FHelloNeighborModStyle::StyleInstance = NULL;

void FHelloNeighborModStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FHelloNeighborModStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FHelloNeighborModStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("HelloNeighborModStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x40(16.0f, 16.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon80x80(80.0f, 80.0f);

TSharedRef< FSlateStyleSet > FHelloNeighborModStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("HelloNeighborModStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("HelloNeighborMod")->GetBaseDir() / TEXT("Resources"));

	Style->Set("HelloNeighborMod.ModIcon", new IMAGE_BRUSH(TEXT("Icon128"), Icon80x80));
	
	Style->Set("HelloNeighborMod.CreateNewMod", new IMAGE_BRUSH(TEXT("CreateIcon_40x"), Icon40x40));
	Style->Set("HelloNeighborMod.PackageMod", new IMAGE_BRUSH(TEXT("PackageIcon_40x"), Icon40x40));

	Style->Set("HelloNeighborMod.ModFolder", new IMAGE_BRUSH(TEXT("Folder"), Icon16x40));

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FHelloNeighborModStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FHelloNeighborModStyle::Get()
{
	return *StyleInstance;
}
