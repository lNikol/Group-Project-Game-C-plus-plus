#include "ActionSlot.h"
#include "worldmap/AssetManager.h"

namespace RPG {

    ActionSlot::ActionSlot(const Spritesheet& iconSet, const sf::Font& font, const sf::Vector2f& size)
        : m_font(font),
        m_icon(iconSet),
        m_cooldownText(font),
        m_chargesText(font),
        m_size(size)
    {
        // Setup Background
        if (const sf::Texture* tex = AssetManager::getInstance().getTexture("ui_slot")) {
            m_background.emplace(*tex);
            // Scale the sprite to match the requested size
            sf::Vector2u texSize = tex->getSize();
            if (texSize.x > 0 && texSize.y > 0) {
                m_background->setScale({ size.x / (float)texSize.x, size.y / (float)texSize.y });
            }
        }

        // Setup Selection Border
        m_border.setSize(size);
        m_border.setFillColor(sf::Color::Transparent);
        m_border.setOutlineColor(sf::Color::Green);
        m_border.setOutlineThickness(-2.f); // Inner stroke

        // Setup Overlay (Greyed out)
        m_overlay.setSize(size);
        m_overlay.setFillColor(sf::Color(0, 0, 0, 180)); // Darker fade

        // Setup Icon
        float scale = (size.x / ICON_SIZE) * 0.7f;
        m_icon.setScale({ scale, scale });
        float scaledIconSize = ICON_SIZE * scale;
        m_icon.setPosition({ (size.x - scaledIconSize) / 2.f, (size.y - scaledIconSize) / 2.f });

        // Setup Charges Text (Bottom Right)
        m_chargesText.setCharacterSize(16);
        m_chargesText.setFillColor(sf::Color::White);
        m_chargesText.setOutlineColor(sf::Color::Black);
        m_chargesText.setOutlineThickness(1.f);

        // Setup Cooldown Text (Center Large)
        m_cooldownText.setCharacterSize(16);
        m_cooldownText.setFillColor(sf::Color::Yellow);
        m_cooldownText.setOutlineColor(sf::Color::Black);
        m_cooldownText.setOutlineThickness(2.f);
    }

    void ActionSlot::setAbility(std::shared_ptr<IAbility> ability) {
        m_ability = ability;

        if (m_ability) {
            setTooltipText(m_ability->getTooltip());

            // Calculate Icon Spritesheet Rect
            int index = m_ability->getIconIndex();
            int cols = 16; //sheet width
            int x = (index % cols) * ICON_SIZE;
            int y = (index / cols) * ICON_SIZE;
            m_icon.setTextureRect(sf::IntRect({ x, y }, { ICON_SIZE, ICON_SIZE }));
        }
        else {
            setTooltipText("");
        }
    }

    std::shared_ptr<IAbility> ActionSlot::getAbility() const {
        return m_ability;
    }

    void ActionSlot::setSelected(bool selected) {
        m_isSelected = selected;
    }

    void ActionSlot::update(float dt) {
        if (!m_ability) return;

        // Cooldown Display
        int cd = m_ability->getCooldown();
        if (cd > 0) {
            m_cooldownText.setString(std::to_string(cd));
            auto bounds = m_cooldownText.getLocalBounds();
            m_cooldownText.setOrigin(bounds.position + bounds.size / 2.f);
            m_cooldownText.setPosition(m_size / 2.f);
        }

        // Charges Display
        int charges = m_ability->getCharges();
        if (charges >= 0) {
            m_chargesText.setString("x" + std::to_string(charges));
            // Bottom Right align
            auto bounds = m_chargesText.getLocalBounds();
            m_chargesText.setOrigin(bounds.position + bounds.size);
            m_chargesText.setPosition(m_size - sf::Vector2f(2.f, 2.f));
        }
    }

    bool ActionSlot::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        if (const auto* mouseEvent = event.getIf<sf::Event::MouseMoved>()) {
            updateHoverState(window, mouseEvent->position);
        }

        // Click Logic
        if (const auto* mouseClick = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseClick->button == sf::Mouse::Button::Left && m_isHovered) {
                updateHoverState(window, mouseClick->position);
                if (m_isInteractive && m_ability != nullptr)
                {
                    m_ability->execute();
                }
                return true;
            }
        }
        return false;
    }

    sf::FloatRect ActionSlot::getGlobalBounds() const {
        return getAbsoluteTransform().transformRect(sf::FloatRect({0.f, 0.f}, m_size));
    }

    void ActionSlot::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();

        // Slot Background
        if (m_background) {
            target.draw(*m_background, states);
        }

        // If Empty, stop here
        if (!m_ability) return;

        // Draw Icon
        target.draw(m_icon, states);

        // Draw Grey Overlay 
        if (!m_ability->canBeCast()) {
            target.draw(m_overlay, states);
        }

        // Draw Text Info
        if (m_ability->getCooldown() > 0) {
            target.draw(m_cooldownText, states);
        }
        else if (m_ability->getCharges() >= 0) {
            // Only show charges if not on cooldown
            target.draw(m_chargesText, states);
        }

        // Draw Selection Border
        if (m_isSelected) {
            target.draw(m_border, states);
        }
    }
}