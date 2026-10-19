'use strict';
const achievementArt=['Kingdom Key','Three Wishes','Oathkeeper','Ultima Weapon','Cure','Mega Potion','Bambi','Bambi','Cloud','Spellbinder','Lionheart','Oblivion','Soul Eater','Crabclaw','Fairy Harp','One Winged Angel'];
function readLocalAchievements(text){
  const lines=String(text).trim().split(/\r?\n/);
  if(lines.length!==18||lines[0]!=='KHLOCAL1'||!/^[0-9a-f]{1,4}$/i.test(lines[1])||lines.slice(2).some(s=>!/^\d{1,20}$/.test(s)||BigInt(s)>0xffffffffffffffffn))return {mask:0,when:[]};
  return {mask:parseInt(lines[1],16),when:lines.slice(2).map(Number)};
}
function achievementProgress(){
  try{return readLocalAchievements(core.FS.readFile('/saves/achievements_'+region.id+'.dat',{encoding:'utf8'}));}
  catch{return {mask:0,when:[]};}
}
function drawAchievementArt(canvas,id){
  const ctx=canvas.getContext('2d');ctx.clearRect(0,0,40,40);
  if(!core||!region)return;
  const r={eu:0,us:1,jp:2}[region.id],card=ReKHCards.cards.find(c=>ReKHCards.kinds[c[0]]===achievementArt[id]);
  const sprite=card&&ReKHCards.sprites[card[5+r]];if(!sprite)return;
  const tiles=0x08000000+sprite[0][r],palette=0x08000000+sprite[1][r],rom=core.HEAPU8,image=ctx.createImageData(40,40);
  if(tiles+256>rom.length||palette+32>rom.length)return;
  for(let t=0;t<8;t++)for(let y=0;y<8;y++)for(let x=0;x<8;x++){
    const b=rom[tiles+t*32+y*4+(x>>1)],c=x&1?b>>4:b&15;if(!c)continue;
    const colour=rom[palette+c*2]|rom[palette+c*2+1]<<8,p=((4+(t>>1)*8+y)*40+12+(t%2)*8+x)*4;
    image.data[p]=(colour&31)*255/31;image.data[p+1]=((colour>>5)&31)*255/31;image.data[p+2]=((colour>>10)&31)*255/31;image.data[p+3]=255;
  }ctx.putImageData(image,0,0);
}
function renderAchievements(){
  const data=achievementProgress(),grid=$('achievement-grid'),es=interfaceLanguage==='es';grid.replaceChildren();
  const earned=Array.from({length:16},(_,id)=>!!(data.mask&(1<<id))).filter(Boolean).length;
  $('achievement-progress').textContent=earned+' / 16 · '+(region?region.id.toUpperCase():optionWord('Choose a ROM','Selecciona una ROM'));
  $('achievement-meter').value=earned;
  $('achievement-local-enabled').disabled=!core||started;
  if(core)try{$('achievement-local-enabled').checked=!/^enabled=0\s*$/m.test(core.FS.readFile('/saves/achievements_local.ini',{encoding:'utf8'}));}catch{$('achievement-local-enabled').checked=true;}
  const query=$('achievement-search').value.trim().toLocaleLowerCase(),filter=$('achievement-filter').value;
  ReKHLocalAchievements.forEach((def,id)=>{
    const unlocked=!!(data.mask&(1<<id)),title=def[es?0:1],description=def[es?2:3];
    if(filter==='unlocked'&&!unlocked||filter==='locked'&&unlocked||query&&!(title+' '+description).toLocaleLowerCase().includes(query))return;
    const row=document.createElement('article');row.className='achievement-row'+(unlocked?' unlocked':'');
    const badge=document.createElement('canvas');badge.width=badge.height=40;badge.className='achievement-badge';badge.setAttribute('aria-hidden','true');drawAchievementArt(badge,id);
    const text=document.createElement('div'),heading=document.createElement('h2'),detail=document.createElement('p'),state=document.createElement('small');heading.textContent=title;detail.textContent=description;state.textContent=unlocked?optionWord('Unlocked','Conseguido'):optionWord('Locked','Pendiente');
    text.append(heading,detail,state);row.append(badge,text);grid.append(row);
  });
  if(!grid.children.length){const empty=document.createElement('p');empty.textContent=optionWord('No achievements match this filter.','No hay logros con ese filtro.');grid.append(empty);}
}
$('achievement-local-enabled').addEventListener('change',async()=>{
  if(!core||started)return;
  core.FS.writeFile('/saves/achievements_local.ini','enabled='+Number($('achievement-local-enabled').checked)+'\n');await store.flush();
});
$('achievement-search').addEventListener('input',renderAchievements);
$('achievement-filter').addEventListener('change',renderAchievements);
$('tab-achievements').addEventListener('click',renderAchievements);
$('interface-language').addEventListener('change',renderAchievements);
window.ReKHAchievements={render:renderAchievements,read:readLocalAchievements};
renderAchievements();
