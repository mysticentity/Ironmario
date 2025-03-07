-- IronMario Tracker Script
-- This script tracks various aspects of a run (attempt count, stars, warp mapping, etc.) by reading emulator memory,
-- logging data to files, and rendering an on-screen overlay.
-- It uses lunajson for JSON encoding and pl.tablex for deep table functions.
local json = require("lib.lunajson") -- JSON encoding/decoding library
local tablex = require("lib.pl.tablex") -- Extended table functions (e.g., deepcopy, deepcompare)
local imgui = require("imgui")

-- Main configuration table that holds version info, file paths, memory addresses, and user data.
local CONFIG = {
    TRACKER_VERSION = '1.1.1',
    FONT_FACE = 'Lucida Console',
    SHOW_SONG_TITLE = false, -- Flag to toggle song title display on the UI.
    FILES = {
        ATTEMPT_COUNT = 'usr/attempts.txt', -- File to record total attempt count.
        ATTEMPT_DATA = 'usr/attempts_data.csv', -- CSV file for detailed run attempt data.
        PB_COUNT = 'usr/pb_stars.txt', -- File for storing the personal best (PB) star count.
        SONG_INFO = 'usr/song_info.txt', -- File for storing song info. (Location, Title)
        WARP_LOG = 'usr/warp_log.json' -- File to log warp map data as JSON.
    },
    MEM = {
        MARIO_BASE = 0x1a0340, -- Base memory address for Mario-related data.
        HUD_BASE = 0x1a0330, -- Base memory address for HUD elements.
        CURRENT_LEVEL_ID = 0x18fd78, -- Address for the current level ID.
        CURRENT_SEED = 0x1cdf80, -- Address for the current run's seed.
        DELAYED_WARP_OP = 0x1a031c, -- Address for delayed warp operation code.
        INTENDED_LEVEL_ID = 0x19f0cc, -- Address for the intended level after a warp.
        CURRENT_SONG_ID = 0x0a3781 -- Address for the current song ID.
    },
    USER = {
        ATTEMPTS = 0, -- Total number of attempts (will be updated from file).
        PB_STARS = 0 -- Personal best star count (will be updated from file).
    },
    BACKGROUND_IMAGE = "(None)" -- Default background image for the UI.
}

-- Additional memory addresses for Mario-specific data derived from MARIO_BASE.
CONFIG.MEM.MARIO = {
    INPUT = CONFIG.MEM.MARIO_BASE + 0x2, -- Address for Mario's input flags or status.
    ACTION = CONFIG.MEM.MARIO_BASE + 0xC, -- Address for Mario's current action/state.
    POS = CONFIG.MEM.MARIO_BASE + 0x3C, -- Address for Mario's 3D position (stored as floats).
    HURT_COUNTER = CONFIG.MEM.MARIO_BASE + 0xB2 -- Address for a counter indicating recent damage.
}

-- Memory addresses for HUD-related data.
CONFIG.MEM.HUD = {
    STARS = CONFIG.MEM.HUD_BASE + 0x4, -- Address for the number of stars displayed.
    HEALTH = CONFIG.MEM.HUD_BASE + 0x6 -- Address for Mario's health.
}

-- Level data configuration including level names and abbreviations.
-- Note: There are duplicate keys (e.g., several entries for key 3626007); only the last assignment will persist.
CONFIG.LEVEL_DATA = {
    HAS_NO_WATER = {9, 24, 4, 22, 8, 14, 15, 27, 31, 29, 18, 17, 30, 19}, -- Currently unused.
    LOCATION_MAP = {
        [0] = {"", ""},
        [1] = {"Menu", "Menu"},
        [10] = {"Snowman's Land", "SL"},
        [11] = {"Wet Dry World", "WDW"},
        [12] = {"Jolly Roger Bay", "JRB"},
        [13] = {"Tiny Huge Island", "THI"},
        [14] = {"Tick Tock Clock", "TTC"},
        [15] = {"Rainbow Ride", "RR"},
        [16] = {"Outside Castle", "Outside"},
        [17] = {"Bowser in the Dark World", "BitDW"},
        [18] = {"Vanish Cap Under the Moat", "Vanish"},
        [19] = {"Bowser in the Fire Sea", "BitFS"},
        [20] = {"Secret Aquarium", "SA"},
        [22] = {"Lethal Lava Land", "LLL"},
        [23] = {"Dire Dire Docks", "DDD"},
        [24] = {"Whomp's Fortress", "WF"},
        [26] = {"Garden", "Garden"},
        [27] = {"Peach's Slide", "PSS"},
        [28] = {"Cavern of the Metal Cap", "Metal"},
        [29] = {"Tower of the Wing Cap", "Wing"},
        [30] = {"Bowser Fight 1", "Bowser1"},
        [31] = {"Wing Mario Over the Rainbow", "WMotR"},
        [36] = {"Tall Tall Mountain", "TTM"},
        -- Duplicate keys: Only the last assignment for key 3626007 will be used.
        -- [3626007] = {"Basement", "B1F"},
        -- [3626007] = {"Second Floor", "2F"},
        -- [3626007] = {"Third Floor", "3F"},
        [3626007] = {"Bowser in the Sky", "BitS"},
        [4] = {"Big Boo's Haunt", "BBH"},
        [5] = {"Cool Cool Mountain", "CCM"},
        [6] = {"Castle", "Castle"},
        [7] = {"Hazy Maze Cave", "HMC"},
        [8] = {"Shifting Sand Land", "SSL"},
        [9] = {"Bob-Omb Battlefield", "BoB"}
    }
}

CONFIG.MUSIC_DATA = {
    SONG_MAP = {
        [12] = "Super Mario 64 - Endless Staircase",
        [13] = "Super Mario 64 - Merry-Go-Round",
        [14] = "Super Mario 64 - Title Theme",
        [15] = "Super Mario 64 - Bob-omb Battlefield",
        [16] = "Super Mario 64 - Inside the Castle Walls",
        [17] = "Super Mario 64 - Dire, Dire Docks",
        [18] = "Super Mario 64 - Lethal Lava Land",
        [19] = "Super Mario 64 - Koopa's Theme",
        [20] = "Super Mario 64 - Snow Mountain",
        [21] = "Super Mario 64 - Slider",
        [22] = "Super Mario 64 - Haunted House",
        [23] = "Super Mario 64 - Piranha Plant's Lullaby",
        [24] = "Super Mario 64 - Cave Dungeon",
        [25] = "Super Mario 64 - Powerful Mario",
        [26] = "Super Mario 64 - Metallic Mario",
        [27] = "Super Mario 64 - Koopa's Road",
        [28] = "Race Fanfare",
        [29] = "Super Mario 64 - Stage Boss",
        [30] = "Super Mario 64 - Ultimate Koopa",
        [31] = "Super Mario 64 - File Select",
        [32] = "Super Mario 64 - Powerful Mario",
        [33] = "Super Mario 64 - Title Theme",
        [34] = "Bomberman 64 - Green Garden",
        [35] = "Bomberman 64 - Blue Resort",
        [36] = "Bomberman Hero - Redial",
        [37] = "Wii Shop Channel",
        [38] = "Chrono Trigger - Delightful Spekkio",
        [39] = "Castlevania: Order of Ecclesia - A Prologue",
        [40] = "Diddy Kong Racing - Darkmoon Caverns",
        [41] = "Diddy Kong Racing - Frosty Village",
        [42] = "Diddy Kong Racing - Spacedust Alley, Star City",
        [43] = "Donkey Kong Country - Aquatic Ambience",
        [44] = "Donkey Kong Country 2 - Forest Interlude",
        [45] = "Donkey Kong Country 2 - Stickerbrush Symphony",
        [46] = "Diddy Kong Racing - Greenwood Village",
        [47] = "Donkey Kong Country 2 - In a Snow-Bound Land",
        [48] = "Earthbound - Home Sweet Home",
        [49] = "Earthbound - Onett",
        [50] = "TLoZ: Ocarina of Time - Gerudo Valley",
        [51] = "Pokemon Shuffle - Stage (Hard)",
        [52] = "Banjo-Kazooie - Gruntilda's Lair",
        [53] = "Kirby: Nightmare in Dream Land - Butter Building",
        [54] = "Kirby 64: The Crystal Shards - Shiver Star",
        [55] = "Kirby's Adventure - Yogurt Yard",
        [56] = "Kirby Super Star - Mine Cart Riding",
        [57] = "TLoZ: Majora's Mask - Clock Town Day 1",
        [58] = "Mario & Luigi: Partners in Time - Thwomp Caverns",
        [59] = "Mario Kart 8 - Rainbow Road",
        [60] = "Mario Kart 64 - Koopa Beach",
        [61] = "Mario Kart Wii - Maple Treeway",
        [62] = "Mega Man 3 - Spark Man Stage",
        [63] = "Mega Man Battle Network 5 - Hero Theme",
        [64] = "Mario Kart 64 - Moo Moo Farm, Yoshi Valley",
        [65] = "New Super Mario Bros. - Athletic Theme",
        [66] = "New Super Mario Bros. - Desert Theme",
        [67] = "New Super Mario Bros. U - Overworld",
        [68] = "New Super Mario Bros. Wii - Forest",
        [69] = "TLoZ: Ocarina of Time - Lost Woods",
        [70] = "Pilotwings - Light Plane",
        [71] = "Pokemon Diamond and Pearl - Eterna Forest",
        [72] = "Pokemon HeartGold and SoulSilver - Lavender Town",
        [73] = "Mario Party - Mario's Rainbow Castle",
        [74] = "Bomberman 64 - Red Mountain",
        [75] = "Deltarune - Rude Buster",
        [76] = "Super Mario 3D World - Overworld",
        [77] = "Super Mario Sunshine - Sky and Sea",
        [78] = "Snowboard Kids - Big Snowman",
        [79] = "Sonic Adventure - Emerald Coast",
        [80] = "Sonic the Hedgehog - Green Hill Zone",
        [81] = "Super Castlevania 4 - The Submerged City",
        [82] = "Super Mario Land - Birabuto Kingdom",
        [83] = "Super Mario RPG - Beware the Forest's Mushrooms",
        [84] = "Super Mario Sunshine - Delfino Plaza",
        [85] = "Super Mario Sunshine - Gelato Beach",
        [86] = "Yoshi's Island - Crystal Caves",
        [87] = "TLoZ: Ocarina of Time - Water Temple",
        [88] = "Wave Race 64 - Sunny Beach",
        [89] = "Whomp's Floating Habitat (Original by MariosHub)",
        [90] = "TLoZ: Ocarina of Time - Kokiri Forest",
        [91] = "TLoZ: Ocarina of Time - Zora's Domain",
        [92] = "TLoZ: Ocarina of Time - Kakariko Village",
        [93] = "A Morning Jog (Original by TheGael95)",
        [94] = "TLoZ: The Wind Waker - Outset Island",
        [95] = "Super Paper Mario - Flipside",
        [96] = "Super Mario Galaxy - Ghostly Galaxy",
        [97] = "Super Mario RPG - Nimbus Land",
        [98] = "Super Mario Galaxy - Battlerock Galaxy",
        [99] = "Sonic Adventure - Windy Hill",
        [100] = "Super Paper Mario - The Overthere Stair",
        [101] = "Super Mario Sunshine - Secret Course",
        [102] = "Super Mario Sunshine - Bianco Hills",
        [103] = "Super Paper Mario - Lineland Road",
        [104] = "Paper Mario: The Thousand-Year Door - X-Naut Fortress",
        [105] = "Mario and Luigi: Bowser's Inside Story - Bumpsy Plains",
        [106] = "Super Mario World - Athletic Theme",
        [107] = "TLoZ: Skyward Sword - Skyloft",
        [108] = "Super Mario World - Castle",
        [109] = "Super Mario Galaxy - Comet Observatory",
        [110] = "Banjo-Kazooie - Freezeezy Peak",
        [111] = "Mario Kart DS - Waluigi Pinball",
        [112] = "Kirby 64: The Crystal Shards - Factory Inspection",
        [113] = "Donkey Kong 64 - Creepy Castle",
        [114] = "Paper Mario 64 - Forever Forest",
        [115] = "Super Mario Bros. - Bowser's Theme (Remix)",
        [116] = "TLoZ: Twilight Princess - Gerudo Desert",
        [117] = "Yoshi's Island - Overworld",
        [118] = "Mario and Luigi: Partners in Time - Gritzy Desert",
        [119] = "Donkey Kong 64 - Angry Aztec",
        [120] = "Mario and Luigi: Partners in Time - Yoshi's Village",
        [121] = "Touhou 10: Mountain of Faith - Youkai Mountain",
        [122] = "Mario and Luigi: Bowser's Inside Story - Deep Castle",
        [123] = "Paper Mario: The Thousand-Year Door - Petal Meadows",
        [124] = "Mario Party - Yoshi's Tropical Island",
        [125] = "Super Mario 3D World - Piranha Creek",
        [126] = "Final Fantasy VII - Forested Temple",
        [127] = "Paper Mario 64 - Dry Dry Desert",
        [128] = "Rayman - Band Land",
        [129] = "Donkey Kong 64 - Hideout Helm",
        [130] = "Donkey Kong 64 - Frantic Factory",
        [131] = "Super Paper Mario - Sammer's Kingdom",
        [132] = "Super Mario Galaxy - Purple Comet",
        [133] = "TLoZ: Majora's Mask - Stone Tower Temple",
        [134] = "Banjo-Kazooie - Bubblegloop Swamp",
        [135] = "Banjo-Kazooie - Gobi's Valley",
        [136] = "Bomberman 64 - Black Fortress",
        [137] = "Donkey Kong 64 - Fungi Forest",
        [138] = "Paper Mario: The Thousand-Year Door - Riddle Tower",
        [139] = "Paper Mario: The Thousand-Year Door - Rogueport Sewers",
        [140] = "Super Mario Galaxy 2 - Honeybloom Galaxy",
        [141] = "Pokemon Mystery Dungeon - Sky Tower",
        [142] = "Super Mario Bros. 3 - Overworld",
        [143] = "Super Mario RPG - Mario's Pad",
        [144] = "Super Mario RPG - Sunken Ship",
        [145] = "Super Mario Galaxy - Buoy Base Galaxy",
        [146] = "Donkey Kong 64 - Crystal Caves",
        [147] = "Super Paper Mario - Floro Caverns",
        [148] = "Yoshi's Story - Title Theme",
        [149] = "TLoZ: Twilight Princess - Lake Hylia",
        [150] = "Mario Kart 64 - Frappe Snowland",
        [151] = "Donkey Kong 64 - Gloomy Galleon",
        [152] = "Mario Kart 64 - Bowser's Castle",
        [153] = "Mario Kart 64 - Rainbow Road",
        [154] = "Banjo-Kazooie - Mumbo's Mountain",
        [155] = "Donkey Kong Country 2 - Jib Jig",
        [156] = "Donkey Kong Country 2 - Welcome to Crocodile Isle",
        [157] = "TLoZ: The Wind Waker - Dragon Roost Island",
        [158] = "Pokemon Black and White - Accumula Town",
        [159] = "Pokemon HeartGold and SoulSilver - Vermilion City",
        [160] = "Undertale - Snowdin Town",
        [161] = "Undertale - Bonetrousle",
        [162] = "Undertale - Death by Glamour",
        [163] = "Undertale - Home",
        [164] = "Undertale - Ruins",
        [165] = "Undertale - Spider Dance",
        [166] = "Undertale - Waterfall"
    }
}

memory.usememorydomain("ROM")

local VALID_ROM_VERSION = nil

local rom_signature_bytes = memory.read_bytes_as_array(0x20, 12) -- Read 6 bytes from ROM.
local rom_signature = string.char(table.unpack(rom_signature_bytes)) -- Convert bytes to string.

if rom_signature == "IronMario 64" then
    VALID_ROM_VERSION = true
else
    VALID_ROM_VERSION = false
end

-- Define possible run states.
local run_state = {
    INACTIVE = 0, -- Run has not started.
    ACTIVE = 1, -- Run is in progress.
    PENDING = 2, -- Run has ended; data pending write.
    COMPLETE = 3 -- Run data has been fully processed.
}

-- Main state table that stores runtime data for input, Mario, run metrics, and game info.
local state = {
    input = {
        music_toggle_pressed = false -- Flag to track toggling of song title display.
    },
    mario = {}, -- Will hold Mario's position, velocity, health, etc.
    run = {
        status = run_state.INACTIVE, -- Current run state.
        stars = 0, -- Total stars collected during the run.
        warp_map = {}, -- Map of intended warp destinations to actual warp outcomes.
        star_map = {}, -- Mapping of levels to star counts collected.
        start_time = os.time(), -- Timestamp for when the run started.
        last_updated_time = os.time(), -- Last time the state was updated.
        end_time = os.time() -- Timestamp for when the run ended (initially same as start).
    },
    game = {
        level_id = 1 -- Current level ID; default value.
    }
}

local BACKGROUND_IMAGES = {"(None)", "Cave", "City", "Desert", "Fire", "Forest", "Mountains", "Ocean", "Pattern", "Sky",
                           "Storm"}

-- Table to store the previous state (for change detection in UI rendering).
local last_state = {}

local config_form = nil -- Placeholder for the configuration form.

-- Initialize the attempt data file if it doesn't exist by writing a CSV header.
local function init_attempt_data_file()
    local file = io.open(CONFIG.FILES.ATTEMPT_DATA, "r")
    if file then
        file:close() -- File exists, so do nothing.
        return
    end

    file = io.open(CONFIG.FILES.ATTEMPT_DATA, "w")
    if file then
        file:write("AttemptNumber,SeedKey,TimeStamp,Stars,TimeTaken,EndLevel,EndCause,StarsCollected\n")
        file:close()
    end
end

init_attempt_data_file() -- Call the initialization function.

-- Read the attempt count from file; if file is missing or empty, default to 0.
local function read_attempts_file()
    local file = io.open(CONFIG.FILES.ATTEMPT_COUNT, "r")
    if file then
        CONFIG.USER.ATTEMPTS = tonumber(file:read("*all"))
        file:close()
    else
        CONFIG.USER.ATTEMPTS = 0
    end
end

-- Read the personal best stars count from file.
local function read_pb_stars_file()
    local file = io.open(CONFIG.FILES.PB_COUNT, "r")
    if file then
        CONFIG.USER.PB_STARS = tonumber(file:read("*all"))
        file:close()
    else
        CONFIG.USER.PB_STARS = 0
    end
end

-- Write the current attempt count to its file.
local function write_attempts_file()
    local file = io.open(CONFIG.FILES.ATTEMPT_COUNT, "w")
    if file then
        file:write(CONFIG.USER.ATTEMPTS)
        file:close()
    end
end

-- Write the personal best stars count to file.
local function write_pb_stars_file()
    local file = io.open(CONFIG.FILES.PB_COUNT, "w")
    if file then
        file:write(CONFIG.USER.PB_STARS)
        file:close()
    end
end

-- Retrieve the full song name based on the song ID.
local function get_song_name(song_id)
    local song_info = CONFIG.MUSIC_DATA.SONG_MAP[song_id]
    if song_info then
        return song_info
    end
    return "Unknown"
end

-- Utility function to read three consecutive floats from a given memory address.
local function read3float(base)
    local arr = {}
    for i = 1, 3 do
        arr[i] = memory.readfloat(base + 4 * (i - 1), true)
    end
    return arr
end

-- Get the full level name based on the level ID using LOCATION_MAP.
local function get_level_name(level_id)
    if CONFIG.LEVEL_DATA.LOCATION_MAP[level_id] then
        return CONFIG.LEVEL_DATA.LOCATION_MAP[level_id][1]
    else
        return "Unknown"
    end
end

-- Get the abbreviated level name based on the level ID.
local function get_level_abbr(level_id)
    if CONFIG.LEVEL_DATA.LOCATION_MAP[level_id] then
        return CONFIG.LEVEL_DATA.LOCATION_MAP[level_id][2]
    else
        return "Unknown"
    end
end

-- Update the game state by reading from memory and updating our state tables.
local function update_game_state()
    last_state = tablex.deepcopy(state) -- Store the previous state for later comparison.

    state.last_updated_time = os.time() -- Update the timestamp.
    state.game.delayed_warp_op = memory.read_u16_be(CONFIG.MEM.DELAYED_WARP_OP)
    state.game.intended_level_id = memory.read_u32_be(CONFIG.MEM.INTENDED_LEVEL_ID)
    state.game.level_id = memory.read_u16_be(CONFIG.MEM.CURRENT_LEVEL_ID)
    state.game.song = memory.readbyte(CONFIG.MEM.CURRENT_SONG_ID)
    state.mario.action = memory.read_u32_be(CONFIG.MEM.MARIO.ACTION)
    state.mario.flags = memory.read_u32_be(CONFIG.MEM.MARIO.INPUT) -- Read flags from the same address.
    state.mario.hp = memory.read_u16_be(CONFIG.MEM.HUD.HEALTH)
    state.mario.input = memory.read_u16_be(CONFIG.MEM.MARIO.INPUT) -- Duplicate read; ensure the correct width.
    state.run.seed = memory.read_u32_be(CONFIG.MEM.CURRENT_SEED)
    state.run.stars = memory.read_u16_be(CONFIG.MEM.HUD.STARS)

    -- Read Mario's 3D position from memory.
    local pos_data = read3float(CONFIG.MEM.MARIO.POS)
    state.mario.pos = {
        x = pos_data[1],
        y = pos_data[2],
        z = pos_data[3]
    }

    -- Calculate Mario's velocity based on change in position from the previous state.
    if last_state.mario.pos then
        state.mario.velocity = {
            x = state.mario.pos.x - last_state.mario.pos.x,
            y = state.mario.pos.y - last_state.mario.pos.y,
            z = state.mario.pos.z - last_state.mario.pos.z
        }
    else
        state.mario.velocity = {
            x = 0,
            y = 0,
            z = 0
        }
    end

    -- Determine Mario's status (e.g., in water, taking gas damage, intangible).
    state.mario.is_in_water = ((state.mario.action & 0xC0) == 0xC0)
    state.mario.is_in_gas = ((state.mario.input & 0x100) == 0x100)
    state.mario.is_intangible = ((state.mario.action & 0x1000) == 0x1000)
    state.mario.has_metal_cap = ((state.mario.flags & 0x4) == 0x4)
    state.mario.is_taking_gas_damage = (state.mario.is_in_gas and not state.mario.is_intangible and
                                           not state.mario.has_metal_cap)

    -- Read the hurt counter to detect if Mario has taken damage.
    state.mario.hurt_counter = memory.readbyte(CONFIG.MEM.MARIO.HURT_COUNTER)

    -- Retrieve abbreviated level names for current and intended levels.
    local level_abbr = get_level_abbr(state.game.level_id)
    local intended_level_abbr = get_level_abbr(state.game.intended_level_id)

    -- Update the warp map if not already set, provided that the intended level is valid
    -- and the current level is not one of the excluded ones (levels 6 and 16).
    if not state.run.warp_map[intended_level_abbr] and state.game.intended_level_id ~= 0 and state.game.level_id ~= 6 and
        state.game.level_id ~= 16 then
        state.run.warp_map[intended_level_abbr] = level_abbr
    end

    -- If stars have increased since the last state update, record the star collection per level.
    if state.run.stars > last_state.run.stars then
        if not state.run.star_map[level_abbr] then
            state.run.star_map[level_abbr] = 0
        end
        state.run.star_map[level_abbr] = state.run.star_map[level_abbr] + 1
    end

    -- If the level indicates a new run (level_id 16) and the run is not already active, initialize a new run.
    if state.game.level_id == 16 and state.run.status == run_state.INACTIVE then
        state.run.status = run_state.ACTIVE
        state.run.end_reason = nil
        state.run.pb = false
        state.run.start_time = os.time() -- Reset the start time.
        state.run.warp_map = {} -- Clear previous warp data.
        state.run.star_map = {} -- Clear previous star data.
        CONFIG.USER.ATTEMPTS = CONFIG.USER.ATTEMPTS + 1 -- Increment the attempt count.
    end

    if state.game.level_id == 1 and state.run.status == run_state.COMPLETE then
        state.run.status = run_state.INACTIVE
    end
end

-- Check for run-ending conditions (e.g., Mario dying, falling, environmental hazards).
local function check_run_over_conditions()
    if state.mario.hp == 0 then
        if last_state.mario.velocity.y <= -55 then
            state.run.end_reason = 'Fall Damage'
        elseif state.mario.hurt_counter > 0 then
            state.run.end_reason = 'Enemy Damage'
        elseif state.mario.is_taking_gas_damage then
            state.run.end_reason = 'Suffocated by Hazy Gas'
        elseif state.mario.is_in_water then
            if state.game.level_id == 10 then
                state.run.end_reason = 'Frozen in Cold Water'
            elseif last_state.mario.hp == 1 then
                state.run.end_reason = 'Drowned'
            else
                state.run.end_reason = "HOW?" -- Fallback for an unhandled case.
            end
        else
            state.run.end_reason = 'Environment Hazard'
        end
    elseif state.game.delayed_warp_op == 18 or state.game.delayed_warp_op == 20 then
        state.run.end_reason = 'Fell Out of Level'
    end

    -- If an end reason is determined, mark the run as pending and record the end time.
    if state.run.end_reason then
        state.run.end_time = os.time()
        state.run.status = run_state.PENDING

        -- Check if the current star count is a new personal best.
        if CONFIG.USER.PB_STARS < state.run.stars then
            CONFIG.USER.PB_STARS = state.run.stars
            state.run.pb = true
        end
    end
end

-- Format a duration (in seconds) as a string in HH:MM:SS format.
local function format_time(seconds)
    local hours = math.floor(seconds / 3600)
    local minutes = math.floor((seconds % 3600) / 60)
    local seconds = math.floor(seconds % 60)
    return string.format("%02d:%02d:%02d", hours, minutes, seconds)
end

-- Write detailed run data as a CSV line to the attempts data file.
local function write_run_data_csv()
    local seed_key = string.format("%s_%s", state.run.seed, os.date("%Y%m%d%H%M%S"))
    local attempt_number = CONFIG.USER.ATTEMPTS
    local timestamp = os.date("%Y-%m-%d %H:%M:%S", state.run.start_time)
    local stars = state.run.stars or 0
    local level_name = get_level_name(state.game.level_id)
    local time_taken = state.run.end_time - state.run.start_time

    -- Create a summary string for stars collected per level.
    local stars_collected = ""
    for abbr, count in pairs(state.run.star_map) do
        stars_collected = stars_collected .. string.format("%s:%d ", abbr, count)
    end
    if stars_collected ~= "" then
        stars_collected = stars_collected:sub(1, -2) -- Remove trailing space.
    end

    -- Format the CSV line with all relevant run data.
    local csv_line = string.format("%d,%s,%s,%d,%s,%s,%s,%s\n", attempt_number, seed_key, timestamp, stars,
        format_time(time_taken), level_name, state.run.end_reason, stars_collected)

    -- Append the CSV line to the attempt data file.
    local file = io.open(CONFIG.FILES.ATTEMPT_DATA, "a")
    if file then
        file:write(csv_line)
        file:close()
    end
end

-- Write the warp map data as a JSON file.
local function write_warp_log()
    local file = io.open(CONFIG.FILES.WARP_LOG, "w")
    if file then
        file:write(json.encode(state.run.warp_map))
        file:close()
    end
end

-- Write all run-related data to files and mark the run as complete.
local function write_data()
    write_attempts_file() -- Write the attempt count.
    write_pb_stars_file() -- Write the PB star count.
    write_run_data_csv() -- Write detailed run data as CSV.
    write_warp_log() -- Write the warp map as JSON.
    state.run.status = run_state.COMPLETE
end

local function load_config()
    -- Load the configuration file if it exists.
    local file = io.open("config.json", "r")
    if file then
        local config_data = json.decode(file:read("*all"))
        if config_data then
            CONFIG.BACKGROUND_IMAGE = config_data.BACKGROUND_IMAGE
            CONFIG.SHOW_SONG_TITLE = config_data.SHOW_SONG_TITLE
        end
        file:close()
    end
end

local function save_config()
    -- Save the configuration to a file.
    local file = io.open("config.json", "w+")
    if file then
        local config_data = {
            BACKGROUND_IMAGE = CONFIG.BACKGROUND_IMAGE,
            SHOW_SONG_TITLE = CONFIG.SHOW_SONG_TITLE
        }
        file:write(json.encode(config_data))
        file:close()
    end
end

-- Render the on-screen UI overlay with lua-imgui
local function render_ui()
    local game_width = client.bufferwidth()
    local game_height = client.bufferheight()
    local ui_width = math.floor((game_height * (16 / 9)) - game_width) - 20
    local font_size = math.max(math.floor(game_height / 50), 8)
    local char_width = math.floor(font_size / 1.6)
    local logo_size = math.floor(game_height / 12)
    client.SetGameExtraPadding(0, 0, ui_width + 20, 0)

    imgui.NewFrame()
    
    -- Draw background image if selected
    if CONFIG.BACKGROUND_IMAGE ~= "(None)" then
        imgui.Image("img/bg/" .. CONFIG.BACKGROUND_IMAGE .. ".jpg", ui_width, game_height)
    end
    
    -- Draw tracker logo
    imgui.SetCursorPos(game_width, game_height - (logo_size + 20))
    imgui.Image("img/logo.png", logo_size, logo_size)
    
    -- Draw tracker title
    imgui.SetCursorPos(game_width + math.floor(ui_width / 2), font_size)
    imgui.TextColored(1, 1, 1, 1, "IronMario Tracker")
    
    -- Render attempt number
    imgui.SetCursorPos(game_width, font_size * 3)
    imgui.Text("Attempt #" .. CONFIG.USER.ATTEMPTS)
    
    -- Render elapsed time
    imgui.SetCursorPos(game_width, font_size * 5)
    if state.run.status == run_state.ACTIVE then
        imgui.Text("Time: " .. format_time(os.time() - state.run.start_time))
    else
        imgui.Text("Time: " .. format_time(state.run.end_time - state.run.start_time))
    end
    
    -- Render current star count and PB
    imgui.SetCursorPos(game_width, font_size * 4)
    imgui.Text("Stars: " .. state.run.stars)
    imgui.SetCursorPos(game_width + math.floor(ui_width / 3), font_size * 4)
    imgui.TextColored(1, 1, 0, 1, "PB: " .. CONFIG.USER.PB_STARS)
    
    -- Render current level name and seed
    imgui.SetCursorPos(game_width, font_size * 6)
    imgui.Text("Level: " .. get_level_name(state.game.level_id))
    imgui.SetCursorPos(game_width, font_size * 7)
    imgui.Text("Seed: " .. state.run.seed)
    
    -- Render warp map
    imgui.SetCursorPos(game_width + math.floor(ui_width / 2), font_size * 9)
    imgui.TextColored(1, 0.5, 0, 1, "== Warp Map ==")
    for _, key in ipairs(ordered_keys) do
        if state.run.warp_map[key] then
            imgui.Text(string.format("%s → %s", key, state.run.warp_map[key]))
        end
    end
    
    -- Render stars collected
    imgui.SetCursorPos(game_width + math.floor(ui_width / 2), font_size * 12)
    imgui.TextColored(1, 1, 0, 1, "== Stars Collected ==")
    for _, key in ipairs(ordered_keys) do
        if state.run.star_map[key] then
            imgui.Text(string.format("%s: %d", key, state.run.star_map[key]))
        end
    end
    
    -- Optionally display the current song title
    if CONFIG.SHOW_SONG_TITLE and CONFIG.MUSIC_DATA.SONG_MAP[state.game.song] then
        imgui.SetCursorPos(20 + math.floor(char_width / 2), game_height - (20 + math.floor(font_size * 1.25)))
        imgui.Text(get_song_name(state.game.song))
    end
    
    imgui.Render()
end

end

load_config()

-- Read stored attempt count and personal best star count from files.
read_attempts_file()
read_pb_stars_file()

console.clear() -- Clear the console for a clean output.

-- Main loop: executes every frame.
while true do
    memory.usememorydomain("RDRAM") -- Switch to the RDRAM memory domain for reading game data.

    -- Process on every other frame to reduce CPU load.
    if emu.framecount() % 2 == 0 then
        -- Update game state if the run isn't already pending (i.e., if it's still in progress).
        if state.run.status ~= run_state.PENDING then
            update_game_state()
        end

        -- If the run is active, check for any conditions that signal the run is over.
        if state.run.status == run_state.ACTIVE then
            check_run_over_conditions()
        end

        -- If a run has ended (pending state), write the run data to files.
        if state.run.status == run_state.PENDING then
            write_data()
        end

        -- Handle input: toggle the song title display if specific buttons are pressed.
        local current_inputs = joypad.get()
        if current_inputs["P1 L"] and current_inputs["P1 R"] and current_inputs["P1 A"] and current_inputs["P1 B"] and
            not state.input.music_toggle_pressed then
            state.input.music_toggle_pressed = true
            CONFIG.SHOW_SONG_TITLE = not CONFIG.SHOW_SONG_TITLE
        elseif (not current_inputs["P1 L"] or not current_inputs["P1 R"] or not current_inputs["P1 A"] or
            not current_inputs["P1 B"]) and state.input.music_toggle_pressed then
            state.input.music_toggle_pressed = false
        end

        render_ui() -- Render the UI overlay with the current state.
    end

    emu.frameadvance() -- Advance to the next frame.
end
