// vim: ts=2:sw=2:noexpandtab

#include "../diana.h"

#include <map>
#include <typeinfo>
#include <string>
#include <cstdlib>
#include <cassert>

namespace Diana {

class Component {
public:
	virtual unsigned int componentFlags() const { return DL_COMPONENT_FLAG_INLINE; }
};

class Entity;
class System;
class Manager;

class World {
public:
	World(void *(*malloc)(size_t) = malloc, void (*free)(void *) = free);

	template<class T>
	unsigned int registerComponent() {
		const std::type_info * tid = &typeid(T);
		if(components.count(tid) == 0) {
			T x;
			components[tid] = 0;
			diana_createComponent(diana, tid->name(), sizeof(T), x.componentFlags(), &components[tid]);
		}
		return components[tid];
	}

	template<class T>
	unsigned int getComponentId() {
		const std::type_info * tid = &typeid(T);

		return components[tid];
	}

	void registerSystem(System *system);
	void registerManager(Manager *manager);

	void initialize();

	Entity spawn();

	void process(float delta);

	struct diana *getDiana() { return diana; }

private:
	struct diana *diana;
	std::map<const std::type_info *, unsigned int> components;
};

class Entity {
public:
	Entity(World *world, unsigned int id) : _world(world), _id(id) { }

	template<class T>
	void setComponent(const T * data = NULL) {
		unsigned int cid = _world->getComponentId<T>();
		diana_setComponent(_world->getDiana(), _id, cid, data);
	}

	template<class T>
	void setComponent(const T & data) {
		setComponent(&data);
	}

    template<class T>
    void addComponent(const T * data = NULL) {
        unsigned int cid = _world->getComponentId<T>();
        diana_appendComponent(_world->getDiana(), _id, cid, data);
    }

    template<class T>
    void addComponent(const T & data) {
        addComponent(&data);
    }

	template<class T>
	T *getComponent() {
		unsigned int cid = _world->getComponentId<T>();
		T * ptr;
		diana_getComponent(_world->getDiana(), _id, cid, (void **)&ptr);
		return ptr;
	}

    template<class T>
    T *getComponent(unsigned int index) {
        unsigned int cid = _world->getComponentId<T>();
        T * ptr;

        unsigned int count = 0;
        diana_getComponentCount(_world->getDiana(), _id, cid, &count);
        assert(index < count);

        diana_getComponentI(_world->getDiana(), _id, cid, index, (void **)&ptr);
        return ptr;
    }

    template<class T>
    unsigned int getComponentCount() {
        unsigned int cid = _world->getComponentId<T>();
        unsigned int count = 0;
        diana_getComponentCount(_world->getDiana(), _id, cid, &count);
        return count;
    }

    template<class T>
    void removeComponent() {
        unsigned int cid = _world->getComponentId<T>();
        diana_removeComponents(_world->getDiana(), _id, cid);
    }

    template<class T>
    void removeComponent(unsigned int index) {
        unsigned int cid = _world->getComponentId<T>();
        
        unsigned int count = 0;
        diana_getComponentCount(_world->getDiana(), _id, cid, &count);
        assert(index < count);
        
        diana_removeComponentI(_world->getDiana(), _id, cid, index);
    }

	void add();
	void enable();
	void disable();
	void remove();

	unsigned int getId() { return _id; }

private:
	World * _world;
	unsigned int _id;
};

class System {
public:
	System(const char *name) : _name(name) { }

	void setWorld(World *);
	World *getWorld() { return _world; }
	unsigned int getId() { return _id; }

	virtual void addWatches() { }
	virtual unsigned int systemFlags() const { return DL_SYSTEM_FLAG_NORMAL; }

	virtual void starting() { }
	virtual void process(Entity &entity, float delta) { }
	virtual void ending() { }

	virtual void subscribed(Entity &entity) { }
	virtual void unsubscribed(Entity &entity) { }

	template<class T>
	void watch() {
		unsigned int cid = _world->registerComponent<T>();
		diana_watch(_world->getDiana(), _id, cid);
	}

	template<class T>
	void exclude() {
		unsigned int cid = _world->registerComponent<T>();
		diana_exclude(_world->getDiana(), _id, cid);
	}

private:
	std::string _name;
	World * _world;
	unsigned int _id;
};

class Manager {
public:
	Manager(const char *name) : _name(name) { }

	void setWorld(World *);
	World *getWorld() { return _world; }
	unsigned int getId() { return _id; }

	virtual unsigned int managerFlags() const { return DL_MANAGER_FLAG_NORMAL; }

	virtual void added(Entity &entity) { }
	virtual void enabled(Entity &entity) { }
	virtual void disabled(Entity &entity) { }
	virtual void deleted(Entity &entity) { }

private:
	std::string _name;
	World * _world;
	unsigned int _id;
};

};
