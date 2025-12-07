AbandonedLabModular

A modular horror‑lab first‑person (or 3D) game / game‑engine prototype. The project aims to create an “abandoned laboratory” environment with modular rooms/components, allowing for flexible lab layout, horror atmosphere, and easy expansion.

🎯 What is this / Why “Abandoned Lab Modular”?

The core idea is to build a base framework / engine for horror‑style exploration in a modular lab — instead of a single fixed map, the lab is built from reusable modules (rooms, corridors, labs, props, etc.).

This modularity enables you (or other developers) to easily rearrange layouts, reuse components, or expand the environment without rewriting core logic.

The project is meant as a foundation: you can treat it as a sandbox for building custom lab-based horror levels / experiments / games.

🧩 Key Features / What Works (for now)

Project source in C++ (with small C parts) — primary language: C++. 
GitHub

Modular architecture / file structure (see src/) to support lab‑modules / map modules. 
GitHub

Visual / 3D (or planned 3D) horror‑laboratory style: the “abandoned lab” theme — moody environment, potentially horror / suspense atmosphere (or at least eerie, modular environment).

Easy to extend: because of modular design, new rooms / layouts / props can be added without rewriting core logic.

🗂️ Repository Structure
/               – Root  
  ├─ src/       – Source code (C++ / C)  
  ├─ abandonedlabmodular.sln  – Visual Studio solution (if using Windows / VS) :contentReference[oaicite:3]{index=3}  
  ├─ .gitignore / .gitattributes   :contentReference[oaicite:4]{index=4}  
  └─ (other assets / future folders: e.g. assets/, docs/, levels/)  


Feel free to reorganize / extend this structure as the project grows (e.g. separating engine code vs game logic vs assets vs levels).

🚀 How to Build / Run (roughly)

Currently the project is a work‑in‑progress. Here’s a minimal setup to get started (on Windows + Visual Studio):

Clone / download the repository.

Open abandonedlabmodular.sln in Visual Studio.

Build the solution (Release or Debug).

Run the compiled executable.

If you add dependencies (e.g. external libraries, asset loaders, audio, physics), add installation / linking instructions here (or in a separate BUILD.md).

(If you target other platforms — Linux, MacOS, etc. — you may need a different build system / makefile / CMake / etc. Feel free to add instructions once support exists.)

🎮 Usage / What to Do After Building

Once built, you’ll start with a minimal environment (empty lab or basic modules). You can then:

Load or build modular lab layouts: combine rooms, corridors, lab spaces, etc.

Add props, textures, lights — to create atmosphere (horror, abandoned‑lab vibes).

Extend the engine / game logic: e.g. add doors, puzzles, items, hazards, creatures, ambient effects, sound, etc.

Use the modular design as a foundation for custom levels / horror games / experiments.

🛠️ How to Contribute

If you want to help build or expand the project (add features, assets, levels, fix bugs):

Fork the repository.

Create a new branch for your changes.

Make changes and test thoroughly.

Open a pull request.

(Optional) Open an issue or discussion before beginning large changes.

Feel free to add: new modules (rooms, corridors), improved graphics / lighting / textures / sound, gameplay logic (doors, interactables, hazards), or platform support (Linux / Mac).

If you plan to add many assets (textures, 3D models, sound), consider adding a /assets, /levels, or /modules directory to keep things organized.

✅ Status & Known Limitations

This is an early prototype / base framework; many features — e.g. rich graphics, sound, full “gameplay/horror” features — are not implemented yet.

The modular lab “engine” works (structurally), but building complex lab layouts or levels will require extra work (level design, assets, optimization, etc.).

No release builds provided. Users will need an appropriate build environment (Visual Studio on Windows) to compile.

📄 License & Attribution

Include your license here (e.g. MIT, GPL, etc.) — or state “All rights reserved (private project)” if you prefer.

If using third‑party assets (models, textures, sounds), document their authors / licenses clearly (especially if those licenses impose redistribution conditions). This is especially important for game dev projects. 
Kentucky Computer Science
+1

🙋 What’s Next / Roadmap (Possible Future Work)

Add asset pipeline — textures, 3D models, sounds, lighting → to realize full horror‑lab atmosphere.

Add level editor or layout exporter/importer — to let creators design labs without recompiling (modular-level data).

Extend game logic — doors, traps, events, puzzles, NPCs / enemies, player interactions.

Cross‑platform support (Linux, Mac) — maybe set up a CMake build system.

Documentation & examples: provide sample lab layout files, assets, and maybe a “demo level” so new users/contributors can quickly see what the project looks like.
