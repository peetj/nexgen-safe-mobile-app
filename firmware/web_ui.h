#ifndef WEB_UI_H
#define WEB_UI_H

static const char INDEX_HTML[] PROGMEM = R"rawliteral(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Nexgen Safe</title>
  <style>
    :root{
      --bg-top:#15181e;
      --bg-bottom:#0b0d12;
      --panel-top:#22262e;
      --panel-bottom:#14181d;
      --modal-top:#1b1f27;
      --modal-bottom:#11141a;
      --orange:#ff5e19;
      --orange-soft:#ff8a47;
      --offwhite:#f2f2f0;
      --green:#34d399;
      --red:#fb7185;
      --amber:#fbbf24;
      --cyan:#5eead4;
      --glow-hot:rgba(255,94,25,.22);
      --glow-cool:rgba(94,234,212,.14);
      --brand-hot-shadow:rgba(255,94,25,.28);
      --brand-cool-start:#ffe27a;
      --brand-cool-end:#5eead4;
      --line-soft:rgba(242,242,240,.08);
      --line-strong:rgba(242,242,240,.14);
      --text-mid:rgba(242,242,240,.72);
      --text-low:rgba(242,242,240,.68);
      --text-subtle:rgba(242,242,240,.52);
      --text-faint:rgba(242,242,240,.42);
      --card-shadow:rgba(0,0,0,.24);
      --primary-shadow:rgba(255,94,25,.24);
      --ghost-bg:rgba(255,255,255,.03);
      --ghost-border:rgba(242,242,240,.14);
      --tonal-bg-top:rgba(94,234,212,.16);
      --tonal-bg-bottom:rgba(94,234,212,.08);
      --tonal-border:rgba(94,234,212,.30);
      --key-bg:rgba(242,242,240,.05);
      --key-border:rgba(242,242,240,.10);
      --field-bg:rgba(0,0,0,.22);
      --field-border:rgba(242,242,240,.14);
      --dot-bg:rgba(242,242,240,.16);
      --dot-glow:rgba(242,242,240,.26);
      --status-locked-glow:rgba(251,113,133,.28);
      --status-locked-border:rgba(251,113,133,.34);
      --status-locked-badge-border:rgba(251,113,133,.30);
      --status-locked-badge-top:rgba(251,113,133,.12);
      --status-unlocked-glow:rgba(52,211,153,.24);
      --status-unlocked-border:rgba(52,211,153,.34);
      --status-unlocked-badge-border:rgba(52,211,153,.28);
      --status-unlocked-badge-top:rgba(52,211,153,.12);
      --status-offline-glow:rgba(251,191,36,.24);
      --status-offline-border:rgba(251,191,36,.30);
      --status-offline-badge-border:rgba(251,191,36,.28);
      --status-offline-badge-top:rgba(251,191,36,.12);
    }
    *{box-sizing:border-box}
    body{
      margin:0;
      min-height:100vh;
      display:flex;
      flex-direction:column;
      font-family:"Segoe UI",Tahoma,Geneva,Verdana,sans-serif;
      color:var(--offwhite);
      background:
        radial-gradient(circle at top left, var(--glow-hot), transparent 30%),
        radial-gradient(circle at top right, var(--glow-cool), transparent 28%),
        linear-gradient(180deg, var(--bg-top) 0%, var(--bg-bottom) 100%);
    }
    header{
      padding:22px 18px 14px;
      border-bottom:1px solid var(--line-soft);
      background:linear-gradient(180deg, rgba(255,255,255,.05), rgba(255,255,255,0));
    }
    .brand-tag{
      font-size:11px;
      letter-spacing:.36em;
      text-transform:uppercase;
      color:var(--text-subtle);
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
      text-shadow:0 0 22px var(--brand-hot-shadow);
    }
    .brand-cool{
      background:linear-gradient(135deg, var(--brand-cool-start) 0%, var(--brand-cool-end) 100%);
      -webkit-background-clip:text;
      background-clip:text;
      color:transparent;
    }
    .wrap{
      width:100%;
      flex:1;
      max-width:560px;
      margin:0 auto;
      padding:18px 16px 12px;
    }
    .card{
      background:linear-gradient(180deg, var(--panel-top), var(--panel-bottom));
      border:1px solid var(--line-soft);
      border-radius:22px;
      padding:16px;
      margin-bottom:14px;
      box-shadow:0 18px 40px var(--card-shadow);
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
      --glow:var(--status-locked-glow);
      border-color:var(--status-locked-border);
    }
    .status-card.unlocked{
      --glow:var(--status-unlocked-glow);
      border-color:var(--status-unlocked-border);
    }
    .status-card.offline{
      --glow:var(--status-offline-glow);
      border-color:var(--status-offline-border);
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
      color:var(--text-mid);
    }
    .ip{
      font-size:13px;
      color:var(--text-low);
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
      border:1px solid var(--line-soft);
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
      border-color:var(--status-locked-badge-border);
      background:linear-gradient(180deg, var(--status-locked-badge-top), rgba(0,0,0,.18));
    }
    .status-card.unlocked .state-badge{
      color:var(--green);
      border-color:var(--status-unlocked-badge-border);
      background:linear-gradient(180deg, var(--status-unlocked-badge-top), rgba(0,0,0,.18));
    }
    .status-card.offline .state-badge{
      color:var(--amber);
      border-color:var(--status-offline-badge-border);
      background:linear-gradient(180deg, var(--status-offline-badge-top), rgba(0,0,0,.18));
    }
    .msg{
      min-height:18px;
      margin:12px 0 0;
      font-size:13px;
      letter-spacing:.04em;
      color:var(--text-mid);
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
    button:disabled{
      opacity:.45;
      cursor:not-allowed;
      filter:saturate(.5);
    }
    button.primary{
      background:linear-gradient(135deg, var(--orange) 0%, var(--orange-soft) 100%);
      color:#111;
      box-shadow:0 10px 26px var(--primary-shadow);
    }
    button.tonal{
      background:linear-gradient(180deg, var(--tonal-bg-top), var(--tonal-bg-bottom));
      color:var(--offwhite);
      border:1px solid var(--tonal-border);
    }
    button.ghost{
      background:var(--ghost-bg);
      color:var(--offwhite);
      border:1px solid var(--ghost-border);
    }
    button.small{
      padding:10px 12px;
      font-size:12px;
      letter-spacing:.14em;
      text-transform:uppercase;
    }
    .action-tools{
      margin-top:18px;
      margin-bottom:26px;
    }
    .prompt{
      min-height:18px;
      margin:0 0 12px;
      text-align:center;
      font-size:12px;
      letter-spacing:.24em;
      text-transform:uppercase;
      color:var(--text-low);
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
      background:var(--dot-bg);
      border:1px solid var(--line-soft);
    }
    .dot.filled{
      background:var(--offwhite);
      box-shadow:0 0 14px var(--dot-glow);
    }
    .grid{
      display:grid;
      grid-template-columns:repeat(3,1fr);
      gap:10px;
      margin-top:8px;
    }
    .key{
      min-height:60px;
      background:var(--key-bg);
      border:1px solid var(--key-border);
      color:var(--offwhite);
      font-size:22px;
    }
    .footer-row{
      display:flex;
      gap:10px;
      margin-top:10px;
    }
    .footer-row button{flex:1}
    .keypad-actions{
      margin-top:20px;
    }
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
      max-height:calc(100vh - 32px);
      overflow:auto;
      background:linear-gradient(180deg, var(--modal-top) 0%, var(--modal-bottom) 100%);
      border:1px solid var(--line-soft);
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
      color:var(--text-low);
      font-size:12px;
      line-height:1.5;
    }
    .settings-label{
      margin:12px 0 6px;
      font-size:11px;
      letter-spacing:.16em;
      text-transform:uppercase;
      color:var(--text-subtle);
    }
    input{
      width:100%;
      padding:14px;
      border-radius:14px;
      border:1px solid var(--field-border);
      background:var(--field-bg);
      color:var(--offwhite);
      font-size:15px;
      margin-bottom:10px;
    }
    select{
      width:100%;
      padding:14px;
      border-radius:14px;
      border:1px solid var(--field-border);
      background:var(--field-bg);
      color:var(--offwhite);
      font-size:15px;
      margin-bottom:10px;
    }
    .settings-block{
      margin-top:14px;
      padding-top:14px;
      border-top:1px solid var(--line-soft);
    }
    .theme-grid{
      display:grid;
      grid-template-columns:repeat(2, minmax(0, 1fr));
      gap:10px;
      margin:10px 0 4px;
    }
    .theme-color{
      padding:12px;
      border-radius:16px;
      border:1px solid var(--line-soft);
      background:linear-gradient(180deg, rgba(255,255,255,.04), rgba(255,255,255,.02));
    }
    .theme-color label{
      display:block;
      margin-bottom:8px;
      font-size:11px;
      letter-spacing:.14em;
      text-transform:uppercase;
      color:var(--text-low);
    }
    .theme-color input[type="color"]{
      width:100%;
      min-height:46px;
      margin:0;
      padding:4px;
      cursor:pointer;
      border-radius:12px;
      border:1px solid var(--field-border);
      background:var(--field-bg);
    }
    .theme-color input[type="color"]::-webkit-color-swatch-wrapper{padding:0}
    .theme-color input[type="color"]::-webkit-color-swatch{
      border:0;
      border-radius:8px;
    }
    .theme-tools{
      margin-top:8px;
    }
    .mini-hint{
      margin:-2px 0 8px;
      font-size:11px;
      line-height:1.5;
      color:var(--text-subtle);
    }
    .theme-delete{
      border-color:var(--status-locked-badge-border);
      color:var(--red);
    }
    input[readonly]{
      color:var(--text-mid);
    }
    .site-footer{
      padding:0 18px 20px;
      text-align:center;
      font-size:12px;
      letter-spacing:.14em;
      text-transform:uppercase;
      color:var(--text-faint);
    }
    @media (max-width:460px){
      h1{font-size:28px}
      .state-badge{font-size:25px;min-height:72px;padding:16px 18px}
      .topline{align-items:flex-start;flex-direction:column}
      .ip{margin-top:-6px}
      .theme-grid{grid-template-columns:1fr}
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
      <div class="footer-row action-tools">
        <button id="setPinBtn" class="ghost">Set PIN</button>
        <button id="settingsBtn" class="ghost">Settings</button>
      </div>
      <div id="prompt" class="prompt"></div>
      <div id="dots" class="dots"></div>
      <div class="grid" id="grid"></div>
      <div class="footer-row keypad-actions">
        <button id="cancelBtn" class="ghost">Cancel</button>
      </div>
    </div>
  </div>

  <footer class="site-footer">&copy; nexgenstemschool.com.au</footer>

  <div id="settingsModal" class="modal" hidden>
    <div class="modal-card">
      <div class="modal-head">
        <div>
          <div class="modal-title">Settings</div>
          <div class="hint">Themes are local to this browser. Renaming the safe needs a restart before the new Wi-Fi name and hostname become active.</div>
        </div>
        <button id="settingsCloseBtn" class="ghost small">Close</button>
      </div>
      <div class="settings-label">Theme</div>
      <select id="themeSelect"></select>
      <div class="mini-hint">Choose a built-in theme or one of your saved custom themes.</div>
      <div class="settings-label">Custom theme name</div>
      <input id="customThemeName" maxlength="24" placeholder="My Safe Theme" value="">
      <div class="theme-tools footer-row">
        <button id="saveThemeBtn" class="tonal">Save Theme</button>
        <button id="deleteThemeBtn" class="ghost theme-delete">Delete Theme</button>
      </div>
      <div class="mini-hint">Change the colors below, then save them as a named custom theme for this phone or browser.</div>
      <div class="theme-grid">
        <div class="theme-color">
          <label for="colorBgTop">Page Top</label>
          <input id="colorBgTop" type="color" value="#15181e">
        </div>
        <div class="theme-color">
          <label for="colorBgBottom">Page Bottom</label>
          <input id="colorBgBottom" type="color" value="#0b0d12">
        </div>
        <div class="theme-color">
          <label for="colorPanelTop">Panel Top</label>
          <input id="colorPanelTop" type="color" value="#22262e">
        </div>
        <div class="theme-color">
          <label for="colorPanelBottom">Panel Bottom</label>
          <input id="colorPanelBottom" type="color" value="#14181d">
        </div>
        <div class="theme-color">
          <label for="colorModalTop">Settings Top</label>
          <input id="colorModalTop" type="color" value="#1b1f27">
        </div>
        <div class="theme-color">
          <label for="colorModalBottom">Settings Bottom</label>
          <input id="colorModalBottom" type="color" value="#11141a">
        </div>
        <div class="theme-color">
          <label for="colorOrange">Primary</label>
          <input id="colorOrange" type="color" value="#ff5e19">
        </div>
        <div class="theme-color">
          <label for="colorOrangeSoft">Secondary</label>
          <input id="colorOrangeSoft" type="color" value="#ff8a47">
        </div>
        <div class="theme-color">
          <label for="colorCyan">Accent</label>
          <input id="colorCyan" type="color" value="#5eead4">
        </div>
        <div class="theme-color">
          <label for="colorOffwhite">Text</label>
          <input id="colorOffwhite" type="color" value="#f2f2f0">
        </div>
        <div class="theme-color">
          <label for="colorGreen">Success</label>
          <input id="colorGreen" type="color" value="#34d399">
        </div>
        <div class="theme-color">
          <label for="colorRed">Danger</label>
          <input id="colorRed" type="color" value="#fb7185">
        </div>
        <div class="theme-color">
          <label for="colorAmber">Warning</label>
          <input id="colorAmber" type="color" value="#fbbf24">
        </div>
      </div>
      <div class="settings-block">
      <div class="settings-label">Safe name</div>
      <input id="safeName" maxlength="31" placeholder="NexgenSafe-01" value="NexgenSafe-01">
      <div class="settings-label">Local hostname</div>
      <input id="localUrl" readonly value="http://nexgensafe-01.local">
      <div class="settings-label">Fallback IP</div>
      <input id="ipUrl" readonly value="http://192.168.4.1">
      </div>
      <div class="settings-block">
      <div class="settings-label">LCD line 1</div>
      <input id="l1" placeholder="Line 1" value="Nexgen Safe">
      <div class="settings-label">LCD line 2</div>
      <input id="l2" placeholder="Line 2" value="Use the browser">
      </div>
      <div class="footer-row">
        <button id="renameBtn" class="tonal">Save Name</button>
        <button id="restartBtn" class="ghost">Restart ESP32</button>
      </div>
      <div class="row">
        <button id="settingsCancelBtn" class="ghost" style="flex:1">Cancel</button>
        <button id="lcdBtn" class="primary" style="flex:1">Save to LCD</button>
      </div>
    </div>
  </div>

  <script>
    const apiBase = location.protocol === 'file:' ? 'http://192.168.4.1' : '';
    const fallbackUrl = 'http://192.168.4.1';
    const statusCardEl = document.getElementById('statusCard');
    const stateBadgeEl = document.getElementById('stateBadge');
    const nameEl = document.getElementById('name');
    const msgEl = document.getElementById('msg');
    const promptEl = document.getElementById('prompt');
    const dotsEl = document.getElementById('dots');
    const gridEl = document.getElementById('grid');
    const settingsModalEl = document.getElementById('settingsModal');
    const themeSelectEl = document.getElementById('themeSelect');
    const customThemeNameEl = document.getElementById('customThemeName');
    const safeNameEl = document.getElementById('safeName');
    const localUrlEl = document.getElementById('localUrl');
    const ipUrlEl = document.getElementById('ipUrl');
    const ACTIVE_THEME_STORAGE_KEY = 'nexgen-safe-theme';
    const CUSTOM_THEMES_STORAGE_KEY = 'nexgen-safe-custom-themes';
    const THEME_FIELDS = [
      { key: 'bgTop', inputId: 'colorBgTop' },
      { key: 'bgBottom', inputId: 'colorBgBottom' },
      { key: 'panelTop', inputId: 'colorPanelTop' },
      { key: 'panelBottom', inputId: 'colorPanelBottom' },
      { key: 'modalTop', inputId: 'colorModalTop' },
      { key: 'modalBottom', inputId: 'colorModalBottom' },
      { key: 'orange', inputId: 'colorOrange' },
      { key: 'orangeSoft', inputId: 'colorOrangeSoft' },
      { key: 'cyan', inputId: 'colorCyan' },
      { key: 'offwhite', inputId: 'colorOffwhite' },
      { key: 'green', inputId: 'colorGreen' },
      { key: 'red', inputId: 'colorRed' },
      { key: 'amber', inputId: 'colorAmber' }
    ];
    const PRESET_THEMES = {
      ember: {
        label: 'Nexgen Ember',
        values: {
          bgTop: '#15181e',
          bgBottom: '#0b0d12',
          panelTop: '#22262e',
          panelBottom: '#14181d',
          modalTop: '#1b1f27',
          modalBottom: '#11141a',
          orange: '#ff5e19',
          orangeSoft: '#ff8a47',
          cyan: '#5eead4',
          offwhite: '#f2f2f0',
          green: '#34d399',
          red: '#fb7185',
          amber: '#fbbf24'
        }
      },
      coast: {
        label: 'Coast Mint',
        values: {
          bgTop: '#0b1720',
          bgBottom: '#041118',
          panelTop: '#14323a',
          panelBottom: '#0a1f26',
          modalTop: '#12313a',
          modalBottom: '#0a1e26',
          orange: '#2dd4bf',
          orangeSoft: '#67e8f9',
          cyan: '#38bdf8',
          offwhite: '#effcfb',
          green: '#22c55e',
          red: '#f97316',
          amber: '#facc15'
        }
      },
      vault: {
        label: 'Vault Gold',
        values: {
          bgTop: '#1c1608',
          bgBottom: '#0d0b04',
          panelTop: '#32270e',
          panelBottom: '#181208',
          modalTop: '#2a210d',
          modalBottom: '#151108',
          orange: '#f59e0b',
          orangeSoft: '#fde047',
          cyan: '#d4d4d8',
          offwhite: '#fff7db',
          green: '#84cc16',
          red: '#ef4444',
          amber: '#f59e0b'
        }
      },
      midnight: {
        label: 'Midnight Signal',
        values: {
          bgTop: '#0b1020',
          bgBottom: '#040712',
          panelTop: '#151b34',
          panelBottom: '#0a1022',
          modalTop: '#161d38',
          modalBottom: '#0a1022',
          orange: '#60a5fa',
          orangeSoft: '#a78bfa',
          cyan: '#38bdf8',
          offwhite: '#edf2ff',
          green: '#22d3ee',
          red: '#f472b6',
          amber: '#c084fc'
        }
      }
    };
    const themeInputEls = Object.fromEntries(THEME_FIELDS.map(({ key, inputId }) => [key, document.getElementById(inputId)]));

    let action = 'none';
    let pin = '';
    let pin1 = '';
    let lastLockedState = null;
    let audioCtx = null;
    let desiredSafeName = '';
    let desiredSafeUrl = '';
    let customThemes = {};
    let activeThemeId = 'preset:ember';

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

    function isHexColor(value) {
      return /^#[0-9a-f]{6}$/i.test(value || '');
    }

    function normalizeHex(value, fallback) {
      return isHexColor(value) ? value.toLowerCase() : fallback;
    }

    function hexToRgb(hex) {
      const clean = normalizeHex(hex, '#000000').slice(1);
      return {
        r: parseInt(clean.slice(0, 2), 16),
        g: parseInt(clean.slice(2, 4), 16),
        b: parseInt(clean.slice(4, 6), 16)
      };
    }

    function rgba(hex, alpha) {
      const rgb = hexToRgb(hex);
      return 'rgba(' + rgb.r + ',' + rgb.g + ',' + rgb.b + ',' + alpha + ')';
    }

    function cloneThemeValues(values) {
      return JSON.parse(JSON.stringify(values));
    }

    function normalizeThemeValues(values) {
      const fallback = PRESET_THEMES.ember.values;
      const normalized = {};
      THEME_FIELDS.forEach(({ key }) => {
        normalized[key] = normalizeHex(values && values[key], fallback[key]);
      });
      return normalized;
    }

    function buildThemeCssVars(values) {
      const theme = normalizeThemeValues(values);
      return {
        '--bg-top': theme.bgTop,
        '--bg-bottom': theme.bgBottom,
        '--panel-top': theme.panelTop,
        '--panel-bottom': theme.panelBottom,
        '--modal-top': theme.modalTop,
        '--modal-bottom': theme.modalBottom,
        '--orange': theme.orange,
        '--orange-soft': theme.orangeSoft,
        '--cyan': theme.cyan,
        '--offwhite': theme.offwhite,
        '--green': theme.green,
        '--red': theme.red,
        '--amber': theme.amber,
        '--glow-hot': rgba(theme.orange, 0.22),
        '--glow-cool': rgba(theme.cyan, 0.16),
        '--brand-hot-shadow': rgba(theme.orange, 0.28),
        '--brand-cool-start': theme.orangeSoft,
        '--brand-cool-end': theme.cyan,
        '--line-soft': rgba(theme.offwhite, 0.08),
        '--line-strong': rgba(theme.offwhite, 0.14),
        '--text-mid': rgba(theme.offwhite, 0.72),
        '--text-low': rgba(theme.offwhite, 0.68),
        '--text-subtle': rgba(theme.offwhite, 0.52),
        '--text-faint': rgba(theme.offwhite, 0.42),
        '--card-shadow': rgba(theme.bgBottom, 0.55),
        '--primary-shadow': rgba(theme.orange, 0.24),
        '--ghost-bg': rgba(theme.offwhite, 0.03),
        '--ghost-border': rgba(theme.offwhite, 0.14),
        '--tonal-bg-top': rgba(theme.cyan, 0.16),
        '--tonal-bg-bottom': rgba(theme.cyan, 0.08),
        '--tonal-border': rgba(theme.cyan, 0.30),
        '--key-bg': rgba(theme.offwhite, 0.05),
        '--key-border': rgba(theme.offwhite, 0.10),
        '--field-bg': rgba(theme.bgBottom, 0.55),
        '--field-border': rgba(theme.offwhite, 0.14),
        '--dot-bg': rgba(theme.offwhite, 0.16),
        '--dot-glow': rgba(theme.offwhite, 0.26),
        '--status-locked-glow': rgba(theme.red, 0.28),
        '--status-locked-border': rgba(theme.red, 0.34),
        '--status-locked-badge-border': rgba(theme.red, 0.30),
        '--status-locked-badge-top': rgba(theme.red, 0.12),
        '--status-unlocked-glow': rgba(theme.green, 0.24),
        '--status-unlocked-border': rgba(theme.green, 0.34),
        '--status-unlocked-badge-border': rgba(theme.green, 0.28),
        '--status-unlocked-badge-top': rgba(theme.green, 0.12),
        '--status-offline-glow': rgba(theme.amber, 0.24),
        '--status-offline-border': rgba(theme.amber, 0.30),
        '--status-offline-badge-border': rgba(theme.amber, 0.28),
        '--status-offline-badge-top': rgba(theme.amber, 0.12)
      };
    }

    function applyThemeValues(values) {
      const cssVars = buildThemeCssVars(values);
      Object.keys(cssVars).forEach((key) => {
        document.documentElement.style.setProperty(key, cssVars[key]);
      });
    }

    function setThemeEditorValues(values) {
      const normalized = normalizeThemeValues(values);
      THEME_FIELDS.forEach(({ key }) => {
        themeInputEls[key].value = normalized[key];
      });
    }

    function getThemeEditorValues() {
      const values = {};
      THEME_FIELDS.forEach(({ key }) => {
        values[key] = normalizeHex(themeInputEls[key].value, PRESET_THEMES.ember.values[key]);
      });
      return values;
    }

    function normalizeStoredThemeId(rawValue) {
      if (!rawValue) return 'preset:ember';
      if (rawValue.startsWith('preset:') || rawValue.startsWith('custom:')) return rawValue;
      if (PRESET_THEMES[rawValue]) return 'preset:' + rawValue;
      return 'preset:ember';
    }

    function slugifyThemeName(name) {
      const slug = String(name || '')
        .trim()
        .toLowerCase()
        .replace(/[^a-z0-9]+/g, '-')
        .replace(/^-+|-+$/g, '')
        .slice(0, 24);
      return slug || 'theme';
    }

    function buildUniqueCustomThemeId(name, ignoreId = '') {
      const base = slugifyThemeName(name);
      let candidate = base;
      let counter = 2;
      while (customThemes[candidate] && candidate !== ignoreId) {
        candidate = base + '-' + counter;
        counter += 1;
      }
      return candidate;
    }

    function loadCustomThemes() {
      customThemes = {};
      try {
        const parsed = JSON.parse(localStorage.getItem(CUSTOM_THEMES_STORAGE_KEY) || '{}');
        Object.keys(parsed || {}).forEach((id) => {
          const item = parsed[id];
          if (!item || typeof item.name !== 'string') return;
          const trimmedName = item.name.trim().slice(0, 24);
          if (!trimmedName) return;
          customThemes[id] = {
            name: trimmedName,
            values: normalizeThemeValues(item.values)
          };
        });
      } catch (_) {
        customThemes = {};
      }
    }

    function persistCustomThemes() {
      localStorage.setItem(CUSTOM_THEMES_STORAGE_KEY, JSON.stringify(customThemes));
    }

    function renderThemeOptions(selectedId = activeThemeId) {
      themeSelectEl.innerHTML = '';

      const presetGroup = document.createElement('optgroup');
      presetGroup.label = 'Built-in';
      Object.entries(PRESET_THEMES).forEach(([id, theme]) => {
        const option = document.createElement('option');
        option.value = 'preset:' + id;
        option.textContent = theme.label;
        presetGroup.appendChild(option);
      });
      themeSelectEl.appendChild(presetGroup);

      const customIds = Object.keys(customThemes).sort((a, b) => customThemes[a].name.localeCompare(customThemes[b].name));
      if (customIds.length) {
        const customGroup = document.createElement('optgroup');
        customGroup.label = 'Custom';
        customIds.forEach((id) => {
          const option = document.createElement('option');
          option.value = 'custom:' + id;
          option.textContent = customThemes[id].name;
          customGroup.appendChild(option);
        });
        themeSelectEl.appendChild(customGroup);
      }

      themeSelectEl.value = themeSelectEl.querySelector('option[value="' + selectedId + '"]') ? selectedId : 'preset:ember';
    }

    function getThemeRecord(themeId) {
      const normalizedId = normalizeStoredThemeId(themeId);
      if (normalizedId.startsWith('custom:')) {
        const customId = normalizedId.slice(7);
        const customTheme = customThemes[customId];
        if (customTheme) {
          return {
            id: normalizedId,
            kind: 'custom',
            key: customId,
            label: customTheme.name,
            values: cloneThemeValues(customTheme.values)
          };
        }
      }

      const presetKey = normalizedId.replace(/^preset:/, '');
      const preset = PRESET_THEMES[presetKey] || PRESET_THEMES.ember;
      const resolvedKey = PRESET_THEMES[presetKey] ? presetKey : 'ember';
      return {
        id: 'preset:' + resolvedKey,
        kind: 'preset',
        key: resolvedKey,
        label: preset.label,
        values: cloneThemeValues(preset.values)
      };
    }

    function applyTheme(themeId, persist = true) {
      const record = getThemeRecord(themeId);
      activeThemeId = record.id;
      renderThemeOptions(record.id);
      themeSelectEl.value = record.id;
      setThemeEditorValues(record.values);
      customThemeNameEl.value = record.kind === 'custom' ? record.label : '';
      document.getElementById('deleteThemeBtn').disabled = record.kind !== 'custom';
      document.body.dataset.theme = record.id.replace(':', '-');
      applyThemeValues(record.values);
      if (persist) {
        localStorage.setItem(ACTIVE_THEME_STORAGE_KEY, record.id);
      }
    }

    function loadTheme() {
      loadCustomThemes();
      renderThemeOptions('preset:ember');
      try {
        applyTheme(normalizeStoredThemeId(localStorage.getItem(ACTIVE_THEME_STORAGE_KEY)), false);
      } catch (_) {
        applyTheme('preset:ember', false);
      }
    }

    function previewThemeFromInputs() {
      applyThemeValues(getThemeEditorValues());
    }

    async function saveCustomTheme() {
      const requestedName = customThemeNameEl.value.trim().slice(0, 24);
      if (!requestedName) {
        playError();
        setMessage('Enter a custom theme name first');
        return;
      }

      const editingCustomId = activeThemeId.startsWith('custom:') ? activeThemeId.slice(7) : '';
      const existingTheme = editingCustomId ? customThemes[editingCustomId] : null;
      let targetId = editingCustomId;

      if (!existingTheme || existingTheme.name.toLowerCase() !== requestedName.toLowerCase()) {
        targetId = buildUniqueCustomThemeId(requestedName, editingCustomId);
        if (editingCustomId && editingCustomId !== targetId) {
          delete customThemes[editingCustomId];
        }
      }

      customThemes[targetId] = {
        name: requestedName,
        values: normalizeThemeValues(getThemeEditorValues())
      };

      persistCustomThemes();
      applyTheme('custom:' + targetId);
      playSuccess();
      setMessage('Custom theme saved: ' + requestedName);
    }

    function deleteCustomTheme() {
      if (!activeThemeId.startsWith('custom:')) {
        playError();
        setMessage('Select a custom theme to delete');
        return;
      }

      const customId = activeThemeId.slice(7);
      const themeName = customThemes[customId] ? customThemes[customId].name : 'Custom theme';
      delete customThemes[customId];
      persistCustomThemes();
      applyTheme('preset:ember');
      playSuccess();
      setMessage('Deleted custom theme: ' + themeName);
    }

    function syncAccessUrls(url, name) {
      const effectiveName = desiredSafeName || name || 'Nexgen Safe';
      const effectiveUrl = desiredSafeUrl || url || fallbackUrl;

      if (document.activeElement !== safeNameEl) {
        safeNameEl.value = effectiveName;
      }
      localUrlEl.value = effectiveUrl;
      ipUrlEl.value = fallbackUrl;
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
        const safeName = json.name || 'Nexgen Safe';
        if (desiredSafeName && safeName === desiredSafeName) {
          desiredSafeName = '';
          desiredSafeUrl = '';
        }
        nameEl.textContent = safeName;
        syncAccessUrls(json.url, safeName);

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
        syncAccessUrls('', 'Nexgen Safe');
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

    async function renameSafe() {
      const requestedName = safeNameEl.value.trim();
      if (!requestedName) {
        playError();
        setMessage('Enter a safe name first');
        return;
      }

      try {
        const json = await api('/rename', { name: requestedName });
        playSuccess();
        desiredSafeName = json.name || requestedName;
        desiredSafeUrl = json.url || fallbackUrl;
        syncAccessUrls(json.url, json.name);
        setMessage((json.restart_required ? 'Name saved. Tap Restart ESP32 to apply ' : 'Safe name updated to ') + (json.name || requestedName));
      } catch (err) {
        playError();
        setMessage(String(err.message || err));
      }
    }

    async function restartSafe() {
      try {
        await api('/restart', {});
        playSuccess();
        closeSettings();
        setMessage('ESP32 restarting. Reconnect to ' + (desiredSafeName || safeNameEl.value.trim() || nameEl.textContent) + ' and open ' + (desiredSafeUrl || localUrlEl.value || fallbackUrl));
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

    themeSelectEl.onchange = () => {
      playTap();
      applyTheme(themeSelectEl.value);
      setMessage('Theme loaded: ' + getThemeRecord(themeSelectEl.value).label);
    };

    document.getElementById('renameBtn').onclick = () => {
      playTap();
      renameSafe();
    };

    document.getElementById('restartBtn').onclick = () => {
      playTap();
      restartSafe();
    };

    document.getElementById('lcdBtn').onclick = () => {
      playTap();
      saveLcd();
    };

    document.getElementById('saveThemeBtn').onclick = () => {
      playTap();
      saveCustomTheme();
    };

    document.getElementById('deleteThemeBtn').onclick = () => {
      playTap();
      deleteCustomTheme();
    };

    THEME_FIELDS.forEach(({ key }) => {
      themeInputEls[key].addEventListener('input', () => {
        previewThemeFromInputs();
      });
    });

    settingsModalEl.onclick = (event) => {
      if (event.target === settingsModalEl) {
        closeSettings();
      }
    };

    buildKeypad();
    resetFlow();
    loadTheme();
    syncAccessUrls('', 'Nexgen Safe');
    refreshStatus();
    setInterval(refreshStatus, 2000);
  </script>
</body>
</html>
)rawliteral";

#endif // WEB_UI_H