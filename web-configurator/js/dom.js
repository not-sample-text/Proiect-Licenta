export const dom = {
	macropad: document.querySelector(".macropad-container"),
	title: document.getElementById("layer-title"),
	sidebar: document.getElementById("main-sidebar"),
	hamburger: document.getElementById("hamburger-btn"),
	// Note: categoryHeaders are dynamic but exist on load, accessed via querySelectorAll in handlers
	keys: document.querySelectorAll(".macropad-container .key"),
	oled: {
		layerName: document.getElementById("oled-layer-name"),
		mainText: document.getElementById("oled-display-text")
	},
	knob: {
		container: document.querySelector(".rotary-encoder"),
		element: document.querySelector(".knob")
	},
	modal: {
		overlay: document.getElementById("config-modal"),
		keyId: document.getElementById("modal-key-id"),
		labelInput: document.getElementById("key-label"),
		shortcutDisplay: document.getElementById("shortcut-display"),
		shortcutValue: document.getElementById("shortcut-value"),
		scriptDisplay: document.getElementById("script-path-display"),
		appInput: document.getElementById("app-path-input"),
		cancelBtn: document.getElementById("btn-cancel"),
		saveBtn: document.getElementById("btn-save")
	},
	exportBtn: document.getElementById("btn-export-json"),
	importBtn: document.getElementById("btn-import-json"),
	importInput: document.getElementById("import-file-input"),
	resizer: document.querySelector(".resizer")
};
