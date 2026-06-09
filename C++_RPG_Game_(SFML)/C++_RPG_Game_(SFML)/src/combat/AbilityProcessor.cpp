#include "AbilityProcessor.h"
#include "StatusEffect.h"
#include "combat/Unit.h" 
#include "combat/BattleManager.h" 
#include <cstdlib>
#include "AudioManager.h"

namespace RPG {

    float AbilityProcessor::calculatePower(const EffectData& effect, const Unit& caster) {
        float statValue = caster.getStats()[effect.scalingStat];
        return effect.baseValue + (statValue * effect.scalingFactor);
    }

    float AbilityProcessor::calculateCritChance(const EffectData& effect, const Unit& caster) {
        float totalChance = caster.getCritChance() + effect.baseCritChance;
        float statValue = caster.getStats()[effect.critScalingStat];
        totalChance += (statValue * effect.critScalingFactor);
        return totalChance;
    }

    void AbilityProcessor::applyEffect(const EffectData& effect, Unit& source, Unit& target, BattleManager& manager) {
        // 1. Roll for Application Chance (Hit/Miss)
        float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        if (roll > effect.applyChance) {
            manager.spawnFloatingText(target.getRenderPosition(), "Miss", sf::Color(150, 150, 150), 16);
            return;
        }

        // 2. Calculate Potency & Crit
        float power = calculatePower(effect, source);
        bool isCrit = false;

        float critChance = calculateCritChance(effect, source);
        float critRoll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        if (critRoll < critChance) {
            isCrit = true;
            power *= effect.critMultiplier;
        }

        // 3. Execution Logic: Instant vs. Over-Time
        if (effect.durationTurns > 0) {
            // --- LINGERING STATUS EFFECT ---
            ActiveEffect active;
            active.baseEffect = effect;
            active.lockedPower = power;
            active.turnsRemaining = effect.durationTurns;
            active.source = &source;

            target.addStatusEffect(active);

            // Visual Confirmation of application
            std::string msg;
            sf::Color color;
            switch (effect.type) {
            case EffectType::Damage: msg = "Poisoned!"; color = sf::Color(150, 0, 200); break;
            case EffectType::Heal:   msg = "Regen!"; color = sf::Color(50, 255, 100); break;
            case EffectType::Buff:   msg = "Buffed!"; color = sf::Color::Cyan; break;
            case EffectType::Debuff: msg = "Debuffed!"; color = sf::Color(200, 100, 50); break;
            case EffectType::Stun:   msg = "Stunned!"; color = sf::Color::Magenta; break;
            }
            manager.spawnFloatingText(target.getRenderPosition(), msg, color, 20);
        }
        else {
            // --- INSTANT EFFECT ---
            switch (effect.type) {
            case EffectType::Damage: {
                target.takeDamage(power);
                sf::Color color = isCrit ? sf::Color::Yellow : sf::Color::White;
                std::string text = std::to_string(static_cast<int>(power)) + (isCrit ? "!" : "");
                manager.spawnFloatingText(target.getRenderPosition(), text, color, isCrit ? 24 : 18);
                break;
            }
            case EffectType::Heal: {
                target.heal(power);
                sf::Color color = isCrit ? sf::Color::Green : sf::Color(100, 255, 100);
                std::string text = "+" + std::to_string(static_cast<int>(power)) + (isCrit ? "!" : "");
                manager.spawnFloatingText(target.getRenderPosition(), text, color, isCrit ? 24 : 18);
                break;
            }
            default: break; // 0-duration Buffs/Stuns do nothing.
            }
        }
    }

    void AbilityProcessor::processAbility(const AbilityDefinition& abilityDef, Unit& source, Unit& target, BattleManager& manager) {
        RPG::AudioManager::getInstance().playSoundFile(abilityDef.filePath);
        for (const auto& effect : abilityDef.effects) {
            applyEffect(effect, source, target, manager);
        } 
    }
}