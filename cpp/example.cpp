// vim: ts=2:sw=2:noexpandtab

#include "diana.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <thread>

using namespace Diana;

class Position : public Component {
public:
	float x, y;
};

class Joint : public Component {
public:
    Joint(int x = 0)
        : id(x)
    {
    }

    int id;

    virtual unsigned int componentFlags() const { return DL_COMPONENT_FLAG_MULTIPLE; }
};

class Velocity : public Component {
public:
	Velocity() : x(0), y(0) { }
	Velocity(float _x, float _y) : x(_x), y(_y) { }

	float x, y;
};

class Renderer : public Component {
public:
	int c;
};

class MovementSystem : public System {
public:
	MovementSystem() : System("Render System") { }

	virtual void addWatches() final {
		watch<Position>();
		watch<Velocity>();
	}

	virtual void process(Entity & entity, float delta) final {
		Position * position = entity.getComponent<Position>();
		Velocity * velocity = entity.getComponent<Velocity>();

		position->x += velocity->x * delta;
		position->y += velocity->y * delta;

		printf("%i move to (%f,%f)\n", entity.getId(), position->x, position->y);
	}
};

class RenderSystem : public System {
public:
	RenderSystem() : System("Render System") { }

	virtual void addWatches() final {
		watch<Position>();
		watch<Renderer>();
        watch<Joint>();
	}

	virtual void process(Entity & entity, float delta) final {
		Position * position = entity.getComponent<Position>();
		Renderer * renderer = entity.getComponent<Renderer>();

        Joint* j = entity.getComponent<Joint>(0);
        Joint* j2 = entity.getComponent<Joint>(1);

        printf("%i has joints with IDs: (%i, %i)\n", entity.getId(), j->id, j2->id);
	}
};

class EntitySpawnManager
    : public Diana::Manager
{
public:
    EntitySpawnManager()
        : Diana::Manager("SpawnManager")
    {
    }

    virtual void added(Entity &entity) final {
        printf("Entity spawned with ID: %i\n", entity.getId());
    }

    virtual void deleted(Entity &entity) final { 
        printf("Entity deleted with ID: %i\n", entity.getId());
    }
};

int main() {
	World *world = new World();

	world->registerSystem(new MovementSystem());
	world->registerSystem(new RenderSystem());

    world->registerManager(new EntitySpawnManager());

	world->initialize();

	Entity e = world->spawn();
	e.addComponent<Position>();
	e.addComponent(Velocity(1.5, 0));
	e.add();

	Entity e1 = world->spawn();
	e1.addComponent<Position>();
	e1.addComponent<Renderer>();
    e1.addComponent<Joint>();
    e1.addComponent<Joint>(1);
	e1.add();

	while(1) {
		// 30 fps
		world->process(1.0/30.0);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

