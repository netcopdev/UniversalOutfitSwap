modded class PlayerBase
{
    override void SetActions(out TInputActionMap InputActionMap)
    {
        super.SetActions(InputActionMap);

        // Player actions are evaluated after actions supplied by the targeted
        // object and its parent. This keeps storage Open/Close as the default.
        AddAction(ActionUOSSwapOutfit, InputActionMap);
        AddAction(ActionUOSStoreOutfit, InputActionMap);
        AddAction(ActionUOSEquipOutfit, InputActionMap);
    }
};
