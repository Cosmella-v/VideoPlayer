#include <Geode/modify/MenuLayer.hpp>
#include "../include/VideoPlayer.hpp"
using namespace geode::prelude;
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
    }
	virtual bool init()
	{
		if (!MenuLayer::init())
			return false;
        
        Loader::get()->queueInMainThread([=] {
            Vid();
        });

        return true;
    };
};
