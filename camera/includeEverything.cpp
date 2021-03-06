#include "AnchoredCamera.h"
#include "AutoSplineCamera.h"
#include "CameraDrawWrapper.h"
#include "CameraManager.h"
#include "CameraMode_CameraBone.h"
#include "CameraMode_Combat.h"
#include "CameraMode_Cover.h"
#include "CameraMode_Dialog.h"
#include "CameraMode_Hacking.h"
#include "CameraMode_IronSight.h"
#include "CameraMode_Ladder.h"
#include "CameraMode_Orbit.h"
#include "CameraModeLyingDown.h"
#include "CameraModeOperation.h"
#include "CameraOperation.h"
#include "CameraOperation_AnimationDOF.h"
#include "CameraOperation_AnimationFOV.h"
#include "CameraOperation_Blend.h"
#include "CameraOperation_Block.h"
#include "CameraOperation_CrossBlend.h"
#include "CameraOperation_FreeLook.h"
#include "CameraOperation_HorizontalFOV.h"
#include "CameraOperation_LocalSpaceOffset.h"
#include "CameraOperation_MultiBlend.h"
#include "CameraOperation_PreventTargetHiding.h"
#include "CameraOperation_ResolveContacts.h"
#include "CameraOperation_SetFromCinematic.h"
#include "CameraOperation_UseAbsoluteTransformation.h"
#include "CameraOperation_UseRotation.h"
#include "CinematicCamera.h"
#include "FreeCamera.h"
#include "GenericCamera.h"
#include "ICamera.h"
#include "ICameraManager.h"
#include "ModelViewCamera.h"
#include "ObjectDebugCamera.h"
#include "OrbitDebugCamera.h"
#include "PlayerCamera.h"
#include "TransitionCamera.h"
