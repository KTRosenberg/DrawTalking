#include "dump.h"

namespace flecs {

// Utility for printing indentation
void dump_indent(int count) {
    for (int i = 0; i < count * 2; i ++) {
        std::cout << " ";
    }
}

// Dump an entity
void dump(flecs::entity e, int indent, bool is_instanceof) {
    flecs::world ecs = e.world();

    auto v = e.type().vector();
    size_t count = v.count();

    if (is_instanceof) {
        std::cout << "Instanceof | " << e.path(".", "") << std::endl;
    } else {
        std::cout << "====================================" << std::endl; 
        std::cout << " " << e.path(".", "") << std::endl;
        std::cout << "------------------------------------" << std::endl;
    }

    indent ++;

    // Iterate type back to front so that Instanceof roles appear on top
    for (unsigned long long i = count - 1; i-- > 0; ) {
        auto id = v[i];
        auto comp = ecs.entity(id);

        dump_indent(indent);
        std::cout << "- ";

        // Instanceof
        if (comp.has_role(flecs::Instanceof)) {
            dump(comp.remove_role(), indent, true);
        } else

        // Parent
        if (comp.has_role(flecs::Childof)) {
            std::cout << "Childof | " << comp.remove_role().path(".", "") << std::endl;
        } else

        // Trait
        if (comp.has_role(flecs::Trait)) {
            flecs::entity hi = comp.remove_role().hi();
            flecs::entity lo = comp.lo();
            std::cout << "Trait | " << hi.path(".", "") << " > " << lo.path(".", "") << std::endl;
        } else

        // Switch
        if (comp.has_role(flecs::Switch)) {
            std::cout << "Switch | " << comp.remove_role().path(".", "") << std::endl;

        // Regular component
        } else {
            std::cout << comp.path(".", "") << std::endl;
        }
    }

    indent --;
    
    if (!is_instanceof) {
        std::cout << "------------------------------------" 
            << std::endl << std::endl;
    }
}

// Dump the currently iterated over value
void dump(flecs::iter it) {
    std::cout << "====================================" << std::endl; 
    std::cout << " Table [" << it.table_type().str() << "]" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    std::cout << " Iterated by:  " << it.system().path(".", "") << std::endl;
    std::cout << " Entity count: " << it.count() << std::endl;
    std::cout << " Delta time  : " << it.delta_time() << std::endl;
    std::cout << "------------------------------------" << std::endl;

    // Print information about each system column
    for (int i = 0; i < it.column_count(); i ++) {
        std::cout << " Column " << it.column_entity(i + 1).path(".", "") << std::endl;
        std::cout << "  - source:   " << it.column_source(i + 1).path(".", "") << std::endl;
        std::cout << "  - shared:   " << (it.is_shared(i + 1) ? "true" : "false") << std::endl;
        std::cout << "  - readonly: " << (it.is_readonly(i + 1) ? "true" : "false") << std::endl;
        std::cout << "  - is set:   " << (it.is_set(i + 1) ? "true" : "false") << std::endl;
        std::cout << "  - size:     " << it.column_size(i + 1) << std::endl;
        std::cout << std::endl;
    }

    std::cout << "------------------------------------" 
            << std::endl << std::endl;
}

}
