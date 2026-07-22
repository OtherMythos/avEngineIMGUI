#pragma once

#include "Compositor/Pass/OgreCompositorPassDef.h"

namespace AVImgui{

    //Identifier used to recognise imgui pass definitions in the provider.
    static const Ogre::uint32 IMGUI_PASS_IDENTIFIER = 3200;

    class CompositorPassImguiDef : public Ogre::CompositorPassDef{
    public:
        CompositorPassImguiDef(Ogre::CompositorTargetDef *parentTargetDef)
            : Ogre::CompositorPassDef(Ogre::PASS_CUSTOM, parentTargetDef){
            mProfilingId = "ImguiCompositor";
            mIdentifier = IMGUI_PASS_IDENTIFIER;
        }
    };
}
