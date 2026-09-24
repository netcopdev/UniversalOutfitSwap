// Runs the actual batching/control-flow method bodies with a mocked DayZ boundary.
// Does not compile Enforce Script or simulate native inventory/network behavior.
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
const source=readFileSync(new URL('../src/UniversalOutfitSwap/Scripts/4_World/UniversalOutfitSwap/UOS_PendingTransaction.c',import.meta.url),'utf8');
function body(signature){
    const i=source.indexOf(signature); assert.ok(i>=0, signature);
    const begin=source.indexOf('{',i); let depth=1,end=begin+1;
    for(;depth && end<source.length;end++){if(source[end]==='{')depth++;if(source[end]==='}')depth--;}
    return source.slice(begin+1,end-1);
}
function compile(signature){
    let s=body(signature)
      .replace(/foreach \(\w+ (\w+) : ([^)]+)\)/g,'for (const $1 of $2)')
      .replace(/\b(?:UOS_PendingTransaction|UOS_PreparedTransfer|bool|string|int) (\w+) =/g,'let $1 =')
      .replace(/\bm_[A-Za-z]+\b/g,'this.$&')
      .replace(/\bs_Active\b/g,'active')
      .replace(/\b(Finish|AtStep)\(/g,'this.$1(')
      .replace(/\bCallLater\(Tick,/g,'CallLater(this.Tick,');
    return new Function('env','errorText', 'with(env) { with(this) {'+s+'} }');
}
const tick=compile('    void Tick()');
const dispatch=compile('    protected void Dispatch()');
const prepare=compile('    protected bool PrepareAndAcquire(');

function array(items=[]){items.Count=()=>items.length;items.Insert=x=>items.push(x);items.RemoveItem=x=>items.splice(items.indexOf(x),1);return items;}
Number.prototype.ToString=function(){return String(this);};
let now, events, deniedPrepare, deniedAcquire, active, accessible, env;
function fixture(count=11){
    now=1000;events=[];deniedPrepare=-1;deniedAcquire=-1;accessible=true;active=array();
    let next=0;
    const obj={m_Steps:array(Array.from({length:count},(_,i)=>({SlotName:'slot'+i,done:false}))), m_Prepared:array(),m_Operation:0,
      m_Storage:{GetPosition:()=>0},m_Player:{IsAlive:()=>true,GetIdentity:()=>({}),GetPosition:()=>0,GetSimulationTimeStamp:()=>1234,
        SendSyncJuncture(type,cmd){assert.equal(events.filter(x=>x[0]==='acquire').length,count,'all reservations precede first send');events.push(['send',cmd,now]);}},
      result:null, Tick(){tick.call(this,env);}, AtStep(player,storage,step){return step.done;},
      Finish(ok,text){this.result={ok,text};},
      PrepareAndAcquire(){return prepare.call(this,env,'');}, Dispatch(){dispatch.call(this,env);},
      ReleaseUnsent(){for(const x of this.m_Prepared)x.ReleaseUnsent(this.m_Player);}
    };
    env={GetGame:()=>({GetTime:()=>now,GetCallQueue:()=>({CallLater(){events.push(['timer']);}})}),
      UOS_Eligibility:{IsEligible:()=>accessible},vector:{Distance:()=>0},UAMaxDistances:{DEFAULT:2},
      GameInventory:{c_InventoryReservationTimeoutMS:5000},UOS_Log:{Debug(){}},CALL_CATEGORY_SYSTEM:0,
      DayZPlayerSyncJunctures:{SJ_INVENTORY:1},UOS_Operation:{UOS_OP_STORE:1,UOS_OP_EQUIP:2},
      UOS_PreparedTransfer:class {constructor(){this.id=next++;this.Command=this.id;} Prepare(){events.push(['prepare',this.id]);return this.id!==deniedPrepare;}
        Acquire(){events.push(['acquire',this.id]);return this.id!==deniedAcquire;} ReleaseUnsent(){events.push(['release',this.id]);}},
      active};
    return obj;
}
let t=fixture();assert.equal(t.PrepareAndAcquire(),true);t.Dispatch();
assert.equal(events.filter(x=>x[0]==='send').length,11,'all slots dispatched before returning');
assert.equal(new Set(events.filter(x=>x[0]==='send').map(x=>x[2])).size,1,'one dispatch timestamp');
assert.equal(events.at(-1)[0],'timer','watchdog starts after every command is sent');
assert.equal(events.slice(0,11).every(x=>x[0]==='prepare'),true,'prepare all before acquiring');
t.Tick();assert.equal(t.result,null);t.m_Steps[0].done=true;t.Tick();assert.equal(t.result,null,'partial completion is not success');
for(const s of t.m_Steps)s.done=true;t.Tick();assert.equal(t.result.ok,true);
assert.equal(events.filter(x=>x[0]==='send').length,11,'polling never resubmits');

t=fixture();deniedPrepare=5;assert.equal(t.PrepareAndAcquire(),false);t.ReleaseUnsent();assert.equal(events.some(x=>x[0]==='acquire'||x[0]==='send'),false);
t=fixture();deniedAcquire=5;assert.equal(t.PrepareAndAcquire(),false);t.ReleaseUnsent();assert.equal(events.some(x=>x[0]==='send'),false);
assert.equal(events.filter(x=>x[0]==='release').length,11,'all prepared entries cleaned after acquisition failure');

t=fixture();t.PrepareAndAcquire();t.Dispatch();now=8000;t.Tick();assert.equal(t.result.ok,false);assert.match(t.result.text,/timed out/);
assert.equal(events.some(x=>x[0]==='release'),false,'do not unlock submitted commands on timeout');
t=fixture();t.PrepareAndAcquire();t.Dispatch();t.m_Player=null;t.Tick();assert.equal(t.result.ok,false);
t=fixture();accessible=false;assert.equal(t.PrepareAndAcquire(),false);assert.deepEqual(events,[]);
for(const [op,verb] of [[1,'stored'],[2,'equipped']]){t=fixture();t.m_Operation=op;t.PrepareAndAcquire();t.Dispatch();for(const s of t.m_Steps)s.done=true;t.Tick();assert.equal(t.result.text,'Outfit '+verb+'.');}
// Ensure Start reports preparation failure before dispatch, and propagates it.
const startBody=body('    static bool Start(');
assert.ok(startBody.indexOf('PrepareAndAcquire')<startBody.indexOf('pending.Dispatch()'));
assert.match(startBody,/pending.ReleaseUnsent\(\);[\s\S]*s_Active.RemoveItem\(pending\);[\s\S]*return false;/);
console.log('Batch scheduling checks passed: 11 slots in one dispatch, reservation failure, partial completion, timeout, disconnect, access, Store/Equip.');


// Run actual reservation bookkeeping against owned versus existing junctures.
// Match DayZ 1.29 exactly: NOT_REQUIRED=0, ACQUIRED=1, DENIED=2, ERROR=3.
const acquire=compile('    bool Acquire(');
const release=compile('    void ReleaseUnsent(PlayerBase player)');
const junctureResults={
    JUNCTURE_NOT_REQUIRED:0,
    JUNCTURE_ACQUIRED:1,
    JUNCTURE_DENIED:2,
    ERROR:3
};
for(const testCase of [
    {name:'not required',result:junctureResults.JUNCTURE_NOT_REQUIRED,expected:true,acquireNew:false},
    {name:'acquired',result:junctureResults.JUNCTURE_ACQUIRED,expected:true,acquireNew:true},
    {name:'denied after partial acquisition',result:junctureResults.JUNCTURE_DENIED,expected:false,acquireNew:true},
    {name:'error after partial acquisition',result:junctureResults.ERROR,expected:false,acquireNew:true}
]) {
    const held=new Set(['existing']);
    const transfer={Src1:{GetItem:()=> 'existing'}, Src2:{GetItem:()=> 'new'},
        Dst1:{},Dst2:{},OwnsJuncture1:false,OwnsJuncture2:false};
    const reservationEnv={player:{}, GetGame:()=>({HasInventoryJuncture:(p,item)=>held.has(item),ClearJunctureEx:(p,item)=>held.delete(item)}),
        TryAcquireTwoInventoryJuncturesFromServer(){
            if(testCase.acquireNew) held.add('new');
            return testCase.result;
        },
        JunctureRequestResult:junctureResults};
    assert.equal(acquire.call(transfer,reservationEnv),testCase.expected,testCase.name);
    assert.equal(transfer.OwnsJuncture1,false,testCase.name+' preserves pre-existing first juncture');
    assert.equal(transfer.OwnsJuncture2,testCase.acquireNew,testCase.name+' tracks only a newly acquired second juncture');
    release.call(transfer,reservationEnv);
    assert.deepEqual([...held],['existing'],testCase.name+' cleanup preserves existing ownership');
}
console.log('Reservation enum, ownership and partial-acquisition cleanup checks passed.');
