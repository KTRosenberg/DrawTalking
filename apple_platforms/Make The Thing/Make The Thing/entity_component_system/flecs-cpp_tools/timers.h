#ifndef FLECS_TIMERS_H
#define FLECS_TIMERS_H

#include "flecs.h"

namespace flecs {

// Trait that adds a component after t seconds
struct AddTimer {
    float timeout;
    float t;
};

// Trait that remove a component after t seconds
struct RemoveTimer {
    float timeout;
    float t;
};

// Trait that deletes entity after t seconds
struct DeleteTimer  {
    float timeout;
    float t;
};

// Module implementation
class timers {
public:
    timers(flecs::world& ecs) {
        ecs.module<flecs::timers>();

        // Forward declare traits so they can be used in signatures
        ecs.component<AddTimer>();
        ecs.component<RemoveTimer>();

        ecs.system<>(nullptr, "TRAIT | AddTimer")
            .action([](flecs::iter it) {
                // Get the timer component
                auto timer = it.column<AddTimer>(1);

                // Get the handle to the trait. This contains information about
                // the component the trait is applied to.
                flecs::entity trait = it.column_entity(1);
                flecs::entity comp = trait.lo(); // Component id

                for (auto i : it) {
                    // Increase timer. When it equals timeout, add component
                    timer[i].t += it.delta_time();
                    if (timer[i].t >= timer[i].timeout) {
                        // Remove trait so that it won't keep triggering for
                        // this entity.
                        it.entity(i).remove(trait);               
                        it.entity(i).add(comp); // Add the component
                    }
                }
            });

        ecs.system<>(nullptr, "TRAIT | RemoveTimer")
            .action([](flecs::iter it) {
                // Get the timer component
                auto timer = it.column<AddTimer>(1);

                // Get the handle to the trait. This contains information about
                // the component the trait is applied to.
                flecs::entity trait = it.column_entity(1);
                flecs::entity comp = trait.lo(); // Component id

                for (auto i : it) {
                    // Increase timer. When it equals timeout, remove component
                    timer[i].t += it.delta_time();
                    if (timer[i].t >= timer[i].timeout) {
                        // Remove trait so that it won't keep triggering for
                        // this entity.
                        it.entity(i).remove(trait);               
                        it.entity(i).remove(comp); // Remove the component
                    }
                }
            });   

        ecs.system<DeleteTimer>()
            .each([](flecs::entity e, DeleteTimer& timer) {
                timer.t += e.delta_time();
                if (timer.t >= timer.timeout) {            
                    e.destruct(); // Delete entity
                }
            });             
    }
};

}

#endif
