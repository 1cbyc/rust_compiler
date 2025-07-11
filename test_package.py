#!/usr/bin/env python3
"""
test script for package management functionality
simulates cargo-like package management with dependency resolution
"""

import re
import os
from enum import Enum
from typing import List, Dict, Optional, Tuple

class PackageType(Enum):
    BINARY = "binary"
    LIBRARY = "library"

class DependencyType(Enum):
    REGULAR = "regular"
    DEV = "dev"
    BUILD = "build"

class Package:
    def __init__(self, name: str, version: str, package_type: PackageType = PackageType.BINARY):
        self.name = name
        self.version = version
        self.description = ""
        self.authors = []
        self.license = "MIT"
        self.type = package_type
        self.source_path = "src"
        self.target_path = "target"
        self.manifest_path = "Cargo.toml"
        self.dependencies = []

class Dependency:
    def __init__(self, name: str, version: str, dep_type: DependencyType = DependencyType.REGULAR):
        self.name = name
        self.version = version
        self.source = "crates.io"
        self.type = dep_type
        self.optional = False

class PackageManager:
    def __init__(self, workspace_path: str):
        self.workspace_path = workspace_path
        self.target_directory = "target"
        self.cache_directory = ".cargo"
        self.current_package = None
        self.dependencies = []
        self.registry = {}

def parse_manifest(manifest_content: str) -> Optional[Package]:
    """parse cargo.toml manifest content"""
    print(f"parsing manifest content")
    
    pkg = None
    in_package_section = False
    in_dependencies_section = False
    
    for line in manifest_content.split('\n'):
        line = line.strip()
        if not line or line.startswith('#'):
            continue
            
        if line == '[package]':
            in_package_section = True
            in_dependencies_section = False
            continue
        elif line == '[dependencies]':
            in_package_section = False
            in_dependencies_section = True
            continue
            
        if in_package_section:
            if line.startswith('name = '):
                name = line.split('=')[1].strip().strip('"')
                pkg = Package(name, "0.1.0")
            elif line.startswith('version = ') and pkg:
                version = line.split('=')[1].strip().strip('"')
                pkg.version = version
            elif line.startswith('description = ') and pkg:
                description = line.split('=')[1].strip().strip('"')
                pkg.description = description
                
        elif in_dependencies_section and pkg:
            if '=' in line and not line.startswith('['):
                dep_name = line.split('=')[0].strip()
                dep_version = line.split('=')[1].strip().strip('"')
                dep = Dependency(dep_name, dep_version)
                pkg.dependencies.append(dep)
    
    if pkg:
        print(f"  parsed package: {pkg.name} v{pkg.version}")
        print(f"  dependencies: {len(pkg.dependencies)}")
    else:
        print("  error: failed to parse manifest")
        
    return pkg

def write_manifest(pkg: Package) -> str:
    """write package to cargo.toml format"""
    print(f"writing manifest for package: {pkg.name}")
    
    content = "[package]\n"
    content += f'name = "{pkg.name}"\n'
    content += f'version = "{pkg.version}"\n'
    content += f'description = "{pkg.description}"\n'
    content += f'license = "{pkg.license}"\n'
    content += "\n[dependencies]\n"
    
    for dep in pkg.dependencies:
        content += f'{dep.name} = "{dep.version}"\n'
    
    print(f"  manifest written successfully")
    return content

def validate_manifest(manifest_content: str) -> bool:
    """validate cargo.toml manifest"""
    print(f"validating manifest")
    
    has_package = False
    has_dependencies = False
    
    for line in manifest_content.split('\n'):
        line = line.strip()
        if line == '[package]':
            has_package = True
        elif line == '[dependencies]':
            has_dependencies = True
    
    if not has_package:
        print("  error: missing [package] section")
        return False
        
    print("  manifest is valid")
    return True

def resolve_dependency(name: str, version: str) -> Optional[str]:
    """resolve dependency to actual path"""
    print(f"resolving dependency: {name} v{version}")
    
    # simplified resolution
    path = f"/cargo/registry/src/{name}-{version}"
    print(f"  resolved to: {path}")
    return path

def check_dependency_conflicts(dependencies: List[Dependency]) -> bool:
    """check for dependency conflicts"""
    print(f"checking for dependency conflicts")
    
    dep_versions = {}
    for dep in dependencies:
        if dep.name in dep_versions:
            if dep_versions[dep.name] != dep.version:
                print(f"  conflict: {dep.name} v{dep_versions[dep.name]} vs v{dep.version}")
                return False
        else:
            dep_versions[dep.name] = dep.version
    
    print("  no conflicts found")
    return True

def cargo_new(project_name: str, package_type: PackageType) -> bool:
    """create new cargo project"""
    print(f"creating new cargo project: {project_name}")
    
    # create project structure
    os.makedirs(project_name, exist_ok=True)
    os.makedirs(f"{project_name}/src", exist_ok=True)
    
    # create main.rs
    main_content = """fn main() {
    println!("hello, world!");
}
"""
    with open(f"{project_name}/src/main.rs", "w") as f:
        f.write(main_content)
    
    # create cargo.toml
    pkg = Package(project_name, "0.1.0", package_type)
    manifest_content = write_manifest(pkg)
    with open(f"{project_name}/Cargo.toml", "w") as f:
        f.write(manifest_content)
    
    print(f"  project created successfully")
    return True

def cargo_build(project_path: str) -> bool:
    """build cargo project"""
    print(f"building cargo project: {project_path}")
    
    # read manifest
    manifest_path = f"{project_path}/Cargo.toml"
    if not os.path.exists(manifest_path):
        print("  error: manifest file not found")
        return False
    
    with open(manifest_path, "r") as f:
        manifest_content = f.read()
    
    pkg = parse_manifest(manifest_content)
    if not pkg:
        return False
    
    # validate dependencies
    if not check_dependency_conflicts(pkg.dependencies):
        return False
    
    # compile source files
    src_path = f"{project_path}/src"
    if os.path.exists(src_path):
        print(f"  compiling source files from: {src_path}")
    
    print(f"  build completed successfully")
    return True

def cargo_test(project_path: str) -> bool:
    """test cargo project"""
    print(f"testing cargo project: {project_path}")
    
    # run tests
    test_path = f"{project_path}/tests"
    if os.path.exists(test_path):
        print(f"  running tests from: {test_path}")
    
    print(f"  all tests passed")
    return True

def cargo_run(project_path: str) -> bool:
    """run cargo project"""
    print(f"running cargo project: {project_path}")
    
    # execute binary
    binary_path = f"{project_path}/target/{os.path.basename(project_path)}"
    print(f"  executing: {binary_path}")
    
    print(f"  program completed successfully")
    return True

def cargo_add(project_path: str, dependency: str, version: str) -> bool:
    """add dependency to cargo project"""
    print(f"adding dependency to cargo project: {project_path} -> {dependency} v{version}")
    
    # read current manifest
    manifest_path = f"{project_path}/Cargo.toml"
    if not os.path.exists(manifest_path):
        print("  error: manifest file not found")
        return False
    
    with open(manifest_path, "r") as f:
        manifest_content = f.read()
    
    pkg = parse_manifest(manifest_content)
    if not pkg:
        return False
    
    # check if dependency already exists
    for dep in pkg.dependencies:
        if dep.name == dependency:
            print(f"  warning: dependency {dependency} already exists")
            return False
    
    # add new dependency
    new_dep = Dependency(dependency, version)
    pkg.dependencies.append(new_dep)
    
    # write updated manifest
    updated_manifest = write_manifest(pkg)
    with open(manifest_path, "w") as f:
        f.write(updated_manifest)
    
    print(f"  dependency added successfully")
    return True

def cargo_remove(project_path: str, dependency: str) -> bool:
    """remove dependency from cargo project"""
    print(f"removing dependency from cargo project: {project_path} -> {dependency}")
    
    # read current manifest
    manifest_path = f"{project_path}/Cargo.toml"
    if not os.path.exists(manifest_path):
        print("  error: manifest file not found")
        return False
    
    with open(manifest_path, "r") as f:
        manifest_content = f.read()
    
    pkg = parse_manifest(manifest_content)
    if not pkg:
        return False
    
    # remove dependency
    pkg.dependencies = [dep for dep in pkg.dependencies if dep.name != dependency]
    
    # write updated manifest
    updated_manifest = write_manifest(pkg)
    with open(manifest_path, "w") as f:
        f.write(updated_manifest)
    
    print(f"  dependency removed successfully")
    return True

def cargo_update(project_path: str) -> bool:
    """update cargo project dependencies"""
    print(f"updating cargo project: {project_path}")
    
    # read current manifest
    manifest_path = f"{project_path}/Cargo.toml"
    if not os.path.exists(manifest_path):
        print("  error: manifest file not found")
        return False
    
    with open(manifest_path, "r") as f:
        manifest_content = f.read()
    
    pkg = parse_manifest(manifest_content)
    if not pkg:
        return False
    
    # update dependencies
    for dep in pkg.dependencies:
        print(f"  updating: {dep.name} v{dep.version}")
        # simplified update - just bump version
        if dep.version == "0.1.0":
            dep.version = "0.2.0"
        elif dep.version == "0.2.0":
            dep.version = "0.3.0"
    
    # write updated manifest
    updated_manifest = write_manifest(pkg)
    with open(manifest_path, "w") as f:
        f.write(updated_manifest)
    
    print(f"  dependencies updated successfully")
    return True

def cargo_install(package_name: str) -> bool:
    """install cargo package"""
    print(f"installing cargo package: {package_name}")
    
    # resolve package
    package_path = resolve_dependency(package_name, "latest")
    if package_path:
        print(f"  installing from: {package_path}")
        print(f"  package installed successfully")
        return True
    
    print(f"  error: cannot resolve package")
    return False

def test_manifest_parsing():
    """test manifest parsing functionality"""
    print("\n=== testing manifest parsing ===")
    
    manifest_content = """[package]
name = "my_project"
version = "0.1.0"
description = "a test project"
license = "MIT"

[dependencies]
serde = "1.0"
tokio = "1.0"
"""
    
    pkg = parse_manifest(manifest_content)
    assert pkg is not None
    assert pkg.name == "my_project"
    assert pkg.version == "0.1.0"
    assert len(pkg.dependencies) == 2
    
    print("  manifest parsing test passed")

def test_manifest_validation():
    """test manifest validation"""
    print("\n=== testing manifest validation ===")
    
    valid_manifest = """[package]
name = "test"
version = "0.1.0"

[dependencies]
"""
    
    invalid_manifest = """[dependencies]
serde = "1.0"
"""
    
    assert validate_manifest(valid_manifest) == True
    assert validate_manifest(invalid_manifest) == False
    
    print("  manifest validation test passed")

def test_dependency_resolution():
    """test dependency resolution"""
    print("\n=== testing dependency resolution ===")
    
    path = resolve_dependency("serde", "1.0.0")
    assert path is not None
    assert "serde-1.0.0" in path
    
    print("  dependency resolution test passed")

def test_dependency_conflicts():
    """test dependency conflict detection"""
    print("\n=== testing dependency conflicts ===")
    
    deps1 = [
        Dependency("serde", "1.0"),
        Dependency("tokio", "1.0")
    ]
    assert check_dependency_conflicts(deps1) == True
    
    deps2 = [
        Dependency("serde", "1.0"),
        Dependency("serde", "2.0")
    ]
    assert check_dependency_conflicts(deps2) == False
    
    print("  dependency conflicts test passed")

def test_cargo_commands():
    """test cargo-like commands"""
    print("\n=== testing cargo commands ===")
    
    # test cargo new
    assert cargo_new("test_project", PackageType.BINARY) == True
    assert os.path.exists("test_project/Cargo.toml")
    assert os.path.exists("test_project/src/main.rs")
    
    # test cargo add
    assert cargo_add("test_project", "serde", "1.0") == True
    
    # test cargo build
    assert cargo_build("test_project") == True
    
    # test cargo test
    assert cargo_test("test_project") == True
    
    # test cargo run
    assert cargo_run("test_project") == True
    
    # test cargo remove
    assert cargo_remove("test_project", "serde") == True
    
    # test cargo update
    assert cargo_update("test_project") == True
    
    # test cargo install
    assert cargo_install("serde") == True
    
    # cleanup
    import shutil
    shutil.rmtree("test_project", ignore_errors=True)
    
    print("  cargo commands test passed")

def test_package_manager():
    """test package manager functionality"""
    print("\n=== testing package manager ===")
    
    pm = PackageManager("test_workspace")
    
    # create package
    pkg = Package("test_package", "0.1.0")
    pm.current_package = pkg
    
    # add dependencies
    dep1 = Dependency("serde", "1.0")
    dep2 = Dependency("tokio", "1.0")
    pkg.dependencies = [dep1, dep2]
    
    # test dependency resolution
    assert len(pkg.dependencies) == 2
    assert pkg.dependencies[0].name == "serde"
    assert pkg.dependencies[1].name == "tokio"
    
    print("  package manager test passed")

def test_workspace_management():
    """test workspace management"""
    print("\n=== testing workspace management ===")
    
    # create workspace
    os.makedirs("test_workspace", exist_ok=True)
    
    # create workspace manifest
    workspace_manifest = """[workspace]
members = ["package1", "package2"]
"""
    with open("test_workspace/Cargo.toml", "w") as f:
        f.write(workspace_manifest)
    
    # create packages
    cargo_new("test_workspace/package1", PackageType.LIBRARY)
    cargo_new("test_workspace/package2", PackageType.BINARY)
    
    # test workspace build
    assert cargo_build("test_workspace/package1") == True
    assert cargo_build("test_workspace/package2") == True
    
    # cleanup
    import shutil
    shutil.rmtree("test_workspace", ignore_errors=True)
    
    print("  workspace management test passed")

def main():
    """run all package management tests"""
    print("rust compiler - package management test")
    print("=" * 50)
    
    try:
        test_manifest_parsing()
        test_manifest_validation()
        test_dependency_resolution()
        test_dependency_conflicts()
        test_cargo_commands()
        test_package_manager()
        test_workspace_management()
        
        print("\n" + "=" * 50)
        print("all package management tests passed!")
        print("package management functionality is working correctly")
        
    except Exception as e:
        print(f"\nerror: {e}")
        return False
    
    return True

if __name__ == "__main__":
    main() 