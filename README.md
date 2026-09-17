# Arsenal Categories

A small, open-source filter bar for the Arma Reforger arsenal panel.

Vanilla lists every item an arsenal offers in one flat grid. This addon adds a category column next to that grid — *All, Submachine Guns, Assault Rifles, Sniper Rifles, Machine Guns, Pistols, Launchers, Ammunition, Attachments, Throwables, Explosives, Clothing, Vests and Backpacks, Medical, Equipment* — each button showing its item count, so you only scroll through what you are looking for. It works with any arsenal box (vanilla, RHS, WCS, …) because it hooks the shared arsenal UI rather than any faction's data.

## Requirements

- Arma Reforger. No other dependency.

Load it on the server like any other addon; clients receive it automatically. It only touches the client UI — nothing is changed on the server, in catalogs, prices or item availability, and it does not modify prefabs, so it coexists with arsenal content mods.

## What it does

- Shows a category column to the left of the item grid inside the arsenal panel (the "Open Arsenal" view), WCS-style; the grid is filtered by the selected button. Only categories that actually contain something in that arsenal get a button, each with its count; an *Other* button appears when items match no category.
- The column comes from an override of `UI/layouts/Menus/Inventory/InventoryContainerGrid.layout` that wraps the grid in a horizontal layout with a hidden `ARC_Sidebar` column (shown only while an arsenal is listed, so backpacks and crates look unchanged). If another mod replaces that layout, the buttons fall back to a two-column block above the grid — the feature never disappears.
- Filtering re-uses the vanilla item list (`SCR_ArsenalComponent.GetFilteredArsenalItems`), so supply costs, rank locks and enabled item types keep working exactly as before.
- Works in both places an arsenal can be listed: browsed from the **Vicinity** panel (the normal "Open Arsenal" flow) and opened as its own column.
- Buttons are the vanilla `WLib_ButtonTextImage` widget (icon + caption), so mouse, keyboard and gamepad navigation behave like the rest of the inventory.

## Customising the categories

### Server admins: JSON, no Workbench needed

On first start the server writes its current categories to

```text
$profile:ArsenalCategories/categories.json
```

(the `-profile` directory of a dedicated server; `Documents\My Games\ArmaReforger\profile` for a hosted game). Edit it, restart the scenario, and every player receives the new list when they spawn — clients never need to touch anything. A copy of the default file is in `Docs/categories.example.json`. Invalid JSON or an unknown type/mode name is reported in the server log (`[ARC] ...`) and the file is not pushed, so a typo can never break the arsenal; players fall back to the addon's built-in list.

```json
{
  "categories": [
    {
      "name": "Submachine Guns",
      "icon": "{71648F15B3984B87}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_AssaultRifles.edds",
      "itemTypes": ["RIFLE"],
      "itemModes": ["WEAPON", "WEAPON_VARIANTS"],
      "prefabContains": ["smg", "_mp5", "mpx"],
      "prefabExcludes": []
    }
  ]
}
```

| Key | Meaning |
|---|---|
| `name` | Button caption |
| `icon` | Icon resource (`{GUID}path.edds`); reuse the vanilla arsenal icons from the example, or `""` for none |
| `itemTypes` | `SCR_EArsenalItemType` names; `[]` = any type |
| `itemModes` | `SCR_EArsenalItemMode` names; `[]` = any mode |
| `prefabContains` | Prefab path must contain one of these (case-insensitive); `[]` = any |
| `prefabExcludes` | Prefab path must contain none of these |

Types: `RIFLE PISTOL LETHAL_THROWABLE ROCKET_LAUNCHER MACHINE_GUN HEAL BACKPACK SNIPER_RIFLE NON_LETHAL_THROWABLE HEADWEAR TORSO VEST_AND_WAIST LEGS FOOTWEAR RADIO_BACKPACK EQUIPMENT WEAPON_ATTACHMENT EXPLOSIVES HANDWEAR MORTARS HELICOPTER VEHICLE`. Modes: `DEFAULT WEAPON WEAPON_VARIANTS AMMUNITION CONSUMABLE ATTACHMENT SUPPORT_STATION PYLON`.

The same file in a client's own profile is used in single player, or when the server does not push one.

### Modders: the .conf

Categories also exist as `Configs/ArsenalCategories/ARC_ArsenalCategories.conf` (used when no JSON is available). Each entry is a name plus rules; an item must satisfy all of them (a mask of 0 / an empty list means "any"):

| Field | Meaning |
|---|---|
| `m_sName` | Button caption |
| `m_sIcon` | Button icon (`.edds`); the defaults reuse the vanilla arsenal icons |
| `m_eItemTypes` | `SCR_EArsenalItemType` flags the category includes |
| `m_eItemModes` | `SCR_EArsenalItemMode` flags the category includes |
| `m_aPrefabContains` | Prefab path must contain one of these substrings (case-insensitive) |
| `m_aPrefabExcludes` | Prefab path must contain none of these |

An item lands in the **first** category whose rules it satisfies, so order matters: narrow rules go first. That is how *Submachine Guns* works — the engine has no SMG type (mods tag them RIFLE), so it matches type RIFLE **and** a prefab name from a list (`smg`, `_mp5`, `mpx`, …) and sits above *Assault Rifles*. Magazines carry the type of their weapon but mode `AMMUNITION`, which is why weapon categories restrict modes to `WEAPON | WEAPON_VARIANTS` and *Ammunition* restricts the mode only.

The same rules let you map any mod's weapons: a category with `m_aPrefabContains {"rhs_"}` groups everything from RHS; `m_aPrefabExcludes` keeps a mod's launchers out of the vanilla one.

Open the config in Workbench to edit with proper enum pickers, or edit the numbers by hand (flags are stored as integer sums). If the config fails to load, the same defaults built into `ARC_ArsenalCategoryConfig.CreateDefault()` are used and `[ARC] Category config unavailable` is logged.

To ship different categories in your own addon, override this config (Resource Browser → right-click → *Override to…*) — the override keeps the same GUID, so the addon picks it up without code changes.

## Layout

```
Scripts/Game/ArsenalCategories/
  ARC_BaseGameMode.c                     server: writes/loads categories.json, pushes it on spawn
  ARC_PlayerController.c                 server->client transport for the JSON (chunked RPC)
  ARC_CategoryJson.c                     JSON parse/write, enum names <-> flags
  ARC_ArsenalCategory.c                  one category: name, icon, type/mode masks, prefab-name rules
  ARC_ArsenalCategoryConfig.c            config root + built-in defaults
  ARC_ArsenalFilterBar.c                 builds the icon+text button column from vanilla WLib_ButtonTextImage
  ARC_ArsenalFilterController.c          shared: selection state, bar lifecycle, filtered item list
  ARC_InventoryStorageLootUI.c           modded Vicinity panel (arsenal browsed in place)
  ARC_InventoryOpenedStorageArsenalUI.c  modded standalone arsenal column
Configs/ArsenalCategories/
  ARC_ArsenalCategories.conf             the shipped categories
UI/layouts/Menus/Inventory/
  InventoryContainerGrid.layout          override of the vanilla storage panel: adds the ARC_Sidebar column
```

Everything new is prefixed `ARC_`, including the members added to the modded classes and the widgets in the layout. The one shared resource is the `InventoryContainerGrid.layout` override; a second mod overriding the same layout wins or loses by load order, in which case the inline fallback kicks in.

## Testing in Workbench

1. Script Editor → **Validate Scripts (F7)**.
2. Open `worlds/MP/MpTest/MpTest_Basic.ent` (vanilla), create a sub-scene, and add three prefabs: `Prefabs/MP/Modes/Plain/GameMode_Plain.et`, `Prefabs/MP/Managers/Factions/FactionManager_USxUSSR.et` and `Prefabs/Props/Military/Arsenal/ArsenalBoxes/US/ArsenalBox_US.et`. Without the game mode every arsenal is empty (`needs a entity catalog manager!`); without exactly one faction manager the game mode crashes on init (`NULL pointer … m_FactionManager`) or complains `Multiple faction managers present!`.
3. **Play**, walk to the box, *Open Arsenal*. The category column appears to the left of the grid, under the "Arsenal" title. Click through the categories; counts add up to All. Clicking the active category keeps it active.
4. Check the Log Console for `[ARC]`: expect `Server categories loaded`, `Received server categories`, `Using categories pushed by the server`, and no warnings.
5. For RHS or other content mods, open the project with those addons (*Open with Addons*) and repeat with their arsenal boxes.

## License

Arma Public License (APL) — see LICENSE. Not affiliated with Bohemia Interactive.
