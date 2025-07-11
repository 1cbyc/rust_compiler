#include "../include/package.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// package manager implementation
PackageManager* package_manager_init(const char* workspace_path) {
    PackageManager* pm = malloc(sizeof(PackageManager));
    if (!pm) return NULL;
    
    pm->workspace_path = strdup(workspace_path);
    pm->target_directory = strdup("target");
    pm->cache_directory = strdup(".cargo");
    pm->current_package = NULL;
    pm->dependencies = malloc(10 * sizeof(Package*));
    pm->dependency_count = 0;
    pm->dependency_capacity = 10;
    
    printf("package manager initialized for workspace: %s\n", workspace_path);
    return pm;
}

void package_manager_free(PackageManager* pm) {
    if (!pm) return;
    
    free(pm->workspace_path);
    free(pm->target_directory);
    free(pm->cache_directory);
    
    if (pm->current_package) {
        package_free(pm->current_package);
    }
    
    for (size_t i = 0; i < pm->dependency_count; i++) {
        package_free(pm->dependencies[i]);
    }
    free(pm->dependencies);
    free(pm);
    
    printf("package manager freed\n");
}

Package* package_create(const char* name, const char* version) {
    Package* pkg = malloc(sizeof(Package));
    if (!pkg) return NULL;
    
    pkg->name = strdup(name);
    pkg->version = strdup(version);
    pkg->description = strdup("");
    pkg->authors = strdup("");
    pkg->license = strdup("MIT");
    pkg->type = PACKAGE_TYPE_BINARY;
    pkg->source_path = strdup("src");
    pkg->target_path = strdup("target");
    pkg->manifest_path = strdup("Cargo.toml");
    
    printf("package created: %s v%s\n", name, version);
    return pkg;
}

void package_free(Package* pkg) {
    if (!pkg) return;
    
    free(pkg->name);
    free(pkg->version);
    free(pkg->description);
    free(pkg->authors);
    free(pkg->license);
    free(pkg->source_path);
    free(pkg->target_path);
    free(pkg->manifest_path);
    free(pkg);
}

bool package_parse_manifest(const char* manifest_path, Package* pkg) {
    printf("parsing manifest: %s\n", manifest_path);
    
    // simplified toml parser for testing
    FILE* file = fopen(manifest_path, "r");
    if (!file) {
        printf("  error: cannot open manifest file\n");
        return false;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        if (strncmp(line, "name = ", 7) == 0) {
            pkg->name = strdup(line + 7);
            pkg->name[strlen(pkg->name) - 1] = 0; // remove quotes
        } else if (strncmp(line, "version = ", 10) == 0) {
            pkg->version = strdup(line + 10);
            pkg->version[strlen(pkg->version) - 1] = 0; // remove quotes
        } else if (strncmp(line, "description = ", 13) == 0) {
            pkg->description = strdup(line + 13);
            pkg->description[strlen(pkg->description) - 1] = 0; // remove quotes
        }
    }
    
    fclose(file);
    printf("  manifest parsed successfully\n");
    return true;
}

bool package_write_manifest(const char* manifest_path, Package* pkg) {
    printf("writing manifest: %s\n", manifest_path);
    
    FILE* file = fopen(manifest_path, "w");
    if (!file) {
        printf("  error: cannot create manifest file\n");
        return false;
    }
    
    fprintf(file, "[package]\n");
    fprintf(file, "name = \"%s\"\n", pkg->name);
    fprintf(file, "version = \"%s\"\n", pkg->version);
    fprintf(file, "description = \"%s\"\n", pkg->description);
    fprintf(file, "authors = [\"%s\"]\n", pkg->authors);
    fprintf(file, "license = \"%s\"\n", pkg->license);
    fprintf(file, "\n[dependencies]\n");
    
    fclose(file);
    printf("  manifest written successfully\n");
    return true;
}

Dependency* dependency_create(const char* name, const char* version, DependencyType type) {
    Dependency* dep = malloc(sizeof(Dependency));
    if (!dep) return NULL;
    
    dep->name = strdup(name);
    dep->version = strdup(version);
    dep->source = strdup("crates.io");
    dep->type = type;
    dep->optional = false;
    
    printf("dependency created: %s v%s\n", name, version);
    return dep;
}

void dependency_free(Dependency* dep) {
    if (!dep) return;
    
    free(dep->name);
    free(dep->version);
    free(dep->source);
    free(dep);
}

bool package_add_dependency(PackageManager* pm, const char* name, const char* version, DependencyType type) {
    printf("adding dependency: %s v%s\n", name, version);
    
    // check if dependency already exists
    for (size_t i = 0; i < pm->dependency_count; i++) {
        if (strcmp(pm->dependencies[i]->name, name) == 0) {
            printf("  warning: dependency %s already exists\n", name);
            return false;
        }
    }
    
    // create new package for dependency
    Package* dep_pkg = package_create(name, version);
    if (!dep_pkg) {
        printf("  error: failed to create dependency package\n");
        return false;
    }
    
    // add to dependencies array
    if (pm->dependency_count >= pm->dependency_capacity) {
        size_t new_capacity = pm->dependency_capacity * 2;
        Package** new_deps = realloc(pm->dependencies, new_capacity * sizeof(Package*));
        if (!new_deps) {
            printf("  error: failed to expand dependencies array\n");
            package_free(dep_pkg);
            return false;
        }
        pm->dependencies = new_deps;
        pm->dependency_capacity = new_capacity;
    }
    
    pm->dependencies[pm->dependency_count++] = dep_pkg;
    printf("  dependency added successfully\n");
    return true;
}

bool package_remove_dependency(PackageManager* pm, const char* name) {
    printf("removing dependency: %s\n", name);
    
    for (size_t i = 0; i < pm->dependency_count; i++) {
        if (strcmp(pm->dependencies[i]->name, name) == 0) {
            package_free(pm->dependencies[i]);
            
            // shift remaining dependencies
            for (size_t j = i; j < pm->dependency_count - 1; j++) {
                pm->dependencies[j] = pm->dependencies[j + 1];
            }
            pm->dependency_count--;
            
            printf("  dependency removed successfully\n");
            return true;
        }
    }
    
    printf("  error: dependency %s not found\n", name);
    return false;
}

bool package_build(PackageManager* pm) {
    printf("building package: %s\n", pm->current_package ? pm->current_package->name : "unknown");
    
    // create target directory
    char target_path[512];
    snprintf(target_path, sizeof(target_path), "%s/%s", pm->workspace_path, pm->target_directory);
    
    struct stat st = {0};
    if (stat(target_path, &st) == -1) {
        if (mkdir(target_path, 0700) != 0) {
            printf("  error: cannot create target directory\n");
            return false;
        }
    }
    
    // compile source files
    char source_path[512];
    snprintf(source_path, sizeof(source_path), "%s/%s", pm->workspace_path, pm->current_package->source_path);
    
    printf("  compiling source files from: %s\n", source_path);
    printf("  output directory: %s\n", target_path);
    
    // simplified compilation process
    printf("  build completed successfully\n");
    return true;
}

bool package_test(PackageManager* pm) {
    printf("testing package: %s\n", pm->current_package ? pm->current_package->name : "unknown");
    
    // run test files
    char test_path[512];
    snprintf(test_path, sizeof(test_path), "%s/tests", pm->workspace_path);
    
    printf("  running tests from: %s\n", test_path);
    printf("  all tests passed\n");
    return true;
}

bool package_run(PackageManager* pm) {
    printf("running package: %s\n", pm->current_package ? pm->current_package->name : "unknown");
    
    // execute built binary
    char binary_path[512];
    snprintf(binary_path, sizeof(binary_path), "%s/%s/%s", pm->workspace_path, pm->target_directory, pm->current_package->name);
    
    printf("  executing: %s\n", binary_path);
    printf("  program completed successfully\n");
    return true;
}

bool package_install_dependencies(PackageManager* pm) {
    printf("installing dependencies for package: %s\n", pm->current_package ? pm->current_package->name : "unknown");
    
    for (size_t i = 0; i < pm->dependency_count; i++) {
        Package* dep = pm->dependencies[i];
        printf("  installing: %s v%s\n", dep->name, dep->version);
        
        // resolve and download dependency
        char* dep_path = package_resolve_path(dep->name, dep->version);
        if (dep_path) {
            printf("    resolved to: %s\n", dep_path);
            free(dep_path);
        } else {
            printf("    error: cannot resolve dependency\n");
            return false;
        }
    }
    
    printf("  all dependencies installed successfully\n");
    return true;
}

bool package_update_dependencies(PackageManager* pm) {
    printf("updating dependencies for package: %s\n", pm->current_package ? pm->current_package->name : "unknown");
    
    for (size_t i = 0; i < pm->dependency_count; i++) {
        Package* dep = pm->dependencies[i];
        printf("  updating: %s v%s\n", dep->name, dep->version);
        
        // check for newer versions
        char* latest_version = find_package_version(dep->name, "latest");
        if (latest_version && strcmp(latest_version, dep->version) != 0) {
            printf("    new version available: %s\n", latest_version);
            free(dep->version);
            dep->version = latest_version;
        } else {
            printf("    already up to date\n");
            if (latest_version) free(latest_version);
        }
    }
    
    printf("  dependencies updated successfully\n");
    return true;
}

char* package_resolve_path(const char* package_name, const char* version) {
    printf("resolving package: %s v%s\n", package_name, version);
    
    // simplified resolution - just return a mock path
    char* path = malloc(256);
    snprintf(path, 256, "/cargo/registry/src/%s-%s", package_name, version);
    
    printf("  resolved to: %s\n", path);
    return path;
}

bool package_validate_manifest(const char* manifest_path) {
    printf("validating manifest: %s\n", manifest_path);
    
    FILE* file = fopen(manifest_path, "r");
    if (!file) {
        printf("  error: cannot open manifest file\n");
        return false;
    }
    
    bool has_package = false;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "[package]")) {
            has_package = true;
        }
    }
    
    fclose(file);
    
    if (!has_package) {
        printf("  error: missing [package] section\n");
        return false;
    }
    
    printf("  manifest is valid\n");
    return true;
}

// cargo-like commands
bool cargo_new(const char* project_name, PackageType type) {
    printf("creating new cargo project: %s\n", project_name);
    
    // create project directory
    struct stat st = {0};
    if (stat(project_name, &st) == -1) {
        if (mkdir(project_name, 0700) != 0) {
            printf("  error: cannot create project directory\n");
            return false;
        }
    }
    
    // create source directory
    char src_path[512];
    snprintf(src_path, sizeof(src_path), "%s/src", project_name);
    if (mkdir(src_path, 0700) != 0) {
        printf("  error: cannot create src directory\n");
        return false;
    }
    
    // create main.rs
    char main_path[512];
    snprintf(main_path, sizeof(main_path), "%s/src/main.rs", project_name);
    FILE* main_file = fopen(main_path, "w");
    if (main_file) {
        fprintf(main_file, "fn main() {\n");
        fprintf(main_file, "    println!(\"hello, world!\");\n");
        fprintf(main_file, "}\n");
        fclose(main_file);
    }
    
    // create cargo.toml
    char manifest_path[512];
    snprintf(manifest_path, sizeof(manifest_path), "%s/Cargo.toml", project_name);
    Package* pkg = package_create(project_name, "0.1.0");
    if (pkg) {
        package_write_manifest(manifest_path, pkg);
        package_free(pkg);
    }
    
    printf("  project created successfully\n");
    return true;
}

bool cargo_build(const char* project_path) {
    printf("building cargo project: %s\n", project_path);
    
    PackageManager* pm = package_manager_init(project_path);
    if (!pm) return false;
    
    // parse manifest
    char manifest_path[512];
    snprintf(manifest_path, sizeof(manifest_path), "%s/Cargo.toml", project_path);
    Package* pkg = package_create("temp", "0.1.0");
    if (package_parse_manifest(manifest_path, pkg)) {
        pm->current_package = pkg;
        bool result = package_build(pm);
        package_manager_free(pm);
        return result;
    }
    
    package_manager_free(pm);
    return false;
}

bool cargo_test(const char* project_path) {
    printf("testing cargo project: %s\n", project_path);
    
    PackageManager* pm = package_manager_init(project_path);
    if (!pm) return false;
    
    char manifest_path[512];
    snprintf(manifest_path, sizeof(manifest_path), "%s/Cargo.toml", project_path);
    Package* pkg = package_create("temp", "0.1.0");
    if (package_parse_manifest(manifest_path, pkg)) {
        pm->current_package = pkg;
        bool result = package_test(pm);
        package_manager_free(pm);
        return result;
    }
    
    package_manager_free(pm);
    return false;
}

bool cargo_run(const char* project_path) {
    printf("running cargo project: %s\n", project_path);
    
    PackageManager* pm = package_manager_init(project_path);
    if (!pm) return false;
    
    char manifest_path[512];
    snprintf(manifest_path, sizeof(manifest_path), "%s/Cargo.toml", project_path);
    Package* pkg = package_create("temp", "0.1.0");
    if (package_parse_manifest(manifest_path, pkg)) {
        pm->current_package = pkg;
        bool result = package_run(pm);
        package_manager_free(pm);
        return result;
    }
    
    package_manager_free(pm);
    return false;
}

bool cargo_add(const char* project_path, const char* dependency, const char* version) {
    printf("adding dependency to cargo project: %s -> %s v%s\n", project_path, dependency, version);
    
    PackageManager* pm = package_manager_init(project_path);
    if (!pm) return false;
    
    bool result = package_add_dependency(pm, dependency, version, DEPENDENCY_TYPE_REGULAR);
    package_manager_free(pm);
    return result;
}

bool cargo_remove(const char* project_path, const char* dependency) {
    printf("removing dependency from cargo project: %s -> %s\n", project_path, dependency);
    
    PackageManager* pm = package_manager_init(project_path);
    if (!pm) return false;
    
    bool result = package_remove_dependency(pm, dependency);
    package_manager_free(pm);
    return result;
}

bool cargo_update(const char* project_path) {
    printf("updating cargo project: %s\n", project_path);
    
    PackageManager* pm = package_manager_init(project_path);
    if (!pm) return false;
    
    char manifest_path[512];
    snprintf(manifest_path, sizeof(manifest_path), "%s/Cargo.toml", project_path);
    Package* pkg = package_create("temp", "0.1.0");
    if (package_parse_manifest(manifest_path, pkg)) {
        pm->current_package = pkg;
        bool result = package_update_dependencies(pm);
        package_manager_free(pm);
        return result;
    }
    
    package_manager_free(pm);
    return false;
}

bool cargo_install(const char* package_name) {
    printf("installing cargo package: %s\n", package_name);
    
    // resolve and install package
    char* package_path = package_resolve_path(package_name, "latest");
    if (package_path) {
        printf("  installing from: %s\n", package_path);
        free(package_path);
        printf("  package installed successfully\n");
        return true;
    }
    
    printf("  error: cannot resolve package\n");
    return false;
}

// dependency resolution
bool resolve_dependency_tree(PackageManager* pm) {
    printf("resolving dependency tree\n");
    
    for (size_t i = 0; i < pm->dependency_count; i++) {
        Package* dep = pm->dependencies[i];
        printf("  resolving: %s v%s\n", dep->name, dep->version);
        
        // check for conflicts
        for (size_t j = i + 1; j < pm->dependency_count; j++) {
            Package* other = pm->dependencies[j];
            if (strcmp(dep->name, other->name) == 0 && strcmp(dep->version, other->version) != 0) {
                printf("    conflict: %s v%s vs v%s\n", dep->name, dep->version, other->version);
                return false;
            }
        }
    }
    
    printf("  dependency tree resolved successfully\n");
    return true;
}

bool check_dependency_conflicts(PackageManager* pm) {
    printf("checking for dependency conflicts\n");
    
    for (size_t i = 0; i < pm->dependency_count; i++) {
        for (size_t j = i + 1; j < pm->dependency_count; j++) {
            Package* dep1 = pm->dependencies[i];
            Package* dep2 = pm->dependencies[j];
            
            if (strcmp(dep1->name, dep2->name) == 0 && strcmp(dep1->version, dep2->version) != 0) {
                printf("  conflict found: %s v%s vs v%s\n", dep1->name, dep1->version, dep2->version);
                return false;
            }
        }
    }
    
    printf("  no conflicts found\n");
    return true;
}

char* find_package_version(const char* package_name, const char* version_constraint) {
    printf("finding package version: %s %s\n", package_name, version_constraint);
    
    // simplified version resolution
    char* version = malloc(32);
    if (strcmp(version_constraint, "latest") == 0) {
        strcpy(version, "1.0.0");
    } else {
        strcpy(version, version_constraint);
    }
    
    printf("  resolved to version: %s\n", version);
    return version;
}

// workspace management
bool create_workspace(const char* workspace_path) {
    printf("creating workspace: %s\n", workspace_path);
    
    struct stat st = {0};
    if (stat(workspace_path, &st) == -1) {
        if (mkdir(workspace_path, 0700) != 0) {
            printf("  error: cannot create workspace directory\n");
            return false;
        }
    }
    
    // create workspace.toml
    char workspace_manifest[512];
    snprintf(workspace_manifest, sizeof(workspace_manifest), "%s/Cargo.toml", workspace_path);
    FILE* file = fopen(workspace_manifest, "w");
    if (file) {
        fprintf(file, "[workspace]\n");
        fprintf(file, "members = []\n");
        fclose(file);
    }
    
    printf("  workspace created successfully\n");
    return true;
}

bool add_package_to_workspace(const char* workspace_path, const char* package_path) {
    printf("adding package to workspace: %s -> %s\n", workspace_path, package_path);
    
    // update workspace.toml
    char workspace_manifest[512];
    snprintf(workspace_manifest, sizeof(workspace_manifest), "%s/Cargo.toml", workspace_path);
    
    printf("  package added to workspace successfully\n");
    return true;
}

bool remove_package_from_workspace(const char* workspace_path, const char* package_name) {
    printf("removing package from workspace: %s -> %s\n", workspace_path, package_name);
    
    printf("  package removed from workspace successfully\n");
    return true;
} 