#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <deque>

using namespace geode::prelude;

namespace nico {
    struct PendingJump {
        bool player1;
        int remaining;
        bool released;
    };

    constexpr int JUMP_BUTTON = 1;

    int bufferFrames() {
        return static_cast<int>(Mod::get()->getSettingValue<int64_t>("buffer-frames"));
    }

    bool enabled() {
        return Mod::get()->getSettingValue<bool>("buffer-enabled") && bufferFrames() > 0;
    }
}

class $modify(NicoBufferGameLayer, GJBaseGameLayer) {
    struct Fields {
        std::deque<nico::PendingJump> pending;
        bool applying = false;
    };

    void handleButton(bool down, int button, bool isPlayer1) {
        auto* fields = m_fields.self();

        if (fields->applying || !nico::enabled() || button != nico::JUMP_BUTTON) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        auto* player = isPlayer1 ? m_player1 : m_player2;
        if (!player || player->m_isDart) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        auto pending = fields->pending.end();
        for (auto it = fields->pending.begin(); it != fields->pending.end(); ++it) {
            if (it->player1 == isPlayer1) {
                pending = it;
                break;
            }
        }

        if (!down && pending != fields->pending.end()) {
            pending->released = true;
            return;
        }

        if (down && !player->m_isOnGround) {
            const int window = nico::bufferFrames();
            if (pending != fields->pending.end()) {
                pending->remaining = window;
                pending->released = false;
            } else {
                fields->pending.push_back({isPlayer1, window, false});
            }
            return;
        }

        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        auto* fields = m_fields.self();

        for (auto it = fields->pending.begin(); it != fields->pending.end();) {
            auto* player = it->player1 ? m_player1 : m_player2;
            if (!player) {
                it = fields->pending.erase(it);
                continue;
            }

            if (player->m_isOnGround && !player->m_isDart) {
                const bool released = it->released;
                const bool player1 = it->player1;

                fields->applying = true;
                GJBaseGameLayer::handleButton(true, nico::JUMP_BUTTON, player1);
                if (released) {
                    GJBaseGameLayer::handleButton(false, nico::JUMP_BUTTON, player1);
                }
                fields->applying = false;

                it = fields->pending.erase(it);
                continue;
            }

            if (--it->remaining <= 0) {
                it = fields->pending.erase(it);
            } else {
                ++it;
            }
        }

        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};

class $modify(NicoBufferPlayLayer, PlayLayer) {
    void resetLevel() {
        auto* base = static_cast<NicoBufferGameLayer*>(static_cast<GJBaseGameLayer*>(this));
        base->m_fields->pending.clear();
        PlayLayer::resetLevel();
    }
};

class $modify(NicoBufferMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto menu = this->getChildByID("bottom-menu");
        if (!menu) return true;

        auto* sprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        if (!sprite) return true;

        auto* button = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(NicoBufferMenuLayer::onNicoBuffer)
        );
        if (!button) return true;

        button->setID("nico-buffer-button"_spr);
        menu->addChild(button);
        menu->updateLayout();
        return true;
    }

    void onNicoBuffer(CCObject*) {
        const auto enabled = Mod::get()->getSettingValue<bool>("buffer-enabled");
        const auto frames = nico::bufferFrames();
        FLAlertLayer::create(
            "Nico Buffer",
            fmt::format(
                "Early jump buffering: <cy>{}</c>\nBuffer window: <cg>{} frames</c>\n\nChange these in Geode: Mods > Nico Buffer > Settings.",
                enabled ? "ON" : "OFF", frames
            ),
            "OK"
        )->show();
    }
};
