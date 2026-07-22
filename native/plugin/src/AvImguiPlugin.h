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
        window contents.

        This happens automatically the first time a frame uses imgui, so most
        projects never need to call it. Ogre appends new workspaces, so the
        overlay always executes after the workspaces which exist at that point.

        @returns true if the workspace was created, false if it already existed.
        */
        static bool createOverlayWorkspace();

        /**
        Destroy the overlay workspace. This also disables automatic creation,
        so the overlay does not immediately come back on the next frame.

        @returns true if the workspace existed and was destroyed.
        */
        static bool destroyOverlayWorkspace();

        /**
        Enable or disable automatic creation of the overlay workspace. Projects
        which render imgui themselves with a 'pass custom imgui' in their own
        compositor should disable this during setup.
        */
        static void setAutoOverlayEnabled(bool enabled) { mAutoOverlayEnabled = enabled; }
        static bool getAutoOverlayEnabled() { return mAutoOverlayEnabled; }

    private:
        //Called before each new imgui frame begins.
        static void _prepareFrame();

        static CompositorPassImguiProvider* mProvider;
        static Ogre::CompositorWorkspace* mOverlayWorkspace;
        static Ogre::Camera* mCamera;
        static bool mAutoOverlayEnabled;
    };
}
