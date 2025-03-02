const { ipcRenderer } = require("electron");

// Update overlay with live data
ipcRenderer.on("updateData", (event, gameState) => {
  document.getElementById("stars").textContent = `Stars: ${gameState.stars}`;
  document.getElementById("level").textContent = `Level ID: ${gameState.levelId}`;
  document.getElementById("marioPos").textContent = `Mario Position: X=${gameState.marioPos.x}, Y=${gameState.marioPos.y}, Z=${gameState.marioPos.z}`;
});
