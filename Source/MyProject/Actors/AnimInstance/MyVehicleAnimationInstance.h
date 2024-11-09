#pragma once

#include "CoreMinimal.h"
#include "VehicleAnimationInstance.h"
#include "MyVehicleAnimationInstance.generated.h"

UCLASS()
class MYPROJECT_API UMyVehicleAnimationInstance : public UVehicleAnimationInstance
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoneRotation")
    float TurretRotationYaws;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoneRotation")
    float GunRotationPitchs;  
};
