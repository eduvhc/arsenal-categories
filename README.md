# Arsenal Categories

A small, open-source filter bar for the Arma Reforger arsenal panel.

Vanilla lists every item an arsenal offers in one flat grid. This addon adds a row of toggle buttons above that grid — *All, Weapons, Ammunition, Attachments, Throwables and Explosives, Clothing, Vests and Backpacks, Medical, Equipment* — so you only scroll through the category you are looking for. It works with any arsenal box (vanilla, RHS, WCS, …) because it hooks the shared arsenal UI rather than any faction's data.

## Requirements

- Arma Reforger. No other dependency.

Load it on the server like any other addon; clients receive it automatically. It only touches the client UI — nothing is changed on the server, in catalogs, prices or item availability, and it does not modify prefabs, so it coexists with arsenal content mods.

## What it does

- Inserts a filter bar into the arsenal storage panel (the "Open Arsenal" view). Only categories that actually contain something in that arsenal get a button; an *Other* button appears when items match no category.
- Filtering re-uses the vanilla item list (`SCR_ArsenalComponent.GetFilteredArsenalItems`), so supply costs, rank locks and enabled item types keep working exactly as before.
- The storage title shows the active category and item count, e.g. `Arsenal - Weapons (42)`.
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
  ARC_InventoryOpenedStorageArsenalUI.c  modded arsenal panel: creates the bar, filters GetAllItems()
Configs/ArsenalCategories/
  ARC_ArsenalCategories.conf             the shipped categories
```

Everything new is prefixed `ARC_`, including the members added to the modded class, so it will not collide with other arsenal mods.

## Testing in Workbench

1. Script Editor → **Validate Scripts (F7)**.
2. Open `worlds/MP/MpTest/MpTest_Basic.ent` (vanilla) and add three prefabs: `Prefabs/MP/Modes/Plain/GameMode_Plain.et`, `Prefabs/MP/Managers/Factions/FactionManager_USxUSSR.et` and `Prefabs/Props/Military/Arsenal/ArsenalBoxes/US/ArsenalBox_US.et`. Without the game mode and faction manager every arsenal is empty (`needs a entity catalog manager!` in the log).
3. **Play**, walk to the box, *Open Arsenal*. The filter bar sits between the panel header and the grid. Click through the categories; the title shows the count. Clicking the active category keeps it active.
4. Check the Log Console for `[ARC]` warnings — none should appear.
5. For RHS or other content mods, open the project with those addons (*Open with Addons*) and repeat with their arsenal boxes.

## License

Arma Public License (APL) — see LICENSE. Not affiliated with Bohemia Interactive.
