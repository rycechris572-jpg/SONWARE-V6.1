
#pragma once
#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Windows.h>
#include <algorithm>
#include <string>
#include <array>

using namespace geode::prelude;

// Implemented in main.cpp. The simulation and F6 hotkey are unchanged.
void sonwareCycleStage();
std::string sonwareStageStatus();

class SonwareGUI final : public cocos2d::CCLayerColor {
    using Color = cocos2d::ccColor3B;
    using RGBA = cocos2d::ccColor4B;
    cocos2d::CCLayerColor* m_window = nullptr;
    cocos2d::CCLayerColor* m_content = nullptr;
    cocos2d::CCLayerColor* m_activeStripe = nullptr;
    cocos2d::CCLabelBMFont* m_title = nullptr;
    cocos2d::CCLabelBMFont* m_body = nullptr;
    cocos2d::CCLabelBMFont* m_status = nullptr;
    cocos2d::CCLabelBMFont* m_themeName = nullptr;
    cocos2d::CCMenu* m_menu = nullptr;
    std::array<cocos2d::CCLabelBMFont*, 6> m_tabLabels{};
    int m_tab = 0;
    int m_theme = 0;
    bool m_dragging = false;
    bool m_themeDirty = false;
    bool m_effectsEnabled = true;
    cocos2d::CCPoint m_lastTouch{};
    float m_width = 0.f, m_height = 0.f;

    static constexpr const char* kTabs[6] = {
        "OVERVIEW", "STAGES", "ASSIST", "PERFORMANCE", "LOGS", "SETTINGS"
    };
    static constexpr const char* kThemes[4] = {
        "TOKYO / VIOLET", "ULTRAVIOLET", "CYAN PULSE", "FROST"
    };

    Color accent() const {
        switch (m_theme) {
            case 1: return {178, 108, 255};
            case 2: return {58, 221, 245};
            case 3: return {139, 207, 255};
            default: return {235, 67, 114};
        }
    }
    Color secondary() const {
        switch (m_theme) {
            case 1: return {235, 100, 208};
            case 2: return {130, 115, 255};
            case 3: return {180, 146, 255};
            default: return {159, 105, 255};
        }
    }
    static RGBA rgba(Color c, GLubyte a = 255) {
        return {c.r, c.g, c.b, a};
    }
    static cocos2d::CCLayerColor* plate(RGBA c, float w, float h,
                                         cocos2d::CCPoint p, cocos2d::CCNode* parent, int z = 0) {
        auto* n = cocos2d::CCLayerColor::create(c, w, h);
        if (!n) return nullptr;
        n->setPosition(p);
        parent->addChild(n, z);
        return n;
    }
    static cocos2d::CCLabelBMFont* txt(const char* s, float scale,
                                       cocos2d::CCPoint p, cocos2d::CCNode* parent,
                                       Color color = {236, 237, 255}) {
        auto* n = cocos2d::CCLabelBMFont::create(s, "bigFont.fnt");
        if (!n) return nullptr;
        n->setScale(scale);
        n->setColor(color);
        n->setPosition(p);
        parent->addChild(n);
        return n;
    }
    cocos2d::CCMenuItemLabel* action(const char* text, float scale,
                                      cocos2d::CCPoint p, SEL_MenuHandler callback, int tag = 0) {
        auto* label = cocos2d::CCLabelBMFont::create(text, "bigFont.fnt");
        label->setScale(scale);
        auto* item = cocos2d::CCMenuItemLabel::create(label, this, callback);
        item->setPosition(p);
        item->setTag(tag);
        m_menu->addChild(item);
        return item;
    }
    void refresh() {
        if (!m_window || !m_body) return;
        m_title->setString(kTabs[m_tab]);
        m_themeName->setString(kThemes[m_theme]);
        // Tab indicator is animated by onTab, not snapped on refresh.
        for (int i = 0; i < 6; ++i) {
            if (m_tabLabels[i]) m_tabLabels[i]->setColor(
                i == m_tab ? Color{255, 255, 255} : Color{156, 154, 187});
        }
        auto stage = sonwareStageStatus();
        m_status->setString(stage.c_str());
        switch (m_tab) {
            case 0:
                m_body->setString("SONWARE  /  V6.3\n\nPREDICTIVE BUFFER: CONNECTED\nSTAGE ENGINE: READY\n\nChoose STAGES to cycle presets.\nF6 remains available in-game.");
                break;
            case 1:
                m_body->setString("BUFFER ENGINE\n\nUse NEXT STAGE below to cycle\nthrough your existing presets.\n\nThe F6 shortcut is unchanged.");
                break;
            case 2:
                m_body->setString("ASSIST MODULES\n\nShip Straight Fly: planned\nLegit Wave: planned\n\nNo experimental assist is enabled.");
                break;
            case 3:
                m_body->setString("PERFORMANCE\n\nAnimated neon / floating motes\nNo per-frame text rebuilding\n\nEngine performance presets are\nmanaged by your existing code.");
                break;
            case 4:
                m_body->setString("SESSION LOGS\n\nDetailed logging UI is planned.\n\nThis panel does not collect\nor fabricate gameplay events.");
                break;
            default:
                m_body->setString("APPEARANCE\n\nPress THEME to cycle palettes:\nTokyo / Violet / Cyan / Frost\n\nRight Alt closes this window.");
                break;
        }
    }
    void onTab(cocos2d::CCObject* sender) {
        m_tab = std::clamp(sender->getTag(), 0, 5);
        refresh();
        if (m_title) {
            m_title->stopAllActions();
            m_title->setScale(.27f);
            m_title->runAction(cocos2d::CCEaseBackOut::create(
                cocos2d::CCScaleTo::create(.33f, .38f)));
        }
        if (m_activeStripe) {
            m_activeStripe->stopAllActions();
            m_activeStripe->runAction(cocos2d::CCEaseBackOut::create(
                cocos2d::CCMoveTo::create(.3f, {4.f, m_height - 96.f - 25.f * m_tab})));
        }
    }
    void onStage(cocos2d::CCObject*) { sonwareCycleStage(); refresh(); }
    void onTheme(cocos2d::CCObject*) {
        m_theme = (m_theme + 1) % 4;
        // Defer rebuilding until the menu callback has returned.
        m_themeDirty = true;
    }
    void onClose(cocos2d::CCObject*) { removeFromParentAndCleanup(true); }

    void ambientEffects(float side) {
        // GPU-friendly drifting motes. Uses Cocos actions,
        // no textures, timers, per-frame allocations or external resources.
        auto a = accent(), b = secondary();
        for (int i = 0; i < 18; ++i) {
            float x = side + 12.f + float((i * 37) % int(std::max(14.f, m_width - side - 25.f)));
            float y = 61.f + float((i * 29) % int(std::max(14.f, m_height - 122.f)));
            float size = i % 4 == 0 ? 3.f : 2.f;
            auto* mote = plate(rgba(i % 2 ? a : b, 100), size, size, {x, y}, m_window, 2);
            if (!mote) continue;
            float duration = 1.2f + .17f * float(i % 5);
            mote->runAction(cocos2d::CCRepeatForever::create(
                cocos2d::CCSequence::create(
                    cocos2d::CCMoveBy::create(duration, {4.f, 7.f}),
                    cocos2d::CCMoveBy::create(duration, {-4.f, -7.f}), nullptr)));
            mote->runAction(cocos2d::CCRepeatForever::create(
                cocos2d::CCSequence::create(
                    cocos2d::CCFadeTo::create(duration, 35),
                    cocos2d::CCFadeTo::create(duration, 165), nullptr)));
        }
        auto* glint = plate(rgba(b, 215), 30.f, 2.f,
                            {side, m_height - 5.f}, m_window, 4);
        if (glint) glint->runAction(cocos2d::CCRepeatForever::create(
            cocos2d::CCSequence::create(
                cocos2d::CCMoveTo::create(2.6f, {m_width - 32.f, m_height - 5.f}),
                cocos2d::CCMoveTo::create(2.6f, {side, m_height - 5.f}), nullptr)));
        auto* halo = plate(rgba(a, 35), m_width - side - 30.f, 5.f,
                           {side + 15.f, m_height - 10.f}, m_window, 2);
        if (halo) halo->runAction(cocos2d::CCRepeatForever::create(
            cocos2d::CCSequence::create(
                cocos2d::CCFadeTo::create(1.5f, 100),
                cocos2d::CCFadeTo::create(1.5f, 20), nullptr)));
    }

    void buildWindow() {
        auto win = cocos2d::CCDirector::sharedDirector()->getWinSize();
        m_width = std::min(468.f, win.width - 16.f);
        m_height = std::min(282.f, win.height - 16.f);
        float side = std::min(130.f, m_width * .31f);
        float rightW = m_width - side;
        auto a = accent(), b = secondary();
        m_window = plate({18, 17, 34, 252}, m_width, m_height,
            {(win.width - m_width) * .5f, (win.height - m_height) * .5f}, this, 5);
        // Depth plates, two-tone header, rail and small accent details.
        plate({7, 5, 18, 185}, m_width, m_height, {4, -4}, m_window, -2);
        plate({26, 21, 49, 255}, side, m_height, {0, 0}, m_window);
        plate({13, 12, 29, 255}, rightW, m_height, {side, 0}, m_window);
        plate(rgba(a), m_width * .56f, 3, {0, m_height - 3}, m_window, 2);
        plate(rgba(b), m_width * .44f, 3, {m_width * .56f, m_height - 3}, m_window, 2);
        plate({40, 32, 70, 255}, 1, m_height - 4, {side - 1, 2}, m_window, 2);
        plate(rgba(b, 210), 3, m_height - 18, {side - 3, 9}, m_window, 2);
        plate({38, 31, 68, 255}, rightW - 18, 1, {side + 9, m_height - 57}, m_window, 2);
        plate({27, 24, 51, 255}, rightW - 20, m_height - 127,
              {side + 10, 64}, m_window);
        plate(rgba(b, 220), 3, m_height - 127, {side + 10, 64}, m_window, 2);
        plate({34, 28, 58, 255}, rightW - 20, 43, {side + 10, 10}, m_window);
        plate(rgba(a), rightW - 20, 2, {side + 10, 10}, m_window, 2);
        txt("SONWARE", .53f, {side * .5f, m_height - 22}, m_window, {255, 248, 255});
        txt("V6.3  /  HYBRID", .25f, {side * .5f, m_height - 42}, m_window, b);
        txt("CONTROL DECK", .25f, {side + rightW * .5f, m_height - 18}, m_window, a);
        m_themeName = txt(kThemes[m_theme], .22f,
                          {side + rightW * .5f, m_height - 40}, m_window, b);
        m_title = txt("OVERVIEW", .38f, {side + rightW * .5f, m_height - 74}, m_window);
        m_body = txt("", .245f, {side + rightW * .5f, (m_height + 1) * .47f}, m_window);
        m_status = txt("", .205f, {side + rightW * .5f, 42}, m_window, {210, 205, 249});
        if (m_effectsEnabled) ambientEffects(side);
        m_menu = cocos2d::CCMenu::create();
        m_menu->setPosition({0, 0});
        m_window->addChild(m_menu, 10);
        m_menu->setTouchPriority(-132);
        m_activeStripe = plate(rgba(a), 3, 17, {4, m_height - 96}, m_window, 3);
        for (int i = 0; i < 6; ++i) {
            auto* item = action(kTabs[i], .275f,
                {side * .5f, m_height - 88.f - 25.f * i},
                menu_selector(SonwareGUI::onTab), i);
            m_tabLabels[i] = static_cast<cocos2d::CCLabelBMFont*>(item->getLabel());
        }
        action("X", .42f, {m_width - 13, m_height - 15},
               menu_selector(SonwareGUI::onClose));
        action("NEXT STAGE", .26f, {side + rightW * .34f, 23},
               menu_selector(SonwareGUI::onStage));
        action("THEME", .26f, {side + rightW * .79f, 23},
               menu_selector(SonwareGUI::onTheme));
    }

public:
    static SonwareGUI* create() {
        auto* p = new SonwareGUI();
        if (p && p->init()) { p->autorelease(); return p; }
        CC_SAFE_DELETE(p);
        return nullptr;
    }
    bool init() override {
        if (!CCLayerColor::initWithColor({0, 0, 0, 180})) return false;
        setID("sonware-v63-modal");
        setTouchEnabled(true);
        setTouchMode(cocos2d::kCCTouchesOneByOne);
        setTouchPriority(-130);
        setKeypadEnabled(true);
        buildWindow();
        refresh();
        // Elastic entrance, anchored at the centre of the window.
        m_window->setScale(.76f);
        m_window->runAction(cocos2d::CCEaseBackOut::create(
            cocos2d::CCScaleTo::create(.55f, 1.f)));
        scheduleUpdate();
        return true;
    }
    void update(float) override {
        // No UI rebuilding per frame; only a key edge check.
        if (GetAsyncKeyState(VK_RMENU) & 1) {
            removeFromParentAndCleanup(true);
            return;
        }
        if (m_themeDirty) {
            m_themeDirty = false;
            auto pos = m_window->getPosition();
            m_window->removeFromParentAndCleanup(true);
            m_window = nullptr;
            buildWindow();
            m_window->setPosition(pos);
            refresh();
            m_window->setScale(.92f);
            m_window->runAction(cocos2d::CCEaseBackOut::create(
                cocos2d::CCScaleTo::create(.32f, 1.f)));
        }
    }
    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent*) override {
        auto pt = convertToNodeSpace(touch->getLocation());
        auto pos = m_window->getPosition();
        m_dragging = pt.x >= pos.x && pt.x <= pos.x + m_width &&
                     pt.y >= pos.y + m_height - 54 && pt.y <= pos.y + m_height;
        m_lastTouch = pt;
        return true; // modal: swallow all underlying touches
    }
    void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent*) override {
        if (!m_dragging) return;
        auto pt = convertToNodeSpace(touch->getLocation());
        auto pos = m_window->getPosition();
        auto win = cocos2d::CCDirector::sharedDirector()->getWinSize();
        auto nx = std::clamp(pos.x + pt.x - m_lastTouch.x, 0.f, std::max(0.f, win.width - m_width));
        auto ny = std::clamp(pos.y + pt.y - m_lastTouch.y, 0.f, std::max(0.f, win.height - m_height));
        m_window->setPosition({nx, ny});
        m_lastTouch = pt;
    }
    void ccTouchEnded(cocos2d::CCTouch*, cocos2d::CCEvent*) override { m_dragging = false; }
    void keyBackClicked() override { removeFromParentAndCleanup(true); }
};

class $modify(SonwarePauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto win = cocos2d::CCDirector::sharedDirector()->getWinSize();
        auto* menu = cocos2d::CCMenu::create();
        menu->setPosition({0, 0});
        addChild(menu, 100);
        auto* bg = cocos2d::CCLayerColor::create({30, 21, 49, 240}, 110, 24);
        bg->setPosition({win.width * .5f - 55.f, 13});
        addChild(bg, 99);
        auto* stripe = cocos2d::CCLayerColor::create({235, 67, 114, 255}, 110, 2);
        stripe->setPosition({win.width * .5f - 55.f, 13});
        addChild(stripe, 100);
        auto* text = cocos2d::CCLabelBMFont::create("SONWARE V6.3", "bigFont.fnt");
        text->setScale(.32f);
        text->setColor({232, 195, 255});
        auto* item = cocos2d::CCMenuItemLabel::create(text, this,
            menu_selector(SonwarePauseLayer::openSonware));
        item->setPosition({win.width * .5f, 25});
        menu->addChild(item);
    }
    void openSonware(cocos2d::CCObject*) {
        if (getChildByID("sonware-v63-modal")) return;
        if (auto* gui = SonwareGUI::create()) addChild(gui, 1000);
    }
};
