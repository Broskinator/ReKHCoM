/* Frontend regression: real JS services, controlled browser/native boundaries. */
const assert=require('node:assert/strict');
const fs=require('node:fs'),vm=require('node:vm'),path=require('node:path');
const {File}=require('node:buffer');
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
      listeners:{},options:[],dataset:{},classList:{toggle(){}},addEventListener(name,fn){this.listeners[name]=fn;},
      getBoundingClientRect(){return {width:1052,height:592};},focus(){},click(){},append(){}});
    return elements.get(id);
  }
  const document={documentElement:{classList:{toggle(){}}},fullscreenElement:null,getElementById:element,
    querySelectorAll(){return [];},addEventListener(name,fn){events[name]=fn;}};
  const context=vm.createContext({document,console,ReKHWeb:api,innerWidth:1920,innerHeight:1080,
    requestAnimationFrame(fn){const id=++frameId;frames.set(id,fn);return id;},cancelAnimationFrame(id){frames.delete(id);},
    window:{addEventListener(name,fn){events[name]=fn;},visualViewport:{addEventListener(){}}},
    ResizeObserver:class{constructor(fn){observed=fn;}observe(){}},Set,Blob,File,URL,TextDecoder,setTimeout,clearTimeout,
    navigator:{}});
  vm.runInContext(fs.readFileSync(path.join(web,'i18n.js'),'utf8'),context);
  vm.runInContext(fs.readFileSync(path.join(web,'web.js'),'utf8'),context);
  assert.equal(document.documentElement.lang,'en');assert.equal(element('status').textContent,'Choose a ROM to begin.');
  const sizes=[];context.record=(w,h)=>sizes.push([w,h]);
  let clears=0;context.clearInputRecord=()=>clears++;
  vm.runInContext('core={_PortWebResize:record,_PortWebClearInput:clearInputRecord};started=true;',context);
  const drain=()=>{const callbacks=[...frames.values()];frames.clear();callbacks.forEach(fn=>fn());};
  document.fullscreenElement=element('canvas');events.fullscreenchange();drain();assert.deepEqual(sizes.at(-1),[1920,1080]);
  // Escape exits DOM fullscreen without changing browser window dimensions.
  document.fullscreenElement=null;events.fullscreenchange();drain();assert.deepEqual(sizes.at(-1),[1052,592]);
  // F11 browser fullscreen uses resize rather than a DOM fullscreen element.
  context.innerHeight=900;events.resize();drain();assert.deepEqual(sizes.at(-1),[1052,592]);
  observed();observed();assert.equal(frames.size,1);drain();
  element('interface-language').value='es';element('interface-language').listeners.change();
  assert.equal(document.documentElement.lang,'es');assert.equal(element('pause').textContent,'Pausar');
  // F11 must request game fullscreen, rather than only resizing browser chrome.
  context.innerHeight=1080;
  element('canvas').requestFullscreen=async()=>{document.fullscreenElement=element('canvas');events.fullscreenchange();};
  let prevented=false;
  element('canvas').listeners.keydown({key:'F11',preventDefault(){prevented=true;},stopImmediatePropagation(){}});
  drain();assert(prevented);assert.equal(document.fullscreenElement,element('canvas'));
  assert.deepEqual(sizes.at(-1),[1920,1080]);
  document.fullscreenElement=null;delete element('canvas').requestFullscreen;
  await element('fullscreen').listeners.click();drain();
  assert.equal(vm.runInContext('expanded',context),true);
  assert.deepEqual(sizes.at(-1),[1052,592]);
  element('canvas').listeners.keydown({key:'Escape',preventDefault(){},stopImmediatePropagation(){}});
  drain();assert.equal(vm.runInContext('expanded',context),false);
  const before=clears;events.blur();element('canvas').listeners.blur();element('canvas').listeners.pointercancel();
  assert.equal(clears,before+3);
  // Storage/file boundaries are synthetic: exercise production import/export
  // without disclosing a user's cartridge or touching their real SRAM.
  const bytes=new Uint8Array(65536);bytes[0]=0x12;bytes[65535]=0xab;
  let written,writeCount=0,shared;
  context.saveCore={_PortWebClearInput:context.clearInputRecord,_PortWebActiveBank:()=>3,_PortWebFlushSave(){},
    FS:{readFile(name,options){
      if(options?.encoding)return 'save_bank=3\n';
      assert.equal(name,'/saves/kh_com_bank3.sav');return bytes;
    },writeFile(name,data){written={name,data};writeCount++;}}};
  vm.runInContext('core=saveCore;region=regions.B8CP;store={flush:async()=>true};started=false;',context);
  element('save').files=[new File([bytes],'backup.sav')];element('save').value='backup.sav';
  await element('save').listeners.change();
  assert.equal(written.name,'/saves/kh_com_bank3.sav');assert(Buffer.from(written.data).equals(Buffer.from(bytes)));
  assert.equal(element('save').value,'');
  element('save').files=[new File([new Uint8Array(12)],'wrong.sav')];
  await element('save').listeners.change();assert.equal(writeCount,1);
  vm.runInContext('started=true;',context);
  context.navigator.canShare=()=>{throw Error('file sharing unsupported');};context.navigator.share=async()=>{};
  await element('export').listeners.click();
  assert.equal(element('save-download').download,'kh_com_bank3.sav');assert(!element('save-export').hidden);
  assert(element('save-share').hidden);assert.equal(element('export').disabled,false);
  const firstUrl=element('save-download').href;
  context.navigator.canShare=()=>true;
  context.navigator.share=async data=>{shared=data.files[0];};
  await element('export').listeners.click();assert(!element('save-share').hidden);
  assert.notEqual(element('save-download').href,firstUrl);
  await element('save-share').listeners.click();
  assert.equal(shared.name,'kh_com_bank3.sav');assert.deepEqual(new Uint8Array(await shared.arrayBuffer()),bytes);
  context.navigator.share=async()=>{throw Object.assign(Error('user cancelled'),{name:'AbortError'});};
  const previousStatus=element('status').textContent;await element('save-share').listeners.click();
  assert.equal(element('status').textContent,previousStatus);
  context.navigator.share=async()=>{throw Object.assign(Error('no activation'),{name:'NotAllowedError'});};
  await element('save-share').listeners.click();assert.equal(element('status').textContent,vm.runInContext("t('shareFallback')",context));
  URL.revokeObjectURL(element('save-download').href);
  const html=fs.readFileSync(path.join(web,'index.html'),'utf8');
  for(const id of ['rom','save'])assert(!html.match(new RegExp(`<input[^>]*id="${id}"[^>]*>`))[0].includes('accept='));
  console.log('Web: sizing, focus/cancel release, iOS-compatible pickers, exact bank SRAM import/export, fresh-click share and download fallback PASS');
}
main().catch(error=>{console.error(error);process.exitCode=1;});
