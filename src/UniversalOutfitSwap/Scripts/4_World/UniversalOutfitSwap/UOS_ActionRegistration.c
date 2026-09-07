modded class ActionConstructor
{
    override void RegisterActions(TTypenameArray actions)
    {
        super.RegisterActions(actions);

        actions.Insert(ActionUOSSwapOutfit);
        actions.Insert(ActionUOSStoreOutfit);
        actions.Insert(ActionUOSEquipOutfit);
    }
};
