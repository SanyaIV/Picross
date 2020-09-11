// Copyright Sanya Larsson 2020

#include "PicrossGrid.h"
#include "Algo/Count.h"
#include "Algo/Reverse.h"
#include "AssetDataObject.h"
#include "AssetToolsModule.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "IAssetTools.h"
#include "UnrealEd.h"
#include "PicrossPuzzleData.h"
#include "PicrossPuzzleFactory.h"
#include "FArray3D.h"

// Sets default values
APicrossGrid::APicrossGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	BlockInstances.Add(EBlockState::Clear, CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Clear Blocks")));
	BlockInstances.Add(EBlockState::Crossed, CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Crossed Blocks")));
	BlockInstances.Add(EBlockState::Filled, CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Filled Blocks")));

	for (auto& Pair : BlockInstances)
	{
		if (Pair.Value)
		{
			Pair.Value->NumCustomDataFloats = 1; // This custom data represents the MasterIndex. Stored as float, cast to int32 required when reading.
			Pair.Value->SetupAttachment(GetRootComponent());
		}
	}
}

// Called when the game starts or when spawned
void APicrossGrid::BeginPlay()
{
	Super::BeginPlay();

	for (auto& Pair : BlockInstances)
	{
		if (Pair.Value)
		{
			if (ensureAlwaysMsgf(BlockMeshes.Contains(Pair.Key), TEXT("Block Meshes missing key: %s"), *UEnum::GetValueAsString<EBlockState>(Pair.Key))
				&& ensureAlwaysMsgf(BlockMeshes.FindChecked(Pair.Key), TEXT("Block Meshes missing mesh for key: %s"), *UEnum::GetValueAsString<EBlockState>(Pair.Key))
				&& ensureAlwaysMsgf(BlockMaterials.Contains(Pair.Key), TEXT("Block Materials missing key: %s"), *UEnum::GetValueAsString<EBlockState>(Pair.Key))
				&& ensureAlwaysMsgf(BlockMaterials.FindChecked(Pair.Key), TEXT("Block Materials missing material for key: %s"), *UEnum::GetValueAsString<EBlockState>(Pair.Key)))
			{
				Pair.Value->SetStaticMesh(BlockMeshes[Pair.Key]);
				Pair.Value->SetMaterial(0, BlockMaterials[Pair.Key]);
			}
		}
	}

	CreatePuzzleBrowser();
}

void APicrossGrid::CreatePuzzleBrowser()
{
	if (PuzzleBrowserWidgetClass)
	{
		PuzzleBrowserWidget = CreateWidget(GetWorld(), PuzzleBrowserWidgetClass, TEXT("Puzzle Browser"));
		if (PuzzleBrowserWidget)
		{
			PuzzleBrowserWidget->AddToViewport();
			UListView* List = Cast<UListView>(PuzzleBrowserWidget->GetWidgetFromName(TEXT("Puzzle_Browser")));
			if (List)
			{
				for (FAssetData AssetData : GetAllPuzzles())
				{
					UAssetDataObject* AssetDataObject = NewObject<UAssetDataObject>(List, UAssetDataObject::StaticClass());
					AssetDataObject->SetAssetData(AssetData);
					AssetDataObject->SetPicrossGrid(this);
					List->AddItem(AssetDataObject);
				}
			}
		}
	}
}

void APicrossGrid::OpenPuzzleBrowser() const
{
	if (PuzzleBrowserWidget)
	{
		PuzzleBrowserWidget->AddToViewport();
	}
}

void APicrossGrid::ClosePuzzleBrowser() const
{
	if (PuzzleBrowserWidget)
	{
		PuzzleBrowserWidget->RemoveFromViewport();
	}
}

TArray<FAssetData> APicrossGrid::GetAllPuzzles() const
{
	UObjectLibrary* ObjectLibrary = nullptr;
	if (!ObjectLibrary)
	{
		UAssetManager& AssetManager = UAssetManager::Get();
		TArray<FAssetData> AssetDatas;
		AssetManager.GetPrimaryAssetDataList(TEXT("PicrossPuzzleData"), AssetDatas);
		return AssetDatas;
	}

	return TArray<FAssetData>();
}

void APicrossGrid::CreateGrid()
{
	Unlock();
	SelectionAxis = ESelectionAxis::All;
	LastPivotXYZ = FIntVector::ZeroValue;
	SolutionFilledBlocksCount = CurrentPuzzle ? Algo::Count(CurrentPuzzle->GetSolution(), true) : -1;
	CurrentlyFilledBlocksCount = 0;
	GridSize = CurrentPuzzle ? CurrentPuzzle->GetGridSize() : DefaultGridSize;
	verify(FArray3D::ValidateDimensions(GridSize));

	if (MasterGrid.Num() > 0)
	{
		DestroyGrid();
	}

	MasterGrid.SetNum(FArray3D::Size(GridSize));

	FVector StartPosition = GetActorLocation();
	StartPosition -= GetActorRightVector() * (DistanceBetweenBlocks * (GridSize.Y / 2) - (GridSize.Y % 2 == 0 ? DistanceBetweenBlocks / 2 : 0));
	StartPosition -= GetActorForwardVector() * (DistanceBetweenBlocks * (GridSize.X / 2) - (GridSize.X % 2 == 0 ? DistanceBetweenBlocks / 2 : 0));

	for (int32 Z = 0; Z < GridSize.Z; ++Z)
	{
		float OffsetZ = DistanceBetweenBlocks * Z;
		for (int32 Y = 0; Y < GridSize.Y; ++Y)
		{
			float OffsetY = DistanceBetweenBlocks * Y;
			for (int32 X = 0; X < GridSize.X; ++X)
			{
				float OffsetX = DistanceBetweenBlocks * X;
				FVector BlockPosition = StartPosition;
				BlockPosition += GetActorForwardVector() * OffsetX;
				BlockPosition += GetActorRightVector() * OffsetY;
				BlockPosition += GetActorUpVector() * OffsetZ;

				int32 MasterIndex = FArray3D::TranslateTo1D(GridSize, X, Y, Z);
				if (MasterGrid.IsValidIndex(MasterIndex))
				{
					FPicrossBlock& Block = MasterGrid[MasterIndex] = FPicrossBlock{ EBlockState::Clear, FTransform(GetActorRotation(), BlockPosition, FVector::OneVector), MasterIndex };
					CreateBlockInstance(Block);
				}
			}
		}
	}

	GenerateNumbers();
}

void APicrossGrid::ClearGrid()
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (FPicrossBlock& Block : MasterGrid)
	{
		Block.State = EBlockState::Clear;
		CreateBlockInstance(Block);
	}
}

void APicrossGrid::DestroyGrid()
{
	for (auto& ISM : BlockInstances)
	{
		if (ISM.Value)
		{
			ISM.Value->ClearInstances();
		}
	}

	MasterGrid.Empty();
}

void APicrossGrid::FillBlock(const int32 MasterIndex, const int32 InstanceIndex)
{
	if (!IsLocked() && MasterGrid.IsValidIndex(MasterIndex))
	{
		FPicrossBlock& Block = MasterGrid[MasterIndex];
		UpdateBlockState(Block, (Block.State != EBlockState::Filled) ? EBlockState::Filled : EBlockState::Clear, InstanceIndex);
	}
}

void APicrossGrid::CrossBlock(const int32 MasterIndex, const int32 InstanceIndex)
{
	if (!IsLocked() && MasterGrid.IsValidIndex(MasterIndex))
	{
		FPicrossBlock& Block = MasterGrid[MasterIndex];
		UpdateBlockState(Block, (Block.State != EBlockState::Crossed) ? EBlockState::Crossed : EBlockState::Clear, InstanceIndex);
	}
}

void APicrossGrid::EnableOnlyFilledBlocks() const
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (const FPicrossBlock& Block : MasterGrid)
	{
		if (Block.State == EBlockState::Filled)
		{
			CreateBlockInstance(Block);
		}
	}
}

void APicrossGrid::UpdateBlockState(FPicrossBlock& Block, const EBlockState NewState, const int32 PreviousInstanceIndex)
{
	if (IsLocked()) return;

	if (Block.State != NewState)
	{
		const EBlockState PreviousState = Block.State;
		BlockInstances[PreviousState]->RemoveInstance(PreviousInstanceIndex);

		Block.State = NewState;
		CreateBlockInstance(Block);

		CurrentlyFilledBlocksCount += PreviousState == EBlockState::Filled ? -1 : NewState == EBlockState::Filled ? 1 : 0;
		TrySolve();
	}
}

void APicrossGrid::CreateBlockInstance(const FPicrossBlock& Block) const
{
	BlockInstances[Block.State]->SetCustomDataValue(BlockInstances[Block.State]->AddInstanceWorldSpace(Block.Transform), 0, static_cast<float>(Block.MasterIndex));
}

void APicrossGrid::GenerateNumbers() const
{
	GenerateNumbersForAxis(ESelectionAxis::X);
	GenerateNumbersForAxis(ESelectionAxis::Y);
	GenerateNumbersForAxis(ESelectionAxis::Z);
}

void APicrossGrid::GenerateNumbersForAxis(ESelectionAxis Axis) const
{
	/*if (!CurrentPuzzle) return;

	const TArray<bool>& Solution = CurrentPuzzle->GetSolution();

	for (int32 Axis1 = 0; Axis1 < (Axis == ESelectionAxis::X ? GridSize.Y : GridSize.X); ++Axis1)
	{
		for (int32 Axis2 = 0; Axis2 < (Axis == ESelectionAxis::Z ? GridSize.Y : GridSize.Z); ++Axis2)
		{
			FFormatOrderedArguments Numbers;
			int32 Sum = 0;

			for (int32 Axis3 = (Axis == ESelectionAxis::Z ? GridSize.Z -1 : 0); (Axis == ESelectionAxis::Z ? Axis3 >= 0 : Axis == ESelectionAxis::X ? Axis3 < GridSize.X : Axis3 < GridSize.Y); Axis == ESelectionAxis::Z ? --Axis3 : ++Axis3)
			{
				FIntVector XYZ = Axis == ESelectionAxis::X ? FIntVector(Axis3, Axis1, Axis2) : Axis == ESelectionAxis::Y ? FIntVector(Axis1, Axis3, Axis2) : FIntVector(Axis1, Axis2, Axis3);

				if (Solution[FArray3D::TranslateTo1D(GridSize, XYZ)])
				{
					++Sum;
				}
				else if (Sum > 0)
				{
					Numbers.Add(Sum);
					Sum = 0;
				}
			}

			if (Sum > 0)
			{
				Numbers.Add(Sum);
			}

			FIntVector XYZ = Axis == ESelectionAxis::X ? FIntVector(0, Axis1, Axis2) : Axis == ESelectionAxis::Y ? FIntVector(Axis1, 0, Axis2) : FIntVector(Axis1, Axis2, GridSize.Z - 1);
			APicrossBlock* Block = MasterGrid[FArray3D::TranslateTo1D(GridSize, XYZ)];

			FVector RelativeLocation = Axis == ESelectionAxis::X ? FVector(-75.f, 0.f, 50.f) : Axis == ESelectionAxis::Y ? FVector(0.f, -75.f, 50.f) : FVector(0.f, 0.f, 115.f);
			FRotator RelativeRotation = Axis == ESelectionAxis::X ? FRotator(0.f, 90.f, 0.f) : Axis == ESelectionAxis::Y ? FRotator(0.f, 180.f, 0.f) : FRotator(0.f, 90.f, 0.f);
			FText Text = FText::Join(FText::FromString(Axis == ESelectionAxis::Z ? TEXT("\n") : TEXT(", ")), Numbers);
			FColor Color = Axis == ESelectionAxis::Z ? FColor::Blue : Axis == ESelectionAxis::Y ? FColor::Green : FColor::Red;
			EHorizTextAligment HAlignment = Axis == ESelectionAxis::Z ? EHorizTextAligment::EHTA_Center : EHorizTextAligment::EHTA_Right;
			EVerticalTextAligment VAlignment = Axis == ESelectionAxis::Z ? EVerticalTextAligment::EVRTA_TextBottom : EVerticalTextAligment::EVRTA_TextCenter;
			

			FRotator RelativeRotation2 = Axis == ESelectionAxis::X ? FRotator(0.f, -90.f, 0.f) : Axis == ESelectionAxis::Y ? FRotator::ZeroRotator : FRotator(0.f, -90.f, 0.f);
			Algo::Reverse(Numbers);
			FText Text2 = Axis == ESelectionAxis::Z ? Text : FText::Join(FText::FromString(TEXT(", ")), Numbers);
			EHorizTextAligment HAlignment2 = Axis == ESelectionAxis::Z ? EHorizTextAligment::EHTA_Center : EHorizTextAligment::EHTA_Left;

			CreateAndAttachTextToBlock(Block, RelativeLocation, RelativeRotation, Text, Color, HAlignment, VAlignment);
			CreateAndAttachTextToBlock(Block, RelativeLocation, RelativeRotation2, Text2, Color, HAlignment2, VAlignment);
		}
	}*/
}

void APicrossGrid::CreateTextRender(FVector WorldLocation, FRotator WorldRotation, FText Text, FColor Color, EHorizTextAligment HAlignment, EVerticalTextAligment VAlignment) const
{
	/*UTextRenderComponent* TextComponent = NewObject<UTextRenderComponent>();
	if (TextComponent)
	{
		TextComponent->RegisterComponent();
		TextComponent->AttachToComponent(Block->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		TextComponent->SetRelativeLocation(RelativeLocation);
		TextComponent->SetRelativeRotation(RelativeRotation);
		TextComponent->SetText(Text);
		TextComponent->SetTextRenderColor(Color);
		TextComponent->SetHorizontalAlignment(HAlignment);
		TextComponent->SetVerticalAlignment(VAlignment);
		if (NumbersTextMaterial)
		{
			TextComponent->SetMaterial(0, NumbersTextMaterial);
		}
	}*/
}

void APicrossGrid::Cycle2DRotation(const int32 MasterIndexPivot)
{
	if (IsLocked()) return;

	SelectionAxis = (SelectionAxis == ESelectionAxis::All ? ESelectionAxis::Z : SelectionAxis == ESelectionAxis::Z ? ESelectionAxis::Y : SelectionAxis == ESelectionAxis::Y ? ESelectionAxis::X : ESelectionAxis::All);

	LastPivotXYZ = MasterIndexPivot >= 0 ? FArray3D::TranslateTo3D(GridSize, MasterIndexPivot) : LastPivotXYZ;

	switch (SelectionAxis)
	{
		case ESelectionAxis::X:		SetRotationXAxis();	break;
		case ESelectionAxis::Y:		SetRotationYAxis();	break;
		case ESelectionAxis::Z:		SetRotationZAxis();	break;
		case ESelectionAxis::All:	EnableAllBlocks();	break;
	}
}

void APicrossGrid::SetRotationXAxis() const
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (int32 Z = 0; Z < GridSize.Z; ++Z)
	{
		for (int32 Y = 0; Y < GridSize.Y; ++Y)
		{
			int32 MasterIndex = FArray3D::TranslateTo1D(GridSize, LastPivotXYZ.X, Y, Z);
			if (MasterGrid.IsValidIndex(MasterIndex))
			{
				CreateBlockInstance(MasterGrid[MasterIndex]);
			}
		}
	}
}

void APicrossGrid::SetRotationYAxis() const
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (int32 Z = 0; Z < GridSize.Z; ++Z)
	{
		for (int32 X = 0; X < GridSize.X; ++X)
		{
			int32 MasterIndex = FArray3D::TranslateTo1D(GridSize, X, LastPivotXYZ.Y, Z);
			if (MasterGrid.IsValidIndex(MasterIndex))
			{
				CreateBlockInstance(MasterGrid[MasterIndex]);
			}
		}
	}
}

void APicrossGrid::SetRotationZAxis() const
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (int32 Y = 0; Y < GridSize.Y; ++Y)
	{
		for (int32 X = 0; X < GridSize.X; ++X)
		{
			int32 MasterIndex = FArray3D::TranslateTo1D(GridSize, X, Y, LastPivotXYZ.Z);
			if (MasterGrid.IsValidIndex(MasterIndex))
			{
				CreateBlockInstance(MasterGrid[MasterIndex]);
			}
		}
	}
}

void APicrossGrid::EnableAllBlocks() const
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (const FPicrossBlock& Block : MasterGrid)
	{
		CreateBlockInstance(Block);
	}
}

void APicrossGrid::DisableAllBlocks() const
{
	if (IsLocked()) return;

	for (auto& Pair : BlockInstances)
	{
		if (Pair.Value)
		{
			Pair.Value->ClearInstances();
		}
	}
}

bool APicrossGrid::IsLocked() const
{
	return bLocked;
}

void APicrossGrid::Lock()
{
	bLocked = true;
}

void APicrossGrid::Unlock()
{
	bLocked = false;
}

bool APicrossGrid::IsSolved() const
{
	if (CurrentPuzzle && CurrentlyFilledBlocksCount == SolutionFilledBlocksCount)
	{
		const TArray<bool>& Solution = CurrentPuzzle->GetSolution();

		if (MasterGrid.Num() == Solution.Num())
		{
			for (int32 i = 0; i < Solution.Num(); ++i)
			{
				if (MasterGrid[i].State != EBlockState::Filled && Solution[i])
				{
					return false;
				}
			}
		}

		return true;
	}

	return false;
}

void APicrossGrid::TrySolve()
{
	if (IsSolved())
	{
		EnableOnlyFilledBlocks();
		Lock();
	}
}

void APicrossGrid::Move2DSelectionUp()
{
	if (IsLocked()) return;

	switch (SelectionAxis)
	{
		case ESelectionAxis::X:
			LastPivotXYZ.X = LastPivotXYZ.X > 0 ? LastPivotXYZ.X - 1 : GridSize.X - 1;
			SetRotationXAxis();
			break;
		case ESelectionAxis::Y:
			LastPivotXYZ.Y = LastPivotXYZ.Y > 0 ? LastPivotXYZ.Y - 1 : GridSize.Y - 1;
			SetRotationYAxis();
			break;
		case ESelectionAxis::Z:
			LastPivotXYZ.Z = LastPivotXYZ.Z < GridSize.Z - 1 ? LastPivotXYZ.Z + 1 : 0;
			SetRotationZAxis();
			break;
	}
}

void APicrossGrid::Move2DSelectionDown()
{
	if (IsLocked()) return;

	switch (SelectionAxis)
	{
		case ESelectionAxis::X:
			LastPivotXYZ.X = LastPivotXYZ.X < GridSize.X - 1 ? LastPivotXYZ.X + 1 : 0;
			SetRotationXAxis();
			break;
		case ESelectionAxis::Y:
			LastPivotXYZ.Y = LastPivotXYZ.Y < GridSize.Y - 1 ? LastPivotXYZ.Y + 1 : 0;
			SetRotationYAxis();
			break;
		case ESelectionAxis::Z:
			LastPivotXYZ.Z = LastPivotXYZ.Z > 0 ? LastPivotXYZ.Z - 1 : GridSize.Z - 1;
			SetRotationZAxis();
			break;
	}
}

void APicrossGrid::SavePuzzle() const
{
	if (!FArray3D::ValidateDimensions(GridSize)) return;

	TArray<bool> Solution;
	for (const FPicrossBlock& Block : MasterGrid)
	{
		Solution.Add(Block.State == EBlockState::Filled);
	}

	UPicrossPuzzleData* ExistingObject = NewObject<UPicrossPuzzleData>();
	ExistingObject->SetGridSize(GridSize);
	ExistingObject->SetSolution(Solution);

	UPicrossPuzzleFactory* NewFactory = NewObject<UPicrossPuzzleFactory>();
	NewFactory->CreatedObjectAsset = ExistingObject;
	
	FAssetToolsModule& AssetToolsModule = FAssetToolsModule::GetModule();
	UObject* NewAsset = AssetToolsModule.Get().CreateAssetWithDialog(NewFactory->GetSupportedClass(), NewFactory);
	TArray<UObject*> ObjectsToSync;
	ObjectsToSync.Add(NewAsset);
	GEditor->SyncBrowserToObjects(ObjectsToSync);
}

void APicrossGrid::LoadPuzzle(FAssetData PuzzleToLoad)
{
	CurrentPuzzle = Cast<UPicrossPuzzleData>(PuzzleToLoad.GetAsset());
	if (CurrentPuzzle)
	{
		ClosePuzzleBrowser();
		CreateGrid();
	}
}