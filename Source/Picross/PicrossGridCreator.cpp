// Copyright Sanya Larsson 2020

extern UNREALED_API class UEditorEngine* GEditor;

#include "PicrossGridCreator.h"
#include "PicrossPuzzleFactory.h"
#include "AssetToolsModule.h"
#include "Editor/EditorEngine.h"


void APicrossGridCreator::BeginPlay()
{
	Super::BeginPlay();

	ClosePuzzleBrowser();
	CreatePuzzle();
}

void APicrossGridCreator::CreatePuzzle()
{
	CreatePuzzleWithSize(GridSize);
}

void APicrossGridCreator::CreatePuzzleWithSize(FIntVector SizeOfGrid)
{
	UPicrossPuzzleData* PuzzleData = NewObject<UPicrossPuzzleData>();
	PuzzleData->SetGridSize(SizeOfGrid);
	Puzzle = FPicrossPuzzle(PuzzleData);
	CreateGrid();
	SolutionFilledBlocksCount = -1;
}

void APicrossGridCreator::SavePuzzle()
{
	if (!Puzzle.IsValid()) return;

	UPicrossPuzzleData* PuzzleData = Puzzle.GetPuzzleData();
	TArray<bool> Solution;

	for (const FPicrossBlock& Block : Puzzle)
	{
		Solution.Add(Block.State == EBlockState::Filled);
	}
	PuzzleData->SetSolution(Solution);

	UPicrossPuzzleFactory* NewFactory = NewObject<UPicrossPuzzleFactory>();
	NewFactory->CreatedObjectAsset = PuzzleData;

	FAssetToolsModule& AssetToolsModule = FAssetToolsModule::GetModule();
	UObject* NewAsset = AssetToolsModule.Get().CreateAssetWithDialog(NewFactory->GetSupportedClass(), NewFactory);
	TArray<UObject*> ObjectsToSync;
	ObjectsToSync.Add(NewAsset);
	GEditor->SyncBrowserToObjects(ObjectsToSync);
}