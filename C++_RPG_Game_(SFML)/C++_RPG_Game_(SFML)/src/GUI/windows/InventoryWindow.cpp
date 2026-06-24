#include "InventoryWindow.h"
#include <sstream>
#include <iomanip>

namespace RPG {
    InventoryWindow::InventoryWindow(const Spritesheet& iconSet, const sf::Font& font)
        : WindowBase(font, sf::Vector2f(500.f, 380.f), "Party Inventory"),
        m_font(font),
        m_iconSet(iconSet),
        m_charNameText(font),
        m_prevBtn(font),
        m_nextBtn(font),
        m_draggedIcon(iconSet),
        m_tooltip(font)
    {
        buildLayout();
        refreshCharacterDisplay();
    }

    void InventoryWindow::buildLayout() {
        float charStartX = 20.f;
        float charStartY = 40.f;

        m_prevBtn.setString("<");
        m_prevBtn.setCharacterSize(24);
        m_prevBtn.setPosition({ charStartX, charStartY });

        m_nextBtn.setString(">");
        m_nextBtn.setCharacterSize(24);
        m_nextBtn.setPosition({ charStartX + 180.f, charStartY });
        m_prevHitbox = sf::FloatRect({ charStartX - 10.f, charStartY - 5.f }, { 40.f, 40.f });
        m_nextHitbox = sf::FloatRect({ charStartX + 180.f - 10.f, charStartY - 5.f }, { 40.f, 40.f });
        m_charNameText.setCharacterSize(22);
        m_charNameText.setFillColor(sf::Color::Black);
        m_charNameText.setPosition({ charStartX + 25.f, charStartY + 2.f });

        float eqBaseX = 30.f;
        float eqBaseY = 80.f;
        float slotSize = 40.f;
        float sp = 10.f;

        auto addEquipSlot = [&](EquipSlot type, float xOffset, float yOffset) {
            auto slot = std::make_unique<ActionSlot>(m_iconSet, m_font, sf::Vector2f(slotSize, slotSize));
            slot->setPosition({ eqBaseX + xOffset, eqBaseY + yOffset });
            m_equipSlots[type] = slot.get();
            addChild(std::move(slot));
            };

        addEquipSlot(EquipSlot::Head, slotSize + sp, 0.f);
        addEquipSlot(EquipSlot::MainHand, 0.f, slotSize + sp);
        addEquipSlot(EquipSlot::Chest, slotSize + sp, slotSize + sp);
        addEquipSlot(EquipSlot::OffHand, (slotSize + sp) * 2, slotSize + sp);
        addEquipSlot(EquipSlot::Ring, 0.f, (slotSize + sp) * 2);
        addEquipSlot(EquipSlot::Feet, slotSize + sp, (slotSize + sp) * 2);
        addEquipSlot(EquipSlot::Necklace, (slotSize + sp) * 2, (slotSize + sp) * 2);

        float bagStartX = 260.f;
        float bagStartY = 40.f;
        float bagSpacing = 5.f;

        for (int i = 0; i < INVENTORY_SIZE; ++i) {
            auto slot = std::make_unique<ActionSlot>(m_iconSet, m_font, sf::Vector2f(slotSize, slotSize));
            slot->setPosition({ bagStartX + (i % 5) * (slotSize + bagSpacing),
                                bagStartY + (i / 5) * (slotSize + bagSpacing) });
            m_bagSlots.push_back(slot.get());
            addChild(std::move(slot));
        }
    }

    UnitStats InventoryWindow::calculateProfileStats(const CharacterProfile& profile) const {
        UnitStats total = profile.baseStats;
        for (const auto& [slot, item] : profile.equipment) {
            if (auto combatItem = std::dynamic_pointer_cast<CombatAbility>(item)) {
                if (const auto* def = combatItem->getDefinition()) {
                    for (const auto& [stat, val] : def->statModifiers) {
                        if (stat != StatType::None) total[stat] += val;
                    }
                }
            }
        }
        return total;
    }

    void InventoryWindow::refreshCharacterDisplay() {
        auto& party = PartyData::getInstance().activeParty;
        if (party.empty() || m_selectedCharIndex >= party.size()) return;

        auto profile = party[m_selectedCharIndex];
        m_charNameText.setString(profile->name);

        UnitStats currentStats = calculateProfileStats(*profile);
        m_statsTexts.clear();

        float statY = 240.f;
        float statX = 30.f;
        auto addStatText = [&](const std::string& label, float value) {
            sf::Text t(m_font);
            t.setCharacterSize(18);
            t.setFillColor(sf::Color::Black);
            std::stringstream ss;
            ss << label << ": " << std::fixed << std::setprecision(0) << value;
            t.setString(ss.str());
            t.setPosition({ statX, statY });
            m_statsTexts.push_back(t);
            statY += 20.f;
            };

        addStatText("HP", profile->currentVitals.hp);
        addStatText("STR", currentStats[StatType::Strength]);
        addStatText("DEX", currentStats[StatType::Dexterity]);
        addStatText("INT", currentStats[StatType::Intellect]);
        addStatText("VIT", currentStats[StatType::Vitality]);
    }

    void InventoryWindow::switchCharacter(int direction) {
        auto& party = PartyData::getInstance().activeParty;
        if (party.empty()) return;

        if (direction > 0) m_selectedCharIndex = (m_selectedCharIndex + 1) % party.size();
        else m_selectedCharIndex = (m_selectedCharIndex == 0) ? party.size() - 1 : m_selectedCharIndex - 1;

        refreshCharacterDisplay();
    }

    bool InventoryWindow::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        if (!isVisible()) return false;

        sf::Vector2f mousePos;

        if (const auto* move = event.getIf<sf::Event::MouseMoved>()) {
            mousePos = window.mapPixelToCoords(move->position);
            m_lastMousePos = mousePos;

            if (m_draggedItem) {
                m_draggedIcon.setPosition(mousePos);
            }

            sf::Transform t = getTransform();
            m_prevHovered = t.transformRect(m_prevHitbox).contains(mousePos);
            m_nextHovered = t.transformRect(m_nextHitbox).contains(mousePos);

            m_prevBtn.setFillColor(m_prevHovered ? sf::Color::Cyan : sf::Color::Black);
            m_nextBtn.setFillColor(m_nextHovered ? sf::Color::Cyan : sf::Color::Black);
        }

        if (const auto* press = event.getIf<sf::Event::MouseButtonPressed>()) {
            mousePos = window.mapPixelToCoords(press->position);
            m_lastMousePos = mousePos;

            if (press->button == sf::Mouse::Button::Left) {
                if (m_prevHovered) { switchCharacter(-1); return true; }
                if (m_nextHovered) { switchCharacter(1); return true; }

                handleDragStart(mousePos);
                if (m_draggedItem) return true;
            }
            else if (press->button == sf::Mouse::Button::Right) {
                if (!m_isCombatMode) return true;

                for (auto* slot : m_bagSlots) {
                    if (slot->getGlobalBounds().contains(mousePos) && slot->getAbility()) {
                        hide();
                        slot->getAbility()->execute();
                        return true;
                    }
                }
            }
        }

        if (const auto* release = event.getIf<sf::Event::MouseButtonReleased>()) {
            mousePos = window.mapPixelToCoords(release->position);
            m_lastMousePos = mousePos;

            if (release->button == sf::Mouse::Button::Left && m_draggedItem) {
                handleDragEnd(mousePos);
                return true;
            }
        }

        return WindowBase::handleEvent(window, event);
    }

    void InventoryWindow::handleDragStart(sf::Vector2f mousePos) {
        auto checkAndStart = [&](ActionSlot* slot, int bagIdx, EquipSlot eqSlot) -> bool {
            if (slot->getGlobalBounds().contains(mousePos) && slot->getAbility() != nullptr) {
                m_draggedItem = slot->getAbility();
                m_dragSourceSlot = slot;
                m_dragSourceBagIndex = bagIdx;
                m_dragSourceEquipSlot = eqSlot;

                int index = m_draggedItem->getIconIndex();
                int x = (index % 16) * 24;
                int y = (index / 16) * 24;
                m_draggedIcon.setTextureRect(sf::IntRect({ x, y }, { 24, 24 }));
                m_draggedIcon.setScale({ 40.f / 24.f, 40.f / 24.f });
                m_draggedIcon.setOrigin({ 12.f, 12.f });
                m_draggedIcon.setPosition(mousePos);
                return true;
            }
            return false;
            };

        for (int i = 0; i < m_bagSlots.size(); ++i) {
            if (checkAndStart(m_bagSlots[i], i, EquipSlot::None)) return;
        }

        for (auto& [type, slot] : m_equipSlots) {
            if (checkAndStart(slot, -1, type)) return;
        }
    }

    void InventoryWindow::handleDragEnd(sf::Vector2f mousePos) {
        ActionSlot* targetSlot = nullptr;
        int targetBagIndex = -1;
        EquipSlot targetEquipSlot = EquipSlot::None;

        for (int i = 0; i < m_bagSlots.size(); ++i) {
            if (m_bagSlots[i]->getGlobalBounds().contains(mousePos)) {
                targetSlot = m_bagSlots[i];
                targetBagIndex = i;
                break;
            }
        }

        if (!targetSlot) {
            for (auto& [type, slot] : m_equipSlots) {
                if (slot->getGlobalBounds().contains(mousePos)) {
                    targetSlot = slot;
                    targetEquipSlot = type;
                    break;
                }
            }
        }

        auto& partyData = PartyData::getInstance();
        auto profile = partyData.activeParty[m_selectedCharIndex];

        if (targetSlot && targetSlot != m_dragSourceSlot) {

            if (m_isCombatMode) {
                if (m_dragSourceEquipSlot != EquipSlot::None || targetEquipSlot != EquipSlot::None) {

                    // TODO: Add Error sound effect

                    m_draggedItem = nullptr;
                    m_dragSourceSlot = nullptr;
                    return;
                }
            }

            std::shared_ptr<IAbility> targetItem = targetSlot->getAbility();
            bool canSwap = true;

            if (targetEquipSlot != EquipSlot::None) {
                if (auto combatItem = std::dynamic_pointer_cast<CombatAbility>(m_draggedItem)) {
                    if (!combatItem->getDefinition()->isEquippable || combatItem->getDefinition()->equipSlot != targetEquipSlot) {
                        canSwap = false;
                    }
                }
                else canSwap = false;
            }
            if (canSwap && m_dragSourceEquipSlot != EquipSlot::None && targetItem != nullptr) {
                if (auto combatItem = std::dynamic_pointer_cast<CombatAbility>(targetItem)) {
                    if (!combatItem->getDefinition()->isEquippable || combatItem->getDefinition()->equipSlot != m_dragSourceEquipSlot) {
                        canSwap = false;
                    }
                }
                else canSwap = false;
            }

            if (canSwap) {
                if (m_dragSourceBagIndex != -1) partyData.sharedInventory[m_dragSourceBagIndex] = targetItem;
                else if (m_dragSourceEquipSlot != EquipSlot::None) profile->equipment[m_dragSourceEquipSlot] = targetItem;

                if (targetBagIndex != -1) partyData.sharedInventory[targetBagIndex] = m_draggedItem;
                else if (targetEquipSlot != EquipSlot::None) profile->equipment[targetEquipSlot] = m_draggedItem;

                refreshCharacterDisplay();
            }
        }

        m_draggedItem = nullptr;
        m_dragSourceSlot = nullptr;
    }

    void InventoryWindow::update(float dt) {
        WindowBase::update(dt);
        if (!m_isVisible) return;

        auto& partyData = PartyData::getInstance();
        partyData.clearConsumedItems();

        auto& inv = partyData.sharedInventory;
        for (size_t i = 0; i < m_bagSlots.size(); ++i) {
            if (m_bagSlots[i]->getAbility() != inv[i]) m_bagSlots[i]->setAbility(inv[i]);
            m_bagSlots[i]->update(dt);
        }

        if (!partyData.activeParty.empty() && m_selectedCharIndex < partyData.activeParty.size()) {
            auto profile = partyData.activeParty[m_selectedCharIndex];
            for (auto& [type, slot] : m_equipSlots) {
                auto equippedItem = profile->equipment[type];
                if (slot->getAbility() != equippedItem) {
                    slot->setAbility(equippedItem);
                    refreshCharacterDisplay();
                }
                slot->update(dt);
            }
        }
    }

    void InventoryWindow::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        WindowBase::draw(target, states);

        if (isVisible()) {
            sf::RenderStates innerStates = states;
            innerStates.transform *= getTransform();

            target.draw(m_prevBtn, innerStates);
            target.draw(m_nextBtn, innerStates);
            target.draw(m_charNameText, innerStates);

            for (const auto& text : m_statsTexts) {
                target.draw(text, innerStates);
            }
        }
        if (m_draggedItem) {
            target.draw(m_draggedIcon, states);
        }
        else {
            if (auto tipText = getChildTooltipIfHovered()) {
                m_tooltip.update(*tipText, m_lastMousePos, target.getView().getSize());
                target.draw(m_tooltip, states);
            }
        }
    }
}