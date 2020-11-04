// Copyright Sanya Larsson 2020

#pragma once

#include "CoreMinimal.h"
#include "Picross/PicrossGrid.h"
#include "PicrossGridCreator.generated.h"

/**
 * Picross Puzzle Creator, used to create Puzzles.
 */
UCLASS()
class PICROSSEDITOR_API APicrossGridCreator : public APicrossGrid
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void CreatePuzzle();
	UFUNCTION(BlueprintCallable, Category = "Picross")
	void CreatePuzzleWithSize(FIntVector SizeOfGrid);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Picross")
	void SavePuzzle();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Picross", meta = (AllowPrivateAccess = "true"))
	FIntVector GridSize{ 5, 5, 5 };
};
