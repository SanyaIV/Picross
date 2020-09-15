// Copyright Sanya Larsson 2020

#include "PicrossGrid.h"
#include "Algo/Count.h"
#include "Algo/ForEach.h"
#include "Algo/Reverse.h"
#include "AssetDataObject.h"
#include "AssetToolsModule.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/TextRenderActor.h"
#include "IAssetTools.h"
#include "UnrealEd.h"
#include "PicrossPuzzleData.h"
#include "PicrossPuzzleFactory.h"

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

	HighlightedBlocks = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Highlight Blocks"));
	if (HighlightedBlocks)
	{
		HighlightedBlocks->SetupAttachment(GetRootComponent());
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

	if (HighlightedBlocks && HighlightMesh && HighlightMaterial)
	{
		HighlightedBlocks->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HighlightedBlocks->SetStaticMesh(HighlightMesh);
		HighlightedBlocks->SetMaterial(0, HighlightMaterial);
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
	DestroyGrid();
	SelectionAxis = ESelectionAxis::All;
	LastPivotXYZ = FIntVector::ZeroValue;
	SolutionFilledBlocksCount = CurrentPuzzle ? Algo::Count(CurrentPuzzle->GetSolution(), true) : -1;
	CurrentlyFilledBlocksCount = 0;
	MasterGrid = FPicrossBlockGrid(CurrentPuzzle ? CurrentPuzzle->GetGridSize() : DefaultGridSize);
	verify(FArray3D::ValidateDimensions(MasterGrid.GetGridSize()));

	FVector StartPosition = GetActorLocation();
	StartPosition -= GetActorRightVector() * (DistanceBetweenBlocks * (MasterGrid.Y() / 2) - (MasterGrid.Y() % 2 == 0 ? DistanceBetweenBlocks / 2 : 0));
	StartPosition -= GetActorForwardVector() * (DistanceBetweenBlocks * (MasterGrid.X() / 2) - (MasterGrid.X() % 2 == 0 ? DistanceBetweenBlocks / 2 : 0));

	for (int32 Z = 0; Z < MasterGrid.Z(); ++Z)
	{
		const float OffsetZ = DistanceBetweenBlocks * Z;
		for (int32 Y = 0; Y < MasterGrid.Y(); ++Y)
		{
			const float OffsetY = DistanceBetweenBlocks * Y;
			for (int32 X = 0; X < MasterGrid.X(); ++X)
			{
				const float OffsetX = DistanceBetweenBlocks * X;
				FVector BlockPosition = StartPosition;
				BlockPosition += GetActorForwardVector() * OffsetX;
				BlockPosition += GetActorRightVector() * OffsetY;
				BlockPosition += GetActorUpVector() * OffsetZ;

				const int32 MasterIndex = FArray3D::TranslateTo1D(MasterGrid.GetGridSize(), X, Y, Z);
				const FPicrossBlock& Block = MasterGrid[MasterIndex] = FPicrossBlock{ EBlockState::Clear, FTransform(GetActorRotation(), BlockPosition, FVector::OneVector), MasterIndex };
				CreateBlockInstance(Block);
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
}

void APicrossGrid::UpdateBlocks(const int32 StartMasterIndex, const int32 EndMasterIndex, const EBlockState Action)
{
	if (StartMasterIndex == INDEX_NONE || EndMasterIndex == INDEX_NONE) return;

	const EBlockState PreviousState = MasterGrid[StartMasterIndex].State;
	const EBlockState NewState = Action == EBlockState::Filled ? (PreviousState != EBlockState::Filled ? EBlockState::Filled : EBlockState::Clear) : (PreviousState != EBlockState::Crossed ? EBlockState::Crossed : EBlockState::Clear);
	const FIntVector StartIndex = MasterGrid.GetIndex(StartMasterIndex);
	const FIntVector EndIndex = MasterGrid.GetIndex(EndMasterIndex);

	for (int32 Z = (StartIndex.Z < EndIndex.Z ? StartIndex.Z : EndIndex.Z); Z <= (StartIndex.Z < EndIndex.Z ? EndIndex.Z : StartIndex.Z); ++Z)
	{
		for (int32 Y = (StartIndex.Y < EndIndex.Y ? StartIndex.Y : EndIndex.Y); Y <= (StartIndex.Y < EndIndex.Y ? EndIndex.Y : StartIndex.Y); ++Y)
		{
			for (int32 X = (StartIndex.X < EndIndex.X ? StartIndex.X : EndIndex.X); X <= (StartIndex.X < EndIndex.X ? EndIndex.X : StartIndex.X); ++X)
			{
				const FIntVector Index = FIntVector(X, Y, Z);
				if (MasterGrid[Index].State == PreviousState)
				{
					UpdateBlockState(MasterGrid[Index], NewState, BlockInstances[PreviousState]->PerInstanceSMCustomData.IndexOfByKey(static_cast<float>(MasterGrid.GetIndex(Index))));
				}
			}
		}
	}
}

void APicrossGrid::HighlightBlocks(const int32 MasterIndexPivot)
{
	HighlightedBlocks->ClearInstances();

	if (IsLocked() || MasterIndexPivot == INDEX_NONE) return;

	switch (SelectionAxis)
	{
		case ESelectionAxis::X:
			HighlightBlocksInAxis(MasterIndexPivot, ESelectionAxis::Y);
			HighlightBlocksInAxis(MasterIndexPivot, ESelectionAxis::Z);
			break;
		case ESelectionAxis::Y:
			HighlightBlocksInAxis(MasterIndexPivot, ESelectionAxis::X);
			HighlightBlocksInAxis(MasterIndexPivot, ESelectionAxis::Z);
			break;
		case ESelectionAxis::Z:
			HighlightBlocksInAxis(MasterIndexPivot, ESelectionAxis::X);
			HighlightBlocksInAxis(MasterIndexPivot, ESelectionAxis::Y);
			break;
		case ESelectionAxis::All:
			HighlightBlocksInAxis(MasterIndexPivot, ESelectionAxis::X);
			HighlightBlocksInAxis(MasterIndexPivot, ESelectionAxis::Y);
			HighlightBlocksInAxis(MasterIndexPivot, ESelectionAxis::Z);
			break;
		default:
			break;
	}
}

void APicrossGrid::HighlightBlocksInAxis(const int32 MasterIndexPivot, const ESelectionAxis AxisToHighlight)
{
	const FIntVector XYZ = MasterGrid.GetIndex(MasterIndexPivot);
	const int32 EndIndex = (AxisToHighlight == ESelectionAxis::X ? MasterGrid.X() : AxisToHighlight == ESelectionAxis::Y ? MasterGrid.Y() : MasterGrid.Z());

	for (int32 AxisIndex = 0; AxisIndex < EndIndex; ++AxisIndex)
	{
		const FIntVector MasterIndex = FIntVector(AxisToHighlight == ESelectionAxis::X ? AxisIndex : XYZ.X, AxisToHighlight == ESelectionAxis::Y ? AxisIndex : XYZ.Y, AxisToHighlight == ESelectionAxis::Z ? AxisIndex : XYZ.Z);
		FTransform HighlightBlockTransform = MasterGrid[MasterIndex].Transform;
		HighlightBlockTransform.SetScale3D(HighlightBlockTransform.GetScale3D() * 1.05f);
		HighlightBlockTransform.AddToTranslation((-GetActorUpVector()) * 100.f * ((HighlightBlockTransform.GetScale3D().Z - MasterGrid[MasterIndex].Transform.GetScale3D().Z) / 2));
		HighlightedBlocks->AddInstanceWorldSpace(HighlightBlockTransform);
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

	if (Block.State != NewState && PreviousInstanceIndex != INDEX_NONE)
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

void APicrossGrid::GenerateNumbers()
{
	Algo::ForEachIf(TextRenderActors, IsValid, PROJECTION_MEMBER(AActor, Destroy));
	TextRenderActors.Empty();

	if (IsLocked()) return;

	GenerateNumbersForAxis(ESelectionAxis::X);
	GenerateNumbersForAxis(ESelectionAxis::Y);
	GenerateNumbersForAxis(ESelectionAxis::Z);
}

void APicrossGrid::GenerateNumbersForAxis(ESelectionAxis Axis)
{
	if (!CurrentPuzzle) return;

	const TArray<bool>& Solution = CurrentPuzzle->GetSolution();

	// Axis1 is Y-axis if we're generating for X-axis, otherwise it's the X-axis.
	int32 Axis1Size = (Axis == ESelectionAxis::X ? MasterGrid.Y() : MasterGrid.X());
	for (int32 Axis1 = 0; Axis1 < Axis1Size; ++Axis1)
	{
		// Axis2 is Y-axis if we're generating for Z-Axis, otherwise it's the Z-axis.
		int32 Axis2Size = (Axis == ESelectionAxis::Z ? MasterGrid.Y() : MasterGrid.Z());
		for (int32 Axis2 = 0; Axis2 < Axis2Size; ++Axis2)
		{
			FFormatOrderedArguments Numbers;
			int32 Sum = 0;

			// Axis3 is the axis we're generating numbers for.
			int32 Axis3Size = (Axis == ESelectionAxis::Z ? MasterGrid.Z() : Axis == ESelectionAxis::X ? MasterGrid.X() : MasterGrid.Y());
			for (int32 Axis3 = 0; Axis3 < Axis3Size; ++Axis3)
			{
				// De-anonymize Axis1,Axis2,Axis3 into their named version (X,Y,Z)
				FIntVector XYZ = Axis == ESelectionAxis::X ? FIntVector(Axis3, Axis1, Axis2) : Axis == ESelectionAxis::Y ? FIntVector(Axis1, Axis3, Axis2) : FIntVector(Axis1, Axis2, Axis3);

				// Count the filled blocks, adding the results to the Numbers "array".
				bool bCountBlock = Solution[MasterGrid.GetIndex(XYZ)];
				if (bCountBlock)
				{
					++Sum;

					if (Sum > 0 && Axis3 == Axis3Size - 1)
					{
						Numbers.Add(Sum);
					}
				}
				else if (Sum > 0)
				{
					Numbers.Add(Sum);
					Sum = 0;
				}
			}
			
			if (Numbers.Num() > 0)
			{
				// If Z-axis we reverse the numbers since we counted them from the bottom instead of the top.
				if (Axis == ESelectionAxis::Z)
				{
					Algo::Reverse(Numbers);
				}

				// Generate text from array of numbers with either a comma delimiter or new-line if Z-axis
				const FText Delimiter = FText::FromString(Axis == ESelectionAxis::Z ? TEXT("\n") : TEXT(", "));
				const FText Text1 = FText::Join(Delimiter, Numbers);

				// Select color based on axis.
				const FColor Color = Axis == ESelectionAxis::Z ? FColor::Blue : Axis == ESelectionAxis::Y ? FColor::Green : FColor::Red;

				// Select text alignment based on axis.
				const EHorizTextAligment HAlignment1 = Axis == ESelectionAxis::Z ? EHorizTextAligment::EHTA_Center : EHorizTextAligment::EHTA_Right;
				const EVerticalTextAligment VAlignment = Axis == ESelectionAxis::Z ? EVerticalTextAligment::EVRTA_TextBottom : EVerticalTextAligment::EVRTA_TextCenter;

				// If the axis isn't Z we reverse the numbers. We don't need it for Z since the order is the same regardless of direction.
				if (Axis != ESelectionAxis::Z)
				{
					Algo::Reverse(Numbers);
				}

				// Generate text from array of numbers unless Z-axis in which case it's the same as the first Text.
				const FText Text2 = Axis == ESelectionAxis::Z ? Text1 : FText::Join(Delimiter, Numbers);

				// Select text alignment based on axis.
				const EHorizTextAligment HAlignment2 = Axis == ESelectionAxis::Z ? EHorizTextAligment::EHTA_Center : EHorizTextAligment::EHTA_Left;

				// Get the index for the block that we're going to place the text by.
				const FIntVector XYZ = Axis == ESelectionAxis::X ? FIntVector(0, Axis1, Axis2) : Axis == ESelectionAxis::Y ? FIntVector(Axis1, 0, Axis2) : FIntVector(Axis1, Axis2, MasterGrid.Z() - 1);
				const FPicrossBlock& Block = MasterGrid[XYZ];

				// Relative Location and Rotation that we're going to add onto the Block Transform to get the correct final transform.
				const FVector RelativeLocation1 = Axis == ESelectionAxis::X ? FVector(-75.f, 0.f, 50.f) : Axis == ESelectionAxis::Y ? FVector(0.f, -75.f, 50.f) : FVector(0.f, 0.f, 115.f);
				const FRotator RelativeRotation1 = Axis == ESelectionAxis::X ? FRotator(0.f, 90.f, 0.f) : Axis == ESelectionAxis::Y ? FRotator(0.f, 180.f, 0.f) : FRotator(0.f, 90.f, 0.f);
				const FRotator RelativeRotation2 = RelativeRotation1 + FRotator(0.f, 180.f, 0.f);

				// World location
				const FVector WorldLocation = Block.Transform.GetTranslation() + Block.Transform.GetRotation().RotateVector(RelativeLocation1);

				CreateTextRenderActor(WorldLocation, RelativeRotation1, Text1, Color, HAlignment1, VAlignment);
				CreateTextRenderActor(WorldLocation, RelativeRotation2, Text2, Color, HAlignment2, VAlignment);
			}
		}
	}
}

void APicrossGrid::CreateTextRenderActor(FVector WorldLocation, FRotator RelativeRotation, FText Text, FColor Color, EHorizTextAligment HAlignment, EVerticalTextAligment VAlignment)
{
	ATextRenderActor* TextActor = GetWorld()->SpawnActorAbsolute<ATextRenderActor>(WorldLocation, RelativeRotation);
	static const FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false);
	TextActor->AttachToActor(this, AttachmentRules);
	UTextRenderComponent* TextComponent = TextActor->GetTextRender();
	if (TextComponent)
	{
		TextComponent->SetText(Text);
		TextComponent->SetTextRenderColor(Color);
		TextComponent->SetHorizontalAlignment(HAlignment);
		TextComponent->SetVerticalAlignment(VAlignment);
		if (NumbersTextMaterial)
		{
			TextComponent->SetMaterial(0, NumbersTextMaterial);
		}
	}
	TextRenderActors.Add(TextActor);
}

void APicrossGrid::Cycle2DRotation(const int32 MasterIndexPivot)
{
	if (IsLocked()) return;

	SelectionAxis = (SelectionAxis == ESelectionAxis::All ? ESelectionAxis::Z : SelectionAxis == ESelectionAxis::Z ? ESelectionAxis::Y : SelectionAxis == ESelectionAxis::Y ? ESelectionAxis::X : ESelectionAxis::All);

	LastPivotXYZ = MasterIndexPivot >= 0 ? MasterGrid.GetIndex(MasterIndexPivot) : LastPivotXYZ;

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

	for (int32 Z = 0; Z < MasterGrid.Z(); ++Z)
	{
		for (int32 Y = 0; Y < MasterGrid.Y(); ++Y)
		{
			CreateBlockInstance(MasterGrid[FIntVector(LastPivotXYZ.X, Y, Z)]);
		}
	}
}

void APicrossGrid::SetRotationYAxis() const
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (int32 Z = 0; Z < MasterGrid.Z(); ++Z)
	{
		for (int32 X = 0; X < MasterGrid.X(); ++X)
		{
			CreateBlockInstance(MasterGrid[FIntVector(X, LastPivotXYZ.Y, Z)]);
		}
	}
}

void APicrossGrid::SetRotationZAxis() const
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (int32 Y = 0; Y < MasterGrid.Y(); ++Y)
	{
		for (int32 X = 0; X < MasterGrid.X(); ++X)
		{
			CreateBlockInstance(MasterGrid[FIntVector(X, Y, LastPivotXYZ.Z)]);
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

		if (MasterGrid.GetGrid().Num() == Solution.Num())
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
		GenerateNumbers();
	}
}

void APicrossGrid::Solve()
{
	const TArray<bool>& Solution = CurrentPuzzle->GetSolution();
	for (int32 i = 0; i < Solution.Num(); ++i)
	{
		MasterGrid[i].State = Solution[i] ? EBlockState::Filled : EBlockState::Clear;
	}

	CurrentlyFilledBlocksCount = SolutionFilledBlocksCount;
	TrySolve();
}

void APicrossGrid::Move2DSelectionUp()
{
	if (IsLocked()) return;

	switch (SelectionAxis)
	{
		case ESelectionAxis::X:
			LastPivotXYZ.X = LastPivotXYZ.X > 0 ? LastPivotXYZ.X - 1 : MasterGrid.X() - 1;
			SetRotationXAxis();
			break;
		case ESelectionAxis::Y:
			LastPivotXYZ.Y = LastPivotXYZ.Y > 0 ? LastPivotXYZ.Y - 1 : MasterGrid.Y() - 1;
			SetRotationYAxis();
			break;
		case ESelectionAxis::Z:
			LastPivotXYZ.Z = LastPivotXYZ.Z < MasterGrid.Z() - 1 ? LastPivotXYZ.Z + 1 : 0;
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
			LastPivotXYZ.X = LastPivotXYZ.X < MasterGrid.X() - 1 ? LastPivotXYZ.X + 1 : 0;
			SetRotationXAxis();
			break;
		case ESelectionAxis::Y:
			LastPivotXYZ.Y = LastPivotXYZ.Y < MasterGrid.Y() - 1 ? LastPivotXYZ.Y + 1 : 0;
			SetRotationYAxis();
			break;
		case ESelectionAxis::Z:
			LastPivotXYZ.Z = LastPivotXYZ.Z > 0 ? LastPivotXYZ.Z - 1 : MasterGrid.Z() - 1;
			SetRotationZAxis();
			break;
	}
}

void APicrossGrid::SavePuzzle() const
{
	if (!FArray3D::ValidateDimensions(MasterGrid.GetGridSize())) return;

	TArray<bool> Solution;
	for (const FPicrossBlock& Block : MasterGrid)
	{
		Solution.Add(Block.State == EBlockState::Filled);
	}

	UPicrossPuzzleData* ExistingObject = NewObject<UPicrossPuzzleData>();
	ExistingObject->SetGridSize(MasterGrid.GetGridSize());
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