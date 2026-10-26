#pragma once
#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Windows.h>
#include <algorithm>
#include <string>
#include <array>

using namespace geode::prelude;

// Keep the existing engine and F6 implementation in main.cpp.
void sonwareCycleStage();
std::string sonwareStageStatus();

class SonwareGUI final : public cocos2d::CCLayerColor {
    using C = cocos2d::ccColor3B;
    using A = cocos2d::ccColor4B;

    cocos2d::CCLayerColor* m_window = nullptr;
    cocos2d::CCLayerColor* m_activeStripe = nullptr;
    cocos2d::CCLabelBMFont* m_title = nullptr;
    cocos2d::CCLabelBMFont* m_body = nullptr;
    cocos2d::CCLabelBMFont* m_status = nullptr;
    cocos2d::CCLabelBMFont* m_themeName = nullptr;
    cocos2d::CCMenu* m_menu = nullptr;
    std::array<cocos2d::CCLabelBMFont*, 6> m_tabLabels{};
    int m_tab = 0, m_theme = 0;
    bool m_dragging = false, m_themeDirty = false;
    cocos2d::CCPoint m_lastTouch{};
    float m_width = 0.f, m_height = 0.f, m_side = 0.f, m_contentW = 0.f;

    static constexpr const char* kTabs[6] = {
        "OVERVIEW", "STAGES", "ASSIST", "PERFORMANCE", "LOGS", "SETTINGS"
    };
    static constexpr const char* kThemes[4] = {
        "TOKYO / VIOLET", "ULTRAVIOLET", "CYAN PULSE", "FROST"
    };

    C accent() const {
        switch (m_theme) {
            case 1: return {188, 80, 255};
            case 2: return {46, 220, 255};
            case 3: return {140, 198, 255};
            default: return {230, 55, 255};
        }
    }
    C secondary() const {
        switch (m_theme) {
            case 1: return {255, 85, 195};
            case 2: return {155, 100, 255};
            case 3: return {190, 155, 255};
            default: return {45, 160, 255};
        }
    }
    static A rgba(C c, GLubyte alpha = 255) { return {c.r, c.g, c.b, alpha}; }

    static cocos2d::CCLayerColor* plate(
        A color, float w, float h, cocos2d::CCPoint pos,
        cocos2d::CCNode* parent, int z = 0
    ) {
        auto* n = cocos2d::CCLayerColor::create(color, w, h);
        if (!n) return nullptr;
        n->setPosition(pos);
        parent->addChild(n, z);
        return n;
    }
    static cocos2d::CCLabelBMFont* txt(
        const char* text, float scale, cocos2d::CCPoint pos,
        cocos2d::CCNode* parent, C color = {239, 238, 255}
    ) {
        auto* n = cocos2d::CCLabelBMFont::create(text, "bigFont.fnt");
        if (!n) return nullptr;
        n->setScale(scale);
        n->setColor(color);
        n->setPosition(pos);
        parent->addChild(n);
        return n;
    }
    cocos2d::CCMenuItemLabel* action(
        const char* text, float scale, cocos2d::CCPoint pos,
        SEL_MenuHandler callback, int tag = 0
    ) {
        auto* label = cocos2d::CCLabelBMFont::create(text, "bigFont.fnt");
        if (!label) return nullptr;
        std::string s = text ? text : "";
        if (s == "PERFORMANCE") scale = std::min(scale, .205f);
        else if (s == "OVERVIEW" || s == "SETTINGS") scale = std::min(scale, .255f);
        label->setScale(scale);
        auto* item = cocos2d::CCMenuItemLabel::create(label, this, callback);
        if (!item) return nullptr;
        item->setPosition(pos);
        item->setTag(tag);
        m_menu->addChild(item);
        return item;
    }

    void refresh() {
        if (!m_window || !m_body || !m_title || !m_status || !m_themeName) return;
        m_title->setString(kTabs[m_tab]);
        m_themeName->setString(kThemes[m_theme]);
        for (int i = 0; i < 6; ++i)
            if (m_tabLabels[i]) m_tabLabels[i]->setColor(
                i == m_tab ? C{255,255,255} : C{157,153,190});

        auto stage = sonwareStageStatus();
        m_status->setString(stage.c_str());

        switch (m_tab) {
            case 0: m_body->setString(
                "PREDICTIVE BUFFER  /  SYSTEM OVERVIEW\n\n"
                "ENGINE STATUS       CONNECTED\n"
                "STAGE ENGINE        READY\n"
                "MAX TICK LIMIT      60\n\n"
                "Choose STAGES to cycle existing presets.\n"
                "Your F6 hotkey and engine logic are unchanged."); break;
            case 1: m_body->setString(
                "STAGE CONTROL\n\nUse NEXT STAGE to cycle through your\n"
                "existing presets, or continue using F6.\n\nCURRENT STAGE STATUS:"); break;
            case 2: m_body->setString(
                "ASSIST MODULES\n\nShip Straight Fly     PLANNED\n"
                "Legit Wave            PLANNED\n\nNo experimental assist is enabled."); break;
            case 3: m_body->setString(
                "PERFORMANCE CONTROL\n\nUI effects             ACTIVE\n"
                "Animation system       ACTIVE\nMaximum tick setting   60\n\n"
                "Engine presets remain managed by your existing code."); break;
            case 4: m_body->setString(
                "SESSION LOGS\n\nThe log viewer is not implemented yet.\n\n"
                "This panel does not invent gameplay events."); break;
            default: m_body->setString(
                "CONFIGURATION\n\nTHEME cycles the color palettes.\n"
                "Drag the top bar to move this window.\n"
                "Press Right Alt or X to close."); break;
        }
    }
    void onTab(cocos2d::CCObject* sender) {
        m_tab = std::clamp(sender->getTag(), 0, 5);
        refresh();
        if (m_title) {
            m_title->stopAllActions();
            m_title->setScale(.30f);
            m_title->runAction(cocos2d::CCEaseBackOut::create(
                cocos2d::CCScaleTo::create(.28f, .38f)));
        }
        if (m_activeStripe) {
            float gap = std::min(49.f, (m_height - 150.f) / 6.f);
            m_activeStripe->stopAllActions();
            m_activeStripe->runAction(cocos2d::CCEaseBackOut::create(
                cocos2d::CCMoveTo::create(.25f, {9.f, m_height - 104.f - gap*m_tab})));
        }
    }
    void onStage(cocos2d::CCObject*) { sonwareCycleStage(); refresh(); }
    void onTheme(cocos2d::CCObject*) { m_theme = (m_theme + 1) % 4; m_themeDirty = true; }
    void onClose(cocos2d::CCObject*) { removeFromParentAndCleanup(true); }

    void ambientEffects(float side) {
        auto a = accent(), b = secondary();
        int xr = std::max(14, int(m_width - side - 28.f));
        int yr = std::max(14, int(m_height - 100.f));
        for (int i = 0; i < 20; ++i) {
            float x = side + 12.f + float((i*37) % xr);
            float y = 50.f + float((i*29) % yr);
            float size = i % 4 == 0 ? 3.f : 2.f;
            auto* mote = plate(rgba(i%2 ? a : b, 110), size, size, {x,y}, m_window, 2);
            if (!mote) continue;
            float d = 1.25f + .16f*float(i%5);
            mote->runAction(cocos2d::CCRepeatForever::create(cocos2d::CCSequence::create(
                cocos2d::CCMoveBy::create(d,{4.f,7.f}),
                cocos2d::CCMoveBy::create(d,{-4.f,-7.f}), nullptr)));
            mote->runAction(cocos2d::CCRepeatForever::create(cocos2d::CCSequence::create(
                cocos2d::CCFadeTo::create(d,35),
                cocos2d::CCFadeTo::create(d,160), nullptr)));
        }
        auto* glint = plate(rgba(b,220), 34.f, 2.f, {side,m_height-5.f}, m_window, 5);
        if (glint) glint->runAction(cocos2d::CCRepeatForever::create(cocos2d::CCSequence::create(
            cocos2d::CCMoveTo::create(2.8f,{m_width-38.f,m_height-5.f}),
            cocos2d::CCMoveTo::create(2.8f,{side,m_height-5.f}), nullptr)));
        auto* halo = plate(rgba(a,30), m_width-side-24.f, 4.f,
                           {side+12.f,m_height-11.f}, m_window, 2);
        if (halo) halo->runAction(cocos2d::CCRepeatForever::create(cocos2d::CCSequence::create(
            cocos2d::CCFadeTo::create(1.4f,95),
            cocos2d::CCFadeTo::create(1.4f,20), nullptr)));
    }

    void buildWindow() {
        auto win = cocos2d::CCDirector::sharedDirector()->getWinSize();
        // Mockup-sized dashboard, constrained to the actual game resolution.
        m_width = std::min(900.f, std::max(320.f, win.width - 28.f));
        m_height = std::min(520.f, std::max(240.f, win.height - 28.f));
        m_side = std::min(190.f, m_width * .25f);
        m_contentW = m_width - m_side;
        auto a = accent(), b = secondary();

        m_window = plate({7,5,18,248}, m_width, m_height,
            {(win.width-m_width)*.5f,(win.height-m_height)*.5f}, this, 5);
        if (!m_window) return;

        // Multi-layer frame gives the panel a bevel/depth effect.
        plate({0,0,0,150}, m_width,m_height,{5.f,-5.f},m_window,-3);
        plate({16,10,34,255},m_width-8.f,m_height-8.f,{4.f,4.f},m_window,-2);
        plate({3,3,12,255},m_width-16.f,m_height-16.f,{8.f,8.f},m_window,-1);
        plate(rgba(a,235),m_width-8.f,3.f,{4.f,m_height-6.f},m_window,4);
        plate(rgba(b,225),3.f,m_height-12.f,{m_width-6.f,6.f},m_window,4);
        plate({25,16,48,255},m_side,m_height-12.f,{6.f,6.f},m_window);
        plate({5,7,20,255},m_contentW-12.f,m_height-12.f,{m_side+6.f,6.f},m_window);
        plate(rgba(b,220),3.f,m_height-24.f,{m_side+4.f,12.f},m_window,3);

        // Header strip and brand.
        plate({10,7,25,245},m_width-18.f,66.f,{9.f,m_height-76.f},m_window,1);
        plate(rgba(a,235),m_width*.52f,2.f,{10.f,m_height-12.f},m_window,3);
        plate(rgba(b,220),m_width*.48f,2.f,{m_width*.52f,m_height-12.f},m_window,3);
        txt("SONWARE",.58f,{m_side*.5f+4.f,m_height-31.f},m_window,{255,244,255});
        txt("V6.4 / TOKYO BLACKOUT",.235f,{m_side*.5f+4.f,m_height-53.f},m_window,b);
        txt("SYSTEM CONTROL DECK",.27f,{m_side+m_contentW*.5f,m_height-28.f},m_window,a);
        txt("PREDICTIVE BUFFER  //  GEODE",.20f,{m_side+m_contentW*.5f,m_height-48.f},m_window,{180,190,235});

        // Keymaster sidebar footer.
        plate({8,6,22,245},m_side-18.f,66.f,{15.f,12.f},m_window,1);
        plate(rgba(a,170),m_side-30.f,2.f,{21.f,76.f},m_window,3);
        txt("KEYMASTER",.25f,{m_side*.5f,56.f},m_window,{220,160,255});
        txt("CONTROL THE GAME",.18f,{m_side*.5f,38.f},m_window,{172,160,205});
        txt("TOKYO BLACKOUT",.16f,{m_side*.5f,24.f},m_window,{120,190,255});

        // Main dashboard cards.
        float cx = m_side+18.f, cw = m_contentW-36.f;
        float top = m_height-91.f;
        float ch = std::max(92.f,m_height*.43f);
        plate({4,6,19,245},cw,ch,{cx,top-ch},m_window,1);
        plate(rgba(a,220),cw-8.f,2.f,{cx+4.f,top-4.f},m_window,3);
        plate(rgba(b,180),2.f,ch-10.f,{cx+cw-5.f,top-ch+5.f},m_window,3);
        float ly = 18.f, lh = std::max(42.f,top-ch-18.f-ly);
        plate({10,8,28,245},cw,lh,{cx,ly},m_window,1);
        plate(rgba(b,200),cw-8.f,2.f,{cx+4.f,ly+lh-4.f},m_window,3);

        m_title = txt("OVERVIEW",.38f,{cx+cw*.5f,top-24.f},m_window);
        m_body = txt("",.235f,{cx+cw*.5f,top-ch*.55f},m_window,{224,222,247});
        m_status = txt("",.20f,{cx+cw*.5f,ly+19.f},m_window,{190,185,226});
        m_themeName = txt(kThemes[m_theme],.22f,{cx+cw*.5f,ly+lh-17.f},m_window,b);

        ambientEffects(m_side);
        m_menu = cocos2d::CCMenu::create();
        m_menu->setPosition({0,0});
        m_window->addChild(m_menu,10);
        m_menu->setTouchPriority(-132);

        float gap = std::min(49.f,(m_height-150.f)/6.f);
        float tabTop = m_height-104.f;
        m_activeStripe = plate(rgba(a),3.f,23.f,{9.f,tabTop},m_window,4);
        for (int i=0;i<6;++i) {
            auto* item = action(kTabs[i],.275f,{m_side*.5f+5.f,tabTop-gap*i},
                                menu_selector(SonwareGUI::onTab),i);
            if (item) m_tabLabels[i] = static_cast<cocos2d::CCLabelBMFont*>(item->getLabel());
        }
        action("X",.40f,{m_width-20.f,m_height-29.f},menu_selector(SonwareGUI::onClose));
        action("NEXT STAGE",.23f,{m_side+m_contentW*.32f,ly+18.f},menu_selector(SonwareGUI::onStage));
        action("THEME",.23f,{m_side+m_contentW*.78f,ly+18.f},menu_selector(SonwareGUI::onTheme));
    }

public:
    static SonwareGUI* create() {
        auto* p = new SonwareGUI();
        if (p && p->init()) { p->autorelease(); return p; }
        CC_SAFE_DELETE(p);
        return nullptr;
    }

    bool init() override {
        if (!CCLayerColor::initWithColor({0,0,0,185})) return false;
        setID("sonware-v64-modal");
        setTouchEnabled(true);
        setTouchMode(cocos2d::kCCTouchesOneByOne);
        setTouchPriority(-130);
        setKeypadEnabled(true);
        buildWindow();
        refresh();
        if (m_window) {
            m_window->setScale(.88f);
            m_window->runAction(cocos2d::CCEaseBackOut::create(
                cocos2d::CCScaleTo::create(.48f,1.f)));
        }
        scheduleUpdate();
        return true;
    }

    void update(float) override {
        if (GetAsyncKeyState(VK_RMENU)&1) {
            removeFromParentAndCleanup(true);
            return;
        }
        if (m_themeDirty) {
            m_themeDirty = false;
            auto oldPos = m_window ? m_window->getPosition() : cocos2d::CCPointZero;
            if (m_window) m_window->removeFromParentAndCleanup(true);
            m_window = nullptr; m_menu = nullptr; m_title = nullptr; m_body = nullptr;
            m_status = nullptr; m_themeName = nullptr; m_activeStripe = nullptr;
            m_tabLabels.fill(nullptr);
            buildWindow();
            if (m_window) {
                m_window->setPosition(oldPos);
                refresh();
                m_window->setScale(.94f);
                m_window->runAction(cocos2d::CCEaseBackOut::create(
                    cocos2d::CCScaleTo::create(.28f,1.f)));
            }
        }
    }

    bool ccTouchBegan(cocos2d::CCTouch* touch,cocos2d::CCEvent*) override {
        if (!m_window) return false;
        auto p = convertToNodeSpace(touch->getLocation());
        auto pos = m_window->getPosition();
        m_dragging = p.x>=pos.x && p.x<=pos.x+m_width &&
                     p.y>=pos.y+m_height-76.f && p.y<=pos.y+m_height;
        m_lastTouch = p;
        return true;
    }
    void ccTouchMoved(cocos2d::CCTouch* touch,cocos2d::CCEvent*) override {
        if (!m_dragging || !m_window) return;
        auto p = convertToNodeSpace(touch->getLocation());
        auto pos = m_window->getPosition();
        auto win = cocos2d::CCDirector::sharedDirector()->getWinSize();
        float x = std::clamp(pos.x+p.x-m_lastTouch.x,0.f,std::max(0.f,win.width-m_width));
        float y = std::clamp(pos.y+p.y-m_lastTouch.y,0.f,std::max(0.f,win.height-m_height));
        m_window->setPosition({x,y});
        m_lastTouch = p;
    }
    void ccTouchEnded(cocos2d::CCTouch*,cocos2d::CCEvent*) override { m_dragging=false; }
    void keyBackClicked() override { removeFromParentAndCleanup(true); }
};

class $modify(SonwarePauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto win = cocos2d::CCDirector::sharedDirector()->getWinSize();
        auto* menu = cocos2d::CCMenu::create();
        menu->setPosition({0,0});
        addChild(menu,100);
        auto* bg = cocos2d::CCLayerColor::create({15,7,31,245},124.f,27.f);
        bg->setPosition({win.width*.5f-62.f,12.f});
        addChild(bg,99);
        auto* stripe = cocos2d::CCLayerColor::create({230,55,255,255},124.f,2.f);
        stripe->setPosition({win.width*.5f-62.f,12.f});
        addChild(stripe,100);
        auto* label = cocos2d::CCLabelBMFont::create("SONWARE V6.4","bigFont.fnt");
        if (!label) return;
        label->setScale(.32f);
        label->setColor({232,195,255});
        auto* item = cocos2d::CCMenuItemLabel::create(
            label,this,menu_selector(SonwarePauseLayer::openSonware));
        if (!item) return;
        item->setPosition({win.width*.5f,25.f});
        menu->addChild(item);
    }
    void openSonware(cocos2d::CCObject*) {
        if (getChildByID("sonware-v64-modal")) return;
        if (auto* gui = SonwareGUI::create()) addChild(gui,1000);
    }
};

