// Copyright Sanya Larsson 2020

#include "PicrossGrid.h"
#include "PicrossNumber.h"
#include "Algo/Count.h"
#include "Algo/ForEach.h"
#include "Algo/Reverse.h"
#include "AssetDataObject.h"
#include "Blueprint/UserWidget.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/TextRenderActor.h"
#include "Materials/MaterialInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "UI/PuzzleBrowserWidget.h"


// Sets default values
APicrossGrid::APicrossGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	BlockInstances.Add(EBlockState::Clear, CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Clear Blocks")));
	BlockInstances.Add(EBlockState::Crossed, CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Crossed Blocks")));
	BlockInstances.Add(EBlockState::Filled, CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Filled Blocks")));

	for (auto& Pair : BlockInstances)
	{
		if (Pair.Value)
		{
			Pair.Value->NumCustomDataFloats = 1; // This custom data represents the MasterIndex. Stored as float, cast to int32 required when reading.
			Pair.Value->SetupAttachment(GetRootComponent());
		}
	}

	HighlightedBlocks = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Highlight Blocks"));
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
		PuzzleBrowserWidget = Cast<UPuzzleBrowserWidget>(CreateWidget(GetWorld(), PuzzleBrowserWidgetClass, TEXT("Puzzle Browser")));
		if (PuzzleBrowserWidget)
		{
			PuzzleBrowserWidget->AddToViewport();
			PuzzleBrowserWidget->SetPicrossGrid(this);
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

void APicrossGrid::CreateGrid()
{
	if (!Puzzle.IsValid()) return;

	Unlock();
	DestroyGrid();
	UndoStack.Empty();
	RedoStack.Empty();
	SelectionAxis = EAxis::None;
	LastPivotXYZ = FIntVector::ZeroValue;
	SolutionFilledBlocksCount = Algo::Count(Puzzle.GetPuzzleData()->GetSolution(), true);
	CurrentlyFilledBlocksCount = 0;

	const int32 MaxAxis = Puzzle.GetGridSize().GetMax();
	const float TargetSize = 10.f;
	Puzzle.DynamicScale = TargetSize / MaxAxis;

	const FVector DynamicScale = FVector::OneVector * Puzzle.DynamicScale;
	const float DynamicDistanceBetweenBlocks = (DistanceBetweenBlocks * Puzzle.DynamicScale) + (100.f * DynamicScale.GetMax());

	FVector StartPosition = GetActorLocation();
	StartPosition -= GetActorRightVector() * (DynamicDistanceBetweenBlocks * (Puzzle.Y() / 2) - (Puzzle.Y() % 2 == 0 ? DynamicDistanceBetweenBlocks / 2 : 0));
	StartPosition -= GetActorForwardVector() * (DynamicDistanceBetweenBlocks * (Puzzle.X() / 2) - (Puzzle.X() % 2 == 0 ? DynamicDistanceBetweenBlocks / 2 : 0));

	for (int32 Z = 0; Z < Puzzle.Z(); ++Z)
	{
		const float OffsetZ = DynamicDistanceBetweenBlocks * Z;
		for (int32 Y = 0; Y < Puzzle.Y(); ++Y)
		{
			const float OffsetY = DynamicDistanceBetweenBlocks * Y;
			for (int32 X = 0; X < Puzzle.X(); ++X)
			{
				const float OffsetX = DynamicDistanceBetweenBlocks * X;
				FVector BlockPosition = StartPosition;
				BlockPosition += GetActorForwardVector() * OffsetX;
				BlockPosition += GetActorRightVector() * OffsetY;
				BlockPosition += GetActorUpVector() * OffsetZ;

				const int32 MasterIndex = Puzzle.GetIndex(FIntVector(X,Y,Z));
				FPicrossBlock& Block = Puzzle[MasterIndex] = FPicrossBlock{ EBlockState::Clear, FTransform(GetActorRotation(), BlockPosition, DynamicScale), MasterIndex, INDEX_NONE };
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

	for (FPicrossBlock& Block : Puzzle)
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

	const EBlockState PreviousState = Puzzle[StartMasterIndex].State;
	const EBlockState NewState = Action == EBlockState::Filled ? (PreviousState != EBlockState::Filled ? EBlockState::Filled : EBlockState::Clear) : (PreviousState != EBlockState::Crossed ? EBlockState::Crossed : EBlockState::Clear);
	UpdateBlocks(StartMasterIndex, EndMasterIndex, PreviousState, NewState, true);
}

void APicrossGrid::UpdateBlocks(const int32 StartMasterIndex, const int32 EndMasterIndex, const EBlockState PreviousState, const EBlockState NewState, const bool AddToStack)
{
	if (StartMasterIndex == INDEX_NONE || EndMasterIndex == INDEX_NONE) return;

	const FIntVector StartIndex = Puzzle.GetIndex(StartMasterIndex);
	const FIntVector EndIndex = Puzzle.GetIndex(EndMasterIndex);

	for (int32 Z = (StartIndex.Z < EndIndex.Z ? StartIndex.Z : EndIndex.Z); Z <= (StartIndex.Z < EndIndex.Z ? EndIndex.Z : StartIndex.Z); ++Z)
	{
		for (int32 Y = (StartIndex.Y < EndIndex.Y ? StartIndex.Y : EndIndex.Y); Y <= (StartIndex.Y < EndIndex.Y ? EndIndex.Y : StartIndex.Y); ++Y)
		{
			for (int32 X = (StartIndex.X < EndIndex.X ? StartIndex.X : EndIndex.X); X <= (StartIndex.X < EndIndex.X ? EndIndex.X : StartIndex.X); ++X)
			{
				const FIntVector Index = FIntVector(X, Y, Z);
				if (Puzzle[Index].State == PreviousState)
				{
					UpdateBlockState(Puzzle[Index], NewState);
				}
			}
		}
	}

	if (AddToStack)
	{
		UndoStack.Push(FPicrossAction{ StartMasterIndex, EndMasterIndex, PreviousState, NewState });
		RedoStack.Empty();
	}
}

void APicrossGrid::Undo()
{
	if (!IsLocked() && UndoStack.Num() > 0)
	{
		FPicrossAction Action = UndoStack.Pop(false);
		UpdateBlocks(Action.StartBlockIndex, Action.EndBlockIndex, Action.NewState, Action.PreviousState, false);
		RedoStack.Push(Action);
	}
}

void APicrossGrid::Redo()
{
	if (!IsLocked() && RedoStack.Num() > 0)
	{
		FPicrossAction Action = RedoStack.Pop(false);
		UpdateBlocks(Action.StartBlockIndex, Action.EndBlockIndex, Action.PreviousState, Action.NewState, false);
		UndoStack.Push(Action);
	}
}

void APicrossGrid::HighlightBlocks(const int32 MasterIndexPivot)
{
	HighlightedBlocks->ClearInstances();

	if (IsLocked() || MasterIndexPivot == INDEX_NONE) return;

	switch (SelectionAxis)
	{
		case EAxis::X:
			HighlightBlocksInAxis(MasterIndexPivot, EAxis::Y);
			HighlightBlocksInAxis(MasterIndexPivot, EAxis::Z);
			break;
		case EAxis::Y:
			HighlightBlocksInAxis(MasterIndexPivot, EAxis::X);
			HighlightBlocksInAxis(MasterIndexPivot, EAxis::Z);
			break;
		case EAxis::Z:
			HighlightBlocksInAxis(MasterIndexPivot, EAxis::X);
			HighlightBlocksInAxis(MasterIndexPivot, EAxis::Y);
			break;
		case EAxis::None:
			HighlightBlocksInAxis(MasterIndexPivot, EAxis::X);
			HighlightBlocksInAxis(MasterIndexPivot, EAxis::Y);
			HighlightBlocksInAxis(MasterIndexPivot, EAxis::Z);
			break;
		default:
			break;
	}
}

void APicrossGrid::HighlightBlocksInAxis(const int32 MasterIndexPivot, const EAxis::Type AxisToHighlight)
{
	const FIntVector XYZ = Puzzle.GetIndex(MasterIndexPivot);
	const int32 EndIndex = (AxisToHighlight == EAxis::X ? Puzzle.X() : AxisToHighlight == EAxis::Y ? Puzzle.Y() : Puzzle.Z());

	for (int32 AxisIndex = 0; AxisIndex < EndIndex; ++AxisIndex)
	{
		const FIntVector MasterIndex = FIntVector(AxisToHighlight == EAxis::X ? AxisIndex : XYZ.X, AxisToHighlight == EAxis::Y ? AxisIndex : XYZ.Y, AxisToHighlight == EAxis::Z ? AxisIndex : XYZ.Z);
		FTransform HighlightBlockTransform = Puzzle[MasterIndex].Transform;
		HighlightBlockTransform.SetScale3D(HighlightBlockTransform.GetScale3D() * 1.05f);
		HighlightBlockTransform.AddToTranslation((-GetActorUpVector()) * 100.f * ((HighlightBlockTransform.GetScale3D().Z - Puzzle[MasterIndex].Transform.GetScale3D().Z) / 2));
		HighlightedBlocks->AddInstanceWorldSpace(HighlightBlockTransform);
	}
}

void APicrossGrid::EnableOnlyFilledBlocks()
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (FPicrossBlock& Block : Puzzle)
	{
		if (Block.State == EBlockState::Filled)
		{
			CreateBlockInstance(Block);
		}
	}
}

void APicrossGrid::UpdateBlockState(FPicrossBlock& Block, const EBlockState NewState)
{
	if (IsLocked()) return;

	if (Block.State != NewState && Block.InstanceIndex != INDEX_NONE)
	{
		const EBlockState PreviousState = Block.State;
		const int32 PreviousInstanceIndex = Block.InstanceIndex;
		BlockInstances[PreviousState]->RemoveInstance(PreviousInstanceIndex);

		// Side effect of removing a instance in a HISM is that it swaps with another block before removing. That other block then has an outdated InstanceIndex saved, we update that here.
		const int32 PreviousInstanceCustomDataIndex = PreviousInstanceIndex * BlockInstances[PreviousState]->NumCustomDataFloats;
		if (BlockInstances[PreviousState]->PerInstanceSMCustomData.IsValidIndex(PreviousInstanceCustomDataIndex))
		{
			const int32 SwappedBlockMasterIndex = static_cast<int32>(BlockInstances[PreviousState]->PerInstanceSMCustomData[PreviousInstanceCustomDataIndex]);
			Puzzle[SwappedBlockMasterIndex].InstanceIndex = PreviousInstanceIndex;
		}

		Block.State = NewState;
		CreateBlockInstance(Block);

		CurrentlyFilledBlocksCount += PreviousState == EBlockState::Filled ? -1 : NewState == EBlockState::Filled ? 1 : 0;
		TrySolve();
	}
}

void APicrossGrid::CreateBlockInstance(FPicrossBlock& Block) const
{
	const int32 InstanceIndex = BlockInstances[Block.State]->AddInstanceWorldSpace(Block.Transform);
	BlockInstances[Block.State]->SetCustomDataValue(InstanceIndex, 0, static_cast<float>(Block.MasterIndex));
	Block.InstanceIndex = InstanceIndex;
}

void APicrossGrid::GenerateNumbers()
{
	CleanupNumbers();

	if (IsLocked()) return;

	GenerateNumbersForAxis(EAxis::X);
	GenerateNumbersForAxis(EAxis::Y);
	GenerateNumbersForAxis(EAxis::Z);
}

void APicrossGrid::GenerateNumbersForAxis(EAxis::Type Axis)
{
	if (!Puzzle.IsValid()) return;

	const TArray<bool>& Solution = Puzzle.GetPuzzleData()->GetSolution();

	if (Solution.Num() != FArray3D::Size(Puzzle.GetGridSize())) return;

	// Axis1 is Y-axis if we're generating for X-axis, otherwise it's the X-axis.
	int32 Axis1Size = (Axis == EAxis::X ? Puzzle.Y() : Puzzle.X());
	for (int32 Axis1 = 0; Axis1 < Axis1Size; ++Axis1)
	{
		// Axis2 is Y-axis if we're generating for Z-Axis, otherwise it's the Z-axis.
		int32 Axis2Size = (Axis == EAxis::Z ? Puzzle.Y() : Puzzle.Z());
		for (int32 Axis2 = 0; Axis2 < Axis2Size; ++Axis2)
		{
			FFormatOrderedArguments Numbers;
			int32 Sum = 0;

			// Axis3 is the axis we're generating numbers for.
			int32 Axis3Size = (Axis == EAxis::Z ? Puzzle.Z() : Axis == EAxis::X ? Puzzle.X() : Puzzle.Y());
			for (int32 Axis3 = 0; Axis3 < Axis3Size; ++Axis3)
			{
				// De-anonymize Axis1,Axis2,Axis3 into their named version (X,Y,Z)
				FIntVector XYZ = Axis == EAxis::X ? FIntVector(Axis3, Axis1, Axis2) : Axis == EAxis::Y ? FIntVector(Axis1, Axis3, Axis2) : FIntVector(Axis1, Axis2, Axis3);

				// Count the filled blocks, adding the results to the Numbers "array".
				bool bCountBlock = Solution[Puzzle.GetIndex(XYZ)];
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
			
			// Reverse Numbers if Axis is Z since we counted them from opposite side.
			if (Axis == EAxis::Z) Algo::Reverse(Numbers);

			CreatePicrossNumber(Axis, Axis1, Axis2, Numbers);
		}
	}
}

void APicrossGrid::CreatePicrossNumber(const EAxis::Type Axis, int32 Axis1, int32 Axis2, const FFormatOrderedArguments& Numbers)
{
	if (Numbers.Num() > 0)
	{
		if (PicrossNumberClass)
		{
			APicrossNumber* PicrossNumber = GetWorld()->SpawnActor<APicrossNumber>(PicrossNumberClass);
			if (PicrossNumber)
			{
				const FIntVector BlockIndex = (Axis == EAxis::X ? FIntVector(0, Axis1, Axis2) : Axis == EAxis::Y ? FIntVector(Axis1, 0, Axis2) : FIntVector(Axis1, Axis2, Puzzle.Z() - 1));
				const FPicrossBlock& Block = Puzzle[BlockIndex];
				const FVector RelativeLocation = (Axis == EAxis::X ? FVector(-75.f, 0.f, 50.f) : Axis == EAxis::Y ? FVector(0.f, -75.f, 50.f) : FVector(0.f, 0.f, 115.f)) * Puzzle.DynamicScale;
				const FVector WorldLocation = Block.Transform.GetTranslation() + Block.Transform.GetRotation().RotateVector(RelativeLocation);
				PicrossNumber->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
				PicrossNumber->SetActorLocation(WorldLocation);
				PicrossNumber->SetActorRelativeRotation(FRotator::ZeroRotator);
				PicrossNumber->SetActorScale3D(FVector(Puzzle.DynamicScale));
				PicrossNumber->Setup(Axis, Numbers);

				switch (Axis)
				{
					case EAxis::X: NumbersXAxis.Add(BlockIndex, PicrossNumber); break;
					case EAxis::Y: NumbersYAxis.Add(BlockIndex, PicrossNumber); break;
					case EAxis::Z: NumbersZAxis.Add(BlockIndex, PicrossNumber); break;
				}
			}
		}
	}
}

void APicrossGrid::ForEachPicrossNumber(const TFunctionRef<void(TPair<FIntVector, APicrossNumber*>&)> Func)
{
	Algo::ForEach(NumbersXAxis, Func);
	Algo::ForEach(NumbersYAxis, Func);
	Algo::ForEach(NumbersZAxis, Func);
}

void APicrossGrid::CleanupNumbers()
{
	static const auto DestroyTextActors = [](TPair<FIntVector, APicrossNumber*>& Pair) -> void { if (Pair.Value) Pair.Value->Destroy(); };

	ForEachPicrossNumber(DestroyTextActors);
	NumbersXAxis.Empty();
	NumbersYAxis.Empty();
	NumbersZAxis.Empty();
}

void APicrossGrid::UpdateNumbersVisibility()
{
	const EAxis::Type Axis = SelectionAxis;
	const FIntVector Index = LastPivotXYZ;
	const auto ShowOrHide = [Axis, Index](TPair<FIntVector, APicrossNumber*>& Pair) -> void 
	{
		const bool bShowAlways = Axis == EAxis::None;
		const bool bSameAxis = Axis == Pair.Value->GetAxis();
		const bool bCorrectIndex = (Axis == EAxis::X ? Pair.Key.X == Index.X : Axis == EAxis::Y ? Pair.Key.Y == Index.Y : Pair.Key.Z == Index.Z);
		const bool bShouldShow = (bShowAlways || (!bSameAxis && bCorrectIndex));
		if (Pair.Value) Pair.Value->SetActorHiddenInGame(!bShouldShow);
	};
	const auto UpdateRotation = [Axis](TPair<FIntVector, APicrossNumber*>& Pair) -> void { if (Pair.Value) Pair.Value->UpdateRotation(Axis); };

	ForEachPicrossNumber(ShowOrHide);
	ForEachPicrossNumber(UpdateRotation);
}

void APicrossGrid::Cycle2DRotation(const int32 MasterIndexPivot)
{
	if (IsLocked()) return;

	SelectionAxis = (SelectionAxis == EAxis::None ? EAxis::Z : SelectionAxis == EAxis::Z ? EAxis::Y : SelectionAxis == EAxis::Y ? EAxis::X : EAxis::None);

	LastPivotXYZ = MasterIndexPivot >= 0 ? Puzzle.GetIndex(MasterIndexPivot) : LastPivotXYZ;

	switch (SelectionAxis)
	{
		case EAxis::X:		SetRotationXAxis();	break;
		case EAxis::Y:		SetRotationYAxis();	break;
		case EAxis::Z:		SetRotationZAxis();	break;
		case EAxis::None:	EnableAllBlocks();	break;
	}

	UpdateNumbersVisibility();
}

void APicrossGrid::SetRotationXAxis()
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (int32 Z = 0; Z < Puzzle.Z(); ++Z)
	{
		for (int32 Y = 0; Y < Puzzle.Y(); ++Y)
		{
			CreateBlockInstance(Puzzle[FIntVector(LastPivotXYZ.X, Y, Z)]);
		}
	}
}

void APicrossGrid::SetRotationYAxis()
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (int32 Z = 0; Z < Puzzle.Z(); ++Z)
	{
		for (int32 X = 0; X < Puzzle.X(); ++X)
		{
			CreateBlockInstance(Puzzle[FIntVector(X, LastPivotXYZ.Y, Z)]);
		}
	}
}

void APicrossGrid::SetRotationZAxis()
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (int32 Y = 0; Y < Puzzle.Y(); ++Y)
	{
		for (int32 X = 0; X < Puzzle.X(); ++X)
		{
			CreateBlockInstance(Puzzle[FIntVector(X, Y, LastPivotXYZ.Z)]);
		}
	}
}

void APicrossGrid::EnableAllBlocks()
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (FPicrossBlock& Block : Puzzle)
	{
		CreateBlockInstance(Block);
	}
}

void APicrossGrid::DisableAllBlocks()
{
	if (IsLocked()) return;

	for (auto& Pair : BlockInstances)
	{
		if (Pair.Value)
		{
			Pair.Value->ClearInstances();
		}
	}
	for (FPicrossBlock& Block : Puzzle)
	{
		Block.InstanceIndex = INDEX_NONE;
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
	if (Puzzle.IsValid() && CurrentlyFilledBlocksCount == SolutionFilledBlocksCount)
	{
		const TArray<bool>& Solution = Puzzle.GetPuzzleData()->GetSolution();

		for (int32 i = 0; i < Solution.Num(); ++i)
		{
			if (Puzzle[i].State != EBlockState::Filled && Solution[i])
			{
				return false;
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
		HighlightBlocks(INDEX_NONE);
		Lock();
		GenerateNumbers();
	}
}

void APicrossGrid::Move2DSelectionUp()
{
	if (IsLocked()) return;

	switch (SelectionAxis)
	{
		case EAxis::X:
			LastPivotXYZ.X = LastPivotXYZ.X > 0 ? LastPivotXYZ.X - 1 : Puzzle.X() - 1;
			SetRotationXAxis();
			break;
		case EAxis::Y:
			LastPivotXYZ.Y = LastPivotXYZ.Y > 0 ? LastPivotXYZ.Y - 1 : Puzzle.Y() - 1;
			SetRotationYAxis();
			break;
		case EAxis::Z:
			LastPivotXYZ.Z = LastPivotXYZ.Z < Puzzle.Z() - 1 ? LastPivotXYZ.Z + 1 : 0;
			SetRotationZAxis();
			break;
	}

	UpdateNumbersVisibility();
}

void APicrossGrid::Move2DSelectionDown()
{
	if (IsLocked()) return;

	switch (SelectionAxis)
	{
		case EAxis::X:
			LastPivotXYZ.X = LastPivotXYZ.X < Puzzle.X() - 1 ? LastPivotXYZ.X + 1 : 0;
			SetRotationXAxis();
			break;
		case EAxis::Y:
			LastPivotXYZ.Y = LastPivotXYZ.Y < Puzzle.Y() - 1 ? LastPivotXYZ.Y + 1 : 0;
			SetRotationYAxis();
			break;
		case EAxis::Z:
			LastPivotXYZ.Z = LastPivotXYZ.Z > 0 ? LastPivotXYZ.Z - 1 : Puzzle.Z() - 1;
			SetRotationZAxis();
			break;
	}

	UpdateNumbersVisibility();
}

TOptional<FTransform> APicrossGrid::GetIdealPawnTransform(const APawn* Pawn) const
{
	static const TMap<EAxis::Type, FRotator> Rotations{
		{ EAxis::X, FRotator(0.f, 0.f, 0.f) },
		{ EAxis::Y, FRotator(0.f, -90.f, 0.f) },
		{ EAxis::Z, FRotator(-89.99f, -90.f, 0.f) }
	};

	if (Rotations.Contains(SelectionAxis) && Pawn)
	{
		const FVector Direction = SelectionAxis == EAxis::X ? -GetActorForwardVector() : SelectionAxis == EAxis::Y ? GetActorRightVector() : GetActorUpVector();
		const float Distance = 1150.f + 300 * Puzzle.DynamicScale;
		FVector Origin, BoxExtent;
		GetActorBounds(false, Origin, BoxExtent, true);
		const FVector Pivot = Puzzle[LastPivotXYZ].Transform.GetLocation();
		Origin = FVector{ 
			SelectionAxis == EAxis::X ? Pivot.X : Origin.X,
			SelectionAxis == EAxis::Y ? Pivot.Y : Origin.Y,
			SelectionAxis == EAxis::Z ? Pivot.Z : Origin.Z
		};
		const FVector Location = Origin + Direction * Distance;
		return FTransform(GetTransform().TransformRotation(Rotations.FindChecked(SelectionAxis).Quaternion()), Location);
	}
	
	return {};
}

void APicrossGrid::LoadPuzzle(FAssetData PuzzleToLoad)
{
	Puzzle = FPicrossPuzzle(Cast<UPicrossPuzzleData>(PuzzleToLoad.GetAsset()));
	if (Puzzle.IsValid())
	{
		ClosePuzzleBrowser();
		CreateGrid();
	}
}