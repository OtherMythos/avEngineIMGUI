#pragma once

#include "OgrePrerequisites.h"
#include "Compositor/Pass/OgreCompositorPassProvider.h"

namespace AVImgui{

    /**
    A compositor pass provider which handles the 'imgui' custom pass id and
    delegates every other custom pass to the provider which was registered
    before the plugin loaded (the engine registers its own provider for the
    'rect2d' and 'colibri_gui' passes).

    Ogre only allows a single pass provider per CompositorManager2, so the
    plugin wraps rather than replaces the existing one.
    */
    class CompositorPassImguiProvider : public Ogre::CompositorPassProvider{
    public:
        CompositorPassImguiProvider(Ogre::CompositorPassProvider* wrapped);

        virtual Ogre::CompositorPassDef* addPassDef( Ogre::CompositorPassType passType,
                                       Ogre::IdString customId,
                                       Ogre::CompositorTargetDef *parentTargetDef,
                                       Ogre::CompositorNodeDef *parentNodeDef );

        virtual Ogre::CompositorPass* addPass( const Ogre::CompositorPassDef *definition, Ogre::Camera *defaultCamera,
                                         Ogre::CompositorNode *parentNode, const Ogre::RenderTargetViewDef* target,
                                         Ogre::SceneManager *sceneManager );

        virtual void translateCustomPass( Ogre::ScriptCompiler *compiler, const Ogre::AbstractNodePtr &node,
                                          Ogre::IdString customId, Ogre::CompositorPassDef *customPassDef );

        Ogre::CompositorPassProvider* getWrappedProvider() const { return mWrapped; }

    private:
        Ogre::CompositorPassProvider* mWrapped;
    };
}
