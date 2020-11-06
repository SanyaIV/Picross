// Copyright Sanya Larsson 2020

#include "PicrossGrid.h"
#include "PicrossNumber.h"
#include "PicrossPuzzleSaveGame.h"
#include "Algo/Count.h"
#include "Algo/ForEach.h"
#include "Algo/Reverse.h"
#include "AssetDataObject.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/TextRenderActor.h"
#include "Materials/MaterialInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"


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
}

void APicrossGrid::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SaveGame();
}

void APicrossGrid::CreateGrid()
{
	if (!Puzzle.IsValid()) return;

	Unlock();
	DestroyGrid();
	UndoStack.Empty();
	RedoStack.Empty();
	SelectionAxis = EAxis::None;
	FocusedBlock = FIntVector::ZeroValue;
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
	HighlightBlocks();
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
	UpdateBlocks(StartMasterIndex, EndMasterIndex, PreviousState, NewState);
}

void APicrossGrid::UpdateBlocks(const int32 StartMasterIndex, const int32 EndMasterIndex, const EBlockState PreviousState, const EBlockState NewState)
{
	if (StartMasterIndex == INDEX_NONE || EndMasterIndex == INDEX_NONE) return;

	FPicrossAction Action;
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
					Action.Actions.Add(FPicrossBlockAction{ Index, PreviousState, NewState });
				}
			}
		}
	}
	
	UndoStack.Push(MoveTemp(Action));
	RedoStack.Empty();
}

void APicrossGrid::Undo()
{
	if (!IsLocked() && UndoStack.Num() > 0)
	{
		for (const FPicrossBlockAction& Action : UndoStack.Top().Actions)
		{
			UpdateBlockState(Puzzle[Action.BlockIndex], Action.PreviousState);
		}
		RedoStack.Push(UndoStack.Pop());
	}
}

void APicrossGrid::Redo()
{
	if (!IsLocked() && RedoStack.Num() > 0)
	{
		for (const FPicrossBlockAction& Action : RedoStack.Top().Actions)
		{
			UpdateBlockState(Puzzle[Action.BlockIndex], Action.NewState);
		}
		UndoStack.Push(RedoStack.Pop());
	}
}

void APicrossGrid::HighlightBlocks()
{
	HighlightedBlocks->ClearInstances();

	if (IsLocked()) return;

	switch (SelectionAxis)
	{
		case EAxis::X:
			HighlightBlocksInAxis(EAxis::Y);
			HighlightBlocksInAxis(EAxis::Z);
			break;
		case EAxis::Y:
			HighlightBlocksInAxis(EAxis::X);
			HighlightBlocksInAxis(EAxis::Z);
			break;
		case EAxis::Z:
			HighlightBlocksInAxis(EAxis::X);
			HighlightBlocksInAxis(EAxis::Y);
			break;
		case EAxis::None:
			HighlightBlocksInAxis(EAxis::X);
			HighlightBlocksInAxis(EAxis::Y);
			HighlightBlocksInAxis(EAxis::Z);
			break;
		default:
			break;
	}
}

void APicrossGrid::HighlightBlocksInAxis(const EAxis::Type AxisToHighlight)
{
	const FIntVector XYZ = FocusedBlock;
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
	const FIntVector Index = FocusedBlock;
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

void APicrossGrid::Cycle2DRotation()
{
	if (IsLocked()) return;

	SelectionAxis = (SelectionAxis == EAxis::None ? EAxis::Z : SelectionAxis == EAxis::Z ? EAxis::Y : SelectionAxis == EAxis::Y ? EAxis::X : EAxis::None);

	switch (SelectionAxis)
	{
		case EAxis::X:		SetRotationXAxis();	break;
		case EAxis::Y:		SetRotationYAxis();	break;
		case EAxis::Z:		SetRotationZAxis();	break;
		case EAxis::None:	EnableAllBlocks();	break;
	}

	UpdateNumbersVisibility();
	HighlightBlocks();
}

void APicrossGrid::SetRotationXAxis()
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (int32 Z = 0; Z < Puzzle.Z(); ++Z)
	{
		for (int32 Y = 0; Y < Puzzle.Y(); ++Y)
		{
			CreateBlockInstance(Puzzle[FIntVector(FocusedBlock.X, Y, Z)]);
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
			CreateBlockInstance(Puzzle[FIntVector(X, FocusedBlock.Y, Z)]);
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
			CreateBlockInstance(Puzzle[FIntVector(X, Y, FocusedBlock.Z)]);
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
		SelectionAxis = EAxis::None;
		EnableOnlyFilledBlocks();
		Lock();
		HighlightBlocks();
		GenerateNumbers();
		DeleteSaveGame();
		SolvedEvent.Broadcast();
	}
}

void APicrossGrid::SaveGame() const
{
	if (Puzzle.IsValid() && !IsSolved())
	{
		if (UPicrossPuzzleSaveGame* SaveGameInstance = Cast<UPicrossPuzzleSaveGame>(UGameplayStatics::CreateSaveGameObject(UPicrossPuzzleSaveGame::StaticClass())))
		{
			for (FPicrossBlock Block : Puzzle)
			{
				SaveGameInstance->PicrossBlockStates.Add(Block.State);
			}

			const FString SaveSlotName = Puzzle.GetPuzzleData()->GetFName().ToString();
			static const int32 UserIndex = 0;
			UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlotName, UserIndex);
		}
	}
}

void APicrossGrid::LoadGame()
{
	if (Puzzle.IsValid())
	{
		const FString SaveSlotName = Puzzle.GetPuzzleData()->GetFName().ToString();
		static const int32 UserIndex = 0;
		const bool bSaveFileExists = UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex);
		if (bSaveFileExists)
		{
			if (UPicrossPuzzleSaveGame* LoadedGame = Cast<UPicrossPuzzleSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex)))
			{
				const bool bSameSize = Puzzle.GetGrid().Num() == LoadedGame->PicrossBlockStates.Num();
				if (bSameSize)
				{
					for (int32 Index = 0; Index < Puzzle.GetGrid().Num(); ++Index)
					{
						Puzzle[Index].State = LoadedGame->PicrossBlockStates[Index];
					}

					CurrentlyFilledBlocksCount = Algo::CountIf(Puzzle.GetGrid(), [](FPicrossBlock Block) { return Block.State == EBlockState::Filled; });
					EnableAllBlocks();
				}
			}
		}
	}
}

void APicrossGrid::DeleteSaveGame() const
{
	if (Puzzle.IsValid())
	{
		const FString SaveSlotName = Puzzle.GetPuzzleData()->GetFName().ToString();
		static const int32 UserIndex = 0;
		if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
		{
			UGameplayStatics::DeleteGameInSlot(SaveSlotName, UserIndex);
		}
	}
}

void APicrossGrid::Move2DSelectionUp()
{
	if (IsLocked()) return;

	switch (SelectionAxis)
	{
		case EAxis::X:
			FocusedBlock.X = FocusedBlock.X > 0 ? FocusedBlock.X - 1 : Puzzle.X() - 1;
			SetRotationXAxis();
			break;
		case EAxis::Y:
			FocusedBlock.Y = FocusedBlock.Y > 0 ? FocusedBlock.Y - 1 : Puzzle.Y() - 1;
			SetRotationYAxis();
			break;
		case EAxis::Z:
			FocusedBlock.Z = FocusedBlock.Z < Puzzle.Z() - 1 ? FocusedBlock.Z + 1 : 0;
			SetRotationZAxis();
			break;
	}

	UpdateNumbersVisibility();
	HighlightBlocks();
}

void APicrossGrid::Move2DSelectionDown()
{
	if (IsLocked()) return;

	switch (SelectionAxis)
	{
		case EAxis::X:
			FocusedBlock.X = FocusedBlock.X < Puzzle.X() - 1 ? FocusedBlock.X + 1 : 0;
			SetRotationXAxis();
			break;
		case EAxis::Y:
			FocusedBlock.Y = FocusedBlock.Y < Puzzle.Y() - 1 ? FocusedBlock.Y + 1 : 0;
			SetRotationYAxis();
			break;
		case EAxis::Z:
			FocusedBlock.Z = FocusedBlock.Z > 0 ? FocusedBlock.Z - 1 : Puzzle.Z() - 1;
			SetRotationZAxis();
			break;
	}

	UpdateNumbersVisibility();
	HighlightBlocks();
}

void APicrossGrid::SetFocusedBlock(const int32 MasterIndex)
{
	if (MasterIndex != INDEX_NONE && FocusedBlock != Puzzle.GetIndex(MasterIndex) && Puzzle.GetGrid().IsValidIndex(MasterIndex))
	{
		FocusedBlock = Puzzle.GetIndex(MasterIndex);
		HighlightBlocks();
	}
}

void APicrossGrid::MoveFocusUp()
{
	switch (SelectionAxis)
	{
		case EAxis::X:
			// Falls through
		case EAxis::Y:
			FocusedBlock.Z = FocusedBlock.Z < Puzzle.Z() - 1 ? FocusedBlock.Z + 1 : 0;
			break;
		case EAxis::Z:
			FocusedBlock.Y = FocusedBlock.Y > 0 ? FocusedBlock.Y - 1 : Puzzle.Y() - 1;
			break;
		default:
			break;
	}

	HighlightBlocks();
}

void APicrossGrid::MoveFocusDown()
{
	switch (SelectionAxis)
	{
		case EAxis::X:
			// Falls through
		case EAxis::Y:
			FocusedBlock.Z = FocusedBlock.Z > 0 ? FocusedBlock.Z - 1 : Puzzle.Z() - 1;
			break;
		case EAxis::Z:
			FocusedBlock.Y = FocusedBlock.Y < Puzzle.Y() -1 ? FocusedBlock.Y + 1 : 0;
			break;
		default:
			break;
	}

	HighlightBlocks();
}

void APicrossGrid::MoveFocusLeft()
{
	switch (SelectionAxis)
	{
		case EAxis::X:
			FocusedBlock.Y = FocusedBlock.Y > 0 ? FocusedBlock.Y - 1 : Puzzle.Y() - 1;
			break;
		case EAxis::Y:
			// Falls through
		case EAxis::Z:
			FocusedBlock.X = FocusedBlock.X > 0 ? FocusedBlock.X - 1 : Puzzle.X() - 1;
			break;
		default:
			break;
	}

	HighlightBlocks();
}

void APicrossGrid::MoveFocusRight()
{
	switch (SelectionAxis)
	{
		case EAxis::X:
			FocusedBlock.Y = FocusedBlock.Y < Puzzle.Y() - 1 ? FocusedBlock.Y + 1 : 0;
			break;
		case EAxis::Y:
			// Falls through
		case EAxis::Z:
			FocusedBlock.X = FocusedBlock.X < Puzzle.X() - 1 ? FocusedBlock.X + 1 : 0;
			break;
		default:
			break;
	}

	HighlightBlocks();
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
		const FVector Origin = [this] {
			FVector Origin, BoxExtent;
			GetActorBounds(true, Origin, BoxExtent, true);
			return Origin;
		}();
		const FVector Pivot = Puzzle[FocusedBlock].Transform.GetLocation();
		const FVector PivotedOrigin = FVector{
			SelectionAxis == EAxis::X ? Pivot.X : Origin.X,
			SelectionAxis == EAxis::Y ? Pivot.Y : Origin.Y,
			SelectionAxis == EAxis::Z ? Pivot.Z : Origin.Z
		};

		const FVector Direction = SelectionAxis == EAxis::X ? -GetActorForwardVector() : SelectionAxis == EAxis::Y ? GetActorRightVector() : GetActorUpVector();
		const float Distance = 1150.f + 300 * Puzzle.DynamicScale;
		const FVector Location = PivotedOrigin + Direction * Distance;
		return FTransform(GetTransform().TransformRotation(Rotations.FindChecked(SelectionAxis).Quaternion()), Location);
	}
	
	return {};
}

void APicrossGrid::LoadPuzzle(FAssetData PuzzleToLoad)
{
	SaveGame();

	Puzzle = FPicrossPuzzle(Cast<UPicrossPuzzleData>(PuzzleToLoad.GetAsset()));
	if (Puzzle.IsValid())
	{
		CreateGrid();
		LoadGame();
		PuzzleLoaded.Broadcast();
	}
}