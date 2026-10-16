/* Frontend regression: real JS services, controlled browser/native boundaries. */
const assert=require('node:assert/strict');
const fs=require('node:fs'),vm=require('node:vm'),path=require('node:path');
const web=path.resolve(__dirname,'../web'),api=require(path.join(web,'runtime.js'));
async function main(){
  assert.deepEqual(api.fitSize(5120,1440),{width:1920,height:540});
  assert.deepEqual(api.fitSize(1052,591.75),{width:1052,height:592});
  assert.deepEqual(api.fitSize(360,202.5),{width:360,height:203});
  assert.equal(api.saveName('kh_com_jp.sav',4),'kh_com_jp_bank4.sav');
  let active=0,maxActive=0,calls=0,resolvers=[];
  const store=api.persistence(()=>{calls++;active++;maxActive=Math.max(maxActive,active);return new Promise(resolve=>resolvers.push(()=>{active--;resolve();}));},()=>assert.fail('unexpected failure'));
  const first=store.flush();store.flush();assert(store.pending);resolvers.shift()();
  await new Promise(resolve=>setImmediate(resolve));assert.equal(calls,2);resolvers.shift()();await first;
  assert.equal(maxActive,1);assert(!store.pending);
  let failures=0;
  const broken=api.persistence(async()=>{throw Error('quota');},()=>failures++);
  assert.equal(await broken.flush(),false);assert.equal(await broken.flush(),false);assert.equal(failures,1);
  assert(!broken.pending);
  const elements=new Map(),events={},frames=new Map();let frameId=0,observed;
  function element(id){
    if(!elements.has(id))elements.set(id,{id,value:id==='language'?'en':'',textContent:'',hidden:false,width:0,height:0,
      listeners:{},options:[],dataset:{},addEventListener(name,fn){this.listeners[name]=fn;},
      getBoundingClientRect(){return {width:1052,height:592};},focus(){},click(){},append(){}});
    return elements.get(id);
  }
  const document={documentElement:{},fullscreenElement:null,getElementById:element,
    querySelectorAll(){return [];},addEventListener(name,fn){events[name]=fn;}};
  const context=vm.createContext({document,console,ReKHWeb:api,innerWidth:1920,innerHeight:1080,
    requestAnimationFrame(fn){const id=++frameId;frames.set(id,fn);return id;},cancelAnimationFrame(id){frames.delete(id);},
    window:{addEventListener(name,fn){events[name]=fn;},visualViewport:{addEventListener(){}}},
    ResizeObserver:class{constructor(fn){observed=fn;}observe(){}},Set,Blob,URL,TextDecoder,setTimeout,clearTimeout});
  vm.runInContext(fs.readFileSync(path.join(web,'i18n.js'),'utf8'),context);
  vm.runInContext(fs.readFileSync(path.join(web,'web.js'),'utf8'),context);
  assert.equal(document.documentElement.lang,'en');assert.equal(element('status').textContent,'Choose a ROM to begin.');
  const sizes=[];context.record=(w,h)=>sizes.push([w,h]);
  vm.runInContext('core={_PortWebResize:record};started=true;',context);
  const drain=()=>{const callbacks=[...frames.values()];frames.clear();callbacks.forEach(fn=>fn());};
  document.fullscreenElement=element('canvas');events.fullscreenchange();drain();assert.deepEqual(sizes.at(-1),[1920,1080]);
  // Escape exits DOM fullscreen without changing browser window dimensions.
  document.fullscreenElement=null;events.fullscreenchange();drain();assert.deepEqual(sizes.at(-1),[1052,592]);
  // F11 browser fullscreen uses resize rather than a DOM fullscreen element.
  context.innerHeight=900;events.resize();drain();assert.deepEqual(sizes.at(-1),[1052,592]);
  observed();observed();assert.equal(frames.size,1);drain();
  element('interface-language').value='es';element('interface-language').listeners.change();
  assert.equal(document.documentElement.lang,'es');assert.equal(element('pause').textContent,'Pausar');
  console.log('Web: fullscreen/Escape/F11 sizing, resize coalescing, languages, bank naming, serial saves and quota recovery PASS');
}
main().catch(error=>{console.error(error);process.exitCode=1;});
