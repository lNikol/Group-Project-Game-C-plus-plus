#include <cmath>
#include <memory>
#include <string>
#include "../interfaces/ICombatActor.h"
#include "../interfaces/IAbility.h"
namespace RPG {
class TestActor : public ICombatActor {
public:
    Vitals getVitals() const override {
        static float time = 0;
        time += 0.01f;
        return {
            50.f + 50.f * sin(time), 100.f,  // HP
            20.f, 50.f,                      // MP
            100.f, 100.f                     // Stamina
        };
    }

    std::string getName() const override { return "Test Hero"; }

    std::shared_ptr<IAbility> getHotbarAbility(int index) const override {
        if (index == 0) { 
            return nullptr;
        }
        return nullptr;
    }

    int getHotbarSize() const override { return 18; }

    std::shared_ptr<IAbility> getInventoryItem(int index) const override {
        return nullptr;
    }

    int getInventorySize() const override { return 20; }

    std::shared_ptr<IAbility> getEquipment(EquipSlot slot) const override {
        return nullptr;
    }
};
}