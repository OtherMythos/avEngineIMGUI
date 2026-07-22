#pragma once

#include "System/Plugins/Plugin.h"

namespace Ogre{
    class CompositorWorkspace;
    class Camera;
}

namespace AVImgui{
    class CompositorPassImguiProvider;

    class AvImguiPlugin : public AV::Plugin {
    public:
        AvImguiPlugin();
        ~AvImguiPlugin();

        virtual void initialise() override;
        virtual void shutdown() override;

        /**
        Create the ready-made overlay workspace which renders imgui over the
        window contents. Created automatically when the engine uses its default
        compositor; projects with a custom compositor setup can call this from
        script once their own workspaces exist.

        @returns true if the workspace was created, false if it already existed.
        */
        static bool createOverlayWorkspace();

        //@returns true if the workspace existed and was destroyed.
        static bool destroyOverlayWorkspace();

    private:
        static CompositorPassImguiProvider* mProvider;
        static Ogre::CompositorWorkspace* mOverlayWorkspace;
        static Ogre::Camera* mCamera;
    };
}
