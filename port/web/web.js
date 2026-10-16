'use strict';
const $ = id => document.getElementById(id);
const regions = {
  B8CP: {id:'eu', sha:'8db73586cdb11b3795907edebf43228dbcd3e6b2', save:'kh_com.sav'},
  B8CE: {id:'us', sha:'10729bd884f8fdca7a310b6d606c52e46657aa48', save:'kh_com_us.sav'},
  B8CJ: {id:'jp', sha:'59ec0a0a4ccd1e6acb3bbd7bfb21d63988958cfa', save:'kh_com_jp.sav'}
};
let core, region, started=false, loading=false;
const lines=[];
function log(message){console.log(message);lines.push(String(message));if(lines.length>60)lines.shift();$('log').textContent=lines.join('\n');}
function status(message){$('status').textContent=message;}
function fail(error){log(error.stack || error);status('No se pudo iniciar: '+error.message);}
function sync(populate=false){return new Promise((resolve,reject)=>core.FS.syncfs(populate,error=>error?reject(error):resolve()));}
function canvasSize(){
  const width=Math.max(240,Math.min(1920,Math.round($('canvas').getBoundingClientRect().width || Math.min(innerWidth-28,1052))));
  // Use CSS pixels, not devicePixelRatio: a phone's high DPI must not multiply
  // the software renderer's workload. Preserve 16:9 in the embedded player.
  $('canvas').width=width;$('canvas').height=Math.max(160,Math.round(width*9/16));
}
$('rom').addEventListener('change',async()=>{
  if(loading || core)return;
  const file=$('rom').files[0];if(!file)return;
  loading=true;$('rom').disabled=true;
  try{
    if(file.size!==33554432)throw Error('La ROM debe ocupar 32 MiB.');
    status('Comprobando ROM…');
    let buffer=await file.arrayBuffer();
    const code=new TextDecoder().decode(new Uint8Array(buffer,0xac,4));
    region=regions[code];if(!region)throw Error('Esta región no está admitida.');
    const hash=Array.from(new Uint8Array(await crypto.subtle.digest('SHA-1',buffer)),n=>n.toString(16).padStart(2,'0')).join('');
    if(hash!==region.sha)throw Error('La ROM está modificada o no coincide con la versión compatible.');
    status('Cargando motor…');
    await new Promise((resolve,reject)=>{const script=document.createElement('script');script.src=`rekhcom-${region.id}.js`;script.onload=resolve;script.onerror=()=>reject(Error('No se pudo descargar el motor de esta región.'));document.head.append(script);});
    canvasSize();
    core=await createReKHCoM({canvas:$('canvas'),noInitialRun:true,print:log,printErr:log,onAbort:message=>status('El motor se detuvo: '+message)});
    core.HEAPU8.set(new Uint8Array(buffer),0x08000000);buffer=null;
    if(!core._PortWebRomReady(file.size))throw Error('El motor rechazó la ROM.');
    core.FS.mkdir('/saves');core.FS.mount(core.IDBFS,{},'/saves');
    try{await sync(true);}catch(error){log(error);status('Almacenamiento persistente no disponible: exporta tu partida antes de salir.');}
    let timer, syncing=false, again=false;
    core.persistSoon=()=>{clearTimeout(timer);timer=setTimeout(async()=>{
      if(syncing){again=true;return;}syncing=true;
      try{await sync();}catch(error){log(error);status('No se pudo guardar en el navegador. Exporta una copia de tu partida.');}
      finally{syncing=false;if(again){again=false;core.persistSoon();}}
    },250);};
    $('touch').checked=matchMedia('(pointer:coarse)').matches;
    core._PortWebTouchEnable($('touch').checked?1:0);
    $('play').disabled=false;$('save').disabled=false;
    status('ROM lista. Puedes importar una partida o pulsar Jugar.');
  }catch(error){fail(error);core=null;region=null;$('rom').disabled=false;$('rom').value='';}
  finally{loading=false;}
});
$('save').addEventListener('change',async()=>{
  try{
    if(started)throw Error('Importa la partida antes de iniciar el juego.');
    const file=$('save').files[0];if(!file)return;
    if(file.size!==65536)throw Error('Se necesita una partida SRAM de 64 KiB (.sav), no un guardado rápido.');
    core.FS.writeFile('/saves/'+region.save,new Uint8Array(await file.arrayBuffer()));await sync();status('Partida importada. Pulsa Jugar.');
  }catch(error){fail(error);}
});
$('play').addEventListener('click',()=>{
  if(!core || started)return;
  started=true;$('setup').hidden=true;$('game').hidden=false;canvasSize();$('canvas').focus();
  status('');
  try{core.callMain(['--data-dir','/saves','--rom',':browser:','--language',$('language').value]);}catch(error){fail(error);}
});
$('touch').addEventListener('change',()=>core?._PortWebTouchEnable($('touch').checked?1:0));
$('fullscreen').addEventListener('click',()=>{$('canvas').requestFullscreen?.().catch(fail);});
$('export').addEventListener('click',()=>{
  try{
    const bytes=core.FS.readFile('/saves/'+region.save);
    const url=URL.createObjectURL(new Blob([bytes],{type:'application/octet-stream'}));
    const link=document.createElement('a');link.href=url;link.download=region.save;link.click();setTimeout(()=>URL.revokeObjectURL(url),1000);
  }catch(error){status('Todavía no hay una partida guardada. Guarda primero desde el juego.');}
});
document.addEventListener('visibilitychange',()=>{if(core){core._PortWebSetVisible(document.hidden?0:1);if(document.hidden)core.persistSoon?.();}});
window.addEventListener('pagehide',()=>core?.persistSoon?.());
// Prevent page scrolling only while keyboard focus belongs to the game.
$('canvas').addEventListener('keydown',event=>{if(['ArrowUp','ArrowDown','ArrowLeft','ArrowRight',' ','Tab','Backspace'].includes(event.key))event.preventDefault();});
