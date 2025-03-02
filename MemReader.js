const ffi = require("ffi-napi");
const ref = require("ref-napi");
const { BrowserWindow, app, ipcMain } = require("electron");

// Windows API functions
const kernel32 = ffi.Library("kernel32", {
  OpenProcess: ["pointer", ["uint32", "int", "uint32"]],
  ReadProcessMemory: ["bool", ["pointer", "uint32", "pointer", "uint32", "pointer"]],
  CloseHandle: ["bool", ["pointer"]],
  GetWindowRect: ["bool", ["pointer", "pointer"]]
});

// Process access rights
const PROCESS_VM_READ = 0x0010;
const PROCESS_QUERY_INFORMATION = 0x0400;
const processName = "Project64.exe";

// Memory addresses
const MEM_ADDRESSES = {
   MARIO_BASE: 0x1a0340,
        HUD_BASE: 0x1a0330,
        CURRENT_LEVEL_ID: 0x18fd78,
        CURRENT_SEED: 0x1cdf80,
        DELAYED_WARP_OP: 0x1a031c,
        INTENDED_LEVEL_ID: 0x19f0cc,
        CURRENT_SONG_ID: 0x19485e
  
};
const CONFIG.MEM.MARIO = {
    INPUT: CONFIG.MEM.MARIO_BASE + 0x2,
    ACTION: CONFIG.MEM.MARIO_BASE + 0xC,
    POS: CONFIG.MEM.MARIO_BASE + 0x3C,
    HURT_COUNTER: CONFIG.MEM.MARIO_BASE + 0xB2
};

const CONFIG.MEM.HUD = {
    STARS: CONFIG.MEM.HUD_BASE + 0x4,
    HEALTH: CONFIG.MEM.HUD_BASE + 0x6
};

const float = ref.types.float;
const int32 = ref.types.int32;

// Function to find Project64 process ID
const getProcessId = () => {
  const execSync = require("child_process").execSync;
  try {
    const output = execSync(`tasklist /FI "IMAGENAME eq ${processName}" /FO CSV /NH`).toString();
    const pid = output.split(",")[1].replace(/"/g, "").trim();
    return parseInt(pid, 10);
  } catch (err) {
    return null;
  }
};

// Read memory function
const readMemory = (processHandle, address, type) => {
  const buffer = Buffer.alloc(type.size);
  const bytesRead = ref.alloc("uint32");

  const success = kernel32.ReadProcessMemory(processHandle, address, buffer, type.size, bytesRead);
  if (!success) return null;

  return type.get(buffer, 0);
};

// Read game data
const readGameState = () => {
  const pid = getProcessId();
  if (!pid) return null;

  const processHandle = kernel32.OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, 0, pid);
  if (!processHandle) return null;

  const gameState = {
    stars: readMemory(processHandle, CONFIG.MEM.HUD_BASE + 0x4, int32),
    levelId: readMemory(processHandle, MEM_ADDRESSES.CURRENT_LEVEL_ID, int32),
    marioPos: {
      x: readMemory(processHandle, CONFIG.MEM.MARIO_BASE + 0x3C, float),
      y: readMemory(processHandle, CONFIG.MEM.MARIO_BASE + 0x3C + 4, float),
      z: readMemory(processHandle, CONFIG.MEM.MARIO_BASE + 0x3C + 8, float)
    }
  };

  kernel32.CloseHandle(processHandle);
  return gameState;
};

// Send memory data to the Electron frontend
setInterval(() => {
  const gameState = readGameState();
  if (gameState) overlayWindow.webContents.send("updateData", gameState);
}, 1000);

// Create the overlay window
let overlayWindow;
app.whenReady().then(() => {
  overlayWindow = new BrowserWindow({
    width: 400,
    height: 200,
    transparent: true,
    frame: false,
    alwaysOnTop: true,
    skipTaskbar: true,
    focusable: false,
    webPreferences: {
      nodeIntegration: true
    }
  });

  overlayWindow.setIgnoreMouseEvents(true);
  overlayWindow.loadURL(`file://${__dirname}/index.html`);
});
