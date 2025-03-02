// Project64 JavaScript Tracker Script (Mirrors IronMarioTracker.lua)

const net = require('net');
const { ipcRenderer } = require('electron');
const { app, BrowserWindow, ipcMain } = require('electron');
let mainWindow;

// Memory address configuration
const CONFIG = {
    MEM: {
        MARIO_BASE: 0x1a0340,
        HUD_BASE: 0x1a0330,
        CURRENT_LEVEL_ID: 0x18fd78,
        CURRENT_SEED: 0x1cdf80,
        DELAYED_WARP_OP: 0x1a031c,
        INTENDED_LEVEL_ID: 0x19f0cc,
        CURRENT_SONG_ID: 0x19485e
    },
    USER: {
        ATTEMPTS: 0,
        PB_STARS: 0
    }
};

CONFIG.MEM.MARIO = {
    INPUT: CONFIG.MEM.MARIO_BASE + 0x2,
    ACTION: CONFIG.MEM.MARIO_BASE + 0xC,
    POS: CONFIG.MEM.MARIO_BASE + 0x3C,
    HURT_COUNTER: CONFIG.MEM.MARIO_BASE + 0xB2
};

CONFIG.MEM.HUD = {
    STARS: CONFIG.MEM.HUD_BASE + 0x4,
    HEALTH: CONFIG.MEM.HUD_BASE + 0x6
};

// State tracking
let state = {
    mario: {},
    game: {},
    run: {
        status: 0,
        stars: 0,
        warp_map: {},
        star_map: {}
    }
};

function updateState() {
    state.game.level_id = mem.u16[CONFIG.MEM.CURRENT_LEVEL_ID];
    state.mario.action = mem.u32[CONFIG.MEM.MARIO.ACTION];
    state.mario.hp = mem.u16[CONFIG.MEM.HUD.HEALTH];
    state.run.stars = mem.u16[CONFIG.MEM.HUD.STARS];
    
    let pos = [
        mem.f32[CONFIG.MEM.MARIO.POS],
        mem.f32[CONFIG.MEM.MARIO.POS + 4],
        mem.f32[CONFIG.MEM.MARIO.POS + 8]
    ];
    state.mario.pos = { x: pos[0], y: pos[1], z: pos[2] };
}

setInterval(updateState, 1000);

// Socket connection
var server = new Server();
server.listen(1337, "127.0.0.1");

server.on('connection', function(connection) {
    const outputer = setInterval(() => {
        connection.write(JSON.stringify(state));
    }, 1000);
    connection.on('close', () => { clearInterval(outputer); });
});

// Electron client to receive data and render overlay
const client = net.createConnection({ port: 1337 }, () => {
    console.log('Connected to server');
});

client.on('data', (data) => {
    ipcRenderer.send('gameStateUpdate', JSON.parse(data));
});

client.on('end', () => {
    console.log('Disconnected from server');
});

// Create Electron window
app.whenReady().then(() => {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        transparent: true,
        frame: false,
        alwaysOnTop: true,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false
        }
    });
    mainWindow.loadURL(`file://${__dirname}/overlay.html`);
});

// Handle game state updates in renderer process
ipcMain.on('gameStateUpdate', (event, newState) => {
    if (mainWindow) {
        mainWindow.webContents.send('updateOverlay', newState);
    }
});
