import { dom } from "../dom.js";
import { configData, state } from "../state.js";
import { OLEDHandler } from "./oled.js";
import { KeyHandler } from "./key.js";
import { ModalHandler } from "./modal.js";

export const SidebarHandler = {
	render: () => {
		configData.forEach((layer, index) => {
			const listContainer = document.getElementById(`layer-list-${index}`);
			if (!listContainer) return;

			listContainer.innerHTML = "";

			for (let r = 0; r < 4; r++) {
				for (let c = 0; c < 3; c++) {
					const keyId = `C${c}R${r}`;
					const keyData = layer.keys[keyId];
					const li = document.createElement("li");

					li.dataset.id = keyId;
					li.dataset.layer = index;

					if (keyData && keyData.value) {
						li.classList.add("sidebar-item-configured");
						let iconClass = "ti-keyboard";
						if (index === 2) iconClass = "ti-file-code";
						if (index === 3) iconClass = "ti-app-window";

						li.innerHTML = `
                            <i class="ti ${iconClass} sidebar-item-icon"></i>
                            <span>${keyData.label}</span>
                        `;
					} else {
						li.classList.add("sidebar-item-unconfigured");
						li.innerHTML = `
                            <i class="ti ti-plus sidebar-item-icon" style="opacity:0.3"></i>
                            <span>[${keyId}]</span>
                        `;
					}

					li.addEventListener("click", () => {
						if (state.activeLayerIndex !== index) {
							// Open correct layer if hidden
							const headers = document.querySelectorAll(
								".sidebar .category-header"
							);
							headers[index].click();
						}

						KeyHandler.simulatePress(keyId);
						const label =
							keyData && keyData.label ? keyData.label : "Unassigned";
						OLEDHandler.updateMain(label);

						if (state.activeLayerIndex > 0) {
							setTimeout(() => ModalHandler.open(keyId, label), 300);
						}
					});

					listContainer.appendChild(li);
				}
			}
		});
	},

	toggleLayer: (index, header) => {
		const keyList = header.nextElementSibling;
		const icon = header.querySelector(".category-icon");
		const name = header.querySelector(".category").innerText;

		// Close others
		document
			.querySelectorAll(".sidebar .key-list")
			.forEach((el) => el.classList.remove("open", "read-only-list"));
		document
			.querySelectorAll(".sidebar .category-icon")
			.forEach((el) => el.classList.remove("open"));

		// Open current
		keyList.classList.add("open");
		icon.classList.add("open");
		dom.title.textContent = name;

		state.activeLayerIndex = index;
		OLEDHandler.updateHeader(name, index);
		KeyHandler.updateFromData(index);

		// Read-only check
		if (state.activeLayerIndex === 0) {
			dom.macropad.classList.add("read-only");
			keyList.classList.add("read-only-list");
		} else {
			dom.macropad.classList.remove("read-only");
		}
	},

	toggleSidebar: () => {
		dom.sidebar.classList.toggle("collapsed");
		dom.hamburger.classList.toggle("active");
	}
};
