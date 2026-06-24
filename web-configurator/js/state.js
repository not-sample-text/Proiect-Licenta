// Application State
export const state = {
    activeLayerIndex: 0,
    activeView: "KEYS",
    isKnobNavMode: false,
    volume: 50,
    knobRotation: 0
};

export const defaultLayer1Keys = {};
let fCount = 13;
for (let r = 0; r < 4; r++) {
    for (let c = 0; c < 3; c++) {
        defaultLayer1Keys[`C${c}R${r}`] = {
            label: `F${fCount}`,
            value: `F${fCount}`, // FIX: This is now the actual F13-F24 string, not "F_KEY"
            type: "KEY"
        };
        fCount++;
    }
}

// The Source of Truth for Configuration
export const configData = {
    layers: [
        { id: 0, name: "FN Keys", keys: { ...defaultLayer1Keys } },
        { id: 1, name: "Shortcuts", keys: {} },
        { id: 2, name: "Commands", keys: {} },
        { id: 3, name: "Launcher", keys: {} }
    ],
    lighting: {
        mode: "SOLID",
        brightness: 128,
        speed: 10,
        color: "#00e5ff"
    }
};

export const layerNameMap = {
    "Layer 1 - FN Keys": "FN KEYS",
    "Layer 2 - Shortcuts": "SHORTCUTS",
    "Layer 3 - Commands": "COMMANDS",
    "Layer 4 - Launcher": "LAUNCHER"
};
