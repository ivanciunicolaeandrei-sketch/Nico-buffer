#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(NicoBufferMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto menu = this->getChildByID("bottom-menu");
        if (!menu) return true;

        auto button = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png"),
            this,
            menu_selector(NicoBufferMenuLayer::onNicoBuffer)
        );
        if (!button) return true;

        button->setID("nico-buffer-button"_spr);
        menu->addChild(button);
        menu->updateLayout();
        return true;
    }

    void onNicoBuffer(CCObject*) {
        FLAlertLayer::create(
            "Nico Buffer",
            "Initial Android64 build scaffold is installed.\nInput-buffer controls are coming in the next development step.",
            "OK"
        )->show();
    }
};
