// Project64 JavaScript Tracker Script (Mirrors IronMarioTracker.lua)

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

console.log(JSON.stringify(CONFIG));

console.log('block', JSON.stringify([mem.getblock(CONFIG.MEM.CURRENT_SONG_ID, 4)].toString()));

events.onwrite(CONFIG.MEM.MARIO_BASE + 0x3C, console.log)

mem.bindvar(this, mem.u16[CONFIG.MEM.CURRENT_LEVEL_ID], 'level_id', u16);
mem.bindvar(this, mem.u16[CONFIG.MEM.MARIO.POS], 'pos_1', float);


// State tracking
var state = {
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
    state.game.level_id = level_id //mem.u16[CONFIG.MEM.CURRENT_LEVEL_ID];
    

    state.mario.action = mem.u32[CONFIG.MEM.MARIO.ACTION];
    state.mario.hp = mem.u16[CONFIG.MEM.HUD.HEALTH];
    state.run.stars = mem.u16[CONFIG.MEM.HUD.STARS];
    
    var pos = [
        mem.u32[CONFIG.MEM.MARIO.POS],
        mem.u32[CONFIG.MEM.MARIO.POS + 4],
        mem.u32[CONFIG.MEM.MARIO.POS + 8]
    ];
    state.mario.pos = [pos_1];
    
}





var server = new Server(1337);

server.on('connection', function(connection) {
    
    connection.on('data', function(data) {
        updateState();
        connection.write(JSON.stringify(state));
        console.log('pos_1', pos_1);
    });
});

server.listen(1337);



var info = Object.keys(this).map(function name(key) {
    return  key+' \r\n'
});

//console.log('Everything is defined'  , info)

// setInterval(updateState, 1000);
// startElectronOverlay();
//server.listen(1337, "127.0.0.1");
