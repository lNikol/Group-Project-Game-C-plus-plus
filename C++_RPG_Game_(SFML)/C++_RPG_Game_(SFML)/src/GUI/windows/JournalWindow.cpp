#include "JournalWindow.h"
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
        
        float startY = 40.f;
        float startX = 20.f;
        float currentY = startY;
        float availableWidth = getSize().x - startX - 20.f;

        // Active Quests
        auto activeTitle = std::make_unique<sf::Text>(m_font);
        activeTitle->setString("--- Active Quests ---");
        activeTitle->setCharacterSize(24);
        activeTitle->setFillColor(sf::Color::Yellow);
        activeTitle->setPosition({startX, currentY});
        m_questTexts.push_back(std::move(activeTitle));
        currentY += 30.f;

        auto activeQuests = QuestManager::getInstance().getActiveQuests();
        if (activeQuests.empty()) {
            auto text = std::make_unique<sf::Text>(m_font);
            text->setString("No active quests.");
            text->setCharacterSize(20);
            text->setFillColor(sf::Color(200, 200, 200));
            text->setPosition({startX, currentY});
            m_questTexts.push_back(std::move(text));
            currentY += 25.f;
        } else {
            for (const auto* q : activeQuests) {
                auto title = std::make_unique<sf::Text>(m_font);
                title->setString(q->title);
                title->setCharacterSize(22);
                title->setFillColor(sf::Color::White);
                title->setPosition({startX, currentY});
                m_questTexts.push_back(std::move(title));
                currentY += 25.f;

                auto desc = std::make_unique<sf::Text>(m_font);
                std::string wrappedDesc = wrapText(q->description, availableWidth - 10.f, 18);
                desc->setString(wrappedDesc);
                desc->setCharacterSize(18);
                desc->setFillColor(sf::Color(180, 180, 180));
                desc->setPosition({startX + 10.f, currentY});
                
                // Advance currentY by the actual height of the text block + padding
                currentY += desc->getLocalBounds().size.y + 10.f;
                m_questTexts.push_back(std::move(desc));

                // Objectives
                for (const auto& obj : q->objectives) {
                    auto objText = std::make_unique<sf::Text>(m_font);
                    std::string progressStr = obj.isCompleted ? " (Done)" : " (" + std::to_string(obj.currentAmount) + "/" + std::to_string(obj.requiredAmount) + ")";
                    std::string objWrapped = wrapText("- " + obj.description + progressStr, availableWidth - 20.f, 16);
                    objText->setString(objWrapped);
                    objText->setCharacterSize(16);
                    objText->setFillColor(obj.isCompleted ? sf::Color(100, 255, 100) : sf::Color(200, 200, 150));
                    objText->setPosition({startX + 20.f, currentY});
                    
                    currentY += objText->getLocalBounds().size.y + 10.f;
                    m_questTexts.push_back(std::move(objText));
                }
                
                if (q->status == QuestStatus::ReadyToTurnIn) {
                    auto readyText = std::make_unique<sf::Text>(m_font);
                    readyText->setString("Ready to turn in!");
                    readyText->setCharacterSize(16);
                    readyText->setFillColor(sf::Color::Cyan);
                    readyText->setPosition({startX + 20.f, currentY});
                    currentY += 25.f;
                    m_questTexts.push_back(std::move(readyText));
                }

                currentY += 10.f;
            }
        }

        currentY += 10.f;

        // Completed Quests
        auto completedTitle = std::make_unique<sf::Text>(m_font);
        completedTitle->setString("--- Completed Quests ---");
        completedTitle->setCharacterSize(24);
        completedTitle->setFillColor(sf::Color::Green);
        completedTitle->setPosition({startX, currentY});
        m_questTexts.push_back(std::move(completedTitle));
        currentY += 30.f;

        auto completedQuests = QuestManager::getInstance().getCompletedQuests();
        if (completedQuests.empty()) {
            auto text = std::make_unique<sf::Text>(m_font);
            text->setString("No completed quests.");
            text->setCharacterSize(20);
            text->setFillColor(sf::Color(200, 200, 200));
            text->setPosition({startX, currentY});
            m_questTexts.push_back(std::move(text));
            currentY += 25.f;
        } else {
            for (const auto* q : completedQuests) {
                auto title = std::make_unique<sf::Text>(m_font);
                title->setString(q->title);
                title->setCharacterSize(22);
                title->setFillColor(sf::Color(150, 150, 150));
                title->setPosition({startX, currentY});
                m_questTexts.push_back(std::move(title));
                currentY += 25.f;
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
            // Draw all quest texts relative to window position
            for (const auto& text : m_questTexts) {
                target.draw(*text, states);
            }
        }
    }
}
