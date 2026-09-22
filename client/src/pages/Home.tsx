import { useEffect, useMemo, useState } from "react";
import {
  Activity,
  AlertTriangle,
  ArrowUpRight,
  Beaker,
  Binary,
  Check,
  ChevronRight,
  CircleHelp,
  Cloud,
  Code2,
  Droplets,
  Fan,
  GitBranch,
  Github,
  Leaf,
  Lightbulb,
  LockKeyhole,
  Menu,
  MoreHorizontal,
  Play,
  Power,
  Radio,
  RefreshCw,
  Save,
  Settings2,
  Sprout,
  SunMedium,
  Thermometer,
  Waves,
  X,
  Zap,
} from "lucide-react";

type NavKey = "overview" | "code" | "automation" | "hardware";

type Telemetry = {
  temperature: number;
  humidity: number;
  ph: number;
  tds: number;
  water: number;
};

type Relay = {
  id: string;
  label: string;
  detail: string;
  pin: string;
  icon: typeof Beaker;
  color: string;
  active: boolean;
  auto: boolean;
};

const navItems: { key: NavKey; label: string; icon: typeof Activity; badge?: string }[] = [
  { key: "overview", label: "Live overview", icon: Activity },
  { key: "code", label: "Code lab", icon: Code2, badge: "5" },
  { key: "automation", label: "Automation", icon: Zap },
  { key: "hardware", label: "Hardware map", icon: Binary },
];

const codeFiles = [
  { name: "firmware/main.ino", type: "ino", lines: 238 },
  { name: "dashboard/index.html", type: "html", lines: 166 },
  { name: "dashboard/style.css", type: "css", lines: 302 },
  { name: "dashboard/api.php", type: "php", lines: 86 },
  { name: "database/schema.sql", type: "sql", lines: 32 },
  { name: "ml/predict.py", type: "py", lines: 94 },
  { name: "README.md", type: "md", lines: 158 },
];

const previewCode: Record<string, string> = {
  "firmware/main.ino": `// Automated NFT Hydroponics — ESP32\n\nconst float PH_MIN = 5.5;\nconst float PH_MAX = 6.5;\nconst float TEMP_FAN_ON = 35.0;\n\nvoid loop() {\n  readSensors();\n  regulateClimate();\n  updateOLED();\n  postReadingsToServer();\n  delay(5000);\n}`,
  "dashboard/index.html": `<!-- Live telemetry dashboard -->\n<section class="metric-grid">\n  <article class="metric-card">\n    <span>Water temperature</span>\n    <strong id="temperatureValue">24.8°</strong>\n    <div class="gauge" data-source="temperature"></div>\n  </article>\n</section>`,
  "dashboard/style.css": `:root {\n  --ink: #10231d;\n  --leaf: #71c858;\n  --cream: #f7f8ee;\n  --amber: #f6bd52;\n}\n\n.metric-card {\n  border: 1px solid rgba(16,35,29,.12);\n  border-radius: 20px;\n  padding: 24px;\n}`,
  "dashboard/api.php": `<?php\n// Receive JSON posted by the ESP32 and persist a safe snapshot.\n$payload = json_decode(file_get_contents('php://input'), true);\nif (!is_array($payload)) { http_response_code(400); exit; }\n\n$stmt = $pdo->prepare('INSERT INTO sensor_readings\n  (temperature, humidity, ph, tds, water_level)\n  VALUES (?, ?, ?, ?, ?)');\n$stmt->execute([$payload['temperature'], $payload['humidity'],\n  $payload['ph'], $payload['tds'], $payload['water_level']]);`,
  "database/schema.sql": `CREATE TABLE sensor_readings (\n  id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,\n  temperature DECIMAL(5,2) NOT NULL,\n  humidity DECIMAL(5,2) NOT NULL,\n  ph DECIMAL(4,2) NOT NULL,\n  tds DECIMAL(7,2) NOT NULL,\n  water_level DECIMAL(5,2) NOT NULL,\n  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP\n);`,
  "ml/predict.py": `# Predict a lettuce growth stage from one image.\nfrom pathlib import Path\nimport cv2\nimport torch\n\nSTAGES = ["seedling", "vegetative", "mature"]\nimage = cv2.imread(str(Path("input.jpg")))\n# Load your fine-tuned checkpoint and run inference here.\nprint({"stage": STAGES[0], "confidence": 0.91})`,
  "README.md": `# HydroNexus IoT NFT Hydroponics\n\nA student-friendly automated lettuce grow system built around ESP32,\nBME680, pH/TDS probes, relays, and a tiny OLED status screen.\n\n## Quick start\n1. Flash firmware/main.ino.\n2. Import database/schema.sql.\n3. Serve dashboard/ with PHP.\n4. Run python ml/predict.py --image ./input.jpg.`,
};

const initialRelays: Relay[] = [
  { id: "ph-up", label: "pH dosing", detail: "Relay 1 · GPIO 16", pin: "16", icon: Beaker, color: "mint", active: false, auto: true },
  { id: "water-pump", label: "Water pump", detail: "Relay 2 · GPIO 17", pin: "17", icon: Droplets, color: "blue", active: true, auto: true },
  { id: "fan", label: "Climate fan", detail: "Relay 3 · GPIO 19", pin: "19", icon: Fan, color: "amber", active: false, auto: true },
  { id: "lights", label: "Grow lights", detail: "Relay 4 · GPIO 23", pin: "23", icon: SunMedium, color: "lavender", active: true, auto: false },
];

const tempHistory = [22, 23.4, 23, 24.1, 24.6, 24.2, 24.8, 24.6, 25.1, 24.8];

function Gauge({ value, min, max, color, unit, label, status, icon: Icon }: { value: number; min: number; max: number; color: string; unit: string; label: string; status: string; icon: typeof Thermometer }) {
  const ratio = Math.min(1, Math.max(0, (value - min) / (max - min)));
  const circumference = 2 * Math.PI * 38;
  const dash = circumference * ratio;
  return (
    <div className="metric-card group">
      <div className="metric-card__top">
        <div className={`metric-icon metric-icon--${color}`}><Icon size={17} strokeWidth={2.2} /></div>
        <span className="metric-label">{label}</span>
        <button className="icon-button icon-button--subtle" aria-label={`More about ${label}`}><MoreHorizontal size={17} /></button>
      </div>
      <div className="gauge-wrap">
        <svg className="gauge-svg" viewBox="0 0 96 96" aria-hidden="true">
          <circle className="gauge-track" cx="48" cy="48" r="38" />
          <circle className={`gauge-progress gauge-progress--${color}`} cx="48" cy="48" r="38" strokeDasharray={`${dash} ${circumference}`} />
        </svg>
        <div className="gauge-value"><strong>{value}</strong><span>{unit}</span></div>
      </div>
      <div className="metric-card__footer"><span className={`status-dot status-dot--${color}`} />{status}<span className="metric-range">{min}–{max}{unit === "pH" ? "" : unit}</span></div>
    </div>
  );
}

function Sparkline() {
  const points = tempHistory.map((value, index) => `${index * 30 + 4},${52 - (value - 22) * 7}`).join(" ");
  return (
    <svg className="sparkline" viewBox="0 0 280 60" preserveAspectRatio="none" role="img" aria-label="Temperature trend over the last hour">
      <defs><linearGradient id="lineFill" x1="0" x2="0" y1="0" y2="1"><stop offset="0" stopColor="#70cb63" stopOpacity=".34" /><stop offset="1" stopColor="#70cb63" stopOpacity="0" /></linearGradient></defs>
      <path d={`M 4,58 L ${points} L 274,58 Z`} fill="url(#lineFill)" />
      <polyline points={points} fill="none" stroke="#78cf67" strokeWidth="2.5" strokeLinecap="round" strokeLinejoin="round" />
      <circle cx="274" cy="31" r="4" fill="#0f221c" stroke="#9cf38a" strokeWidth="2" />
    </svg>
  );
}

function PlantGraphic() {
  return (
    <div className="plant-graphic" aria-label="Illustration of the HydroNexus vertical grow rack">
      <div className="plant-graphic__glow" />
      <div className="plant-graphic__rack rack-one"><span /><span /><span /></div>
      <div className="plant-graphic__rack rack-two"><span /><span /><span /><span /></div>
      <div className="plant-graphic__base"><span>HYDRO</span><span>NEXUS</span></div>
      <div className="plant plant-a"><i /><i /><i /><b /></div>
      <div className="plant plant-b"><i /><i /><i /><b /></div>
      <div className="plant plant-c"><i /><i /><i /><b /></div>
      <div className="water-lines"><span /><span /><span /></div>
    </div>
  );
}

function CodeLab({ selectedFile, onSelect }: { selectedFile: string; onSelect: (file: string) => void }) {
  const current = codeFiles.find((file) => file.name === selectedFile) ?? codeFiles[0];
  const content = previewCode[current.name];
  return (
    <section className="code-lab fade-in">
      <div className="page-heading code-heading">
        <div><p className="eyebrow">Repository workspace</p><h1>Code lab <span className="heading-note">/ beginner-ready</span></h1><p className="subheading">Every layer of your NFT system, documented and ready to flash, serve, or train.</p></div>
        <div className="heading-actions"><button className="button button--ghost"><Github size={16} /> Open repository</button><button className="button button--primary"><Save size={16} /> Download bundle</button></div>
      </div>
      <div className="code-workspace">
        <aside className="file-tree">
          <div className="file-tree__header"><span>hydronexus/</span><span className="tree-status"><span className="status-dot status-dot--mint" /> synced</span></div>
          <div className="file-tree__items">{codeFiles.map((file) => <button key={file.name} className={`file-row ${selectedFile === file.name ? "file-row--active" : ""}`} onClick={() => onSelect(file.name)}><span className={`file-type file-type--${file.type}`}>{file.type}</span><span>{file.name}</span><span className="file-lines">{file.lines}</span></button>)}</div>
          <div className="tree-note"><LockKeyhole size={14} /><span>Local preview · no secrets exposed</span></div>
        </aside>
        <div className="code-panel">
          <div className="code-panel__bar"><div className="code-tab"><span className={`file-type file-type--${current.type}`}>{current.type}</span>{current.name}</div><div className="code-actions"><button className="icon-button" aria-label="Refresh code"><RefreshCw size={15} /></button><button className="icon-button" aria-label="More code actions"><MoreHorizontal size={17} /></button></div></div>
          <div className="code-editor"><div className="line-numbers">{content.split("\n").map((_, index) => <span key={index}>{String(index + 1).padStart(2, "0")}</span>)}</div><pre><code>{content}</code></pre></div>
          <div className="code-panel__footer"><span><span className="status-dot status-dot--mint" /> Ready to flash</span><span>UTF-8</span><span>main branch</span></div>
        </div>
      </div>
    </section>
  );
}

function HardwareMap({ relays }: { relays: Relay[] }) {
  return <section className="hardware-view fade-in"><div className="page-heading"><div><p className="eyebrow">Pin-level overview</p><h1>Hardware map <span className="heading-note">/ GPIO topology</span></h1><p className="subheading">A visual inventory of every sensor, relay, and display in the loop.</p></div><button className="button button--ghost"><Settings2 size={16} /> Wiring guide</button></div><div className="hardware-grid"><div className="board-card"><div className="board-card__top"><div><span className="eyebrow">Controller</span><h2>ESP32 DevKit V1</h2></div><span className="connected-pill"><span className="status-dot status-dot--mint" /> connected</span></div><div className="esp-board"><div className="esp-board__chip"><span>ESP</span><strong>32</strong></div><div className="board-pin pin-34">34 <small>pH</small></div><div className="board-pin pin-35">35 <small>TDS</small></div><div className="board-pin pin-i2c">I²C <small>BME680 + OLED</small></div><div className="board-pin pin-5">05 <small>TRIG</small></div><div className="board-pin pin-18">18 <small>ECHO</small></div><div className="board-pin pin-relay">16 · 17 · 19 · 23 <small>4CH RELAY</small></div><div className="board-led led-one" /><div className="board-led led-two" /></div><div className="board-card__footer"><span><Radio size={14} /> last heartbeat 12 sec ago</span><span>firmware v0.4.2</span></div></div><div className="sensor-list"><div className="list-heading"><span>Connected modules</span><span className="count-badge">7 online</span></div>{[{ icon: Thermometer, name: "BME680 environmental", pin: "I²C · 0x76", state: "Healthy" }, { icon: Beaker, name: "Analog pH probe", pin: "ADC · GPIO 34", state: "Healthy" }, { icon: Waves, name: "HC-SR04 water level", pin: "GPIO 5 / 18", state: "Watch" }, { icon: Lightbulb, name: "0.96” OLED display", pin: "I²C · 0x3C", state: "Healthy" }].map((item) => <div className="sensor-row" key={item.name}><div className="sensor-row__icon"><item.icon size={16} /></div><div><strong>{item.name}</strong><span>{item.pin}</span></div><span className={`sensor-state ${item.state === "Watch" ? "sensor-state--watch" : ""}`}><span className="status-dot status-dot--mint" />{item.state}</span></div>)}</div></div></section>;
}

export default function Home() {
  const [activeNav, setActiveNav] = useState<NavKey>("overview");
  const [relays, setRelays] = useState(initialRelays);
  const [mobileNavOpen, setMobileNavOpen] = useState(false);
  const [selectedFile, setSelectedFile] = useState("firmware/main.ino");
  const [lastUpdated, setLastUpdated] = useState("12 sec ago");
  const [telemetry, setTelemetry] = useState<Telemetry>({ temperature: 24.8, humidity: 61, ph: 6.2, tds: 842, water: 78 });
  const [toast, setToast] = useState<string | null>(null);

  useEffect(() => {
    const interval = window.setInterval(() => {
      setTelemetry((current) => ({ temperature: Number((current.temperature + (Math.random() - 0.5) * 0.18).toFixed(1)), humidity: Math.round(current.humidity + (Math.random() - 0.5)), ph: Number(Math.min(6.5, Math.max(5.5, current.ph + (Math.random() - 0.5) * 0.03)).toFixed(2)), tds: Math.round(current.tds + (Math.random() - 0.5) * 6), water: Math.round(Math.min(100, Math.max(45, current.water + (Math.random() - 0.5) * 1.5))) }));
      setLastUpdated("just now");
    }, 5000);
    return () => window.clearInterval(interval);
  }, []);

  const activeCount = useMemo(() => relays.filter((relay) => relay.active).length, [relays]);
  const openView = (view: NavKey) => { setActiveNav(view); setMobileNavOpen(false); };
  const toggleRelay = (id: string) => { setRelays((current) => current.map((relay) => relay.id === id ? { ...relay, active: !relay.active } : relay)); const relay = relays.find((item) => item.id === id); setToast(`${relay?.label ?? "Relay"} ${relay?.active ? "switched off" : "switched on"}`); window.setTimeout(() => setToast(null), 2600); };

  return (
    <div className="app-shell">
      {mobileNavOpen && <button className="mobile-backdrop" aria-label="Close navigation" onClick={() => setMobileNavOpen(false)} />}
      <aside className={`sidebar ${mobileNavOpen ? "sidebar--open" : ""}`}>
        <div className="brand"><div className="brand-mark"><span /><span /><span /></div><div><strong>Hydro<span>Nexus</span></strong><small>IoT grow lab</small></div><button className="sidebar-close icon-button" onClick={() => setMobileNavOpen(false)} aria-label="Close menu"><X size={17} /></button></div>
        <div className="system-status"><div className="status-orb"><Activity size={16} /></div><div><span>System status</span><strong>All systems nominal</strong></div><span className="status-dot status-dot--mint" /></div>
        <nav className="sidebar-nav" aria-label="Primary navigation"><span className="nav-overline">Control room</span>{navItems.map((item) => <button key={item.key} className={`nav-item ${activeNav === item.key ? "nav-item--active" : ""}`} onClick={() => openView(item.key)}><item.icon size={17} /><span>{item.label}</span>{item.badge && <span className="nav-badge">{item.badge}</span>}{activeNav === item.key && <ChevronRight className="nav-arrow" size={15} />}</button>)}</nav>
        <div className="sidebar-plant"><div className="sidebar-plant__copy"><span className="eyebrow">Current cycle</span><strong>Lettuce · day 18</strong><span>Vegetative stage</span></div><div className="mini-leaf"><Leaf size={33} /></div><div className="cycle-bar"><span style={{ width: "64%" }} /></div><div className="cycle-meta"><span>Growth progress</span><span>64%</span></div></div>
        <div className="sidebar-footer"><button className="sidebar-link"><CircleHelp size={16} /> Docs & help</button><button className="sidebar-link"><Settings2 size={16} /> Settings</button><div className="profile"><div className="avatar">AS</div><div><strong>Alex Student</strong><span>Student workspace</span></div><MoreHorizontal size={17} /></div></div>
      </aside>
      <main className="main-content">
        <header className="topbar"><div className="topbar-left"><button className="mobile-menu icon-button" aria-label="Open navigation" onClick={() => setMobileNavOpen(true)}><Menu size={19} /></button><div className="breadcrumb"><span>Control room</span><ChevronRight size={14} /><strong>{navItems.find((item) => item.key === activeNav)?.label}</strong></div></div><div className="topbar-right"><div className="live-pulse"><span className="status-dot status-dot--mint" /> live <span className="divider" /> {lastUpdated}</div><button className="icon-button" aria-label="Refresh telemetry" onClick={() => setToast("Telemetry refreshed") }><RefreshCw size={16} /></button><button className="icon-button" aria-label="Open notifications"><span className="notification-dot" /><AlertTriangle size={17} /></button></div></header>
        {activeNav === "overview" && <div className="overview-view fade-in"><section className="page-heading overview-heading"><div><p className="eyebrow">Wednesday · 23 September 2026</p><h1>Good morning, Alex <span className="heading-sprout">✦</span></h1><p className="subheading">Your lettuce is looking strong. Here’s the pulse of the grow room.</p></div><div className="heading-actions"><button className="button button--ghost" onClick={() => setToast("Snapshot saved to local workspace")}><Save size={16} /> Save snapshot</button><button className="button button--primary" onClick={() => openView("code")}><Code2 size={16} /> View code <ArrowUpRight size={15} /></button></div></section>
          <section className="hero-panel"><div className="hero-copy"><div className="hero-kicker"><span className="status-dot status-dot--mint" /> live system snapshot <span>·</span> 08:42 AM</div><h2>Small signals.<br /><em>Better greens.</em></h2><p>Automated NFT circulation is stable. Water level is healthy and the canopy is in its ideal vegetative window.</p><div className="hero-stats"><div><strong>{activeCount}<small>/4</small></strong><span>relays active</span></div><div><strong>{telemetry.water}<small>%</small></strong><span>reservoir level</span></div><div><strong>18<small>d</small></strong><span>current cycle</span></div></div></div><PlantGraphic /><div className="hero-stamp"><Sprout size={15} /> vegetative window</div></section>
          <div className="section-label"><span>Environmental telemetry</span><span className="section-line" /><button onClick={() => setToast("Sensor readings are simulated in this preview")}><CircleHelp size={14} /> about readings</button></div>
          <section className="metrics-grid"><Gauge value={telemetry.temperature} min={18} max={32} color="amber" unit="°" label="Temperature" status="Ideal for lettuce" icon={Thermometer} /><Gauge value={telemetry.humidity} min={40} max={80} color="blue" unit="%" label="Humidity" status="In target range" icon={Cloud} /><Gauge value={telemetry.ph} min={5.0} max={7.0} color="mint" unit="pH" label="Nutrient pH" status="Perfect balance" icon={Beaker} /><Gauge value={telemetry.tds} min={400} max={1200} color="lavender" unit="ppm" label="TDS / nutrients" status="Healthy concentration" icon={Zap} /><Gauge value={telemetry.water} min={0} max={100} color="water" unit="%" label="Water level" status="Reservoir healthy" icon={Waves} /></section>
          <section className="lower-grid"><div className="trend-card panel"><div className="panel-heading"><div><span className="eyebrow">Past 60 minutes</span><h3>Temperature trend</h3></div><span className="trend-value"><strong>24.8°</strong><span><ArrowUpRight size={14} /> 0.6%</span></span></div><div className="trend-chart"><div className="chart-label chart-label--top">26°</div><div className="chart-label chart-label--bottom">22°</div><div className="chart-grid"><span /><span /><span /><span /></div><Sparkline /></div><div className="chart-axis"><span>07:42</span><span>08:00</span><span>08:20</span><span>08:42</span></div></div><div className="alerts-card panel"><div className="panel-heading"><div><span className="eyebrow">Attention queue</span><h3>System notes</h3></div><span className="count-badge count-badge--amber">1 watch</span></div><div className="alert-item alert-item--watch"><div className="alert-icon"><AlertTriangle size={16} /></div><div><strong>Reservoir fill soon</strong><span>Water level is at {telemetry.water}%. Refill before it reaches 35%.</span></div><button aria-label="Dismiss alert" onClick={() => setToast("Alert snoozed for 1 hour")}><X size={15} /></button></div><div className="alert-item"><div className="alert-icon alert-icon--mint"><Check size={16} /></div><div><strong>pH balance is stable</strong><span>Automatic dosing has been quiet for 2h 14m.</span></div></div></div></section>
          <section className="section-block"><div className="section-label"><span>Relay control</span><span className="section-line" /><span className="section-meta"><Power size={13} /> {activeCount} of 4 active</span></div><section className="relay-grid">{relays.map((relay) => <div className={`relay-card ${relay.active ? "relay-card--active" : ""}`} key={relay.id}><div className="relay-card__top"><div className={`relay-icon relay-icon--${relay.color}`}><relay.icon size={18} /></div><button className={`toggle ${relay.active ? "toggle--on" : ""}`} onClick={() => toggleRelay(relay.id)} aria-label={`Toggle ${relay.label}`}><span /></button></div><div className="relay-card__copy"><strong>{relay.label}</strong><span>{relay.detail}</span></div><div className="relay-card__bottom"><span className={`relay-state ${relay.active ? "relay-state--on" : ""}`}><span className="status-dot" /> {relay.active ? "Running" : "Off"}</span><button className={`auto-chip ${relay.auto ? "auto-chip--on" : ""}`} onClick={() => setToast(`${relay.label} automation ${relay.auto ? "paused" : "enabled"}`)}>{relay.auto ? "Auto" : "Manual"}</button></div></div>)}</section></section>
        </div>}
        {activeNav === "code" && <CodeLab selectedFile={selectedFile} onSelect={setSelectedFile} />}
        {activeNav === "hardware" && <HardwareMap relays={relays} />}
        {activeNav === "automation" && <section className="automation-view fade-in"><div className="page-heading"><div><p className="eyebrow">Rule engine</p><h1>Automation <span className="heading-note">/ if this, then grow</span></h1><p className="subheading">The quiet logic that keeps your lettuce in its comfort zone.</p></div><button className="button button--primary" onClick={() => setToast("Rule builder is ready for your next rule")}><Zap size={16} /> New rule</button></div><div className="rules-list"><div className="rule-card"><div className="rule-number">01</div><div className="rule-copy"><span className="rule-when">WHEN <strong>pH</strong> falls below <strong>5.5</strong></span><span className="rule-then">THEN run <strong>pH up dosing</strong> for 8 seconds</span></div><span className="rule-status"><span className="status-dot status-dot--mint" /> active</span><MoreHorizontal size={17} /></div><div className="rule-card"><div className="rule-number">02</div><div className="rule-copy"><span className="rule-when">WHEN <strong>temperature</strong> rises above <strong>35°C</strong></span><span className="rule-then">THEN switch on <strong>climate fan</strong></span></div><span className="rule-status"><span className="status-dot status-dot--mint" /> active</span><MoreHorizontal size={17} /></div><div className="rule-card"><div className="rule-number">03</div><div className="rule-copy"><span className="rule-when">WHEN <strong>water level</strong> falls below <strong>35%</strong></span><span className="rule-then">THEN send <strong>low reservoir alert</strong></span></div><span className="rule-status"><span className="status-dot status-dot--amber" /> watch</span><MoreHorizontal size={17} /></div></div><div className="automation-tip"><div className="tip-icon"><Sprout size={20} /></div><div><strong>Good automation is boring.</strong><span>These rules are defined in <code>firmware/main.ino</code> and can be tuned before you flash the board.</span></div><button className="button button--ghost" onClick={() => openView("code")}>Inspect firmware <ArrowUpRight size={15} /></button></div></section>}
      </main>
      {toast && <div className="toast"><Check size={15} /> {toast}</div>}
    </div>
  );
}
