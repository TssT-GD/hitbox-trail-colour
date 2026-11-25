#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <deque>
using namespace geode::prelude;

namespace {
    enum class Kind { Idle, Held, Press, Release };
    struct Sample { CCRect rect; Kind kind; };
    bool enabled() { return Mod::get()->getSettingValue<bool>("enabled"); }
    size_t limit() { return static_cast<size_t>(Mod::get()->getSettingValue<int64_t>("length")); }
    void append(std::deque<Sample>& q, Sample sample) {
        q.push_back(sample);
        while (q.size() > limit()) q.pop_front();
    }
}

class $modify(InputPlayer, PlayerObject) {
    struct Fields { bool held = false; std::deque<Sample> pending; };
    void capture(Kind kind) {
        auto layer = PlayLayer::get();
        if (!layer || layer->m_player1 != this || m_isDead || !enabled()) return;
        append(m_fields->pending, {getObjectRect(), kind});
    }
    bool pushButton(PlayerButton button) {
        if (static_cast<int>(button) == 1 && !m_fields->held) {
            m_fields->held = true;
            if (Mod::get()->getSettingValue<bool>("extra-samples")) capture(Kind::Press);
        }
        return PlayerObject::pushButton(button);
    }
    bool releaseButton(PlayerButton button) {
        if (static_cast<int>(button) == 1 && m_fields->held) {
            m_fields->held = false;
            if (Mod::get()->getSettingValue<bool>("extra-samples")) capture(Kind::Release);
        }
        return PlayerObject::releaseButton(button);
    }
    void releaseAllButtons() {
        m_fields->held = false;
        PlayerObject::releaseAllButtons();
    }
};

class $modify(InputTrail, PlayLayer) {
    struct Fields {
        CCDrawNode* draw = nullptr;
        std::deque<Sample> samples;
        bool frozen = false;
    };
    bool isDead() {
        return m_playerDied || (m_player1 && m_player1->m_isDead) ||
            (m_player2 && m_player2->m_isDead);
    }
    void resetLevel() {
        auto& f = m_fields;
        f->frozen = false;
        f->samples.clear();
        if (f->draw) { f->draw->setVisible(false); f->draw->clear(); }
        if (m_player1) static_cast<InputPlayer*>(m_player1)->m_fields->pending.clear();
        PlayLayer::resetLevel();
        if (m_player1) static_cast<InputPlayer*>(m_player1)->m_fields->pending.clear();
        f->frozen = false;
        f->samples.clear();
        if (f->draw) { f->draw->setVisible(false); f->draw->clear(); }
    }
    void flushInputs() {
        if (!m_player1) return;
        auto p = static_cast<InputPlayer*>(m_player1);
        for (auto const& s : p->m_fields->pending) append(m_fields->samples, s);
        p->m_fields->pending.clear();
    }
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // Save the collision position before the game changes the player state.
        bool captureDeath = enabled() && !m_fields->frozen && m_player1 && !isDead();
        Sample last{};
        if (captureDeath) {
            auto p = static_cast<InputPlayer*>(m_player1);
            last = {p->getObjectRect(), p->m_fields->held ? Kind::Held : Kind::Idle};
        }
        PlayLayer::destroyPlayer(player, object);
        // Noclip and other mods may suppress the actual death.
        if (captureDeath && isDead() && !m_fields->frozen) {
            flushInputs();
            append(m_fields->samples, last);
            showDeathTrail();
        }
    }
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        auto& f = m_fields;
        if (!m_objectLayer || !m_player1) return;
        auto p = static_cast<InputPlayer*>(m_player1);
        if (!enabled()) {
            f->samples.clear(); p->m_fields->pending.clear(); f->frozen = false;
            if (f->draw) { f->draw->clear(); f->draw->setVisible(false); }
            return;
        }
        if (isDead()) {
            // Fallback if death was not observed by our destroyPlayer hook.
            if (!f->frozen) { flushInputs(); showDeathTrail(); }
            return;
        }
        if (f->draw) f->draw->setVisible(false);
        if (f->frozen) { f->frozen = false; f->samples.clear(); }
        flushInputs();
        append(f->samples, {p->getObjectRect(), p->m_fields->held ? Kind::Held : Kind::Idle});
    }
    void showDeathTrail() {
        auto& f = m_fields;
        if (!m_objectLayer || !enabled()) return;
        if (!f->draw) {
            f->draw = CCDrawNode::create();
            m_objectLayer->addChild(f->draw, 10000);
        }
        f->frozen = true;
        f->draw->clear();
        f->draw->setVisible(true);
        while (f->samples.size() > limit()) f->samples.pop_front();
        // Draw input events last so coincident regular samples cannot hide them.
        for (int pass = 0; pass < 2; ++pass) {
            for (auto const& s : f->samples) {
                bool event = s.kind == Kind::Press || s.kind == Kind::Release;
                if (event != (pass == 1)) continue;
                ccColor4F color;
                switch (s.kind) {
                    case Kind::Press: color = {1.f,.75f,.1f,1.f}; break;
                    case Kind::Release: color = {.2f,.75f,1.f,1.f}; break;
                    case Kind::Held: color = {.1f,1.f,.2f,1.f}; break;
                    default: color = {1.f,.15f,.15f,1.f}; break;
                }
                auto r = s.rect;
                CCPoint v[] = {{r.getMinX(),r.getMinY()},{r.getMaxX(),r.getMinY()},{r.getMaxX(),r.getMaxY()},{r.getMinX(),r.getMaxY()}};
                f->draw->drawPolygon(v, 4, {0.f,0.f,0.f,0.f}, event ? .8f : .45f, color);
                // Different corner markers distinguish coincident press/release.
                if (event) f->draw->drawDot(s.kind == Kind::Press ? v[0] : v[2], 1.5f, color);
            }
        }
    }
};
