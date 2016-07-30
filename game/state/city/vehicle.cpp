#include "game/state/city/vehicle.h"
#include "framework/logger.h"
#include "game/state/city/building.h"
#include "game/state/city/city.h"
#include "game/state/city/projectile.h"
#include "game/state/city/vehiclemission.h"
#include "game/state/city/vequipment.h"
#include "game/state/equipment/equipmentusertype.h"
#include "game/state/equipment/store/baseequipmentstore.h"
#include "game/state/equipment/store/lockedequipmentstore.h"
#include "game/state/organisation.h"
#include "game/state/rules/vequipment_type.h"
#include "game/state/tileview/tileobject_shadow.h"
#include "game/state/tileview/tileobject_vehicle.h"
#include "game/state/tileview/voxel.h"
#include "library/sp.h"
#include <limits>
#include <queue>
#include <random>

namespace OpenApoc
{
template <> const UString &StateObject<Vehicle>::getPrefix()
{
	static UString prefix = "VEHICLE_";
	return prefix;
}

template <> const UString &StateObject<Vehicle>::getTypeName()
{
	static UString name = "Vehicle";
	return name;
}

template <>
const UString &StateObject<Vehicle>::getId(const GameState &state, const sp<Vehicle> ptr)
{
	static const UString emptyString = "";
	for (auto &v : state.vehicles)
	{
		if (v.second == ptr)
			return v.first;
	}
	LogError("No vehicle matching pointer %p", ptr.get());
	return emptyString;
}

class FlyingVehicleMover : public VehicleMover
{
  public:
	Vec3<float> goalPosition;

	FlyingVehicleMover(Vehicle &v, Vec3<float> initialGoal)
	    : VehicleMover(v), goalPosition(initialGoal)
	{
	}

	void update(GameState &state, unsigned int ticks) override
	{
		float speed = vehicle.getSpeed();
		if (!vehicle.missions.empty())
		{
			auto vehicleTile = this->vehicle.tileObject;
			if (!vehicleTile)
			{
				return;
			}
			float distanceLeft = speed * ticks;
			distanceLeft /= TICK_SCALE;
			while (distanceLeft > 0)
			{
				Vec3<float> vectorToGoal = goalPosition - vehicleTile->getPosition();
				float distanceToGoal = glm::length(vectorToGoal * VELOCITY_SCALE);
				if (distanceToGoal <= distanceLeft)
				{
					distanceLeft -= distanceToGoal;
					vehicle.setPosition(goalPosition);
					auto dir = glm::normalize(vectorToGoal);
					if (dir.z >= 0.9f || dir.z <= -0.9f)
					{
						dir = vehicleTile->getDirection();
						dir.z = 0;
						dir = glm::normalize(vectorToGoal);
					}
					vehicleTile->setDirection(dir);
					while (vehicle.missions.front()->isFinished(state, this->vehicle))
					{
						LogInfo("Vehicle mission \"%s\" finished",
						        vehicle.missions.front()->getName().cStr());
						vehicle.missions.pop_front();
						if (!vehicle.missions.empty())
						{
							LogInfo("Vehicle mission \"%s\" starting",
							        vehicle.missions.front()->getName().cStr());
							vehicle.missions.front()->start(state, this->vehicle);
							continue;
						}
						else
						{
							LogInfo("No next vehicle mission, going idle");
							break;
						}
					}
					if (vehicle.missions.empty() ||
					    vehicle.missions.front()->getNextDestination(state, this->vehicle,
					                                                 goalPosition) == false)
					{
						break;
					}
				}
				else
				{
					// If we're going straight up/down  use the horizontal version of the last
					// direction
					// instead
					auto dir = glm::normalize(vectorToGoal);
					if (dir.z >= 0.9f || dir.z <= -0.9f)
					{
						dir = vehicleTile->getDirection();
						dir.z = 0;
						dir = glm::normalize(vectorToGoal);
					}
					vehicleTile->setDirection(dir);
					Vec3<float> newPosition = distanceLeft * dir;
					newPosition /= VELOCITY_SCALE;
					newPosition += vehicleTile->getPosition();
					vehicle.setPosition(newPosition);
					break;
				}
			}
		}
	}
};

VehicleMover::VehicleMover(Vehicle &v) : vehicle(v) {}

VehicleMover::~VehicleMover() = default;
Vehicle::Vehicle()
    : attackMode(AttackMode::Standard), altitude(Altitude::Standard), position(0, 0, 0),
      velocity(0, 0, 0), facing(1, 0, 0), health(0), shield(0), shieldRecharge(0), equipment(this)
{
}

Vehicle::~Vehicle() = default;

const std::map<Vehicle::AttackMode, UString> Vehicle::AttackModeMap = {
    {Vehicle::AttackMode::Aggressive, "aggressive"},
    {Vehicle::AttackMode::Standard, "standard"},
    {Vehicle::AttackMode::Defensive, "defensive"},
    {Vehicle::AttackMode::Evasive, "evasive"},
};

const std::map<Vehicle::Altitude, UString> Vehicle::AltitudeMap = {
    {Vehicle::Altitude::Highest, "highest"},
    {Vehicle::Altitude::High, "high"},
    {Vehicle::Altitude::Standard, "standard"},
    {Vehicle::Altitude::Low, "low"},
};

void Vehicle::launch(TileMap &map, GameState &state, Vec3<float> initialPosition)
{
	LogInfo("Launching %s", this->name.cStr());
	if (this->tileObject)
	{
		LogError("Trying to launch already-launched vehicle");
		return;
	}
	auto bld = this->currentlyLandedBuilding;
	if (bld)
	{
		bld->landed_vehicles.erase({&state, shared_from_this()});
		this->currentlyLandedBuilding = "";
	}
	this->position = initialPosition;
	this->mover.reset(new FlyingVehicleMover(*this, initialPosition));
	map.addObjectToMap(shared_from_this());
}

void Vehicle::land(GameState &state, StateRef<Building> b)
{
	auto vehicleTile = this->tileObject;
	if (!vehicleTile)
	{
		LogError("Trying to land already-landed vehicle");
		return;
	}
	if (this->currentlyLandedBuilding)
	{
		LogError("Vehicle already in a building?");
		return;
	}
	this->currentlyLandedBuilding = b;
	b->landed_vehicles.insert({&state, shared_from_this()});
	this->tileObject->removeFromMap();
	this->tileObject.reset();
	this->shadowObject->removeFromMap();
	this->shadowObject = nullptr;
	this->position = {0, 0, 0};
}

void Vehicle::setupMover()
{
	switch (this->type->type)
	{
		case VehicleType::Type::Flying:
			this->mover.reset(new FlyingVehicleMover(*this, this->position));
		default:
			LogWarning("TODO: non flying vehicle movers");
	}
}

void Vehicle::update(GameState &state, unsigned int ticks)

{
	if (!this->missions.empty())
		this->missions.front()->update(state, *this, ticks);
	while (!this->missions.empty() && this->missions.front()->isFinished(state, *this))
	{
		LogInfo("Vehicle mission \"%s\" finished", this->missions.front()->getName().cStr());
		this->missions.pop_front();
		if (!this->missions.empty())
		{
			LogInfo("Vehicle mission \"%s\" starting", this->missions.front()->getName().cStr());
			this->missions.front()->start(state, *this);
			continue;
		}
		else
		{
			LogInfo("No next vehicle mission, going idle");
			break;
		}
	}

	if (this->type->type == VehicleType::Type::UFO && this->missions.empty())
	{
		auto alien_city = state.cities["CITYMAP_ALIEN"];
		// Make UFOs patrol their city if we're looking at it
		if (this->city.getSp() == alien_city && state.current_city == this->city)
		{
			this->missions.emplace_back(VehicleMission::patrol(*this));
		}
	}

	if (this->mover)
		this->mover->update(state, ticks);

	auto vehicleTile = this->tileObject;
	if (vehicleTile)
	{
		if (!this->type->animation_sprites.empty())
		{
			vehicleTile->nextFrame(ticks);
		}

		bool has_active_weapon = false;
		for (auto &item : equipment.weapons())
		{
			item->update(ticks);
			if (!this->isCrashed() && this->attackMode != Vehicle::AttackMode::Evasive &&
			    item->canFire())
			{
				has_active_weapon = true;
			}
		}

		if (has_active_weapon)
		{
			// Find something to shoot at!
			sp<TileObjectVehicle> enemy;
			if (!missions.empty() &&
			    missions.front()->type == VehicleMission::MissionType::AttackVehicle &&
			    vehicleTile->getDistanceTo(missions.front()->targetVehicle->tileObject) <=
			        getFiringRange())
			{
				enemy = missions.front()->targetVehicle->tileObject;
			}
			else
			{
				enemy = findClosestEnemy(state, vehicleTile);
			}

			if (enemy)
			{
				attackTarget(state, vehicleTile, enemy);
			}
		}
	}

	int maxShield = this->getMaxShield();
	if (maxShield)
	{
		int threshold = SHIELD_RECHARGE_TIME / maxShield;
		this->shieldRecharge += ticks;
		if (this->shieldRecharge > threshold)
		{
			this->shield += this->shieldRecharge / threshold;
			this->shieldRecharge %= threshold;
			if (this->shield > maxShield)
			{
				this->shield = maxShield;
			}
		}
	}
}

bool Vehicle::isCrashed() const { return this->health < this->type->crash_health; }

bool Vehicle::applyDamage(GameState &state, int damage, float armour)
{
	if (this->shield <= damage)
	{
		if (this->shield > 0)
		{
			damage -= this->shield;
			this->shield = 0;

			std::list<sp<VEquipment>> shieldModules;

			// destroy the shield modules
			for (auto &item : equipment.general())
			{
				if (item->type->shielding > 0)
				{
					shieldModules.push_back(item);
				}
			}
			for (auto &itemToRemove : shieldModules)
			{
				equipment.removeEquipment(itemToRemove);
			}
		}

		damage -= static_cast<int>(armour);
		if (damage > 0)
		{
			this->health -= damage;
			if (this->health <= 0)
			{
				this->health = 0;
				return true;
			}
			else if (isCrashed())
			{
				this->missions.clear();
				this->missions.emplace_back(VehicleMission::crashLand(*this));
				this->missions.front()->start(state, *this);
				return false;
			}
		}
	}
	else
	{
		this->shield -= damage;
	}
	return false;
}

void Vehicle::handleCollision(GameState &state, Collision &c)
{
	if (!this->tileObject)
	{
		LogError("It's possible multiple projectiles hit the same tile in the same tick (?)");
		return;
	}

	auto projectile = c.projectile.get();
	if (projectile)
	{
		auto vehicleDir = glm::round(this->facing);
		auto projectileDir = glm::normalize(projectile->getVelocity());
		auto dir = vehicleDir + projectileDir;
		dir = glm::round(dir);

		auto armourDirection = VehicleType::ArmourDirection::Right;
		if (dir.x == 0 && dir.y == 0 && dir.z == 0)
		{
			armourDirection = VehicleType::ArmourDirection::Front;
		}
		else if (dir * 0.5f == vehicleDir)
		{
			armourDirection = VehicleType::ArmourDirection::Rear;
		}
		// FIXME: vehicle Z != 0
		else if (dir.z < 0)
		{
			armourDirection = VehicleType::ArmourDirection::Top;
		}
		else if (dir.z > 0)
		{
			armourDirection = VehicleType::ArmourDirection::Bottom;
		}
		else if ((vehicleDir.x == 0 && dir.x != dir.y) || (vehicleDir.y == 0 && dir.x == dir.y))
		{
			armourDirection = VehicleType::ArmourDirection::Left;
		}

		float armourValue = 0.0f;
		auto armour = this->type->armour.find(armourDirection);
		if (armour != this->type->armour.end())
		{
			armourValue = armour->second;
		}

		if (applyDamage(state, projectile->damage, armourValue))
		{
			auto doodad = city->placeDoodad(StateRef<DoodadType>{&state, "DOODAD_EXPLOSION_2"},
			                                this->tileObject->getPosition());

			this->shadowObject->removeFromMap();
			this->tileObject->removeFromMap();
			this->shadowObject.reset();
			this->tileObject.reset();
			state.vehicles.erase(this->getId(state, this->shared_from_this()));
			return;
		}
	}
}

sp<TileObjectVehicle> Vehicle::findClosestEnemy(GameState &state, sp<TileObjectVehicle> vehicleTile)
{
	// Find the closest enemy within the firing arc
	float closestEnemyRange = std::numeric_limits<float>::max();
	sp<TileObjectVehicle> closestEnemy;
	for (auto &pair : state.vehicles)
	{
		auto otherVehicle = pair.second;
		if (otherVehicle.get() == this)
		{
			/* Can't fire at yourself */
			continue;
		}
		if (otherVehicle->city != this->city)
		{
			/* Can't fire on things a world away */
			continue;
		}
		if (this->owner->isRelatedTo(otherVehicle->owner) != Organisation::Relation::Hostile)
		{
			/* Not hostile, skip */
			continue;
		}
		auto otherVehicleTile = otherVehicle->tileObject;
		if (!otherVehicleTile)
		{
			/* Not in the map, ignore */
			continue;
		}
		float distance = vehicleTile->getDistanceTo(otherVehicleTile);

		// FIXME: Check weapon arc against otherVehicle
		if (distance < closestEnemyRange)
		{
			closestEnemyRange = distance;
			closestEnemy = otherVehicleTile;
		}
	}
	return closestEnemy;
}

void Vehicle::attackTarget(GameState &state, sp<TileObjectVehicle> vehicleTile,
                           sp<TileObjectVehicle> enemyTile)
{
	auto target = enemyTile->getCentrePosition();
	float distance = this->tileObject->getDistanceTo(enemyTile);

	for (auto &item : equipment.weapons())
	{
		if (!item->canFire())
		{
			continue;
		}
		if (distance <= item->getRange())
		{
			std::uniform_int_distribution<int> toHit(1, 99);
			int accuracy = 100 - item->type->accuracy + this->getAccuracy();
			int shotDiff = toHit(state.rng);

			if (shotDiff > accuracy)
			{
				float offset = (shotDiff - accuracy) / 25.0f;
				std::uniform_real_distribution<float> offsetRng(-offset, offset);
				target.x += offsetRng(state.rng);
				target.y += offsetRng(state.rng);
				target.z += offsetRng(state.rng) / 2;
			}

			auto projectile = item->fire(target);
			if (projectile)
			{
				vehicleTile->map.addObjectToMap(projectile);
				this->city->projectiles.insert(projectile);
			}
			else
			{
				LogWarning("Fire() produced no object");
			}
		}
	}
}

float Vehicle::getFiringRange() const
{
	float range = 0;
	for (auto &item : equipment.weapons())
	{
		if (range < item->getRange())
		{
			range = item->getRange();
		}
	}
	return range;
}

const Vec3<float> &Vehicle::getDirection() const
{
	static const Vec3<float> noDirection = {1, 0, 0};
	if (!this->tileObject)
	{
		LogError("getDirection() called on vehicle with no tile object");
		return noDirection;
	}
	return this->tileObject->getDirection();
}

void Vehicle::setPosition(const Vec3<float> &pos)
{
	this->position = pos;
	if (!this->tileObject)
	{
		LogError("setPosition called on vehicle with no tile object");
	}
	else
	{
		this->tileObject->setPosition(pos);
	}

	if (!this->shadowObject)
	{
		LogError("setPosition called on vehicle with no shadow object");
	}
	else
	{
		this->shadowObject->setPosition(pos);
	}
}

float Vehicle::getSpeed() const
{
	// FIXME: This is somehow modulated by weight?
	float speed = this->type->top_speed;

	for (auto &item : equipment.general())
	{
		speed += item->type->top_speed;
	}

	return speed;
}

std::map<UString, UString> Vehicle::getStats()
{
	int constitution = getConstitution();
	int maxConstitution = getMaxConstitution();
	UString constitutionString;

	if (constitution != maxConstitution)
	{
		constitutionString = UString::format("%d/%d", constitution, maxConstitution);
	}
	else
	{
		constitutionString = UString::format("%d", maxConstitution);
	}

	return {
	    {"Constitution", constitutionString},
	    {"Armor", UString::format("%d", getArmor())},
	    {"Accuracy", UString::format("%d%%", getAccuracy())},
	    {"Top Speed", UString::format("%d", getTopSpeed())},
	    {"Acceleration", UString::format("%d", getAcceleration())},
	    {"Weight", UString::format("%d", getWeight())},
	    {"Fuel", UString::format("%d", getFuel())},
	    {"Passengers", UString::format("%d/%d", getPassengers(), getMaxPassengers())},
	    {"Cargo", UString::format("%d/%d", getCargo(), getMaxCargo())},
	};
}

int Vehicle::getMaxConstitution() const
{
	return static_cast<int>(this->getMaxHealth() + this->getMaxShield());
}

int Vehicle::getConstitution() const
{
	return static_cast<int>(this->getHealth() + this->getShield());
}

int Vehicle::getMaxHealth() const { return static_cast<int>(this->type->health); }

int Vehicle::getHealth() const { return static_cast<int>(this->health); }

int Vehicle::getMaxShield() const
{
	int maxShield = 0;

	for (auto &item : equipment.general())
	{
		maxShield += item->type->shielding;
	}

	return maxShield;
}

int Vehicle::getShield() const { return this->shield; }

int Vehicle::getArmor() const
{
	int armor = 0;
	// FIXME: Check this the sum of all directions
	for (auto &armorDirection : this->type->armour)
	{
		armor += static_cast<int>(armorDirection.second);
	}
	return armor;
}

int Vehicle::getAccuracy() const
{
	int accuracy = 0;
	std::priority_queue<int> accModifiers;

	for (auto &item : equipment.general())
	{
		if (item->type->accuracy_modifier <= 0)
			continue;
		// accuracy percentages are inverted in the data (e.g. 10% module gives 90)
		accModifiers.push(100 - item->type->accuracy_modifier);
	}

	double moduleEfficiency = 1.0;
	while (!accModifiers.empty())
	{
		accuracy += static_cast<int>(accModifiers.top() * moduleEfficiency);
		moduleEfficiency /= 2;
		accModifiers.pop();
	}
	return accuracy;
}

// FIXME: Check int/float speed conversions
int Vehicle::getTopSpeed() const { return static_cast<int>(this->getSpeed()); }

int Vehicle::getAcceleration() const
{
	// FIXME: This is somehow related to enginer 'power' and weight
	int weight = this->getWeight();
	int acceleration = static_cast<int>(this->type->acceleration);
	int power = 0;
	for (auto &item : equipment.engines())
	{
		power += item->type->power;
	}
	if (weight == 0)
	{
		LogError("Vehicle %s has zero weight", this->name.cStr());
		return 0;
	}
	acceleration += std::max(1, power / weight);

	if (power == 0 && acceleration == 0)
	{
		// No engine shows a '0' acceleration in the stats ui
		return 0;
	}
	return acceleration;
}

int Vehicle::getWeight() const
{
	int weight = static_cast<int>(this->type->weight);

	for (auto it = equipment.cbegin(); it != equipment.cend(); ++it)
	{
		weight += static_cast<int>((*it)->type->weight);
	}
	if (weight == 0)
	{
		LogError("Vehicle with no weight");
	}
	return weight;
}

int Vehicle::getFuel() const
{
	// Zero fuel is normal on some vehicles (IE ufos/'dimension-capable' xcom)
	int fuel = 0;

	for (auto &item : equipment.engines())
	{
		fuel += item->type->max_ammo;
	}

	return fuel;
}

int Vehicle::getMaxPassengers() const
{
	int passengers = this->type->passengers;

	for (auto &item : equipment.general())
	{
		passengers += item->type->passengers;
	}
	return passengers;
}

int Vehicle::getPassengers() const
{ // FIXME: Track passengers
	return 0;
}

int Vehicle::getMaxCargo() const
{
	int cargo = 0;

	for (auto &item : equipment.general())
	{
		cargo += item->type->cargo_space;
	}
	return cargo;
}

int Vehicle::getCargo() const
{ // FIXME: Track cargo
	return 0;
}

void Vehicle::addEquipment(GameState &state, Vec2<int> slotOrigin, StateRef<VEquipmentType> type)
{
	// We can't check this here, as some of the non-buyable vehicles have insane initial equipment
	// layouts
	// if (!this->canAddEquipment(pos, type))
	//{
	//	LogError("Trying to add \"%s\" at {%d,%d} on vehicle \"%s\" failed", type.id.cStr(), pos.x,
	//	         pos.y, this->name.cStr());
	//}
	switch (type->type)
	{
		case VEquipmentType::VEquipmentClass::Engine:
		{
			auto engine = mksp<VEquipment>();
			engine->type = type;
			this->equipment.addEquipment(engine);
			engine->equippedPosition = slotOrigin;
			LogInfo("Equipped \"%s\" with engine \"%s\"", this->name.cStr(), type->name.cStr());
			break;
		}
		case VEquipmentType::VEquipmentClass::Weapon:
		{
			auto thisRef = StateRef<Vehicle>(&state, shared_from_this());
			auto weapon = mksp<VEquipment>();
			weapon->type = type;
			weapon->owner = thisRef;
			weapon->ammo = type->max_ammo;
			this->equipment.addEquipment(weapon);
			weapon->equippedPosition = slotOrigin;
			LogInfo("Equipped \"%s\" with weapon \"%s\"", this->name.cStr(), type->name.cStr());
			break;
		}
		case VEquipmentType::VEquipmentClass::General:
		{
			auto equipment = mksp<VEquipment>();
			equipment->type = type;
			LogInfo("Equipped \"%s\" with general equipment \"%s\"", this->name.cStr(),
			        type->name.cStr());
			equipment->equippedPosition = slotOrigin;
			this->equipment.addEquipment(equipment);
			break;
		}
		default:
			LogError("Equipment \"%s\" for \"%s\" at pos (%d,%d} has invalid type",
			         type->name.cStr(), this->name.cStr(), slotOrigin.x, slotOrigin.y);
	}
}

void Vehicle::removeEquipment(sp<VEquipment> object)
{
	this->equipment.removeEquipment(object);
	// TODO: Any other variable values here?
	// Clamp shield
	if (this->shield > this->getMaxShield())
	{
		this->shield = this->getMaxShield();
	}
}

std::list<EquipmentSlot> Vehicle::getEquipmentSlots() { return type->equipment_layout_slots; }

sp<Image> Vehicle::getEqScreenBackgound() const { return type->equipment_screen; }

sp<Image> Vehicle::getImage() const { return type->equip_icon_big; }

sp<Image> Vehicle::getImageSmall() const { return type->equip_icon_small; }

const UString &Vehicle::getName() const { return name; }

const UString &Vehicle::getTName() const { return type->name; }

Inventory &Vehicle::getInventory() { return equipment; }

const EquipmentUserType Vehicle::getUserType() const
{
	switch (type->type)
	{
		case VehicleType::Type::Flying:
			return EquipmentUserType::Air;
		case VehicleType::Type::Ground:
			return EquipmentUserType::Ground;
		default:
			LogError(
			    "Trying to draw equipment screen of unsupported vehicle type for vehicle \"%s\"",
			    getName().cStr());
			return EquipmentUserType::Air;
	}
}

void Vehicle::equipDefaultEquipment(GameState &state)
{
	LogInfo("Equipping \"%s\" with default equipment", this->type->name.cStr());
	for (auto &pair : this->type->initial_equipment_list)
	{
		auto &pos = pair.first;
		auto &etype = pair.second;

		this->addEquipment(state, pos, etype);
	}
}

template <> sp<Vehicle> StateObject<Vehicle>::get(const GameState &state, const UString &id)
{
	auto it = state.vehicles.find(id);
	if (it == state.vehicles.end())
	{
		LogError("No vehicle matching ID \"%s\"", id.cStr());
		return nullptr;
	}
	return it->second;
}

sp<EquipmentStore> Vehicle::getStore(sp<GameState> state)
{
	StateRef<Base> base = nullptr;
	for (auto &b : state->player_bases)
	{
		if (b.second->building == this->currentlyLandedBuilding)
			base = {state.get(), b.first};
	}

	if (base != nullptr)
	{
		return std::static_pointer_cast<EquipmentStore>(mksp<BaseEquipmentStore>(base, state));
	}

	// airbone or non-base building
	return std::static_pointer_cast<EquipmentStore>(mksp<LockedEquipmentStore>());
}

}; // namespace OpenApoc
