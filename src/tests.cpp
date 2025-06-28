#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "../include/VideoPlayer.hpp"
using namespace geode::prelude;
// 33.0 MB
class $modify(TestLayer, MenuLayer)
{
    void Vid() {
        auto video = videoplayer::VideoPlayer::create(Mod::get()->getResourcesDir() / "tennaIntroF1_compressed_28.mpeg",false);
        
        video->onVideoEnd([=](){
            video->removeMeAndCleanup();
        });

        auto pos = CCDirector::sharedDirector();
        video->setPosition(pos->getWinSize() / 2);

        this->addChild(video);
    };


	bool init()
	{
		if (!MenuLayer::init())
			return false;

        log::debug("inited");
        //Loader::get()->queueInMainThread([=] {
            Vid();
       // });

        return true;
    };
};
