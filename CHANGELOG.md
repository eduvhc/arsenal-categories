# Changelog

## 1.1.0

- Category tiles restyled to the inventory-slot look (icon, name, count column, orange selection frame), in one column that scrolls with the number of categories (`categoriesPerColumn: 0`; set a number to wrap into columns instead).
- Wide arsenal panel, WCS-style: own layout for the Vicinity panel only (categories inside the panel, 8-column grid), configurable through the JSON `layout` object / the .conf (`widePanel`, `columns`, `rows`, `categoriesPerColumn`, `categoryWidth`).
- Smart Buy, WCS-style, configurable through the JSON `buy` object / the .conf: magazines go to a pouch instead of into the weapon, weapon swap into an occupied holster slot (refund + request on the server), fallback storages for rejected items.
- Attachments while inspecting (`buy.arsenalAttachments`): the row under an attachment slot lists the arsenal's compatible attachments and magazines as buyable tiles, bought straight onto the weapon.
- Gamepad/keyboard navigation between the category grid and the item grid; the selected category is remembered when the inventory is reopened at the same arsenal.
- Saved arsenal loadouts persisted on the server (`loadouts.persist`): written to `$profile:ArsenalCategories/loadouts/<UUID>.json`, restored on connect.
- Arsenal tiles created over several frames (`layout.slotsPerFrame`, default 48) so large arsenals open instantly.
- 40 default categories (was 14): Marksman Rifles for DMRs / anti-materiel rifles mods leave on the RIFLE type (SR25, M110, M107, ORSIS), Submachine Guns also matches Scorpion,: weapons split further (Shotguns, Grenade Launchers), ammunition (Vehicle & Aircraft, Rockets & Shells, Launcher Rounds, Magazines), attachments (Optics, Muzzle Devices, Lasers & Lights, Grips & Bipods), clothing (Helmets, Face & Eyewear, Headwear, Jackets & Shirts, Trousers, Boots & Gloves, Vests, Pouches & Plates, Backpacks) and equipment (Medical, Navigation, Binoculars & Rangefinders, Radios, Night Vision, Flares & Lights, Tools & Kits, Deployables, Patches). Verified against every vanilla and RHS catalog entry: nothing lands in Other.
- Weapon categories exclude the AMMUNITION/ATTACHMENT modes instead of requiring WEAPON (RHS M4A1 Block 1 no longer lands in Other).
- Side column moved into the inventory content row; the InventoryContainerGrid.layout override is gone (it broke panel titles, drag and drop and the context menu).
- Icons are written to the JSON as strings; bare GUIDs are still accepted when reading.
- RHS-only example keeps the vanilla gear RHS has no replacement for (kits, tools, mortars, grenades).

## 1.0.0

- Category sidebar (WCS-style) left of the arsenal grid, icon + caption + item count per category; Vicinity panel and standalone arsenal column.
- 14 default categories incl. Submachine Guns by prefab name; data-driven via .conf.
- Server JSON (`$profile:ArsenalCategories/categories.json`) pushed to clients on spawn; client JSON fallback.
- Server-enforced visibility rules (show/hide by addon, item type, mode, prefab path).
