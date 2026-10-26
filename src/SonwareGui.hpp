#pragma once
#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Windows.h>
#include <algorithm>
#include <string>

using namespace geode::prelude;

void sonwareCycleStage();
std::string sonwareStageStatus();

// SONWARE V6.25: pause-only modal. No global keyboard polling or scene persistence.
class SonwareGUI final : public cocos2d::CCLayerColor {
    cocos2d::CCMenu* m_buttons = nullptr;
    cocos2d::CCLayerColor* m_panel = nullptr;
    cocos2d::CCLabelBMFont* m_heading = nullptr;
    cocos2d::CCLabelBMFont* m_body = nullptr;
    int m_tab = 0;
    int m_theme = 1; // Tokyo Night by default
    cocos2d::CCLayerColor* m_accentBar = nullptr;
    cocos2d::CCLayerColor* m_footer = nullptr;
    bool m_dragging = false;
    cocos2d::CCPoint m_lastTouch;

    static constexpr const char* tabs[6] = {
        "DASHBOARD", "STAGES", "ASSIST", "OPTIMISATION", "LOGS", "SETTINGS"
    };
    cocos2d::ccColor3B accent() const {
        switch (m_theme) {
            case 1: return {230, 57, 70};   // Tokyo Night crimson
            case 2: return {179, 110, 255}; // ultraviolet
            case 3: return {87, 240, 163};  // mint
            default: return {53, 232, 255}; // cyan neon
        }
    }
    cocos2d::ccColor3B accentSoft() const {
        switch (m_theme) {
            case 1: return {255, 183, 197};
            case 2: return {220, 190, 255};
            case 3: return {180, 255, 215};
            default: return {160, 250, 255};
        }
    }
    void applyTheme() {
        if (!m_panel) return;
        m_panel->setColor(m_theme == 1 ? cocos2d::ccColor3B{25, 17, 29} : cocos2d::ccColor3B{18, 21, 30});
        if (m_accentBar) m_accentBar->setColor(accent());
        if (m_footer) m_footer->setColor(accent());
        if (m_heading) m_heading->setColor(accentSoft());
        if (m_body) m_body->setColor({238, 239, 250});
        if (m_buttons) {
            for (unsigned int i = 0; i < m_buttons->getChildrenCount(); ++i) {
                auto* node = static_cast<cocos2d::CCNode*>(m_buttons->getChildren()->objectAtIndex(i));
                if (auto* item = typeinfo_cast<cocos2d::CCMenuItemLabel*>(node)) {
                    if (auto* lbl = typeinfo_cast<cocos2d::CCLabelBMFont*>(item->getLabel())) lbl->setColor(accentSoft());
                }
            }
        }
    }
    cocos2d::CCLabelBMFont* label(const char* text, float scale, cocos2d::CCPoint pos) {
        auto* l = cocos2d::CCLabelBMFont::create(text, "bigFont.fnt");
        l->setScale(scale);
        l->setPosition(pos);
        return l;
    }
    void refresh() {
        m_heading->setString(tabs[m_tab]);
        m_heading->setColor(accentSoft());
        switch (m_tab) {
            case 0: m_body->setString("SONWARE V6.25 / ACTIVE\n\nPause-screen control deck.\nUse STAGES to change buffer.\nRight Alt closes this window."); break;
            case 1: { auto info = sonwareStageStatus(); m_body->setString(info.c_str()); break; }
            case 2: m_body->setString("ASSIST\n\nShip Straight Fly: not wired yet\nLegit Wave: not wired yet"); break;
            case 3: m_body->setString("OPTIMISATION\n\nControls will be connected after testing."); break;
            case 4: m_body->setString("LOGS\n\nGUI opened from Pause.\nNo automatic scene reopening."); break;
            case 5: m_body->setString("SETTINGS\n\nTheme button cycles:\nCyan / Japan / Violet / Mint"); break;
        }
    }
    void onTab(cocos2d::CCObject* sender) {
        m_tab = sender->getTag();
        refresh();
    }
    void onStage(cocos2d::CCObject*) { sonwareCycleStage(); refresh(); }
    void onTheme(cocos2d::CCObject*) {
        m_theme = (m_theme + 1) % 4;
        applyTheme();
        refresh();
    }
    void onClose(cocos2d::CCObject*) { removeFromParentAndCleanup(true); }

public:
    static SonwareGUI* create() {
        auto* p = new SonwareGUI();
        if (p && p->init()) { p->autorelease(); return p; }
        CC_SAFE_DELETE(p);
        return nullptr;
    }
    bool init() override {
        if (!CCLayerColor::initWithColor({0, 0, 0, 165})) return false;
        setID("sonware-v625-modal");
        setTouchEnabled(true);
        setTouchMode(cocos2d::kCCTouchesOneByOne);
        setTouchPriority(-129); // swallow before underlying pause/game input
        setKeypadEnabled(true);
        auto win = cocos2d::CCDirector::sharedDirector()->getWinSize();
        auto width = std::min(440.f, win.width - 20.f);
        auto height = std::min(260.f, win.height - 20.f);
        m_panel = cocos2d::CCLayerColor::create({25, 17, 29, 252}, width, height);
        m_panel->setContentSize({width, height});
        m_panel->setPosition({(win.width - width) / 2, (win.height - height) / 2});
        addChild(m_panel, 2);
        // Layered offset plates give the deck a raised, bevel-like silhouette.
        auto* shadow = cocos2d::CCLayerColor::create({3, 4, 10, 220}, width, height);
        shadow->setPosition({5, -5});
        m_panel->addChild(shadow, -2);
        auto* rail = cocos2d::CCLayerColor::create({14, 12, 25, 255}, 135, height);
        m_panel->addChild(rail, -1);
        m_accentBar = cocos2d::CCLayerColor::create(accent(), 5, height);
        m_accentBar->setPosition({0, 0});
        m_panel->addChild(m_accentBar, 3);
        m_footer = cocos2d::CCLayerColor::create(accent(), width - 135, 3);
        m_footer->setPosition({135, 0});
        m_panel->addChild(m_footer, 3);
        auto* brand = label("SONWARE", .55f, {67, height - 20});
        brand->setColor(accentSoft());
        m_panel->addChild(brand);
        auto* version = label("V6.25", .3f, {67, height - 40});
        m_panel->addChild(version);
        m_buttons = cocos2d::CCMenu::create();
        m_buttons->setPosition({0, 0});
        m_panel->addChild(m_buttons);
        m_buttons->setTouchPriority(-130); // buttons receive clicks before overlay
        for (int i = 0; i < 6; ++i) {
            auto* text = label(tabs[i], .34f, {0, 0});
            auto* item = cocos2d::CCMenuItemLabel::create(text, this, menu_selector(SonwareGUI::onTab));
            item->setTag(i);
            item->setPosition({67, height - 65.f - i * 28.f});
            m_buttons->addChild(item);
        }
        m_heading = label("", .55f, {width * .62f, height - 26});
        m_panel->addChild(m_heading);
        m_body = label("", .32f, {width * .66f, height * .48f});
        m_body->setAlignment(cocos2d::kCCTextAlignmentCenter);
        m_body->setColor({238, 239, 250});
        m_panel->addChild(m_body);
        auto* close = cocos2d::CCMenuItemLabel::create(label("X", .55f, {0, 0}), this, menu_selector(SonwareGUI::onClose));
        close->setPosition({width - 16, height - 17});
        m_buttons->addChild(close);
        auto* theme = cocos2d::CCMenuItemLabel::create(label("THEME", .38f, {0, 0}), this, menu_selector(SonwareGUI::onTheme));
        theme->setPosition({width - 48, 18});
        m_buttons->addChild(theme);
        auto* stage = cocos2d::CCMenuItemLabel::create(
            label("NEXT STAGE", .34f, {0, 0}), this,
            menu_selector(SonwareGUI::onStage));
        stage->setPosition({width - 75, 43});
        m_buttons->addChild(stage);
        scheduleUpdate();
        applyTheme();
        refresh();
        return true;
    }
    void update(float) override {
        // Only close on Right Alt, never reopen automatically.
        if (GetAsyncKeyState(VK_RMENU) & 1) {
            removeFromParentAndCleanup(true);
            return;
        }
    }
    // Swallow touches so clicks cannot fall through to gameplay or Pause controls.
    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent*) override {
        auto pt = convertToNodeSpace(touch->getLocation());
        auto pos = m_panel->getPosition();
        auto size = m_panel->getContentSize();
        m_dragging = pt.x >= pos.x && pt.x <= pos.x + size.width &&
                     pt.y >= pos.y + size.height - 55 && pt.y <= pos.y + size.height;
        m_lastTouch = pt;
        return true;
    }
    void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent*) override {
        if (!m_dragging) return;
        auto pt = convertToNodeSpace(touch->getLocation());
        auto pos = m_panel->getPosition();
        m_panel->setPosition({pos.x + pt.x - m_lastTouch.x, pos.y + pt.y - m_lastTouch.y});
        m_lastTouch = pt;
    }
    void ccTouchEnded(cocos2d::CCTouch*, cocos2d::CCEvent*) override { m_dragging = false; }
    void keyBackClicked() override { removeFromParentAndCleanup(true); }
};

// Attach a button to the pause layer. Closing destroys the modal; it cannot
// automatically reappear when a different screen or scene opens.
class $modify(SonwarePauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto* menu = cocos2d::CCMenu::create();
        menu->setPosition({0, 0});
        addChild(menu, 100);
        auto win = cocos2d::CCDirector::sharedDirector()->getWinSize();
        auto* backing = cocos2d::CCLayerColor::create({31, 18, 35, 235}, 100, 25);
        backing->setPosition({0, 0});
        menu->addChild(backing, -1);
        auto* edge = cocos2d::CCLayerColor::create({230, 57, 70, 255}, 100, 2);
        edge->setPosition({0, 0});
        menu->addChild(edge, 1);
        auto* text = cocos2d::CCLabelBMFont::create("SONWARE V6.25", "bigFont.fnt");
        text->setScale(.40f);
        text->setColor({255, 183, 197});
        auto* item = cocos2d::CCMenuItemLabel::create(text, this, menu_selector(SonwarePauseLayer::openSonware));
        item->setPosition({win.width / 2, 28.f});
        menu->addChild(item);
    }
    void openSonware(cocos2d::CCObject*) {
        if (getChildByID("sonware-v625-modal")) return;
        if (auto* gui = SonwareGUI::create()) addChild(gui, 1000);
    }
};

