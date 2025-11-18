import { configData } from "../state.js";
import { KeyHandler } from "../handlers/key.js";
import { SidebarHandler } from "../handlers/sidebar.js";
import { state } from "../state.js";

export const PersistenceHandler = {
	STORAGE_KEY: "macropad_config_v1",

	saveToLocal: () => {
		try {
			localStorage.setItem(
				PersistenceHandler.STORAGE_KEY,
				JSON.stringify(configData)
			);
			console.log("Auto-saved to LocalStorage");
		} catch (e) {
			console.warn("LocalStorage failed:", e);
		}
	},

	loadFromLocal: () => {
		const saved = localStorage.getItem(PersistenceHandler.STORAGE_KEY);
		if (saved) {
			try {
				const parsed = JSON.parse(saved);
				PersistenceHandler.applyConfig(parsed);
				console.log("Restored from LocalStorage");
				return true;
			} catch (e) {
				console.error("Corrupt LocalStorage data", e);
				return false;
			}
		}
		return false;
	},

	applyConfig: (newData) => {
		if (!Array.isArray(newData) || newData.length !== 4) {
			throw new Error("Invalid Config Structure");
		}

		// Update Data Model in place
		configData.splice(0, configData.length, ...newData);

		// Refresh UI
		SidebarHandler.render();
		if (state.activeLayerIndex !== -1) {
			KeyHandler.updateFromData(state.activeLayerIndex);
		}
	}
};
