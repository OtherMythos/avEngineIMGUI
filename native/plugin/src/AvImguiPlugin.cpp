#include "AvImguiPlugin.h"

#include "System/Plugins/PluginManager.h"
#include "System/BaseSingleton.h"
#include "System/SystemSetup/SystemSettings.h"
#include "Window/Window.h"
#include "Scripting/ScriptVM.h"

#include "ImguiOgre/ImguiManager.h"
#include "Compositor/CompositorPassImguiProvider.h"
#include "Input/ImguiInput.h"
#include "Scripting/ImguiNamespace.h"

#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreWindow.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/Pass/OgreCompositorPassDef.h"

namespace AVImgui{

#ifdef WIN32
    #define DLLEXPORT __declspec(dllexport)
#else
    #define DLLEXPORT
#endif

    extern "C" DLLEXPORT void dllStartPlugin(void){
        AvImguiPlugin* p = new AvImguiPlugin();
        AV::PluginManager::registerPlugin(p);
    }

    CompositorPassImguiProvider* AvImguiPlugin::mProvider = 0;
    Ogre::CompositorWorkspace* AvImguiPlugin::mOverlayWorkspace = 0;
    Ogre::Camera* AvImguiPlugin::mCamera = 0;

    AvImguiPlugin::AvImguiPlugin()
        : Plugin("AvImguiPlugin"){

    }

    AvImguiPlugin::~AvImguiPlugin(){

    }

    void AvImguiPlugin::initialise(){
        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        Ogre::CompositorManager2* compositorManager = root->getCompositorManager2();

        //Ogre supports a single compositor pass provider, and the engine
        //registers one of its own, so wrap it rather than replace it.
        mProvider = OGRE_NEW CompositorPassImguiProvider(compositorManager->getCompositorPassProvider());
        compositorManager->setCompositorPassProvider(mProvider);

        Ogre::SceneManager* sceneManager = AV::BaseSingleton::getSceneManager();
        Ogre::TextureGpu* windowTexture = AV::BaseSingleton::getWindow()->getRenderWindow()->getTexture();

        ImguiManager::createSingleton();
        ImguiManager::getSingletonPtr()->init(sceneManager, windowTexture);
        ImguiManager::getSingletonPtr()->setPreNewFrameCallback(&ImguiInput::applyTextInputState);

        //Register a ready-made node and workspace containing the imgui pass.
        //This is done programmatically because compositor scripts are parsed
        //before plugins load, so a 'pass custom imgui' in a script parsed at
        //startup would not be recognised.
        {
            Ogre::CompositorNodeDef* nodeDef = compositorManager->addNodeDefinition("avImgui/RenderNode");
            nodeDef->addTextureSourceName("rt_renderwindow", 0, Ogre::TextureDefinitionBase::TEXTURE_INPUT);
            nodeDef->setNumTargetPass(1);
            Ogre::CompositorTargetDef* targetDef = nodeDef->addTargetPass("rt_renderwindow");
            targetDef->setNumPasses(1);
            targetDef->addPass(Ogre::PASS_CUSTOM, "imgui");

            Ogre::CompositorWorkspaceDef* workspaceDef = compositorManager->addWorkspaceDefinition("avImgui/Workspace");
            workspaceDef->connectExternal(0, "avImgui/RenderNode", 0);
        }

        //With the default compositor the overlay can be created immediately.
        //Projects with their own compositor setup create their workspaces from
        //script, so the overlay would execute before them and be drawn over.
        //Those projects call _imgui.createOverlayWorkspace() themselves.
        if(AV::SystemSettings::getUseDefaultCompositor()){
            createOverlayWorkspace();
        }

        ImguiInput::initialise();

        AV::ScriptVM::setupNamespace("_imgui", ImguiNamespace::setupNamespace);
    }

    bool AvImguiPlugin::createOverlayWorkspace(){
        if(mOverlayWorkspace) return false;

        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        Ogre::CompositorManager2* compositorManager = root->getCompositorManager2();
        Ogre::SceneManager* sceneManager = AV::BaseSingleton::getSceneManager();
        Ogre::TextureGpu* windowTexture = AV::BaseSingleton::getWindow()->getRenderWindow()->getTexture();

        //The workspace requires a camera even though the imgui pass never
        //reads it. A dedicated one keeps the plugin decoupled from the
        //lifetime of any camera the project owns.
        if(!mCamera){
            mCamera = sceneManager->createCamera("avImgui/Camera");
        }

        mOverlayWorkspace = compositorManager->addWorkspace(sceneManager, windowTexture, mCamera, "avImgui/Workspace", true);

        return true;
    }

    bool AvImguiPlugin::destroyOverlayWorkspace(){
        if(!mOverlayWorkspace) return false;

        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        root->getCompositorManager2()->removeWorkspace(mOverlayWorkspace);
        mOverlayWorkspace = 0;

        return true;
    }

    void AvImguiPlugin::shutdown(){
        ImguiInput::shutdown();

        destroyOverlayWorkspace();

        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        if(root){
            Ogre::CompositorManager2* compositorManager = root->getCompositorManager2();

            if(mCamera){
                AV::BaseSingleton::getSceneManager()->destroyCamera(mCamera);
                mCamera = 0;
            }

            if(mProvider){
                compositorManager->setCompositorPassProvider(mProvider->getWrappedProvider());
                OGRE_DELETE mProvider;
                mProvider = 0;
            }
        }

        if(ImguiManager::getSingletonPtr()){
            ImguiManager::getSingletonPtr()->shutdown();
            ImguiManager::destroySingleton();
        }
    }
}
