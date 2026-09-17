//! JSON form of the category list, so server admins can change categories without Workbench:
//!
//! {
//!   "categories": [
//!     {
//!       "name": "Submachine Guns",
//!       "icon": "{71648F15B3984B87}UI/Textures/Editor/Attributes/Arsenal/Attribute_Arsenal_AssaultRifles.edds",
//!       "itemTypes": ["RIFLE"],
//!       "itemModes": ["WEAPON", "WEAPON_VARIANTS"],
//!       "prefabContains": ["smg", "_mp5"],
//!       "prefabExcludes": []
//!     }
//!   ]
//! }
//!
//! itemTypes / itemModes use the SCR_EArsenalItemType / SCR_EArsenalItemMode names; an empty list
//! means "any". Unknown names are reported and skipped. Same first-match semantics as the .conf.
//!
//! An optional "visibility" array hides items from every arsenal (enforced on the server too):
//!
//!   "visibility": [
//!     { "action": "show", "prefabContains": ["/Medicine/", "/Maps/", "/Compass/"] },
//!     { "action": "show", "addons": ["RHS_Core", "RHS_Content_01", "RHS_Content_02"] },
//!     { "action": "hide" }
//!   ]
//!
//! Rules are checked in order, first match decides, unmatched items are shown. Conditions:
//! addons, addonsExclude (addon.gproj IDs), itemTypes, itemModes, prefabContains, prefabExcludes.
class ARC_CategoryJson
{
	static const string FILE_PATH = "$profile:ArsenalCategories/categories.json";
	static const string DIRECTORY = "$profile:ArsenalCategories";

	//------------------------------------------------------------------------------------------------
	//! Parse a JSON document into categories.
	//! \return false when the document is invalid or contains no usable category
	static bool Parse(string json, out notnull array<ref ARC_ArsenalCategory> categories, out notnull array<ref ARC_VisibilityRule> rules)
	{
		JsonLoadContext context = new JsonLoadContext();
		if (!context.LoadFromString(json))
		{
			Print("[ARC] categories JSON is not valid JSON", LogLevel.WARNING);
			return false;
		}

		return Read(context, categories, rules);
	}

	//------------------------------------------------------------------------------------------------
	//! Load categories from a JSON file in the profile directory.
	//! \return false when the file does not exist or is unusable
	static bool LoadFile(string path, out notnull array<ref ARC_ArsenalCategory> categories, out notnull array<ref ARC_VisibilityRule> rules)
	{
		if (!FileIO.FileExists(path))
			return false;

		JsonLoadContext context = new JsonLoadContext();
		if (!context.LoadFromFile(path))
		{
			Print("[ARC] " + path + " is not valid JSON", LogLevel.WARNING);
			return false;
		}

		return Read(context, categories, rules);
	}

	//------------------------------------------------------------------------------------------------
	//! Read a whole file into a string (for pushing to clients).
	static string ReadFileText(string path)
	{
		if (!FileIO.FileExists(path))
			return string.Empty;

		FileHandle file = FileIO.OpenFile(path, FileMode.READ);
		if (!file)
			return string.Empty;

		string text;
		string line;
		while (!file.IsEOF())
		{
			file.ReadLine(line);
			text += line;
		}

		file.Close();
		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! Write categories as a JSON file, e.g. to give admins a template with the defaults.
	static bool SaveFile(string path, notnull array<ref ARC_ArsenalCategory> categories, array<ref ARC_VisibilityRule> rules = null)
	{
		PrettyJsonSaveContext context = new PrettyJsonSaveContext();

		int count = categories.Count();
		context.StartArray("categories", count);
		foreach (ARC_ArsenalCategory category : categories)
		{
			context.StartObject();
			context.WriteValue("name", category.GetName());

			// As a ResourceName the serializer keeps only the GUID; as a string the full {GUID}path survives.
			string iconText = category.GetIcon();
			context.WriteValue("icon", iconText);

			array<string> typeNames = {};
			FlagsToNames(SCR_EArsenalItemType, category.GetItemTypes(), typeNames);
			context.WriteValue("itemTypes", typeNames);

			array<string> modeNames = {};
			FlagsToNames(SCR_EArsenalItemMode, category.GetItemModes(), modeNames);
			context.WriteValue("itemModes", modeNames);

			array<string> typeExcludeNames = {};
			FlagsToNames(SCR_EArsenalItemType, category.GetItemTypesExclude(), typeExcludeNames);
			context.WriteValue("itemTypesExclude", typeExcludeNames);

			array<string> modeExcludeNames = {};
			FlagsToNames(SCR_EArsenalItemMode, category.GetItemModesExclude(), modeExcludeNames);
			context.WriteValue("itemModesExclude", modeExcludeNames);

			array<string> contains = {};
			if (category.GetPrefabContains())
				contains.Copy(category.GetPrefabContains());
			context.WriteValue("prefabContains", contains);

			array<string> excludes = {};
			if (category.GetPrefabExcludes())
				excludes.Copy(category.GetPrefabExcludes());
			context.WriteValue("prefabExcludes", excludes);

			context.EndObject();
		}
		context.EndArray();

		int ruleCount;
		if (rules)
			ruleCount = rules.Count();

		context.StartArray("visibility", ruleCount);
		for (int i = 0; i < ruleCount; i++)
		{
			ARC_VisibilityRule rule = rules[i];
			context.StartObject();

			string action = "hide";
			if (rule.IsShow())
				action = "show";

			context.WriteValue("action", action);
			context.WriteValue("addons", rule.GetAddons());
			context.WriteValue("addonsExclude", rule.GetAddonsExclude());

			array<string> ruleTypes = {};
			FlagsToNames(SCR_EArsenalItemType, rule.GetItemTypes(), ruleTypes);
			context.WriteValue("itemTypes", ruleTypes);

			array<string> ruleModes = {};
			FlagsToNames(SCR_EArsenalItemMode, rule.GetItemModes(), ruleModes);
			context.WriteValue("itemModes", ruleModes);

			context.WriteValue("prefabContains", rule.GetPrefabContains());
			context.WriteValue("prefabExcludes", rule.GetPrefabExcludes());
			context.EndObject();
		}
		context.EndArray();

		if (context.SaveToFile(path))
			return true;

		Print("[ARC] Failed to write " + path, LogLevel.WARNING);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Read(notnull JsonLoadContext context, out notnull array<ref ARC_ArsenalCategory> categories, out notnull array<ref ARC_VisibilityRule> rules)
	{
		int count;
		if (!context.StartArray("categories", count))
		{
			Print("[ARC] categories JSON has no \"categories\" array", LogLevel.WARNING);
			return false;
		}

		for (int i = 0; i < count; i++)
		{
			if (!context.StartObject())
				break;

			string name;
			string iconText;
			array<string> typeNames = {};
			array<string> modeNames = {};
			array<string> typeExcludeNames = {};
			array<string> modeExcludeNames = {};
			array<string> contains = {};
			array<string> excludes = {};

			context.ReadValue("name", name);
			context.ReadValue("icon", iconText);
			context.ReadValue("itemTypes", typeNames);
			context.ReadValue("itemModes", modeNames);
			context.ReadValue("itemTypesExclude", typeExcludeNames);
			context.ReadValue("itemModesExclude", modeExcludeNames);
			context.ReadValue("prefabContains", contains);
			context.ReadValue("prefabExcludes", excludes);
			context.EndObject();

			if (name.IsEmpty())
			{
				PrintFormat("[ARC] categories[%1] has no name; skipped", i, level: LogLevel.WARNING);
				continue;
			}

			SCR_EArsenalItemType types = NamesToFlags(SCR_EArsenalItemType, typeNames, name);
			SCR_EArsenalItemMode modes = NamesToFlags(SCR_EArsenalItemMode, modeNames, name);
			SCR_EArsenalItemType typesExclude = NamesToFlags(SCR_EArsenalItemType, typeExcludeNames, name);
			SCR_EArsenalItemMode modesExclude = NamesToFlags(SCR_EArsenalItemMode, modeExcludeNames, name);
			categories.Insert(ARC_ArsenalCategory.Create(name, ToResourceName(iconText), types, modes, contains, excludes, typesExclude, modesExclude));
		}

		context.EndArray();

		if (categories.IsEmpty())
		{
			Print("[ARC] categories JSON defines no usable category", LogLevel.WARNING);
			return false;
		}

		ReadVisibility(context, rules);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Optional "visibility" array; absent = no rules.
	protected static void ReadVisibility(notnull JsonLoadContext context, out notnull array<ref ARC_VisibilityRule> rules)
	{
		int count;
		if (!context.StartArray("visibility", count))
			return;

		for (int i = 0; i < count; i++)
		{
			if (!context.StartObject())
				break;

			string action;
			array<string> addons = {};
			array<string> addonsExclude = {};
			array<string> typeNames = {};
			array<string> modeNames = {};
			array<string> contains = {};
			array<string> excludes = {};

			context.ReadValue("action", action);
			context.ReadValue("addons", addons);
			context.ReadValue("addonsExclude", addonsExclude);
			context.ReadValue("itemTypes", typeNames);
			context.ReadValue("itemModes", modeNames);
			context.ReadValue("prefabContains", contains);
			context.ReadValue("prefabExcludes", excludes);
			context.EndObject();

			action.ToLower();
			action.Trim();
			if (action != "show" && action != "hide")
			{
				PrintFormat("[ARC] visibility[%1]: action must be show or hide (got %2); rule skipped", i, action, level: LogLevel.WARNING);
				continue;
			}

			string ruleName = string.Format("visibility[%1]", i);
			SCR_EArsenalItemType types = NamesToFlags(SCR_EArsenalItemType, typeNames, ruleName);
			SCR_EArsenalItemMode modes = NamesToFlags(SCR_EArsenalItemMode, modeNames, ruleName);
			rules.Insert(ARC_VisibilityRule.Create(action == "show", addons, addonsExclude, types, modes, contains, excludes));
		}

		context.EndArray();
	}

	//------------------------------------------------------------------------------------------------
	//! Accepts "{GUID}path", "{GUID}" or a bare 16-hex GUID (what older files written by the
	//! ResourceName serializer contain) and returns something the engine can load.
	protected static ResourceName ToResourceName(string text)
	{
		text.Trim();
		if (text.IsEmpty() || text.StartsWith("{"))
			return text;

		if (text.Length() == 16)
			return "{" + text + "}";

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! "RIFLE" | "PISTOL" -> flag mask. Unknown names are logged with the category they belong to.
	protected static int NamesToFlags(typename enumType, notnull array<string> names, string categoryName)
	{
		int flags;
		array<string> enumNames = {};
		array<int> enumValues = {};
		SCR_Enum.GetEnumNames(enumType, enumNames);
		SCR_Enum.GetEnumValues(enumType, enumValues);

		foreach (string rawName : names)
		{
			string wanted = rawName;
			wanted.ToUpper();
			wanted.Trim();

			int index = enumNames.Find(wanted);
			if (index < 0)
			{
				PrintFormat("[ARC] Category \"%1\": unknown %2 value \"%3\"; ignored", categoryName, enumType.ToString(), rawName, level: LogLevel.WARNING);
				continue;
			}

			flags |= enumValues[index];
		}

		return flags;
	}

	//------------------------------------------------------------------------------------------------
	protected static void FlagsToNames(typename enumType, int flags, out notnull array<string> names)
	{
		array<string> enumNames = {};
		array<int> enumValues = {};
		SCR_Enum.GetEnumNames(enumType, enumNames);
		SCR_Enum.GetEnumValues(enumType, enumValues);

		foreach (int i, int value : enumValues)
		{
			if (value != 0 && (flags & value) == value)
				names.Insert(enumNames[i]);
		}
	}
}
