import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';

const sync=readFileSync(new URL('../src/UniversalOutfitSwap/Scripts/4_World/UniversalOutfitSwap/UOS_ConfigSync.c',import.meta.url),'utf8');
const actions=readFileSync(new URL('../src/UniversalOutfitSwap/Scripts/4_World/UniversalOutfitSwap/UOS_Actions.c',import.meta.url),'utf8');
const mission=readFileSync(new URL('../src/UniversalOutfitSwap/Scripts/5_Mission/UniversalOutfitSwap/UOS_MissionServer.c',import.meta.url),'utf8');

function assertOrdered(text, needles, label) {
    let pos=-1;
    for(const needle of needles) {
        const next=text.indexOf(needle,pos+1);
        assert.ok(next>pos, label+': missing/out-of-order '+needle);
        pos=next;
    }
}

assertOrdered(sync,[
    'rpc.Write(UOS_CONFIG_SYNC_PROTOCOL);',
    'rpc.Write(cfg.Enabled);',
    'rpc.Write(cfg.EnableSwapOutfit);',
    'rpc.Write(cfg.EnableStoreOutfit);',
    'rpc.Write(cfg.EnableEquipOutfit);',
    'rpc.Write(cfg.MinWearableSlots);',
    'rpc.Write(cfg.RequireOpenWhenOpenable);',
    'WriteStringArray(rpc, cfg.RequiredSlots);',
    'WriteStringArray(rpc, cfg.IncludeClasses);',
    'WriteStringArray(rpc, cfg.ExcludeClasses);'
],'server serialization');

assertOrdered(sync,[
    'ctx.Read(protocol)',
    'ctx.Read(enabled)',
    'ctx.Read(enableSwapOutfit)',
    'ctx.Read(enableStoreOutfit)',
    'ctx.Read(enableEquipOutfit)',
    'ctx.Read(minWearableSlots)',
    'ctx.Read(requireOpenWhenOpenable)',
    'ReadStringArray(ctx, requiredSlots)',
    'ReadStringArray(ctx, includeClasses)',
    'ReadStringArray(ctx, excludeClasses)'
],'client deserialization');

for(const field of ['NotifyOnSuccess','NotifyOnFailure','DebugLogging'])
    assert.ok(!sync.includes('cfg.'+field),'server-only field leaked into client eligibility snapshot: '+field);

assert.ok(actions.indexOf('player.UOS_HasEligibilityConfig()') < actions.indexOf('UOS_Eligibility.IsOperationEnabled(m_UOSOperation)'),
    'action visibility must fail closed before evaluating client defaults');

for(const hook of ['OnClientReadyEvent','OnClientReconnectEvent','OnClientRespawnEvent']) {
    const start=mission.indexOf('override void '+hook);
    assert.ok(start>=0,'missing '+hook);
    const send=mission.indexOf('UOS_ConfigSync.SendToClient(player, identity);',start);
    assert.ok(send>start,hook+' does not send config');
}

console.log('Client eligibility config sync contract checks passed.');
