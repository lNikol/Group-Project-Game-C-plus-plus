#include "JournalWindow.h"
#include "worldmap/AssetManager.h"
#include <sstream>

namespace RPG {
    JournalWindow::JournalWindow(const sf::Font& font)
        : WindowBase(font, sf::Vector2f(400.f, 500.f), "Quest Journal"), m_font(font)
    {
    }

    void JournalWindow::setSize(const sf::Vector2f& size) {
        WindowBase::setSize(size);
        if (isVisible()) {
            refreshQuestList();
        }
    }

    void JournalWindow::update(float dt) {
        WindowBase::update(dt);
        if (isVisible()) {
            refreshQuestList();
        }
    }

    void JournalWindow::refreshQuestList() {
        m_questTexts.clear();
        m_questCards.clear();
        
        float startY = 40.f;
        float startX = 20.f;
        float currentY = startY;
        float availableWidth = getSize().x - startX - 20.f;

        const sf::Texture* uiTex = AssetManager::getInstance().getTexture("ui_window");

        // Active Quests Title
        auto activeTitle = std::make_unique<sf::Text>(m_font);
        activeTitle->setString("Active Quests");
        activeTitle->setCharacterSize(32);
        activeTitle->setFillColor(sf::Color::White);
        activeTitle->setPosition({startX, currentY});
        currentY += 40.f;
        m_questTexts.push_back(std::move(activeTitle));

        auto activeQuests = QuestManager::getInstance().getActiveQuests();
        if (activeQuests.empty()) {
            auto text = std::make_unique<sf::Text>(m_font);
            text->setString("No active quests.");
            text->setCharacterSize(16);
            text->setFillColor(sf::Color(200, 200, 200));
            text->setPosition({startX, currentY});
            m_questTexts.push_back(std::move(text));
            currentY += 25.f;
        } else {
            for (const auto* q : activeQuests) {
                QuestCard card;
                if (uiTex) {
                    card.background.setTexture(*uiTex, 32, 0.5f);
                    card.background.setColor(sf::Color(160, 160, 160, 255));
                }

                float cardStartY = currentY;
                float cardContentY = cardStartY + 10.f;
                float cardContentX = startX + 15.f;
                float cardWidth = availableWidth;

                auto title = std::make_unique<sf::Text>(m_font);
                title->setString(q->title);
                title->setCharacterSize(16);
                title->setFillColor(sf::Color::White);
                title->setPosition({cardContentX, cardContentY});
                cardContentY += 16.f;
                card.texts.push_back(std::move(title));

                auto desc = std::make_unique<sf::Text>(m_font);
                std::string wrappedDesc = wrapText(q->description, cardWidth - 30.f, 16);
                desc->setString(wrappedDesc);
                desc->setCharacterSize(16);
                desc->setFillColor(sf::Color(220, 220, 220));
                desc->setPosition({cardContentX, cardContentY});
                cardContentY += desc->getLocalBounds().size.y + 10.f;
                card.texts.push_back(std::move(desc));

                for (const auto& obj : q->objectives) {
                    auto objText = std::make_unique<sf::Text>(m_font);
                    std::string progressStr = obj.isCompleted ? " (Done)" : " (" + std::to_string(obj.currentAmount) + "/" + std::to_string(obj.requiredAmount) + ")";
                    std::string objWrapped = wrapText("- " + obj.description + progressStr, cardWidth - 30.f, 16);
                    objText->setString(objWrapped);
                    objText->setCharacterSize(16);
                    objText->setFillColor(obj.isCompleted ? sf::Color(100, 255, 100) : sf::Color::White);
                    objText->setPosition({cardContentX + 10.f, cardContentY});
                    cardContentY += objText->getLocalBounds().size.y + 10.f;
                    card.texts.push_back(std::move(objText));
                }

                if (q->status == QuestStatus::ReadyToTurnIn) {
                    auto readyText = std::make_unique<sf::Text>(m_font);
                    readyText->setString("Ready to turn in!");
                    readyText->setCharacterSize(16);
                    readyText->setFillColor(sf::Color::Cyan);
                    readyText->setPosition({cardContentX + 10.f, cardContentY});
                    cardContentY += 20.f;
                    card.texts.push_back(std::move(readyText));
                }

                cardContentY += 10.f; // Bottom padding
                card.background.setPosition({startX, cardStartY});
                card.background.setSize({cardWidth, cardContentY - cardStartY});

                m_questCards.push_back(std::move(card));
                currentY = cardContentY + 15.f; // Space between cards
            }
        }

        currentY += 10.f;

        // Completed Quests
        auto completedTitle = std::make_unique<sf::Text>(m_font);
        completedTitle->setString("Completed Quests");
        completedTitle->setCharacterSize(32);
        completedTitle->setFillColor(sf::Color::White);
        completedTitle->setPosition({startX, currentY});
        currentY += 40.f;
        m_questTexts.push_back(std::move(completedTitle));

        auto completedQuests = QuestManager::getInstance().getCompletedQuests();
        if (completedQuests.empty()) {
            auto text = std::make_unique<sf::Text>(m_font);
            text->setString("No completed quests.");
            text->setCharacterSize(16);
            text->setFillColor(sf::Color(200, 200, 200));
            text->setPosition({startX, currentY});
            m_questTexts.push_back(std::move(text));
            currentY += 25.f;
        } else {
            for (const auto* q : completedQuests) {
                QuestCard card;
                if (uiTex) {
                    card.background.setTexture(*uiTex, 32, 0.5f);
                    card.background.setColor(sf::Color(80, 80, 80, 255));
                }

                float cardStartY = currentY;
                float cardContentY = cardStartY + 10.f;
                float cardContentX = startX + 15.f;
                float cardWidth = availableWidth;

                auto title = std::make_unique<sf::Text>(m_font);
                title->setString(q->title);
                title->setCharacterSize(16);
                title->setFillColor(sf::Color::White);
                title->setPosition({cardContentX, cardContentY});
                cardContentY += 16.f;
                card.texts.push_back(std::move(title));

                cardContentY += 10.f; // Bottom padding
                card.background.setPosition({startX, cardStartY});
                card.background.setSize({cardWidth, cardContentY - cardStartY});

                m_questCards.push_back(std::move(card));
                currentY = cardContentY + 15.f;
            }
        }
    }

    std::string JournalWindow::wrapText(const std::string& str, float width, unsigned int charSize) {
        if (width <= 0) return str;
        std::string result;
        std::istringstream words(str);
        std::string word;
        
        sf::Text text(m_font);
        text.setCharacterSize(charSize);
        
        std::string currentLine;
        while (words >> word) {
            std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
            text.setString(testLine);
            if (text.getLocalBounds().size.x > width) {
                if (!currentLine.empty()) {
                    result += currentLine + "\n";
                    currentLine = word;
                } else {
                    result += word + "\n";
                    currentLine = "";
                }
            } else {
                currentLine = testLine;
            }
        }
        result += currentLine;
        return result;
    }

    void JournalWindow::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        WindowBase::draw(target, states);

        if (isVisible()) {
            states.transform *= getTransform();
            
            for (const auto& card : m_questCards) {
                target.draw(card.background, states);
                for (const auto& text : card.texts) {
                    target.draw(*text, states);
                }
            }

            // Draw floating texts
            for (const auto& text : m_questTexts) {
                target.draw(*text, states);
            }
        }
    }
}
