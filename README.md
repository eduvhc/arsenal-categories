# Arsenal Categories

A small, open-source filter bar for the Arma Reforger arsenal panel.

Vanilla lists every item an arsenal offers in one flat grid. This addon adds a row of toggle buttons above that grid — *All, Weapons, Ammunition, Attachments, Throwables and Explosives, Clothing, Vests and Backpacks, Medical, Equipment* — so you only scroll through the category you are looking for. It works with any arsenal box (vanilla, RHS, WCS, …) because it hooks the shared arsenal UI rather than any faction's data.

## Requirements

- Arma Reforger. No other dependency.

Load it on the server like any other addon; clients receive it automatically. It only touches the client UI — nothing is changed on the server, in catalogs, prices or item availability, and it does not modify prefabs, so it coexists with arsenal content mods.

## What it does

- Inserts a filter bar into the arsenal storage panel (the "Open Arsenal" view). Only categories that actually contain something in that arsenal get a button; an *Other* button appears when items match no category.
- Filtering re-uses the vanilla item list (`SCR_ArsenalComponent.GetFilteredArsenalItems`), so supply costs, rank locks and enabled item types keep working exactly as before.
- Works in both places an arsenal can be listed: browsed from the **Vicinity** panel (the normal "Open Arsenal" flow) and opened as its own column.
- Buttons are the vanilla `WLib_ButtonFilter` widget, so mouse, keyboard and gamepad navigation behave like the rest of the inventory.

## Customising the categories

Categories are data: `Configs/ArsenalCategories/ARC_ArsenalCategories.conf`. Each entry has a name, an icon and two masks:

| Field | Meaning |
|---|---|
| `m_sName` | Shown in the storage title |
| `m_sIcon` | `.edds` texture, or an `.imageset` together with `m_sImageName` |
| `m_eItemTypes` | `SCR_EArsenalItemType` flags the category includes; `0` = any type |
| `m_eItemModes` | `SCR_EArsenalItemMode` flags the category includes; `0` = any mode |

An item lands in the **first** category whose masks it satisfies, so order matters. Magazines carry the type of their weapon but mode `AMMUNITION`, which is why *Weapons* restricts modes to `WEAPON | WEAPON_VARIANTS` and *Ammunition* restricts modes only.

Open the config in Workbench to edit with proper enum pickers, or edit the numbers by hand (flags are stored as integer sums). If the config fails to load, the same defaults built into `ARC_ArsenalCategoryConfig.CreateDefault()` are used and `[ARC] Category config unavailable` is logged.

To ship different categories in your own addon, override this config (Resource Browser → right-click → *Override to…*) — the override keeps the same GUID, so the addon picks it up without code changes.

## Layout

```
Scripts/Game/ArsenalCategories/
  ARC_ArsenalCategory.c                  one category: name, icon, type/mode masks
  ARC_ArsenalCategoryConfig.c            config root + built-in defaults
  ARC_ArsenalFilterBar.c                 builds the button row from vanilla WLib_ButtonFilter
  ARC_ArsenalFilterController.c          shared: selection state, bar lifecycle, filtered item list
  ARC_InventoryStorageLootUI.c           modded Vicinity panel (arsenal browsed in place)
  ARC_InventoryOpenedStorageArsenalUI.c  modded standalone arsenal column
Configs/ArsenalCategories/
  ARC_ArsenalCategories.conf             the shipped categories
```

Everything new is prefixed `ARC_`, including the members added to the modded class, so it will not collide with other arsenal mods.

## Testing in Workbench

1. Script Editor → **Validate Scripts (F7)**.
2. Open `worlds/MP/MpTest/MpTest_Basic.ent` (vanilla) and add two prefabs: `Prefabs/MP/Modes/Plain/GameMode_Plain.et`, `Prefabs/Props/Military/Arsenal/ArsenalBoxes/US/ArsenalBox_US.et`. `MpTest_Basic` already contains a faction manager; do not add a second one (`Multiple faction managers present!`). Without a game mode every arsenal is empty (`needs a entity catalog manager!` in the log).
3. **Play**, walk to the box, *Open Arsenal*. The filter bar sits right under the "Arsenal" title, above the grid. Click through the categories. Clicking the active category keeps it active.
4. Check the Log Console for `[ARC]` warnings — none should appear.
5. For RHS or other content mods, open the project with those addons (*Open with Addons*) and repeat with their arsenal boxes.

## License

Arma Public License (APL) — see LICENSE. Not affiliated with Bohemia Interactive.
