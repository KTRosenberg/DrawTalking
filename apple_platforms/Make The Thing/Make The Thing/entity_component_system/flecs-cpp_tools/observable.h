#ifndef FLECS_OBSERVABLE_H
#define FLECS_OBSERVABLE_H

#include "flecs.h"
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace flecs {

// Observer callback type
template <typename T>
using observer_func = std::function<void(flecs::entity, const T&)>;

// Observer data that is stored in list of observers
struct observer_data {
    void *ctx;
    void(*invoke)(flecs::entity e, void *ptr, void *ctx);
};

// Trait that stores a list of observers
struct Observable {
    std::unordered_map<entity_t, observer_data> observers;
};

// Observer context data, responsible for reintroducing type safety
template <typename T>
class observer_mgr {
public:
    observer_mgr(const observer_func<T>& func) 
        : m_func(func)
        , m_id(0)
        , m_world(nullptr)
        , m_disabled(false) { }

    ~observer_mgr() {
        clear_observables();
    }

    // Start observing entity
    void add_observable(flecs::entity e) {
        // Only start observing if the entity wasn't already being observed
        if (m_observables.insert(e.id()).second) {
            add_observable_trait(e);
        }
    }

    // Stop observing entity
    void remove_observable(flecs::entity e) {
        if (!m_id) {
            return;
        }
        remove_observable_trait(e);
        m_observables.erase(e.id());
    }

    // Stop observing all observables
    void clear_observables() {
        for (auto e : m_observables) {
            remove_observable_trait(flecs::entity(m_world, e));
        }
        m_observables.clear();
    }

    // Enable observer
    void enable() {
        // Add self to observables
        for (auto e : m_observables) {
            add_observable_trait(flecs::entity(m_world, e));
        }
    }

    // Disable observer
    void disable() {
        // Remove self from observables
        for (auto e : m_observables) {
            remove_observable_trait(flecs::entity(m_world, e));
        }
    }

    // Static function that can be stored in observer data
    static void invoke(flecs::entity e, void *ptr, void *ctx) {
        // Instance of self is stored in observer context
        observer_mgr<T> *self = static_cast<observer_mgr<T>*>(ctx);
        self->m_func(e, static_cast<T*>(ptr)[0]);
    }
private:
    void add_observable_trait(flecs::entity e) {
        if (!m_id) {
            // Create a unique id for the observer, so we can store it in a 
            // map. Register the id with the invoker, so that the observer 
            // object can be moved around on the stack.
            m_world = e.world().c_ptr();
            m_id = ecs_new_id(m_world);
        }

        // Create the observer data that will be added to the list of observers
        // of the observable.
        observer_data data;
        data.ctx = this;
        data.invoke = observer_mgr<T>::invoke;
        
        // Add the Observable trait for the type of the observer
        Observable *o = e.get_trait_mut<Observable, T>();

        // Add the observer to the list
        o->observers[m_id] = data;
    }

    void remove_observable_trait(flecs::entity e) {
        Observable *o = e.get_trait_mut<Observable, T>();
        o->observers.erase(m_id);

        // If observable has no more observers, remove trait
        if (o->observers.empty()) {
            e.remove_trait<Observable, T>();
        }
    }

    const observer_func<T>& m_func;
    std::unordered_set<entity_t> m_observables;
    entity_t m_id;
    world_t *m_world;
    bool m_disabled;
};

// Typed observer class which allows for observing multiple entities
template<typename T>
class observer final {
public:
    observer(observer_func<T> func) 
        : m_func(func)
        , m_mgr(new observer_mgr<T>(m_func)) { }

    ~observer() {
        delete m_mgr;
    }

    void observe(flecs::entity observable) {
        m_mgr->add_observable(observable);
    }

    void unobserve(flecs::entity observable) {
        m_mgr->remove_observable(observable);
    }

    void clear() {
        m_mgr->clear_observables();
    }

    void enable() {
        m_mgr->enable();
    }

    void disable() {
        m_mgr->disable();
    }

private:
    const observer_func<T> m_func;
    observer_mgr<T>* m_mgr;
};

// Module implementation
class observable {
public:
    observable(flecs::world& ecs) {
        ecs.module<flecs::observable>();

        // Register component so it can be accessed by name in signature
        ecs.component<Observable>();

        // Invoke observers when component is set.
        // This system subscribes for the Observable trait which stores the list
        // of observers, in addition to the component to which the trait is
        // applied (the wildcard). The latter needs to be part of the signature
        // or the system would not trigger when the component is set. 
        //
        // By using traits instead of a regular OnSet system we ensure that only
        // entities with the Observable trait trigger the system. Without the
        // trait, updates from any entity would trigger the system.
        ecs.system<>("ObserverDispatch", "TRAIT | Observable, TRAIT | Observable > *")
            .kind(flecs::OnSet)
            .action([](flecs::iter it) {
                // List of observers
                auto observables = it.column<Observable>(1);

                // The component data. Since we don't know the type of the 
                // component at compile time, we need to use an untyped column.
                auto data = it.column(2);

                // It is possible that multiple observable entities were updated
                for (auto i : it) {
                    // Iterate observers, pass data to each one
                    for (auto observer : observables[i].observers) {
                        observer.second.invoke( 
                                it.entity(i), data[i], observer.second.ctx ); 
                    }
                }
            });
    }
};

}

#endif
