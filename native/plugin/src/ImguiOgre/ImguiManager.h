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

        /**
        Scale the whole gui by a factor, for displays where imgui's default
        13px font comes out too small - a window on a 2x screen renders at
        twice the size in pixels, which is what imgui is given as its display
        size, so everything in it is half the physical size it would be at 1x.

        Style sizes are scaled and the font is re-baked at the scaled size, so
        text is drawn from real glyphs rather than a stretched bitmap.

        Takes effect at the start of the next frame: the atlas cannot be
        re-baked while a frame is being built. @see ensureFrameStarted
        */
        void setGlobalScale(float scale);
        //The scale asked for, which is in effect from the next frame.
        float getGlobalScale() const { return mPendingScale > 0.0f ? mPendingScale : mGlobalScale; }

        //Called at the very start of each new ImGui frame, before NewFrame().
        //Used by the input layer to sync SDL text input state with WantTextInput.
        void setPreNewFrameCallback(std::function<void()> callback) { mPreNewFrameCallback = callback; }

        void shutdown();

    private:
        void createFontTexture();
        //Bake the font atlas at the current scale and give the result to Ogre
        //as a texture, replacing the one already in use if there is one.
        void buildFontTexture();
        void applyGlobalScale(float scale);
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

        //The scale the style and font atlas were last built for, and the one
        //asked for since, applied between frames. @see setGlobalScale
        float mGlobalScale;
        float mPendingScale;
        //The style sizes imgui set up, before any scale was applied to them.
        //Scaling always starts from these, as scaling a scaled style compounds.
        ImGuiStyle mBaseStyle;
        //How many atlas textures have been replaced, which is what keeps their
        //names apart. @see buildFontTexture
        unsigned int mFontTexIndex;

        //The descriptor the PSO cache was last primed with.
        Ogre::RenderPassDescriptor* mLastPsoDescriptor;

        std::chrono::steady_clock::time_point mLastFrameTime;
        bool mHasFrameTime;

        std::function<void()> mPreNewFrameCallback;
    };
}
