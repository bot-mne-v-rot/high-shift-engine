# High Shift Engine

A pure OpenGL game engine that uses ECS architectural pattern for entities and resources management.

## ECS

This game engine features ergonomic archetypes-based entity-component-system implementation.

### Short description

The essence of the ECS pattern is in the following:
- Behavior is separated from data and encapsulated in systems
- Data is split into components
- Systems operate on components
- Entities are bunches of components

### Resources

There are global resources stored in the `World` that the systems can access. For example, `input::Input` resource allows you to read the input from the mouse and the keyboard.

### Systems

The system is declared as a struct that has an `update` method:
```c++
struct SomeSystem {
    void update(SomeResource &a, const OtherResource &b) {
        // You list all the resources you need as arguments.
        
        // You can access a resource by, possibly const, lvalue-reference.
        
        // If the resource does not exist, it is automatically created.
    }
};
```

If the system needs setup you can add a `setup` method:
```c++
struct SomeSystem {
    void setup(ecs::World &world, SomeOtherSystem &other_system) {
        // The first argument must be `world` which 
        // allows the system to create or access resources.
        
        // Other arguments are systems on which your system depends.
        // Systems are initialized and updated in the topological order.
        
        // If the system setup may fail, return 
        // `tl::expected<void, std::string> instead of `void`.
    }
    
    void update(SomeResource &a, const OtherResource &b) {
        // ...
    }
};
```

If your system needs to release some resources before having its destructor called, you can add `teardown` method:
```c++
struct SomeSystem {
    void setup(ecs::World &world, SomeOtherSystem &other_system) {
        // ...
    }
    
    void update(SomeResource &a, const OtherResource &b) {
        // ...
    }
    
    void teardown(ecs::World &world) {
        // This method only receives `ecs::World` and can't fail.
    }
};
```

### Components

Components are just plain-old data structs which store entities data. Formally, they are trivially-destructible copy-constructible non-fundamental types. For example:
```c++
struct Transform {
    glm::vec3 position;
    glm::quat rotation;
};

struct RigidBody {
    glm::vec3 velocity;
    glm::vec3 acceleration;
};

struct MeshRenderer {
    Handle<Model> model_handle;
    Handle<ShaderProgram> shader_program_handle;
};
```

### Entities

You can create, read, update and delete entities via the special `ecs::Entities` resource. It can be requested in the system as any other resource.

Its main feature is that you can iterate over entities that have a particular resource:
```c++
struct PhysicsSystem {
    void update(ecs::Entities &entities, const ecs::DeltaTime &dt) {
        entities.foreach([&](Transform &transform, RigidBody &rigid_body) {
            rigid_body.velocity += rigid_body.acceleration * dt();
            transform.position += rigid_body.velocity * dt();
        })
    }
};
```
In this example, `foreach` method calls the lambda on each entity that has both `Transform` and `RigidBody` components.

If you also need to get the entity itself, just list it as any argument:
```c++
entites.foreach([&](ecs::Entity entity, /* ... components */) {
    // ...
});
```

### Dispatcher

Dispatcher wires everything together. You can use a builder to create it:
```c++
auto dispatcher_result = ecs::DispatcherBuilder()
        .add_system<render::WindowSystem>(render::WindowArgs{
            .width = 800, .height = 600, .title = "README example"
        })
        .add_system<render::RenderSystem>()
        .add_system<input::InputSystem>()
        .add_system<MainCameraSystem>()
        .build();

assert(dispatcher_result);
ecs::Dispatcher dispatcher = std::move(dispatcher_result.value());
```

The dispatcher gives you access to the world which means that you can access resources and entities:

```c++
auto &entities = dispatcher.get_world().get<ecs::Entities>();
entities.create(
        render::Transform{
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::quat(glm::vec3(0.f, glm::radians(-90.f), 0.f))
        },
        render::MeshRenderer{model, shader_program}
);
```

When you have set up everything, you can enter the game-loop:
```c++
dispatcher.loop();
```

## Building

All the libraries are either git submodules or directly included in the sources. Single `CMakeLists.txt` controls the whole build process. Therefore, the build process is straightforward:
```bash
$ git submodule init
$ mkdir build
$ cd build
build/$ cmake ..
build/$ make
```

## Using

The main build artefact is a single static library which contains the game engine and all the dependencies. Take a look into an [example CMakeLists.txt](examples/lighting/CMakeLists.txt).