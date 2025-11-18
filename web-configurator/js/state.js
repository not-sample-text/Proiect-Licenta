// Application State
export const state = {
	activeLayerIndex: -1,
	isKnobNavMode: false,
	volume: 50,
	knobRotation: 0
};

// Helper to generate Layer 1 (F13-F24)
const layer1Keys = {};
let fCount = 13;
for (let r = 0; r < 4; r++) {
	for (let c = 0; c < 3; c++) {
		layer1Keys[`C${c}R${r}`] = {
			label: `F${fCount}`,
			value: "F_KEY",
			type: "KEY"
		};
		fCount++;
	}
}

// The Source of Truth for Configuration
export const configData = [
	{ id: 0, name: "FN Keys", keys: layer1Keys },
	{ id: 1, name: "Shortcuts", keys: {} },
	{ id: 2, name: "Commands", keys: {} },
	{ id: 3, name: "Launcher", keys: {} }
];

export const layerNameMap = {
	"Layer 1 - FN Keys": "FN KEYS",
	"Layer 2 - Shortcuts": "SHORTCUTS",
	"Layer 3 - Commands": "COMMANDS",
	"Layer 4 - Launcher": "LAUNCHER"
};
