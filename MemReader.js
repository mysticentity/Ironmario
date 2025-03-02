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
  STARS: 0x1A0334,
  LEVEL_ID: 0x18FD78,
  MARIO_POS: 0x1A037C
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
    stars: readMemory(processHandle, MEM_ADDRESSES.STARS, int32),
    levelId: readMemory(processHandle, MEM_ADDRESSES.LEVEL_ID, int32),
    marioPos: {
      x: readMemory(processHandle, MEM_ADDRESSES.MARIO_POS, float),
      y: readMemory(processHandle, MEM_ADDRESSES.MARIO_POS + 4, float),
      z: readMemory(processHandle, MEM_ADDRESSES.MARIO_POS + 8, float)
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
