#ifndef PACKAGE_H
#define PACKAGE_H

#include <stdbool.h>
#include <stddef.h>

// package management types
typedef enum {
    PACKAGE_TYPE_BINARY,
    PACKAGE_TYPE_LIBRARY
} PackageType;

typedef enum {
    DEPENDENCY_TYPE_REGULAR,
    DEPENDENCY_TYPE_DEV,
    DEPENDENCY_TYPE_BUILD
} DependencyType;

typedef struct Package {
    char* name;
    char* version;
    char* description;
    char* authors;
    char* license;
    PackageType type;
    char* source_path;
    char* target_path;
    char* manifest_path;
} Package;

typedef struct Dependency {
    char* name;
    char* version;
    char* source;
    DependencyType type;
    bool optional;
} Dependency;

typedef struct PackageManager {
    char* workspace_path;
    char* target_directory;
    char* cache_directory;
    Package* current_package;
    Package** dependencies;
    size_t dependency_count;
    size_t dependency_capacity;
} PackageManager;

// package management functions
PackageManager* package_manager_init(const char* workspace_path);
void package_manager_free(PackageManager* pm);

Package* package_create(const char* name, const char* version);
void package_free(Package* pkg);

bool package_parse_manifest(const char* manifest_path, Package* pkg);
bool package_write_manifest(const char* manifest_path, Package* pkg);

Dependency* dependency_create(const char* name, const char* version, DependencyType type);
void dependency_free(Dependency* dep);

bool package_add_dependency(PackageManager* pm, const char* name, const char* version, DependencyType type);
bool package_remove_dependency(PackageManager* pm, const char* name);

bool package_build(PackageManager* pm);
bool package_test(PackageManager* pm);
bool package_run(PackageManager* pm);

bool package_install_dependencies(PackageManager* pm);
bool package_update_dependencies(PackageManager* pm);

char* package_resolve_path(const char* package_name, const char* version);
bool package_validate_manifest(const char* manifest_path);

// cargo-like commands
bool cargo_new(const char* project_name, PackageType type);
bool cargo_build(const char* project_path);
bool cargo_test(const char* project_path);
bool cargo_run(const char* project_path);
bool cargo_add(const char* project_path, const char* dependency, const char* version);
bool cargo_remove(const char* project_path, const char* dependency);
bool cargo_update(const char* project_path);
bool cargo_install(const char* package_name);

// dependency resolution
bool resolve_dependency_tree(PackageManager* pm);
bool check_dependency_conflicts(PackageManager* pm);
char* find_package_version(const char* package_name, const char* version_constraint);

// workspace management
bool create_workspace(const char* workspace_path);
bool add_package_to_workspace(const char* workspace_path, const char* package_path);
bool remove_package_from_workspace(const char* workspace_path, const char* package_name);

#endif // PACKAGE_H 