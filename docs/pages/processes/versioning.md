# Versioning {#versioning}

## Overview

Wren uses [Semantic Versioning 2.0.0](https://semver.org/) as the foundation
for all release numbering. A single source of truth — `project.json` in the
repository root — declares the current version. The CMake build system reads
that file, enriches it with Git metadata and a local build counter, then
propagates the result to every artifact that needs it: C++ headers, CMake
targets, install packages, and CI logs.

The full version string produced at configure time looks like:

```
MAJOR.MINOR.PATCH[-EXTRA][+BUILD.gCOMMIT]
```

The build metadata suffix (`+BUILD.gCOMMIT`) is included for development
builds to distinguish otherwise identical version numbers. It is **omitted**
when the build counter is `0` and HEAD is on a tag or the `master`/`main`
branch, producing a clean release string.

For example:

| Context                           | Rendered string                     |
| --------------------------------- | ----------------------------------- |
| Tagged release on `master`        | `1.2.0`                             |
| Work in progress on `develop`     | `1.3.0-dev+4.gf00bbaa`              |
| Feature branch `feature/mesh-lod` | `1.3.0-feature-mesh-lod+0.g9abcdef` |
| Explicit pre-release label        | `2.0.0-rc.1+0.g1234567`             |
| Rebuild on `master` (build 1)     | `1.2.0+1.ga1b2c3d`                  |

______________________________________________________________________

## Semantic Versioning Rules

Given a version `MAJOR.MINOR.PATCH`:

| Component | Increment when…                                        |
| --------- | ------------------------------------------------------ |
| **MAJOR** | You make incompatible API or ABI changes.              |
| **MINOR** | You add functionality in a backward-compatible manner. |
| **PATCH** | You make backward-compatible bug fixes only.           |

While the project is in initial development (`0.x.y`), the public API is not
considered stable. Minor version bumps may contain breaking changes.

### Pre-release Labels (`EXTRA`)

The optional `EXTRA` segment marks a version as pre-release. Pre-release
versions have lower precedence than the associated normal version:

- `alpha` — feature-incomplete, may be unstable.
- `beta` — feature-complete, undergoing wider testing.
- `rc.N` — release candidate N, expected to ship unless blockers surface.
- `dev` — automatic label applied on the `develop` branch.

______________________________________________________________________

## The Version Source of Truth: `project.json`

All version information originates from a single JSON file at the repository
root:

```json
{
  "project": "wren",
  "version": {
    "major": 0,
    "minor": 1,
    "patch": 0
  }
}
```

An optional `"extra"` field can be added to set a pre-release label explicitly:

```json
{
  "project": "wren",
  "version": {
    "major": 2,
    "minor": 0,
    "patch": 0,
    "extra": "rc.1"
  }
}
```

### Field Reference

| Field           | Type    | Required | Description                                                  |
| --------------- | ------- | -------- | ------------------------------------------------------------ |
| `project`       | string  | yes      | Project name. Used in packaging and install targets.         |
| `version.major` | integer | yes      | Major version number.                                        |
| `version.minor` | integer | yes      | Minor version number.                                        |
| `version.patch` | integer | yes      | Patch version number.                                        |
| `version.extra` | string  | no       | Explicit pre-release label. Overrides branch-derived labels. |

______________________________________________________________________

## CMake Integration

### How It Works

The root `CMakeLists.txt` calls `wren_extract_version()` **before** the
`project()` command. This function:

1. **Reads** `project.json` for major, minor, patch, and optional extra.
1. **Queries Git** for the current branch, abbreviated commit SHA, and
   whether HEAD is exactly on an annotated or lightweight tag.
1. **Derives the pre-release label** using a priority chain:
   - The `WREN_VERSION_EXTRA` CMake variable (highest priority, e.g. from CI).
   - The `"extra"` field in `project.json`.
   - An automatic label from the current Git branch (see below).
1. **Manages a build counter** via a state file (`build/last_build.json`).
   The counter increments on each configure where the version, extra label,
   and commit SHA are all unchanged. It resets to `0` whenever any of those
   change.
1. **Composes** the full version string. When the build counter is `0`
   **and** HEAD is on a tag or the `master`/`main` branch, the build
   metadata suffix (`+BUILD.gCOMMIT`) is omitted to produce a clean release
   version.
1. **Generates** a C++ header (`wren/version.hpp`) and persists the state
   file for the next run.

### Automatic Branch Labels

When no explicit extra label is provided, the branch name determines the
label automatically:

| Branch             | Label              | Example string                       |
| ------------------ | ------------------ | ------------------------------------ |
| `master` / `main`  | *(none)*           | `1.0.0`                              |
| `develop`          | `dev`              | `1.1.0-dev+3.g9876fed`               |
| `feature/mesh-lod` | `feature-mesh-lod` | `1.1.0-feature-mesh-lod+0.gdeadbeef` |
| `bugfix/fix-crash` | `bugfix-fix-crash` | `1.0.1-bugfix-fix-crash+0.gcafe123`  |

Branch names are sanitized to contain only lowercase alphanumerics, hyphens,
and dots.

### Exported CMake Variables

After `wren_extract_version()` returns, the following variables are available
in the calling scope:

| Variable              | Example                | Description                                                             |
| --------------------- | ---------------------- | ----------------------------------------------------------------------- |
| `WREN_VERSION_MAJOR`  | `1`                    | Major component.                                                        |
| `WREN_VERSION_MINOR`  | `2`                    | Minor component.                                                        |
| `WREN_VERSION_PATCH`  | `0`                    | Patch component.                                                        |
| `WREN_VERSION_EXTRA`  | `dev`                  | Pre-release label (may be empty).                                       |
| `WREN_VERSION`        | `1.2.0`                | `MAJOR.MINOR.PATCH` — used by `project(VERSION)`.                       |
| `WREN_VERSION_STRING` | `1.2.0-dev+3.ga1b2c3d` | Full SemVer string with metadata (metadata omitted for clean releases). |
| `WREN_VERSION_BUILD`  | `3`                    | Local build counter.                                                    |
| `WREN_GIT_BRANCH`     | `develop`              | Current Git branch.                                                     |
| `WREN_GIT_COMMIT`     | `a1b2c3d`              | Abbreviated commit SHA (7 chars).                                       |
| `WREN_GIT_TAG`        | `v1.2.0`               | Tag name if HEAD is exactly on a tag (empty otherwise).                 |

### Generated C++ Header

The build produces `<build-dir>/include/wren/version.hpp`:

```cpp
#pragma once
#define WREN_VERSION_MAJOR 1
#define WREN_VERSION_MINOR 2
#define WREN_VERSION_PATCH 0
#define WREN_VERSION_BUILD 3
#define WREN_VERSION_STRING "1.2.0-dev+3.ga1b2c3d"
#define WREN_GIT_COMMIT "a1b2c3d"
#define WREN_GIT_BRANCH "develop"
#define WREN_VERSION_EXTRA "dev"
```

Any target that links against `wren::version_info` automatically receives the
include path for this header.

### Build State File

The file `build/last_build.json` tracks the version and commit from the
previous configure. It is a build artifact — **never commit it** to version
control. Its schema:

```json
{
  "last_version": {
    "major": 1,
    "minor": 2,
    "patch": 0,
    "build": 3,
    "commit": "a1b2c3d",
    "extra": "dev"
  }
}
```

______________________________________________________________________

## Multi-Component Versioning

Wren is structured as a multi-library project under `projects/libs/`. All
components share the top-level version defined in `project.json`. This
provides a unified release identity: when version `1.2.0` ships,
**every** library and executable in the project is `1.2.0`.

Individual libraries do not maintain independent version numbers. If a
component needs to evolve at its own pace in the future, it can declare
its own `VERSION` in its `CMakeLists.txt` and a local `version.json`,
but the default is a single coordinated version.

______________________________________________________________________

## Git Workflow and Branching Strategy

Wren follows a simplified **Git Flow** model suited for open-source projects
with a small-to-medium contributor base.

### Branch Roles

| Branch              | Lifetime  | Purpose                                                                                                     |
| ------------------- | --------- | ----------------------------------------------------------------------------------------------------------- |
| `master`            | Permanent | Always reflects the latest **released** state. Every commit on `master` is a tagged release.                |
| `develop`           | Permanent | Integration branch for the next release. All feature work merges here first.                                |
| `feature/<name>`    | Temporary | New functionality. Branched from and merged back into `develop`.                                            |
| `bugfix/<name>`     | Temporary | Non-critical fixes. Branched from and merged back into `develop`.                                           |
| `hotfix/<name>`     | Temporary | Critical fixes for a released version. Branched from `master`, merged into both `master` and `develop`.     |
| `release/<version>` | Temporary | Stabilization before a release. Branched from `develop`, merged into both `master` and `develop` when done. |

### Branch Lifecycle

```
master ─────●───────────────────●──────────── (tags: v1.0.0, v1.1.0)
             \                 /
develop ──────●───●───●───●───● ────────────
               \     / \     /
feature/foo ────●───●   \   /
                     bugfix/bar
```

#### Feature Development

1. Branch from `develop`:
   ```bash
   git checkout develop
   git checkout -b feature/my-feature
   ```
1. Develop, commit, push.
1. Open a pull request targeting `develop`.
1. After review and CI pass, **squash-merge** or **rebase-merge** into
   `develop`. Delete the feature branch.

#### Bug Fixes

Same workflow as features, but use the `bugfix/` prefix for clarity in logs
and CI.

#### Releases

1. Branch from `develop`:
   ```bash
   git checkout develop
   git checkout -b release/1.2.0
   ```
1. Update `project.json` to set the release version (remove any `"extra"`).
1. Fix release-blocking issues only — no new features.
1. Merge into `master` **and** back into `develop`:
   ```bash
   git checkout master
   git merge --no-ff release/1.2.0
   git tag -a v1.2.0 -m "Release 1.2.0"

   git checkout develop
   git merge --no-ff release/1.2.0
   ```
1. Delete the release branch.
1. Bump `project.json` on `develop` to the next anticipated version (e.g.
   `1.3.0`).

#### Hotfixes

1. Branch from `master`:
   ```bash
   git checkout master
   git checkout -b hotfix/1.2.1
   ```
1. Fix the issue. Bump `patch` in `project.json`.
1. Merge into `master` **and** back into `develop`:
   ```bash
   git checkout master
   git merge --no-ff hotfix/1.2.1
   git tag -a v1.2.1 -m "Hotfix 1.2.1"

   git checkout develop
   git merge --no-ff hotfix/1.2.1
   ```
1. Delete the hotfix branch.

### Tagging Conventions

- Tags use the format `v<MAJOR>.<MINOR>.<PATCH>` (e.g. `v1.2.0`).
- Pre-release tags append the label: `v2.0.0-rc.1`.
- Tags are **annotated** (`git tag -a`), never lightweight.
- Tag messages should include a brief changelog summary.

### Commit Message Guidelines

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <short summary>

<optional body>

<optional footer(s)>
```

| Type       | When to use                                              |
| ---------- | -------------------------------------------------------- |
| `feat`     | A new feature.                                           |
| `fix`      | A bug fix.                                               |
| `refactor` | Code change that neither fixes a bug nor adds a feature. |
| `perf`     | Performance improvement.                                 |
| `docs`     | Documentation only.                                      |
| `test`     | Adding or correcting tests.                              |
| `build`    | Changes to build system or dependencies.                 |
| `ci`       | CI configuration changes.                                |
| `chore`    | Maintenance tasks (formatting, tooling, etc.).           |

Examples:

```
feat(rhi): add Vulkan swapchain recreation on resize
fix(foundation): correct off-by-one in ring buffer capacity
build(cmake): remove custom vcpkg triplet
docs: update versioning documentation
```

### Best Practices

- **Keep `master` releasable.** Every commit on `master` must build, pass
  tests, and correspond to a tagged release.
- **Keep `develop` green.** Broken builds on `develop` block the entire team.
  Fix or revert immediately.
- **Rebase feature branches** onto `develop` before merging to maintain a
  linear, readable history.
- **Never force-push** `master` or `develop`.
- **Delete merged branches** promptly to keep the branch list clean.
- **One logical change per commit.** Avoid mixing unrelated changes.
- **Write meaningful commit messages.** The summary line should make sense
  in a changelog without additional context.

______________________________________________________________________

## Version Bump Checklist

When preparing any release or pre-release:

1. Update `version.major`, `version.minor`, and/or `version.patch` in
   `project.json`.
1. Set or remove `version.extra` as appropriate.
1. Commit with message: `build: bump version to X.Y.Z[-extra]`.
1. Merge to `master` (for releases) and tag with `vX.Y.Z[-extra]`.
1. Merge back to `develop` and bump to the next development version.
1. Verify the generated `version.hpp` is correct by running:
   ```bash
   cmake --preset dev-msvc # or dev-unix
   ```
   and inspecting the configure output:
   ```
   -- Wren version: 1.2.0 (1.2.0, build: 0)
   ```
