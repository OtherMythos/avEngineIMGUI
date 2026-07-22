#include "CompositorPassImgui.h"

#include "CompositorPassImguiDef.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorNodeDef.h"

#include "ImguiOgre/ImguiManager.h"

#include <limits>

namespace AVImgui{

    CompositorPassImgui::CompositorPassImgui(const CompositorPassImguiDef* definition, const Ogre::RenderTargetViewDef* rtv, Ogre::CompositorNode* parentNode)
        : Ogre::CompositorPass(definition, parentNode),
          mTargetTexture(0){

        initialize(rtv);

        if(!rtv->colourAttachments.empty()){
            mTargetTexture = mParentNode->getDefinedTexture(rtv->colourAttachments[0].textureName);
        }
    }

    void CompositorPassImgui::execute(const Ogre::Camera* lodCamera){
        //Execute a limited number of times?
        if(mNumPassesLeft != std::numeric_limits<Ogre::uint32>::max())
        {
            if(!mNumPassesLeft) return;
            --mNumPassesLeft;
        }

        profilingBegin();

        notifyPassEarlyPreExecuteListeners();

        executeResourceTransitions();

        notifyPassPreExecuteListeners();

        ImguiManager* manager = ImguiManager::getSingletonPtr();
        if(manager && mTargetTexture){
            manager->render(mRenderPassDesc, mTargetTexture);
        }

        notifyPassPosExecuteListeners();

        profilingEnd();
    }
}
