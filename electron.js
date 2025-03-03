var { app, BrowserWindow, ipcMain } = require('electron');
var net = require('net');
var EventEmitter = require('events');

class DataEmitter extends EventEmitter {}
var dataEmitter = new DataEmitter();

var mainWindow;

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 400,
        height: 200,
        transparent: true,
        frame: false,
        alwaysOnTop: true,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false
        }
    });
    
    mainWindow.loadFile('Overlay.html');
}

app.whenReady().then(function () {
    createWindow();
    setupTCPServer();

    app.on('activate', function () {
        if (BrowserWindow.getAllWindows().length === 0) {
            createWindow();
        }
    });
});

function setupTCPServer() {
    var server = net.createServer(function (socket) {
        socket.on('data', function (data) {
            var message = data.toString();
            dataEmitter.emit('update', message);
            if (mainWindow) {
                mainWindow.webContents.send('data-update', message);
            }
        });
    });

    server.listen(8080, '127.0.0.1', function () {
        console.log('TCP server listening on port 8080');
    });
}

ipcMain.on('request-update', function (event) {
    dataEmitter.once('update', function (data) {
        event.reply('data-update', data);
    });
});

// Handle game state updates in renderer process
ipcMain.on('gameStateUpdate', (event, newState) => {
    if (mainWindow) {
        mainWindow.webContents.send('updateOverlay', newState);
    }
}); 
