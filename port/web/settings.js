'use strict';
/* One settings model, shared with kh_com.ini and PortWebOption. */
const optionWord=(en,es)=>interfaceLanguage==='es'?es:en;
const optionLabel=(en,es)=>{const s=document.createElement('span');s.dataset.en=en;s.dataset.es=es;s.textContent=optionWord(en,es);return s;};
const optionSpecs=[
  ['graphics','Picture','Imagen','view_mode','View','Vista',4,[[0,'Original GBA','GBA original'],[1,'Widescreen','Panorámica'],[2,'Enhanced','Ampliada'],[3,'Maximum','Máxima'],[4,'Auto','Automática']]],
  ['graphics','Picture','Imagen','zoom','Pixel zoom','Zoom de píxeles',8,[[4,'×2','×2'],[6,'×3','×3'],[8,'×4','×4'],[10,'×5','×5'],[12,'×6','×6'],[14,'×7','×7'],[16,'×8','×8']]],
  ['graphics','Picture','Imagen','aspect','Aspect ratio','Proporción',0,[[0,'Fill window','Toda la ventana'],[1,'Up to 16:9','Hasta 16:9'],[2,'Up to 21:9','Hasta 21:9'],[3,'21:9','21:9'],[4,'32:9','32:9'],[5,'4:3','4:3'],[6,'GBA 3:2','GBA 3:2']]],
  ['graphics','Picture','Imagen','scale_mode','Scaling','Escalado',0,[[0,'Integer','Entero'],[1,'Fit','Ajustado'],[2,'Stretch','Estirado']]],
  ['graphics','Picture','Imagen','filter','Filter','Filtro',0,[[0,'Nearest pixels','Píxeles nítidos'],[1,'Sharp','Nítido adaptable'],[2,'Linear','Suave']]],
  ['graphics','Text & interface','Texto e interfaz','narrow_fit','Fill height in menus','Llenar altura en menús',1,'check'],
  ['graphics','Text & interface','Texto e interfaz','hd_text','HD text','Texto HD',0,'check'],
  ['graphics','Text & interface','Texto e interfaz','hud_size','HUD size','Tamaño del HUD',1,[[0,'Small','Pequeño'],[1,'Normal','Normal'],[2,'Large','Grande']]],
  ['graphics','Text & interface','Texto e interfaz','card_hud_raise','Raise card hand','Elevar mano de cartas',8,[0,16]],
  ['graphics','Text & interface','Texto e interfaz','scanlines','Scanlines','Líneas de pantalla',0,[0,100]],
  ['graphics','Text & interface','Texto e interfaz','color_correction','GBA colours','Colores GBA',0,'check'],
  ['graphics','Text & interface','Texto e interfaz','show_fps','Show FPS','Mostrar FPS',0,'check'],
  ['sound','Volume','Volumen','master_volume','Master','General',100,[0,100]],
  ['sound','Volume','Volumen','music_volume','Music','Música',100,[0,100]],
  ['sound','Volume','Volumen','sfx_volume','Sound effects','Efectos',100,[0,100]],
  ['sound','Extras','Extras','undub','Japanese voices','Voces japonesas',0,'check','startup'],
  ['sound','Extras','Extras','smooth_audio','Smooth audio (experimental)','Audio suave (experimental)',0,'check'],
  ['improvements','Battle','Combate','dodge_button','Dedicated dodge button','Botón para esquivar',1,'check'],
  ['improvements','Battle','Combate','stock_button','Dedicated stock button','Botón para acumular',1,'check'],
  ['improvements','Battle','Combate','cheat_one_hit','Defeat enemies in one hit','Derrotar enemigos de un golpe',0,'check'],
  ['improvements','Battle','Combate','cheat_infinite_hp','Infinite HP','HP infinito',0,'check'],
  ['improvements','Extras','Extras','cheat_unlock_riku','Riku on title screen','Riku en el título',0,'check'],
  ['improvements','Extras','Extras','save_bank','Save bank','Banco de partidas',1,[[1,'1','1'],[2,'2','2'],[3,'3','3'],[4,'4','4'],[5,'5','5']],'startup'],
  ['improvements','Experimental','Experimental','experimental_save_point_slot_select','Choose slot at save points','Elegir ranura en cristales',0,'check'],
  ['improvements','Experimental','Experimental','experimental_moogle_notice','Moogle points notice','Aviso de puntos moguri',0,'check'],
  ['improvements','Experimental','Experimental','experimental_unstock','Return stocked cards','Devolver cartas acumuladas',0,'check'],
  ['improvements','Experimental','Experimental','ps2_mode','Re:CoM mode','Modo Re:CoM',0,'check','startup'],
  ['improvements','Experimental','Experimental','ps2_difficulty','Re:CoM difficulty','Dificultad Re:CoM',0,[[0,'Standard','Normal'],[1,'Beginner','Principiante'],[2,'Proud','Experto']],'startup'],
  ['improvements','Experimental','Experimental','ps2_fast_text','Fast dialogue','Diálogos rápidos',0,'check'],
  ['improvements','Experimental','Experimental','experimental_hide_touch_controls','Hide touch panels','Ocultar paneles táctiles',0,'check']
];
const bindingSpecs=[['a','A',120,0],['b','B',122,1],['l','L',97,9],['r','R',115,10],['start','START',13,6],['select','SELECT',8,4],['up','↑',1073741906,11],['down','↓',1073741905,12],['left','←',1073741904,13],['right','→',1073741903,14],['stock','Stock / Acumular',100,3],['dodge','Dodge / Esquivar',99,2],['unstock','Return / Devolver',118,8]];
const padChoices=[[0,'A'],[1,'B'],[2,'X'],[3,'Y'],[9,'LB'],[10,'RB'],[108,'LT'],[110,'RT'],[6,'START'],[4,'BACK'],[7,'LS'],[8,'RS'],[11,'D-pad ↑'],[12,'D-pad ↓'],[13,'D-pad ←'],[14,'D-pad →'],[101,'LS ←'],[100,'LS →'],[103,'LS ↑'],[102,'LS ↓'],[-1,'—']];
const preferences=Object.fromEntries(optionSpecs.map(s=>[s[3],s[6]]));
for(const [key,,kb,pad]of bindingSpecs){preferences['kb_'+key]=kb;preferences['pad_'+key]=pad;}
const dirtyPreferences=new Set();
try{const saved=JSON.parse(localStorage.getItem('rekhcom-options')||'{}');for(const k in preferences)if(Number.isInteger(saved[k]))preferences[k]=saved[k];}catch{}
let baseIni='',listeningKey=null;
const optionInputs=new Map();
function preferenceNotice(en,es){$('preferences-status').textContent=optionWord(en,es);}
function keyName(v){const names={0:'—',13:'Enter',8:'Backspace',32:'Space',1073741906:'↑',1073741905:'↓',1073741904:'←',1073741903:'→',1073742053:'R Shift'};return names[v]||(v>=32&&v<127?String.fromCharCode(v).toUpperCase():String(v));}
function rememberPreferences(){try{localStorage.setItem('rekhcom-options',JSON.stringify(preferences));}catch{preferenceNotice('Browser settings storage is unavailable.','No se pueden conservar las opciones del navegador.');}}
function mergeIni(){let text=baseIni||'config_version=11\n';for(const[k,v]of Object.entries(preferences)){const line=new RegExp('^'+k+'=.*$','m');text=line.test(text)?text.replace(line,k+'='+v):text+'\n'+k+'='+v;}return text+'\n';}
function changePreference(key,value){
  preferences[key]=Number(value);dirtyPreferences.add(key);rememberPreferences();
  if(started){
    if(!core.ccall||!core._PortWebOption){preferenceNotice('Reload the updated engine to change options.','Recarga el motor actualizado para cambiar opciones.');return;}
    preferences[key]=core.ccall('PortWebOption','number',['string','number','number'],[key,value,1]);
    if(key==='experimental_hide_touch_controls'){$('touch').checked=!value;core._PortWebTouchEnable(value?0:1);}
    core._PortWebResize($('canvas').width,$('canvas').height);store.request();
  }else if(core){core.FS.writeFile('/saves/kh_com.ini',mergeIni());store.request();}
  preferenceNotice('Settings saved.','Opciones guardadas.');
}
function addOption(spec){
  const[tab,enGroup,esGroup,key,en,es,initial,kind,locked]=spec;
  const groupId=tab+'-'+enGroup.replace(/\W/g,'');let group=$(groupId);
  if(!group){group=document.createElement('div');group.className='option-group';group.id=groupId;const heading=document.createElement('h2');heading.append(optionLabel(enGroup,esGroup));group.append(heading);$('options-'+tab).append(group);}
  const label=document.createElement('label');label.className='option-row';label.append(optionLabel(en,es));let input;
  if(Array.isArray(kind)&&Array.isArray(kind[0])){input=document.createElement('select');for(const[v,a,b]of kind){const option=document.createElement('option');option.value=v;option.dataset.en=a;option.dataset.es=b;option.textContent=optionWord(a,b);input.append(option);}input.value=preferences[key];}
  else{input=document.createElement('input');input.type=kind==='check'?'checkbox':'range';if(input.type==='range'){input.min=kind[0];input.max=kind[1];input.value=preferences[key];}else input.checked=!!preferences[key];}
  input.id='option-'+key;input.dataset.key=key;if(locked)input.dataset.startup='1';optionInputs.set(key,input);
  if(input.type==='range'){const wrap=document.createElement('div');wrap.className='volume-control';const out=document.createElement('output');out.htmlFor=input.id;out.textContent=input.value;wrap.append(input,out);label.append(wrap);input.addEventListener('input',()=>out.textContent=input.value);}else label.append(input);
  input.addEventListener('change',()=>changePreference(key,input.type==='checkbox'?+input.checked:Number(input.value)));group.append(label);
  if(locked){const note=document.createElement('p');note.className='option-note';note.append(optionLabel('Choose before starting the game.','Se elige antes de iniciar el juego.'));group.append(note);}
}
for(const spec of optionSpecs)addOption(spec);
for(const[key,label,kb,pad]of bindingSpecs){
  const row=document.createElement('div');row.className='option-row binding-row';const name=document.createElement('span');name.textContent=label;const button=document.createElement('button');button.textContent=keyName(preferences['kb_'+key]);button.setAttribute('aria-label',label+' keyboard');const select=document.createElement('select');select.setAttribute('aria-label',label+' gamepad');
  for(const[v,text]of padChoices){const o=document.createElement('option');o.value=v;o.textContent=text;select.append(o);}select.value=preferences['pad_'+key];
  button.addEventListener('click',()=>{if(listeningKey)listeningKey.button.classList.remove('listening');listeningKey={key,button};button.classList.add('listening');button.textContent='…';preferenceNotice('Press a key; Escape cancels, Delete clears.','Pulsa una tecla; Escape cancela, Suprimir borra.');});
  select.addEventListener('change',()=>changePreference('pad_'+key,Number(select.value)));optionInputs.set('kb_'+key,button);optionInputs.set('pad_'+key,select);row.append(name,button,select);$('options-controls').append(row);
}
document.addEventListener('keydown',event=>{
  if(!listeningKey)return;event.preventDefault();event.stopImmediatePropagation();
  const arrows={ArrowUp:1073741906,ArrowDown:1073741905,ArrowLeft:1073741904,ArrowRight:1073741903,Enter:13,Backspace:8,Tab:9,' ':32,Shift:1073742049,Control:1073742048,Alt:1073742050};
  if(event.key!=='Escape'){const value=event.key==='Delete'?0:arrows[event.key]??(event.key.length===1?event.key.toLowerCase().charCodeAt(0):null);if(value===null)return;changePreference('kb_'+listeningKey.key,value);}
  listeningKey.button.textContent=keyName(preferences['kb_'+listeningKey.key]);listeningKey.button.classList.remove('listening');listeningKey=null;
},true);
window.ReKHOptions={paused:false,
  load(ini){baseIni=ini||'';for(const match of baseIni.matchAll(/^([a-z0-9_]+)=(-?\d+)$/gm))if(match[1]in preferences&&!dirtyPreferences.has(match[1]))preferences[match[1]]=Number(match[2]);this.refresh();},
  refresh(){for(const[k,input]of optionInputs){if(k.startsWith('kb_'))input.textContent=keyName(preferences[k]);else if(input.type==='checkbox')input.checked=!!preferences[k];else input.value=preferences[k];const out=input.parentElement.querySelector('output');if(out)out.textContent=input.value;input.disabled=started&&!!input.dataset.startup;}},
  beforeStart(){if(core){core.FS.writeFile('/saves/kh_com.ini',mergeIni());store.request();}},
  translate(){document.querySelectorAll('[data-en]').forEach(el=>{el.textContent=el.dataset[interfaceLanguage]||el.dataset.en;});}
};
function selectOptionsTab(id){
  for(const tab of document.querySelectorAll('[role=tab]')){const selected=tab.dataset.panel===id;tab.setAttribute('aria-selected',String(selected));tab.tabIndex=selected?0:-1;$(tab.getAttribute('aria-controls')).hidden=!selected;}
  ReKHOptions.paused=id!=='game';clearInput();visibility();scheduleResize();
  if(id==='achievements')window.ReKHAchievements?.render();
}
const tabs=Array.from(document.querySelectorAll('[role=tab]'));
tabs.forEach((tab,index)=>{tab.addEventListener('click',()=>selectOptionsTab(tab.dataset.panel));tab.addEventListener('keydown',event=>{let n;if(event.key==='ArrowRight')n=(index+1)%tabs.length;if(event.key==='ArrowLeft')n=(index+tabs.length-1)%tabs.length;if(event.key==='Home')n=0;if(event.key==='End')n=tabs.length-1;if(n!==undefined){event.preventDefault();tabs[n].focus();selectOptionsTab(tabs[n].dataset.panel);}});});
$('interface-language').addEventListener('change',()=>ReKHOptions.translate());
ReKHOptions.translate();
