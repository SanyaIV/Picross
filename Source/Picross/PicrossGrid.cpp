// Copyright Sanya Larsson 2020

#include "PicrossGrid.h"
#include "Algo/Count.h"
#include "Algo/ForEach.h"
#include "Algo/Reverse.h"
#include "AssetDataObject.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/ObjectLibrary.h"
#include "Engine/TextRenderActor.h"
#include "Materials/MaterialInstance.h"


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
	if (!Puzzle.IsValid()) return;

	Unlock();
	DestroyGrid();
	UndoStack.Empty();
	RedoStack.Empty();
	SelectionAxis = ESelectionAxis::All;
	LastPivotXYZ = FIntVector::ZeroValue;
	SolutionFilledBlocksCount = Algo::Count(Puzzle.GetPuzzleData()->GetSolution(), true);
	CurrentlyFilledBlocksCount = 0;

	FVector StartPosition = GetActorLocation();
	StartPosition -= GetActorRightVector() * (DistanceBetweenBlocks * (Puzzle.Y() / 2) - (Puzzle.Y() % 2 == 0 ? DistanceBetweenBlocks / 2 : 0));
	StartPosition -= GetActorForwardVector() * (DistanceBetweenBlocks * (Puzzle.X() / 2) - (Puzzle.X() % 2 == 0 ? DistanceBetweenBlocks / 2 : 0));

	for (int32 Z = 0; Z < Puzzle.Z(); ++Z)
	{
		const float OffsetZ = DistanceBetweenBlocks * Z;
		for (int32 Y = 0; Y < Puzzle.Y(); ++Y)
		{
			const float OffsetY = DistanceBetweenBlocks * Y;
			for (int32 X = 0; X < Puzzle.X(); ++X)
			{
				const float OffsetX = DistanceBetweenBlocks * X;
				FVector BlockPosition = StartPosition;
				BlockPosition += GetActorForwardVector() * OffsetX;
				BlockPosition += GetActorRightVector() * OffsetY;
				BlockPosition += GetActorUpVector() * OffsetZ;

				const int32 MasterIndex = Puzzle.GetIndex(FIntVector(X,Y,Z));
				const FPicrossBlock& Block = Puzzle[MasterIndex] = FPicrossBlock{ EBlockState::Clear, FTransform(GetActorRotation(), BlockPosition, FVector::OneVector), MasterIndex };
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
					UpdateBlockState(Puzzle[Index], NewState, BlockInstances[PreviousState]->PerInstanceSMCustomData.IndexOfByKey(static_cast<float>(Puzzle.GetIndex(Index))));
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
	const FIntVector XYZ = Puzzle.GetIndex(MasterIndexPivot);
	const int32 EndIndex = (AxisToHighlight == ESelectionAxis::X ? Puzzle.X() : AxisToHighlight == ESelectionAxis::Y ? Puzzle.Y() : Puzzle.Z());

	for (int32 AxisIndex = 0; AxisIndex < EndIndex; ++AxisIndex)
	{
		const FIntVector MasterIndex = FIntVector(AxisToHighlight == ESelectionAxis::X ? AxisIndex : XYZ.X, AxisToHighlight == ESelectionAxis::Y ? AxisIndex : XYZ.Y, AxisToHighlight == ESelectionAxis::Z ? AxisIndex : XYZ.Z);
		FTransform HighlightBlockTransform = Puzzle[MasterIndex].Transform;
		HighlightBlockTransform.SetScale3D(HighlightBlockTransform.GetScale3D() * 1.05f);
		HighlightBlockTransform.AddToTranslation((-GetActorUpVector()) * 100.f * ((HighlightBlockTransform.GetScale3D().Z - Puzzle[MasterIndex].Transform.GetScale3D().Z) / 2));
		HighlightedBlocks->AddInstanceWorldSpace(HighlightBlockTransform);
	}
}

void APicrossGrid::EnableOnlyFilledBlocks() const
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (const FPicrossBlock& Block : Puzzle)
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
	CleanupNumbers();

	if (IsLocked()) return;

	GenerateNumbersForAxis(ESelectionAxis::X);
	GenerateNumbersForAxis(ESelectionAxis::Y);
	GenerateNumbersForAxis(ESelectionAxis::Z);
}

void APicrossGrid::GenerateNumbersForAxis(ESelectionAxis Axis)
{
	if (!Puzzle.IsValid()) return;

	const TArray<bool>& Solution = Puzzle.GetPuzzleData()->GetSolution();

	if (Solution.Num() != FArray3D::Size(Puzzle.GetGridSize())) return;

	// Axis1 is Y-axis if we're generating for X-axis, otherwise it's the X-axis.
	int32 Axis1Size = (Axis == ESelectionAxis::X ? Puzzle.Y() : Puzzle.X());
	for (int32 Axis1 = 0; Axis1 < Axis1Size; ++Axis1)
	{
		// Axis2 is Y-axis if we're generating for Z-Axis, otherwise it's the Z-axis.
		int32 Axis2Size = (Axis == ESelectionAxis::Z ? Puzzle.Y() : Puzzle.Z());
		for (int32 Axis2 = 0; Axis2 < Axis2Size; ++Axis2)
		{
			FFormatOrderedArguments Numbers;
			int32 Sum = 0;

			// Axis3 is the axis we're generating numbers for.
			int32 Axis3Size = (Axis == ESelectionAxis::Z ? Puzzle.Z() : Axis == ESelectionAxis::X ? Puzzle.X() : Puzzle.Y());
			for (int32 Axis3 = 0; Axis3 < Axis3Size; ++Axis3)
			{
				// De-anonymize Axis1,Axis2,Axis3 into their named version (X,Y,Z)
				FIntVector XYZ = Axis == ESelectionAxis::X ? FIntVector(Axis3, Axis1, Axis2) : Axis == ESelectionAxis::Y ? FIntVector(Axis1, Axis3, Axis2) : FIntVector(Axis1, Axis2, Axis3);

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
			if (Axis == ESelectionAxis::Z) Algo::Reverse(Numbers);

			CreateTextFromNumbersForAxis(Axis, Axis1, Axis2, Numbers);
		}
	}
}

void APicrossGrid::CreateTextFromNumbersForAxis(ESelectionAxis Axis, int32 Axis1, int32 Axis2, const FFormatOrderedArguments& Numbers)
{
	if (Numbers.Num() > 0)
	{
		FFormatOrderedArguments ReversedNumbers = Numbers;
		Algo::Reverse(ReversedNumbers);

		// Generate text from array of numbers with either a comma delimiter or new-line if Z-axis
		const FText Delimiter = FText::FromString(Axis == ESelectionAxis::Z ? TEXT("\n") : TEXT(", "));
		const FText Text1 = FText::Join(Delimiter, Numbers);
		const FText Text2 = FText::Join(Delimiter, ReversedNumbers);

		const FIntVector XYZ = Axis == ESelectionAxis::X ? FIntVector(0, Axis1, Axis2) : Axis == ESelectionAxis::Y ? FIntVector(Axis1, 0, Axis2) : FIntVector(Axis1, Axis2, Puzzle.Z() - 1);
		const FPicrossBlock& Block = Puzzle[XYZ];

		const FVector RelativeLocation = Axis == ESelectionAxis::X ? FVector(-75.f, 0.f, 50.f) : Axis == ESelectionAxis::Y ? FVector(0.f, -75.f, 50.f) : FVector(0.f, 0.f, 115.f);
		const FVector WorldLocation = Block.Transform.GetTranslation() + Block.Transform.GetRotation().RotateVector(RelativeLocation);

		const FRotator RelativeRotation1 = Axis == ESelectionAxis::X ? FRotator(0.f, 90.f, 0.f) : Axis == ESelectionAxis::Y ? FRotator(0.f, 180.f, 0.f) : FRotator(0.f, 90.f, 0.f);
		const FRotator RelativeRotation2 = RelativeRotation1 + FRotator(0.f, 180.f, 0.f);

		const FColor Color = Axis == ESelectionAxis::Z ? FColor::Blue : Axis == ESelectionAxis::Y ? FColor::Green : FColor::Red;

		const EHorizTextAligment HAlignment1 = Axis == ESelectionAxis::Z ? EHorizTextAligment::EHTA_Center : EHorizTextAligment::EHTA_Right;
		const EHorizTextAligment HAlignment2 = Axis == ESelectionAxis::Z ? EHorizTextAligment::EHTA_Center : EHorizTextAligment::EHTA_Left;
		const EVerticalTextAligment VAlignment = Axis == ESelectionAxis::Z ? EVerticalTextAligment::EVRTA_TextBottom : EVerticalTextAligment::EVRTA_TextCenter;

		ATextRenderActor* TextActor1 = CreateTextRenderActor(WorldLocation, RelativeRotation1, Text1, Color, HAlignment1, VAlignment);
		ATextRenderActor* TextActor2 = CreateTextRenderActor(WorldLocation, RelativeRotation2, Text2, Color, HAlignment2, VAlignment);
		FTextPair TextPair{ TextActor1, TextActor2 };

		switch (Axis)
		{
			case ESelectionAxis::X: NumbersXAxis.Add(FIntVector(0, Axis1, Axis2), TextPair); break;
			case ESelectionAxis::Y: NumbersYAxis.Add(FIntVector(Axis1, 0, Axis2), TextPair); break;
			case ESelectionAxis::Z: NumbersZAxis.Add(FIntVector(Axis1, Axis2, 0), TextPair); break;
		}
	}
}

ATextRenderActor* APicrossGrid::CreateTextRenderActor(FVector WorldLocation, FRotator RelativeRotation, FText Text, FColor Color, EHorizTextAligment HAlignment, EVerticalTextAligment VAlignment)
{
	ATextRenderActor* TextActor = GetWorld()->SpawnActorAbsolute<ATextRenderActor>(WorldLocation, RelativeRotation);
	if (TextActor)
	{
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
	}

	return TextActor;
}

void APicrossGrid::ForEachTextActor(const TFunctionRef<void(TPair<FIntVector, FTextPair>&)> Func)
{
	Algo::ForEach(NumbersXAxis, Func);
	Algo::ForEach(NumbersYAxis, Func);
	Algo::ForEach(NumbersZAxis, Func);
}

void APicrossGrid::CleanupNumbers()
{
	static const auto DestroyTextActors = [](TPair<FIntVector, FTextPair>& Pair) -> void
	{
		if (Pair.Value.Text1) Pair.Value.Text1->Destroy();
		if (Pair.Value.Text2) Pair.Value.Text2->Destroy();
	};

	ForEachTextActor(DestroyTextActors);
	NumbersXAxis.Empty();
	NumbersYAxis.Empty();
	NumbersZAxis.Empty();
}

void APicrossGrid::UpdateNumbersVisibility()
{
	const ESelectionAxis Axis = SelectionAxis;
	const FIntVector Index = LastPivotXYZ;
	auto ShouldShow = [Axis, Index](TPair<FIntVector, FTextPair>& Pair) -> bool
	{
		return (Axis == ESelectionAxis::X ? Pair.Key.X == Index.X : Axis == ESelectionAxis::Y ? Pair.Key.Y == Index.Y : Pair.Key.Z == Index.Z);
	};

	static const auto HideText = [](TPair<FIntVector, FTextPair>& Pair) -> void
	{
		if (Pair.Value.Text1) Pair.Value.Text1->SetActorHiddenInGame(true);
		if (Pair.Value.Text2) Pair.Value.Text2->SetActorHiddenInGame(true);
	};

	static const auto ShowText = [](TPair<FIntVector, FTextPair>& Pair) -> void
	{
		if (Pair.Value.Text1) Pair.Value.Text1->SetActorHiddenInGame(false);
		if (Pair.Value.Text2) Pair.Value.Text2->SetActorHiddenInGame(false);
	};

	ForEachTextActor(HideText);

	switch (SelectionAxis)
	{
		case ESelectionAxis::X:
			Algo::ForEachIf(NumbersYAxis, ShouldShow, ShowText);
			Algo::ForEachIf(NumbersZAxis, ShouldShow, ShowText);
			RotateNumbersXAxis();
			break;
		case ESelectionAxis::Y:
			Algo::ForEachIf(NumbersXAxis, ShouldShow, ShowText);
			Algo::ForEachIf(NumbersZAxis, ShouldShow, ShowText);
			RotateNumbersYAxis();
			break;
		case ESelectionAxis::Z:
			Algo::ForEachIf(NumbersXAxis, ShouldShow, ShowText);
			Algo::ForEachIf(NumbersYAxis, ShouldShow, ShowText);
			RotateNumbersZAxis();
			break;
		case ESelectionAxis::All:
			ForEachTextActor(ShowText);
			RotateNumbersAllAxis();
			break;
	}
}

void APicrossGrid::RotateNumbersXAxis()
{
	const FRotator Rotation1{ 0.f, -180.f, 0.f };
	const FRotator Rotation2{ 0.f, 0.f, 0.f };

	static const auto SetRotation = [Rotation1, Rotation2](TPair<FIntVector, FTextPair>& Pair) -> void
	{
		if (Pair.Value.Text1) Pair.Value.Text1->SetActorRelativeRotation(Rotation1);
		if (Pair.Value.Text2) Pair.Value.Text2->SetActorRelativeRotation(Rotation2);
	};

	Algo::ForEach(NumbersYAxis, SetRotation);
	Algo::ForEach(NumbersZAxis, SetRotation);
}

void APicrossGrid::RotateNumbersYAxis()
{
	const FRotator Rotation1{ 0.f, 90.f, 0.f };
	const FRotator Rotation2{ 0.f, -90.f, 0.f };

	static const auto SetRotation = [Rotation1, Rotation2](TPair<FIntVector, FTextPair>& Pair) -> void
	{
		if (Pair.Value.Text1) Pair.Value.Text1->SetActorRelativeRotation(Rotation1);
		if (Pair.Value.Text2) Pair.Value.Text2->SetActorRelativeRotation(Rotation2);
	};

	Algo::ForEach(NumbersXAxis, SetRotation);
	Algo::ForEach(NumbersZAxis, SetRotation);
}

void APicrossGrid::RotateNumbersZAxis()
{
	const FRotator RotationX1{ 90.f, 90.f, 0.f };
	const FRotator RotationX2{ -90.f, -90.f, 0.f };

	static const auto SetRotationX = [RotationX1, RotationX2](TPair<FIntVector, FTextPair>& Pair) -> void
	{
		if (Pair.Value.Text1) Pair.Value.Text1->SetActorRelativeRotation(RotationX1);
		if (Pair.Value.Text2) Pair.Value.Text2->SetActorRelativeRotation(RotationX2);
	};

	const FRotator RotationY1{ 90.f, -180.f, 0.f };
	const FRotator RotationY2{ -90.f, 0.f, 0.f };

	static const auto SetRotationY = [RotationY1, RotationY2](TPair<FIntVector, FTextPair>& Pair) -> void
	{
		if (Pair.Value.Text1) Pair.Value.Text1->SetActorRelativeRotation(RotationY1);
		if (Pair.Value.Text2) Pair.Value.Text2->SetActorRelativeRotation(RotationY2);
	};

	Algo::ForEach(NumbersXAxis, SetRotationX);
	Algo::ForEach(NumbersYAxis, SetRotationY);
}

void APicrossGrid::RotateNumbersAllAxis()
{
	const FRotator RotationXZ1{ 0.f, 90.f, 0.f };
	const FRotator RotationXZ2{ 0.f, -90.f, 0.f };

	static const auto SetRotationXZ = [RotationXZ1, RotationXZ2](TPair<FIntVector, FTextPair>& Pair) -> void
	{
		if (Pair.Value.Text1) Pair.Value.Text1->SetActorRelativeRotation(RotationXZ1);
		if (Pair.Value.Text2) Pair.Value.Text2->SetActorRelativeRotation(RotationXZ2);
	};

	const FRotator RotationY1{ 0.f, -180.f, 0.f };
	const FRotator RotationY2{ 0.f, 0.f, 0.f };

	static const auto SetRotationY = [RotationY1, RotationY2](TPair<FIntVector, FTextPair>& Pair) -> void
	{
		if (Pair.Value.Text1) Pair.Value.Text1->SetActorRelativeRotation(RotationY1);
		if (Pair.Value.Text2) Pair.Value.Text2->SetActorRelativeRotation(RotationY2);
	};

	Algo::ForEach(NumbersXAxis, SetRotationXZ);
	Algo::ForEach(NumbersYAxis, SetRotationY);
	Algo::ForEach(NumbersZAxis, SetRotationXZ);
}

void APicrossGrid::Cycle2DRotation(const int32 MasterIndexPivot)
{
	if (IsLocked()) return;

	SelectionAxis = (SelectionAxis == ESelectionAxis::All ? ESelectionAxis::Z : SelectionAxis == ESelectionAxis::Z ? ESelectionAxis::Y : SelectionAxis == ESelectionAxis::Y ? ESelectionAxis::X : ESelectionAxis::All);

	LastPivotXYZ = MasterIndexPivot >= 0 ? Puzzle.GetIndex(MasterIndexPivot) : LastPivotXYZ;

	switch (SelectionAxis)
	{
		case ESelectionAxis::X:		SetRotationXAxis();	break;
		case ESelectionAxis::Y:		SetRotationYAxis();	break;
		case ESelectionAxis::Z:		SetRotationZAxis();	break;
		case ESelectionAxis::All:	EnableAllBlocks();	break;
	}

	UpdateNumbersVisibility();
}

void APicrossGrid::SetRotationXAxis() const
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

void APicrossGrid::SetRotationYAxis() const
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

void APicrossGrid::SetRotationZAxis() const
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

void APicrossGrid::EnableAllBlocks() const
{
	if (IsLocked()) return;

	DisableAllBlocks();

	for (const FPicrossBlock& Block : Puzzle)
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
		case ESelectionAxis::X:
			LastPivotXYZ.X = LastPivotXYZ.X > 0 ? LastPivotXYZ.X - 1 : Puzzle.X() - 1;
			SetRotationXAxis();
			break;
		case ESelectionAxis::Y:
			LastPivotXYZ.Y = LastPivotXYZ.Y > 0 ? LastPivotXYZ.Y - 1 : Puzzle.Y() - 1;
			SetRotationYAxis();
			break;
		case ESelectionAxis::Z:
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
		case ESelectionAxis::X:
			LastPivotXYZ.X = LastPivotXYZ.X < Puzzle.X() - 1 ? LastPivotXYZ.X + 1 : 0;
			SetRotationXAxis();
			break;
		case ESelectionAxis::Y:
			LastPivotXYZ.Y = LastPivotXYZ.Y < Puzzle.Y() - 1 ? LastPivotXYZ.Y + 1 : 0;
			SetRotationYAxis();
			break;
		case ESelectionAxis::Z:
			LastPivotXYZ.Z = LastPivotXYZ.Z > 0 ? LastPivotXYZ.Z - 1 : Puzzle.Z() - 1;
			SetRotationZAxis();
			break;
	}

	UpdateNumbersVisibility();
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