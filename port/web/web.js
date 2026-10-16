'use strict';
const $ = id => document.getElementById(id);
const regions = {
  B8CP: {id:'eu', sha:'8db73586cdb11b3795907edebf43228dbcd3e6b2', save:'kh_com.sav'},
  B8CE: {id:'us', sha:'10729bd884f8fdca7a310b6d606c52e46657aa48', save:'kh_com_us.sav'},
  B8CJ: {id:'jp', sha:'59ec0a0a4ccd1e6acb3bbd7bfb21d63988958cfa', save:'kh_com_jp.sav'}
};
let core,region,store,started=false,loading=false,paused=false,resizeFrame=0,pendingSave=false;
const lines=[];
const warningKeys=new Set();
function log(message){console.log(message);lines.push(String(message));if(lines.length>60)lines.shift();$('log').textContent=lines.join('\n');}
function status(key){$('status').textContent=key?t(key):'';}
function warning(key){warningKeys.add(key);$('storage-status').textContent=Array.from(warningKeys,t).join(' ');$('storage-status').hidden=false;}
function fail(error){log(error.stack||error);$('status').textContent=t('error')+error.message;}
function sync(populate=false){return new Promise((resolve,reject)=>core.FS.syncfs(populate,error=>error?reject(error):resolve()));}
function canvasSize(){
  const canvas=$('canvas'),full=document.fullscreenElement===canvas;
  const width=full?innerWidth:canvas.getBoundingClientRect().width||Math.min(innerWidth-28,1052);
  const size=ReKHWeb.fitSize(width,full?innerHeight:width*9/16);
  if(started)core._PortWebResize(size.width,size.height);
  else{canvas.width=size.width;canvas.height=size.height;}
  $('fullscreen').textContent=t(full?'exitFullscreen':'fullscreen');
}
function scheduleResize(){
  cancelAnimationFrame(resizeFrame);resizeFrame=requestAnimationFrame(canvasSize);
}
function visibility(){
  if(!started)return;
  core._PortWebSetVisible(!document.hidden&&!paused?1:0);
  if(document.hidden||paused)store.flush();
}
async function flushSave(){
  pendingSave=true;
  try{if(started)core._PortWebFlushSave();return await store.flush();}
  finally{pendingSave=false;}
}
async function loadEngine(){
  const manifest=await fetch('manifest.json',{cache:'no-cache'}).then(response=>{if(!response.ok)throw Error(t('offline'));return response.json();});
  const name=`rekhcom-${region.id}.js`,wasm=`rekhcom-${region.id}.wasm`;
  const jsVersion=manifest.files[name].sha256,wasmVersion=manifest.files[wasm].sha256;
  await new Promise((resolve,reject)=>{
    const script=document.createElement('script');script.src=`${name}?v=${jsVersion}`;
    script.onload=resolve;script.onerror=()=>reject(Error(t('engine')));document.head.append(script);
  });
  canvasSize();
  return createReKHCoM({canvas:$('canvas'),webFiles:manifest.files,noInitialRun:true,print:log,printErr:log,
    locateFile:path=>path.endsWith('.wasm')?`${wasm}?v=${wasmVersion}`:path,
    onAbort:message=>fail(Error(String(message)))});
}
$('rom').addEventListener('change',async()=>{
  if(loading||core)return;
  const file=$('rom').files[0];if(!file)return;
  loading=true;$('rom').disabled=true;
  try{
    if(file.size!==33554432)throw Error(t('romSize'));
    status('checking');let buffer=await file.arrayBuffer();
    const code=new TextDecoder().decode(new Uint8Array(buffer,0xac,4));region=regions[code];
    if(!region)throw Error(t('region'));
    const hash=Array.from(new Uint8Array(await crypto.subtle.digest('SHA-1',buffer)),n=>n.toString(16).padStart(2,'0')).join('');
    if(hash!==region.sha)throw Error(t('romHash'));
    status('loading');core=await loadEngine();
    core.HEAPU8.set(new Uint8Array(buffer),0x08000000);buffer=null;
    if(!core._PortWebRomReady(file.size))throw Error(t('romHash'));
    core.FS.mkdir('/saves');core.FS.mount(core.IDBFS,{},'/saves');
    store=ReKHWeb.persistence(sync,error=>{log(error);warning('storage');});
    try{await sync(true);}catch(error){store.disable();log(error);warning('storage');}
    core.persistSoon=()=>store.request();
    core.saveBeforeReset=async()=>{
      status('reset');pendingSave=true;
      try{const ok=await store.flush();if(!ok)status('resetFailed');return ok?1:0;}
      finally{pendingSave=false;}
    };
    try{await ReKHWeb.fonts(core);core.hdReady=true;core._PortWebSetHdReady(1);}
    catch(error){log(error);core.hdReady=false;core._PortWebSetHdReady(0);warning('fonts');}
    const allowed=region.id==='eu'?['en','fr','de','es','it']:region.id==='jp'?['ja']:['en'];
    if(region.id==='jp'&&!Array.from($('language').options).some(option=>option.value==='ja'))$('language').add(new Option('日本語','ja'));
    Array.from($('language').options).forEach(option=>option.disabled=!allowed.includes(option.value));
    if(!allowed.includes($('language').value))$('language').value=allowed[0];
    $('touch').checked=matchMedia('(pointer:coarse)').matches;
    core._PortWebTouchEnable($('touch').checked?1:0);
    $('play').disabled=false;$('save').disabled=false;status('ready');
  }catch(error){fail(error);core=null;region=null;$('rom').disabled=false;$('rom').value='';}
  finally{loading=false;}
});
$('save').addEventListener('change',async()=>{
  try{
    if(started)throw Error(t('beforePlay'));
    const file=$('save').files[0];if(!file)return;
    if(file.size!==65536)throw Error(t('saveSize'));
    // Use the configured bank, so importing does not silently target another save.
    let bank=1;
    try{const ini=core.FS.readFile('/saves/kh_com.ini',{encoding:'utf8'});bank=Number(ini.match(/^save_bank=(\d+)$/m)?.[1]||1);}catch{}
    bank=Math.max(1,Math.min(5,bank));
    core.FS.writeFile('/saves/'+ReKHWeb.saveName(region.save,bank),new Uint8Array(await file.arrayBuffer()));
    await store.flush();status('imported');
  }catch(error){fail(error);}
});
$('play').addEventListener('click',()=>{
  if(!core||started)return;
  $('setup').hidden=true;$('game').hidden=false;canvasSize();started=true;$('canvas').focus();status('');
  try{core.callMain(['--data-dir','/saves','--rom',':browser:','--language',$('language').value]);scheduleResize();}catch(error){fail(error);}
});
$('touch').addEventListener('change',()=>{core?._PortWebTouchEnable($('touch').checked?1:0);$('canvas').focus();});
async function toggleFullscreen(){
  try{if(document.fullscreenElement)await document.exitFullscreen();else await $('canvas').requestFullscreen();}
  catch(error){fail(error);}
}
$('fullscreen').addEventListener('click',toggleFullscreen);
$('pause').addEventListener('click',()=>{paused=!paused;$('pause').textContent=t(paused?'resume':'pause');visibility();if(!paused)$('canvas').focus();});
$('export').addEventListener('click',async()=>{
  try{
    await flushSave();const name=ReKHWeb.saveName(region.save,core._PortWebActiveBank());
    const bytes=core.FS.readFile('/saves/'+name);
    const url=URL.createObjectURL(new Blob([bytes],{type:'application/octet-stream'}));
    const link=document.createElement('a');link.href=url;link.download=name;link.click();setTimeout(()=>URL.revokeObjectURL(url),1000);
  }catch(error){status('noSave');log(error);}
});
$('interface-language').addEventListener('change',()=>{translateWeb($('interface-language').value);$('pause').textContent=t(paused?'resume':'pause');if(warningKeys.size)warning(Array.from(warningKeys)[0]);scheduleResize();if(!core)status('choose');});
document.addEventListener('fullscreenchange',()=>{scheduleResize();if(started)$('canvas').focus();});
window.addEventListener('resize',scheduleResize);
window.visualViewport?.addEventListener('resize',scheduleResize);
new ResizeObserver(scheduleResize).observe($('canvas'));
document.addEventListener('visibilitychange',visibility);
window.addEventListener('pagehide',()=>{if(started){core._PortWebFlushSave();store.flush();}});
window.addEventListener('beforeunload',event=>{if(pendingSave||store?.pending){event.preventDefault();event.returnValue=t('leave');}});
$('canvas').addEventListener('keydown',event=>{
  if(event.key==='Enter'&&event.altKey){event.preventDefault();event.stopPropagation();toggleFullscreen();return;}
  if(['ArrowUp','ArrowDown','ArrowLeft','ArrowRight',' ','Backspace'].includes(event.key))event.preventDefault();
});
$('canvas').addEventListener('contextmenu',event=>event.preventDefault());
translateWeb('en');status('choose');
