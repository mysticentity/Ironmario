const net = require('net');
const { ipcRenderer } = require('electron');
const { app, BrowserWindow, ipcMain } = require('electron');
let mainWindow;

// Memory address configuration
const CONFIG = {
    MEM: {
@@ -72,39 +67,3 @@ server.on('connection', function(connection) {
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
