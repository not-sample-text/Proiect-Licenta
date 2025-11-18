import { dom } from "../dom.js";
import { configData } from "../state.js";

export const KeyHandler = {
	updateFromData: (layerIndex) => {
		// ! FIX: use .layers array
		const layerKeys = configData.layers[layerIndex].keys;

		dom.keys.forEach((key) => {
			const id = key.getAttribute("data-id");
			key.classList.remove("unassigned-key");

			if (layerKeys[id] && layerKeys[id].value) {
				key.innerText = layerKeys[id].label;
			} else {
				key.innerText = "Unassigned";
				key.classList.add("unassigned-key");
			}
		});
	},
	simulatePress: (keyId) => {
		const key = document.querySelector(
			`.macropad-container .key[data-id="${keyId}"]`
		);
		if (key) {
			key.classList.add("pressed-simulation");
			setTimeout(() => key.classList.remove("pressed-simulation"), 200);
		}
	}
};
