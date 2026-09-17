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
class ARC_CategoryJson
{
	static const string FILE_PATH = "$profile:ArsenalCategories/categories.json";
	static const string DIRECTORY = "$profile:ArsenalCategories";

	//------------------------------------------------------------------------------------------------
	//! Parse a JSON document into categories.
	//! \return false when the document is invalid or contains no usable category
	static bool Parse(string json, out notnull array<ref ARC_ArsenalCategory> categories)
	{
		JsonLoadContext context = new JsonLoadContext();
		if (!context.LoadFromString(json))
		{
			Print("[ARC] categories JSON is not valid JSON", LogLevel.WARNING);
			return false;
		}

		return Read(context, categories);
	}

	//------------------------------------------------------------------------------------------------
	//! Load categories from a JSON file in the profile directory.
	//! \return false when the file does not exist or is unusable
	static bool LoadFile(string path, out notnull array<ref ARC_ArsenalCategory> categories)
	{
		if (!FileIO.FileExists(path))
			return false;

		JsonLoadContext context = new JsonLoadContext();
		if (!context.LoadFromFile(path))
		{
			Print("[ARC] " + path + " is not valid JSON", LogLevel.WARNING);
			return false;
		}

		return Read(context, categories);
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
	static bool SaveFile(string path, notnull array<ref ARC_ArsenalCategory> categories)
	{
		PrettyJsonSaveContext context = new PrettyJsonSaveContext();

		int count = categories.Count();
		context.StartArray("categories", count);
		foreach (ARC_ArsenalCategory category : categories)
		{
			context.StartObject();
			context.WriteValue("name", category.GetName());
			context.WriteValue("icon", category.GetIcon());

			array<string> typeNames = {};
			FlagsToNames(SCR_EArsenalItemType, category.GetItemTypes(), typeNames);
			context.WriteValue("itemTypes", typeNames);

			array<string> modeNames = {};
			FlagsToNames(SCR_EArsenalItemMode, category.GetItemModes(), modeNames);
			context.WriteValue("itemModes", modeNames);

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

		if (context.SaveToFile(path))
			return true;

		Print("[ARC] Failed to write " + path, LogLevel.WARNING);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Read(notnull JsonLoadContext context, out notnull array<ref ARC_ArsenalCategory> categories)
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
			ResourceName icon;
			array<string> typeNames = {};
			array<string> modeNames = {};
			array<string> contains = {};
			array<string> excludes = {};

			context.ReadValue("name", name);
			context.ReadValue("icon", icon);
			context.ReadValue("itemTypes", typeNames);
			context.ReadValue("itemModes", modeNames);
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
			categories.Insert(ARC_ArsenalCategory.Create(name, icon, types, modes, contains, excludes));
		}

		context.EndArray();

		if (categories.IsEmpty())
		{
			Print("[ARC] categories JSON defines no usable category", LogLevel.WARNING);
			return false;
		}

		return true;
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
