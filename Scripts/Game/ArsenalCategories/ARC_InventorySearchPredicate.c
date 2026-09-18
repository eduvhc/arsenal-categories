//! Lets a search predicate be asked about one entity directly. The inventory manager only runs
//! predicates over items it owns; the arsenal's items exist as preview entities, so the inspect
//! panel matches them here with the same IsMatch the predicate uses for real items.
modded class InventorySearchPredicate
{
	//------------------------------------------------------------------------------------------------
	//! \param item entity to test (a real item or a preview entity)
	//! \return true when the predicate accepts it
	bool ARC_Matches(notnull IEntity item)
	{
		array<GenericComponent> components = {};
		foreach (typename componentType : QueryComponentTypes)
		{
			GenericComponent component = GenericComponent.Cast(item.FindComponent(componentType));
			if (!component)
				return false;

			components.Insert(component);
		}

		array<BaseItemAttributeData> attributes = {};
		if (!QueryAttributeTypes.IsEmpty())
		{
			InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
			if (!itemComponent || !itemComponent.GetAttributes())
				return false;

			foreach (typename attributeType : QueryAttributeTypes)
			{
				BaseItemAttributeData attribute = itemComponent.GetAttributes().FindAttribute(attributeType);
				if (!attribute)
					return false;

				attributes.Insert(attribute);
			}
		}

		return IsMatch(null, item, components, attributes);
	}
}
