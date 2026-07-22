#pragma once

//Baked-in copy of the Dear ImGui Ogre-next renderer from ogre-next-imgui (MIT),
//ported to imgui 1.92 and re-architected to render from a compositor pass
//rather than a frame listener.

#include "imgui.h"
#include "ImguiRenderable.h"

#include <OgreTextureGpu.h>
#include <OgreFastArray.h>
#include <OgreRenderPassDescriptor.h>

#include <chrono>
#include <functional>

namespace Ogre{
    class PsoCacheHelper;
    class SceneManager;
}

namespace AVImgui{

    class ImguiManager{
    public:
        ImguiManager();
        ~ImguiManager();

        static ImguiManager* getSingletonPtr(void);
        static void createSingleton();
        static void destroySingleton();

        //call once before using ImGui
        void init(Ogre::SceneManager* sceneManager, Ogre::TextureGpu* displayTarget);

        /**
        Begin the ImGui frame if it has not been begun yet this engine frame.
        Called implicitly by every _imgui script binding, so scripts never have
        to manage the frame lifecycle themselves.
        */
        void ensureFrameStarted();

        //True if a frame has been started and not yet rendered.
        bool isFrameActive() const { return mFrameActive; }

        /**
        True if no imgui frame has been begun yet for the engine frame which is
        about to render. The engine runs script updates on a fixed timestep, so
        a single rendered frame can receive multiple update calls; scripts can
        use this to avoid submitting their gui more than once per rendered frame.
        */
        bool wouldBeFirstUpdateOfFrame() const;

        /**
        Render the current ImGui frame. Called by the imgui compositor pass.
        The render pass descriptor and target texture are provided by the pass.
        */
        void render(Ogre::RenderPassDescriptor* renderPassDesc, Ogre::TextureGpu* renderTarget);

        void updateProjectionMatrix(float width, float height);

        void setRenderingEnabled(bool enabled) { mRenderingEnabled = enabled; }
        bool getRenderingEnabled() const { return mRenderingEnabled; }

        //Called at the very start of each new ImGui frame, before NewFrame().
        //Used by the input layer to sync SDL text input state with WantTextInput.
        void setPreNewFrameCallback(std::function<void()> callback) { mPreNewFrameCallback = callback; }

        void shutdown();

    private:
        void createFontTexture();
        void createMaterial();

        static ImguiManager* msSingleton;

        Ogre::FastArray<ImguiRenderable*> mRenderables;

        Ogre::PsoCacheHelper* mPSOCache;

        Ogre::SceneManager* mSceneMgr;
        Ogre::TextureGpu* mDisplayTarget;

        Ogre::Pass* mPass;
        Ogre::TextureGpu* mFontTex;
        const Ogre::HlmsSamplerblock* mSamplerblock;

        bool mFrameActive;
        unsigned long mFrameStartedOgreFrame;
        bool mRenderingEnabled;
        bool mVulkan;

        //Whether valid draw data exists from a previous ImGui::Render(). The
        //engine's fixed timestep means some rendered frames receive no script
        //update; the previous frame's gui is re-presented for those so the gui
        //does not flicker when the framerate exceeds the fixed update rate.
        bool mHaveDrawData;
        unsigned long mLastFreshDataFrame;

        float mPrevWidth, mPrevHeight;

        //The descriptor the PSO cache was last primed with.
        Ogre::RenderPassDescriptor* mLastPsoDescriptor;

        std::chrono::steady_clock::time_point mLastFrameTime;
        bool mHasFrameTime;

        std::function<void()> mPreNewFrameCallback;
    };
}
