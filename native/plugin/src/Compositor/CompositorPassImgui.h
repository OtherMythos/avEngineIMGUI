#pragma once

#include "OgrePrerequisites.h"
#include "Compositor/Pass/OgreCompositorPass.h"

namespace AVImgui{
    class CompositorPassImguiDef;

    /**
    A custom ogre compositor pass which renders the current Dear ImGui frame.
    Intended to be placed as the final pass of a target so the gui draws over
    the rendered scene.
    */
    class CompositorPassImgui : public Ogre::CompositorPass{
    public:
        CompositorPassImgui(const CompositorPassImguiDef* definition, const Ogre::RenderTargetViewDef* rtv, Ogre::CompositorNode* parentNode);

        virtual void execute(const Ogre::Camera* lodCamera);

    private:
        Ogre::TextureGpu* mTargetTexture;
    };
}
