import { dom } from "../dom.js";
import { layerNameMap } from "../state.js";

export const OLEDHandler = {
	updateHeader: (fullName, index) => {
		const shortName = layerNameMap[fullName] || "LAYER " + (index + 1);
		dom.oled.layerName.innerText = shortName;
		dom.oled.mainText.innerText = "READY";
	},
	updateMain: (text) => {
		if (text === "Unassigned") {
			dom.oled.mainText.innerText = "UNASSIGNED";
		} else {
			// Truncate long text
			dom.oled.mainText.innerText =
				text.length > 12 ? text.substring(0, 10) + ".." : text;
		}
	}
};
