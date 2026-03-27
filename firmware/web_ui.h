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
    :root{
      --orange:#ff5e19;
      --orange-soft:#ff8a47;
      --charcoal:#15181e;
      --offwhite:#f2f2f0;
      --green:#34d399;
      --red:#fb7185;
      --amber:#fbbf24;
      --cyan:#5eead4;
    }
    *{box-sizing:border-box}
    body{
      margin:0;
      min-height:100vh;
      font-family:"Segoe UI",Tahoma,Geneva,Verdana,sans-serif;
      color:var(--offwhite);
      background:
        radial-gradient(circle at top left, rgba(255,94,25,.22), transparent 30%),
        radial-gradient(circle at top right, rgba(94,234,212,.14), transparent 28%),
        linear-gradient(180deg, #15181e 0%, #0b0d12 100%);
    }
    header{
      padding:22px 18px 14px;
      border-bottom:1px solid rgba(242,242,240,.08);
      background:linear-gradient(180deg, rgba(255,255,255,.05), rgba(255,255,255,0));
    }
    .brand-tag{
      font-size:11px;
      letter-spacing:.36em;
      text-transform:uppercase;
      color:rgba(242,242,240,.55);
      margin-bottom:8px;
    }
    h1{
      margin:0;
      font-size:32px;
      line-height:1;
      letter-spacing:.08em;
      text-transform:uppercase;
      font-family:"Arial Black","Trebuchet MS",sans-serif;
    }
    .brand-hot{
      color:var(--orange-soft);
      text-shadow:0 0 22px rgba(255,94,25,.28);
    }
    .brand-cool{
      background:linear-gradient(135deg, #ffe27a 0%, #5eead4 100%);
      -webkit-background-clip:text;
      background-clip:text;
      color:transparent;
    }
    .wrap{
      max-width:560px;
      margin:0 auto;
      padding:18px 16px 28px;
    }
    .card{
      background:linear-gradient(180deg, rgba(255,255,255,.06), rgba(255,255,255,.03));
      border:1px solid rgba(242,242,240,.08);
      border-radius:22px;
      padding:16px;
      margin-bottom:14px;
      box-shadow:0 18px 40px rgba(0,0,0,.24);
    }
    .status-card{
      position:relative;
      overflow:hidden;
    }
    .status-card::after{
      content:"";
      position:absolute;
      right:-30px;
      bottom:-52px;
      width:180px;
      height:180px;
      border-radius:50%;
      background:radial-gradient(circle, var(--glow, rgba(251,191,36,.22)) 0%, rgba(0,0,0,0) 72%);
      pointer-events:none;
    }
    .status-card.locked{
      --glow:rgba(251,113,133,.28);
      border-color:rgba(251,113,133,.34);
    }
    .status-card.unlocked{
      --glow:rgba(52,211,153,.24);
      border-color:rgba(52,211,153,.34);
    }
    .status-card.offline{
      --glow:rgba(251,191,36,.24);
      border-color:rgba(251,191,36,.3);
    }
    .topline{
      display:flex;
      align-items:center;
      justify-content:space-between;
      gap:12px;
      margin-bottom:14px;
    }
    .device-name{
      font-size:13px;
      letter-spacing:.14em;
      text-transform:uppercase;
      color:rgba(242,242,240,.72);
    }
    .ip{
      font-size:13px;
      color:rgba(242,242,240,.68);
      white-space:nowrap;
    }
    .state-badge{
      display:inline-flex;
      align-items:center;
      gap:10px;
      min-height:78px;
      padding:18px 22px;
      border-radius:20px;
      font-size:30px;
      font-weight:900;
      letter-spacing:.16em;
      text-transform:uppercase;
      border:1px solid rgba(242,242,240,.1);
      background:rgba(0,0,0,.22);
      box-shadow:inset 0 1px 0 rgba(255,255,255,.04);
    }
    .state-badge::before{
      content:"";
      width:16px;
      height:16px;
      border-radius:50%;
      background:currentColor;
      box-shadow:0 0 18px currentColor;
      flex:0 0 auto;
    }
    .status-card.locked .state-badge{
      color:var(--red);
      border-color:rgba(251,113,133,.3);
      background:linear-gradient(180deg, rgba(251,113,133,.12), rgba(0,0,0,.18));
    }
    .status-card.unlocked .state-badge{
      color:var(--green);
      border-color:rgba(52,211,153,.28);
      background:linear-gradient(180deg, rgba(52,211,153,.12), rgba(0,0,0,.18));
    }
    .status-card.offline .state-badge{
      color:var(--amber);
      border-color:rgba(251,191,36,.28);
      background:linear-gradient(180deg, rgba(251,191,36,.12), rgba(0,0,0,.18));
    }
    .msg{
      min-height:18px;
      margin:12px 0 0;
      font-size:13px;
      letter-spacing:.04em;
      color:rgba(242,242,240,.82);
    }
    .row{
      display:flex;
      gap:10px;
    }
    button{
      appearance:none;
      border:0;
      border-radius:16px;
      padding:14px;
      font-weight:800;
      letter-spacing:.06em;
      cursor:pointer;
      transition:transform .08s ease, filter .15s ease, border-color .15s ease;
    }
    button:active{transform:translateY(1px) scale(.99)}
    button.primary{
      background:linear-gradient(135deg, var(--orange) 0%, #ff9d5c 100%);
      color:#111;
      box-shadow:0 10px 26px rgba(255,94,25,.24);
    }
    button.tonal{
      background:linear-gradient(180deg, rgba(94,234,212,.16), rgba(94,234,212,.08));
      color:var(--offwhite);
      border:1px solid rgba(94,234,212,.3);
    }
    button.ghost{
      background:rgba(255,255,255,.03);
      color:var(--offwhite);
      border:1px solid rgba(242,242,240,.14);
    }
    button.small{
      padding:10px 12px;
      font-size:12px;
      letter-spacing:.14em;
      text-transform:uppercase;
    }
    .prompt{
      min-height:18px;
      margin:14px 0 8px;
      text-align:center;
      font-size:12px;
      letter-spacing:.24em;
      text-transform:uppercase;
      color:rgba(242,242,240,.6);
    }
    .prompt:empty{display:none}
    .dots{
      display:flex;
      justify-content:center;
      gap:18px;
      padding:8px 0 16px;
    }
    .dots.hidden{display:none}
    .dot{
      width:14px;
      height:14px;
      border-radius:50%;
      background:rgba(242,242,240,.16);
      border:1px solid rgba(242,242,240,.08);
    }
    .dot.filled{
      background:var(--offwhite);
      box-shadow:0 0 14px rgba(242,242,240,.26);
    }
    .grid{
      display:grid;
      grid-template-columns:repeat(3,1fr);
      gap:10px;
    }
    .key{
      min-height:60px;
      background:rgba(242,242,240,.05);
      border:1px solid rgba(242,242,240,.1);
      color:var(--offwhite);
      font-size:22px;
    }
    .footer-row{
      display:flex;
      gap:10px;
      margin-top:10px;
    }
    .footer-row button{flex:1}
    .modal[hidden]{display:none}
    .modal{
      position:fixed;
      inset:0;
      z-index:20;
      display:flex;
      align-items:flex-end;
      justify-content:center;
      padding:16px;
      background:rgba(3,5,8,.68);
      backdrop-filter:blur(10px);
    }
    .modal-card{
      width:min(100%, 560px);
      background:linear-gradient(180deg, #1b1f27 0%, #11141a 100%);
      border:1px solid rgba(242,242,240,.1);
      border-radius:24px;
      padding:18px;
      box-shadow:0 24px 60px rgba(0,0,0,.38);
    }
    .modal-head{
      display:flex;
      align-items:flex-start;
      justify-content:space-between;
      gap:12px;
      margin-bottom:14px;
    }
    .modal-title{
      margin:0 0 4px;
      font-size:20px;
      font-weight:900;
      letter-spacing:.08em;
      text-transform:uppercase;
    }
    .hint{
      color:rgba(242,242,240,.66);
      font-size:12px;
      line-height:1.5;
    }
    input{
      width:100%;
      padding:14px;
      border-radius:14px;
      border:1px solid rgba(242,242,240,.14);
      background:rgba(0,0,0,.22);
      color:var(--offwhite);
      font-size:15px;
      margin-bottom:10px;
    }
    @media (max-width:460px){
      h1{font-size:28px}
      .state-badge{font-size:25px;min-height:72px;padding:16px 18px}
      .topline{align-items:flex-start;flex-direction:column}
      .ip{margin-top:-6px}
    }
  </style>
</head>
<body>
  <header>
    <div class="brand-tag">Wireless Control</div>
    <h1><span class="brand-hot">Nexgen</span> <span class="brand-cool">Safe</span></h1>
  </header>

  <div class="wrap">
    <div id="statusCard" class="card status-card offline">
      <div class="topline">
        <div class="device-name" id="name">No device</div>
        <div class="ip" id="ip">192.168.4.1</div>
      </div>
      <div id="stateBadge" class="state-badge">Offline</div>
      <p id="msg" class="msg"></p>
    </div>

    <div class="card">
      <div class="row">
        <button id="lockBtn" class="primary" style="flex:1">Lock</button>
        <button id="unlockBtn" class="tonal" style="flex:1">Unlock</button>
      </div>
      <div class="footer-row">
        <button id="setPinBtn" class="ghost">Set PIN</button>
        <button id="settingsBtn" class="ghost">Settings</button>
      </div>
      <div id="prompt" class="prompt"></div>
      <div id="dots" class="dots"></div>
      <div class="grid" id="grid"></div>
      <div class="footer-row">
        <button id="cancelBtn" class="ghost">Cancel</button>
      </div>
    </div>
  </div>

  <div id="settingsModal" class="modal" hidden>
    <div class="modal-card">
      <div class="modal-head">
        <div>
          <div class="modal-title">Settings</div>
          <div class="hint">Write a short message to the safe LCD.</div>
        </div>
        <button id="settingsCloseBtn" class="ghost small">Close</button>
      </div>
      <input id="l1" placeholder="Line 1" value="Nexgen Safe">
      <input id="l2" placeholder="Line 2" value="Use the browser">
      <div class="row">
        <button id="settingsCancelBtn" class="ghost" style="flex:1">Cancel</button>
        <button id="lcdBtn" class="primary" style="flex:1">Save to LCD</button>
      </div>
    </div>
  </div>

  <script>
    const apiBase = location.protocol === 'file:' ? 'http://192.168.4.1' : '';
    const statusCardEl = document.getElementById('statusCard');
    const stateBadgeEl = document.getElementById('stateBadge');
    const nameEl = document.getElementById('name');
    const msgEl = document.getElementById('msg');
    const promptEl = document.getElementById('prompt');
    const dotsEl = document.getElementById('dots');
    const gridEl = document.getElementById('grid');
    const settingsModalEl = document.getElementById('settingsModal');

    let action = 'none';
    let pin = '';
    let pin1 = '';
    let lastLockedState = null;
    let audioCtx = null;

    function getAudioContext() {
      const AudioCtor = window.AudioContext || window.webkitAudioContext;
      if (!AudioCtor) return null;
      if (!audioCtx) audioCtx = new AudioCtor();
      if (audioCtx.state === 'suspended') audioCtx.resume();
      return audioCtx;
    }

    function playTone(freq, duration, type, gain, delaySeconds = 0) {
      const ctx = getAudioContext();
      if (!ctx) return;

      const start = ctx.currentTime + delaySeconds;
      const osc = ctx.createOscillator();
      const amp = ctx.createGain();

      osc.type = type || 'sine';
      osc.frequency.setValueAtTime(freq, start);
      amp.gain.setValueAtTime(gain, start);
      amp.gain.exponentialRampToValueAtTime(0.0001, start + duration);

      osc.connect(amp);
      amp.connect(ctx.destination);
      osc.start(start);
      osc.stop(start + duration);
    }

    function playTap() {
      playTone(620, 0.05, 'triangle', 0.025);
    }

    function playSuccess() {
      playTone(660, 0.08, 'triangle', 0.035);
      playTone(990, 0.12, 'triangle', 0.03, 0.07);
    }

    function playError() {
      playTone(220, 0.12, 'sawtooth', 0.03);
      playTone(160, 0.16, 'sawtooth', 0.025, 0.08);
    }

    function playStateChange(locked) {
      if (locked === true) {
        playTone(280, 0.11, 'square', 0.028);
      } else if (locked === false) {
        playTone(480, 0.08, 'triangle', 0.03);
        playTone(720, 0.12, 'triangle', 0.025, 0.08);
      }
    }

    function setMessage(text) {
      msgEl.textContent = text || '';
    }

    function renderDots() {
      dotsEl.innerHTML = '';
      dotsEl.classList.toggle('hidden', action === 'none');
      for (let i = 0; i < 4; i++) {
        const dot = document.createElement('div');
        dot.className = 'dot' + (i < pin.length ? ' filled' : '');
        dotsEl.appendChild(dot);
      }
    }

    function setPrompt() {
      if (action === 'none') promptEl.textContent = '';
      if (action === 'lock') promptEl.textContent = 'Enter PIN to lock';
      if (action === 'unlock') promptEl.textContent = 'Enter PIN to unlock';
      if (action === 'set1') promptEl.textContent = 'Enter new PIN';
      if (action === 'set2') promptEl.textContent = 'Confirm new PIN';
    }

    function resetFlow() {
      action = 'none';
      pin = '';
      pin1 = '';
      setPrompt();
      renderDots();
    }

    async function api(path, body) {
      const res = await fetch(apiBase + path, {
        method: body ? 'POST' : 'GET',
        headers: body ? { 'Content-Type': 'application/json' } : undefined,
        body: body ? JSON.stringify(body) : undefined
      });

      const json = await res.json().catch(() => ({ ok: false, err: 'BAD_JSON' }));
      if (!res.ok || json.ok === false) {
        throw new Error(json.err || ('HTTP_' + res.status));
      }
      return json;
    }

    function applyStatus(stateText, modeClass) {
      stateBadgeEl.textContent = stateText;
      statusCardEl.className = 'card status-card ' + modeClass;
    }

    async function refreshStatus() {
      try {
        const json = await api('/status');
        nameEl.textContent = json.name || 'Nexgen Safe';

        if (json.locked === true) {
          applyStatus('Locked', 'locked');
        } else if (json.locked === false) {
          applyStatus('Unlocked', 'unlocked');
        } else {
          applyStatus('Unknown', 'offline');
        }

        if (lastLockedState !== null && lastLockedState !== json.locked) {
          playStateChange(json.locked);
        }
        lastLockedState = json.locked;
      } catch (_) {
        applyStatus('Offline', 'offline');
        nameEl.textContent = 'Nexgen Safe';
        lastLockedState = null;
      }
    }

    function addKey(label, onClick) {
      const button = document.createElement('button');
      button.className = 'key';
      button.textContent = label;
      button.onclick = () => {
        playTap();
        onClick();
      };
      gridEl.appendChild(button);
    }

    function buildKeypad() {
      gridEl.innerHTML = '';
      ['1','2','3','4','5','6','7','8','9'].forEach((d) => addKey(d, () => digit(d)));
      addKey('Del', back);
      addKey('0', () => digit('0'));
      addKey('Go', submit);
    }

    function digit(d) {
      if (action === 'none' || pin.length >= 4) return;
      pin += d;
      renderDots();
    }

    function back() {
      if (!pin.length) return;
      pin = pin.slice(0, -1);
      renderDots();
    }

    function openSettings() {
      settingsModalEl.hidden = false;
    }

    function closeSettings() {
      settingsModalEl.hidden = true;
    }

    async function submit() {
      if (pin.length !== 4) return;
      try {
        if (action === 'lock') {
          await api('/lock', { pin });
          setMessage('Safe locked');
          playSuccess();
        }
        if (action === 'unlock') {
          await api('/unlock', { pin });
          setMessage('Safe unlocked');
          playSuccess();
        }
        if (action === 'set1') {
          pin1 = pin;
          pin = '';
          action = 'set2';
          setPrompt();
          renderDots();
          playTap();
          return;
        }
        if (action === 'set2') {
          await api('/setpin', { pin1, pin2: pin });
          setMessage('PIN updated');
          playSuccess();
        }
        resetFlow();
        await refreshStatus();
      } catch (err) {
        playError();
        setMessage(String(err.message || err));
      }
    }

    async function saveLcd() {
      try {
        await api('/lcd', {
          line1: document.getElementById('l1').value,
          line2: document.getElementById('l2').value
        });
        setMessage('LCD updated');
        playSuccess();
        closeSettings();
      } catch (err) {
        playError();
        setMessage(String(err.message || err));
      }
    }

    document.getElementById('lockBtn').onclick = () => {
      playTap();
      action = 'lock';
      pin = '';
      setPrompt();
      renderDots();
      setMessage('');
    };

    document.getElementById('unlockBtn').onclick = () => {
      playTap();
      action = 'unlock';
      pin = '';
      setPrompt();
      renderDots();
      setMessage('');
    };

    document.getElementById('setPinBtn').onclick = () => {
      playTap();
      action = 'set1';
      pin = '';
      pin1 = '';
      setPrompt();
      renderDots();
      setMessage('');
    };

    document.getElementById('cancelBtn').onclick = () => {
      playTap();
      resetFlow();
      setMessage('');
    };

    document.getElementById('settingsBtn').onclick = () => {
      playTap();
      openSettings();
    };

    document.getElementById('settingsCloseBtn').onclick = () => {
      playTap();
      closeSettings();
    };

    document.getElementById('settingsCancelBtn').onclick = () => {
      playTap();
      closeSettings();
    };

    document.getElementById('lcdBtn').onclick = () => {
      playTap();
      saveLcd();
    };

    settingsModalEl.onclick = (event) => {
      if (event.target === settingsModalEl) {
        closeSettings();
      }
    };

    buildKeypad();
    resetFlow();
    refreshStatus();
    setInterval(refreshStatus, 2000);
  </script>
</body>
</html>
)rawliteral";

#endif