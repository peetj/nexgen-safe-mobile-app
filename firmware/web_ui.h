#ifndef WEB_UI_H
#define WEB_UI_H

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Nexgen Safe</title>
  <style>
    :root{--orange:#ff5e19;--charcoal:#15181e;--offwhite:#f2f2f0;--green:#34d399;--red:#fb7185;--amber:#fbbf24}
    *{box-sizing:border-box}
    body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial,sans-serif;background:var(--charcoal);color:var(--offwhite)}
    header{padding:16px;border-bottom:1px solid rgba(242,242,240,.1)}
    h1{margin:0;font-size:18px}
    .wrap{max-width:520px;margin:0 auto;padding:16px}
    .card{background:rgba(255,255,255,.04);border:1px solid rgba(242,242,240,.08);border-radius:16px;padding:16px;margin-bottom:12px}
    .row{display:flex;gap:10px}
    .status{display:flex;align-items:center;gap:8px}
    .pill{width:10px;height:10px;border-radius:50%;background:var(--amber)}
    .pill.locked{background:var(--red)}
    .pill.unlocked{background:var(--green)}
    .hint{color:rgba(242,242,240,.72);font-size:12px;line-height:1.4}
    .msg{min-height:18px;margin:10px 0 0}
    button{appearance:none;border:0;border-radius:14px;padding:14px;font-weight:700;cursor:pointer}
    button.primary{background:var(--orange);color:#111}
    button.tonal{background:rgba(255,94,25,.15);color:var(--offwhite);border:1px solid rgba(255,94,25,.35)}
    button.ghost{background:transparent;color:var(--offwhite);border:1px solid rgba(242,242,240,.18)}
    .dots{display:flex;justify-content:center;gap:18px;padding:10px 0 16px}
    .dot{width:14px;height:14px;border-radius:50%;background:rgba(242,242,240,.18)}
    .dot.filled{background:var(--offwhite)}
    .grid{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}
    .key{background:rgba(242,242,240,.06);border:1px solid rgba(242,242,240,.1);color:var(--offwhite);font-size:20px}
    input{width:100%;padding:12px;border-radius:12px;border:1px solid rgba(242,242,240,.14);background:rgba(0,0,0,.2);color:var(--offwhite)}
  </style>
</head>
<body>
  <header><h1>Nexgen Safe</h1></header>
  <div class="wrap">
    <div class="card">
      <div class="status">
        <div id="pill" class="pill"></div>
        <div><b id="state">UNKNOWN</b> <span id="name" style="opacity:.72"></span></div>
        <div style="margin-left:auto;opacity:.72">192.168.4.1</div>
      </div>
      <p class="hint" style="margin:10px 0 0">Join the safe Wi-Fi hotspot, then use this page in your browser.</p>
      <p id="msg" class="hint msg"></p>
    </div>
    <div class="card">
      <div class="row">
        <button id="lockBtn" class="primary" style="flex:1">Lock</button>
        <button id="unlockBtn" class="tonal" style="flex:1">Unlock</button>
      </div>
      <div style="height:10px"></div>
      <button id="setPinBtn" class="ghost" style="width:100%">Set PIN</button>
      <div style="height:16px"></div>
      <div class="hint" id="prompt">Choose an action</div>
      <div class="dots" id="dots"></div>
      <div class="grid" id="grid"></div>
      <div style="height:10px"></div>
      <button id="cancelBtn" class="ghost" style="width:100%">Cancel</button>
    </div>
    <div class="card">
      <div class="hint" style="margin-bottom:8px">LCD (optional)</div>
      <input id="l1" placeholder="Line 1" value="Nexgen Safe">
      <div style="height:10px"></div>
      <input id="l2" placeholder="Line 2" value="Use the browser">
      <div style="height:10px"></div>
      <button id="lcdBtn" class="ghost" style="width:100%">Write to LCD</button>
    </div>
  </div>
  <script>
    const stateEl=document.getElementById('state'),nameEl=document.getElementById('name'),pillEl=document.getElementById('pill'),msgEl=document.getElementById('msg'),promptEl=document.getElementById('prompt'),dotsEl=document.getElementById('dots'),gridEl=document.getElementById('grid');
    let action='none',pin='',pin1='';
    function setMessage(t){msgEl.textContent=t||''}
    function renderDots(){dotsEl.innerHTML='';for(let i=0;i<4;i++){const d=document.createElement('div');d.className='dot'+(i<pin.length?' filled':'');dotsEl.appendChild(d)}}
    function setPrompt(){if(action==='none')promptEl.textContent='Choose an action';if(action==='lock')promptEl.textContent='Enter PIN to LOCK';if(action==='unlock')promptEl.textContent='Enter PIN to UNLOCK';if(action==='set1')promptEl.textContent='Enter NEW PIN';if(action==='set2')promptEl.textContent='Confirm NEW PIN'}
    function resetFlow(){action='none';pin='';pin1='';setPrompt();renderDots()}
    async function api(path,body){const res=await fetch(path,{method:body?'POST':'GET',headers:body?{'Content-Type':'application/json'}:undefined,body:body?JSON.stringify(body):undefined});const json=await res.json().catch(()=>({ok:false,err:'BAD_JSON'}));if(!res.ok||json.ok===false)throw new Error(json.err||('HTTP_'+res.status));return json}
    async function refreshStatus(){try{const json=await api('/status');nameEl.textContent=json.name?'('+json.name+')':'';if(json.locked===true){stateEl.textContent='LOCKED';pillEl.className='pill locked'}else if(json.locked===false){stateEl.textContent='UNLOCKED';pillEl.className='pill unlocked'}else{stateEl.textContent='UNKNOWN';pillEl.className='pill'}}catch(_){stateEl.textContent='OFFLINE';pillEl.className='pill'}}
    function addKey(label,onClick){const b=document.createElement('button');b.className='key';b.textContent=label;b.onclick=onClick;gridEl.appendChild(b)}
    function buildKeypad(){gridEl.innerHTML='';['1','2','3','4','5','6','7','8','9'].forEach((d)=>addKey(d,()=>digit(d)));addKey('Del',back);addKey('0',()=>digit('0'));addKey('Go',submit)}
    function digit(d){if(action==='none'||pin.length>=4)return;pin+=d;renderDots()}
    function back(){if(!pin.length)return;pin=pin.slice(0,-1);renderDots()}
    async function submit(){if(pin.length!==4)return;try{if(action==='lock'){await api('/lock',{pin});setMessage('Safe locked')}if(action==='unlock'){await api('/unlock',{pin});setMessage('Safe unlocked')}if(action==='set1'){pin1=pin;pin='';action='set2';setPrompt();renderDots();return}if(action==='set2'){await api('/setpin',{pin1,pin2:pin});setMessage('PIN updated')}resetFlow();await refreshStatus()}catch(err){setMessage(String(err.message||err))}}
    document.getElementById('lockBtn').onclick=()=>{action='lock';pin='';setPrompt();renderDots();setMessage('')};
    document.getElementById('unlockBtn').onclick=()=>{action='unlock';pin='';setPrompt();renderDots();setMessage('')};
    document.getElementById('setPinBtn').onclick=()=>{action='set1';pin='';pin1='';setPrompt();renderDots();setMessage('')};
    document.getElementById('cancelBtn').onclick=()=>{resetFlow();setMessage('')};
    document.getElementById('lcdBtn').onclick=async()=>{try{await api('/lcd',{line1:document.getElementById('l1').value,line2:document.getElementById('l2').value});setMessage('LCD updated')}catch(err){setMessage(String(err.message||err))}};
    buildKeypad();resetFlow();refreshStatus();setInterval(refreshStatus,2000);
  </script>
</body>
</html>
)rawliteral";

#endif
