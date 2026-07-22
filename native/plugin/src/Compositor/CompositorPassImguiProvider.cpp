#include "CompositorPassImguiProvider.h"

#include "CompositorPassImguiDef.h"
#include "CompositorPassImgui.h"

namespace AVImgui{

    CompositorPassImguiProvider::CompositorPassImguiProvider(Ogre::CompositorPassProvider* wrapped)
        : mWrapped(wrapped) {

    }

    Ogre::CompositorPassDef* CompositorPassImguiProvider::addPassDef( Ogre::CompositorPassType passType,
                                   Ogre::IdString customId,
                                   Ogre::CompositorTargetDef *parentTargetDef,
                                   Ogre::CompositorNodeDef *parentNodeDef){
        if(customId == "imgui"){
            return OGRE_NEW CompositorPassImguiDef(parentTargetDef);
        }

        if(mWrapped){
            return mWrapped->addPassDef(passType, customId, parentTargetDef, parentNodeDef);
        }

        return 0;
    }

    Ogre::CompositorPass* CompositorPassImguiProvider::addPass( const Ogre::CompositorPassDef *definition, Ogre::Camera *defaultCamera,
                                     Ogre::CompositorNode *parentNode, const Ogre::RenderTargetViewDef* target,
                                     Ogre::SceneManager *sceneManager){
        if(definition->mIdentifier == IMGUI_PASS_IDENTIFIER){
            const CompositorPassImguiDef* imguiDef = static_cast<const CompositorPassImguiDef*>( definition );
            return OGRE_NEW CompositorPassImgui(imguiDef, target, parentNode);
        }

        if(mWrapped){
            return mWrapped->addPass(definition, defaultCamera, parentNode, target, sceneManager);
        }

        assert(false && "Unknown custom compositor pass definition.");
        return 0;
    }

    void CompositorPassImguiProvider::translateCustomPass( Ogre::ScriptCompiler *compiler, const Ogre::AbstractNodePtr &node,
                                          Ogre::IdString customId, Ogre::CompositorPassDef *customPassDef ){
        if(customId == "imgui"){
            //No custom script properties for the imgui pass.
            return;
        }

        if(mWrapped){
            mWrapped->translateCustomPass(compiler, node, customId, customPassDef);
        }
    }
}
