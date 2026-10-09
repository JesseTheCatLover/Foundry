# Foundry

**Foundry is RedleafEngine's unified build system.**

*Multiple projects. Multiple toolchains. One decent build workflow.*

Foundry brings modules, dependencies, and build tools together under a unified system for configuring and building multi-language software projects.

Whether your project uses CMake, Cargo, Python, or a combination of tools, Foundry aims to make the entire build workflow easier to manage.

## Why Foundry?

Modern applications rarely rely on a single language or toolchain.
Coordinating their dependencies and build processes can leave developers maintaining disconnected configurations and scripts.

Foundry brings these relationships together.

- **Unify your workflow.** Configure, inspect, and build through one consistent interface.
- **Understand your project.** Keep modules, dependencies, and build relationships connected.
- **Reuse your infrastructure.** Share build logic and conventions across projects.
- **Keep your tools.** Integrate existing toolchains without forcing everything into one technology.

## Projects, Modules & Dependencies

Foundry models software as interconnected projects and modules.

A **project** contains modules that work together to form a software product or library. Modules can depend on other modules, while projects can depend on independently developed projects.

This creates two levels of composition:

- **Module dependencies** connect components within a project.
- **Project dependencies** connect independent projects, allowing one project to consume modules exposed by another.

For example, an engine might contain its runtime, editor, and launch modules while depending on a separate UI framework project.

```text
RedleafEngine (Project)
├── Engine (Module)
├── EditorRuntime (Module)
│   └── depends on Engine
├── Editor (Module)
│   └── depends on EditorRuntime
└── Launch (Module)
    └── depends on Engine

Silverware (Project)
└── UI (Module)

RedleafEngine (Project)
└── depends on Silverware
```

The two projects retain their own identities and source trees. Foundry's project dependency model connects them without requiring their contents to be merged.

This also establishes an important distinction: a project defines its modules, while dependencies describe how those modules and projects relate to one another.

## One Project, Multiple Toolchains

Foundry coordinates the tools your software already needs.

| Component | Toolchain | Purpose |
|---|---|---|
| Core library | C++ / CMake | Application functionality |
| Code generator | Rust / Cargo | Generate source code |
| Asset processor | Python | Process application assets |

These components may have different build requirements, but their outputs and dependencies can affect the same final application.

Foundry aims to coordinate their interactions and execution order while leaving each tool responsible for its specialized work.

## Overall Process

```text
.foundry files
      │
      ▼
Project Model
      │
      ▼
Dependency Resolution
      │
      ▼
Build Orchestration
      │
      ▼
CMake · Cargo · Python · Other Tools
```

## The `.foundry` Language

Foundry uses declarative `.foundry` files to describe projects, modules, and their configuration.

A project:

```cpp
# Project(MyApplication):

language(c, cxx);
cppversion(cxx20);
```

A module:

```cpp
# Module(Core):

type(static_library);

public_dep(glfw, glm, assimp);
```

Another module can declare a dependency on it:

```cpp
# Module(Editor):

type(static_library);

public_dep(Core);
```

Foundry is designed to keep common configuration concise while allowing more complex build behavior to be expressed through reusable functions and native build-system integration.

### Reusable Build Functions

Not every project needs the same build behavior. Instead of hardcoding every feature into Foundry, projects can define their own reusable functions.

A function definition begins with `$`:

```cpp
$ enable_reflection():

    // Reusable Foundry build logic.
```

Modules can invoke that function through an ordinary function call:

```cpp
# Module(Engine):

enable_reflection();
```

This separates the declaration of a module from the implementation of reusable build behavior.

For example, RedleafEngine could define `enable_reflection()` to integrate its reflection-generation workflow. Another project that doesn't use reflection wouldn't need that function at all.

Foundry provides the function mechanism; projects define the functionality they need. Functions can combine Foundry-level operations with native build-system configuration.

### Direct CMake Integration

When specialized CMake configuration is needed, a `.foundry` file can contain a `cmake` block:

```cpp
cmake
{
    target_compile_definitions(
        Engine
        PRIVATE
        HAVE_GLFW
        HAVE_OPENGL
    )
}
```

or include a CMake entry and integrating an existing CMake project:

```cpp
# ExternalDep(ThirdParty):

cmake_include(ThirdParty/CMakeLists.txt);
```

This provides two complementary approaches:

- **Foundry abstractions** for common, reusable build operations.
- **Native CMake blocks** for specialized configuration that doesn't need a dedicated abstraction.

The result is a higher-level build description without losing access to underlying CMake configurations.

## Development Status

Foundry is in early development. Its architecture and language are still evolving.

Planned capabilities include project and module discovery, dependency resolution, multi-language build orchestration, a command-line interface, reusable build functions, and IDE support for `.foundry` files.

Development will proceed incrementally, prioritizing a useful foundation before expanding into more advanced features.

Foundry is not yet ready for production use.

## License

To be determined.