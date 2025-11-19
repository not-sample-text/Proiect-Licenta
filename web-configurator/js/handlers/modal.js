import { dom } from "../dom.js";
import { configData, state } from "../state.js";
import { PersistenceHandler } from "../utils/storage.js";
import { KeyHandler } from "./key.js";
import { SidebarHandler } from "./sidebar.js";

export const ModalHandler = {
	setupRecorder: () => {
		const recorder = document.querySelector(".shortcut-recorder");
		recorder.addEventListener("click", () => {
			recorder.classList.add("recording");
			dom.modal.shortcutDisplay.innerText = "Listening...";
		});
		recorder.addEventListener("keydown", (e) => {
			e.preventDefault();
			const keys = [];
			if (e.ctrlKey) keys.push("Ctrl");
			if (e.shiftKey) keys.push("Shift");
			if (e.altKey) keys.push("Alt");
			if (e.metaKey) keys.push("Meta");
			if (!["Control", "Shift", "Alt", "Meta"].includes(e.key)) {
				keys.push(e.key.toUpperCase());
			}
			const result = keys.join(" + ");
			dom.modal.shortcutDisplay.innerText = result;
			dom.modal.shortcutValue.value = result;

			if (!["Control", "Shift", "Alt", "Meta"].includes(e.key)) {
				recorder.blur();
				recorder.classList.remove("recording");
			}
		});
	},

	setupFilePickers: () => {
		const handleFile = (input, display) => {
			input.addEventListener("change", (e) => {
				if (e.target.files.length > 0) display.value = e.target.files[0].name;
			});
		};
		handleFile(
			document.getElementById("script-file-input"),
			dom.modal.scriptDisplay
		);
		handleFile(document.getElementById("app-file-input"), dom.modal.appInput);
	},

	open: (keyId, currentText) => {
		dom.modal.keyId.innerText = keyId;
		dom.modal.labelInput.value =
			currentText === "Unassigned" ? "" : currentText;

		dom.modal.overlay.classList.remove("hidden");
		document
			.querySelectorAll(".input-type-container")
			.forEach((el) => el.classList.add("hidden"));

		// ! FIX: Access configData.layers array properly
		const savedKeyData = configData.layers[state.activeLayerIndex].keys[keyId];
		const savedValue = savedKeyData ? savedKeyData.value : "";

		// Handle Layer 0 (FN Keys) - Only Name input is shown (default behavior)
		if (state.activeLayerIndex === 0) {
			// Intentionally empty: Layer 0 allows renaming but has no extra inputs
		} else if (state.activeLayerIndex === 1) {
			document
				.getElementById("input-container-shortcut")
				.classList.remove("hidden");
			dom.modal.shortcutDisplay.innerText = savedValue || "Click to Record...";
			dom.modal.shortcutValue.value = savedValue;
		} else if (state.activeLayerIndex === 2) {
			document
				.getElementById("input-container-script")
				.classList.remove("hidden");
			dom.modal.scriptDisplay.value = savedValue;
		} else if (state.activeLayerIndex === 3) {
			document.getElementById("input-container-app").classList.remove("hidden");
			dom.modal.appInput.value = savedValue;
		}
	},

	save: () => {
		const layerIdx = state.activeLayerIndex;
		const keyId = dom.modal.keyId.innerText;
		let displayName = dom.modal.labelInput.value.trim();
		let value = "";

		// ! FIX: Handle Layer 0 Saving (Nickname only)
		if (layerIdx === 0) {
			if (!displayName) displayName = configData.layers[0].keys[keyId].value; // Revert to Fxx if empty

			// Only update label. Keep existing Value and Type.
			configData.layers[0].keys[keyId].label = displayName;

			PersistenceHandler.saveToLocal();
			KeyHandler.updateFromData(layerIdx);
			SidebarHandler.render();
			ModalHandler.close();
			return;
		}

		// Logic for Layers 1, 2, 3
		if (layerIdx === 1) value = dom.modal.shortcutValue.value;
		else if (layerIdx === 2) value = dom.modal.scriptDisplay.value;
		else if (layerIdx === 3) value = dom.modal.appInput.value;

		if (!value) {
			delete configData.layers[layerIdx].keys[keyId];
			PersistenceHandler.saveToLocal();
			KeyHandler.updateFromData(layerIdx);
			SidebarHandler.render();
			ModalHandler.close();
			return;
		}

		if (value && !displayName) {
			displayName = value;
		}

		configData.layers[layerIdx].keys[keyId] = {
			label: displayName,
			value: value,
			type: layerIdx === 1 ? "SHORTCUT" : layerIdx === 2 ? "SCRIPT" : "APP"
		};

		PersistenceHandler.saveToLocal();
		KeyHandler.updateFromData(layerIdx);
		SidebarHandler.render();
		ModalHandler.close();
	},

	close: () => {
		dom.modal.overlay.classList.add("hidden");
	},

	init: () => {
		dom.modal.cancelBtn.addEventListener("click", ModalHandler.close);
		dom.modal.saveBtn.addEventListener("click", ModalHandler.save);
		dom.modal.overlay.addEventListener("click", (e) => {
			if (e.target === dom.modal.overlay) ModalHandler.close();
		});
		ModalHandler.setupRecorder();
		ModalHandler.setupFilePickers();
	}
};
