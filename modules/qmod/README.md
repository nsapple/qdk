# QMOD - Mod System for Godot

QMOD is a mod packaging and loading system for Godot Engine that allows you to create, export, and load game mods.

## Features

- **Easy Mod Creation**: Export scenes as QMOD files with metadata
- **Automatic Installation**: Mods are automatically installed to `user://mods`
- **Metadata Support**: Include title, description, icon, and type information
- **Two Mod Types**: Support for "level" and "character" mod types
- **PCK-Based Format**: Uses Godot's PCK format for compatibility

## Usage

### Exporting a Mod

```gdscript
# Create an exporter
var exporter = QModExporter.new()
exporter.title = "My Awesome Level"
exporter.description = "A challenging platformer level with unique mechanics"
exporter.icon_path = "res://icon.png"
exporter.mod_type = QModExporter.MOD_TYPE_LEVEL

# Export the mod
var error = exporter.export_qmod("res://levels/my_level.tscn", "user://my_mod.qmod")
if error == OK:
    print("Mod exported successfully!")
```

### Installing a Mod

```gdscript
# Create a loader
var loader = QModLoader.new()

# Install a QMOD file
var error = loader.install_qmod("user://downloads/cool_mod.qmod")
if error == OK:
    print("Mod installed successfully!")
```

### Getting Installed Mods

```gdscript
var loader = QModLoader.new()

# Get list of installed mods
var mods = loader.get_installed_mods()
for mod_name in mods:
    print("Found mod: ", mod_name)

    # Get mod info
    var info = loader.get_mod_info(mod_name)
    print("  Title: ", info["title"])
    print("  Description: ", info["description"])
    print("  Type: ", info["type"])
```

### Loading a Mod Scene

```gdscript
var loader = QModLoader.new()

# Load a mod's scene
var error = loader.load_mod_scene("my_awesome_level")
if error == OK:
    print("Mod scene loaded!")
```

### Uninstalling a Mod

```gdscript
var loader = QModLoader.new()

# Uninstall a mod
var error = loader.uninstall_qmod("my_awesome_level")
if error == OK:
    print("Mod uninstalled!")
```

## QMOD File Format

A QMOD file is a standard Godot PCK file containing:

1. **mod.json** - Metadata file with the following structure:
   ```json
   {
       "title": "Mod Title",
       "description": "Mod description",
       "icon": "icon.png",
       "type": "level",
       "scene": "level.tscn"
   }
   ```

2. **Scene file** - The main scene file (e.g., `level.tscn`)

3. **Icon file** (optional) - Icon image for the mod

4. **Additional resources** - Any other files referenced by the scene

## Mod Types

- `QModExporter.MOD_TYPE_LEVEL` - For level/map mods
- `QModExporter.MOD_TYPE_CHARACTER` - For character mods

## Installation Directory

All mods are installed to: `user://mods/<mod_name>/`

Where `<mod_name>` is derived from the mod's title (lowercase, spaces replaced with underscores).

## Example Workflow

1. **Create your mod content** in Godot (scene, scripts, assets)
2. **Export as QMOD** using `QModExporter`
3. **Distribute** the .qmod file to players
4. **Players install** using `QModLoader.install_qmod()`
5. **Game loads mods** using `QModLoader.get_installed_mods()` and `load_mod_scene()`

## API Reference

### QModExporter

**Properties:**
- `title: String` - Mod title
- `description: String` - Mod description
- `icon_path: String` - Path to icon file
- `mod_type: ModType` - Type of mod (LEVEL or CHARACTER)

**Methods:**
- `export_qmod(scene_path: String, output_path: String) -> Error`

### QModLoader

**Methods:**
- `install_qmod(qmod_path: String) -> Error`
- `uninstall_qmod(mod_name: String) -> Error`
- `get_installed_mods() -> Array`
- `get_mod_info(mod_name: String) -> Dictionary`
- `load_mod_scene(mod_name: String) -> Error`
- `static get_mods_directory() -> String`

## License

This module is part of the Godot Engine and follows the same MIT license.
